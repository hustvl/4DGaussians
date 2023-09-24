import functools
import math
import os
import time
import tinycudann as tcnn
from tkinter import W

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.cpp_extension import load
import torch.nn.init as init
class TriPlaneGrid(nn.Module):
    def __init__(self,
                 desired_resolution=256,
                 base_solution=32,
                 n_levels=4,
                 ):
        super(TriPlaneGrid, self).__init__()

        per_level_scale = np.exp2(np.log2(desired_resolution / base_solution) / (int(n_levels) - 1))
        encoding_2d_config = {
            "otype": "Grid",
            "type": "Dense",
            "n_levels": n_levels,
            "n_features_per_level": 4,
            "base_resolution": base_solution,
            "per_level_scale":per_level_scale,
        }
        self.xy = tcnn.Encoding(n_input_dims=2, encoding_config=encoding_2d_config,dtype=torch.float32)
        self.yz = tcnn.Encoding(n_input_dims=2, encoding_config=encoding_2d_config,dtype=torch.float32)
        self.xz = tcnn.Encoding(n_input_dims=2, encoding_config=encoding_2d_config,dtype=torch.float32)
        self.feat_dim = n_levels * 4 *3

    def forward(self, x,bound):
        x = (x + bound) / (2 * bound)  # zyq: map to [0, 1]
        xy_feat = self.xy(x[:, [0, 1]])
        yz_feat = self.yz(x[:, [0, 2]])
        xz_feat = self.xz(x[:, [1, 2]])
        return torch.cat([xy_feat, yz_feat, xz_feat], dim=-1)   
class TriPlanetimeGrid(nn.Module):
    def __init__(self,
                 desired_resolution=256,
                 base_solution=16,
                 n_levels=6,
                 ):
        super(TriPlanetimeGrid, self).__init__()

        per_level_scale = np.exp2(np.log2(desired_resolution / base_solution) / (int(n_levels) - 1))
        encoding_2d_config = {
            "otype": "Grid",
            "type": "Dense",
            "n_levels": n_levels,
            "n_features_per_level": 4,
            "base_resolution": base_solution,
            "per_level_scale":per_level_scale,
        }
        self.xt = tcnn.Encoding(n_input_dims=2, encoding_config=encoding_2d_config,dtype=torch.float32)
        self.yt = tcnn.Encoding(n_input_dims=2, encoding_config=encoding_2d_config,dtype=torch.float32)
        self.zt = tcnn.Encoding(n_input_dims=2, encoding_config=encoding_2d_config,dtype=torch.float32)
        self.feat_dim = n_levels * 4 *3

    def forward(self, x, time, bound):
        x = (x + bound) / (2 * bound)  # zyq: map to [0, 1]
        xt = torch.cat([x[:,0:1],time],-1)
        yt = torch.cat([x[:,1:2],time],-1)
        zt = torch.cat([x[:,2:3],time],-1)
        xt_feat = self.xt(xt)
        yt_feat = self.yt(yt)
        xt_feat = self.xt(zt)
        return torch.cat([xt_feat, yt_feat, xt_feat], dim=-1)   
class Deformation(nn.Module):
    def __init__(self, D=8, W=256, input_ch=27, input_ch_time=9, skips=[],):
        super(Deformation, self).__init__()
        self.D = D
        self.W = W
        self.input_ch = input_ch
        self.input_ch_time = input_ch_time
        self.skips = skips
        self.grid = TriPlaneGrid()
        self.timegrid = TriPlanetimeGrid()
        self._time, self.pos_deform, self.scales_deform, self.rotations_deform, self.opacity_deform = self.create_net()
        
    def create_net(self):
        layers = [nn.Linear(self.input_ch + self.input_ch_time, self.W)]
        for i in range(self.D):
            layer = nn.Linear
            in_channels = self.W
            layers += [layer(in_channels, self.W)]
        self.mlp_out = nn.Linear(self.W,self.W)
        # self.grid_out = nn.Linear(self.grid.feat_dim+self.timegrid.feat_dim,self.W//2)
        self.feature_out = nn.Linear(self.W+self.grid.feat_dim + self.timegrid.feat_dim,self.W)
        output_dim = self.W
        return nn.ModuleList(layers), nn.Sequential(nn.Linear(output_dim,self.W),nn.ReLU(),nn.Linear(self.W, 3)), nn.Sequential(nn.Linear(output_dim,self.W),nn.ReLU(),nn.Linear(self.W, 3)),\
            nn.Sequential(nn.Linear(output_dim,self.W),nn.ReLU(), nn.Linear(self.W, 4)), nn.Sequential(nn.Linear(output_dim,self.W),nn.ReLU(),nn.Linear(self.W, 1))

    def query_time(self, rays_pts_emb, scales_emb, rotations_emb, t, net, time_emb):
        h = torch.cat([rays_pts_emb, scales_emb, rotations_emb, t], dim=-1)
        for i, l in enumerate(net):
            h = net[i](h)
            h = F.relu(h)
        mlp_feature = self.mlp_out(h)
        # mlp_feature = F.relu(mlp_feature)
        grid_feature = self.grid(rays_pts_emb[:,:3],bound=2)
        time_feature = self.timegrid(rays_pts_emb[:,:3],time_emb[:,0:1],bound=2)
        
        voxel_feature = torch.cat([grid_feature,time_feature],-1)
        # voxel_feature = self.grid_out(voxel_feature)
        h = torch.cat([mlp_feature,voxel_feature],-1)
        
        h = self.feature_out(h)
        h = F.relu(h)
        # h = self.out_layers(h)
        # h = F.sigmoid(h) # map to [0,1]
        # h = self.grid(h)
        return h

    def forward(self, rays_pts_emb, scales_emb, rotations_emb, ts, time_emb):
        hidden = self.query_time(rays_pts_emb, rotations_emb, scales_emb, ts, self._time, time_emb).float()
        dx = self.pos_deform(hidden)
        pts = rays_pts_emb[:, :3] + dx
        ds = self.scales_deform(hidden)
        scales = scales_emb[:,:3] + ds
        dr = self.rotations_deform(hidden)
        rotations = rotations_emb[:,:4] + dr
        # do = self.opacity_deform(hidden)
        # opacity = opacity_emb[:,:1] + do
        # print("deformation value:","pts:",torch.abs(dx).mean(),"rotation:",torch.abs(dr).mean())

        return pts, scales, rotations
    def get_mlp_parameters(self):
        parameter_list = []
        for name, param in self.named_parameters():
            if  "grid" not in name:
                parameter_list.append(param)
        return parameter_list
    def get_grid_parameters(self):
        return list(self.grid.parameters() ) + list(self.timegrid.parameters())
