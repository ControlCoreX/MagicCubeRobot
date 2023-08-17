#include "debug.h"
#include "log.h"
#include "colorful.h"
#include "tucube.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "tucomm.h"
#include "unistd.h"

using namespace cv;
using namespace std;

extern tucube cube;//!< 用于线程间传递数据.

extern const uint32_t STEP_CMD_INDEX = 1U;//!< 好像这种const类型的, 在extern的时候需要带上值.
extern const uint32_t SERVO_CMD_INDEX = 2U;

extern const uint32_t SERVO_POS_1_CMP = 47U;
extern const uint32_t SERVO_POS_2_CMP = 153U;
extern const uint32_t SERVO_POS_3_CMP = 245U;

extern uint32_t record_steps;

extern int mcu_fd;

void* debug_thread_routine(void* param)
{
	cout << endl << COLOR_BOLD_BLUE "entry debug thread! Please activate the terminal!" << COLOR_OFF << endl;

	Mat backend_img = *((Mat*)param);

    char str[5] = {'\0'};

    char choice = '\0';
    Mat frontend_img;

    unsigned b = 0U, g = 0U, r = 0U;

    //! 依次指定一个面上的每个色块.
    for(unsigned row = 0U; row < 3U; row++)
    {
        for(unsigned col = 0U; col < 3U; col++)
        {
            frontend_img = backend_img.clone();//!< 得到一张干净图像.
            Rect tag(cube.samp_area[row][col].tl(), Size(15, 15));//!< 标记的大小固定为15×15.
            rectangle(frontend_img, tag, Scalar(0, 255, 0), -1);//!< 指定色块做上标记.

            imshow("debug", frontend_img);

            cube.calc_rect_bgr(backend_img, cube.samp_area[row][col], b, g, r);//!< 计算出该处的bgr值.

            cout << COLOR_BOLD_BLUE "tell me what color is this: " COLOR_OFF;
            cin >> choice;

            //!< 加 跳过键, 因为有时候两种相同的颜色挨着了.
            //!< 加 结束键. 可以提前结束.
            switch(choice)
            {
                case 'B'://!< 把这个bgr存到blue所在的位置上.
                case 'b':
                    cube.bgrs[0 + 0] = b;
                    cube.bgrs[0 + 1] = g;
                    cube.bgrs[0 + 2] = r;
                    cout << COLOR_BOLD_WHITE "blue = " << b << ", " << g << ", " << r << COLOR_OFF << endl << endl;
                    break;

                case 'Y'://!< 把这个bgr存到yellow所在的位置上.
                case 'y':
                    cube.bgrs[3 + 0] = b;
                    cube.bgrs[3 + 1] = g;
                    cube.bgrs[3 + 2] = r;
                    cout << COLOR_BOLD_WHITE "yellow = " << b << ", " << g << ", " << r << COLOR_OFF <<  endl << endl;
                    break;

                case 'R'://!< 把这个bgr存到red所在的位置上.
                case 'r':
                    cube.bgrs[6 + 0] = b;
                    cube.bgrs[6 + 1] = g;
                    cube.bgrs[6 + 2] = r;
                    cout << COLOR_BOLD_WHITE "red = " << b << ", " << g << ", " << r << COLOR_OFF <<  endl << endl;
                    break;

                case 'G'://!< 把这个bgr存到yellow所在的位置上.
                case 'g':
                    cube.bgrs[9 + 0] = b;
                    cube.bgrs[9 + 1] = g;
                    cube.bgrs[9 + 2] = r;
                    cout << COLOR_BOLD_WHITE "green = " << b << ", " << g << ", " << r << COLOR_OFF <<  endl << endl;
                    break;

                case 'W'://!< 把这个bgr存到white所在的位置上.
                case 'w':
                    cube.bgrs[12 + 0] = b;
                    cube.bgrs[12 + 1] = g;
                    cube.bgrs[12 + 2] = r;
                    cout << COLOR_BOLD_WHITE "white = " << b << ", " << g << ", " << r << COLOR_OFF <<  endl << endl;
                    break;

                case 'O'://!< 把这个bgr存到orange所在的位置上.
                case 'o':
                    cube.bgrs[15 + 0] = b;
                    cube.bgrs[15 + 1] = g;
                    cube.bgrs[15 + 2] = r;
                    cout << COLOR_BOLD_WHITE "orange = " << b << ", " << g << ", " << r << COLOR_OFF <<  endl << endl;
                    break;

                case 'N'://!< 跳过键
                case 'n':
                    cout << COLOR_BOLD_BLUE "SKIP!" << COLOR_OFF <<  endl << endl;
                    break;

                case 'Q'://!< 退出键
                case 'q':
                    goto end;

                default://!< 添加输入错误, 重新输入的功能.
                    cout << COLOR_BOLD_RED "No such color!" << COLOR_OFF <<  endl << endl;
                    break;
            }
        }
    }

end:
    //! 退出线程前, 把debug窗口销毁掉.
    destroyWindow("debug");

    //! 把计算得到的新的bgr的值存到文件里面, 以备下次用.
    cube.store_bgrs_to_file();
    
    tucomm_write_cmd(mcu_fd, SERVO_CMD_INDEX, SERVO_POS_1_CMP, 0U, 0U);//!< 你调整完了, 需要重新开始识别, 所以要舵机归位.
    usleep(500000);//!< 延时500ms, 防止被当成是一个命令.
    for(uint32_t i = 0; i < record_steps; i++)
    {
        tucomm_write_cmd(mcu_fd, STEP_CMD_INDEX, 0U, 0U, 377U);//!< 魔方反方向旋转到起始位置.
        sleep(1);
    }

    record_steps = 0U;//!< 把这个也清零.

    cout << COLOR_BOLD_GREEN "debug successful! Please activate the video window!" COLOR_OFF << endl;

    //! 输出指示用户操作的信息.
    cout << COLOR_BOLD_BLUE "press 'N'/'n' to recongnize next face" << endl << \
                            "press 'G'/'g' to debug" COLOR_OFF << endl;


    //! 到这里, 阈值已经调整, 机械装置已经完全复位. 更系统刚上电的时候是一模一样的.

    return (void*)0;
}


