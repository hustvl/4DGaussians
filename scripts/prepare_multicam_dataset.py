#!/usr/bin/env python3
"""Prepare synchronized multi-camera videos for 4DGaussians multipleview training."""

from __future__ import annotations

import argparse
import json
import math
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path
from typing import Any


CAMERA_ID_RE = re.compile(r"^cam(\d{2})$")
FRAME_RE_TEMPLATE = r"^frame_(\d{{5}})\.{ext}$"


class DatasetPrepError(RuntimeError):
    pass


@dataclass(frozen=True)
class CameraConfig:
    id: str
    video: Path
    sync_offset_seconds: float = 0.0
    sync_offset_frames: int = 0
    trim_start_seconds: float = 0.0
    trim_end_seconds: float = 0.0


@dataclass(frozen=True)
class Manifest:
    scene_name: str
    output_dir: Path
    target_fps: float | None
    image_ext: str
    cameras: list[CameraConfig]
    duration_warning_threshold_seconds: float = 1.0


@dataclass(frozen=True)
class VideoInfo:
    path: Path
    width: int
    height: int
    fps: float
    avg_fps: float
    duration: float
    codec: str
    container: str
    frame_count: int | None


@dataclass(frozen=True)
class ExtractionWindow:
    camera: CameraConfig
    start_seconds: float
    duration_seconds: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert synced iPhone/GoPro videos into the 4DGaussians multipleview dataset layout."
    )
    parser.add_argument("--manifest", required=True, help="Path to a YAML or JSON capture manifest.")
    parser.add_argument("--validate-only", action="store_true", help="Validate inputs without extracting frames.")
    parser.add_argument("--overwrite", action="store_true", help="Replace an existing output directory.")
    return parser.parse_args()


def load_manifest(path: Path) -> Manifest:
    if not path.exists():
        raise DatasetPrepError(f"Manifest does not exist: {path}")

    raw = path.read_text()
    if path.suffix.lower() == ".json":
        data = json.loads(raw)
    else:
        try:
            import yaml  # type: ignore
        except ImportError as exc:
            raise DatasetPrepError(
                "YAML manifests require PyYAML. Install it with `pip install pyyaml`, "
                "or use a .json manifest."
            ) from exc
        data = yaml.safe_load(raw)

    if not isinstance(data, dict):
        raise DatasetPrepError("Manifest must be a mapping/object.")

    scene_name = require_string(data, "scene_name")
    output_dir = Path(data.get("output_dir") or Path("data") / "multipleview" / scene_name)
    target_fps = data.get("target_fps")
    image_ext = str(data.get("image_ext", "jpg")).lower().lstrip(".")
    if image_ext not in {"jpg", "jpeg"}:
        raise DatasetPrepError("Only jpg/jpeg output is supported for the multipleview loader.")

    cameras_raw = data.get("cameras")
    if not isinstance(cameras_raw, list) or not cameras_raw:
        raise DatasetPrepError("Manifest must include a non-empty `cameras` list.")

    cameras = []
    manifest_dir = path.parent
    for item in cameras_raw:
        if not isinstance(item, dict):
            raise DatasetPrepError("Each camera entry must be a mapping/object.")
        camera_id = require_string(item, "id")
        video_value = require_string(item, "video")
        video_path = Path(video_value)
        if not video_path.is_absolute():
            video_path = manifest_dir / video_path
        cameras.append(
            CameraConfig(
                id=camera_id,
                video=video_path,
                sync_offset_seconds=float(item.get("sync_offset_seconds", 0.0)),
                sync_offset_frames=int(item.get("sync_offset_frames", 0)),
                trim_start_seconds=float(item.get("trim_start_seconds", item.get("trim_start", 0.0))),
                trim_end_seconds=float(item.get("trim_end_seconds", item.get("trim_end", 0.0))),
            )
        )

    threshold = float(data.get("duration_warning_threshold_seconds", 1.0))
    return Manifest(
        scene_name=scene_name,
        output_dir=output_dir,
        target_fps=float(target_fps) if target_fps is not None else None,
        image_ext=image_ext,
        cameras=cameras,
        duration_warning_threshold_seconds=threshold,
    )


