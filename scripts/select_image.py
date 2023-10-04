


import os

import imageio
data_path = "output/cut_roasted_beef/render"
# coarse_id = [i*50 for i in range(1,60)]
# fine_id = [i * 50 for i in range(1,399)]
coarse_id = [i * 50 for i in range(1, 20)] + [i * 100 for i in range(10,30)]  
fine_id  = [i * 50 for i in range(1, 20)] + [i* 100 for i in range(10,30)] + [i * 300+3000 for i in range(23)] + [i* 500 + 10000 for i in range(40) ]
# breakpoint()
times = 120
# loading coarse images
coarse_path = os.path.join(data_path,"coarse_render","images")
fine_path = os.path.join(data_path,"fine_render","images")

load_path = []
for index, frame in enumerate(coarse_id):
    
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
writer = imageio.get_writer(os.path.join(data_path,"trainingstep.mp4"), fps=30)
for image_file in load_path:
    image = imageio.imread(image_file)
    writer.append_data(image)

writer.close()