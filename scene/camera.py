import numpy as np
import os,sys,time
import torch
import torch.nn.functional as torch_F
import collections
from easydict import EasyDict as edict

import util
from util import log,debug

class Pose():
    """
    A class of operations on camera poses (PyTorch tensors with shape [...,3,4])
    each [3,4] camera pose takes the form of [R|t]
    """

    def __call__(self,R=None,t=None):
        # construct a camera pose from the given R and/or t
        assert(R is not None or t is not None)
        if R is None:
            if not isinstance(t,torch.Tensor): t = torch.tensor(t)
            R = torch.eye(3,device=t.device).repeat(*t.shape[:-1],1,1)
        elif t is None:
            if not isinstance(R,torch.Tensor): R = torch.tensor(R)
            t = torch.zeros(R.shape[:-1],device=R.device)
        else:
            if not isinstance(R,torch.Tensor): R = torch.tensor(R)
            if not isinstance(t,torch.Tensor): t = torch.tensor(t)
        assert(R.shape[:-1]==t.shape and R.shape[-2:]==(3,3))
        R = R.float()
        t = t.float()
        pose = torch.cat([R,t[...,None]],dim=-1) # [...,3,4]
        assert(pose.shape[-2:]==(3,4))
        return pose

    def invert(self,pose,use_inverse=False):
        # invert a camera pose
        R,t = pose[...,:3],pose[...,3:]
        R_inv = R.inverse() if use_inverse else R.transpose(-1,-2)
        t_inv = (-R_inv@t)[...,0]
        pose_inv = self(R=R_inv,t=t_inv)
        return pose_inv

    def compose(self,pose_list):
        # compose a sequence of poses together
        # pose_new(x) = poseN o ... o pose2 o pose1(x)
        pose_new = pose_list[0]
        for pose in pose_list[1:]:
            pose_new = self.compose_pair(pose_new,pose)
        return pose_new

    def compose_pair(self,pose_a,pose_b):
        # pose_new(x) = pose_b o pose_a(x)
        R_a,t_a = pose_a[...,:3],pose_a[...,3:]
        R_b,t_b = pose_b[...,:3],pose_b[...,3:]
        R_new = R_b@R_a
        t_new = (R_b@t_a+t_b)[...,0]
        pose_new = self(R=R_new,t=t_new)
        return pose_new

class Lie():
    """
    Lie algebra for SO(3) and SE(3) operations in PyTorch
    """

    def so3_to_SO3(self,w): # [...,3]
        wx = self.skew_symmetric(w)
        theta = w.norm(dim=-1)[...,None,None]
        I = torch.eye(3,device=w.device,dtype=torch.float32)
        A = self.taylor_A(theta)
        B = self.taylor_B(theta)
        R = I+A*wx+B*wx@wx
        return R

    def SO3_to_so3(self,R,eps=1e-7): # [...,3,3]
        trace = R[...,0,0]+R[...,1,1]+R[...,2,2]
        theta = ((trace-1)/2).clamp(-1+eps,1-eps).acos_()[...,None,None]%np.pi # ln(R) will explode if theta==pi
        lnR = 1/(2*self.taylor_A(theta)+1e-8)*(R-R.transpose(-2,-1)) # FIXME: wei-chiu finds it weird
        w0,w1,w2 = lnR[...,2,1],lnR[...,0,2],lnR[...,1,0]
        w = torch.stack([w0,w1,w2],dim=-1)
        return w

    def se3_to_SE3(self,wu): # [...,3]
        w,u = wu.split([3,3],dim=-1)
        wx = self.skew_symmetric(w)
        theta = w.norm(dim=-1)[...,None,None]
        I = torch.eye(3,device=w.device,dtype=torch.float32)
        A = self.taylor_A(theta)
        B = self.taylor_B(theta)
        C = self.taylor_C(theta)
        R = I+A*wx+B*wx@wx
        V = I+B*wx+C*wx@wx
        Rt = torch.cat([R,(V@u[...,None])],dim=-1)
        return Rt

    def SE3_to_se3(self,Rt,eps=1e-8): # [...,3,4]
        R,t = Rt.split([3,1],dim=-1)
        w = self.SO3_to_so3(R)
        wx = self.skew_symmetric(w)
        theta = w.norm(dim=-1)[...,None,None]
        I = torch.eye(3,device=w.device,dtype=torch.float32)
        A = self.taylor_A(theta)
        B = self.taylor_B(theta)
        invV = I-0.5*wx+(1-A/(2*B))/(theta**2+eps)*wx@wx
        u = (invV@t)[...,0]
        wu = torch.cat([w,u],dim=-1)
        return wu    

    def skew_symmetric(self,w):
        w0,w1,w2 = w.unbind(dim=-1)
        O = torch.zeros_like(w0)
        wx = torch.stack([torch.stack([O,-w2,w1],dim=-1),
                          torch.stack([w2,O,-w0],dim=-1),
                          torch.stack([-w1,w0,O],dim=-1)],dim=-2)
        return wx

    def taylor_A(self,x,nth=10):
        # Taylor expansion of sin(x)/x
        ans = torch.zeros_like(x)
        denom = 1.
        for i in range(nth+1):
            if i>0: denom *= (2*i)*(2*i+1)
            ans = ans+(-1)**i*x**(2*i)/denom
        return ans
    def taylor_B(self,x,nth=10):
        # Taylor expansion of (1-cos(x))/x**2
        ans = torch.zeros_like(x)
        denom = 1.
        for i in range(nth+1):
            denom *= (2*i+1)*(2*i+2)
            ans = ans+(-1)**i*x**(2*i)/denom
        return ans
    def taylor_C(self,x,nth=10):
        # Taylor expansion of (x-sin(x))/x**3
        ans = torch.zeros_like(x)
        denom = 1.
        for i in range(nth+1):
            denom *= (2*i+2)*(2*i+3)
            ans = ans+(-1)**i*x**(2*i)/denom
        return ans

