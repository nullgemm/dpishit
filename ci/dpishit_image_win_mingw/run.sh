#!/bin/bash

# example for a release build
# $ ./run.sh /scripts/build_win.sh release win
docker run --name dpishit_container_win_mingw dpishit_image_win_mingw "$@" &> log
