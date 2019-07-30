cwiidtest:
	gcc -L/home/pi/ili9340spi_rpi/cwiid/libcwiid cwiidtest.c -o cwiidtest -lcwiid -lbluetooth

all:
	gcc -L/home/pi/ili9340spi_rpi/cwiid/libcwiid -I.. tetris.c ../fontx.c ../ili9340.c -lbcm2835  -lcwiid -lbluetooth -lm -DBCM -o game
