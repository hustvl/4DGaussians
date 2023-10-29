import torch
import open3d as o3d

from torch.utils.data import TensorDataset, random_split
from tqdm import tqdm
import open3d as o3d
import numpy as np
from torch_cluster import grid_cluster
def voxel_down_sample_custom(points, voxel_size):
    # 将点云归一化到体素网格
    voxel_grid = torch.floor(points / voxel_size)

    # 找到唯一的体素，并获取它们在原始体素网格中的索引
    unique_voxels, inverse_indices = torch.unique(voxel_grid, dim=0, return_inverse=True)

    # 创建一个新的点云，其中每个点是其对应体素中所有点的平均值
    new_points = torch.zeros_like(unique_voxels)
    new_points_count = torch.zeros(unique_voxels.size(0), dtype=torch.long)
    # for i in tqdm(range(points.size(0))):
    new_points[inverse_indices] = points
        # new_points_count[inverse_indices[i]] += 1
    # new_points /= new_points_count.unsqueeze(-1)

    return new_points, inverse_indices
def downsample_point_cloud(points, ratio):
    # 创建一个TensorDataset
    dataset = TensorDataset(points)

    # 计算下采样后的点的数量
    num_points = len(dataset)
    num_downsampled_points = int(num_points * ratio)

    # 使用random_split进行下采样
    downsampled_dataset, _ = random_split(dataset, [num_downsampled_points, num_points - num_downsampled_points])

    # 获取下采样后的点的index和点云矩阵
    indices = torch.tensor([i for i, _ in enumerate(downsampled_dataset)])
    downsampled_points = torch.stack([x for x, in downsampled_dataset])

    return indices, downsampled_points

def downsample_point_cloud_open3d(points, voxel_size):
    # 创建一个点云对象

    downsampled_pcd, inverse_indices = voxel_down_sample_custom(points, voxel_size)
    downsampled_points = downsampled_pcd
    # 获取下采样后的点云矩阵

    return torch.tensor(downsampled_points)
def downsample_point_cloud_cluster(points, voxel_size):
    # 创建一个点云对象
    cluster = grid_cluster(points, size=torch.tensor([1,1,1]))

    # 获取下采样后的点云矩阵
    # downsampled_points = np.asarray(downsampled_pcd.points)

    return cluster, points
import torch
from sklearn.neighbors import NearestNeighbors

def upsample_point_cloud(points, density_threshold, displacement_scale, iter_pass):
    # 计算每个点的密度
    # breakpoint()
    try:
        nbrs = NearestNeighbors(n_neighbors=2+iter_pass, algorithm='ball_tree').fit(points)
        distances, indices = nbrs.kneighbors(points)
    except:
        print("no point added")
        return points, torch.tensor([]), torch.tensor([]), torch.zeros((points.shape[0]), dtype=torch.bool)  

    # 找出密度低的点
    low_density_points = points[distances[:,1] > density_threshold]
    low_density_index = distances[:,1] > density_threshold
    low_density_index = torch.from_numpy(low_density_index)
    # 复制这些点并添加随机位移
    num_points = low_density_points.shape[0]
    displacements = torch.randn(num_points, 3) * displacement_scale
    new_points = low_density_points + displacements
    # 返回新的点云矩阵
    return points, low_density_points, new_points, low_density_index

    
def visualize_point_cloud(points, low_density_points, new_points):
    # 创建一个点云对象
    pcd = o3d.geometry.PointCloud()

    # 给被选中的点云添加一个小的偏移量
    low_density_points += 0.01

    # 将所有的点合并到一起
    all_points = np.concatenate([points, low_density_points, new_points], axis=0)
    pcd.points = o3d.utility.Vector3dVector(all_points)

    # 创建颜色数组
    colors = np.zeros((all_points.shape[0], 3))
    colors[:points.shape[0]] = [0, 0, 0]  # 黑色表示初始化的点云
    colors[points.shape[0]:points.shape[0]+low_density_points.shape[0]] = [1, 0, 0]  # 红色表示被选中的点云
    colors[points.shape[0]+low_density_points.shape[0]:] = [0, 1, 0]  # 绿色表示增长的点云
    pcd.colors = o3d.utility.Vector3dVector(colors)

    # 显示点云
    o3d.visualization.draw_geometries([pcd])
