import json
import os
exp_name = ["dnerf_gridlarge","dnerf_gridsmall","dnerf_gridsmaller","dnerf_mlplarge","dnerf_mlplarger","dnerf_nocoarse","dnerf_silm","dnerf_imageloss",
            "dnerf_3dgs","dnerf_tv","dnerf_noboth","dnerf_nogrid"]
scene_name = ["bouncingball","jumpingjack","lego","standup","hook","mutant","hellwarrior","trex"]
json_name = "results.json"
result_json = {"SSIM":0,"PSNR":0,"LPIPS":0}
exp_json = {}
for exps in exp_name:
    exp_json[exps] = result_json.copy()
for scene in scene_name:
    for experiment in exp_name:
        load_path = os.path.join("output",experiment,scene,json_name)
        with open(load_path) as f:
            js = json.load(f)
        # print(js)
        # print(scene, experiment, js["ours_20000"])
        for res in ["ours_30000","ours_20000","ours_14000","ours_7000","ours_3000"]:
            if res in js.keys():
                for key, item in js[res].items():
                    exp_json[experiment][key] += item
                break

# for scene in scene_name:

for experiment in exp_name:
    print(exp_json[experiment])
    for key, item in exp_json[experiment].items():
        exp_json[experiment][key] /= 8
for key,item in exp_json.items():
    print(key)
    print("%.4f"%item["PSNR"],"&","%.4f"%item["SSIM"],"&","%.4f"%item["LPIPS"],)
        # break