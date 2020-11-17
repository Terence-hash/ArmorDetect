/*----------------------------
 *   @file：     ArmorDetector.h
 *   @date：     2020-7-10
 *   @brief：    装甲板识别 头文件
 *   @Author：   谭明瑞
 *----------------------------
 */
#ifndef ARMORDETECTOR_H
#define ARMORDETECTOR_H
#include<opencv2/opencv.hpp>
using namespace cv;


class Detect{
public:
	RotatedRect adjustRec(RotatedRect& rec);
	void process(Mat&src);
	
public:
	int enemy_color;
	int RED = 0;
	int BLUE = 1;

};
#endif // !ARMORDETECTOR_H

