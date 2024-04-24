#!/bin/bash

docker run -v "$(pwd)"/outputs:/app/outputs -it image_to_ascii /bin/bash -c "$*"
