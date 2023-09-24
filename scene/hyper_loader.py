import warnings

warnings.filterwarnings("ignore")

import json
import os
import random

import numpy as np
import torch
from PIL import Image
import math
from tqdm import tqdm
from scene.utils import Camera
from typing import NamedTuple
# from scene.dataset_readers import CameraInfo
class CameraInfo(NamedTuple):
    uid: int
    R: np.array
    T: np.array
    FovY: np.array
    FovX: np.array
    image: np.array
    image_path: str
    image_name: str
    width: int
    height: int
    time : float
    # flow_f: np.array
    # flow_mask_f: np.array
    # flow_b: np.array
    # flow_mask_b: np.array
    # motion_mask: np.array

class Load_hyper_data():
    def __init__(self, 
                 datadir, 
                 ratio=1.0,
                 use_bg_points=False,
                 add_cam=False):
        from .utils import Camera
        datadir = os.path.expanduser(datadir)
        with open(f'{datadir}/scene.json', 'r') as f:
            scene_json = json.load(f)
        with open(f'{datadir}/metadata.json', 'r') as f:
            meta_json = json.load(f)
        with open(f'{datadir}/dataset.json', 'r') as f:
            dataset_json = json.load(f)

        self.near = scene_json['near']
        self.far = scene_json['far']
        self.coord_scale = scene_json['scale']
        self.scene_center = scene_json['center']

        self.all_img = dataset_json['ids']
        self.val_id = dataset_json['val_ids']

        self.add_cam = False
        if len(self.val_id) == 0:
            self.i_train = np.array([i for i in np.arange(len(self.all_img)) if
                            (i%4 == 0)])
            self.i_test = self.i_train+2
            self.i_test = self.i_test[:-1,]
        else:
            self.add_cam = True
            self.train_id = dataset_json['train_ids']
            self.i_test = []
            self.i_train = []
            for i in range(len(self.all_img)):
                id = self.all_img[i]
                if id in self.val_id:
                    self.i_test.append(i)
                if id in self.train_id:
                    self.i_train.append(i)
        assert self.add_cam == add_cam
        
        print('self.i_train',self.i_train)
        print('self.i_test',self.i_test)
        self.all_cam = [meta_json[i]['camera_id'] for i in self.all_img]
        self.all_time = [meta_json[i]['warp_id'] for i in self.all_img]
        max_time = max(self.all_time)
        self.all_time = [meta_json[i]['warp_id']/max_time for i in self.all_img]
        self.selected_time = set(self.all_time)
        self.ratio = ratio
        self.max_time = max(self.all_time)


        # all poses
        self.all_cam_params = []
        for im in self.all_img:
            camera = Camera.from_json(f'{datadir}/camera/{im}.json')
            camera = camera.scale(ratio)
            camera.position = camera.position - self.scene_center
            camera.position = camera.position * self.coord_scale
            camera.orientation = camera.orientation.T
            # camera.orientation[0:3, 1:3] *= -1  # switch cam coord x,y
            camera.orientation = camera.orientation[[1, 0, 2], :]  # switch world x,y
            # camera.orientation[2, :] *= -1  # invert world z
            
            camera.orientation = - camera.orientation
            camera.orientation[:,0] = -camera.orientation[:,0]
            camera.orientation = camera.orientation.T
            camera.position = -camera.position.dot(camera.orientation)
            self.all_cam_params.append(camera)

        self.all_img = [f'{datadir}/rgb/{int(1/ratio)}x/{i}.png' for i in self.all_img]
        self.h, self.w = self.all_cam_params[0].image_shape

        self.use_bg_points = use_bg_points
        if use_bg_points:
            with open(f'{datadir}/points.npy', 'rb') as f:
                points = np.load(f)
            self.bg_points = (points - self.scene_center) * self.coord_scale
            self.bg_points = torch.tensor(self.bg_points).float()
        print(f'total {len(self.all_img)} images ',
                'use cam =',self.add_cam, 
                'use bg_point=',self.use_bg_points)

    def load_idx(self, idx,not_dic=False):

        all_data = self.load_raw(idx)
        if not_dic == True:
            rays_o = all_data['rays_ori']
            rays_d = all_data['rays_dir']
            viewdirs = all_data['viewdirs']
            rays_color = all_data['rays_color']
            return rays_o, rays_d, viewdirs,rays_color
        return all_data

    def load_raw(self, idx):
        
        camera = self.all_cam_params[idx]
        image = Image.open(self.all_img[idx])
        im_data = np.array(image.convert("RGBA"))
        norm_data = im_data / 255.0
        bg = np.array([1,1,1]) if self.use_bg_points else np.array([0, 0, 0])
        arr = norm_data[:,:,:3] * norm_data[:, :, 3:4] + bg * (1 - norm_data[:, :, 3:4])
        rays_color = Image.fromarray(np.array(arr*255.0, dtype=np.byte), "RGB")
        time = self.all_time[idx]
        import math
        
        # def calculate_corrected_fov(w, h, focal_length, k1, k2, k3, p1, p2, x, y):
        #     r = math.sqrt(x**2 + y**2)
        #     fovx_corrected = 2 * math.atan(w / (2 * (focal_length * (1 + k1 * r**2 + k2 * r**4 + k3 * r**6))) + 2 * p1 * x * y + p2 * (r**2 + 2 * x**2))
        #     fov_y_corrected = 2 * math.atan(h / (2 * (focal_length * (1 + k1 * r**2 + k2 * r**4 + k3 * r**6))) + 2 * p1 * x * y + p2 * (r**2 + 2 * y**2))
        #     return fovx_corrected, fov_y_corrected
        # fovx, fovy = calculate_corrected_fov(rays_color.size[0],
        #                                     rays_color.size[1],
        #                                     camera.focal_length,
        #                                     camera.radial_distortion[0],
        #                                     camera.radial_distortion[1],
        #                                     camera.radial_distortion[2],
        #                                     camera.tangential_distortion[0],
        #                                     camera.tangential_distortion[1],
        #                                     0,0)
        pixels = camera.get_pixel_centers()
        rays_dir_tensor = torch.tensor(camera.pixels_to_rays(pixels)).float().view([-1,3])
        rays_ori_tensor = torch.tensor(camera.position[None, :]).float().expand_as(rays_dir_tensor)
        rays_color_tensor = torch.tensor(np.array(image)).view([-1,3])/255.
        
        # poses = np.eye(4)
        # poses[:3, :3] = camera.orientation
        # poses[:3, 3] = camera.position
        # matrix = np.linalg.inv(np.array(poses))
        # R = -np.transpose(matrix[:3,:3])
        # R[:,0] = -R[:,0]
        # T = -matrix[:3, 3]
        return {'camera': camera,
                'image_path':"/".join(self.all_img[idx].split("/")[:-1]),
                "image_name":self.all_img[idx].split("/")[-1],
                'image': rays_color, 
                'width':int(self.w),
                'height':int(self.h),
                'FovX':2 * math.atan(self.w / (2 * camera.focal_length)),
                'FovY':2 * math.atan(self.h / (2 * camera.focal_length)),
                'R':camera.orientation,
                'T':camera.position,
                'time':time,
                'rays_ori': rays_ori_tensor, 
                'rays_dir': rays_dir_tensor, 
                'viewdirs':rays_dir_tensor / rays_dir_tensor.norm(dim=-1, keepdim=True),
                'rays_color': rays_color_tensor, 
                'near': torch.tensor(self.near).float().view([-1]), 
                'far': torch.tensor(self.far).float().view([-1]),
                }
def format_hyper_data(data_class, split):
    if split == "train":
        data_idx = data_class.i_train
    elif split == "test":
        data_idx = data_class.i_test
    
    cam_infos = []
    for uid, index in tqdm(enumerate(data_idx)):
        frame_info = data_class.load_idx(index)
        image = frame_info['image']
        image_path = frame_info["image_path"]
        image_name = frame_info["image_name"]
        width = frame_info["width"]
        height = frame_info["height"]
        R = frame_info["R"]
        T = frame_info["T"]
        FovY = frame_info["FovY"]
        FovX = frame_info["FovX"]
        time = frame_info["time"]
        cam_info = CameraInfo(uid=uid, R=R, T=T, FovY=FovY, FovX=FovX, image=image,
                              image_path=image_path, image_name=image_name, width=width, height=height, time=time,
                              )
        cam_infos.append(cam_info)
    return cam_infos
        # matrix = np.linalg.inv(np.array(poses))
        # R = -np.transpose(matrix[:3,:3])
        # R[:,0] = -R[:,0]
        # T = -matrix[:3, 3]