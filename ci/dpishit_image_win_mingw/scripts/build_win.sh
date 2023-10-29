#!/bin/sh

git clone https://github.com/nullgemm/dpishit.git
cd ./dpishit || exit

# test build
./make/scripts/build.sh "$@"
