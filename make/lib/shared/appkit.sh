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
link+=("-framework")
link+=("AppKit")
link+=("-framework")
link+=("QuartzCore")

# parse soname
soname="$folder_library/appkit/dpishit_appkit.dylib"

# extract objects from static archives
cd "$folder_objects"
ar -x "../../$folder_library/appkit/dpishit_appkit_native.a"
ar -x "../../$folder_library/dpishit_macho_native.a"
cd ../..

# build shared object
clang -shared -o $soname $folder_objects/*.o "${link[@]}"
