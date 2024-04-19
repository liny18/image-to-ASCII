#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <getopt.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"
#include "stb_image/stb_image_resize.h"

void reverse_string(std::string &str)
{
    std::reverse(str.begin(), str.end());
}

void show_usage(const std::string &executable_name)
{
    std::cerr << "\nUsage: " << executable_name << " -i <FILE> [-o <FILE>] [options] \n\n";
    std::cerr << "Options:\n"
                 "  -i, --input  <FILE>     Specify the path of the input image file (required)\n"
                 "  -o, --output <FILE>     Specify the path where the ASCII output will be saved\n"
                 "  -w, --width  <NUMBER>   Set the width of the ASCII output; maintains aspect ratio\n"
                 "  -c, --chars  <STRING>   Define the set of characters used in the ASCII output\n"
                 "  -p, --print             Print the ASCII output to the console instead of a file\n"
                 "  -r, --reverse           Reverse the character set used for ASCII output\n\n";
    exit(EXIT_FAILURE);
}

std::string get_basename(const std::string &full_path)
{
    size_t last_slash = full_path.find_last_of("/\\");
    std::string basename = (last_slash != std::string::npos) ? full_path.substr(last_slash + 1) : full_path;
    size_t last_dot = basename.find_last_of('.');
    if (last_dot != std::string::npos)
    {
        basename = basename.substr(0, last_dot);
    }
    return basename + ".txt";
}

int main(int argc, char **argv)
{

    std::string executable_name = argv[0];

    // Exit if no command line arguments are given

    if (argc == 1)
    {
        show_usage(executable_name);
    }

    std::string characters = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

    // Parse arguments from the command line

    std::string input_filepath;
    std::string output_filepath;
    bool reverse_flag = false;
    bool print_flag = false;
    bool resize_image = false;
    int desired_width = 0;

    struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"input", required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"width", required_argument, nullptr, 'w'},
        {"chars", required_argument, nullptr, 'c'},
        {"print", no_argument, nullptr, 'p'},
        {"reverse", no_argument, nullptr, 'r'},
        {0, 0, 0, 0}};

    int option;
    const char *short_options = "hi:o:w:c:pr";
    while ((option = getopt_long(argc, argv, short_options, long_options, nullptr)) != EOF)
    {
        switch (option)
        {
        case 'h':
            show_usage(executable_name);
            break;
        case 'i':
            input_filepath = optarg;
            break;
        case 'o':
            output_filepath = optarg;
            break;
        case 'w':
            desired_width = std::atoi(optarg);
            resize_image = true;
            break;
        case 'c':
            characters = optarg;
            break;
        case 'p':
            print_flag = true;
            break;
        case 'r':
            reverse_flag = true;
            break;
        default:
            show_usage(executable_name);
        }
    }

    if (output_filepath.empty())
    {
        output_filepath = get_basename(input_filepath);
    }

    // Load the image in grayscale

    int width, height;
    std::unique_ptr<unsigned char, void (*)(void *)> image(stbi_load(input_filepath.c_str(), &width, &height, nullptr, STBI_grey), stbi_image_free);

    if (!image)
    {
        std::cerr << "Could not load image\n";
        exit(EXIT_FAILURE);
    }

    // Resize the image

    int desired_height;

    if (resize_image)
    {
        if (desired_width <= 0)
        {
            fprintf(stderr, "Argument 'width' must be greater than 0 \n");
            exit(EXIT_FAILURE);
        }

        desired_height = height * desired_width / width;

        std::unique_ptr<unsigned char, void (*)(void *)> resized_image(new unsigned char[desired_width * desired_height], stbi_image_free);
        if (resized_image == NULL)
        {
            std::cerr << "Could not allocate memory for the resized image\n";
            exit(EXIT_FAILURE);
        }

        stbir_resize_uint8(image.get(), width, height, 0, resized_image.get(), desired_width, desired_height, 0, STBI_grey);

        image.swap(resized_image);
        width = desired_width;
        height = desired_height;
    }
    else
    {
        desired_width = width;
        desired_height = height;
    }

    std::ofstream file(output_filepath);
    if (!file)
    {
        std::cerr << "Could not create an output file\n";
        exit(EXIT_FAILURE);
    }

    if (reverse_flag)
    {
        reverse_string(characters);
    }

    file << executable_name << ' ';
    for (int i = 1; i < argc; ++i)
    {
        file << argv[i] << ' ';
    }

    file << "\n\nInput: " << input_filepath << "\nOutput: " << output_filepath
         << "\nResolution: " << desired_width << 'x' << desired_height << "\nCharacters (" << characters.size() << "): \"" << characters << "\"\n\n";

    for (int i = 0; i < desired_height * desired_width; i++)
    {
        int intensity = image.get()[i];
        int character_index = intensity / (255 / (float)(characters.size() - 1));
        if (print_flag)
        {
            std::cout << characters[character_index];
        }
        file << characters[character_index];

        if ((i + 1) % desired_width == 0)
        {
            if (print_flag)
            {
                std::cout << '\n';
            }
            file << '\n';
        }
    }

    std::cout << "File saved as '" << output_filepath << "'\n";

    return EXIT_SUCCESS;
}