def require_string(data: dict[str, Any], key: str) -> str:
    value = data.get(key)
    if not isinstance(value, str) or not value.strip():
        raise DatasetPrepError(f"Manifest field `{key}` must be a non-empty string.")
    return value.strip()


def validate_camera_ids(cameras: list[CameraConfig]) -> None:
    ids = [camera.id for camera in cameras]
    if len(ids) != len(set(ids)):
        raise DatasetPrepError(f"Camera IDs must be unique: {ids}")

    expected = [f"cam{i:02d}" for i in range(1, len(cameras) + 1)]
    if ids != expected:
        raise DatasetPrepError(f"Camera IDs must be contiguous and ordered: expected {expected}, got {ids}")

    for camera_id in ids:
        if not CAMERA_ID_RE.match(camera_id):
            raise DatasetPrepError(f"Invalid camera ID `{camera_id}`; expected cam01, cam02, ...")


def check_tools() -> tuple[str, str]:
    ffmpeg = shutil.which("ffmpeg")
    ffprobe = shutil.which("ffprobe")
    missing = [name for name, value in (("ffmpeg", ffmpeg), ("ffprobe", ffprobe)) if value is None]
    if missing:
        raise DatasetPrepError(f"Missing required tool(s): {', '.join(missing)}")
    return ffmpeg or "ffmpeg", ffprobe or "ffprobe"


def probe_video(ffprobe: str, video_path: Path) -> VideoInfo:
    if not video_path.exists():
        raise DatasetPrepError(f"Video file does not exist: {video_path}")

    cmd = [
        ffprobe,
        "-v",
        "error",
        "-print_format",
        "json",
        "-show_format",
        "-show_streams",
        str(video_path),
    ]
    result = subprocess.run(cmd, check=False, text=True, capture_output=True)
    if result.returncode != 0:
        raise DatasetPrepError(f"ffprobe failed for {video_path}: {result.stderr.strip()}")

    data = json.loads(result.stdout)
    streams = data.get("streams") or []
    video_stream = next((stream for stream in streams if stream.get("codec_type") == "video"), None)
    if not video_stream:
        raise DatasetPrepError(f"No video stream found in {video_path}")

    width = int(video_stream.get("width") or 0)
    height = int(video_stream.get("height") or 0)
    fps = parse_rate(video_stream.get("r_frame_rate"))
    avg_fps = parse_rate(video_stream.get("avg_frame_rate"))
    duration = parse_duration(video_stream.get("duration")) or parse_duration((data.get("format") or {}).get("duration"))
    codec = str(video_stream.get("codec_name") or "unknown")
    container = str((data.get("format") or {}).get("format_name") or "unknown")
    frame_count = parse_int(video_stream.get("nb_frames"))

    if width <= 0 or height <= 0:
        raise DatasetPrepError(f"Invalid resolution for {video_path}: {width}x{height}")
    if fps <= 0 or avg_fps <= 0:
        raise DatasetPrepError(f"Invalid FPS metadata for {video_path}")
    if duration <= 0:
        raise DatasetPrepError(f"Invalid duration metadata for {video_path}")

    return VideoInfo(video_path, width, height, fps, avg_fps, duration, codec, container, frame_count)


def parse_rate(value: Any) -> float:
    if not value or value == "0/0":
        return 0.0
    return float(Fraction(str(value)))


def parse_duration(value: Any) -> float:
    try:
        return float(value)
    except (TypeError, ValueError):
        return 0.0


