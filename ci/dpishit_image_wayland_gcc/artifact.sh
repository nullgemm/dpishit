#!/bin/bash

tag=$(git tag --sort v:refname | tail -n 1)
release=dpishit_bin_"$tag"

docker cp dpishit_container_wayland_gcc:/dpishit/"$release" .
