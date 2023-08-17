#define _CRT_SECURE_NO_WARNINGS

#include "tucube.h"
#include <cstring>
#include <cstdio>

tucube::tucube(unsigned step = 2U)
{
	this->step = step;

	this->color_place[0] = PLACE_0;
	this->color_place[1] = PLACE_1;
	this->color_place[2] = PLACE_2;
	this->color_place[3] = PLACE_3;
	this->color_place[4] = PLACE_4;
	this->color_place[5] = PLACE_5;

	load_bgrs_from_file();
	cout << "cube construct successful!" << endl;
}

void tucube::load_bgrs_from_file(void)
{
	FILE *bgr_file = fopen(BGRS_FILE_PATH, "rb");
	if(bgr_file == NULL)
	{
		cout << "bgrs file open failed!" << endl;
		exit(-1);
	}

	fseek(bgr_file, 0, SEEK_SET);
	fread(this->bgrs, 4, 18, bgr_file);
	fclose(bgr_file);
	cout << "load bgrs form file successful!" << endl;
}
void tucube::store_bgrs_to_file(void)
{
	FILE* bgr_file = fopen(BGRS_FILE_PATH, "wb");
	if(bgr_file == NULL)
	{
		cout << "bgrs file open failed!" << endl;
		exit(-1);
	}
	
	fseek(bgr_file, 0, SEEK_SET);
	fwrite(this->bgrs, 4, 18, bgr_file);
	fclose(bgr_file);
	cout << "store bgrs to file successful!" << endl;
}
void tucube::get_roi_from_user(cv::Rect roi_rect)
{
	this->roi_rect = roi_rect;

	cout << "get roi_rect successful!" << endl;
	cout << "roi_rect = " << this->roi_rect << endl;
}

void tucube::calc_samp_area(void)
{
	unsigned divx   = (unsigned)(this->roi_rect.width  / 3U);
	unsigned divy   = (unsigned)(this->roi_rect.height / 3U);

	unsigned edgex  = (unsigned)(this->roi_rect.width  / 12U);
	unsigned edgey  = (unsigned)(this->roi_rect.height / 12U);
	
	unsigned width  = (unsigned)((this->roi_rect.width  - 6 * edgex) / 3U);
	unsigned height = (unsigned)((this->roi_rect.height - 6 * edgey) / 3U);

	for(unsigned i = 0U; i < 3U; i++)
	{
		for(unsigned j = 0U; j < 3U; j++)
		{
			this->samp_area[i][j] = cv::Rect(roi_rect.tl().x + j * divx + edgex,
				                         roi_rect.tl().y + i * divy + edgey, width, height);
		}	
	}	
	cout << "calculate sample area successful!" << endl;
}
void tucube::recongnize_frame(cv::Mat& frame, color_t colors[9])
{
	unsigned b[9], g[9], r[9];

	this->calc_frame_bgr(frame, b, g, r);
	this->judge_frame_color(colors, b, g, r);
}
void tucube::show_recongnize_result(cv::Mat& frame, color_t colors[9])
{
	unsigned b, g, r;
	cv::Scalar color;

	for(unsigned i = 0U; i < 3U; i++)
	{
		for(unsigned j = 0U; j < 3U; j++)
		{
			this->color_t2Scalar(colors[i * 3U + j], color);
			
			rectangle(frame, this->samp_area[i][j], color, -1);	
		}	
	}
}
void tucube::store_recongnize_result(color_t colors[9], uint8_t index)
{
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->recongnize_result[index][i] = colors[i];
	}
}
void tucube::show_final_recongnize_result(uint8_t choice)
{
	if(choice == 'a')
	{
		cv::Mat result(342, 342, CV_8UC3, cv::Scalar(255, 255, 255));
		cv::Scalar color[9];

		for(unsigned i = 0U; i < 6U; i++)
		{
			for(unsigned j = 0U; j < 9U; j++)
			{
				this->color_t2Scalar(this->recongnize_result[i][j], color[j]);
			}
			this->color_one_face(result, color);
			imshow("recongnize result", result);
			cout << "face " << i + 1U << endl;
			cv::waitKey();
		}
	}
	else if(choice == 'b')
	{
		// uint32_t background_width = ;
		// uint32_t background_height = ;
		// uint32_t edge = ;

		// Mat background(background_height + edge * 2U, background_width + edge * 2U, CV_8UC3, Scalar(255, 255, 255));
		// Mat face(, , CV_8UC3, Scalar(255, 255, 255));

		// Mat U = background(Rect(5, 317, 312, 312));
		// Mat R = background(Rect())
	}
	cv::destroyWindow("recongnize result");
}

