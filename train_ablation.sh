exp_name1="main_coarse2fine_timegrid4_pointgrow"
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/standup --port 6015 --expname "standup/$exp_name1" &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/lego --port 6016 --expname "lego/$exp_name1"
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/jumpingjacks --port 6020 --expname "jumpingjack/$exp_name1/"&
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/bouncingballs --port 6018 --expname "bouncingball/$exp_name1/" 
# export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/hellwarrior --port 6015 --expname "hellwarrior/exp_name1" &
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/hook --port 6016 --expname "$hook/exp_name1" 
# export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/trex --port 6017 --expname "$trex/exp_name1" &
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/mutant --port 6018 --expname "$mutant/exp_name1" 

echo "Done"
