#include <wiringPi.h>
#include <wiringSerial.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include "colorful.h"
// #include "log.h"
#include "tucube.h"
#include "debug.h"
#include "tucomm.h"

using namespace std;


//! 因为在识别颜色的过程中需要转动魔方和摄像头, 这是控制转角的全局常量(没用宏).
const uint32_t STEP_CMD_INDEX = 1U;
const uint32_t SERVO_CMD_INDEX = 2U;

const uint32_t SERVO_POS_1_CMP = 47U;
const uint32_t SERVO_POS_2_CMP = 153U;
const uint32_t SERVO_POS_3_CMP = 245U;

uint32_t record_steps = 0U;//!< 记录步进电机转了多少个90°. 顺时针旋转+1, 不会逆时针旋转, 因为这里不包括手动模式时的转动.


//! 注意: 退出程序的时候, 把打开的所有文件都关闭掉!


int mcu_fd, inet_fd;//!< 串口文件描述符.
VideoCapture video;

pthread_t video_thread_tid;
pthread_t debug_thread_tid;
pthread_t com_thread_tid;

tucube cube(2U);//!< 定义一个对象.


void* video_thread_routine(void* param)
{
	Mat before_resize_frame, frame, frame_to_send;

	int posx = 0, posy = 0;//!< 用鼠标调整的坐标.
	
	Point roi_tl, roi_br;//!< ROI区域的左上角和右下角坐标.
	int roi_cnt = 0;//!< 记录本次选中的是ROI区域的第几个坐标.
	
	uint8_t mode = 0U;//!< 是在选择区域还是在拍照.
	const uint8_t CHOOSE_ROI_MODE = 0U, RECONGNIZE_COLOR_MODE = 1U;

	uint8_t index = 0U;//!< 拍照时图像的索引.

	char choice = 0;//!< 存储用户按下的键值.
	
	color_t colors[9];

	double ticks = 0.0, fps = 0.0;
	char fps_string[20];

	for( ; ; )
	{
		ticks = (double)getTickCount();

		video >> before_resize_frame;
		if(before_resize_frame.empty() == true)
			break;
		resize(before_resize_frame, frame, Size(640, 480));//!< 输入到对象中的图像是固定大小640×480, 不受摄像头的影响.
		
		choice = waitKey(25);
		
		if(mode == CHOOSE_ROI_MODE)
		{
			switch(choice)
			{
				case 'W':
				case 'w': 
					if(posy > 0) 
						posy -= 2;
					break;

				case 'K':
				case 'k': 
					if(posy > 0)
						posy -= 20;
					break;

				case 'S':
				case 's':
					posy += 2;
					break;

				case 'J':
				case 'j':
					posy += 20;
					break;

				case 'A':
				case 'a': 
					if(posx > 0)
						posx -= 2;
					break;

				case 'H':
				case 'h':
					if(posx > 0)
						posx -= 20;
					break;

				case 'D':
				case 'd':
					posx += 2;
					break;

				case 'L':
				case 'l':
					posx += 20;
					break;

				case 'N':
				case 'n':
					if(roi_cnt == 0)//!< ROI的左上角坐标确定好了.
					{
						//! 左上角坐标确定了, 就把左上角坐标存起来.
						roi_tl.x = posx;
						roi_tl.y = posy;

						roi_cnt++;//!< 标记要开始下一个点了.
						
						cout << COLOR_BOLD_GREEN "roi_rect tl is ok!" COLOR_OFF << endl;
					}	
					else
					{
						roi_br.x = posx + 5;
						roi_br.y = posy + 5;

						cout << COLOR_BOLD_GREEN "roi_rect br is ok!" COLOR_OFF << endl;

						cube.get_roi_from_user(Rect(roi_tl, roi_br));
						cube.calc_samp_area();//!< 开始计算采样点坐标.

						mode = RECONGNIZE_COLOR_MODE;//!< 进入实时识别模式. 

						//! 输出指示用户操作的信息.
						cout << COLOR_BOLD_BLUE "press 'N'/'n' to recongnize next face" << endl << \
												"press 'G'/'g' to debug" COLOR_OFF << endl;
					}
					break;
			}//!< end of 'switch(choice)'

			rectangle(frame, Rect(posx, posy, 5, 5), Scalar(0, 255, 0), -1);//!< 画出反馈点
		}//!< end of 'if(mode == CHOOSE_ROI_MODE)'
		else if(mode == RECONGNIZE_COLOR_MODE)
		{
			frame_to_send = frame.clone();

			rectangle(frame, cube.roi_rect, Scalar(0, 255, 0), 2);//!< 圈出ROI那个框.

			cube.recongnize_frame(frame, colors);//!< 识别当前帧.
			cube.show_recongnize_result(frame, colors);//!< 将识别结果显示到当前帧.

			//! 按下'n'键就把当前这个面的识别结果保存起来, 然后拍下一张.
			if(choice == 'N' || choice == 'n')
			{
				cube.store_recongnize_result(colors, index);//!< 识别结果存起来.

				cout << COLOR_BOLD_GREEN "current face recongnize successful!" COLOR_OFF << endl;//!< 提示识别成功.

				//! 舵机转动 或者 魔方转动, 这里还需要条件语句判断一下.
				switch(index)
				{
					case 0U://!< 如果识别的是第0个面, 那摄像头就转到位置2.
						tucomm_write_cmd(mcu_fd, SERVO_CMD_INDEX, SERVO_POS_2_CMP, 0U, 0U);//!< 舵机转到2位置.
						break;

					case 1U:
					case 2U:
					case 3U:
						tucomm_write_cmd(mcu_fd, STEP_CMD_INDEX, 0U, 1U, 377U);//!< 2, 3, 4的时候, 动作都是魔方旋转90度.
						record_steps += 1U;
						break;

					case 4U:
						tucomm_write_cmd(mcu_fd, STEP_CMD_INDEX, 0U, 1U, 377U);//!< 魔方旋转90度.
						record_steps += 1U;
						usleep(500000);//!< 此处休眠500ms, 因为这两句都要写文件, 如果间隔太短, 就会被当成是一个命令. (这里sleep会导致视频流卡住)
						tucomm_write_cmd(mcu_fd, SERVO_CMD_INDEX, SERVO_POS_3_CMP, 0U, 0U);//!< 舵机转到3位置
						break;
				}
			 	
				index++;

			 	if(index == 6U)//!< 6个面拍完了, 退出视频.
				{
					tucomm_write_cmd(mcu_fd, SERVO_CMD_INDEX, SERVO_POS_1_CMP, 0U, 0U);//!< 舵机归位.
			 		break;
				}

			}//!< end of 'if(choice == 'N' || choice == 'n')'
			else if(choice == 'G' || choice == 'g')//!< 发现颜色识别不对, 按'g'启动debug线程, 重新调整阈值.
			{
				pthread_create(&debug_thread_tid, NULL, debug_thread_routine, (void*)&frame_to_send);
				pthread_detach(debug_thread_tid);
				
				//! 重新开始识别6个面, 把index清零.
				index = 0U;
			}//!< end of 'else if(choice == 'G' || choice == 'g')'
		}//!< end of 'else if(mode == RECONGNIZE_COLOR_MODE)'

		//! 计算帧率并显示.
		fps = 1.0 / (((double)getTickCount() - ticks) / getTickFrequency());
		sprintf(fps_string, "fps:%.1lf", fps);
		putText(frame, fps_string, Point(2, 25), FONT_HERSHEY_DUPLEX, 1, Scalar(255, 0, 0), 1);

		imshow("video0", frame);

	}//!< end of 'for( ; ; )'

	video.release();//!< 释放视频流.
	destroyWindow("video0");

	cout << COLOR_BOLD_GREEN "six face recongnize successful!" COLOR_OFF << endl << endl;

	cout << COLOR_BOLD_BLUE "would like to see them:(y/n)" COLOR_OFF;//!< 查看识别结果.
	cin >> choice;
	if(choice == 'Y' || choice == 'y')
	{
		cube.show_final_recongnize_result('a');//!< 这里默认就用a方法, 因为b方法我没有写.
	}

	//! 生成描述魔方色块信息的字符串.
	cube.product_describe();
	printf(COLOR_BOLD_BLUE "\ncube descirbe = %s\n" COLOR_OFF, cube.describe);

	//! 将describe字符串发送给上位机, 获取魔方解法.
	write(inet_fd, cube.describe, 54);
	delay(1000);
	int slove_len = read(inet_fd, cube.slove, sizeof(cube.slove));//!< 读到的slove字符串中没有'\0'.
	if((slove_len != -1) && (cube.slove[0] != 'C'))//!< 这样才表示此魔方可解.
	{
		printf(COLOR_BOLD_BLUE "slove = %s\n" COLOR_OFF, cube.slove);//!< 把slove打印出来.
		tucomm_write_data(mcu_fd, slove_len, cube.slove);//!< 把slove发送给单片机. 单片机接收到要去解析.
	}
	else
	{
		printf(COLOR_BOLD_RED "get slove failed!\n" COLOR_OFF);
		exit(0);
	}

	exit(0);
}


