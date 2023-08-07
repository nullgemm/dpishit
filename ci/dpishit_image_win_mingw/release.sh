#!/bin/bash

echo "please clean docker"

sudo rm -rf dpishit_bin_v* log
sudo docker rm dpishit_container_win_mingw
sudo docker rmi dpishit_image_win_mingw
sudo docker rmi alpine:edge

sudo ./build.sh
sudo ./run.sh /scripts/build_win.sh release win native
sudo ./artifact.sh

sudo chown -R $(id -un):$(id -gn) dpishit_bin_v*
