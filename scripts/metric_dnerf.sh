exp_name1=$1

export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name1/standup/"  &
export CUDA_VISIBLE_DEVICES=1&&python metrics.py --model_path "output/$exp_name1/jumpingjacks/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name1/bouncingballs/" &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name1/lego/"   

export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name1/hellwarrior/"  &
export CUDA_VISIBLE_DEVICES=1&&python metrics.py --model_path "output/$exp_name1/hook/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name1/trex/" &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name1/mutant/"   &
wait
echo "Done"
