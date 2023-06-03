#!/bin/bash

echo "please clean docker"

sudo rm -rf dpishit_bin_v* log
sudo docker rm dpishit_container_x11_gcc
sudo docker rmi dpishit_image_x11_gcc
sudo docker rmi alpine:edge

sudo ./build.sh
sudo ./run.sh /scripts/build_x11.sh release x11 native
sudo ./artifact.sh

sudo chown -R $(id -un):$(id -gn) dpishit_bin_v*
