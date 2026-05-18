import importlib.util
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT_PATH = REPO_ROOT / "scripts" / "prepare_multicam_dataset.py"
SPEC = importlib.util.spec_from_file_location("prepare_multicam_dataset", SCRIPT_PATH)
prepare = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
sys.modules[SPEC.name] = prepare
SPEC.loader.exec_module(prepare)


class PrepareMulticamDatasetTests(unittest.TestCase):
    def test_frame_name_is_multipleview_compatible(self):
        self.assertEqual(prepare.frame_name(1, "jpg"), "frame_00001.jpg")
        self.assertEqual(prepare.frame_name(123, "jpg"), "frame_00123.jpg")

    def test_camera_ids_must_be_contiguous_and_ordered(self):
        cameras = [
            prepare.CameraConfig(id="cam01", video=Path("one.mov")),
            prepare.CameraConfig(id="cam03", video=Path("three.mov")),
        ]
        with self.assertRaises(prepare.DatasetPrepError):
            prepare.validate_camera_ids(cameras)

    def test_manifest_json_defaults_output_dir_and_resolves_videos(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            manifest_path = root / "manifest.json"
            manifest_path.write_text(
                json.dumps(
                    {
                        "scene_name": "redzone_poc",
                        "cameras": [
                            {"id": "cam01", "video": "iphone_01.mov"},
                            {"id": "cam02", "video": "iphone_02.mov"},
                        ],
                    }
                )
            )

            manifest = prepare.load_manifest(manifest_path)

            self.assertEqual(manifest.scene_name, "redzone_poc")
            self.assertEqual(manifest.output_dir, Path("data/multipleview/redzone_poc"))
            self.assertEqual(manifest.image_ext, "jpg")
            self.assertEqual(manifest.cameras[0].video, root / "iphone_01.mov")

    def test_synced_window_uses_offsets_and_shortest_available_duration(self):
        manifest = prepare.Manifest(
            scene_name="redzone_poc",
            output_dir=Path("data/multipleview/redzone_poc"),
            target_fps=30,
            image_ext="jpg",
            cameras=[
                prepare.CameraConfig(id="cam01", video=Path("one.mov"), sync_offset_seconds=0.5),
                prepare.CameraConfig(id="cam02", video=Path("two.mov"), sync_offset_frames=15),
            ],
        )
        infos = {
            "cam01": video_info(Path("one.mov"), duration=5.0),
            "cam02": video_info(Path("two.mov"), duration=4.0),
        }

        windows, duration = prepare.compute_windows(manifest, infos)

        self.assertAlmostEqual(windows[0].start_seconds, 0.5)
        self.assertAlmostEqual(windows[1].start_seconds, 0.5)
        self.assertAlmostEqual(duration, 3.5)
        self.assertEqual(prepare.expected_frame_count(duration, 30), 105)

    @unittest.skipIf(shutil.which("ffmpeg") is None or shutil.which("ffprobe") is None, "ffmpeg/ffprobe unavailable")
    def test_integration_extracts_matching_frame_folders(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            video1 = root / "cam1.mp4"
            video2 = root / "cam2.mp4"
            make_test_video(video1, "red")
            make_test_video(video2, "blue")

            manifest_path = root / "manifest.json"
            output_dir = root / "data" / "multipleview" / "redzone_poc"
            manifest_path.write_text(
                json.dumps(
                    {
                        "scene_name": "redzone_poc",
                        "output_dir": str(output_dir),
                        "target_fps": 5,
                        "cameras": [
                            {"id": "cam01", "video": str(video1)},
                            {"id": "cam02", "video": str(video2)},
                        ],
                    }
                )
            )

            result = subprocess.run(
                [
                    "python3",
                    str(SCRIPT_PATH),
                    "--manifest",
                    str(manifest_path),
                ],
                cwd=REPO_ROOT,
                text=True,
                capture_output=True,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue((output_dir / "cam01" / "frame_00001.jpg").exists())
            self.assertTrue((output_dir / "cam02" / "frame_00001.jpg").exists())
            self.assertTrue((output_dir / "dataset_report.json").exists())
            self.assertEqual(
                len(list((output_dir / "cam01").glob("frame_*.jpg"))),
                len(list((output_dir / "cam02").glob("frame_*.jpg"))),
            )


def video_info(path, duration):
    return prepare.VideoInfo(
        path=path,
        width=1920,
        height=1080,
        fps=30,
        avg_fps=30,
        duration=duration,
        codec="h264",
        container="mov",
        frame_count=None,
    )


def make_test_video(path, color):
    subprocess.run(
        [
            "ffmpeg",
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-f",
            "lavfi",
            "-i",
            f"color=c={color}:s=160x90:r=5:d=1",
            str(path),
        ],
        check=True,
    )


if __name__ == "__main__":
    unittest.main()
