#include "debug.h"
#include "tucube.h"
#include <opencv2/opencv.hpp>
#include <iostream>
extern tucube cube;
void debug_thread_routine(cv::Mat* param)
{
    cv::Mat backend_img = *param;
    char str[5] = {'\0'};
    char choice = '\0';
    cv::Mat frontend_img;
    unsigned b = 0U, g = 0U, r = 0U;
    for(unsigned row = 0U; row < 3U; row++)
    {
        for(unsigned col = 0U; col < 3U; col++)
        {
            frontend_img = backend_img.clone();
            cv::Rect tag(cube.samp_area[row][col].tl(), cv::Size(15, 15));
            cv::rectangle(frontend_img, tag, cv::Scalar(0, 255, 0), -1);
            imshow("debug", frontend_img);
            cube.calc_rect_bgr(backend_img, cube.samp_area[row][col], b, g, r);
            choice = cv::waitKey(0);
            switch(choice)
            {
                case 'B':
                case 'b':
                    cube.bgrs[0 + 0] = b;
                    cube.bgrs[0 + 1] = g;
                    cube.bgrs[0 + 2] = r;
                    cout << "blue = " << b << ", " << g << ", " << r << endl << endl;
                    break;

                case 'Y':
                case 'y':
                    cube.bgrs[3 + 0] = b;
                    cube.bgrs[3 + 1] = g;
                    cube.bgrs[3 + 2] = r;
                    cout << "yellow = " << b << ", " << g << ", " << r << endl << endl;
                    break;

                case 'R':
                case 'r':
                    cube.bgrs[6 + 0] = b;
                    cube.bgrs[6 + 1] = g;
                    cube.bgrs[6 + 2] = r;
                    cout << "red = " << b << ", " << g << ", " << r << endl << endl;
                    break;

                case 'G':
                case 'g':
                    cube.bgrs[9 + 0] = b;
                    cube.bgrs[9 + 1] = g;
                    cube.bgrs[9 + 2] = r;
                    cout << "green = " << b << ", " << g << ", " << r << endl << endl;
                    break;

                case 'W':
                case 'w':
                    cube.bgrs[12 + 0] = b;
                    cube.bgrs[12 + 1] = g;
                    cube.bgrs[12 + 2] = r;
                    cout << "white = " << b << ", " << g << ", " << r << endl << endl;
                    break;

                case 'O':
                case 'o':
                    cube.bgrs[15 + 0] = b;
                    cube.bgrs[15 + 1] = g;
                    cube.bgrs[15 + 2] = r;
                    cout << "orange = " << b << ", " << g << ", " << r << endl << endl;
                    break;

                case 'N':
                case 'n':
                    cout << "SKIP!" << endl << endl;
                    break;

                case 'Q':
                case 'q':
                    goto end;

                default:
                    cout << "No such color!" <<  endl << endl;
                    break;
            }
        }
    }

end:
    cv::destroyWindow("debug");
    cube.store_bgrs_to_file();
    cout << "debug successful!" << endl;
    return ;
}
