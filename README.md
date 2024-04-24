# Image to ASCII Converter
Transform your images into ASCII art. Leveraging MPI and optional CUDA support, this application executes parallel processing to ensure both speed and efficiency in image conversion.

## Installation
If you do not have Docker installed, install at [Docker🐳](https://docs.docker.com/engine/install/)

## Usage :clipboard:
### Pull the Docker Image
- For systems with CUDA support (ensure your environment is CUDA-capable):
```shell
docker pull yuxlin21/image-to-ascii:cuda
```
- For systems without CUDA support:
```shell
docker pull yuxlin21/image-to-ascii:latest
```

### Preparing Your Images
1. Create an Image Directory:
   
Create a directory on your host machine to store the images you want to convert.
```shell
mkdir images
```
2. Add Images:
   
Place the images you want to convert into this directory.

### Running The Application
- Template for running
```shell
docker run -v "$(pwd)"/outputs:/app/outputs -v "$(pwd)"/images:/app/images -it yuxlin21/image-to-ascii:[latest/cuda] '[COMMAND]'
```
- Choose either latest or cuda depending on your image for [latest/cuda]
- Replace '[COMMAND]' with these options:
```
Command: 'mpirun -np <INT> (number of processes) ./out [Options]'
Options:
    -h, --help              Display this help message
    -i, --input  <FILE>     Specify the path of the input image filepath (required)
    -o, --output <STRING>   Specify the name of the output file name (e.g. image)
    -w, --width  <INT>      Set the width of the ASCII output; maintains aspect ratio
    -s, --chars  <STRING>   Define the set of characters used in the ASCII output
    -p, --print             Print the ASCII output to the console
    -n, --negate            Create a negative ASCII art
    -f, --factor <FLOAT>    Set the scale factor from 0.1 to 1.0 (default) to resize the image
    -c, --color             Get ASCII PNGs in colors
    -t, --threads <INT>     Set the number of threads to use, default is 256  
```
- Example
```shell
docker run -v "$(pwd)"/outputs:/app/outputs -v "$(pwd)"/images:/app/images -it yuxlin21/image-to-ascii:latest 'mpirun -np 1 ./out -i images/hwoarang.png -w 90 -c -p'
```

Note:
    When running with CUDA, OpenCV generates many warnings, but they do not affect the usability of the program, safely ignore.

## Examples :eyes:

|                     Input                     |                                 Output                                 |
|-----------------------------------------------|------------------------------------------------------------------------|
| <img src="images/cpp.png" width="300">        | <img src="outputs/cpp_color.png" width="300">                          |
| command:                                      | `mpirun -np 4 ./out -i images/cpp.png -c -w 150`            |
| <img src="images/hwoarang.png" width="300">   | <img src="outputs/hwoarang_color.png" width="300">                     |
| command:                                      | `mpirun -np 4 ./out -i images/hwoarang.png -c -w 150`       |
| <img src="images/pikachu.png" width="300">    | <img src="outputs/pikachu_color.png" width="300">                      |
| command:                                      | `mpirun -np 4 ./out -i images/pikachu.png -c -w 150`        |
| <img src="images/miffy.png" width="300">      | <img src="outputs/miffy_color.png" width="300">                        |
| command:                                      | `mpirun -np 4 ./out -i images/miffy.png -c -w 150`          |
