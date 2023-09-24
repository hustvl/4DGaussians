


import os

import imageio
data_path = "output/hypernerf_render/split-cookie/"
# coarse_id = [i*50 for i in range(1,60)]
# fine_id = [i * 50 for i in range(1,399)]
coarse_id = [i * 50 for i in range(1, 10)] + [i * 50+1000 for i in range(40)]  
fine_id  = [i * 10 for i in range(1, 100)] + [i * 50 for i in range(20,60)]  + [i* 100 for i in range(30,100)] + [i*200 for i in range(50,140)]
# breakpoint()
times = 268
# loading coarse images
coarse_path = os.path.join(data_path,"coarse_render","images")
fine_path = os.path.join(data_path,"fine_render","images")

load_path = []
for index, frame in enumerate(coarse_id):
    idx = index * 2
    if (index // times) % 2 ==0:
        time_stamp = index % times
    else:
        time_stamp = times - 1 - (index % times)
    load_path.append(os.path.join(coarse_path,f"{frame}_{time_stamp}.jpg"))
    last_index = index
for index, frame in enumerate(fine_id):
    thisindex = index + last_index
    if (thisindex // times) % 2 ==0:
        time_stamp = thisindex % times
    else:
        time_stamp = times - 1 - (thisindex % times)
    load_path.append(os.path.join(fine_path,f"{frame}_{time_stamp}.jpg"))
# print(load_path,sep="\n")
# breakpoint()
writer = imageio.get_writer(os.path.join(data_path,"trainingstep.mp4"), fps=15)
for image_file in load_path:
    image = imageio.imread(image_file)
    writer.append_data(image)

writer.close()