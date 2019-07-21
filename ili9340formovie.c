// ili9340.c
// Used by bcm2835 library.(Mike McCauley)
//
// Original is http://imagewriteriij.blogspot.jp/2014/01/raspberry-pi-9-lcd-1.html
// LCD test program 20130103 by ImageWriter
//
// [Pin connection]
// ILI9340-SPI     Rpi(pin)
// ------------------------
// MISO------------MISO(21)
// LED---220ohm----3.3V( 1)
// SCK-------------SCLK(23)
// MOSI------------MOSI(19)
// D/C-------------IO02( 3)  LOW = 0 = COMMAND
// RES-------------IO03( 5)  LOW = 0 = RESET
// CS--------------CS0 (24)  LOW = 0 = Chip Select
// GND-------------0V  ( 6)
// VCC-------------3.3V( 1)
// ------------------------
//  
// [SPI settings]
// ORDER  : MSB First
// MODE   : Mode0
// CLOCK  : 31.25MHz on Rpi/Rpi2, 50MHz on RPI3 
// CS     : CS0
// CS_POL : LOW
//
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#ifdef WPI
#include <wiringPi.h>
#include <wiringPiSPI.h>
#endif
#ifdef BCM
#include <bcm2835.h>
#endif

#include "ili9340formovie.h"

#define D_C  2  // GPIO2=Pin#3
#define RES  3  // GPIO3=Pin#5
#define C_S  8  // GPIO8=Pin#24

#define _DEBUG_   0

uint16_t _FONT_DIRECTION_;
uint16_t _FONT_FILL_;
uint16_t _FONT_FILL_COLOR_;
uint16_t _FONT_UNDER_LINE_;
uint16_t _FONT_UNDER_LINE_COLOR_;

int _width;
int _height;
int _offsetx;
int _offsety;

#ifdef BCM
// Write Command
// D/C=LOW then,write command(8bit)
void lcdWriteCommandByte(uint8_t c){
  bcm2835_gpio_write(D_C, LOW);
  bcm2835_spi_transfer(c);
}

// Write Data 8Bit
// D/C=HIGH then,write data(8bit)
void lcdWriteDataByte(uint8_t c){
  bcm2835_gpio_write(D_C, HIGH);
  bcm2835_spi_transfer(c);
}

// Write Data 16Bit
void lcdWriteDataWord(uint16_t w){
  bcm2835_gpio_write(D_C, HIGH);
  bcm2835_spi_write(w);
}

// Write Tow Data 8Bit
void lcdWriteAddr(uint8_t addr1, uint8_t addr2){
  bcm2835_gpio_write(D_C, HIGH);
  bcm2835_spi_write(addr1);
  bcm2835_spi_write(addr2);
}

// Write Data 16Bit
void lcdWriteColor(uint16_t color, uint16_t size) {
  bcm2835_gpio_write(D_C, HIGH);
  int i;
  for(i=0;i<size;i++) bcm2835_spi_write(color);
}

#endif


#ifdef BCM
// SPI interfase initialize
// MSB,mode0,clock=8,cs0=low
void lcdInit(int width, int height, int offsetx, int offsety){
  _width = width;
  _height = height;
  _offsetx = offsetx;
  _offsety = offsety;
  if (bcm2835_init() == -1) {
    printf("bmc2835_init Error\n");
    return;
  }

  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
  // Send a byte to the slave and simultaneously read a byte back from the slave
  // If you tie MISO to MOSI, you should read back what was sent

  _FONT_DIRECTION_ = DIRECTION0;
  _FONT_FILL_ = false;
  _FONT_UNDER_LINE_ = false;

}

// TFT Reset
void lcdReset(void){
  bcm2835_gpio_fsel(D_C,BCM2835_GPIO_FSEL_OUTP); // D/C
  bcm2835_gpio_fsel(RES,BCM2835_GPIO_FSEL_OUTP); // Reset
  //bcm2835_gpio_write(D_C, HIGH);

  bcm2835_gpio_write(RES, LOW);
  bcm2835_delay(100);
  bcm2835_gpio_write(RES, HIGH);
  bcm2835_delay(100); 
}
#endif

