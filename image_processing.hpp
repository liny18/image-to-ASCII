#ifndef __IMAGE_PROCESSING_HPP__
#define __IMAGE_PROCESSING_HPP__

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// map each rank to a GPU
void map_rank_to_gpu(int my_rank);

// load the image from the input file path
cv::Mat load_image(const std::string &input_filepath);

// resize the image to the desired width and height
void resize_image(cv::Mat &image, int desired_width, int desired_height);

// split the image into parts for each rank
cv::Mat split_image(const cv::Mat &full_image, int my_rank, int num_ranks);

// process the image to get the ASCII art string and the ASCII image
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag);

#endif // __IMAGE_PROCESSING_HPP__