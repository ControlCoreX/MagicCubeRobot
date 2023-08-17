#include "tucube.h"
#include "colorful.h"
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include "log.h"


/*!
 * @brief: 构造tucube类对象.
 * @param: step: 采样步长. 用于在识别速度和准确度之间找到一个平衡点. 默认为2.
 */
tucube::tucube(unsigned step = 2U)
{
	this->step = step;

	this->color_place[0] = PLACE_0;
	this->color_place[1] = PLACE_1;
	this->color_place[2] = PLACE_2;
	this->color_place[3] = PLACE_3;
	this->color_place[4] = PLACE_4;
	this->color_place[5] = PLACE_5;

	strcpy(this->bgrs_file_path, BGRS_FILE_PATH);
	load_bgrs_from_file();

	cout << COLOR_BOLD_GREEN "cube construct successful!" COLOR_OFF << endl;
}


void tucube::load_bgrs_from_file(void)
{
	int fd = open(this->bgrs_file_path, O_APPEND | O_RDONLY);
	if(fd == -1)
	{
		cout << COLOR_BOLD_RED "bgrs file open failed!" COLOR_OFF << endl;
		exit(-1);
	}

	lseek(fd, SEEK_SET, 0);//!< 把file offset移动到开头.
	
	read(fd, this->bgrs, sizeof(this->bgrs));

	close(fd);

	cout << COLOR_BOLD_GREEN "load bgrs form file successful!" COLOR_OFF << endl;
}


void tucube::store_bgrs_to_file(void)
{
	int fd = open(this->bgrs_file_path, O_TRUNC | O_WRONLY);
	if(fd == -1)
	{
		cout << COLOR_BOLD_RED "bgrs file open failed!" COLOR_OFF << endl;
		exit(-1);
	}
	
	write(fd, this->bgrs, sizeof(this->bgrs));

	close(fd);

	cout << COLOR_BOLD_GREEN "store bgrs to file successful!" COLOR_OFF << endl;
}


/*!
 * @brief: 从用户那里获取ROI.
 * @param: roi_rect: 表示ROI区域的矩形. 就直接把你传进去的矩形赋值给对象内的表示ROI区域的矩形.
 */
void tucube::get_roi_from_user(Rect roi_rect)
{
	this->roi_rect = roi_rect;

	cout << COLOR_BOLD_GREEN "get roi_rect successful!" COLOR_OFF << endl;
	cout << COLOR_BOLD_WHITE "roi_rect = " << this->roi_rect << COLOR_OFF << endl;
}


/*!
 * @brief: 计算采样区域的坐标, 并存储到对象中.
 * @note: 
 */
void tucube::calc_samp_area(void)
{
	unsigned divx   = (unsigned)(this->roi_rect.width  / 3U);
	unsigned divy   = (unsigned)(this->roi_rect.height / 3U);

	unsigned edgex  = (unsigned)(this->roi_rect.width  / 12U);//!< thickness.
	unsigned edgey  = (unsigned)(this->roi_rect.height / 12U);
	
	unsigned width  = (unsigned)((this->roi_rect.width  - 6 * edgex) / 3U);
	unsigned height = (unsigned)((this->roi_rect.height - 6 * edgey) / 3U);

	for(unsigned i = 0U; i < 3U; i++)
	{
		for(unsigned j = 0U; j < 3U; j++)
		{
			this->samp_area[i][j] = Rect(roi_rect.tl().x + j * divx + edgex,
				                         roi_rect.tl().y + i * divy + edgey, width, height);
		}	
	}	

	cout << COLOR_BOLD_GREEN "calculate sample area successful!" COLOR_OFF << endl;
}



/*!
 * @brief: 识别frame中9个色块的颜色, 并参数color返回出来.
 * @param: frame: 待识别的图像.
 *         colors[9]: 识别的结果.
 */
void tucube::recongnize_frame(Mat& frame, color_t colors[9])
{
	unsigned b[9], g[9], r[9];

	this->calc_frame_bgr(frame, b, g, r);
	this->judge_frame_color(colors, b, g, r);
}