class Quaternion():

    def q_to_R(self,q):
        # https://en.wikipedia.org/wiki/Rotation_matrix#Quaternion
        qa,qb,qc,qd = q.unbind(dim=-1)
        R = torch.stack([torch.stack([1-2*(qc**2+qd**2),2*(qb*qc-qa*qd),2*(qa*qc+qb*qd)],dim=-1),
                         torch.stack([2*(qb*qc+qa*qd),1-2*(qb**2+qd**2),2*(qc*qd-qa*qb)],dim=-1),
                         torch.stack([2*(qb*qd-qa*qc),2*(qa*qb+qc*qd),1-2*(qb**2+qc**2)],dim=-1)],dim=-2)
        return R

    def R_to_q(self,R,eps=1e-8): # [B,3,3]
        # https://en.wikipedia.org/wiki/Rotation_matrix#Quaternion
        # FIXME: this function seems a bit problematic, need to double-check
        row0,row1,row2 = R.unbind(dim=-2)
        R00,R01,R02 = row0.unbind(dim=-1)
        R10,R11,R12 = row1.unbind(dim=-1)
        R20,R21,R22 = row2.unbind(dim=-1)
        t = R[...,0,0]+R[...,1,1]+R[...,2,2]
        r = (1+t+eps).sqrt()
        qa = 0.5*r
        qb = (R21-R12).sign()*0.5*(1+R00-R11-R22+eps).sqrt()
        qc = (R02-R20).sign()*0.5*(1-R00+R11-R22+eps).sqrt()
        qd = (R10-R01).sign()*0.5*(1-R00-R11+R22+eps).sqrt()
        q = torch.stack([qa,qb,qc,qd],dim=-1)
        for i,qi in enumerate(q):
            if torch.isnan(qi).any():
                K = torch.stack([torch.stack([R00-R11-R22,R10+R01,R20+R02,R12-R21],dim=-1),
                                 torch.stack([R10+R01,R11-R00-R22,R21+R12,R20-R02],dim=-1),
                                 torch.stack([R20+R02,R21+R12,R22-R00-R11,R01-R10],dim=-1),
                                 torch.stack([R12-R21,R20-R02,R01-R10,R00+R11+R22],dim=-1)],dim=-2)/3.0
                K = K[i]
                eigval,eigvec = torch.linalg.eigh(K)
                V = eigvec[:,eigval.argmax()]
                q[i] = torch.stack([V[3],V[0],V[1],V[2]])
        return q

    def invert(self,q):
        qa,qb,qc,qd = q.unbind(dim=-1)
        norm = q.norm(dim=-1,keepdim=True)
        q_inv = torch.stack([qa,-qb,-qc,-qd],dim=-1)/norm**2
        return q_inv

    def product(self,q1,q2): # [B,4]
        q1a,q1b,q1c,q1d = q1.unbind(dim=-1)
        q2a,q2b,q2c,q2d = q2.unbind(dim=-1)
        hamil_prod = torch.stack([q1a*q2a-q1b*q2b-q1c*q2c-q1d*q2d,
                                  q1a*q2b+q1b*q2a+q1c*q2d-q1d*q2c,
                                  q1a*q2c-q1b*q2d+q1c*q2a+q1d*q2b,
                                  q1a*q2d+q1b*q2c-q1c*q2b+q1d*q2a],dim=-1)
        return hamil_prod

pose = Pose()
lie = Lie()
quaternion = Quaternion()

def to_hom(X):
    # get homogeneous coordinates of the input
    X_hom = torch.cat([X,torch.ones_like(X[...,:1])],dim=-1)
    return X_hom

# basic operations of transforming 3D points between world/camera/image coordinates
def world2cam(X,pose): # [B,N,3]
    X_hom = to_hom(X)
    return X_hom@pose.transpose(-1,-2)
def cam2img(X,cam_intr):
    return X@cam_intr.transpose(-1,-2)
def img2cam(X,cam_intr):
    return X@cam_intr.inverse().transpose(-1,-2)
def cam2world(X,pose):
    X_hom = to_hom(X)
    pose_inv = Pose().invert(pose)
    return X_hom@pose_inv.transpose(-1,-2)

