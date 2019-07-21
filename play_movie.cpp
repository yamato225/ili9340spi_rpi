#include <iostream>
#include <iostream>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ili9340formovie.h"

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

time_t pixelTest(int width, int height) {
    uint16_t color;

    uint16_t image[128*160];

    time_t diff;
    struct timeval startTime, endTime;

	cv::VideoCapture cap;
	cap.open("filesrc location=/home/pi/D0002040402_00000_V_000.mp4 ! qtdemux ! h264parse ! omxh264dec ! videoconvert ! videoscale ! video/x-raw,width=160,height=128 ! queue ! appsink");
	//cap.open("/home/pi/D0002040402_00000_V_000.mp4");

	if (!cap.isOpened())
	{
		printf("=ERR= fail to open\n");
		return -1;
	}

	cv::Mat srcImg;
	int b,g,r;
	while (cap.read(srcImg))
	{
        gettimeofday(&startTime, NULL);
        for( int y = 0; y < srcImg.rows; y++ ) {
            cv::Vec3b* ptr = srcImg.ptr<cv::Vec3b>( y );
            for( int x = 0; x < srcImg.cols; x++ ) {
                cv::Vec3b bgr = ptr[x];
                image[y*128+x]=rgb565_conv(bgr[0], bgr[1], bgr[2]);
            }
        }
        lcdDrawImage(0,0,127,159,image);
        gettimeofday(&endTime, NULL);
        diff = elapsedTime(startTime, endTime);
        printf("%s elapsed time[ms]=%ld\n",__func__, diff);
    }

    return diff;
}

int main(int argc, char *argv[])
{
	cv::VideoCapture gstreamer;
	//gstreamer.open("filesrc location=D0002040402_00000_V_000.mp4 ! mp4mux ! h264parse ! omxh264dec ! videoconvert ! queue ! appsink");
	gstreamer.open("filesrc location=/home/pi/D0002040402_00000_V_000.mp4 ! qtdemux ! h264parse ! omxh264dec ! videoconvert ! videoscale ! video/x-raw,width=160,height=128 ! queue ! appsink");
	//gstreamer.open("D0002040402_00000_V_000.mp4");

	if (!gstreamer.isOpened())
	{
		printf("=ERR= fail to open\n");
		return -1;
	}


	int i=0;
	cv::Mat GstCap;
	int b,g,r;
	while (gstreamer.read(GstCap))
	{
		cv::Vec3b* ptr = GstCap.ptr<cv::Vec3b>( 10 );
		cv::Vec3b bgr = ptr[10];
		b=bgr[0];
		g=bgr[1];
		r=bgr[2];
		//sprintf(filename,"nfs/preview/%04d.bmp",i);
		//cv::imwrite(filename,GstCap);
		//cv::imshow("z",GstCap);
		//cv::waitKey(0);
		std::cout << "frame=" <<  i << "(" << r << "," << g << "," << b << ")" << std::endl;
		i++;
	}
	return 0;
}
