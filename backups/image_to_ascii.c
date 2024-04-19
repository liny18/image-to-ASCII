#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"
#include "stb_image/stb_image_resize.h"

char characters[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

void reverse_string(char *str)
{
    char *end = str + strlen(str) - 1; // Point to the last character of the string
    while (str < end)
    {
        char temp = *str;
        *str++ = *end;
        *end-- = temp;
    }
}

void show_usage(char *executable_name)
{
    printf("\nUsage: .\\%s [options] -i <FILE> [-o <FILE>]\n\n", executable_name);
    printf("Options:\n"
           "  -i, --input  <FILE>     Specify the path of the input image file (required)\n"
           "  -o, --output <FILE>     Specify the path where the ASCII output will be saved\n"
           "  -w, --width  <NUMBER>   Set the width of the ASCII output; maintains aspect ratio\n"
           "  -c, --chars  <STRING>   Define the set of characters used in the ASCII output\n"
           "  -p, --print             Print the ASCII output to the console instead of a file\n"
           "  -r, --reverse           Reverse the character set used for ASCII output\n\n");
    exit(EXIT_FAILURE);
}

char *get_basename(const char *full_path)
{
    const char *last_slash = strrchr(full_path, '/');
    if (last_slash == NULL)
        last_slash = full_path; // No slash found
    else
        last_slash++; // Move past the last slash

    const char *last_dot = strrchr(last_slash, '.');
    size_t length = (last_dot != NULL) ? (last_dot - last_slash) : strlen(last_slash);

    char *filename = malloc(length + 5); // +4 for ".txt" + 1 for '\0'
    if (filename != NULL)
    {
        snprintf(filename, length + 5, "%.*s.txt", (int)length, last_slash);
    }

    return filename;
}

int main(int argc, char **argv)
{
    char *executable_name = argv[0];

    // Exit if no command line arguments are given

    if (argc <= 1)
    {
        show_usage(executable_name);
    }

    // Parse arguments from the command line

    char *input_filepath = NULL;
    char *output_filepath = NULL;
    bool print_flag = false;
    bool reverse_flag = false;
    bool resize_image = false;
    int desired_width = 0;

    struct option long_options[] =
        {
            {"help", no_argument, NULL, 'h'},
            {"input", required_argument, NULL, 'i'},
            {"output", required_argument, NULL, 'o'},
            {"width", required_argument, NULL, 'w'},
            {"chars", required_argument, NULL, 'c'},
            {"print", no_argument, NULL, 'p'},
            {"reverse", no_argument, NULL, 'r'},
            {0, 0, 0, 0}};

    int option;
    const char *short_options = "hi:o:w:c:pr";

    while ((option = getopt_long(argc, argv, short_options, long_options, NULL)) != EOF)
    {
        switch (option)
        {
        case 'h':
            show_usage(executable_name);

        case 'i':
            input_filepath = optarg;
            break;

        case 'o':
            output_filepath = optarg;
            break;

        case 'w':
            desired_width = atoi(optarg);
            resize_image = true;
            break;

        case 'c':
            strncpy(characters, optarg, sizeof(characters) - 1);
            characters[sizeof(characters) - 1] = '\0';
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

    if (output_filepath == NULL)
    {
        output_filepath = get_basename(input_filepath);
    }

    // Load the image in grayscale

    int width, height;
    unsigned char *image = NULL;

    image = stbi_load(input_filepath, &width, &height, NULL, STBI_grey);

    if (image == NULL)
    {
        fprintf(stderr, "ERROR: could not load image \n");
        exit(EXIT_FAILURE);
    }

    // Resize the image

    int desired_height;

    if (resize_image)
    {
        if (desired_width <= 0)
        {
            fprintf(stderr, "ERROR: argument 'width' must be greater than 0 \n");
            exit(EXIT_FAILURE);
        }

        // Calculate the new height to maintain the aspect ratio
        desired_height = height * desired_width / width;

        // Allocate memory for the resized image
        unsigned char *resized_image = calloc(desired_width * desired_height, sizeof(unsigned char));
        if (resized_image == NULL)
        {
            fprintf(stderr, "ERROR: failed to allocate memory for the resized image\n");
            stbi_image_free(image);
            exit(EXIT_FAILURE);
        }

        // Resize the image whether it is upscaling or downscaling
        stbir_resize_uint8(image, width, height, 0, resized_image, desired_width, desired_height, 0, STBI_grey);

        // Free the original image and replace it with the resized image
        stbi_image_free(image);
        image = resized_image;
        width = desired_width;
        height = desired_height;
    }
    else
    {
        desired_width = width;
        desired_height = height;
    }

    // Create an output file

    FILE *file_pointer = NULL;

    file_pointer = fopen(output_filepath, "w");

    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR: could not create an output file \n");
        exit(EXIT_FAILURE);
    }

    if (reverse_flag)
    {
        reverse_string(characters);
    }

    // Write some useful information to the file

    fprintf(file_pointer, "%s", executable_name);

    for (int i = 1; i < argc; i++)
    {
        fprintf(file_pointer, "%s ", argv[i]);
    }

    int characters_count = strlen(characters);

    fprintf(
        file_pointer,
        "\n\n"
        "Input: %s \n"
        "Output: %s \n"
        "Resolution: %dx%d \n"
        "Characters (%d): \"%s\" \n\n",
        input_filepath, output_filepath, desired_width, desired_height, characters_count, characters);

    // Write the ASCII art to the file

    for (int i = 0; i < desired_height * desired_width; i++)
    {
        int intensity = image[i];

        int character_index = intensity / (255 / (float)(characters_count - 1));

        if (print_flag)
        {
            putchar(characters[character_index]);
        }
        fputc(characters[character_index], file_pointer);

        if ((i + 1) % desired_width == 0)
        {
            if (print_flag)
            {
                putchar('\n');
            }
            fputc('\n', file_pointer);
        }
    }

    printf("\nSaved output to '%s' \n", output_filepath);

    // Free memory
    free(output_filepath);
    fclose(file_pointer);
    if (image)
    {
        stbi_image_free(image);
    }

    return EXIT_SUCCESS;
}