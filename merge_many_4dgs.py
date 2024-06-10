import imageio
import numpy as np
import torch
from scene import Scene
import os
import cv2
from tqdm import tqdm
from os import makedirs
from gaussian_renderer import render
import torchvision
from utils.general_utils import safe_state
from argparse import ArgumentParser
from arguments import ModelParams, PipelineParams, get_combined_args, ModelHiddenParams
from gaussian_renderer import GaussianModel
from time import time
import open3d as o3d
# import torch.multiprocessing as mp
import threading
import concurrent.futures
from copy import deepcopy
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
import torch
import math
from diff_gaussian_rasterization import GaussianRasterizationSettings, GaussianRasterizer
from scene.gaussian_model import GaussianModel
from utils.render_utils import get_state_at_time
from tqdm import tqdm
def rotate_point_cloud(point_cloud, displacement, rotation_angles, scales_bias):
    
    theta, phi = rotation_angles

    rotation_matrix_z = torch.tensor([
        [torch.cos(theta), -torch.sin(theta), 0],
        [torch.sin(theta),  torch.cos(theta), 0],
        [0,                0,               1]
    ]).to(point_cloud)
    rotation_matrix_x = torch.tensor([
        [1, 0,                0],
        [0, torch.cos(phi), -torch.sin(phi)],
        [0, torch.sin(phi),  torch.cos(phi)]
    ]).to(point_cloud)
    rotation_matrix = torch.matmul(rotation_matrix_z, rotation_matrix_x)
    # print(rotation_matrix)
    point_cloud = point_cloud*scales_bias
    rotated_point_cloud = torch.matmul(point_cloud, rotation_matrix.t())
    displaced_point_cloud = rotated_point_cloud + displacement

    return displaced_point_cloud
@torch.no_grad()
def render(viewpoint_camera, gaussians, bg_color : torch.Tensor, scaling_modifier = 1.0, motion_bias = [torch.tensor([0,0,0])], rotation_bias = [torch.tensor([0,0])],
           scales_bias=[1,1]):
    """
    Render the scene. 
    
    Background tensor (bg_color) must be on GPU!
    """
 
    # Create zero tensor. We will use it to make pytorch return gradients of the 2D (screen-space) means


    # Set up rasterization configuration
    
    tanfovx = math.tan(viewpoint_camera.FoVx * 0.5)
    tanfovy = math.tan(viewpoint_camera.FoVy * 0.5)
    screenspace_points = None
    for pc in gaussians:
        if screenspace_points is None:
            screenspace_points = torch.zeros_like(pc.get_xyz, dtype=pc.get_xyz.dtype, requires_grad=True, device="cuda") + 0
        else:
            screenspace_points1 = torch.zeros_like(pc.get_xyz, dtype=pc.get_xyz.dtype, requires_grad=True, device="cuda") + 0
            screenspace_points = torch.cat([screenspace_points,screenspace_points1],dim=0)
    try:
        screenspace_points.retain_grad()
    except:
        pass
    raster_settings = GaussianRasterizationSettings(
        image_height=int(viewpoint_camera.image_height),
        image_width=int(viewpoint_camera.image_width),
        tanfovx=tanfovx,
        tanfovy=tanfovy,
        bg=bg_color,
        scale_modifier=scaling_modifier,
        viewmatrix=viewpoint_camera.world_view_transform.cuda(),
        projmatrix=viewpoint_camera.full_proj_transform.cuda(),
        sh_degree=gaussians[0].active_sh_degree,
        campos=viewpoint_camera.camera_center.cuda(),
        prefiltered=False,
        debug=False
    )

    rasterizer = GaussianRasterizer(raster_settings=raster_settings)
    # means3D = pc.get_xyz
    # add deformation to each points
    # deformation = pc.get_deformation
    means3D_final, scales_final, rotations_final, opacity_final, shs_final = None, None, None, None, None
    for index, pc in enumerate(gaussians):
        
        means3D_final1, scales_final1, rotations_final1, opacity_final1, shs_final1 = get_state_at_time(pc, viewpoint_camera)
        scales_final1 = pc.scaling_activation(scales_final1)
        rotations_final1 = pc.rotation_activation(rotations_final1)
        opacity_final1 = pc.opacity_activation(opacity_final1)
        if index == 0:
            means3D_final, scales_final, rotations_final, opacity_final, shs_final = means3D_final1, scales_final1, rotations_final1, opacity_final1, shs_final1
        else:
            motion_bias_t = motion_bias[index-1].to(means3D_final)
            rotation_bias_t = rotation_bias[index-1].to(means3D_final)
            means3D_final1 = rotate_point_cloud(means3D_final1,motion_bias_t,rotation_bias_t,scales_bias[index-1])
            # breakpoint()
            scales_final1 = scales_final1*scales_bias[index-1]
            means3D_final = torch.cat([means3D_final,means3D_final1],dim=0)
            scales_final = torch.cat([scales_final,scales_final1],dim=0)
            rotations_final = torch.cat([rotations_final,rotations_final1],dim=0)
            opacity_final = torch.cat([opacity_final,opacity_final1],dim=0)
            shs_final = torch.cat([shs_final,shs_final1],dim=0)

    colors_precomp = None
    cov3D_precomp = None
    rendered_image, radii, depth = rasterizer(
        means3D = means3D_final,
        means2D = screenspace_points,
        shs = shs_final,
        colors_precomp = colors_precomp,
        opacities = opacity_final,
        scales = scales_final,
        rotations = rotations_final,
        cov3D_precomp = cov3D_precomp)

    return {"render": rendered_image,
            "viewspace_points": screenspace_points,
            "visibility_filter" : radii > 0,
            "radii": radii,
            "depth":depth}