// TFT initialize
void lcdSetup(void){
  lcdWriteCommandByte(0xC0);		//Power Control 1
  lcdWriteDataByte(0x23); 

  lcdWriteCommandByte(0xC1);		//Power Control 2
  lcdWriteDataByte(0x10);

  lcdWriteCommandByte(0xC5);		//VCOM Control 1
  lcdWriteDataByte(0x3e);
  lcdWriteDataByte(0x28); 

  lcdWriteCommandByte(0xC7);		//VCOM Control 2 
  lcdWriteDataByte(0x86);

  lcdWriteCommandByte(0x36);		//Memory Access Control 
  lcdWriteDataByte(0x48);		//Left bottom start

  lcdWriteCommandByte(0x3A);		//Pixel Format Set
  lcdWriteDataByte(0x55);		//65K color: 16-bit/pixel

  lcdWriteCommandByte(0x20);		//Display Inversion OFF

  lcdWriteCommandByte(0xB1);		//Frame Rate Control
  lcdWriteDataByte(0x00);  
  lcdWriteDataByte(0x18); 

  lcdWriteCommandByte(0xB6);		//Display Function Control 
  lcdWriteDataByte(0x08); 
  lcdWriteDataByte(0xA2);
  lcdWriteDataByte(0x27);  
  lcdWriteDataByte(0x00);  

  lcdWriteCommandByte(0xF2);		//3Gamma Function Disable 
  lcdWriteDataByte(0x00); 

  lcdWriteCommandByte(0x26);		//Gamma Set 
  lcdWriteDataByte(0x01); 

  lcdWriteCommandByte(0xE0);		//Positive Gamma Correction
  lcdWriteDataByte(0x0F); 
  lcdWriteDataByte(0x31); 
  lcdWriteDataByte(0x2B); 
  lcdWriteDataByte(0x0C); 
  lcdWriteDataByte(0x0E); 
  lcdWriteDataByte(0x08); 
  lcdWriteDataByte(0x4E); 
  lcdWriteDataByte(0xF1); 
  lcdWriteDataByte(0x37); 
  lcdWriteDataByte(0x07); 
  lcdWriteDataByte(0x10); 
  lcdWriteDataByte(0x03); 
  lcdWriteDataByte(0x0E); 
  lcdWriteDataByte(0x09); 
  lcdWriteDataByte(0x00); 

  lcdWriteCommandByte(0XE1);		//Negative Gamma Correction
  lcdWriteDataByte(0x00); 
  lcdWriteDataByte(0x0E); 
  lcdWriteDataByte(0x14); 
  lcdWriteDataByte(0x03); 
  lcdWriteDataByte(0x11); 
  lcdWriteDataByte(0x07); 
  lcdWriteDataByte(0x31); 
  lcdWriteDataByte(0xC1); 
  lcdWriteDataByte(0x48); 
  lcdWriteDataByte(0x08); 
  lcdWriteDataByte(0x0F); 
  lcdWriteDataByte(0x0C); 
  lcdWriteDataByte(0x31); 
  lcdWriteDataByte(0x36); 
  lcdWriteDataByte(0x0F); 

  lcdWriteCommandByte(0x11);			//Sleep Out 
#ifdef BCM
  bcm2835_delay(200); 
#endif
  
  lcdWriteCommandByte(0x29);			//Display ON 
}

// Draw pixel
// x:X coordinate
// y:Y coordinate
// color:color
void lcdDrawPixel(uint16_t x, uint16_t y, uint16_t color){
  if (x >= _width) return;
  if (y >= _height) return;

  uint16_t _x = x + _offsetx;
  uint16_t _y = y + _offsety;
  lcdWriteCommandByte(0x2A); // set column(x) address
  lcdWriteDataWord(_x);
  lcdWriteDataWord(_x);
  lcdWriteCommandByte(0x2B); // set Page(y) address
  lcdWriteDataWord(_y);
  lcdWriteDataWord(_y);
  lcdWriteCommandByte(0x2C); // Memory Write
  lcdWriteDataWord(color);
}

// Draw rectangule of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// color:color
void lcdDrawFillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  int i,j; 
  if (x1 >= _width) return;
  if (x2 >= _width) x2=_width-1;
  if (y1 >= _height) return;
  if (y2 >= _height) y2=_height-1;

  uint16_t _x1 = x1 + _offsetx;
  uint16_t _x2 = x2 + _offsetx;
  uint16_t _y1 = y1 + _offsety;
  uint16_t _y2 = y2 + _offsety;
  lcdWriteCommandByte(0x2A); // set column(x) address
  lcdWriteDataWord(_x1);
  lcdWriteDataWord(_x2);
  //lcdWriteAddr(_x1, _x2); // Don't work 
  lcdWriteCommandByte(0x2B); // set Page(y) address
  lcdWriteDataWord(_y1);
  lcdWriteDataWord(_y2);
  //lcdWriteAddr(_y1, _y2); // Don't work 
  lcdWriteCommandByte(0x2C); // Memory Write
  for(i=x1;i<=x2;i++){
    uint16_t size = y2-y1+1;
    lcdWriteColor(color, size);
#if 0
    for(j=y1;j<=y2;j++){
      lcdWriteDataWord(color);
    }
#endif
  }
}

