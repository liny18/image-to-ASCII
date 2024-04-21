```
Pre-req: CMake,C++14, CUDA, MPI, OpenCV, Python(Optional)
1. Run make
2. Usage: ./image_to_ascii -i <FILE> [Options]
Options:
    -i, --input  <FILE>     Specify the path of the input image filepath (required)
    -o, --output <FILE>     Specify the path of the output filepath without the file extension (eg. images/image)
    -w, --width  <INT>      Set the width of the ASCII output; maintains aspect ratio
    -s, --chars  <STRING>   Define the set of characters used in the ASCII output
    -p, --print             Print the ASCII output to the console
    -r, --reverse           Reverse the character set used for ASCII output(This option usually results in better PNGs)
    -f, --factor  <FLOAT>   Set the scale factor from 0.1 to 1.0 (default) to resize the image
    -c, --color             Get ASCII PNGs in colors
3. (Optional)Run 'python generateImage.py <num>'
    There are 4 pre-generated images. However, if you wish to get new images, run this command, but make sure you have an Unsplash API key.
```