def init_gaussians(dataset : ModelParams, hyperparam, iteration : int, pipeline : PipelineParams, skip_train : bool, skip_test : bool, skip_video: bool):
    with torch.no_grad():
        gaussians = GaussianModel(dataset.sh_degree, hyperparam)
        scene = Scene(dataset, gaussians, load_iteration=iteration, shuffle=False)

        bg_color = [1,1,1] if dataset.white_background else [0, 0, 0]
        background = torch.tensor(bg_color, dtype=torch.float32, device="cuda")

        print("hello!!")
    return gaussians, scene, background

def save_point_cloud(points, model_path, timestamp):
    output_path = os.path.join(model_path,"point_pertimestamp")
    if not os.path.exists(output_path):
        os.makedirs(output_path,exist_ok=True)
    points = points.detach().cpu().numpy()
    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(points)
    ply_path = os.path.join(output_path,f"points_{timestamp}.ply")
    o3d.io.write_point_cloud(ply_path, pcd)
parser = ArgumentParser(description="Testing script parameters")
model = ModelParams(parser, sentinel=True)
pipeline = PipelineParams(parser)
hyperparam = ModelHiddenParams(parser)
parser.add_argument("--iteration", default=-1, type=int)
parser.add_argument("--skip_train", action="store_true")
parser.add_argument("--skip_test", action="store_true")
parser.add_argument("--quiet", action="store_true")
parser.add_argument("--skip_video", action="store_true")
parser.add_argument("--configs1", type=str, default="arguments/dynerf_9/flame_salmon_1.py")
parser.add_argument("--configs2", type=str, default="arguments/dnerf_tv_2/hellwarrior.py")
parser.add_argument("--modelpath2", type=str, default="output/dnerf_tv_2/hellwarrior")
parser.add_argument("--configs3", type=str, default="arguments/dnerf_tv_2/mutant.py")
parser.add_argument("--modelpath3", type=str, default="output/dnerf_tv_2/mutant")
render_path = "output/editing_render_flame_salmon"

args = get_combined_args(parser)
print("Rendering " , args.model_path)
args2 = deepcopy(args)
args3 = deepcopy(args)

if args.configs1:
    import mmcv
    from utils.params_utils import merge_hparams
    config = mmcv.Config.fromfile(args.configs1)
    args1 = merge_hparams(args, config)
# breakpoint()
if args2.configs2:
    import mmcv
    from utils.params_utils import merge_hparams
    config = mmcv.Config.fromfile(args2.configs2)
    args2 = merge_hparams(args2, config)
    args2.model_path = args2.modelpath2
if args3.configs3:
    import mmcv
    from utils.params_utils import merge_hparams
    config = mmcv.Config.fromfile(args3.configs3)
    args3 = merge_hparams(args3, config)
    args3.model_path = args3.modelpath3
safe_state(args.quiet)
gaussians1, scene1, background = init_gaussians(model.extract(args1), hyperparam.extract(args1), args1.iteration, pipeline.extract(args1), args1.skip_train, args1.skip_test, args1.skip_video)
gaussians2, scene2, background = init_gaussians(model.extract(args2), hyperparam.extract(args2), args2.iteration, pipeline.extract(args2), args2.skip_train, args2.skip_test, args2.skip_video)
gaussians3, scene3, background = init_gaussians(model.extract(args3), hyperparam.extract(args3), args3.iteration, pipeline.extract(args3), args3.skip_train, args3.skip_test, args3.skip_video)
gaussians = [gaussians1,gaussians2,gaussians3]
# breakpoint()
to8b = lambda x : (255*np.clip(x.cpu().numpy(),0,1)).astype(np.uint8)

render_images=[]
if not os.path.exists(render_path):
    os.makedirs(render_path,exist_ok=True)
for index, viewpoint in tqdm(enumerate(scene1.getVideoCameras())):
    result = render(viewpoint, gaussians, 
                    bg_color=background,
                    motion_bias=[
                        torch.tensor([4,4,12]),
                        torch.tensor([-2,4,12])
                        ]
                    ,rotation_bias=[
                        torch.tensor([0,1.9*np.pi/4]),
                        torch.tensor([0,1.9*np.pi/4])
                           ],
                    scales_bias = [1,1])
    render_images.append(to8b(result["render"]).transpose(1,2,0))
    
    torchvision.utils.save_image(result["render"],os.path.join(render_path,f"output_image{index}.png"))
    
imageio.mimwrite(os.path.join(render_path, 'video_rgb.mp4'), render_images, fps=30, codec='libx265') 
    # points = get_state_at_time(gaussians, viewpoint)
    # save_point_cloud(points, args.model_path, index)