/*!
 * @brief: 将通过参数color[9]指定的颜色, 显示到frame上.
 * @param: frame: 色块的颜色就画到它上面.
 *         colors[9]: 9个色块的颜色通过这个参数指定.
 */
void tucube::show_recongnize_result(Mat& frame, color_t colors[9])
{
	unsigned b, g, r;
	Scalar color;

	for(unsigned i = 0U; i < 3U; i++)
	{
		for(unsigned j = 0U; j < 3U; j++)
		{
			this->color_t2Scalar(colors[i * 3U + j], color);
			
			rectangle(frame, this->samp_area[i][j], color, -1);	
		}	
	}
}


/*!
 * @brief: 将识别的结果存到对象中.
 * @param: colors[9]: 待存入的颜色.
 *         index: 这个颜色是第几个面的.
 */
void tucube::store_recongnize_result(color_t colors[9], uint8_t index)
{
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->recongnize_result[index][i] = colors[i];
	}
}


/*!
 * @brief: 将最终的识别结果显示出来, 以供用户检查识别结果是否正确, 便于调试.
 * @param: choice: 显示的方式. 可取的值有: 'a', 'b'. 即只有a, b两种显示方式.
 */
void tucube::show_final_recongnize_result(uint8_t choice)
{
	if(choice == 'a')//!< 第一种显示方法. 逐张显示.
	{
		Mat result(342, 342, CV_8UC3, Scalar(255, 255, 255));//!< 一张白色图像, 作为背景.
		Scalar color[9];

		for(unsigned i = 0U; i < 6U; i++)
		{
			for(unsigned j = 0U; j < 9U; j++)
			{
				this->color_t2Scalar(this->recongnize_result[i][j], color[j]);
			}

			this->color_one_face(result, color);

			imshow("recongnize result", result);

			cout << COLOR_BOLD_BLUE "face " << i + 1U << COLOR_OFF << endl;

			waitKey();
		}
	}
	else if(choice == 'b')//!< 第二种显示方法. 一次显示6个面, 便于调试.
	{
		// uint32_t background_width = ;
		// uint32_t background_height = ;
		// uint32_t edge = ;

		// Mat background(background_height + edge * 2U, background_width + edge * 2U, CV_8UC3, Scalar(255, 255, 255));
		// Mat face(, , CV_8UC3, Scalar(255, 255, 255));

		// Mat U = background(Rect(5, 317, 312, 312));
		// Mat R = background(Rect())
	}

	destroyWindow("recongnize result");
}


/*!
 * @brief: 给一个面着色. 使用白色背景, 免得划线了.
 * @param: face: 在这张图像上显示颜色.
 *         colors: 9个色块的颜色. 从上到下, 从左到右的顺序, 依次为0~9.
 * @note: 这个函数只是个face上的9个色块涂上颜色, 并不会直接显示出来, 它内部没有调用imshow.
 *        这个函数内部会自动根据face的大小, 来计算出9个色块的坐标.
 */
void tucube::color_one_face(Mat& face, Scalar colors[9])
{
	unsigned interval = 3U;//!< 缝隙的宽度.
	unsigned w = (face.cols  - 4 * interval) / 3;//!< 这里没有用浮点数, 而是整数除法, 所以尽量让它除得尽.
	unsigned h = (face.rows - 4 * interval) / 3;

	rectangle(face, Rect(interval,             interval,             w, h), colors[0], -1);
	rectangle(face, Rect(interval * 2 + w,     interval,             w, h), colors[1], -1);
	rectangle(face, Rect(interval * 3 + w * 2, interval,             w, h), colors[2], -1);
	
	rectangle(face, Rect(interval,             interval * 2 + h,     w, h), colors[3], -1);
	rectangle(face, Rect(interval * 2 + w,     interval * 2 + h,     w, h), colors[4], -1);
	rectangle(face, Rect(interval * 3 + w * 2, interval * 2 + h,     w, h), colors[5], -1);

	rectangle(face, Rect(interval,             interval * 3 + h * 2, w, h), colors[6], -1);
	rectangle(face, Rect(interval * 2 + w,     interval * 3 + h * 2, w, h), colors[7], -1);
	rectangle(face, Rect(interval * 3 + w * 2, interval * 3 + h * 2, w, h), colors[8], -1);
}


