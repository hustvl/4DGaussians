#include <iostream>
#include <torch/torch.h>
#include "rasterize_points.h"

int main() {
    // Create input tensors for testing
    int image_height = 256;
    int image_width = 256;
    torch::Tensor background = torch::zeros({3}).cuda();
    torch::Tensor means3D = torch::rand({100, 3}).cuda();  // Adjust the dimensions as needed
    torch::Tensor colors = torch::rand({100, 3}).cuda();   // Adjust the dimensions as needed
    torch::Tensor opacity = torch::rand({100}).cuda();     // Adjust the dimensions as needed
    torch::Tensor scales = torch::rand({100, 3}).cuda();    // Adjust the dimensions as needed
    torch::Tensor rotations = torch::rand({100, 4}).cuda(); // Adjust the dimensions as needed
    torch::Tensor deformation = torch::rand({100, 3}).cuda(); // Adjust the dimensions as needed
    float scale_modifier = 1.0;
    torch::Tensor cov3D_precomp = torch::rand({100, 3}).cuda(); // Adjust the dimensions as needed
    torch::Tensor viewmatrix = torch::rand({4, 4}).cuda();     // Adjust the dimensions as needed
    torch::Tensor projmatrix = torch::rand({4, 4}).cuda();     // Adjust the dimensions as needed
    float tan_fovx = 1.0;
    float tan_fovy = 1.0;
    torch::Tensor sh = torch::rand({100, 16, 3}).cuda();         // Adjust the dimensions as needed
    int degree = 2;
    torch::Tensor campos = torch::rand({3}).cuda();           // Adjust the dimensions as needed

    // Test RasterizeGaussiansCUDA
    bool debug = false;
    auto result = RasterizeGaussiansCUDA(
        background, means3D, colors, opacity, scales, rotations,
        deformation, scale_modifier, cov3D_precomp, viewmatrix, projmatrix,
        tan_fovx, tan_fovy, image_height, image_width, sh, degree, campos, false, debug);

    // Test RasterizeGaussiansBackwardCUDA
    torch::Tensor radii = torch::rand({100}).cuda();           // Adjust the dimensions as needed
    torch::Tensor dL_dout_color = torch::rand({1, 3, image_height, image_width}).cuda();
    torch::Tensor dL_dout_depth = torch::rand({1, 1, image_height, image_width}).cuda();
    torch::Tensor dL_dout_velocity = torch::rand({1, 3, image_height, image_width}).cuda();
    torch::Tensor geomBuffer = torch::rand({1, 3, image_height, image_width}).cuda();
    int R = 4;  // Adjust as needed
    torch::Tensor binningBuffer = torch::rand({1, R * R, image_height, image_width}).cuda();
    torch::Tensor imageBuffer = torch::rand({1, 3, image_height, image_width}).cuda();

    auto backwardResult = RasterizeGaussiansBackwardCUDA(
        background, means3D, radii, colors, scales, rotations, deformation,
        scale_modifier, cov3D_precomp, viewmatrix, projmatrix, tan_fovx, tan_fovy,
        dL_dout_color, dL_dout_depth, dL_dout_velocity, sh, degree, campos,
        geomBuffer, R, binningBuffer, imageBuffer, debug);

    // Test markVisible
    torch::Tensor marked = markVisible(means3D, viewmatrix, projmatrix);

    // Print or use the results as needed
    std::cout << "RasterizeGaussiansCUDA result: " << std::get<0>(result) << std::endl;
    std::cout << "RasterizeGaussiansBackwardCUDA result: " << std::get<0>(backwardResult) << std::endl;

    return 0;
}
