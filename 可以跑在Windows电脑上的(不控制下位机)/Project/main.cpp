#include <opencv2/opencv.hpp>
#include <iostream>
#include "tucube.h"
#include "debug.h"
#include "WSocketTool.h"
#include <string>

using namespace std;
tucube cube(2);
cv::VideoCapture video;
int time_flag = 0;
cv::Point tl, br;
void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		if (time_flag == 0)
		{
			tl.x = x;
			tl.y = y;

			time_flag++;
		}
		else if (time_flag == 1)
		{
			br.x = x;
			br.y = y;

			time_flag++;
		}
	}
}


int main(void)
{
	cv::Mat before_resize_frame, frame, frame_to_send;
	bool roi_coor_isstored = false;
	color_t colors[9];
	uint8_t index = 0U;
	char choice = '0';
	double ticks = 0.0, fps = 0.0;
	char fps_string[20];
	video.open(1, cv::CAP_DSHOW);
	if (video.isOpened() == false)
	{
		cout << "video opened failed!" << endl;
		return -1;
	}
	cv::namedWindow("video0", cv::WINDOW_AUTOSIZE);
	cv::setMouseCallback("video0", mouseCallback, NULL);
	for (; ; )
	{
		ticks = (double)cv::getTickCount();
		video >> before_resize_frame;
		if (before_resize_frame.empty() == true)
		{
			cout << "video stream end!" << endl;
			break;
		}
		resize(before_resize_frame, frame, cv::Size(640, 480));
		choice = cv::waitKey(25);
		if (time_flag > 1)
		{
			frame_to_send = frame.clone();
			rectangle(frame, cv::Rect(tl, br), cv::Scalar(0, 255, 0), 2);
			if (roi_coor_isstored == false)
			{
				cube.get_roi_from_user(cv::Rect(tl, br));
				cube.calc_samp_area();
				roi_coor_isstored = true;
			}
			cube.recongnize_frame(frame, colors);
			cube.show_recongnize_result(frame, colors);
			if (choice == 'N' || choice == 'n')
			{
				cube.store_recongnize_result(colors, index);
				cout << "current face recongnize successful!" << endl;
				index++;
				if (index == 6U)
					break;
			}
			else if (choice == 'G' || choice == 'g')
			{
				debug_thread_routine(&frame_to_send);
			}

		}
		fps = 1.0 / (((double)cv::getTickCount() - ticks) / cv::getTickFrequency());
		sprintf_s(fps_string, "fps:%.1lf", fps);
		putText(frame, fps_string, cv::Point(2, 25), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 0, 0), 1);
		imshow("video0", frame);
	}

	video.release();
	cv::destroyWindow("video0");
	cube.product_describe();
	printf("\ncube descirbe = %s\n", cube.describe);
	string cube_describe_string(54, 1);
	memmove(&cube_describe_string[0], cube.describe, 54);
	WSocketTool wSocketTool;
	string url = "/?" + cube_describe_string;
	wSocketTool.sendGetData(url);
	string result = wSocketTool.receiptData();
	cout << "result = " << result << endl;
	return 0;
}