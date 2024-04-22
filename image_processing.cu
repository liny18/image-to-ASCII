#include <iostream>
#include <string>
#include <cuda.h>
#include <cuda_runtime.h>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudaimgproc.hpp>

#include "image_processing.hpp"
#include "constants.hpp"

using namespace constants;

int cudaDeviceCount;
cudaError_t cE;

// map each rank to a GPU
void map_rank_to_gpu(int my_rank)
{

    if ((cE = cudaGetDeviceCount(&cudaDeviceCount)) != cudaSuccess)
    {
        std::cerr << "Unable to determine cuda device count, error is " << cudaGetErrorString(cE) << std::endl;
        return;
    }

    // Assign GPU (simple round-robin)
    int assignedGpu = my_rank % cudaDeviceCount;
    if ((cE = cudaSetDevice(assignedGpu)) != cudaSuccess)
    {
        std::cerr << "Unable to set cuda device, error is " << cudaGetErrorString(cE) << std::endl;
        return;
    }
}

// Load an image from the input file path
cv::Mat load_image(const std::string &input_filepath)
{
    cv::Mat image = cv::imread(input_filepath, cv::IMREAD_COLOR);

    if (image.empty())
    {
        throw std::runtime_error("Could not read the image: " + input_filepath);
    }

    return image;
}

// resize the image to the desired width and height
void resize_image(cv::Mat &image, int &desired_width, int &desired_height)
{
    cv::cuda::GpuMat gpu_image;
    gpu_image.upload(image);

    cv::cuda::resize(gpu_image, gpu_image, cv::Size(static_cast<int>(desired_width * SCALE_FACTOR), static_cast<int>(desired_height * SCALE_FACTOR * (static_cast<float>(CHARACTER_WIDTH) / CHARACTER_HEIGHT))));

    gpu_image.download(image);
}

cv::Mat split_image(const cv::Mat &full_image, int my_rank, int num_ranks)
{
    int rows = full_image.rows;
    int rows_per_rank = rows / num_ranks;
    int remainder = rows % num_ranks;

    int start_row = my_rank * rows_per_rank + std::min(my_rank, remainder);
    int end_row = start_row + rows_per_rank - 1;
    if (my_rank < remainder)
    {
        end_row++;
    }

    // Ensure end_row does not exceed the last row of the image
    end_row = std::min(end_row, rows - 1);

    cv::cuda::GpuMat gpu_full_image;
    gpu_full_image.upload(full_image);

    cv::cuda::GpuMat gpu_image = gpu_full_image.rowRange(start_row, end_row + 1).clone();

    cv::Mat image;
    gpu_image.download(image);

    return image;
}

// process the image to get the ASCII art string and the ASCII image
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag)
{
    std::string ascii_art;

    cv::Mat ascii_image(CHARACTER_HEIGHT * image.rows, CHARACTER_WIDTH * image.cols, CV_8UC3, cv::Scalar(0, 0, 0));

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            // Magic 🧙🏻🧙🏻‍♂️🧙🏻‍♀️
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);

            float gray = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
            int index = static_cast<int>(gray * (CHARACTERS.size() - 1) / 255);
            char asciiChar = CHARACTERS[index];

            cv::Scalar textColor = (colored_flag) ? cv::Scalar(pixel[0], pixel[1], pixel[2]) : cv::Scalar::all(255);

            cv::putText(ascii_image, std::string(1, asciiChar), cv::Point(j * CHARACTER_WIDTH, i * CHARACTER_HEIGHT + CHARACTER_HEIGHT), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

            ascii_art += asciiChar;
        }

        ascii_art += '\n';
    }

    return std::make_pair(ascii_art, ascii_image);
}
