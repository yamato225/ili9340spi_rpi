#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "ili9340.h"

#define JAPANESE 1
#define _DEBUG_ 0

//When you'd like to wait by a keyboard entries, enable this line.
//#define WAIT inputKey()

//When you'd like to wait in the waiting time, enable this line.
#define WAIT sleep(5)

int ReadTFTConfig(char *path, int *width, int *height, int *offsetx, int *offsety) {
  FILE *fp;
  char buff[128];
  
//  printf("path=%s\n",path);
  fp = fopen(path,"r");
  if(fp == NULL) return 0;
  while (fgets(buff,128,fp) != NULL) {
//    printf("buf=%s\n",buff);
//    printf("buff[0]=%x\n",buff[0]);
    if (buff[0] == '#') continue;
    if (buff[0] == 0x0a) continue;
    if (strncmp(buff, "width=", 6) == 0) {
      sscanf(buff, "width=%d height=%d",width,height);
if(_DEBUG_)printf("width=%d height=%d\n",*width,*height);
    } else if (strncmp(buff, "offsetx=", 8) == 0) {
      sscanf(buff, "offsetx=%d",offsetx);
if(_DEBUG_)printf("offsetx=%d\n",*offsetx);
    } else if (strncmp(buff, "offsety=", 8) == 0) {
      sscanf(buff, "offsety=%d",offsety);
if(_DEBUG_)printf("offsety=%d\n",*offsety);
    }
  }
  fclose(fp);
  return 1;
}

time_t elapsedTime(struct timeval startTime, struct timeval endTime) {
    time_t diffsec = difftime(endTime.tv_sec, startTime.tv_sec);
    suseconds_t diffsub = endTime.tv_usec - startTime.tv_usec;
//printf("diffsec=%ld diffsub=%ld\n",diffsec, diffsub);
    if(diffsub < 0) {
        diffsec--;
        diffsub = (endTime.tv_usec+1000000) - startTime.tv_usec;
    }
    uint16_t diffmsec = diffsub / 1000;
    time_t diff = (diffsec * 1000) + diffmsec;
    return diff;
}

time_t ColorBarTest(int width, int height) {
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    uint16_t y1 = height/3;
    uint16_t y2 = (height/3)*2;
    lcdDrawFillRect(0, 0, width-1, y1-1, RED);
    lcdDrawFillRect(0, y1-1, width-1, y2-1, GREEN);
    lcdDrawFillRect(0, y2-1, width-1, height-1, BLUE);

    gettimeofday(&endTime, NULL);
    time_t diff = elapsedTime(startTime, endTime);
    printf("%s elapsed time[ms]=%ld\n",__func__, diff);
    return diff;
}

#define draw_width 13
#define draw_height 16 

void lcddrawGrid(int drawGrid[]){
    int draw_x,draw_y;
    uint16_t color;

    for(draw_y=0;draw_y<draw_height;draw_y++){
        for(draw_x=0;draw_x<draw_width;draw_x++){
            if(drawGrid[draw_y*draw_width+draw_x] > 0){
                color=WHITE;
            }else{
                color=BLACK;
            }
            lcdDrawFillRect(draw_x*11, draw_y*10, draw_x*11+8, draw_y*10+8, color);
        }
    }
}

