#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <getopt.h>
#include <vector>

// Check if a directory exists
bool directory_exists(const std::string &path);

// Create a directory
bool create_directory(const std::string &path);

// Check if a file exists
bool file_exists(const std::string &name);

// Remove a file
bool remove_file(const std::string &name);

// Need to make sure we're writing to a new file and directory
void check_file_exist(const std::string &input_filepath);

// Get the list of image files in a directory
void get_image_files(const std::string &path, std::vector<std::string> &files);

// Reverse the characters used for ASCII art. This is used to get the negative of the ASCII image
void reverse_string(std::string &str);

// Display the usage of the program
void show_usage(const std::string &executable_name);

// Get the basename of a file, used to match the output file name with the input file name
std::string get_basename(const std::string &full_path);

// Parse the command line arguments and set the configuration
void parse_arguments(int argc, char **argv, std::string &input_filepath, std::string &output_filepath, std::string &executable_name, bool &resize_flag, int &desired_width, bool &print_flag, bool &negate_flag, bool &colored_flag, bool &help_flag);

#endif // __UTILS_HPP__