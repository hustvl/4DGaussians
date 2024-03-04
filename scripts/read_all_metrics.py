import json
import os
# exp_name = ["hypernerf"]
# exp_name= ["dnerf"]
exp_name=["dynerf"]
scene_name = ["coffee_martini", "cook_spinach", "cut_roasted_beef", "flame_salmon_1", "flame_steak", "sear_steak"]
# scene_name = ["bouncingballs","jumpingjacks","lego","standup","hook","mutant","hellwarrior","trex"]
# scene_name = ["3dprinter","broom2","peel-banana","vrig-chicken"]
json_name = "results.json"
result_json = {"PSNR":0,"SSIM":0,"MS-SSIM":0,"D-SSIM":0,"LPIPS-vgg":0,"LPIPS-alex":0,"LPIPS":0}
exp_json = {}
for exps in exp_name:
    exp_json[exps] = result_json.copy()
for scene in scene_name:
    for experiment in exp_name:
        load_path = os.path.join("output",experiment,scene,json_name)
        with open(load_path) as f:
            js = json.load(f)
        for res in ["ours_30000","ours_20000","ours_14000","ours_10000","ours_7000","ours_3000"]:
            if res in js.keys():
                for key, item in js[res].items():
                    if key in exp_json[experiment].keys():
                        exp_json[experiment][key] += item
                    print(scene, key, item)
                break

# for scene in scene_name:

for experiment in exp_name:
    print(exp_json[experiment])
    for key, item in exp_json[experiment].items():
        exp_json[experiment][key] /= len(scene_name)
for key,item in exp_json.items():
    print(key)
    print("PSNR,SSIM,D-SSIM,MS-SSIM,LPIPS-alex,LPIPS-vgg","LPIPS")
    print("%.4f"%item["PSNR"],"&","%.4f"%item["SSIM"],"%.4f"%item["D-SSIM"],
          "%.4f"%item["MS-SSIM"],"&","%.4f"%item["LPIPS-alex"],"%.4f"%item["LPIPS-vgg"],
          "%.4f"%item["LPIPS"])
        # break