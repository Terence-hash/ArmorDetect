#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<math.h>
#include<iostream>
#include<algorithm>
#include"ArmorDetector.h"
using namespace std;
using namespace cv;


int main() {
	Detect armor;
	armor.enemy_color = armor.BLUE;    //ѡ��з�������ɫ

	VideoCapture capture("�����ز���������-ev-0.MOV");    //��ȡ��Ƶ
	while (capture.isOpened()) {
		Mat frame;
		capture >> frame;
		if (frame.empty()) {
			break;
		}
		armor.process(frame);     //��ʼʶ��
	}
	return 0;
}

/**�ο�����
https://blog.csdn.net/weixin_42755384/article/details/88530186?utm_medium=distribute.pc_relevant.none-task-blog-searchFromBaidu-7.nonecase&depth_1-utm_source=distribute.pc_relevant.none-task-blog-searchFromBaidu-7.nonecase
https://www.cnblogs.com/IaCorse/p/11567355.html
https://blog.csdn.net/u010750137/article/details/96428059
https://blog.csdn.net/Taunt_/article/details/99894463
**/



