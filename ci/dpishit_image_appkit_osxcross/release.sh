#!/bin/bash

echo "please clean docker"

sudo rm -rf dpishit_bin_v* log
sudo docker rm dpishit_container_appkit_osxcross
sudo docker rmi dpishit_image_appkit_osxcross
sudo docker rmi alpine:edge

sudo ./build.sh
sudo ./run.sh /scripts/build_appkit.sh release appkit osxcross
sudo ./artifact.sh

sudo chown -R $(id -un):$(id -gn) dpishit_bin_v*
