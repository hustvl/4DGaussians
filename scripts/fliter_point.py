import open3d as o3d
import os
# 指定根目录路径
root_path = "data/dynerf/sear_steak/"

# 文件名
input_file = "points3D.ply"
output_file = "points3d_filtered.ply"

# 读取点云数据
point_cloud_before = o3d.io.read_point_cloud(os.path.join(root_path, input_file))

# 计算过滤前的点的数量
num_points_before = len(point_cloud_before.points)

# 计算过滤前的点云的边界框大小
bbox_before = point_cloud_before.get_axis_aligned_bounding_box()
bbox_size_before = bbox_before.get_max_bound() - bbox_before.get_min_bound()

# 进行离群点滤波
cl, ind = point_cloud_before.remove_statistical_outlier(nb_neighbors=20, std_ratio=2.0)

# 创建一个新的点云对象，包含滤波后的点
filtered_point_cloud = point_cloud_before.select_by_index(ind)

# 保存滤波后的点云到新文件
o3d.io.write_point_cloud(os.path.join(root_path, output_file), filtered_point_cloud)

# 计算过滤后的点的数量
num_points_after = len(filtered_point_cloud.points)

# 计算边界框的大小
bbox = filtered_point_cloud.get_axis_aligned_bounding_box()
bbox_size = bbox.get_max_bound() - bbox.get_min_bound()

print(f"过滤前的点数: {num_points_before}")
print(f"过滤前的点云边界框大小: {bbox_size_before}")
print(f"过滤后的点数: {num_points_after}")
print(f"过滤后的点云边界框大小: {bbox_size}")
print(f"离群点过滤完成，结果已保存到 {output_file}")
