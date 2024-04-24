#!/bin/bash

usage() {
    echo "Usage: $0 [USE_CUDA]"
    echo "  USE_CUDA: Specify 1 to use CUDA, any other value defaults to no CUDA."
    exit 1
}

if [ "$#" -ne 1 ]; then
    usage
fi

USE_CUDA=${1:-0}

# Set the base image based on whether CUDA is enabled
if [ "$USE_CUDA" = "1" ]; then
    BASE_IMAGE="nvidia/cuda:12.4.1-devel-ubuntu20.04"
else
    BASE_IMAGE="ubuntu:20.04"
fi

docker build --build-arg BASE_IMAGE=$BASE_IMAGE --build-arg USE_CUDA=$USE_CUDA -t image-to-ascii .
