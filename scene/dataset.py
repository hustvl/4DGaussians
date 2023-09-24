from torch.utils.data import Dataset
from scene.cameras import Camera
import numpy as np
from utils.general_utils import PILtoTorch
from utils.graphics_utils import fov2focal, focal2fov
import torch
from utils.camera_utils import loadCam
from utils.graphics_utils import focal2fov
class FourDGSdataset(Dataset):
    def __init__(
        self,
        dataset,
        args
    ):
        self.dataset = dataset
        self.args = args
    def __getitem__(self, index):
        # cam_list = []
        # N_cams = self.dataset.cam_number
        # select_view = torch.randperm(N_cams)[:8]
        # # for cam_idx in select_view:
        #     image, w2c, time = self.dataset[index+cam_idx*len(self)]
        #     R,T = w2c
        #     FovX = focal2fov(self.dataset.focal[0], image.shape[2])
        #     FovY = focal2fov(self.dataset.focal[0], image.shape[1])
        #     cam = Camera(colmap_id=index,R=R,T=T,FoVx=FovX,FoVy=FovY,image=image,gt_alpha_mask=None,
                        #   image_name=f"{index}",uid=index,data_device=torch.device("cuda"),time=time)
            # cam_list.append(cam)
        # return cam_list
        image, w2c, time = self.dataset[index]
        R,T = w2c
        FovX = focal2fov(self.dataset.focal[0], image.shape[2])
        FovY = focal2fov(self.dataset.focal[0], image.shape[1])
        return Camera(colmap_id=index,R=R,T=T,FoVx=FovX,FoVy=FovY,image=image,gt_alpha_mask=None,
                          image_name=f"{index}",uid=index,data_device=torch.device("cuda"),time=time)
    def __len__(self):
        return len(self.dataset)
        # return self.dataset.time_number
