import os

def calculate_total_size_of_files(folders):
    total_size = 0
    
    for folder_name in folders:
        deformation_path = os.path.join(folder_name, "./point_cloud/coarse_iteration_3000/deformation.pth")
        point_cloud_path = os.path.join(folder_name, "./point_cloud/coarse_iteration_3000/point_cloud.ply")
        # print(point_cloud_path)
        if os.path.exists(deformation_path):
            deformation_size = os.path.getsize(deformation_path)/(1024*1024)
            total_size += deformation_size
        
        if os.path.exists(point_cloud_path):
            point_cloud_size = os.path.getsize(point_cloud_path)/(1024*1024)
            total_size += point_cloud_size
    
    return total_size

for model_name in ["dnerf_3dgs"]:
    # model_name = "dnerf_tv"
    folder_names = ["bouncingball", "hook", "hellwarrior","jumpingjack","lego","mutant","standup","trex"]
    new_folder_names = [os.path.join("output",model_name,i) for i in folder_names]
    total_size = calculate_total_size_of_files(new_folder_names)
    print(model_name, "average size (MB):", total_size/len(folder_names))