void tucube::color_one_face(cv::Mat& face, cv::Scalar colors[9])
{
	unsigned interval = 3U;
	unsigned w = (face.cols  - 4 * interval) / 3;
	unsigned h = (face.rows - 4 * interval) / 3;
	rectangle(face, cv::Rect(interval,             interval,             w, h), colors[0], -1);
	rectangle(face, cv::Rect(interval * 2 + w,     interval,             w, h), colors[1], -1);
	rectangle(face, cv::Rect(interval * 3 + w * 2, interval,             w, h), colors[2], -1);
	rectangle(face, cv::Rect(interval,             interval * 2 + h,     w, h), colors[3], -1);
	rectangle(face, cv::Rect(interval * 2 + w,     interval * 2 + h,     w, h), colors[4], -1);
	rectangle(face, cv::Rect(interval * 3 + w * 2, interval * 2 + h,     w, h), colors[5], -1);
	rectangle(face, cv::Rect(interval,             interval * 3 + h * 2, w, h), colors[6], -1);
	rectangle(face, cv::Rect(interval * 2 + w,     interval * 3 + h * 2, w, h), colors[7], -1);
	rectangle(face, cv::Rect(interval * 3 + w * 2, interval * 3 + h * 2, w, h), colors[8], -1);
}
void tucube::color_t2Scalar(color_t color1, cv::Scalar& color2)
{
	switch(color1)
	{
		case 0: color2 = cv::Scalar(this->bgrs[0 * 3 + 0], this->bgrs[0 * 3 + 1], this->bgrs[0 * 3 + 2]); break;
		case 1: color2 = cv::Scalar(this->bgrs[1 * 3 + 0], this->bgrs[1 * 3 + 1], this->bgrs[1 * 3 + 2]); break;
		case 2: color2 = cv::Scalar(this->bgrs[2 * 3 + 0], this->bgrs[2 * 3 + 1], this->bgrs[2 * 3 + 2]); break;
		case 3: color2 = cv::Scalar(this->bgrs[3 * 3 + 0], this->bgrs[3 * 3 + 1], this->bgrs[3 * 3 + 2]); break;
		case 4: color2 = cv::Scalar(this->bgrs[4 * 3 + 0], this->bgrs[4 * 3 + 1], this->bgrs[4 * 3 + 2]); break;
		case 5: color2 = cv::Scalar(this->bgrs[5 * 3 + 0], this->bgrs[5 * 3 + 1], this->bgrs[5 * 3 + 2]); break;
		default: cout << "match failed in 'color_t2scalar'!" << endl;
	}
}
void tucube::calc_rect_bgr(cv::Mat& img, cv::Rect& rect, unsigned& b, unsigned& g, unsigned& r)
{
    float sumb = 0.0f, sumg = 0.0f, sumr = 0.0f;
	unsigned point_cnt = 0U;
    for(unsigned row = 0U; row < rect.height; row += step)
    {
        for(unsigned col = 0U; col < rect.width; col += step)
        {
			sumb += img.at<cv::Vec3b>(rect.tl().y + row, rect.tl().x + col)[0];
			sumg += img.at<cv::Vec3b>(rect.tl().y + row, rect.tl().x + col)[1];
			sumr += img.at<cv::Vec3b>(rect.tl().y + row, rect.tl().x + col)[2];
            point_cnt++;
        }
    }
    b = (unsigned)(sumb / point_cnt);
    g = (unsigned)(sumg / point_cnt);
    r = (unsigned)(sumr / point_cnt);
}
void tucube::calc_frame_bgr(cv::Mat& frame, unsigned b[9], unsigned g[9], unsigned r[9])
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
void tucube::judge_color_by_bgr(unsigned b, unsigned g, unsigned r, color_t& color)
{
	unsigned square_of_b_distance[6];
	unsigned square_of_g_distance[6];
	unsigned square_of_r_distance[6];
	unsigned average_distance[6];
	for(unsigned i = 0U; i < 6U; i++)//!< which color.
	{
		square_of_b_distance[i] = (b - this->bgrs[i * 3 + 0]) * (b - this->bgrs[i * 3 + 0]);
		square_of_g_distance[i] = (g - this->bgrs[i * 3 + 1]) * (g - this->bgrs[i * 3 + 1]); 
		square_of_r_distance[i] = (r - this->bgrs[i * 3 + 2]) * (r - this->bgrs[i * 3 + 2]); 
		average_distance[i] = (unsigned)sqrt(square_of_b_distance[i] + square_of_g_distance[i] + square_of_r_distance[i]);
	}	
	color = this->get_min_item_index(average_distance, 6U);
}
void tucube::judge_frame_color(color_t color[9], unsigned b[9], unsigned g[9], unsigned r[9])
{
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->judge_color_by_bgr(b[i], g[i], r[i], color[i]);		
	}
}
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
void tucube::product_describe(void)
{
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->describe[0 * 9 + i] = this->color_place[this->recongnize_result[5][i]];
	}
	this->describe[4] = 'u';
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->describe[1 * 9 + i] = this->color_place[this->recongnize_result[4][i]];
	}
	this->describe[13] = 'r';
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->describe[2 * 9 + i] = this->color_place[this->recongnize_result[1][i]];
	}
	this->describe[22] = 'f';
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->describe[3 * 9 + i] = this->color_place[this->recongnize_result[0][i]];
	}
	this->describe[31] = 'd';
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->describe[4 * 9 + i] = this->color_place[this->recongnize_result[2][i]];
	}
	this->describe[40] = 'l';
	for(unsigned i = 0U; i < 9U; i++)
	{
		this->describe[5 * 9 + i] = this->color_place[this->recongnize_result[3][i]];
	}
	this->describe[49] = 'b';
}
