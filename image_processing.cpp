#include <iostream>
#include <vector>

#include "constants.hpp"
#include "image_processing.hpp"

using namespace constants;  

// load the image from the input directory
cv::Mat load_image(const std::string &input_directory)
{
    cv::Mat image = cv::imread(input_directory, cv::IMREAD_COLOR);

    if (image.empty())
    {
        std::cerr << "Could not read the image." << std::endl;
        exit(EXIT_FAILURE);
    }

    return image;
}

// resize the image to the desired width and height
void resize_image(cv::Mat &image, int &desired_width, int &desired_height, bool resize_flag)
{
    if (resize_flag)
    {
        desired_height = (image.rows * desired_width) / image.cols;
    }
    else
    {
        desired_width = image.cols;
        desired_height = image.rows;
    }

    cv::resize(image, image, cv::Size(static_cast<int>(desired_width * SCALE_FACTOR), static_cast<int>(desired_height * SCALE_FACTOR * (static_cast<float>(CHARACTER_WIDTH) / CHARACTER_HEIGHT))));
}

// process the image to get the ASCII art string and the ASCII image
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag)
{
    std::string asciiArt;

    cv::Mat asciiImage(CHARACTER_HEIGHT * image.rows, CHARACTER_WIDTH * image.cols, image.type());

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            // Magic 🧙🏻🧙🏻‍♂️🧙🏻‍♀️
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);

            float gray = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
            int index = static_cast<int>(gray * (characters.size() - 1) / 255);
            char asciiChar = characters[index];

            cv::Scalar textColor = (colored_flag) ? cv::Scalar(pixel[0], pixel[1], pixel[2]) : cv::Scalar::all(255);

            cv::putText(asciiImage, std::string(1, asciiChar), cv::Point(j * CHARACTER_WIDTH, i * CHARACTER_HEIGHT + CHARACTER_HEIGHT), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

            asciiArt += asciiChar;
        }

        asciiArt += '\n';
    }

    asciiArt += '\n';

    return {asciiArt, asciiImage};
}