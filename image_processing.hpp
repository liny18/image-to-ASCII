#ifndef __IMAGE_PROCESSING_HPP__
#define __IMAGE_PROCESSING_HPP__

#include <opencv2/opencv.hpp>
#include <string>

// load the image from the input directory
cv::Mat load_image(const std::string &input_directory);

// resize the image to the desired width and height
void resize_image(cv::Mat &image, int &desired_width, int &desired_height, bool resize_flag);

// process the image to get the ASCII art string and the ASCII image
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag);

#endif // __IMAGE_PROCESSING_HPP__