void lcdDrawImage(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *image) {
  int i,j;
  if (x1 >= _width) return;
  if (x2 >= _width) x2=_width-1;
  if (y1 >= _height) return;
  if (y2 >= _height) y2=_height-1;

  uint16_t _x1 = x1 + _offsetx;
  uint16_t _x2 = x2 + _offsetx;
  uint16_t _y1 = y1 + _offsety;
  uint16_t _y2 = y2 + _offsety;
  lcdWriteCommandByte(0x2A); // set column(x) address
  lcdWriteDataWord(_x1);
  lcdWriteDataWord(_x2);
  //lcdWriteAddr(_x1, _x2); // Don't work 
  lcdWriteCommandByte(0x2B); // set Page(y) address
  lcdWriteDataWord(_y1);
  lcdWriteDataWord(_y2);
  //lcdWriteAddr(_y1, _y2); // Don't work 
  lcdWriteCommandByte(0x2C); // Memory Write
  bcm2835_gpio_write(D_C, HIGH);
  printf("%d,%d,%d,%d",_x1,_x2,_y1,_y2);
	for (i=0; i<160; i++){
        for (j=0; j<128; j++) {
            //fread(buf, 3, 1, f);
            bcm2835_spi_write(image[i*128+j]);
            //usleep(1000);
        }
  }
}



// Display Off
void lcdDisplayOff(void) {
  lcdWriteCommandByte(0x28);    //Display OFF
}
 
// Display On
void lcdDisplayOn(void) {
  lcdWriteCommandByte(0x29);    //Display ON 
}

// Display Inversion On
void lcdInversionOn(void) {
  lcdWriteCommandByte(0x21);    //Display Inversion ON 
}

// Fill screen
// color:color
void lcdFillScreen(uint16_t color) {
  lcdDrawFillRect(0, 0, _width-1, _height-1, color);
}

// Draw line
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// color:color 
void lcdDrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  int i;
  int dx,dy;
  int sx,sy;
  int E;

  /* distance between two points */
  dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
  dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

  /* direction of two point */
  sx = ( x2 > x1 ) ? 1 : -1;
  sy = ( y2 > y1 ) ? 1 : -1;

  /* inclination < 1 */
  if ( dx > dy ) {
    E = -dx;
    for ( i = 0 ; i <= dx ; i++ ) {
      lcdDrawPixel( x1, y1, color );
      x1 += sx;
      E += 2 * dy;
      if ( E >= 0 ) {
        y1 += sy;
        E -= 2 * dx;
      }
    }
  /* inclination >= 1 */
  } else {
    E = -dy;
    for ( i = 0 ; i <= dy ; i++ ) {
      lcdDrawPixel( x1, y1, color );
      y1 += sy;
      E += 2 * dx;
      if ( E >= 0 ) {
        x1 += sx;
        E -= 2 * dy;
      }
    }
  }
}

// Draw rectangule
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End   X coordinate
// y2:End   Y coordinate
// color:color
void lcdDrawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
  lcdDrawLine(x1,y1,x2,y1,color);
  lcdDrawLine(x2,y1,x2,y2,color);
  lcdDrawLine(x2,y2,x1,y2,color);
  lcdDrawLine(x1,y2,x1,y1,color);
}

// Draw round
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdDrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
  int x;
  int y;
  int err;
  int old_err;
  
  x=0;
  y=-r;
  err=2-2*r;
  do{
    lcdDrawPixel(x0-x,y0+y,color); 
    lcdDrawPixel(x0-y,y0-x,color); 
    lcdDrawPixel(x0+x,y0-y,color); 
    lcdDrawPixel(x0+y,y0+x,color); 
    if ((old_err=err)<=x)   err+=++x*2+1;
    if (old_err>y || err>x) err+=++y*2+1;    
  } while(y<0);
  
}