time_t tetris() {
    struct timeval startTime, endTime;

    lcdFillScreen(BLACK);

    const int grid_width=14;
    const int grid_height=20;

    int gridMat[grid_width][grid_height];
    int drawGrid[draw_width*draw_height];

    int mino[4][4];
    int org_mino[5*4*4]={
        0,0,0,0,            //0: ト
        1,1,1,0,
        0,1,0,0,
        0,0,0,0,
        //----
        0,0,1,0,            //1:L
        0,0,1,0,
        0,1,1,0,
        0,0,0,0,
        //----
        0,1,0,0,            //2:棒
        0,1,0,0,
        0,1,0,0,
        0,1,0,0,
        //----
        0,0,0,0,            //3:四角
        0,1,1,0,
        0,1,1,0,
        0,0,0,0,
        //----
        1,1,0,0,            //4:S
        0,1,1,0,
        0,0,0,0,
        0,0,0,0};

    int tmp_mino[4][4];

    int x,y;
    int i;
    int draw_x,draw_y;
    int mx,my;
    mx=-1;my=0;
    i=1;
    for(y=0;y<4;y++){
        for(x=0;x<4;x++){
            mino[x][y]=org_mino[i*16+y*4+x];
        }
    }
    srand( (unsigned int)time( NULL ) );
    for(y=0;y<20;y++){
        for(x=0;x<14;x++){
            gridMat[x][y]=0;
            if(x==0 || x==13 || y==19){
                gridMat[x][y]=1;
            }
        }
    }
    
    bool isTouch=false;

    while(1){
        gettimeofday(&startTime, NULL);

        if (!isTouch){
            for(y=0;y<4;y++){
                for(x=0;x<4;x++){
                    if (mino[x][y] >0){
                        gridMat[mx+x][my+y]=0;
                    }
                }
            }
            my++;
        }else{
            printf("RESET\n\n");

            //そろってるかチェック
            int xblock_num=0;
            for(y=draw_height-1;y>=0;y--){
                xblock_num=0;
                for(x=0;x<draw_width;x++){
                    xblock_num+=drawGrid[y*draw_width+x]>0 ? 1 : 0;
                }
                if(xblock_num == draw_width){
                    printf("sorotta");
                    return 0;
                }
            }

            mx=(mx+1)%13;my=0;
            //i=rand()%5;
            i=2;
            for(y=0;y<4;y++){
                for(x=0;x<4;x++){
                    mino[x][y]=org_mino[i*16+y*4+x];
                }
            }
        }

        if(false){
            for(y=0;y<4;y++){
                for(x=0;x<4;x++){
                    tmp_mino[x][y]=mino[x][y];
                }
            }
            for(y=0;y<4;y++){
                for(x=0;x<4;x++){
                    mino[3-y][x]=tmp_mino[x][y];
                }
            }
        }

        for(y=0;y<4;y++){
            for(x=0;x<4;x++){
                if (mino[x][y] >0){
                    gridMat[mx+x][my+y]=1;
                }
            }
        }

        for(y=0;y<20;y++){
            printf("%02d:",y);
            for(x=0;x<14;x++){
                printf("%d",gridMat[x][y]);
            }
            printf("\n");
        }

        printf("----------------\n");
        for(y=0;y<4;y++){
            for(x=0;x<4;x++){
                if(gridMat[mx+x][my+y]>0){
                    printf("@");
                }else{
                    printf(" ");
                }
            }
            printf("\n");
        }
        printf("----------------\n");

        isTouch=false;
        for(y=3;y>=0;y--){
            for(x=0;x<4;x++){
                if (y==3){
                    if (mino[x][y] >0 && gridMat[mx+x][my+y+1]>0){
                        isTouch=true;
                    }   
                }else{
                    if (mino[x][y]>0 && mino[x][y+1]==0 && gridMat[mx+x][my+y+1]>0){
                        isTouch=true;
                    }
                }
            }
        }
        printf("\n");

        for(y=0;y<draw_height;y++){
            for(x=0;x<draw_width;x++){
                drawGrid[y*draw_width+x]=gridMat[x+1][18-y];
            }
        }

        lcddrawGrid(drawGrid);

        gettimeofday(&endTime, NULL);
        time_t diff = elapsedTime(startTime, endTime);
        printf("%s elapsed time[ms]=%ld\n",__func__, diff);

        usleep(50000);
    }


    return 0;
}

time_t ColorTest(int width, int height) {
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    uint16_t color;
    lcdFillScreen(WHITE);
    color = RED;
    uint16_t delta = height/16;
    uint16_t ypos = 0;
    int i;
    for(i=0;i<16;i++) {
        lcdDrawFillRect(0, ypos, width-1, ypos+delta, color);
        color = color >> 1;
        ypos = ypos + delta;
    }

    gettimeofday(&endTime, NULL);
    time_t diff = elapsedTime(startTime, endTime);
    printf("%s elapsed time[ms]=%ld\n",__func__, diff);
    return diff;
}


int main(int argc, char **argv)
{
    int i;
    int screenWidth = 0;
    int screenHeight = 0;
    int offsetx = 0;
    int offsety = 0;
    char dir[128];
    char cpath[128];

if(_DEBUG_)  printf("argv[0]=%s\n",argv[0]);
    strcpy(dir, argv[0]);
    for(i=strlen(dir);i>0;i--) {
        if (dir[i-1] == '/') {
          dir[i] = 0;
          break;
        } // end if
    } // end for
if(_DEBUG_)printf("dir=%s\n",dir);
    strcpy(cpath,dir);
    strcat(cpath,"../tft.conf");
if(_DEBUG_)printf("cpath=%s\n",cpath);
    if (ReadTFTConfig(cpath, &screenWidth, &screenHeight, &offsetx, &offsety) == 0) {
        printf("%s Not found\n",cpath);
        return 0;
    }
if(_DEBUG_)printf("ReadTFTConfig:screenWidth=%d height=%d\n",screenWidth, screenHeight);
    printf("Your TFT resolution is %d x %d.\n",screenWidth, screenHeight);
    printf("Your TFT offsetx    is %d.\n",offsetx);
    printf("Your TFT offsety    is %d.\n",offsety);

    lcdInit(screenWidth, screenHeight, offsetx, offsety);
    lcdReset();
    lcdSetup();


    ColorBarTest(screenWidth, screenHeight);

    tetris();

    return 0;
}

