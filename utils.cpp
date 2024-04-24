#include "utils.hpp"
#include "constants.hpp"

using namespace constants;

// Check if a directory exists
bool directory_exists(const std::string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Create a directory
bool create_directory(const std::string &path)
{
    return (mkdir(path.c_str(), 0777) == 0);
}

// Check if a file exists
bool file_exists(const std::string &name)
{
    return (access(name.c_str(), F_OK) != -1);
}

// Remove a file
bool remove_file(const std::string &name)
{
    return (remove(name.c_str()) == 0);
}

// need to make sure we're writing to a new file and directory
void check_file_exist(const std::string &input_filepath)
{
    std::string input_filename = get_basename(input_filepath);

    if (!directory_exists("outputs"))
    {
        create_directory("outputs");
    }

    if (file_exists("outputs/" + input_filename + ".txt"))
    {
        remove_file("outputs/" + input_filename + ".txt");
    }
}

// reverse the characters used for ASCII art. This is used to get the negative of the ASCII image
void reverse_string(std::string &str)
{
    std::reverse(str.begin(), str.end());
}

// display the usage of the program
void show_usage(const std::string &executable_name)
{
    std::cerr << "\nUsage: 'mpirun -np <INT>" << executable_name << " [options]' \n\n";
    std::cerr << "Options:\n"
                 "  -h, --help              Display this help message\n"
                 "  -i, --input  <FILE>     Specify the path of the input image FILE (required)\n"
                 "  -o, --output <STRING>   Specify the name of the output file (e.g. 'output')\m"
                 "  -w, --width  <INT>      Set the width of the ASCII output; maintains aspect ratio\n"
                 "  -s, --chars  <STRING>   Define the set of characters used in the ASCII output\n"
                 "  -p, --print             Print the ASCII output to the console\n"
                 "  -n, --negate            Get the negative of the ASCII image\n"
                 "  -f, --factor  <FLOAT>   Set the scale factor from 0.1 to 1.0 (default) to resize the image\n"
                 "  -c, --color             Get ASCII PNG's in colors\n"
                 "  -t, --threads <INT>     Set the number of threads to use, default is 256\n\n";
    std::cerr << "Example: 'mpirun -np 4 " << executable_name << " -i images/your_image.png -w 90 -c -p'\n\n";
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

// calculate the dimensions of the block that multiply to the total number of threads
std::pair<int, int> calculate_thread_dimensions(int thread_count)
{
    int x = static_cast<int>(std::sqrt(thread_count));
    while (thread_count % x != 0)
    {
        --x;
    }
    int y = thread_count / x;
    return {x, y};
}

// parse the command line arguments and set the configuration
void parse_arguments(int argc, char **argv, std::string &input_filepath, std::string &output_filepath, std::string &executable_name, bool &resize_flag, int &desired_width, bool &print_flag, bool &negate_flag, bool &colored_flag, bool &help_flag, int &thread_count)
{
    struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"input", required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"width", required_argument, nullptr, 'w'},
        {"chars", required_argument, nullptr, 's'},
        {"print", no_argument, nullptr, 'p'},
        {"negate", no_argument, nullptr, 'n'},
        {"factor", required_argument, nullptr, 'f'},
        {"color", no_argument, nullptr, 'c'},
        {"threads", required_argument, nullptr, 't'},
        {0, 0, 0, 0}};

    int option;
    const char *short_options = "hi:o:w:s:pnf:ct:";
    while ((option = getopt_long(argc, argv, short_options, long_options, nullptr)) != EOF)
    {
        switch (option)
        {
        case 'h':
            show_usage(executable_name);
            help_flag = true;
            break;
        case 'i':
            input_filepath = optarg;
            break;
        case 'o':
            output_filepath = optarg;
            break;
        case 'w':
            desired_width = std::atoi(optarg);
            resize_flag = true;
            break;
        case 's':
            CHARACTERS = optarg;
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
        case 't':
            thread_count = std::atoi(optarg);
            break;
        default:
            show_usage(executable_name);
        }
    }

    // make sure all inputs numbers are valid
    if (desired_width <= 0 && resize_flag)
    {
        std::cerr << "Error: The width must be a positive integer.\n";
        help_flag = true;
    }

    if (SCALE_FACTOR < 0.1 || SCALE_FACTOR > 1.0)
    {
        std::cerr << "Error: The scale factor must be between 0.1 and 1.0.\n";
        help_flag = true;
    }

    if (output_filepath.empty())
    {
        output_filepath = get_basename(input_filepath);
    }

    if (thread_count == 0)
    {
        thread_count = 256;
    }
}