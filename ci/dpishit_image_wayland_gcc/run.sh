#!/bin/bash

# example for a release build
# $ ./run.sh /scripts/build_wayland.sh release wayland
docker run --name dpishit_container_wayland_gcc dpishit_image_wayland_gcc "$@" &> log
