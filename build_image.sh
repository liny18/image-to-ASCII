#!/bin/bash
USE_CUDA=$1

if [ "$USE_CUDA" = "1" ]; then
    BASE_IMAGE="nvidia/cuda:12.4.1-devel-ubuntu20.04"
else
    BASE_IMAGE="ubuntu:20.04"
fi

docker build --build-arg BASE_IMAGE=$BASE_IMAGE --build-arg USE_CUDA=$USE_CUDA -t image_to_ascii .