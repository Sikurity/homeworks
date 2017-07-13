#define _CRT_SECURE_NO_WARNINGS
#define ANSI

#include <iostream>
#include "opencv.hpp"

using namespace cv;
using namespace std;

IplImage *bilinear_interpolation(IplImage *src, float dx, float dy)
{
	int test;
	int x, y;
	IplImage *dst = cvCreateImage(cvSize((int)(src->width * dx), (int)(src->height * dy)), src->depth, src->nChannels);

	float const x_ratio = (src->width - 1) / (float)dst->width;
	float const y_ratio = (src->height - 1) / (float)dst->height;

	//cout << dst->height << "/" << dst->width << endl;

	for(int row = 0; row != dst->height; ++row)
	{
		int y = row * y_ratio;
		float y_d1 = (row * y_ratio) - y;
		float y_d2 = 1 - y_d1;

		for(int col = 0; col != dst->width; ++col)
		{
			int x = col * x_ratio;
			float x_d1 = (col * x_ratio) - x;
			float x_d2 = 1 - x_d1;
			
			float y2_x2 = y_d2 * x_d2;
			float y2_x1 = y_d2 * x_d1;
			float y1_x2 = y_d1 * x_d2;
			float y1_x1 = y_d1 * x_d1;

			for(int channel = 0; channel != src->nChannels; channel++)
			{
				int index1 = y * src->widthStep + x * src->nChannels + channel;
				int index2 = y * src->widthStep + (x + 1) * src->nChannels + channel;
				int index3 = (y + 1) * src->widthStep + x * src->nChannels + channel;
				int index4 = (y + 1) * src->widthStep + (x + 1) * src->nChannels + channel;

				//cout << row << "," << col << "," << channel << ":" << row * dst->widthStep + col * dst->nChannels + channel << endl;

				dst->imageData[row * dst->widthStep + col * dst->nChannels + channel] = 
					y2_x2 * (unsigned char)src->imageData[index1] +
					y2_x1 * (unsigned char)src->imageData[index2] +
					y1_x2 * (unsigned char)src->imageData[index3] +
					y1_x1 * (unsigned char)src->imageData[index4];
			}
		}
	}

	return dst;
}

int main()
{
	int percent = 200;
	IplImage *src = cvLoadImage("images.jpg", 1);
	IplImage *dst = cvCreateImage(cvSize((int)((src->width) * percent / 100.0f), (int)((src->height) * percent / 100.0f)), src->depth, src->nChannels);

	namedWindow("1", WINDOW_AUTOSIZE);
	cvShowImage("1", src);

	cvResize(src, dst, INTER_LINEAR);

	namedWindow("2", WINDOW_AUTOSIZE);
	cvShowImage("2", dst);

	namedWindow("3", WINDOW_AUTOSIZE);
	cvShowImage("3", bilinear_interpolation(src, 3.0f, 3.0f));

	cvWaitKey(0);

	return 0;
}