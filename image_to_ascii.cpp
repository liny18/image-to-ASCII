#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib>
#include <getopt.h>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <mpi.h>

namespace fs = std::filesystem;

// default configuration for ASCII art
int CHAR_WIDTH = 10;
int CHAR_HEIGHT = 18;
float SCALE_FACTOR = 1.0;

// default characters used for ASCII art
std::string characters = " .'`^\",:;Il!i><~+_-?][}{1)(|\\//tfjrxnuvczXYUJCLQ0OZmwqpbdkhao*#MWM&8%B@$";

// reversing the characters creates a negative image
void reverse_string(std::string &str)
{
    std::reverse(str.begin(), str.end());
}

// display the usage of the program
void show_usage(const std::string &executable_name)
{
    std::cerr << "\nUsage: " << executable_name << " -i <DIRECTORY> [options] \n\n";
    std::cerr << "Options:\n"
                 "  -h, --help              Display this help message\n"
                 "  -i, --input  <DIRECTORY>     Specify the path of the input image directory (required)\n"
                 "  -w, --width  <NUMBER>   Set the width of the ASCII output; maintains aspect ratio\n"
                 "  -s, --chars  <STRING>   Define the set of characters used in the ASCII output\n"
                 "  -p, --print             Print the ASCII output to the console\n"
                 "  -n, --negate            Get the negative of the ASCII image\n"
                 "  -f, --factor            Set the scale factor(a number from 0.0 to 1.0) to resize the image\n"
                 "  -c, --color             Get ASCII PNG's in colors\n\n";
    exit(EXIT_FAILURE);
}

// get the basename of a file, used to match the output file name with the input file name
std::string get_basename(const std::string &full_path)
{
    size_t last_slash = full_path.find_last_of("/\\");
    std::string basename = (last_slash != std::string::npos) ? full_path.substr(last_slash + 1) : full_path;
    size_t last_dot = basename.find_last_of('.');
    if (last_dot != std::string::npos)
    {
        basename = basename.substr(0, last_dot);
    }
    return basename;
}

// need to make sure we're writing to a new file and directory
void check_output_directory()
{
    if (!fs::exists("outputs"))
    {
        fs::create_directory("outputs");
    }

    if (fs::exists("outputs/all_output.txt"))
    {
        fs::remove("outputs/all_output.txt");
    }
}

// parse the command line arguments and set the configuration
void parse_arguments(int argc, char **argv, std::string &input_directory, std::string &executable_name, bool &resize_flag, int &desired_width, bool &print_flag, bool &negate_flag, bool &colored_flag)
{
    struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"input", required_argument, nullptr, 'i'},
        {"width", required_argument, nullptr, 'w'},
        {"chars", required_argument, nullptr, 's'},
        {"print", no_argument, nullptr, 'p'},
        {"negate", no_argument, nullptr, 'n'},
        {"factor", required_argument, nullptr, 'f'},
        {"color", no_argument, nullptr, 'c'},
        {0, 0, 0, 0}};

    int option;
    const char *short_options = "hi:w:s:pncf:";
    while ((option = getopt_long(argc, argv, short_options, long_options, nullptr)) != EOF)
    {
        switch (option)
        {
        case 'h':
            show_usage(executable_name);
            break;
        case 'i':
            input_directory = optarg;
            break;
        case 'w':
            desired_width = std::atoi(optarg);
            resize_flag = true;
            break;
        case 's':
            characters = optarg;
            break;
        case 'p':
            print_flag = true;
            break;
        case 'n':
            negate_flag = true;
            break;
        case 'f':
            SCALE_FACTOR = std::atof(optarg);
            break;
        case 'c':
            colored_flag = true;
            break;
        default:
            show_usage(executable_name);
        }
    }
}

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

    cv::resize(image, image, cv::Size(static_cast<int>(desired_width * SCALE_FACTOR), static_cast<int>(desired_height * SCALE_FACTOR * (static_cast<float>(CHAR_WIDTH) / CHAR_HEIGHT))));
}

// process the image to get the ASCII art string and the ASCII image
std::pair<std::string, cv::Mat> process_image(const cv::Mat &image, bool colored_flag)
{
    std::string asciiArt;
    cv::Mat asciiImage(CHAR_HEIGHT * image.rows, CHAR_WIDTH * image.cols, image.type());

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

            cv::putText(asciiImage, std::string(1, asciiChar), cv::Point(j * CHAR_WIDTH, i * CHAR_HEIGHT + CHAR_HEIGHT), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

            asciiArt += asciiChar;
        }

        asciiArt += '\n';
    }

    asciiArt += '\n';

    return {asciiArt, asciiImage};
}

// save the ASCII art to a matching .txt file and a .png file
void save_output(const std::string &asciiArt, const cv::Mat &asciiImage, const std::string &input_filepath, bool colored_flag, int desired_width, int desired_height)
{
    std::string output_filepath = get_basename(input_filepath);
    std::ofstream file("outputs/" + output_filepath + ".txt");

    file << "Input: " << input_filepath << "\nOutput: " << output_filepath
         << ".txt\nResolution: " << desired_width << 'x' << desired_height
         << "\nCharacters (" << characters.size() << "): \"" << characters << "\"\n\n"
         << asciiArt;

    std::string color_output_string = (colored_flag) ? "_color" : "";

    cv::imwrite("outputs/" + output_filepath + color_output_string + ".png", asciiImage);

    file.close();
}

// void process_image(const cv::Mat &image, const std::string &input_filepath, bool colored_flag, std::unique_ptr<std::string> &local_output)
// {
//     std::string output_filepath = get_basename(input_filepath);

//     std::ofstream file("outputs/" + output_filepath + "wtf.txt");

