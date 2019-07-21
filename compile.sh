#!/bin/bash
g++ `pkg-config --cflags opencv4` -o playmp4 play_movie.cpp -g `pkg-config --libs opencv4`
