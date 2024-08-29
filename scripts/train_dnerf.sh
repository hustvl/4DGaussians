exp_name1=$1

export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/lego --port 6068 --expname "$exp_name1/lego" --configs arguments/$exp_name1/lego.py &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/bouncingballs --port 6266 --expname "$exp_name1/bouncingballs" --configs arguments/$exp_name1/bouncingballs.py &
wait
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/jumpingjacks --port 6069 --expname "$exp_name1/jumpingjacks" --configs arguments/$exp_name1/jumpingjacks.py  &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/trex --port 6070 --expname "$exp_name1/trex" --configs arguments/$exp_name1/trex.py &
wait
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/mutant --port 6068 --expname "$exp_name1/mutant" --configs arguments/$exp_name1/mutant.py &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/standup --port 6066 --expname "$exp_name1/standup" --configs arguments/$exp_name1/standup.py &
wait
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/hook --port 6069 --expname "$exp_name1/hook" --configs arguments/$exp_name1/hook.py  &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/hellwarrior --port 6070 --expname "$exp_name1/hellwarrior" --configs arguments/$exp_name1/hellwarrior.py &
wait
echo "Done"