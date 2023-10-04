exp_name="train"
# export CUDA_VISIBLE_DEVICES=0&&python metrics.py --m "output/standup$exp_name"  &
# export CUDA_VISIBLE_DEVICES=1&&python metrics.py --m "output/lego$exp_name"  &
# export CUDA_VISIBLE_DEVICES=2&&python metrics.py --m "output/jumpingjack/$exp_name1/" &
# export CUDA_VISIBLE_DEVICES=2&&python metrics.py --m "output/$exp_name/trex" &

export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/bouncingball/$exp_name"  
echo "Done"