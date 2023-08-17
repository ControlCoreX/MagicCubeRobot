#ifndef _TUCUBE_TUCUBE_H_
#define _TUCUBE_TUCUBE_H_

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

#define BGRS_FILE_PATH "../docs/bgrs.txt"


typedef unsigned color_t;
#define PLACE_0 'u' // U: 'b' -> 0
#define PLACE_1 'r' // R: 'y' -> 1
#define PLACE_2 'f' // F: 'r' -> 2
#define PLACE_3 'd' // D: 'g' -> 3
#define PLACE_4 'l' // L: 'w' -> 4
#define PLACE_5 'b' // B: 'o' -> 5
//! 魔方放置的方位: 红色朝前, 蓝色朝上, 黄色朝右.
//! 拍摄6个面的顺序为: 下, 前, 左, 后, 右, 上 (这个问题在函数里面解决过了, 已经适配到机械结构了)


class tucube
{
public:
	tucube(unsigned step);

	void load_bgrs_from_file(void);
	void store_bgrs_to_file(void);

	void get_roi_from_user(Rect roi_rect);
	void calc_samp_area(void);

	void recongnize_frame(Mat& frame, color_t colors[9]);
	void show_recongnize_result(Mat& frame, color_t colors[9]);
	void store_recongnize_result(color_t colors[9], uint8_t index);

	void show_final_recongnize_result(uint8_t choice);
	void color_one_face(Mat& face, Scalar colors[9]);

	void color_t2Scalar(color_t color1, Scalar& color2);

	void calc_rect_bgr(Mat& img, Rect& rect, unsigned& b, unsigned& g, unsigned& r);
	void calc_frame_bgr(Mat& frame, unsigned b[9], unsigned g[9], unsigned r[9]);

	unsigned get_min_item_index(unsigned* array, unsigned n);

	void judge_color_by_bgr(unsigned b, unsigned g, unsigned r, color_t& color);
	void judge_frame_color(color_t color[9], unsigned b[9], unsigned g[9], unsigned r[9]);

	void product_describe(void);//!< 生成魔方色块信息描述字符串.


	char bgrs_file_path[20];//!< 存储bgrs数组的文件的路径.

	Rect roi_rect;//!< roi区域.
	Rect samp_area[3][3];//!< 采样区域.
	unsigned step;//!< 采样步长. 速度与准确度的权衡.

	unsigned bgrs[18];//!< 存储6种颜色的bgr值. (这里存成一维数组方便一些).

	color_t recongnize_result[6][9];//!< 最终的识别结果.

	uint8_t color_place[6];
	uint8_t describe[6 * 9];//!< 魔方色块信息描述字符串.
	uint8_t slove[100];//!< 魔方解法描述字符串.
};


#endif /* _TUCUBE_TUCUBE_H_ */
