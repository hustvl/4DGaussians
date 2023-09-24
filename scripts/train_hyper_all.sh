
# bash scripts/train_3dvideo.sh
# wait
bash scripts/train_hyper_one.sh hypernerf_format2_virg2
wait
bash scripts/train_hyper_one.sh hypernerf_format2_virg3
wait
# bash scripts/train_hyper.sh hypernerf_format2_lr2
# wait
# bash scripts/train_hyper.sh hypernerf_format2_prune
# wait
# wait
# bash scripts/train_ablation.sh dnerf_imageloss