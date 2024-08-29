import open3d as o3d
import numpy as np

def grow_sparse_regions(input_file, output_file):
    pcd = o3d.io.read_point_cloud(input_file)
    densities = o3d.geometry.PointCloud.compute_nearest_neighbor_distance(pcd)
    avg_density = np.average(densities)
    print(f"Average density: {avg_density}")
    sparse_indices = np.where(densities > avg_density * 1.2)[0] 
    sparse_points = np.asarray(pcd.points)[sparse_indices]


    o3d.io.write_point_cloud(output_file, pcd)

grow_sparse_regions("data/hypernerf/vrig/chickchicken/dense_downsample.ply", "data/hypernerf/interp/chickchicken/dense_downsample.ply")