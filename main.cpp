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
#include "clockcycle.h"

using namespace constants;

// ------------------ Function Prototypes ------------------
void broadcast_config(std::string &input_filepath, std::string &output_filepath, bool &resize_flag, int &desired_width, bool &print_flag, bool &negate_flag, bool &colored_flag, bool &help_flag, int rank, std::string &CHARACTERS, int &threads_x, int &threads_y);
void broadcast_image(cv::Mat &image, int my_rank, MPI_Comm comm);
void gather_and_print_to_console(const std::string& processed_string, int num_ranks, int my_rank, bool print_flag, bool colored_flag, const std::string& output_filepath);
void gather_and_save_ascii_art(cv::Mat &ascii_image, int rank, int size, const std::string &output_filepath, bool colored_flag);
void write_ascii_art_to_file(const std::string &ascii_art, const std::string &output_filepath_txt, MPI_Comm comm, int my_rank, MPI_Offset initial_offset);
// ---------------------------------------------------------

// broadcast the configuration to all ranks
void broadcast_config(std::string &input_filepath, std::string &output_filepath, bool &resize_flag, int &desired_width, bool &print_flag, bool &negate_flag, bool &colored_flag, bool &help_flag, int rank, std::string &CHARACTERS, int &threads_x, int &threads_y)
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

    // Broadcast other configuration parameters
    MPI_Bcast(&resize_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&desired_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&print_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&negate_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colored_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&help_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(&threads_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&threads_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

// broadcast the image dimensions and type to all ranks
void broadcast_image(cv::Mat &image, int my_rank, MPI_Comm comm) 
{
    int dims[3] = {image.rows, image.cols, image.type()};
    MPI_Bcast(dims, 3, MPI_INT, 0, comm);

    if (my_rank != 0) {
        image.create(dims[0], dims[1], dims[2]);
    }

    MPI_Bcast(image.data, image.total() * image.elemSize(), MPI_BYTE, 0, comm);
}


// print the ASCII art in order to the console
void gather_and_print_to_console(const std::string& processed_string, int num_ranks, int my_rank, bool print_flag, bool colored_flag, const std::string& output_filepath) {
    // Buffer to gather all strings
    std::vector<char> full_output;

    // Size of each local string
    int local_size = processed_string.size();

    // Gather sizes first to prepare buffer on rank 0
    std::vector<int> sizes(num_ranks, 0);
    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Displacements for gather
    std::vector<int> displacements(num_ranks, 0);

    if (my_rank == 0) {
        // Compute total size and displacements
        int total_size = 0;
        for (int i = 0; i < num_ranks; ++i) {
            displacements[i] = total_size;
            total_size += sizes[i];
        }
        full_output.resize(total_size);
    }

    // Gather all strings to rank 0
    MPI_Gatherv(processed_string.data(), local_size, MPI_CHAR,
                full_output.data(), sizes.data(), displacements.data(), MPI_CHAR, 0, MPI_COMM_WORLD);

    // Print the ASCII art in order to the console
    if (my_rank == 0 && print_flag) {
        std::cout << "\n";
        std::cout.write(full_output.data(), full_output.size());
        std::string color_output_string = (colored_flag) ? "_color" : "";
        std::cout << "\nASCII art saved to outputs/" << output_filepath << ".txt and outputs/" << output_filepath << color_output_string << ".png\n";
        std::cout.flush();
    }
}

// gather the ASCII art from all ranks and save it to a file
void gather_and_save_ascii_art(cv::Mat &ascii_image, int rank, int size, const std::string &output_filepath, bool colored_flag)
{
    // total number of bytes in the ascii_image
    int send_count = ascii_image.total() * ascii_image.elemSize();
    std::vector<int> recvcounts(size);
    std::vector<int> displs(size);

    // Gather all sendcounts to calculate total size and displacements
    MPI_Gather(&send_count, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    cv::Mat full_image;

    if (rank == 0)
    {
        int total_size = 0;
        for (int i = 0; i < size; ++i)
        {
            displs[i] = total_size;
            total_size += recvcounts[i];
        }

        // Allocate the full_image to hold all gathered data
        full_image.create(ascii_image.rows * size, ascii_image.cols, ascii_image.type());

        // Gather all images into full_image
        MPI_Gatherv(ascii_image.data, send_count, MPI_UNSIGNED_CHAR,
                    full_image.data, recvcounts.data(), displs.data(),
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        // Save the full image at rank 0
        std::string color_output_string = (colored_flag) ? "_color" : "";
        cv::imwrite("outputs/" + output_filepath + color_output_string + ".png", full_image);
    }
    else
    {
        MPI_Gatherv(ascii_image.data, send_count, MPI_UNSIGNED_CHAR,
                    nullptr, nullptr, nullptr,
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    }
}

// write the ASCII art to a file using MPI I/O
void write_ascii_art_to_file(const std::string &ascii_art, const std::string &output_filepath_txt, MPI_Comm comm, int my_rank, MPI_Offset initial_offset)
{
    MPI_File fh;
    MPI_Status status;
    MPI_Offset offset = 0;

    // Calculate offset for ASCII art based on header size
    int local_size = ascii_art.size();
    MPI_Exscan(&local_size, &offset, 1, MPI_INT, MPI_SUM, comm);

    if (my_rank == 0) {
        offset = initial_offset;
    } else {
        offset += initial_offset;
    }

    #ifdef TIME
    uint64_t start_time = clock_now();
    #endif
    // Open the file collectively
    MPI_File_open(comm, output_filepath_txt.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    // Write ASCII art at calculated offset
    MPI_File_write_at_all(fh, offset, ascii_art.c_str(), local_size, MPI_CHAR, &status);

    // Close the file
    MPI_File_close(&fh);

    #ifdef TIME
    uint64_t end_time = clock_now();
    uint64_t cycles_spent = end_time - start_time;

    if (my_rank == 0) {
        std::cout << "MPI I/O operation took " << cycles_spent << " cycles." << std::endl;
    }
    #endif
}

int main(int argc, char **argv)
{
    // Initialize MPI
    int my_rank, num_ranks;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    #ifdef TIME
    double start_time = 0.0, end_time = 0.0;

    if (my_rank == 0)
    {
        start_time = MPI_Wtime();
    }
    #endif

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
    int thread_count = 0;

    if (my_rank == 0)
    {
        parse_arguments(argc, argv, input_filepath, output_filepath, executable_name, resize_flag, desired_width, print_flag, negate_flag, colored_flag, help_flag, thread_count);

        // Reverse the characters used for ASCII art if negate_flag is set
        if (negate_flag)
        {
            reverse_string(CHARACTERS);
        }

        // Check if the input file exists
        check_file_exist(input_filepath);
    }

    std::pair<int, int> threads = calculate_thread_dimensions(thread_count);
    int threads_x = threads.first;
    int threads_y = threads.second;

    // Broadcast the configuration to all ranks
    broadcast_config(input_filepath, output_filepath, resize_flag, desired_width, print_flag, negate_flag, colored_flag, help_flag, my_rank, CHARACTERS, threads_x, threads_y);

    MPI_Barrier(MPI_COMM_WORLD);

    if (help_flag)
    {
        MPI_Finalize();
        exit(EXIT_SUCCESS);
    }

    int desired_height = 0;
    cv::Mat input_image;

    map_rank_to_gpu(my_rank);

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
    std::pair<std::string, cv::Mat> process_output = process_image(subimage, colored_flag, threads_x, threads_y);
    std::string processed_string = process_output.first;
    cv::Mat processed_image = process_output.second;

    // Combine the ASCII art from all ranks into a single string and print it to the console
    gather_and_print_to_console(processed_string, num_ranks, my_rank, print_flag, colored_flag, output_filepath);

    // Gather the ASCII art from all ranks and save it to a file
    gather_and_save_ascii_art(processed_image, my_rank, num_ranks, output_filepath, colored_flag);

    // Write the header to MPI I/O file
    std::string output_filepath_txt = "outputs/" + output_filepath + ".txt";
    MPI_Offset initial_offset = 0;
    std::string header;
    if (my_rank == 0) {
        header += "mpirun -np " + std::to_string(num_ranks) + " ";
        header += std::string(argv[0]) + " ";
        for (int i = 1; i < argc; ++i) {
            header += std::string(argv[i]) + " ";
        }
        header += "\n";
        header += "Dimensions: " + std::to_string(desired_width) + "x" + std::to_string(desired_height) + "\n\n";

        MPI_File fh;
        MPI_Status status;
        MPI_File_open(MPI_COMM_SELF, output_filepath_txt.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
        MPI_File_write(fh, header.c_str(), header.size(), MPI_CHAR, &status);
        MPI_File_close(&fh);

        initial_offset = header.size();
    }

    MPI_Bcast(&initial_offset, 1, MPI_OFFSET, 0, MPI_COMM_WORLD);
    
    // Write the ASCII art to a file
    write_ascii_art_to_file(processed_string, output_filepath_txt, MPI_COMM_WORLD, my_rank, initial_offset);

    #ifdef TIME
    if (my_rank == 0)
    {
        end_time = MPI_Wtime();
        std::cout << "Total execution time: " << end_time - start_time << " seconds" << std::endl;
    }
    #endif

    MPI_Finalize();

    return EXIT_SUCCESS;
}