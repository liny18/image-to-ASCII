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

// broadcast the configuration to all ranks since only rank 0 reads the command line arguments
void broadcast_config(std::string &input_filepath, std::string &output_filepath, bool &resize_flag, int &desired_width, bool &print_flag, bool &negate_flag, bool &colored_flag, bool &help_flag, int rank, std::string &CHARACTERS)
{

    int chars_length = CHARACTERS.size();
    MPI_Bcast(&chars_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        CHARACTERS.resize(chars_length);
    }

    MPI_Bcast(&CHARACTERS[0], chars_length, MPI_CHAR, 0, MPI_COMM_WORLD);

    int input_path_length = input_filepath.size();
    MPI_Bcast(&input_path_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        input_filepath.resize(input_path_length);
    }
    MPI_Bcast(&input_filepath[0], input_path_length, MPI_CHAR, 0, MPI_COMM_WORLD);

    int output_path_length = output_filepath.size();
    MPI_Bcast(&output_path_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        output_filepath.resize(output_path_length);
    }
    MPI_Bcast(&output_filepath[0], output_path_length, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Broadcast other configuration flags
    MPI_Bcast(&resize_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&desired_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&print_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&negate_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colored_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&help_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
}

// broadcast the image dimensions and type to all ranks
void broadcast_image(cv::Mat &image, int my_rank, MPI_Comm comm)
{
    int dims[3] = {image.rows, image.cols, image.type()};
    MPI_Bcast(dims, 3, MPI_INT, 0, comm);

    if (my_rank != 0)
    {
        image.create(dims[0], dims[1], dims[2]);
    }

    MPI_Bcast(image.data, image.total() * image.elemSize(), MPI_BYTE, 0, comm);
}

// gather the ASCII art from all ranks and save it to a file
void gather_and_save_ascii_art(cv::Mat &asciiImage, int rank, int size, const std::string &output_filepath, bool colored_flag)
{
    // total number of bytes in the asciiImage
    int sendcount = asciiImage.total() * asciiImage.elemSize();
    std::vector<int> recvcounts(size);
    std::vector<int> displs(size);

    // Gather all sendcounts to calculate total size and displacements
    MPI_Gather(&sendcount, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    cv::Mat fullImage;

    if (rank == 0)
    {
        int total_size = 0;
        for (int i = 0; i < size; ++i)
        {
            displs[i] = total_size;
            total_size += recvcounts[i];
        }

        // Allocate the fullImage to hold all gathered data
        fullImage.create(asciiImage.rows * size, asciiImage.cols, asciiImage.type());

        // Gather all images into fullImage
        MPI_Gatherv(asciiImage.data, sendcount, MPI_UNSIGNED_CHAR,
                    fullImage.data, recvcounts.data(), displs.data(),
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        // Save the full image at rank 0
        std::string color_output_string = (colored_flag) ? "_color" : "";
        cv::imwrite("outputs/" + output_filepath + color_output_string + ".png", fullImage);
    }
    else
    {
        MPI_Gatherv(asciiImage.data, sendcount, MPI_UNSIGNED_CHAR,
                    nullptr, nullptr, nullptr,
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    }
}

// write the ASCII art to a file using MPI I/O
void write_ascii_art_to_file(const std::string &ascii_art, const std::string &output_filepath, MPI_Comm comm, int my_rank, int num_ranks, int argc, char **argv)
{
    MPI_File fh;
    MPI_Status status;
    MPI_Offset offset = 0;

    int initial_offset = 0;
    std::string header;

    if (my_rank == 0)
    {
        header += "Ranks: " + std::to_string(num_ranks) + "\n";
        header += std::string(argv[0]) + " ";
        for (int i = 1; i < argc; ++i)
        {
            header += std::string(argv[i]) + " ";
        }
        header += "\n\n";
    }

    // Broadcast the size of the header to all processes
    initial_offset = header.size();
    MPI_Bcast(&initial_offset, 1, MPI_INT, 0, comm);

    // Each process calculates the length of its own ASCII art string
    int local_size = ascii_art.size();

    // Use MPI_Exscan to compute the offset for each process
    MPI_Exscan(&local_size, &offset, 1, MPI_INT, MPI_SUM, comm);

    if (my_rank == 0)
    {
        offset = initial_offset;
    }

    std::string output_filepath_txt = "outputs/" + output_filepath + ".txt";

    MPI_File_open(comm, output_filepath_txt.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    if (my_rank == 0)
    {
        MPI_File_write_at(fh, 0, header.c_str(), header.size(), MPI_CHAR, &status);
    }

    // Wait until the header is written by rank 0
    MPI_Barrier(comm);

    MPI_File_write_at(fh, offset, ascii_art.c_str(), local_size, MPI_CHAR, &status);

    MPI_File_close(&fh);
}

// print the ASCII art in order to the console
void print_in_order(const std::string &ascii_art, int my_rank, int num_ranks, MPI_Comm comm)
{
    if (num_ranks == 1)
    {
        std::cout << ascii_art;
        std::cout.flush();
        return;
    }

    if (my_rank == 0)
    {
        std::cout << ascii_art;
        std::cout.flush();
        // Signal next rank to print
        MPI_Send(NULL, 0, MPI_CHAR, 1, 0, comm);
        // Wait for the last rank to finish
        MPI_Recv(NULL, 0, MPI_CHAR, num_ranks - 1, 0, comm, MPI_STATUS_IGNORE);
    }
    else
    {
        MPI_Status status;
        // Wait for signal from previous rank
        MPI_Recv(NULL, 0, MPI_CHAR, my_rank - 1, 0, comm, &status);
        std::cout << ascii_art;
        std::cout.flush();
        // Notify the next rank, or wrap back to rank 0 if it's the last rank
        int next_rank = (my_rank + 1) % num_ranks;
        MPI_Send(NULL, 0, MPI_CHAR, next_rank, 0, comm);
    }
}

int main(int argc, char **argv)
{
    // Initialize MPI
    int my_rank, num_ranks;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    std::string executable_name = argv[0];

    // Exit if no command line arguments are given
    if (argc < 2)
    {
        if (my_rank == 0)
        {
            show_usage(executable_name);
        }

        MPI_Finalize();
        return EXIT_SUCCESS;
    }

    // set up necessary variables
    std::string input_filepath;
    std::string output_filepath;
    bool negate_flag = false;
    bool print_flag = false;
    bool colored_flag = false;
    bool resize_flag = false;
    bool help_flag = false;
    int desired_width = 0;

    if (my_rank == 0)
    {
        parse_arguments(argc, argv, input_filepath, output_filepath, executable_name, resize_flag, desired_width, print_flag, negate_flag, colored_flag, help_flag);

        // Reverse the characters used for ASCII art if negate_flag is set
        if (negate_flag)
        {
            reverse_string(CHARACTERS);
        }

        // Check if the input file exists
        check_file_exist();
    }

    // Broadcast the configuration to all ranks
    broadcast_config(input_filepath, output_filepath, resize_flag, desired_width, print_flag, negate_flag, colored_flag, help_flag, my_rank, CHARACTERS);

    MPI_Barrier(MPI_COMM_WORLD);

    if (help_flag)
    {
        MPI_Finalize();
        exit(EXIT_SUCCESS);
    }

    int desired_height = 0;
    cv::Mat input_image;

    if (my_rank == 0)
    {
        input_image = load_image(input_filepath);

        // Resize the image to the desired width and height
        if (resize_flag)
        {
            desired_height = (input_image.rows * desired_width) / input_image.cols;
        }
        else
        {
            desired_width = input_image.cols;
            desired_height = input_image.rows;
        }

        resize_image(input_image, desired_width, desired_height);
    }

    // Broadcast the calculated dimensions and type to all ranks
    broadcast_image(input_image, my_rank, MPI_COMM_WORLD);

    // Wait for all ranks to finish broadcasting
    MPI_Barrier(MPI_COMM_WORLD);

    // Split the image into parts for each rank
    cv::Mat subimage = split_image(input_image, my_rank, num_ranks);

    // Process the image to get the ASCII art string and the ASCII image
    std::pair<std::string, cv::Mat> process_output = process_image(subimage, colored_flag);
    std::string processed_string = process_output.first;
    cv::Mat processed_image = process_output.second;

    // Gather the ASCII art from all ranks and save it to a file
    gather_and_save_ascii_art(processed_image, my_rank, num_ranks, output_filepath, colored_flag);

    // Write the ASCII art to a file
    write_ascii_art_to_file(processed_string, output_filepath, MPI_COMM_WORLD, my_rank, num_ranks, argc, argv);

    if (my_rank == 0)
    {
        std::string color_output_string = (colored_flag) ? "_color" : "";
        std::cout << "ASCII art saved to outputs/" << output_filepath << ".txt and outputs/" << output_filepath << color_output_string << ".png\n"
                  << std::endl;
    }

    // Print the ASCII art in order to the console
    if (print_flag)
    {
        print_in_order(processed_string, my_rank, num_ranks, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}