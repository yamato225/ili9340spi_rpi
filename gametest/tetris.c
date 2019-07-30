#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <stdarg.h>

#include "ili9340.h"

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

#define JAPANESE 1
#define _DEBUG_ 0

//When you'd like to wait by a keyboard entries, enable this line.
//#define WAIT inputKey()

//When you'd like to wait in the waiting time, enable this line.
#define WAIT sleep(5)

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

cwiid_err_t err;
void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}

void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}


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
#define draw_height 23
#define grid_width 15
#define grid_height 27
#define draw_start_y 18
#define draw_start_x 1

void lcddrawGrid(int drawGrid[]){
    int draw_x,draw_y;
    uint16_t color;

    for(draw_y=0;draw_y<draw_height;draw_y++){
        for(draw_x=0;draw_x<draw_width;draw_x++){
            if(drawGrid[draw_y*draw_width+draw_x] > 0){
                color=GREEN;
            }else{
                color=BLACK;
            }
            lcdDrawFillRect(draw_x*9, draw_y*7, draw_x*9+7, draw_y*7+5, color);
        }
    }
}

void debugGrid(int gridMat[]){
    int x,y;
    for(y=0;y<grid_height;y++){
        printf("%02d:",y);
        for(x=0;x<grid_width;x++){
            printf("%d",gridMat[y*grid_width+x]);
        }
        printf("\n");
    }
    printf("----------------\n");
}

time_t tetris() {
    struct timeval startTime, endTime;

    lcdFillScreen(BLACK);

    int gridMat[grid_width*grid_height];
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
    mx=0;my=0;
    i=2;
    for(y=0;y<4;y++){
        for(x=0;x<4;x++){
            mino[x][y]=org_mino[i*16+y*4+x];
        }
    }
    srand( (unsigned int)time( NULL ) );
    for(y=0;y<grid_height;y++){
        for(x=0;x<grid_width;x++){
            gridMat[y*grid_width+x]=0;
            if(x==0 || x==grid_width-1 || y==grid_height-1){
                gridMat[y*grid_width+x]=1;
            }
        }
    }
    
    #if 1
    //wiimote setup
	cwiid_wiimote_t *wiimote;	/* wiimote handle */
	struct cwiid_state state;	/* wiimote state */
	bdaddr_t bdaddr;	/* bluetooth device address */

    unsigned char led_state = 0;
    unsigned char rpt_mode = 0;

    cwiid_set_err(err);

    bdaddr = *BDADDR_ANY;

    i=0;
	printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
	while (!(wiimote = cwiid_open(&bdaddr, 0))) {
        i++;
		fprintf(stderr, "Unable to connect to wiimote. Trying...(%d)\n",i);
        if(i>2){
            return -1;
        }
	}

    printf("get button\n");
	toggle_bit(rpt_mode, CWIID_RPT_BTN);
	set_rpt_mode(wiimote, rpt_mode);

    #endif




    bool isTouch=false;
    int isRotate=0;
    while(1){
        gettimeofday(&startTime, NULL);

        if (!isTouch){
            for(y=0;y<4;y++){
                for(x=0;x<4;x++){
                    if (mino[x][y] >0){
                        gridMat[(my+y)*grid_width+mx+x]=0;
                    }
                }
            }
            my++;
            //controller
            #if 1
            if (cwiid_get_state(wiimote, &state)) {
                fprintf(stderr, "Error getting state\n");
            }
            printf("Buttons: ");
            if (state.buttons & CWIID_BTN_LEFT){
                mx--;
                printf("←");
            }
            if (state.buttons & CWIID_BTN_RIGHT){
                mx++;
                printf("→");
            }
            if (state.buttons==CWIID_BTN_B){
                isRotate++;
                printf("↑");
            }
            if (state.buttons & CWIID_BTN_DOWN){
                printf("↓");
            }
            printf("\n");

            if(state.buttons==CWIID_BTN_HOME){
                printf("break!!!\n\n");
                break;
            }

            #endif
        }else{
            printf("RESET\n\n");

            //そろってるかチェック
            int xblock_num=0;
            int check_y,check_x;
            for(check_y=grid_height-2;check_y>=3;){
                xblock_num=0;
                printf("y=%d\n",check_y);
                for(check_x=0;check_x<grid_width;check_x++){
                    xblock_num+=gridMat[check_y*grid_width+check_x]>0 ? 1 : 0;
                }
                if(xblock_num == grid_width){
                    //何らかのエフェクトを加える
                    //1行下にずらす。
                    int empty_row[grid_width]={1,0,0,0,0,0,0,0,0,0,0,0,0,1};
                    int tmp_gridMat[grid_width*grid_height];
                    for(i=0;i<grid_width*grid_height;i++){
                        tmp_gridMat[i]=gridMat[i];
                    }
                    for(i=0;i<grid_width*(check_y+1);i++){
                        gridMat[i]=i<grid_width ? empty_row[i] : tmp_gridMat[i-grid_width];
                    }
                    debugGrid(gridMat);
                    for(y=0;y<draw_height;y++){
                        for(x=0;x<draw_width;x++){
                            drawGrid[y*draw_width+x]=gridMat[(grid_height-2-y)*grid_width+x+1];
                        }
                    }
                    lcddrawGrid(drawGrid);
                    sleep(1);
                }else{
                    printf("else\n");
                    check_y--;
                }
            }

            //mx=(mx+1)%13;my=0;
            mx=6;my=0;
            
            i=rand()%5;
            //i=2;
            for(y=0;y<4;y++){
                for(x=0;x<4;x++){
                    mino[x][y]=org_mino[i*16+y*4+x];
                }
            }
        }

        if(isRotate>1){
            isRotate=0;
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
                    gridMat[(my+y)*grid_width+mx+x]=1;
                }
            }
        }

        debugGrid(gridMat);

        for(y=0;y<4;y++){
            for(x=0;x<4;x++){
                if(gridMat[(my+y)*grid_width+mx+x]>0){
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
                    if (mino[x][y] >0 && gridMat[(my+y+1)*grid_width+mx+x]>0){
                        isTouch=true;
                    }   
                }else{
                    if (mino[x][y]>0 && mino[x][y+1]==0 && gridMat[(my+y+1)*grid_width+mx+x]>0){
                        isTouch=true;
                    }
                }
            }
        }
        printf("\n");

        for(y=0;y<draw_height;y++){
            for(x=0;x<draw_width;x++){
                drawGrid[y*draw_width+x]=gridMat[(grid_height-2-y)*grid_width+x+1];
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