def angle_to_rotation_matrix(a,axis):
    # get the rotation matrix from Euler angle around specific axis
    roll = dict(X=1,Y=2,Z=0)[axis]
    O = torch.zeros_like(a)
    I = torch.ones_like(a)
    M = torch.stack([torch.stack([a.cos(),-a.sin(),O],dim=-1),
                     torch.stack([a.sin(),a.cos(),O],dim=-1),
                     torch.stack([O,O,I],dim=-1)],dim=-2)
    M = M.roll((roll,roll),dims=(-2,-1))
    return M

def get_center_and_ray(opt,pose,intr=None): # [HW,2]
    # given the intrinsic/extrinsic matrices, get the camera center and ray directions]
    assert(opt.camera.model=="perspective")
    with torch.no_grad():
        # compute image coordinate grid
        y_range = torch.arange(opt.H,dtype=torch.float32,device=opt.device).add_(0.5)
        x_range = torch.arange(opt.W,dtype=torch.float32,device=opt.device).add_(0.5)
        Y,X = torch.meshgrid(y_range,x_range) # [H,W]
        xy_grid = torch.stack([X,Y],dim=-1).view(-1,2) # [HW,2]
    # compute center and ray
    batch_size = len(pose)
    xy_grid = xy_grid.repeat(batch_size,1,1) # [B,HW,2]
    grid_3D = img2cam(to_hom(xy_grid),intr) # [B,HW,3]
    center_3D = torch.zeros_like(grid_3D) # [B,HW,3]
    # transform from camera to world coordinates
    grid_3D = cam2world(grid_3D,pose) # [B,HW,3]
    center_3D = cam2world(center_3D,pose) # [B,HW,3]
    ray = grid_3D-center_3D # [B,HW,3]
    return center_3D,ray

def get_3D_points_from_depth(opt,center,ray,depth,multi_samples=False):
    if multi_samples: center,ray = center[:,:,None],ray[:,:,None]
    # x = c+dv
    points_3D = center+ray*depth # [B,HW,3]/[B,HW,N,3]/[N,3]
    return points_3D

def convert_NDC(opt,center,ray,intr,near=1):
    # shift camera center (ray origins) to near plane (z=1)
    # (unlike conventional NDC, we assume the cameras are facing towards the +z direction)
    center = center+(near-center[...,2:])/ray[...,2:]*ray
    # projection
    cx,cy,cz = center.unbind(dim=-1) # [B,HW]
    rx,ry,rz = ray.unbind(dim=-1) # [B,HW]
    scale_x = intr[:,0,0]/intr[:,0,2] # [B]
    scale_y = intr[:,1,1]/intr[:,1,2] # [B]
    cnx = scale_x[:,None]*(cx/cz)
    cny = scale_y[:,None]*(cy/cz)
    cnz = 1-2*near/cz
    rnx = scale_x[:,None]*(rx/rz-cx/cz)
    rny = scale_y[:,None]*(ry/rz-cy/cz)
    rnz = 2*near/cz
    center_ndc = torch.stack([cnx,cny,cnz],dim=-1) # [B,HW,3]
    ray_ndc = torch.stack([rnx,rny,rnz],dim=-1) # [B,HW,3]
    return center_ndc,ray_ndc

def rotation_distance(R1,R2,eps=1e-7):
    # http://www.boris-belousov.net/2016/12/01/quat-dist/
    R_diff = R1@R2.transpose(-2,-1)
    trace = R_diff[...,0,0]+R_diff[...,1,1]+R_diff[...,2,2]
    angle = ((trace-1)/2).clamp(-1+eps,1-eps).acos_() # numerical stability near -1/+1
    return angle

def procrustes_analysis(X0,X1): # [N,3]
    # translation
    t0 = X0.mean(dim=0,keepdim=True)
    t1 = X1.mean(dim=0,keepdim=True)
    X0c = X0-t0
    X1c = X1-t1
    # scale
    s0 = (X0c**2).sum(dim=-1).mean().sqrt()
    s1 = (X1c**2).sum(dim=-1).mean().sqrt()
    X0cs = X0c/s0
    X1cs = X1c/s1
    # rotation (use double for SVD, float loses precision)
    U,S,V = (X0cs.t()@X1cs).double().svd(some=True)
    R = (U@V.t()).float()
    if R.det()<0: R[2] *= -1
    # align X1 to X0: X1to0 = (X1-t1)/s1@R.t()*s0+t0
    sim3 = edict(t0=t0[0],t1=t1[0],s0=s0,s1=s1,R=R)
    return sim3

def get_novel_view_poses(opt,pose_anchor,N=60,scale=1):
    # create circular viewpoints (small oscillations)
    theta = torch.arange(N)/N*2*np.pi
    R_x = angle_to_rotation_matrix((theta.sin()*0.05).asin(),"X")
    R_y = angle_to_rotation_matrix((theta.cos()*0.05).asin(),"Y")
    pose_rot = pose(R=R_y@R_x)
    pose_shift = pose(t=[0,0,-4*scale])
    pose_shift2 = pose(t=[0,0,3.8*scale])
    pose_oscil = pose.compose([pose_shift,pose_rot,pose_shift2])
    pose_novel = pose.compose([pose_oscil,pose_anchor.cpu()[None]])
    return pose_novel
