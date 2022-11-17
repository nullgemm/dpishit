#!/bin/bash

# example for a release build
# $ ./run.sh /scripts/build_x11.sh release x11
docker run --name dpishit_container_x11_gcc dpishit_image_x11_gcc "$@" &> log
