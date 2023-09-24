exp_name1=$1

export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path "output/$exp_name1/standup/"  --skip_train --configs arguments/$exp_name1/standup.py &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path "output/$exp_name1/jumpingjacks/"  --skip_train --configs arguments/$exp_name1/jumpingjacks.py &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path "output/$exp_name1/bouncingballs/"  --skip_train --configs arguments/$exp_name1/bouncingballs.py  &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path "output/$exp_name1/lego/"  --skip_train --configs arguments/$exp_name1/lego.py  &
wait
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path "output/$exp_name1/hellwarrior/"  --skip_train --configs arguments/$exp_name1/hellwarrior.py  &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path "output/$exp_name1/hook/"  --skip_train --configs arguments/$exp_name1/hook.py  &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path "output/$exp_name1/trex/"  --skip_train --configs arguments/$exp_name1/trex.py  &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path "output/$exp_name1/mutant/"  --skip_train --configs arguments/$exp_name1/mutant.py   &
# wait
echo "Done"
