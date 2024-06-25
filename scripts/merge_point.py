import open3d as o3d
import os
from tqdm import tqdm
def merge_point_clouds(directory, output_file):
    merged_pcd = o3d.geometry.PointCloud()

    for filename in tqdm(os.listdir(directory)):
        if filename.endswith('.ply'):
            pcd = o3d.io.read_point_cloud(os.path.join(directory, filename))
            merged_pcd += pcd

    merged_pcd = merged_pcd.remove_duplicate_points()

    o3d.io.write_point_cloud(output_file, merged_pcd)

merge_point_clouds("point_clouds_directory", "merged.ply")