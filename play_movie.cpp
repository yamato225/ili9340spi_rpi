#include <iostream>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ili9340formovie.h"

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
