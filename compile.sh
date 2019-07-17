#!/bin/bash
cc -o demo demo.c fontx.c ili9340.c -lbcm2835 -lm -DBCM
