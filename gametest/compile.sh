#!/bin/bash
gcc -I.. game.c ../fontx.c ../ili9340.c -lbcm2835 -lm -DBCM -o game
