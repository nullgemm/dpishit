#!/bin/bash

docker run --name dpishit_container_gcc dpishit_image_gcc "$@" &> log
