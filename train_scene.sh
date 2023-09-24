exp_name="main_coarse2fine_timegrid2"
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/standup --port 6015 --expname "$exp_name/standup" &
# export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/lego --port 6016 --expname "$exp_name/lego" &
# export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/jumpingjacks --port 6020 --expname "$exp_name/jumpingjack" &
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/bouncingballs --port 6018 --expname "$exp_name/bouncingball" 

# export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/dnerf/hellwarrior --port 6015 --expname "$exp_nam\hellwarrior" &
# export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/dnerf/hook --port 6016 --expname "$exp_name\hook" &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/trex --port 6017 --expname "$exp_name/trex" &
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/standup --port 6018 --expname "$exp_name/standup" &

echo "Done"