// Draw round of filling
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdDrawFillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
  int x;
  int y;
  int err;
  int old_err;
  int ChangeX;
  
  x=0;
  y=-r;
  err=2-2*r;
  ChangeX=1;
  do{
    if(ChangeX) {
      lcdDrawLine(x0-x,y0-y,x0-x,y0+y,color);
      lcdDrawLine(x0+x,y0-y,x0+x,y0+y,color);
    } // if
    ChangeX=(old_err=err)<=x;
    if (ChangeX)            err+=++x*2+1;
    if (old_err>y || err>x) err+=++y*2+1;
  } while(y<=0);
    
} 

// Draw rectangule with round corner
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End   X coordinate
// y2:End   Y coordinate
// r:radius
// color:color
void lcdDrawRoundRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t color) {
  int x;
  int y;
  int err;
  int old_err;
  unsigned char temp;

  if(x1>x2) {
    temp=x1; x1=x2; x2=temp;
  }
  
  if(y1>y2) {
    temp=y1; y1=y2; y2=temp;
  }
  if (x2 - x1 < r) return; // Add 20190517
  if (y2 - y1 < r) return; // Add 20190517
  
  x=0;
  y=-r;
  err=2-2*r;

  do{
    if(x) {
      lcdDrawPixel(x1+r-x,y1+r+y,color); 
      lcdDrawPixel(x2-r+x,y1+r+y,color); 
      lcdDrawPixel(x1+r-x,y2-r-y,color); 
      lcdDrawPixel(x2-r+x,y2-r-y,color);
    } // if 
    if ((old_err=err)<=x)   err+=++x*2+1;
    if (old_err>y || err>x) err+=++y*2+1;    
  } while(y<0);
   
  lcdDrawLine(x1+r,y1  ,x2-r,y1  ,color);
  lcdDrawLine(x1+r,y2  ,x2-r,y2  ,color);
  lcdDrawLine(x1  ,y1+r,x1  ,y2-r,color);
  lcdDrawLine(x2  ,y1+r,x2  ,y2-r,color);  
} 

// Draw arrow
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End   X coordinate
// y2:End   Y coordinate
// w:Width of the botom
// color:color
// Thanks http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html
void lcdDrawArrow(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color) {

   double Vx= x1 - x0;
   double Vy= y1 - y0;
   double v = sqrt(Vx*Vx+Vy*Vy);
//   printf("v=%f\n",v);
   double Ux= Vx/v;
   double Uy= Vy/v;

   uint16_t L[2],R[2];
   L[0]= x1 - Uy*w - Ux*v;
   L[1]= y1 + Ux*w - Uy*v;
   R[0]= x1 + Uy*w - Ux*v;
   R[1]= y1 - Ux*w - Uy*v;
//   printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

//   lcdDrawLine(x0,y0,x1,y1,color);
   lcdDrawLine(x1,y1,L[0],L[1],color);
   lcdDrawLine(x1,y1,R[0],R[1],color);
   lcdDrawLine(L[0],L[1],R[0],R[1],color);
}


// Draw arrow of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End   X coordinate
// y2:End   Y coordinate
// w:Width of the botom
// color:color
void lcdDrawFillArrow(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color) {

   double Vx= x1 - x0;
   double Vy= y1 - y0;
   double v = sqrt(Vx*Vx+Vy*Vy);
//   printf("v=%f\n",v);
   double Ux= Vx/v;
   double Uy= Vy/v;

   uint16_t L[2],R[2];
   L[0]= x1 - Uy*w - Ux*v;
   L[1]= y1 + Ux*w - Uy*v;
   R[0]= x1 + Uy*w - Ux*v;
   R[1]= y1 - Ux*w - Uy*v;
//   printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

   lcdDrawLine(x0,y0,x1,y1,color);
   lcdDrawLine(x1,y1,L[0],L[1],color);
   lcdDrawLine(x1,y1,R[0],R[1],color);
   lcdDrawLine(L[0],L[1],R[0],R[1],color);

   int ww;
   for(ww=w-1;ww>0;ww--) {
     L[0]= x1 - Uy*ww - Ux*v;
     L[1]= y1 + Ux*ww - Uy*v;
     R[0]= x1 + Uy*ww - Ux*v;
     R[1]= y1 - Ux*ww - Uy*v;
//     printf("Fill>L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);
     lcdDrawLine(x1,y1,L[0],L[1],color);
     lcdDrawLine(x1,y1,R[0],R[1],color);
   }
}


// RGB565 conversion
// RGB565 is R(5)+G(6)+B(5)=16bit color format.
// Bit image "RRRRRGGGGGGBBBBB"
uint16_t rgb565_conv(uint16_t r,uint16_t g,uint16_t b) {
	return (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}



