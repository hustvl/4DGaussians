import open3d as o3d
import os
from tqdm import tqdm
def merge_point_clouds(directory, output_file):
    # 初始化一个空的点云
    merged_pcd = o3d.geometry.PointCloud()

    # 遍历文件夹下的所有文件
    for filename in tqdm(os.listdir(directory)):
        if filename.endswith('.ply'):
            # 读取点云文件
            pcd = o3d.io.read_point_cloud(os.path.join(directory, filename))
            # 将点云合并
            merged_pcd += pcd

    # 移除位置相同的点
    merged_pcd = merged_pcd.remove_duplicate_points()

    # 将合并后的点云输出到一个文件中
    o3d.io.write_point_cloud(output_file, merged_pcd)

# 使用函数
merge_point_clouds("point_clouds_directory", "merged.ply")