exp_name="train"
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/dnerf/hellwarrior --port 6015 --expname "hellwarrior$exp_name" &
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/dnerf/hook --port 6016 --expname "hook$exp_name" &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dnerf/trex --port 6017 --expname "trex$exp_name" &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dnerf/mutant --port 6018 --expname "mutant$exp_name" &

echo "Done"