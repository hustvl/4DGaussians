from argparse import ArgumentParser
import sys
sys.path.append('./scene')
from neural_3D_dataset_NDC import Neural3D_NDC_Dataset
# import scene
# from scene.neural_3D_dataset_NDC import Neural3D_NDC_Dataset

if __name__ == '__main__':
    parser = ArgumentParser(description="Extract images from dynerf videos")
    parser.add_argument("--datadir", default='data/dynerf/cut_roasted_beef', type=str)
    args = parser.parse_args()
    train_dataset = Neural3D_NDC_Dataset(args.datadir, "train", 1.0, time_scale=1, 
                                         scene_bbox_min=[-2.5, -2.0, -1.0], scene_bbox_max=[2.5, 2.0, 1.0], eval_index=0)    
    test_dataset = Neural3D_NDC_Dataset(args.datadir, "test", 1.0, time_scale=1, 
                                        scene_bbox_min=[-2.5, -2.0, -1.0], scene_bbox_max=[2.5, 2.0, 1.0], eval_index=0)
