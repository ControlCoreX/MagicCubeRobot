#ifndef _TUCUBE_TUCUBE_H_
#define _TUCUBE_TUCUBE_H_

#include "opencv2/opencv.hpp"

using namespace std;

#define BGRS_FILE_PATH ".//bgrs.txt"

typedef unsigned color_t;
#define PLACE_0 'u'
#define PLACE_1 'r'
#define PLACE_2 'f'
#define PLACE_3 'd'
#define PLACE_4 'l'
#define PLACE_5 'b' 
class tucube
{
public:
	tucube(unsigned step);
	void load_bgrs_from_file(void);
	void store_bgrs_to_file(void);
	void get_roi_from_user(cv::Rect roi_rect);
	void calc_samp_area(void);
	void recongnize_frame(cv::Mat& frame, color_t colors[9]);
	void show_recongnize_result(cv::Mat& frame, color_t colors[9]);
	void store_recongnize_result(color_t colors[9], uint8_t index);
	void show_final_recongnize_result(uint8_t choice);
	void color_one_face(cv::Mat& face, cv::Scalar colors[9]);
	void color_t2Scalar(color_t color1, cv::Scalar& color2);
	void calc_rect_bgr(cv::Mat& img, cv::Rect& rect, unsigned& b, unsigned& g, unsigned& r);
	void calc_frame_bgr(cv::Mat& frame, unsigned b[9], unsigned g[9], unsigned r[9]);
	unsigned get_min_item_index(unsigned* array, unsigned n);
	void judge_color_by_bgr(unsigned b, unsigned g, unsigned r, color_t& color);
	void judge_frame_color(color_t color[9], unsigned b[9], unsigned g[9], unsigned r[9]);
	void product_describe(void);
	char bgrs_file_path[20];
	cv::Rect roi_rect;
	cv::Rect samp_area[3][3];
	unsigned step;
	unsigned bgrs[18];
	color_t recongnize_result[6][9];
	uint8_t color_place[6];
	uint8_t describe[6 * 9];
	uint8_t slove[100];
};


#endif /* _TUCUBE_TUCUBE_H_ */