/*!
 * @brief: 将用color_t表示的颜色, 转化为用Scalar表示的颜色.
 * @param: color1: 用color_t表示的颜色. 即待转化的颜色.
 *         color2: 用Scalar表示的颜色. 即转化的结果.
 */
void tucube::color_t2Scalar(color_t color1, Scalar& color2)
{
	switch(color1)
	{
		case 0: color2 = Scalar(this->bgrs[0 * 3 + 0], this->bgrs[0 * 3 + 1], this->bgrs[0 * 3 + 2]); break;
		case 1: color2 = Scalar(this->bgrs[1 * 3 + 0], this->bgrs[1 * 3 + 1], this->bgrs[1 * 3 + 2]); break;
		case 2: color2 = Scalar(this->bgrs[2 * 3 + 0], this->bgrs[2 * 3 + 1], this->bgrs[2 * 3 + 2]); break;
		case 3: color2 = Scalar(this->bgrs[3 * 3 + 0], this->bgrs[3 * 3 + 1], this->bgrs[3 * 3 + 2]); break;
		case 4: color2 = Scalar(this->bgrs[4 * 3 + 0], this->bgrs[4 * 3 + 1], this->bgrs[4 * 3 + 2]); break;
		case 5: color2 = Scalar(this->bgrs[5 * 3 + 0], this->bgrs[5 * 3 + 1], this->bgrs[5 * 3 + 2]); break;
		default: cout << COLOR_BOLD_RED "match failed in 'color_t2scalar'!" COLOR_OFF << endl;
	}
}


/*! private, 仅被tucube::calc_frame_bgr()函数调用.
 * @biref: 计算img图像中rect位置处的bgr, 3个通道的平均值.
 * @param: img: 待计算图像.
 *         rect: 通过这个参数指定要计算的区域.
 *         b, g, r: 3个通道的平均值.
 * @note: 这个函数只是计算了一张图像中, 一个rect中的bgr平均值.
 *        如果要计算一个面, 9个色块的平均值, 则需要循环调用它9次, 每次指定不同的rect.
 */
void tucube::calc_rect_bgr(Mat& img, Rect& rect, unsigned& b, unsigned& g, unsigned& r)
{
    float sumb = 0.0f, sumg = 0.0f, sumr = 0.0f;
	unsigned point_cnt = 0U;//!< 记录参与计算的一共有多少个点.

    for(unsigned row = 0U; row < rect.height; row += step)//!< which row.
    {
        for(unsigned col = 0U; col < rect.width; col += step)//!< which col.
        {
			sumb += img.at<Vec3b>(rect.tl().y + row, rect.tl().x + col)[0];
			sumg += img.at<Vec3b>(rect.tl().y + row, rect.tl().x + col)[1];
			sumr += img.at<Vec3b>(rect.tl().y + row, rect.tl().x + col)[2];

            point_cnt++;
        }
    }
	
    b = (unsigned)(sumb / point_cnt);
    g = (unsigned)(sumg / point_cnt);
    r = (unsigned)(sumr / point_cnt);
}


/*!
 * @brief: 分别计算frame中9个色块的bgr值, 并通过参数返回.
 * @param: frame: 待计算的图像. 一般就是魔方的一个面.
 *         b[9], g[9], r[9]: 9个色块的bgr值. 如第一个色块的bgr值分别为b[0], g[0], r[0].
 * @note: 这个函数不需要通过参数指定frame中9个色块的位置, 它内部根据this->samp_area计算.
 */
void tucube::calc_frame_bgr(Mat& frame, unsigned b[9], unsigned g[9], unsigned r[9])
{
	unsigned tmp_b = 0U, tmp_g = 0U, tmp_r = 0U;

	for(unsigned row = 0U; row < 3U; row++)
	{
		for(unsigned col = 0U; col < 3U; col++)
		{
			calc_rect_bgr(frame, this->samp_area[row][col], tmp_b, tmp_g, tmp_r);
			b[row * 3U + col] = tmp_b;
			g[row * 3U + col] = tmp_g;
			r[row * 3U + col] = tmp_r;
		}
	}
}