//! 专门用来处理串口发来的指令的线程, 发一个指令, 这个线程置位一个标志位, 然后其它线程来处理.
//! 是否可以使用bitmap的方法.
void* com_thread_routine(void* param)
{
	int buf[10] = {0};
	uint16_t n = 0;

	for( ; ; )
	{
	//	tucom_read_data(fd, buf, &n);
		delay(3000);
	}

	return 0;
}


int main(int argc, char* argv[])
{
	if(wiringPiSetup() == -1)
	{
		cout << COLOR_BOLD_RED "wiringPi library open failed!" COLOR_OFF << endl;
		return -1;	
	}

	//! 打开与单片机通信的串口.
	mcu_fd = serialOpen("/dev/ttyAMA0", 115200);
	if(mcu_fd == -1)
	{
		cout << COLOR_BOLD_RED "mcu serial port open failed!" COLOR_OFF << endl;
		return -1;	
	}

	//! 打开与网络通信的串口. 这里使用uart2, 它对应的设备文件是/dev/ttyAMA1.
	inet_fd = serialOpen("/dev/ttyAMA1", 115200);
	if(inet_fd == -1)
	{
		cout << COLOR_BOLD_RED "inet serial port open failed!" COLOR_OFF << endl;
		return -1;	
	}

	//! open the camera.
	video.open(0);
	if(video.isOpened() == false)
	{
		cout << COLOR_BOLD_RED "video opened failed!" COLOR_OFF << endl;
		return -1;	
	}

	//! get the video's fps, because I want to clear the video buffer.	
	cout << "camera's fps = " << video.get(CAP_PROP_FPS) << endl;

	namedWindow("video0", WINDOW_AUTOSIZE);
	
	//! create the video thread.
    pthread_create(&video_thread_tid, NULL, video_thread_routine, NULL);	

	//! create the com thread.
//	pthread_create(&com_thread_tid, NULL, com_thread_routine, NULL);

	pthread_exit(NULL);
	return 0;
}
