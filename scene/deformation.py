
from tkinter import W

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.nn.init as init
from scene.AggregationPlane import AggregationPlane

class Deformation(nn.Module):
    def __init__(self, D=8, W=256, skips=[], args=None):
        super(Deformation, self).__init__()
        self.D = D
        self.W = W

        self.skips = skips

        self.grid = AggregationPlane(args.bounds, args.kplanes_config, args.multires)
        self.pos_deform, self.scales_deform, self.rotations_deform, self.opacity_deform = self.create_net()
    def create_net(self):

        mlp_out_dim = 0

        self.feature_out = [nn.Linear(mlp_out_dim + self.grid.feat_dim ,self.W)]
        for i in range(self.D-1):
            self.feature_out.append(nn.ReLU())
            self.feature_out.append(nn.Linear(self.W,self.W))
        self.feature_out = nn.Sequential(*self.feature_out)
        return  \
            nn.Sequential(nn.ReLU(),nn.Linear(self.W,self.W),nn.ReLU(),nn.Linear(self.W, 3)),\
            nn.Sequential(nn.ReLU(),nn.Linear(self.W,self.W),nn.ReLU(),nn.Linear(self.W, 3)),\
            nn.Sequential(nn.ReLU(),nn.Linear(self.W,self.W),nn.ReLU(),nn.Linear(self.W, 4)), \
            nn.Sequential(nn.ReLU(),nn.Linear(self.W,self.W),nn.ReLU(),nn.Linear(self.W, 1))
    
    def query_time(self, rays_pts_emb, time_emb):

        grid_feature = self.grid(rays_pts_emb[:,:3], time_emb[:,:1])
        voxel_feature = torch.cat([grid_feature],-1)
        h = torch.cat([voxel_feature],-1)
        h = self.feature_out(h)
        return h

    def forward(self, rays_pts, scales=None, rotations=None, opacity = None, time_emb=None):

        return self.forward_dynamic(rays_pts, scales, rotations, opacity, time_emb)


    def forward_dynamic(self,rays_pts_emb, scales_emb, rotations_emb, opacity_emb, time_emb):
        hidden = self.query_time(rays_pts_emb, scales_emb, rotations_emb, time_emb).float()
        dx = self.pos_deform(hidden)
        pts = rays_pts_emb[:, :3] + dx
        ds = self.scales_deform(hidden)
        scales = scales_emb[:,:3] + ds
        dr = self.rotations_deform(hidden)
        rotations = rotations_emb[:,:4] + dr
        opacity = opacity_emb[:,:1] 


        return pts, scales, rotations, opacity
    def get_mlp_parameters(self):
        parameter_list = []
        for name, param in self.named_parameters():
            if  "grid" not in name:
                parameter_list.append(param)
        return parameter_list
    def get_grid_parameters(self):
        return list(self.grid.parameters() ) 
class deform_network(nn.Module):
    def __init__(self, args) :
        super(deform_network, self).__init__()
        net_width = args.net_width
        defor_depth= args.defor_depth

        self.deformation_net = Deformation(W=net_width, D=defor_depth,  args=args)
        self.apply(initialize_weights)
        print(self)

    def forward(self, point, scales=None, rotations=None, opacity=None, times_sel=None):
        if times_sel is not None:
            return self.forward_dynamic(point, scales, rotations, opacity, times_sel)   

    def forward_dynamic(self, point, scales=None, rotations=None, opacity=None, times_sel=None):
        means3D, scales, rotations, opacity = self.deformation_net(point, scales, rotations, times_sel)
        return means3D, scales, rotations, opacity
    
    def get_mlp_parameters(self):
        return self.deformation_net.get_mlp_parameters() + list(self.timenet.parameters())
    
    def get_grid_parameters(self):
        return self.deformation_net.get_grid_parameters()

def poc_fre(input_data,poc_buf):

    input_data_emb = (input_data.unsqueeze(-1) * poc_buf).flatten(-2)
    input_data_sin = input_data_emb.sin()
    input_data_cos = input_data_emb.cos()
    input_data_emb = torch.cat([input_data, input_data_sin,input_data_cos], -1)
    return input_data_emb
def initialize_weights(m):
    if isinstance(m, nn.Linear):
        # init.constant_(m.weight, 0)
        init.xavier_uniform_(m.weight,gain=1)
        if m.bias is not None:
            init.xavier_uniform_(m.weight,gain=1)
            # init.constant_(m.bias, 0)