/*! private, 仅被tucube::judge_frame_color()函数调用.
 * @brief:
 */
void tucube::judge_color_by_bgr(unsigned b, unsigned g, unsigned r, color_t& color)
{
	unsigned square_of_b_distance[6];
	unsigned square_of_g_distance[6];
	unsigned square_of_r_distance[6];

	unsigned average_distance[6];

	//! 给出一个颜色, 我算出它离6种颜色的距离, 找出距离最小者, 即找到颜色.
	for(unsigned i = 0U; i < 6U; i++)//!< which color.
	{
		square_of_b_distance[i] = (b - this->bgrs[i * 3 + 0]) * (b - this->bgrs[i * 3 + 0]);
		square_of_g_distance[i] = (g - this->bgrs[i * 3 + 1]) * (g - this->bgrs[i * 3 + 1]); 
		square_of_r_distance[i] = (r - this->bgrs[i * 3 + 2]) * (r - this->bgrs[i * 3 + 2]); 

		average_distance[i] = (unsigned)sqrt(square_of_b_distance[i] + square_of_g_distance[i] + square_of_r_distance[i]);
	}	

	color = this->get_min_item_index(average_distance, 6U);
}


/*!
 * 
 */
void tucube::judge_frame_color(color_t color[9], unsigned b[9], unsigned g[9], unsigned r[9])
{
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->judge_color_by_bgr(b[i], g[i], r[i], color[i]);		
	}
}




/*!
 * 
 */
unsigned tucube::get_min_item_index(unsigned* array, unsigned n)
{
	unsigned min_item_index = 0U;
	unsigned min_item = array[0];

	for(unsigned i = 0U; i < n; i++)
	{
		if(array[i] < min_item)
		{
			min_item = array[i];
			min_item_index = i;	
		}		
	}

	return min_item_index;
}


/*!
 * @brief: 根据对象中的recongnize_result[6][9]来生成描述魔方色块信息的字符串.
 * @note: //! recongnize_result最后存放的顺序是: D F L B R U (逆时针旋转)
 *        //! 最终describe中的顺序需要是: U R F D L B
 *        所以只能分别处理.
 */
void tucube::product_describe(void)
{
	//! U面第6个识别.
	for(unsigned i = 0U; i < 9U; i++)//!< 哪个色块.
	{
		this->describe[0 * 9 + i] = this->color_place[this->recongnize_result[5][i]];
	}
	this->describe[4] = 'u';//!< 手动指定中心块的颜色.

	//! R面第5个识别.
	for(unsigned i = 0U; i < 9U; i++)//!< 哪个色块.
	{
		this->describe[1 * 9 + i] = this->color_place[this->recongnize_result[4][i]];
	}
	this->describe[13] = 'r';//!< 手动指定中心块的颜色.

	//! F面第2个识别.
	for(unsigned i = 0U; i < 9U; i++)//!< 哪个色块.
	{
		this->describe[2 * 9 + i] = this->color_place[this->recongnize_result[1][i]];
	}
	this->describe[22] = 'f';//!< 手动指定中心块的颜色.

	//! D第1个识别.
	for(unsigned i = 0U; i < 9U; i++)//!< 哪个色块.
	{
		this->describe[3 * 9 + i] = this->color_place[this->recongnize_result[0][i]];
	}
	this->describe[31] = 'd';//!< 手动指定中心块的颜色.

	//! L面第3个识别.
	for(unsigned i = 0U; i < 9U; i++)//!< 哪个色块.
	{
		this->describe[4 * 9 + i] = this->color_place[this->recongnize_result[2][i]];
	}
	this->describe[40] = 'l';//!< 手动指定中心块的颜色.

	//! B面第4个识别.
	for(unsigned i = 0U; i < 9U; i++)//!< 哪个色块.
	{
		this->describe[5 * 9 + i] = this->color_place[this->recongnize_result[3][i]];
	}
	this->describe[49] = 'b';//!< 手动指定中心块的颜色.
}