cwiidtest:
	gcc -L/home/pi/ili9340spi_rpi/cwiid/libcwiid cwiidtest.c -o cwiidtest -lcwiid -lbluetooth

all:
	gcc -I.. tetris.c ../fontx.c ../ili9340.c -lbcm2835 -lm -DBCM -o game
