#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <mpi.h>
#include <opencv2/opencv.hpp>

#include "utils.hpp"
#include "constants.hpp"
#include "image_processing.hpp"

using namespace constants;

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

    check_file_exist();

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

    // Load all image files from the input directory for processing
    std::vector<std::string> input_files;

    get_image_files(input_directory, input_files);

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

        // Process the image to get ASCII art string and optionally adjust the image size again for ASCII dimensions
        std::pair<std::string, cv::Mat> process_output = process_image(image, colored_flag);

        std::string processed_string = process_output.first;

        *local_output += processed_string;

        cv::Mat processed_image = process_output.second;

        // Save the ASCII art to a file and optionally save the colored ASCII image
        save_output(processed_string, processed_image, input_filepath, colored_flag, desired_width, desired_height);

        *local_output += "\nASCII art saved to '" + get_basename(input_filepath) + ".txt' and '" + get_basename(input_filepath) + "_color.png'\n\n";

        image.release();
        processed_image.release();
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