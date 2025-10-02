#!/bin/bash

# get into the script's folder
cd "$(dirname "$0")" || exit
cd ../../..

# utilitary variables
tag=$(git tag --sort v:refname | tail -n 1)
folder_ninja="build"
folder_objects="$folder_ninja/shared"
folder_dpishit="dpishit_bin_$tag"
folder_library="$folder_dpishit/lib/dpishit"
mkdir -p "$folder_objects"

# list link flags (order matters)
link+=("-lshcore")
link+=("-lgdi32")
link+=("-ldwmapi")

# list objs (order matters)
obj+=("$folder_objects/dpishit_win.o")
obj+=("$folder_objects/dpishit_pe.o")

# parse soname
soname="$folder_library/win/dpishit_win.dll"

# extract objects from static archives
ar --output "$folder_objects" -x "$folder_library/win/dpishit_win.a"
ar --output "$folder_objects" -x "$folder_library/dpishit_pe.a"

# build shared object
x86_64-w64-mingw32-gcc -shared -o $soname "${obj[@]}" "${link[@]}"