class deform_network(nn.Module):
    def __init__(self) :
        super(deform_network, self).__init__()
        net_width = 256
        timebase_pe = 4
        defor_depth= 1
        posbase_pe= 10
        scale_rotation_pe = 4
        opacity_pe = 2
        timenet_width = 256
        timenet_output = 32
        times_ch = 2*timebase_pe+1
        self.timenet = nn.Sequential(
        nn.Linear(times_ch, timenet_width), nn.ReLU(),
        nn.Linear(timenet_width, timenet_output))
        self.deformation_net = Deformation(W=net_width, D=defor_depth, input_ch=(3+4+3)+(3*posbase_pe+(4+3)*scale_rotation_pe)*2, input_ch_time=timenet_output)
        self.register_buffer('time_poc', torch.FloatTensor([(2**i) for i in range(timebase_pe)]))
        self.register_buffer('pos_poc', torch.FloatTensor([(2**i) for i in range(posbase_pe)]))
        self.register_buffer('rotation_scaling_poc', torch.FloatTensor([(2**i) for i in range(scale_rotation_pe)]))
        self.register_buffer('opacity_poc', torch.FloatTensor([(2**i) for i in range(opacity_pe)]))
        self.apply(initialize_weights)
    def forward(self, point, scales, rotations, opacity, times_sel):
        times_emb = poc_fre(times_sel, self.time_poc)
        times_feature = self.timenet(times_emb)
        pts_emb = poc_fre(point, self.pos_poc)
        scales_emb = poc_fre(scales, self.rotation_scaling_poc)
        rotations_emb = poc_fre(rotations, self.rotation_scaling_poc)
        # opacity_emb = poc_fre(opacity, self.opacity_poc)
        means3D, scales, rotations = self.deformation_net( pts_emb,
                                                  scales_emb,
                                                rotations_emb,
                                                # opacity_emb,
                                                times_feature,
                                                times_emb)
        return means3D, scales, rotations, opacity
    def get_mlp_parameters(self):
        return self.deformation_net.get_mlp_parameters() + list(self.timenet.parameters())
    def get_grid_parameters(self):
        return self.deformation_net.get_grid_parameters()
class dynamic_gate(nn.Module):
    def __init__(self) -> None:
        super(dynamic_gate).__init__()
        net_width = 256
        timebase_pe = 4
        posbase_pe= 10
        scale_rotation_pe = 4
        timenet_width = 256
        timenet_output = 32
        times_ch = 2*timebase_pe+1
        self.timenet = nn.Sequential(
        nn.Linear(times_ch, timenet_width), nn.ReLU(),
        nn.Linear(timenet_width, timenet_output))
        self.deformation_net = nn.Sequential(
            nn.Linear((3+4+3)+(3*posbase_pe+(4+3)*scale_rotation_pe)*2+timenet_output,net_width),
            nn.ReLU(),
            nn.Linear(net_width,1)
        )
        # self.deformation_net = Deformation(W=net_width, D=defor_depth, input_ch=(3+4+3)+(3*posbase_pe+(4+3)*scale_rotation_pe)*2, input_ch_time=timenet_output)
        self.register_buffer('time_poc', torch.FloatTensor([(2**i) for i in range(timebase_pe)]))
        self.register_buffer('pos_poc', torch.FloatTensor([(2**i) for i in range(posbase_pe)]))
        self.register_buffer('rotation_scaling_poc', torch.FloatTensor([(2**i) for i in range(scale_rotation_pe)]))
        self.apply(initialize_weights)
    def forward(self, point, scales, rotations, opacity, times_sel):
        times_emb = poc_fre(times_sel, self.time_poc)
        times_feature = self.timenet(times_emb)
        pts_emb = poc_fre(point, self.pos_poc)
        scales_emb = poc_fre(scales, self.rotation_scaling_poc)
        rotations_emb = poc_fre(rotations, self.rotation_scaling_poc)
        motion_rate = self.deformation_net(torch.cat([pts_emb,
                                                  scales_emb,
                                                rotations_emb,
                                                times_feature],-1))
        return motion_rate
class Tineuvox(nn.Module):
    def __init__(self) -> None:
        super(Tineuvox).__init__()
        pass
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