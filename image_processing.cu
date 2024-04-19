#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/cuda.hpp>

cv::cuda::GpuMat load_image(const std::string &input_directory)
{
    cv::Mat image = cv::imread(input_directory, cv::IMREAD_COLOR);
    if (image.empty())
    {
        std::cerr << "Could not read the image." << std::endl;
        exit(EXIT_FAILURE);
    }

    cv::cuda::GpuMat gpu_image;
    gpu_image.upload(image);

    return gpu_image;
}

void resize_image(cv::cuda::GpuMat &gpu_image, bool resize_flag, int &desired_width, int &desired_height)
{
    if (resize_flag)
    {
        desired_height = (gpu_image.rows * desired_width) / gpu_image.cols;
    }
    else
    {
        desired_width = gpu_image.cols;
        desired_height = gpu_image.rows;
    }

    cv::cuda::GpuMat resized_image;
    cv::Size new_size(static_cast<int>(desired_width * SCALE_FACTOR),
                      static_cast<int>(desired_height * SCALE_FACTOR * (static_cast<float>(CHAR_WIDTH) / CHAR_HEIGHT)));

    cv::cuda::resize(gpu_image, resized_image, new_size);

    gpu_image = std::move(resized_image);
}

void process_image