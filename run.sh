#!/bin/bash

# Check if any command was provided
if [ "$#" -eq 0 ]; then
    echo "Usage: $0 <command>"
    echo "Example: $0 'mpirun -np 1 ./out -h'"
    exit 1
fi

# Join all input arguments into one command string
COMMAND="$*"

# Define the local directory for images and outputs
IMAGE_DIR="$(pwd)/images"
OUTPUT_DIR="$(pwd)/outputs"

# Ensure the image and output directories exist
mkdir -p "$IMAGE_DIR"
mkdir -p "$OUTPUT_DIR"

# Run the Docker command
docker run -v "$OUTPUT_DIR":/app/outputs -v "$IMAGE_DIR":/app/images -it yuxlin21/image-to-ascii /bin/bash -c "$COMMAND"
