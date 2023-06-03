#!/bin/sh

git clone https://github.com/nullgemm/dpishit.git
cd ./dpishit || exit

# TODO remove
git checkout next

# test build
./make/scripts/build.sh "$@"
