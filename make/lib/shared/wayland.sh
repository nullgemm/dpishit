#!/bin/bash

ar -x dpishit_wayland.a
ar -x ../dpishit_elf.a
gcc -shared -o dpishit_wayland.so *.o -lwayland-client -lwayland-cursor -lxkbcommon -lpthread
