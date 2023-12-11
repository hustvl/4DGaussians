import open3d as o3d
import numpy as np

def grow_sparse_regions(input_file, output_file):
    # 读取输入的ply文件
    pcd = o3d.io.read_point_cloud(input_file)

    # 计算点云的密度
    densities = o3d.geometry.PointCloud.compute_nearest_neighbor_distance(pcd)
    avg_density = np.average(densities)
    print(f"Average density: {avg_density}")

    # 找到稀疏部分
    sparse_indices = np.where(densities > avg_density * 1.2)[0]  # 这里我们假设稀疏部分的密度大于平均密度的1.2倍
    sparse_points = np.asarray(pcd.points)[sparse_indices]
    breakpoint()
    # 复制并增长稀疏部分
    # for _ in range(5):  # 这里我们假设每个稀疏点复制5次
        # pcd.points.extend(sparse_points)

    # 将结果保存到输入的路径中
    o3d.io.write_point_cloud(output_file, pcd)

# 使用函数
grow_sparse_regions("data/hypernerf/vrig/chickchicken/dense_downsample.ply", "data/hypernerf/interp/chickchicken/dense_downsample.ply")