def parse_int(value: Any) -> int | None:
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def validate_video_compatibility(manifest: Manifest, infos: list[VideoInfo]) -> list[str]:
    warnings: list[str] = []
    first = infos[0]

    for info in infos:
        if (info.width, info.height) != (first.width, first.height):
            raise DatasetPrepError(
                f"Resolution mismatch: {info.path} is {info.width}x{info.height}, "
                f"expected {first.width}x{first.height}"
            )
        if not math.isclose(info.fps, first.fps, rel_tol=0.001, abs_tol=0.01):
            raise DatasetPrepError(f"FPS mismatch: {info.path} is {info.fps:.3f}, expected {first.fps:.3f}")
        if abs(info.fps - info.avg_fps) > 0.01:
            warnings.append(
                f"{info.path.name} may be variable-frame-rate: r_frame_rate={info.fps:.3f}, "
                f"avg_frame_rate={info.avg_fps:.3f}"
            )

    codecs = {info.codec for info in infos}
    containers = {info.container for info in infos}
    if len(codecs) > 1:
        warnings.append(f"Codec differs across cameras: {sorted(codecs)}")
    if len(containers) > 1:
        warnings.append(f"Container differs across cameras: {sorted(containers)}")

    durations = [info.duration for info in infos]
    if max(durations) - min(durations) > manifest.duration_warning_threshold_seconds:
        warnings.append(
            f"Video durations differ by {max(durations) - min(durations):.2f}s "
            f"(threshold {manifest.duration_warning_threshold_seconds:.2f}s)."
        )

    if first.width >= 3840 or first.height >= 2160:
        warnings.append(f"High resolution detected: {first.width}x{first.height}. Consider a smaller pilot first.")
    if len(infos) < 6:
        warnings.append(f"Only {len(infos)} cameras listed. Large field captures usually need more viewpoints.")

    return warnings


def compute_windows(manifest: Manifest, infos_by_camera: dict[str, VideoInfo]) -> tuple[list[ExtractionWindow], float]:
    fps = manifest.target_fps or next(iter(infos_by_camera.values())).fps
    available_durations = []
    starts_by_camera: dict[str, float] = {}

    for camera in manifest.cameras:
        info = infos_by_camera[camera.id]
        start = camera.sync_offset_seconds + camera.trim_start_seconds + (camera.sync_offset_frames / fps)
        if start < 0:
            raise DatasetPrepError(f"{camera.id} has a negative effective start time: {start:.3f}s")
        available = info.duration - start - camera.trim_end_seconds
        if available <= 0:
            raise DatasetPrepError(f"{camera.id} is too short after sync/trim.")
        starts_by_camera[camera.id] = start
        available_durations.append(available)

    duration = min(available_durations)
    if duration <= 0:
        raise DatasetPrepError("No shared synced duration remains after offsets and trims.")

    windows = [
        ExtractionWindow(camera=camera, start_seconds=starts_by_camera[camera.id], duration_seconds=duration)
        for camera in manifest.cameras
    ]
    return windows, duration


def expected_frame_count(duration_seconds: float, fps: float) -> int:
    return max(1, int(math.floor(duration_seconds * fps + 1e-6)))


def frame_name(index: int, image_ext: str) -> str:
    return f"frame_{index:05d}.{image_ext}"


def extract_frames(
    ffmpeg: str,
    manifest: Manifest,
    windows: list[ExtractionWindow],
    fps: float,
    frame_count: int,
) -> None:
    for window in windows:
        output_dir = manifest.output_dir / window.camera.id
        output_dir.mkdir(parents=True, exist_ok=True)
        pattern = output_dir / f"frame_%05d.{manifest.image_ext}"
        cmd = [
            ffmpeg,
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-ss",
            f"{window.start_seconds:.6f}",
            "-t",
            f"{window.duration_seconds:.6f}",
            "-i",
            str(window.camera.video),
            "-vf",
            f"fps={fps}",
            "-frames:v",
            str(frame_count),
            "-q:v",
            "2",
            "-start_number",
            "1",
            str(pattern),
        ]
        result = subprocess.run(cmd, check=False, text=True, capture_output=True)
        if result.returncode != 0:
            raise DatasetPrepError(f"ffmpeg failed for {window.camera.id}: {result.stderr.strip()}")


