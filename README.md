## Install

## Usage :clipboard:
1. Command: 'mpirun -np <INT> (number of processes) ./image_to_ascii -i <FILE> [Options]'
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
Note:
    Running with CUDA, OpenCV generates many warnings, but they do not affect the usability of the program, safely ignore.

## Examples :eyes:

|                     Input                     |                                 Output                                 |
|-----------------------------------------------|------------------------------------------------------------------------|
| <img src="images/cpp.png" width="300">        | <img src="outputs/cpp_color.png" width="300">                          |
| command:                                      | `mpirun -np 4 ./image_to_ascii -i images/cpp.png -c -w 150`            |
| <img src="images/hwoarang.png" width="300">   | <img src="outputs/hwoarang_color.png" width="300">                     |
| command:                                      | `mpirun -np 4 ./image_to_ascii -i images/hwoarang.png -c -w 150`       |
| <img src="images/pikachu.png" width="300">    | <img src="outputs/pikachu_color.png" width="300">                      |
| command:                                      | `mpirun -np 4 ./image_to_ascii -i images/pikachu.png -c -w 150`        |
| <img src="images/miffy.png" width="300">      | <img src="outputs/miffy_color.png" width="300">                        |
| command:                                      | `mpirun -np 4 ./image_to_ascii -i images/miffy.png -c -w 150`          |
