#include <opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;


int getColSum(Mat src, int col) {           //ͳ�������ص��ܺ�
	int sum = 0;
	for (int j = 0; j < src.rows; j++) {
		sum += src.at<uchar>(j, col);
	}
	return sum;
}


int getRowSum(Mat src, int row) {       //ͳ�������ص��ܺ�
	int sum = 0;
	for (int j = 0; j < src.cols; j++) {
		sum += src.at<uchar>(row, j);
	}
	return sum;
}


void cutTop(Mat& src, Mat& dst) {      //�����и�
	int top, bottom;
	top = 0;
	bottom = src.rows;
	int i;
	for (i = 0; i < src.rows; ++i) {
		int RowValue = getRowSum(src, i);    //��������ɨ�裬��������һ�����غͲ�Ϊ0���У�ֹͣɨ�裬���м�Ϊtop
		if (RowValue > 0) {
			top = i;
			break;
		}
	}
	for (; i < src.rows; ++i) {           
		int RowValue = getRowSum(src, i);     //��top�м�������ɨ�裬֮��������һ�����غ�Ϊ0���У�ֹͣɨ�裬���м�Ϊbottom
		if (RowValue == 0) {
			bottom = i;
			break;
		}
	}
	int height = bottom - top;      //�и��ͼƬ�߶�
	Rect rect(0, top, src.cols, height);
	dst = src(rect).clone();       //�����и���ͼƬ
}

int cutLeft(Mat&src, Mat&LeftImg, Mat&RightImg) {     //�����и�
	int left, right;
	left = -1;
	right = src.cols;
	int i;
	for (i = 0; i < src.cols; ++i) {
		int ColValue = getColSum(src, i);     //��������ɨ��
		if (ColValue > 0) {     //�������غͲ�Ϊ0�ĵ�һ�У�ֹͣɨ�裬���м�Ϊleft
			left = i;
			break;
		}
	}
	if (left == -1) {     //���ͼƬ�����������е������غ;�Ϊ0����left��ֵ��Ϊ��ʼ����-1������1��������
		return 1;
	}

	for (; i < src.cols; ++i) {
		int ColValue = getColSum(src, i);    //��left ��һ�м�������ɨ�裬֮���������غ�Ϊ0�ĵ�һ�У�ֹͣɨ��,���м�Ϊright
		if (ColValue == 0) {
			right = i;
			break;
		}
	}
	int width = right - left;    //�и�ͼƬ�Ŀ��
	Rect rect(left, 0, width, src.rows);
	LeftImg = src(rect).clone();       //�и�����֣���߲��֣�ͼƬ
	Rect rectright(right, 0, src.cols - right, src.rows);
	RightImg = src(rectright).clone();   //�и����ֺ����µģ��ұ߲��֣�ͼƬ
	cutTop(LeftImg, LeftImg);      //������ͼƬ���������и�

	return 0;
}


int getPXSum(Mat srcImg)   //��ȡ�������غ�
{
	threshold(srcImg, srcImg, 248, 255, 0);  //��ֵ��(����������ʵ���Ѿ����˶�ֵ�����˲����ʡ��)
	int pixsum = 0;
	for (int i = 0; i < srcImg.rows; i++)
	{
		for (int j = 0; j < srcImg.cols; j++)
		{
			pixsum += srcImg.at <uchar>(i, j);
		}
	}
	return pixsum;   //����srcImg�������غ�
}


int main()
{
	Mat src = imread("taskNum.jpg", CV_LOAD_IMAGE_GRAYSCALE);      //��ȡԭͼƬ
	threshold(src, src, 248, 255, 1);    //��ֵ��

	Mat leftImg, rightImg;
	vector<int>vec;   //������Ϊvec��vector �������Դ���ʶ����
	int num = 0;    
	int reval = cutLeft(src, leftImg, rightImg);   //��cutLeft�ĺ���ֵ����reval,reval��Ϊwhile����������
	int pix = getPXSum(leftImg);   //��ȡ�и�ͼƬ�����ص���
	int i = 1;
	while (reval == 0) {     //��reval��=0,���и���ԭͼ�����غ�Ϊ0��û�����֣���ʱѭ�����������ٶ�ȡ����
		int mindist = 1000000;
		if (pix > 10000) {           //ȥ���и�ͼƬ���غͼ�С�Ĳ��֣�ɸѡ�������ֵ��и�ͼƬ
			char nameCut[90];
			sprintf_s(nameCut, "%dcutImg", i);
			i++;
			imshow(nameCut, leftImg);    //��ʾ�и���ͼƬ
			int n = 1;
			Mat pict;
			for (n; n < 10; n++) {
				char name[20];
				sprintf_s(name, "%dCutPc.jpg", n);
				Mat tem = imread(name, CV_LOAD_IMAGE_GRAYSCALE);    //��ͼ��ת��Ϊ�Ҷ�ͼ�ٷ���
				resize(tem, tem, Size(90, 120), CV_INTER_LINEAR);    //���и�ͼƬ��ģ���������ͬ��С
				resize(leftImg, leftImg, Size(90, 120), CV_INTER_LINEAR);
				absdiff(tem, leftImg, pict);        //��tem��leftImg֮��Ĳ�ľ���ֵ
				int difpix = getPXSum(pict);     //difpixΪpict�������غͣ���tem��leftImg����֮��ľ���ֵ
				cout << difpix<< endl;
				if (difpix< mindist) {       //���и�ͼƬ��ӽ���ģ�������Ϊʶ����
					mindist = difpix;
					num = n;
				}

			}
			cout << "\n\t��" << i - 1 << "������Ϊ" << num;
			cout << "\n\n\n\n";
			vec.push_back(num);      //��ʶ�����������ӵ�vec��ĩβ
		}
		Mat srcTmp = rightImg;     //���и��Ĳ�����Ϊԭͼ�����и�
		reval = cutLeft(srcTmp, leftImg, rightImg);
		pix = getPXSum(leftImg);
	}
	cout << "����ʶ����Ϊ��";
	for (int m = 0; m < vec.size(); m++) {
		cout << vec[m] << " ";       //���ʶ����
	}
	waitKey(0);
	return 0;
}

