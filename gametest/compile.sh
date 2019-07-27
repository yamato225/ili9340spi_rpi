#!/bin/bash
g++ -I.. $1 ../fontx.c ../ili9340.c -lbcm2835 -lm -DBCM -o game
