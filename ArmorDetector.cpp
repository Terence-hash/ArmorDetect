/*----------------------------
 *   @file：     ArmorDetector.cpp
 *   @date：     2020-7-10
 *   @brief：    装甲板识别 源文件
 *   @Author：   谭明瑞
 *----------------------------
 */
#include"ArmorDetector.h"
using namespace cv;
using namespace std;

/**********************
 funtion：  绘制包围装甲板的旋转矩形
 parameter：旋转矩形vector，输入的原图像
 return:    无
**********************/
void drawline(vector<RotatedRect> rec, Mat img)
{
	for (int i = 0; i < rec.size(); i++)
	{
		Point2f p[4];
		rec[i].points(p);
		line(img, p[0], p[1], Scalar(0, 255, 0), 2, 8, 0);
		line(img, p[1], p[2], Scalar(0, 255, 0), 2, 8, 0);
		line(img, p[2], p[3], Scalar(0, 255, 0), 2, 8, 0);
		line(img, p[3], p[0], Scalar(0, 255, 0), 2, 8, 0);
	}
}
/**********************
 funtion：  矫正矩形
 parameter：旋转矩形
 return:    矫正后的矩形
**********************/
RotatedRect Detect::adjustRec(RotatedRect& rec) {
	double width = rec.size.width;
	double height = rec.size.height;
	double angle = rec.angle;

	while (angle >= 90.0)  angle -= 180.0;
	while (angle < -90.0)  angle += 180.0;

	if (angle >= 45.0) {
		swap(width, height);
		angle -= 90.0;
	}
	else if (angle < -45.0) {
		swap(width, height);
		angle += 90.0;
	}
	return rec;

}
/**********************
 funtion：  识别装甲板
 parameter：原图像
 return:    无
**********************/
 void Detect::process(Mat&src){
		//src.convertTo(src, -1, 0.45, -20);//降低亮度
		vector<RotatedRect>lightInfos;   //用于储存轮廓拟合得到的旋转矩形
		double MaxVal;  
		Mat hsvImg;
		Mat splitImg;
		Mat binImg;
		Mat temImg;
		vector<Mat> channels;

		cvtColor(src, hsvImg, COLOR_BGR2HSV);
		//imshow("hsvImg", hsvImg); waitKey(0);
		split(hsvImg, channels);    //分离色彩通道

		int lowH, lowS, lowV;
		int highH, highS, highV;
		//根据敌方颜色选择不同的阈值范围
		if (enemy_color==BLUE) {
			splitImg = channels[2];
			lowH = 80;
			lowS = 60;
			lowV = 100;	
			highH = 150;
			highS = 255;
			highV = 255;

			minMaxLoc(splitImg, 0, &MaxVal, 0, 0);  //获得channels[2]最大值的指针
			threshold(splitImg, splitImg, MaxVal*0.98, 255, CV_THRESH_BINARY);     //筛选出亮度较高的一部分图像
			medianBlur(splitImg, splitImg, 3);       //消除图像中孤立的噪声点
			Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
			dilate(splitImg, splitImg, kernel);
			hsvImg.copyTo(temImg, splitImg);
			//imshow("temImg", temImg); waitKey(0);
		}
		else if (enemy_color == RED) {
			splitImg = channels[1];
			lowH = 0;
			lowS = 0;
			lowV = 110;
			highH = 255;
			highS = 255;
			highV = 255;

			minMaxLoc(splitImg, 0, &MaxVal, 0, 0);  //获得channels[1]最大值的指针
			threshold(splitImg, splitImg, MaxVal*0.85, 255, CV_THRESH_BINARY);     //筛选出亮度较高的一部分图像
			medianBlur(splitImg, splitImg, 3);       //消除图像中孤立的噪声点
			Mat kernel = getStructuringElement(MORPH_RECT, Size(4, 4));
			dilate(splitImg, splitImg, kernel, Point(-1, -1), 2);
			hsvImg.copyTo(temImg, splitImg);
			//imshow("temImg", temImg); waitKey(0);
		}
		inRange(temImg, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), binImg);  //二值化
		//imshow("binImg", binImg); waitKey(25);

		vector<vector<Point>>contours;
		findContours(binImg.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);   //在二值图像中寻找轮廓
		

		lightInfos.clear();

		for (const auto&contour : contours) {
			double area0 = contourArea(contour);
			//去掉极小的轮廓
			if (contour.size() <= 5 || area0 < 10) {
				continue;
			}
			RotatedRect litRect = fitEllipse(contour);   //拟合目标区域成为椭圆，返回一个旋转矩形
			litRect = adjustRec(litRect);   //矫正矩形
			double w_hRatio = litRect.size.width / litRect.size.height;   //宽高比
			double convex = area0 / litRect.size.area();   //凸度
			//去掉宽高比过大及凸度过小的一部分轮廓
			if ((w_hRatio > 1.0) || (convex < 0.5)) {
				continue;
			}
			
			//适当扩大灯条的范围
			litRect.size.width *= 1.2;
			litRect.size.height *= 1.2;

			if ((litRect.size.height > 10 && (litRect.size.height < 150) && (litRect.angle < 45 || litRect.angle>135))) {
				lightInfos.push_back(litRect);   //将轮廓拟合得到的旋转矩形存入lightInfos
			}
		}

		vector<RotatedRect> armorRects;   //用于储存包围装甲板的旋转矩形
		RotatedRect armorRect;   //定义包围装甲板的旋转矩形
		armorRects.clear();
	

		//将灯条按照灯条中心的x坐标由小到大排序
		sort(lightInfos.begin(), lightInfos.end(), [](const RotatedRect& rec_1, const RotatedRect& rec_2)
		{	return rec_1.center.x < rec_2.center.x;
		});

		//开始遍历轮廓拟合得到的旋转矩形，获得包围装甲板的旋转矩形
		for (size_t i = 0; i < lightInfos.size(); i++)
		{
			for (size_t j = i + 1; j < lightInfos.size(); j++)
			{
				const RotatedRect& lightLeft = lightInfos[i];
				const RotatedRect& lightRight = lightInfos[j];

				double heightDiff = abs(lightLeft.size.height - lightRight.size.height);    //高度差
				double widthDiff = abs(lightLeft.size.width - lightRight.size.width);   //宽度差
				double angleDiff = abs(lightLeft.angle - lightRight.angle);    //角度差
				double yDiff = abs(lightLeft.center.y - lightRight.center.y);    //y坐标之差
				double xDiff = abs(lightLeft.center.x - lightRight.center.x);    //x坐标之差
				double meanheight = (lightLeft.size.height + lightRight.size.height) / 2;    //平均高度
				double yDiffRatio = yDiff / meanheight;    //y坐标差比率
				double xDiffRatio = xDiff / meanheight;    //x坐标差比率
				double dist = sqrt((lightLeft.center.x - lightRight.center.x)*(lightLeft.center.x - lightRight.center.x) + (lightLeft.center.y - lightRight.center.y)*(lightLeft.center.y - lightRight.center.y));  //距离
				double distRatio = dist / meanheight;   //距离差比率
				//float heightDiff_ratio = heightDiff / max(lightLeft.size.height, lightRight.size.height);   //高度差比率

				//根据包围装甲板的旋转矩形的特征进行筛选
				if (angleDiff > 8 || xDiffRatio < 0.5 || yDiffRatio>0.7 || distRatio > 3 || distRatio < 1)
					continue;
			    //获得包围装甲板的旋转矩形的中心位置，角度，高度，宽度
				armorRect.center.x = (lightLeft.center.x + lightRight.center.x) / 2;
				armorRect.center.y = (lightLeft.center.y + lightRight.center.y) / 2;
				armorRect.angle = (lightLeft.angle + lightRight.angle) / 2;
				if (180 - angleDiff < 10) { armorRect.angle += 90; }
				armorRect.size.height = (lightLeft.size.height + lightRight.size.height) / 2;
				armorRect.size.width = sqrt((lightLeft.center.x - lightRight.center.x)*(lightLeft.center.x - lightRight.center.x) + (lightLeft.center.y - lightRight.center.y)*(lightLeft.center.y - lightRight.center.y));

				//矫正使矩形框的宽度大于或等于高度
				double rec_H = armorRect.size.height;
				double rec_W = armorRect.size.width;
				if (rec_H < rec_W)
				{
					armorRect.size.height = rec_H;
					armorRect.size.width = rec_W;
				}
				else
				{
					armorRect.size.height = rec_W;
					armorRect.size.width = rec_H;
				}

				armorRects.emplace_back(armorRect);  //将包围装甲板的旋转矩形存入armorRects中
			}
		}
		if (armorRects.empty())
			cout << "No Armor!" << endl;

		//绘制包围装甲板的旋转，显示装甲板识别图
		drawline(armorRects, src);
		namedWindow("装甲板识别图", 0);
		imshow("装甲板识别图", src);
		waitKey(20);
	
}

