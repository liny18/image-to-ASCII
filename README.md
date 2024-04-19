```cpp
1. Run make
2. Usage: ./image_to_ascii -i <DIRECTORY> [Options]

Options:
    -i, --input  <DIRECTORY>     Specify the path of the input image directory (required)
    -w, --width  <NUMBER>   Set the width of the ASCII output; maintains aspect ratio
    -s, --chars  <STRING>   Define the set of characters used in the ASCII output
    -p, --print             Print the ASCII output to the console
    -r, --reverse           Reverse the character set used for ASCII output(This option usually results in better PNGs)
    -f, --factor            Set the scale factor to resize the image(0.0 - 1.0)
    -c, --color             Get ASCII PNGs in colors
3. (Optional)Run 'Python3 generateImage.py <num>'
    There are 5 pre-generated images. However, if you wish to get new images, run this command, but make sure you have an Unsplash API key.
```