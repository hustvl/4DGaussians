# bash scripts/process_dnerf.sh dnerf_ab/dnerf_tv_30000
# bash scripts/process_dnerf.sh dnerf_ab/dnerf_tv_depth2
# bash scripts/process_dnerf_2.sh dnerf_tv_dshs
# bash scripts/process_dnerf_2.sh dnerf_tv_do
# bash scripts/process_dnerf_2.sh dnerf_tv_2
# bash scripts/process_dnerf_2.sh dnerf_tv_8
# bash scripts/process_dnerf_2.sh dnerf_tv_deepmlp
# bash scripts/process_dnerf_2.sh dnerf_tv_nods

# bash scripts/process_dnerf.sh dnerf_ab/dnerf_tv
# exp_name1="hypernerf_3dgs"
# export CUDA_VISIBLE_DEVICES=2&&python render2.py --model_path "output/$exp_name1/3dprinter/"  --skip_train --configs arguments/$exp_name1/3dprinter.py  &
# export CUDA_VISIBLE_DEVICES=3&&python render2.py --model_path "output/$exp_name1/broom2/"  --skip_train --configs arguments/$exp_name1/broom2.py  &
# # 
# wait
# export CUDA_VISIBLE_DEVICES=2&&python render2.py --model_path "output/$exp_name1/peel-banana/"  --skip_train --configs arguments/$exp_name1/banana.py  &
# export CUDA_VISIBLE_DEVICES=3&&python render2.py --model_path "output/$exp_name1/vrig-chicken/"  --skip_train --configs arguments/$exp_name1/chicken.py  &
# wait
# exp_name="hypernerf_3dgs"
# export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/vrig-chicken/" &
# export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/peel-banana/" &
# export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/broom2/" &
# export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/3dprinter/" &

# bash scripts/train_ablation.sh dnerf_tv_2_1

bash scripts/process_dnerf.sh dnerf
bash scripts/train_dynerf.sh dynerf
bash scripts/train_hyper_interp.sh hypernerf
bash scripts/train_hyper_virg.sh hypernerf