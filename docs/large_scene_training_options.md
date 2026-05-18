# Large Scene Training Options

Use this file to track practical paths for training larger custom scenes with this repo. The baseline assumption is a Linux machine with an NVIDIA CUDA GPU.

## Options

| Option | Best for | GPU target | Notes |
| --- | --- | --- | --- |
| Small pilot scene | Proving the data pipeline | RTX 3090 / 4090, 24GB | Start here before spending on larger GPUs. |
| Full scene, reduced quality | First full-scene pass | RTX 4090 / A5000, 24GB | Lower input resolution and limit densification. |
| Full scene, higher quality | Medium-large scenes | L40S / A6000 / RTX 6000 Ada, 48GB | Good default if 24GB hits OOM. |
| Heavy scene, fewer compromises | Large scenes with many frames/views | A100 80GB / H100 80GB | Expensive; use after validating configs. |
| Spatial chunking | Wide rooms, buildings, outdoor paths | 24-48GB per chunk | Split by area, train chunks, then render/merge. |
| Temporal chunking | Long dynamic captures | 24-48GB per segment | Split by frame ranges if motion duration is the main issue. |
| Code-level scaling | Repeated large-scene work | 48-80GB for development | Add chunk loaders, staged rendering, and better memory controls. |

## Cloud GPU Notes

- RunPod: easiest starting point for short experiments.
- Vast.ai: often cheapest, but check host reliability, disk, bandwidth, and CUDA image details.
- Lambda Cloud: cleaner managed environment if available.
- Paperspace/DigitalOcean: usable, but verify current GPU availability before committing.

## Adjustment Checklist

1. Reduce image resolution first with `--resolution`.
2. Tune temporal grid size in `kplanes_config['resolution']`; the last value is temporal resolution.
3. Limit Gaussian growth with `densify_until_iter`, `densification_interval`, `densify_grad_threshold_*`, `opacity_threshold_*`, and `percent_dense`.
4. Keep `batch_size=1` unless memory headroom is confirmed.
5. Shorten early experiments with fewer `iterations` and `coarse_iterations`.
6. Split the scene spatially when VRAM grows with scene area.
7. Split the scene temporally when VRAM or quality issues grow with frame count.
8. Move from 24GB to 48GB before rewriting code; move to 80GB only after the pipeline is proven.

## First Experiment

Train one small chunk end-to-end on a 24GB RTX 4090. Once the dataset format, COLMAP outputs, and rendering commands are confirmed, scale the same config to a larger chunk or a 48GB GPU.
