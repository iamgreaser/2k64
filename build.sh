#!/bin/sh

cc -O1 -g --pipe -o 2k64 src/*.c src/rdp/*.c -Isrc -lm `sdl2-config --cflags --libs` &&
true