def validate_extracted_dataset(manifest: Manifest) -> list[str]:
    warnings: list[str] = []
    counts: dict[str, int] = {}
    expected_size: tuple[int, int] | None = None

    for camera in manifest.cameras:
        camera_dir = manifest.output_dir / camera.id
        if not camera_dir.is_dir():
            raise DatasetPrepError(f"Missing camera folder after extraction: {camera_dir}")

        frames = sorted(camera_dir.glob(f"frame_*.{manifest.image_ext}"))
        if not frames:
            raise DatasetPrepError(f"No frames extracted for {camera.id}")

        validate_frame_sequence(frames, manifest.image_ext, camera.id)
        counts[camera.id] = len(frames)

        sample_indices = sorted({0, len(frames) // 2, len(frames) - 1})
        for idx in sample_indices:
            size, sample_warnings = inspect_image(frames[idx])
            warnings.extend(f"{camera.id}/{frames[idx].name}: {warning}" for warning in sample_warnings)
            if expected_size is None:
                expected_size = size
            elif size != expected_size:
                raise DatasetPrepError(
                    f"Frame dimension mismatch: {camera.id}/{frames[idx].name} is {size}, expected {expected_size}"
                )

        first_frame = camera_dir / frame_name(1, manifest.image_ext)
        if not first_frame.exists():
            raise DatasetPrepError(f"Missing first frame required by multipleviewprogress.sh: {first_frame}")

    unique_counts = set(counts.values())
    if len(unique_counts) != 1:
        raise DatasetPrepError(f"Frame counts differ across cameras: {counts}")

    total_images = sum(counts.values())
    if total_images > 10000:
        warnings.append(f"Large extracted dataset: {total_images} images. COLMAP and GPU runs may be expensive.")

    return warnings


def validate_frame_sequence(frames: list[Path], image_ext: str, camera_id: str) -> None:
    pattern = re.compile(FRAME_RE_TEMPLATE.format(ext=re.escape(image_ext)))
    for expected_index, path in enumerate(frames, start=1):
        match = pattern.match(path.name)
        if not match:
            raise DatasetPrepError(f"{camera_id} has invalid frame filename: {path.name}")
        actual_index = int(match.group(1))
        if actual_index != expected_index:
            raise DatasetPrepError(
                f"{camera_id} frame sequence gap: expected {frame_name(expected_index, image_ext)}, got {path.name}"
            )


def inspect_image(path: Path) -> tuple[tuple[int, int], list[str]]:
    try:
        from PIL import Image, ImageFilter, ImageStat
    except ImportError as exc:
        raise DatasetPrepError("Image validation requires Pillow. Install it with `pip install pillow`.") from exc

    warnings: list[str] = []
    with Image.open(path) as image:
        image.verify()

    with Image.open(path) as image:
        rgb = image.convert("RGB")
        size = rgb.size
        stat = ImageStat.Stat(rgb.resize((1, 1)))
        brightness = sum(stat.mean) / 3.0
        if brightness < 20:
            warnings.append("sample looks very dark")
        elif brightness > 235:
            warnings.append("sample looks very bright")

        gray = rgb.convert("L")
        extrema = gray.resize((256, 256)).filter(ImageFilter.FIND_EDGES).getextrema()
        if extrema[1] - extrema[0] < 8:
            warnings.append("sample has very low local contrast; possible blur or blank frame")

    return size, warnings


def prepare_output_dir(path: Path, overwrite: bool, validate_only: bool) -> None:
    if validate_only:
        return
    if path.exists():
        if not overwrite:
            raise DatasetPrepError(f"Output directory already exists: {path}. Pass --overwrite to replace it.")
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def build_report(
    manifest: Manifest,
    infos_by_camera: dict[str, VideoInfo],
    windows: list[ExtractionWindow],
    fps: float,
    frame_count: int,
    warnings: list[str],
    validate_only: bool,
) -> dict[str, Any]:
    return {
        "scene_name": manifest.scene_name,
        "output_dir": str(manifest.output_dir),
        "validate_only": validate_only,
        "target_fps": fps,
        "expected_frame_count_per_camera": frame_count,
        "expected_total_images": frame_count * len(manifest.cameras),
        "warnings": warnings,
        "videos": {
            camera_id: {
                "path": str(info.path),
                "width": info.width,
                "height": info.height,
                "fps": info.fps,
                "avg_fps": info.avg_fps,
                "duration": info.duration,
                "codec": info.codec,
                "container": info.container,
                "frame_count": info.frame_count,
            }
            for camera_id, info in infos_by_camera.items()
        },
        "windows": {
            window.camera.id: {
                "start_seconds": window.start_seconds,
                "duration_seconds": window.duration_seconds,
            }
            for window in windows
        },
    }


def write_reports(output_dir: Path, report: dict[str, Any]) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "dataset_report.json").write_text(json.dumps(report, indent=2) + "\n")
    (output_dir / "dataset_report.md").write_text(render_markdown_report(report))


def render_markdown_report(report: dict[str, Any]) -> str:
    lines = [
        f"# Dataset Report: {report['scene_name']}",
        "",
        f"- Output: `{report['output_dir']}`",
        f"- Target FPS: `{report['target_fps']}`",
        f"- Frames per camera: `{report['expected_frame_count_per_camera']}`",
        f"- Total images: `{report['expected_total_images']}`",
        f"- Validate only: `{report['validate_only']}`",
        "",
        "## Cameras",
        "",
        "| Camera | Resolution | FPS | Duration | Codec | Video |",
        "| --- | --- | --- | --- | --- | --- |",
    ]
    for camera_id, info in report["videos"].items():
        lines.append(
            f"| {camera_id} | {info['width']}x{info['height']} | {info['fps']:.3f} | "
            f"{info['duration']:.2f}s | {info['codec']} | `{info['path']}` |"
        )
    lines.extend(["", "## Warnings", ""])
    if report["warnings"]:
        lines.extend(f"- {warning}" for warning in report["warnings"])
    else:
        lines.append("- None")
    lines.append("")
    return "\n".join(lines)


def print_summary(report: dict[str, Any]) -> None:
    print(f"Scene: {report['scene_name']}")
    print(f"Output: {report['output_dir']}")
    print(f"Cameras: {len(report['videos'])}")
    print(f"Target FPS: {report['target_fps']:.3f}")
    print(f"Frames per camera: {report['expected_frame_count_per_camera']}")
    print(f"Total images: {report['expected_total_images']}")
    if report["warnings"]:
        print("Warnings:")
        for warning in report["warnings"]:
            print(f"  - {warning}")
    else:
        print("Warnings: none")


def main() -> int:
    args = parse_args()
    try:
        manifest = load_manifest(Path(args.manifest))
        validate_camera_ids(manifest.cameras)
        ffmpeg, ffprobe = check_tools()

        infos_by_camera = {
            camera.id: probe_video(ffprobe, camera.video)
            for camera in manifest.cameras
        }
        warnings = validate_video_compatibility(manifest, list(infos_by_camera.values()))
        fps = manifest.target_fps or next(iter(infos_by_camera.values())).fps
        windows, duration = compute_windows(manifest, infos_by_camera)
        frame_count = expected_frame_count(duration, fps)
        if frame_count * len(manifest.cameras) > 10000:
            warnings.append(
                f"Expected extracted dataset is large: {frame_count * len(manifest.cameras)} images."
            )

        prepare_output_dir(manifest.output_dir, args.overwrite, args.validate_only)
        if not args.validate_only:
            extract_frames(ffmpeg, manifest, windows, fps, frame_count)
            warnings.extend(validate_extracted_dataset(manifest))

        report = build_report(manifest, infos_by_camera, windows, fps, frame_count, warnings, args.validate_only)
        write_reports(manifest.output_dir, report)
        print_summary(report)
        return 0
    except DatasetPrepError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
