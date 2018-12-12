#!/bin/sh

cc -O3 -flto -g -o 2k64 src/*.c src/rdp/*.c -Isrc -lm `sdl2-config --cflags --libs` &&
true

