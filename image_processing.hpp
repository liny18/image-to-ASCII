#ifndef __IMAGE_PROCESSING_HPP__
#define __IMAGE_PROCESSING_HPP__

#include <string>

// map each rank to a GPU
#ifdef USE_GPU
void map_rank_to_gpu(int my_rank);
#endif // USE_GPU

// load the image from the input file path
cv::Mat load_image(const std::string &input_filepath);

// resize the image to the desired width and height
void resize_image(cv::Mat &image, int &desired_width, int &desired_height);

// Split the image into equal parts for each rank
cv::Mat split_image(const cv::Mat &full_image, int my_rank, int num_ranks);

// process the image to get the ASCII art string and the ASCII image
#ifdef USE_GPU
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag, int threads_x, int threads_y);
#else
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag);
#endif // USE_GPU

#endif // __IMAGE_PROCESSING_HPP__
