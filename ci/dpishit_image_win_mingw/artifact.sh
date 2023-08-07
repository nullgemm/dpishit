#!/bin/bash

tag=$(git tag --sort v:refname | tail -n 1)
release=dpishit_bin_"$tag"

docker cp dpishit_container_win_mingw:/dpishit/"$release" .
