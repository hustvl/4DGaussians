# bash colmap.sh data/hypernerf/interp/aleks-teapot
# bash colmap.sh data/hypernerf/interp/chickchicken
# bash colmap.sh data/hypernerf/interp/cut-lemon1
# bash colmap.sh data/hypernerf/interp/hand1-dense-v2
# bash colmap.sh data/hypernerf/interp/slice-banana
# bash colmap.sh data/hypernerf/interp/torchocolate


# bash colmap.sh data/hypernerf/virg/broom2
# bash colmap.sh data/hypernerf/virg/peel-banana
# bash colmap.sh data/hypernerf/virg/vrig-3dprinter
# bash colmap.sh data/hypernerf/virg/vrig-chicken
# python scripts/downsample_point.py data/dynerf/coffee_martini/points3D_downsample.ply data/dynerf/coffee_martini/points3D_downsample2.ply
# python scripts/downsample_point.py data/dynerf/flame_salmon_1/points3D_downsample.ply data/dynerf/flame_salmon_1/points3D_downsample2.ply
# python scripts/downsample_point.py data/dynerf/cut_roasted_beef/points3D_downsample.ply data/dynerf/cut_roasted_beef/points3D_downsample2.ply
# python scripts/downsample_point.py data/dynerf/cook_spinach/points3D_downsample.ply data/dynerf/cook_spinach/points3D_downsample2.ply
# python scripts/downsample_point.py data/dynerf/flame_steak/points3D_downsample.ply data/dynerf/flame_steak/points3D_downsample2.ply
# python scripts/downsample_point.py data/dynerf/sear_steak/points3D_downsample.ply data/dynerf/sear_steak/points3D_downsample2.ply
# python scripts/downsample_point.py data/hypernerf/virg/broom2/dense.ply data/hypernerf/virg/broom2/dense_downsample.ply
# python scripts/downsample_point.py data/hypernerf/virg/peel-banana/dense.ply data/hypernerf/virg/peel-banana/dense_downsample.ply
# python scripts/downsample_point.py data/hypernerf/virg/vrig-chicken/dense.ply data/hypernerf/virg/vrig-chicken/dense_downsample.ply
# python scripts/downsample_point.py data/hypernerf/virg/vrig-3dprinter/dense.ply data/hypernerf/virg/vrig-3dprinter/dense_downsample.ply
# bash colmap.sh data/dycheck/sriracha-tree
# bash colmap.sh data/dycheck/apple
# bash colmap.sh data/dycheck/space-out
# bash colmap.sh data/dycheck/teddy
# bash colmap.sh data/dycheck/wheel
# bash colmap.sh data/dycheck/spin
# bash colmap.sh data/dnerf/hook/
# bash colmap.sh data/dnerf/mutant
# bash colmap.sh data/dnerf/standup
# bash colmap.sh data/dnerf/lego
# bash colmap.sh data/dnerf/trex
# bash colmap.sh data/dnerf/bouncingballs
# bash colmap.sh data/dnerf/hellwarrior

# bash colmap.sh data/nerf_synthetic/chair
# bash colmap.sh data/nerf_synthetic/drums
# bash colmap.sh data/nerf_synthetic/ficus
# bash colmap.sh data/nerf_synthetic/hotdog
# bash colmap.sh data/nerf_synthetic/lego
# bash colmap.sh data/nerf_synthetic/materials
# bash colmap.sh data/nerf_synthetic/mic
# bash colmap.sh data/nerf_synthetic/ship

# bash scripts/metric_dynerf.sh dynerf_batch4_do
# wait
# bash scripts/metric_hyper_one.sh hypernerf2
# wait
# bash scripts/metric_hyper_one.sh hypernerf_emptyvoxel2
# wait
# bash scripts/metric_hyper_one.sh hypernerf_emptyvoxel
# wait

# bash scripts/metric_dynerf.sh dynerf_batch1_do
# wait
# bash scripts/metric_dynerf.sh dynerf_res124
# wait
# bash scripts/metric_dynerf.sh dynerf_emptyvoxel1
# wait

# bash scripts/metric_dynerf.sh dynerf_emptyvoxel2
# wait
# exp_name="dynerf_static"
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dynerf/flame_salmon_1/colmap/dense/workspace --port 6368 --expname "$exp_name/flame_salmon_1" --configs arguments/$exp_name/default.py &
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dynerf/coffee_martini/colmap/dense/workspace --port 6369 --expname "$exp_name/coffee_martini" --configs arguments/$exp_name/default.py

# exp_name="dynerf_4_batch1"
# bash scripts/train_dynerf_ab1.sh dynerf_4_batch1_2 &

# bash scripts/train_dynerf_ab2.sh dynerf_4_batch4_2
# wait
# bash scripts/train_hyper_virg.sh hypernerf3
# bash scripts/train_hyper_interp.sh hypernerf4
# bash scripts/train_hyper_virg.sh hypernerf_3dgs
# exp_name="hypernerf4"
# export CUDA_VISIBLE_DEVICES=0&&python vis_point.py --model_path output/$exp_name/broom2 --configs arguments/$exp_name/broom2.py  &
# export CUDA_VISIBLE_DEVICES=2&&python vis_point.py --model_path output/$exp_name/3dprinter  --configs arguments/$exp_name/3dprinter.py &
# export CUDA_VISIBLE_DEVICES=2&&python vis_point.py --model_path output/$exp_name/peel-banana --configs arguments/$exp_name/banana.py&
# export CUDA_VISIBLE_DEVICES=3&&python vis_point.py --model_path output/$exp_name/vrig-chicken  --configs arguments/$exp_name/chicken.py &
# wait


# exp_name="dnerf_tv_2"
# export CUDA_VISIBLE_DEVICES=3&&python editing.py --model_path output/$exp_name/lego


# exp_name="dnerf_tv_2_1"
# export CUDA_VISIBLE_DEVICES=3&&python vis_point.py --model_path output/ablation/$exp_name/hook  --configs arguments/$exp_name/hook.py
# export CUDA_VISIBLE_DEVICES=3&&python vis_point.py --model_path output/ablation/$exp_name/hellwarrior  --configs arguments/$exp_name/hellwarrior.py
# export CUDA_VISIBLE_DEVICES=3&&python vis_point.py --model_path output/ablation/$exp_name/jumpingjacks  --configs arguments/$exp_name/jumpingjacks.py
# export CUDA_VISIBLE_DEVICES=3&&python vis_point.py --model_path output/ablation/$exp_name/standup  --configs arguments/$exp_name/standup.py

exp_name1="medical"
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/medicaldata/images --port 6068 --expname "medical/$exp_name1/" --configs arguments/$exp_name1/bouncingballs.py
