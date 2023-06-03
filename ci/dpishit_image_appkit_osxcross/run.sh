#!/bin/bash

# example for a release build
# $ ./run.sh /scripts/build_appkit.sh release appkit osxcross
docker run \
	--privileged \
	--name dpishit_container_appkit_osxcross \
	-e AR=x86_64-apple-darwin20.2-ar \
	dpishit_image_appkit_osxcross "$@" &>> log
