#
# Copyright (C) 2023, Inria
# GRAPHDECO research group, https://team.inria.fr/graphdeco
# All rights reserved.
#
# This software is free for non-commercial, research and evaluation use
# under the terms of the LICENSE.md file.
#
# For inquiries contact  george.drettakis@inria.fr
#
# Modified: streaming write instead of accumulating all frames in memory.
# Original render_set held every rendered frame as a CUDA tensor until the
# loop finished, causing OOM on large datasets. This version writes each
# frame to disk immediately and builds the video from saved files.
#
import imageio
import numpy as np
import torch
from scene import Scene
import os
from tqdm import tqdm
from os import makedirs
from gaussian_renderer import render
import torchvision
from utils.general_utils import safe_state
from argparse import ArgumentParser
from arguments import ModelParams, PipelineParams, get_combined_args, ModelHiddenParams
from gaussian_renderer import GaussianModel
from time import time
import glob

to8b = lambda x: (255 * np.clip(x.cpu().numpy(), 0, 1)).astype(np.uint8)


def render_set(model_path, name, iteration, views, gaussians, pipeline, background, cam_type):
    render_path = os.path.join(model_path, name, "ours_{}".format(iteration), "renders")
    gts_path = os.path.join(model_path, name, "ours_{}".format(iteration), "gt")

    makedirs(render_path, exist_ok=True)
    makedirs(gts_path, exist_ok=True)

    print("point nums:", gaussians._xyz.shape[0])
    time1 = None
    count = 0

    for idx, view in enumerate(tqdm(views, desc="Rendering progress")):
        if idx == 0:
            time1 = time()

        rendering = render(view, gaussians, pipeline, background, cam_type=cam_type)["render"]

        # Write render to disk immediately — no accumulation
        torchvision.utils.save_image(
            rendering, os.path.join(render_path, "{:05d}.png".format(idx))
        )

        # Write GT to disk immediately
        if name in ["train", "test"]:
            if cam_type != "PanopticSports":
                gt = view.original_image[0:3, :, :]
            else:
                gt = view["image"].cuda()
            torchvision.utils.save_image(
                gt, os.path.join(gts_path, "{:05d}.png".format(idx))
            )

        count += 1

    if time1 is not None and count > 1:
        print("FPS:", (count - 1) / (time() - time1))

    # Build video from saved renders
    video_path = os.path.join(
        model_path, name, "ours_{}".format(iteration), "video_rgb.mp4"
    )
    render_files = sorted(glob.glob(os.path.join(render_path, "*.png")))
    if render_files:
        frames = [imageio.imread(f) for f in render_files]
        imageio.mimwrite(video_path, frames, fps=30)
        print(f"Video saved: {video_path} ({len(frames)} frames)")


def render_sets(
    dataset: ModelParams,
    hyperparam,
    iteration: int,
    pipeline: PipelineParams,
    skip_train: bool,
    skip_test: bool,
    skip_video: bool,
):
    with torch.no_grad():
        gaussians = GaussianModel(dataset.sh_degree, hyperparam)
        scene = Scene(dataset, gaussians, load_iteration=iteration, shuffle=False)
        cam_type = scene.dataset_type
        bg_color = [1, 1, 1] if dataset.white_background else [0, 0, 0]
        background = torch.tensor(bg_color, dtype=torch.float32, device="cuda")

        if not skip_train:
            render_set(
                dataset.model_path, "train", scene.loaded_iter,
                scene.getTrainCameras(), gaussians, pipeline, background, cam_type,
            )

        if not skip_test:
            render_set(
                dataset.model_path, "test", scene.loaded_iter,
                scene.getTestCameras(), gaussians, pipeline, background, cam_type,
            )

        if not skip_video:
            render_set(
                dataset.model_path, "video", scene.loaded_iter,
                scene.getVideoCameras(), gaussians, pipeline, background, cam_type,
            )


if __name__ == "__main__":
    parser = ArgumentParser(description="Testing script parameters")
    model = ModelParams(parser, sentinel=True)
    pipeline = PipelineParams(parser)
    hyperparam = ModelHiddenParams(parser)
    parser.add_argument("--iteration", default=-1, type=int)
    parser.add_argument("--skip_train", action="store_true")
    parser.add_argument("--skip_test", action="store_true")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--skip_video", action="store_true")
    parser.add_argument("--configs", type=str)
    args = get_combined_args(parser)
    print("Rendering ", args.model_path)
    if args.configs:
        import mmcv
        from utils.params_utils import merge_hparams
        config = mmcv.Config.fromfile(args.configs)
        args = merge_hparams(args, config)
    safe_state(args.quiet)
    render_sets(
        model.extract(args), hyperparam.extract(args), args.iteration,
        pipeline.extract(args), args.skip_train, args.skip_test, args.skip_video,
    )
