#!/bin/bash
##g++ `pkg-config --cflags opencv4` -o playmp4 play_movie.cpp -g `pkg-config --libs opencv4`
g++ `pkg-config --cflags opencv4` play_movie.cpp ili9340formovie.c -std=c++11 -g `pkg-config --libs opencv4` -lbcm2835 -lm -DBCM -o playmp4
