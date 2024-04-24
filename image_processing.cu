#include <iostream>
#include <string>
#include <cuda.h>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>

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
    cv::resize(image, image, cv::Size(static_cast<int>(desired_width * SCALE_FACTOR), static_cast<int>(desired_height * SCALE_FACTOR * (static_cast<float>(CHARACTER_WIDTH) / CHARACTER_HEIGHT))));
}

// Split the image into equal parts for each rank
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

    return full_image.rowRange(start_row, end_row + 1).clone();
}

// Convert the image to ASCII art
__global__ void imageToAsciiKernel(cv::cuda::PtrStepSz<uchar3> input, char *output, int width, const char *device_characters, int num_chats)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < input.rows)
    {
        uchar3 pixel = input(y, x);
        // Magic 🧙🏻🧙🏻‍♂️🧙🏻‍♀️
        float gray = 0.299f * pixel.x + 0.587f * pixel.y + 0.114f * pixel.z;
        int index = static_cast<int>(gray * (num_chats - 1) / 255);
        index = max(0, min(index, num_chats - 1));
        char ascii_char = device_characters[index];

        output[y * width + x] = ascii_char;
    }
}

// Process the image on the GPU and return the ASCII art and the ASCII image
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag, int threads_x, int threads_y)
{
    cv::cuda::GpuMat d_image(image);
    size_t num_chars = image.rows * image.cols;
    char *d_ascii_art;
    cudaMalloc(&d_ascii_art, sizeof(char) * num_chars);

    // Transfer CHARACTERS to device
    const char *device_characters;
    size_t char_size = CHARACTERS.size();
    cudaMalloc(&device_characters, char_size);
    cudaMemcpy((void *)device_characters, CHARACTERS.c_str(), char_size, cudaMemcpyHostToDevice);

    dim3 threads_per_block(threads_x, threads_y);
    dim3 num_blocks((image.cols + threads_per_block.x - 1) / threads_per_block.x,
                    (image.rows + threads_per_block.y - 1) / threads_per_block.y);

    imageToAsciiKernel<<<num_blocks, threads_per_block>>>(d_image, d_ascii_art, image.cols, device_characters, char_size);
    cudaDeviceSynchronize();

    // Copy the ASCII art from device to host
    std::vector<char> ascii_art(num_chars);
    cudaMemcpy(&ascii_art[0], d_ascii_art, sizeof(char) * num_chars, cudaMemcpyDeviceToHost);

    std::string ascii_art_str;
    // Extra space for newlines
    ascii_art_str.reserve(num_chars + image.rows);
    for (int i = 0; i < image.rows; ++i)
    {
        ascii_art_str.append(&ascii_art[i * image.cols], image.cols);
        ascii_art_str.push_back('\n');
    }

    cudaFree(d_ascii_art);

    // Handling the drawing on host
    cv::Mat ascii_image = cv::Mat::zeros(image.rows * CHARACTER_HEIGHT, image.cols * CHARACTER_WIDTH, CV_8UC3);
    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            // Get the character at the current position
            std::string text(1, ascii_art[y * image.cols + x]);
            int pos_x = x * CHARACTER_WIDTH;
            int pox_y = y * CHARACTER_HEIGHT + CHARACTER_HEIGHT;
            cv::Scalar color = colored_flag ? image.at<cv::Vec3b>(y, x) : cv::Scalar::all(255);
            cv::putText(ascii_image, text, cv::Point(pos_x, pox_y), cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
        }
    }

    return std::make_pair(ascii_art_str, ascii_image);
}