//     file << "Input: " << input_filepath << "\nOutput: " << output_filepath
//          << ".txt\nResolution: " << image.cols << 'x' << image.rows << "\nCharacters (" << characters.size() << "): \"" << characters << "\"\n\n";

//     cv::Mat asciiImage(CHAR_HEIGHT * image.rows, CHAR_WIDTH * image.cols, image.type());

//     for (int i = 0; i < image.rows; i++)
//     {
//         for (int j = 0; j < image.cols; j++)
//         {
//             // Magic 🧙🏻🧙🏻‍♂️🧙🏻‍♀️
//             cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
//             float gray = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
//             int index = static_cast<int>(gray * (characters.size() - 1) / 255);

//             char asciiChar = characters[index];

//             cv::Scalar textColor = (colored_flag) ? cv::Scalar(pixel[0], pixel[1], pixel[2]) : cv::Scalar::all(255);

//             cv::putText(asciiImage, std::string(1, asciiChar), cv::Point(j * CHAR_WIDTH, i * CHAR_HEIGHT + CHAR_HEIGHT), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

//             *local_output += asciiChar;
//             file << asciiChar;
//         }

//         *local_output += '\n';
//         file << '\n';
//     }

//     std::string color_output_string = (colored_flag) ? "_color" : "";

//     cv::imwrite("outputs/" + output_filepath + color_output_string + "wtf.png", asciiImage);

//     *local_output += "\nASCII art saved to '" + output_filepath + ".txt' and '" + output_filepath + color_output_string + ".png'\n\n";

//     file.close();
// }

int main(int argc, char **argv)
{

    std::string executable_name = argv[0];

    // Exit if no command line arguments are given

    if (argc == 1)
    {
        show_usage(executable_name);
    }

    // Parse arguments from the command line

    std::string input_directory;
    bool negate_flag = false;
    bool print_flag = false;
    bool colored_flag = false;
    bool resize_flag = false;
    int desired_width = 0;

    parse_arguments(argc, argv, input_directory, executable_name, resize_flag, desired_width, print_flag, negate_flag, colored_flag);

    if (negate_flag)
    {
        reverse_string(characters);
    }

    check_output_directory();

    // MPI variables
    int myRank, numRanks;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);

    int desired_height = 0;

    // MPI I/O
    MPI_File mpi_file;
    int err = MPI_File_open(MPI_COMM_WORLD, "outputs/all_output.txt",
                            MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpi_file);

    if (err != MPI_SUCCESS)
    {
        std::cerr << "Error opening file\n";
        MPI_Abort(MPI_COMM_WORLD, err);
    }

    int initial_offset = 0;

    if (myRank == 0)
    {
        // Construct the header string
        std::string header = std::string(argv[0]) + " ";
        for (int i = 1; i < argc; ++i)
        {
            header += std::string(argv[i]) + " ";
        }
        header += "\n\n";

        // Rank 0 writes the header at the beginning of the file
        MPI_File_write(mpi_file, header.c_str(), header.size(), MPI_CHAR, MPI_STATUS_IGNORE);

        initial_offset = header.size();
    }

    // Ensure the header is written before other ranks start writing
    MPI_Barrier(MPI_COMM_WORLD);

    std::vector<std::string> input_files;

    for (const auto &entry : fs::directory_iterator(input_directory))
    {
        std::string extension = entry.path().extension().string();
        if (extension != ".jpg" && extension != ".jpeg" && extension != ".png")
        {
            continue;
        }
        input_files.push_back(entry.path().string());
    }

    int num_files = input_files.size();
    int files_per_rank = num_files / numRanks;
    int start_index = myRank * files_per_rank;
    int end_index = (myRank + 1) * files_per_rank;

    // Ensure the last rank processes any remaining files
    if (myRank == numRanks - 1)
    {
        end_index = num_files;
    }

    if (start_index >= num_files)
    {
        // Synchronize before any rank exits
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        // Exit if no work for this rank
        return EXIT_SUCCESS;
    }

    // create local storage for each rank
    std::unique_ptr<std::string> local_output = std::make_unique<std::string>();

    // Load and process the images for each rank
    for (int i = start_index; i < end_index; i++)
    {
        std::string input_filepath = input_files[i];
        cv::Mat image = load_image(input_filepath);

        resize_image(image, desired_width, desired_height, resize_flag);

        // process_image(image, input_filepath, colored_flag, local_output);

        // Process the image to get ASCII art string and optionally adjust the image size again for ASCII dimensions
        std::pair<std::string, cv::Mat> process_output = process_image(image, colored_flag);

        std::string processed_string = process_output.first;

        *local_output += processed_string;

        cv::Mat processed_image = process_output.second;

        // Save the ASCII art to a file and optionally save the colored ASCII image
        save_output(processed_string, processed_image, input_filepath, colored_flag, desired_width, desired_height);

        *local_output += "\nASCII art saved to '" + get_basename(input_filepath) + ".txt' and '" + get_basename(input_filepath) + "_color.png'\n\n";
    }

    if (print_flag)
    {
        std::cout << *local_output;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    int local_output_size = local_output->size();
    MPI_Offset offset = 0;

    MPI_Exscan(&local_output_size, &offset, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (myRank == 0)
    {
        offset = initial_offset;
    }

    MPI_File_write_at(mpi_file, offset, local_output->c_str(), local_output->size(), MPI_CHAR, MPI_STATUS_IGNORE);

    if (myRank == 0)
    {
        std::cout << "All outputs saved to 'outputs/all_output.txt'\n";
    }

    MPI_File_close(&mpi_file);

    MPI_Finalize();

    return EXIT_SUCCESS;
}