def combine_pointcloud(points, low_density_points, new_points):
    pcd = o3d.geometry.PointCloud()

    # 给被选中的点云添加一个小的偏移量
    low_density_points += 0.01
    new_points -= 0.01
    # 将所有的点合并到一起
    all_points = np.concatenate([points, low_density_points, new_points], axis=0)
    pcd.points = o3d.utility.Vector3dVector(all_points)

    # 创建颜色数组
    colors = np.zeros((all_points.shape[0], 3))
    colors[:points.shape[0]] = [0, 0, 0]  # 黑色表示初始化的点云
    colors[points.shape[0]:points.shape[0]+low_density_points.shape[0]] = [1, 0, 0]  # 红色表示被选中的点云
    colors[points.shape[0]+low_density_points.shape[0]:] = [0, 1, 0]  # 绿色表示增长的点云
    pcd.colors = o3d.utility.Vector3dVector(colors)
    return pcd
def addpoint(point_cloud,density_threshold,displacement_scale, iter_pass,):
    # density_threshold: 密度的阈值，越大能筛选出越稀疏的点。
    # displacement_scale: 在以displacement_scale的圆心内随机生成点

    points, low_density_points, new_points, low_density_index = upsample_point_cloud(point_cloud,density_threshold,displacement_scale, iter_pass)
    # breakpoint()
    # breakpoint()
    print("low_density_points",low_density_points.shape[0])

    
    return point_cloud, low_density_points, new_points, low_density_index
def find_point_indices(origin_point, goal_point):
    indices = torch.nonzero((origin_point[:, None] == goal_point).all(-1), as_tuple=True)[0]
    return indices
def find_indices_in_A(A, B):
    """
    找出子集矩阵 B 中每个点在点云矩阵 A 中的索引 u。

    参数:
    A (torch.Tensor): 点云矩阵 A，大小为 [N, 3]。
    B (torch.Tensor): 子集矩阵 B，大小为 [M, 3]。

    返回:
    torch.Tensor: 包含 B 中每个点在 A 中的索引 u 的张量，形状为 (M,)。
    """
    is_equal = torch.eq(B.view(1, -1, 3), A.view(-1, 1, 3))
    u_indices = torch.nonzero(is_equal, as_tuple=False)[:, 0]
    return torch.unique(u_indices)
if __name__ =="__main__":
    # 
    from time import time
    pass_=0
    # filename=f"pointcloud/pass_{pass_}.ply"
    filename = "point_cloud.ply"
    pcd = o3d.io.read_point_cloud(filename)
    point_cloud = torch.tensor(pcd.points)
    voxel_size = 8
    density_threshold=20
    displacement_scale=5
    for i in range(pass_+1, 50):
        print("pass ",i)
        time0 = time()
        
        point_downsample = point_cloud
        flag = False
        while point_downsample.shape[0]>1000:
            if flag:
                voxel_size+=8
            print("point size:",point_downsample.shape[0])
            point_downsample = downsample_point_cloud_open3d(point_cloud,voxel_size=voxel_size)
            flag = True
            
        print("point size:",point_downsample.shape[0])
        # downsampled_point_index = find_point_indices(point_cloud, point_downsample)
        downsampled_point_index = find_indices_in_A(point_cloud, point_downsample)
        print("selected_num",point_cloud[downsampled_point_index].shape[0])
        _, low_density_points, new_points, low_density_index = addpoint(point_cloud[downsampled_point_index],density_threshold=density_threshold,displacement_scale=displacement_scale,iter_pass=0)
        if new_points.shape[0] < 100:
            density_threshold /= 2
            displacement_scale /= 2
            print("reduce diplacement_scale to: ",displacement_scale)
            
        global_mask = torch.zeros((point_cloud.shape[0]), dtype=torch.bool)

        global_mask[downsampled_point_index] = low_density_index
        time1 = time()

        print("time cost:",time1-time0,"new_points:",new_points.shape[0])
        if low_density_points.shape[0] == 0:
            print("no more points.")
            continue
        # breakpoint()
        point = combine_pointcloud(point_cloud, low_density_points, new_points)
        point_cloud = torch.tensor(point.points)
        o3d.io.write_point_cloud(f"pointcloud/pass_{i}.ply",point)
        # visualize_qpoint_cloud( point_cloud, low_density_points, new_points)
    