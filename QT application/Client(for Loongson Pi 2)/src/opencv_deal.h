#ifndef OPENCV_DEAL_H
#define OPENCV_DEAL_H

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include <iostream>
#include <QImage>
#include <QDebug>
#include <pthread.h>

using namespace cv;
using namespace std;
//using namespace cv::face;


class OpencvDeal
{
public:
	OpencvDeal();

	int opencvDeal_init(void);
	QImage v4l22QIamge(uchar *data, int len);
	Mat QImage2cvMat(QImage image);
	QImage cvMat2QImage(const Mat& mat);
	void opencvDeal_DeInit(void);

public:
	QImage				QImgV4l2;			// 从V4L2数据转化成的QImage
	Mat					MatV4l2;			// v4l2->QImage->MatV4l2
	Mat					MatOpencv;			// MatV4l2 >> opencv >> MatOpencv
	QImage				QImgOpencv;			// 经opnecv处理后转化成QImage
//	pthread_mutex_t		imgV4Lock;			// QImgV4l2访问锁
//	pthread_mutex_t		imgOpLock;			// QImgOpencv访问锁
//	pthread_mutex_t		matV4Lock;			// MatV4l2访问锁

	/* C不能包含C++,未解 */
//	int 				detectFlag;			// 检测到人脸标记 1-有
//	Rect				vFacesRect;			// 人脸矩形容器

};



#endif // OPENCV_DEAL_H

