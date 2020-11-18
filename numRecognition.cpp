#include <opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;


int getColSum(Mat src, int col) {           //统计列像素的总和
	int sum = 0;
	for (int j = 0; j < src.rows; j++) {
		sum += src.at<uchar>(j, col);
	}
	return sum;
}


int getRowSum(Mat src, int row) {       //统计行像素的总和
	int sum = 0;
	for (int j = 0; j < src.cols; j++) {
		sum += src.at<uchar>(row, j);
	}
	return sum;
}


void cutTop(Mat& src, Mat& dst) {      //上下切割
	int top, bottom;
	top = 0;
	bottom = src.rows;
	int i;
	for (i = 0; i < src.rows; ++i) {
		int RowValue = getRowSum(src, i);    //从上向下扫描，当遇到第一个像素和不为0的行，停止扫描，此列记为top
		if (RowValue > 0) {
			top = i;
			break;
		}
	}
	for (; i < src.rows; ++i) {           
		int RowValue = getRowSum(src, i);     //从top行继续向下扫描，之后当遇到第一个像素和为0的行，停止扫描，此列记为bottom
		if (RowValue == 0) {
			bottom = i;
			break;
		}
	}
	int height = bottom - top;      //切割的图片高度
	Rect rect(0, top, src.cols, height);
	dst = src(rect).clone();       //上下切割后的图片
}

int cutLeft(Mat&src, Mat&LeftImg, Mat&RightImg) {     //左右切割
	int left, right;
	left = -1;
	right = src.cols;
	int i;
	for (i = 0; i < src.cols; ++i) {
		int ColValue = getColSum(src, i);     //从左向右扫描
		if (ColValue > 0) {     //遇到像素和不为0的第一列，停止扫描，此列记为left
			left = i;
			break;
		}
	}
	if (left == -1) {     //如果图片从左到右所有列的列像素和均为0，则left的值仍为初始化的-1，返回1结束函数
		return 1;
	}

	for (; i < src.cols; ++i) {
		int ColValue = getColSum(src, i);    //从left 这一列继续向右扫描，之后遇到像素和为0的第一列，停止扫描,此列记为right
		if (ColValue == 0) {
			right = i;
			break;
		}
	}
	int width = right - left;    //切割图片的宽度
	Rect rect(left, 0, width, src.rows);
	LeftImg = src(rect).clone();       //切割的数字（左边部分）图片
	Rect rectright(right, 0, src.cols - right, src.rows);
	RightImg = src(rectright).clone();   //切割数字后余下的（右边部分）图片
	cutTop(LeftImg, LeftImg);      //对数字图片进行上下切割

	return 0;
}


int getPXSum(Mat srcImg)   //获取所有像素和
{
	threshold(srcImg, srcImg, 248, 255, 0);  //二值化(赋给函数的实参已经过了二值化，此步骤可省略)
	int pixsum = 0;
	for (int i = 0; i < srcImg.rows; i++)
	{
		for (int j = 0; j < srcImg.cols; j++)
		{
			pixsum += srcImg.at <uchar>(i, j);
		}
	}
	return pixsum;   //返回srcImg的总像素和
}


int main()
{
	Mat src = imread("taskNum.jpg", CV_LOAD_IMAGE_GRAYSCALE);      //读取原图片
	threshold(src, src, 248, 255, 1);    //二值化

	Mat leftImg, rightImg;
	vector<int>vec;   //创建名为vec的vector 容器用以储存识别结果
	int num = 0;    
	int reval = cutLeft(src, leftImg, rightImg);   //将cutLeft的函数值赋给reval,reval作为while函数的条件
	int pix = getPXSum(leftImg);   //获取切割图片的像素点数
	int i = 1;
	while (reval == 0) {     //当reval！=0,即切割后的原图中像素和为0，没有数字，此时循环结束，不再读取数字
		int mindist = 1000000;
		if (pix > 10000) {           //去除切割图片像素和极小的部分，筛选出含数字的切割图片
			char nameCut[90];
			sprintf_s(nameCut, "%dcutImg", i);
			i++;
			imshow(nameCut, leftImg);    //显示切割后的图片
			int n = 1;
			Mat pict;
			for (n; n < 10; n++) {
				char name[20];
				sprintf_s(name, "%dCutPc.jpg", n);
				Mat tem = imread(name, CV_LOAD_IMAGE_GRAYSCALE);    //将图像转换为灰度图再返回
				resize(tem, tem, Size(90, 120), CV_INTER_LINEAR);    //将切割图片与模板调整成相同大小
				resize(leftImg, leftImg, Size(90, 120), CV_INTER_LINEAR);
				absdiff(tem, leftImg, pict);        //求tem与leftImg之间的差的绝对值
				int difpix = getPXSum(pict);     //difpix为pict的总像素和，即tem与leftImg像素之差的绝对值
				cout << difpix<< endl;
				if (difpix< mindist) {       //与切割图片最接近的模板的数字为识别结果
					mindist = difpix;
					num = n;
				}

			}
			cout << "\n\t第" << i - 1 << "个数字为" << num;
			cout << "\n\n\n\n";
			vec.push_back(num);      //将识别出的数字添加到vec的末尾
		}
		Mat srcTmp = rightImg;     //用切割后的部分作为原图继续切割
		reval = cutLeft(srcTmp, leftImg, rightImg);
		pix = getPXSum(leftImg);
	}
	cout << "数字识别结果为：";
	for (int m = 0; m < vec.size(); m++) {
		cout << vec[m] << " ";       //输出识别结果
	}
	waitKey(0);
	return 0;
}

