#ifndef OPENCV_DEAL_H
#define OPENCV_DEAL_H

#include <opencv2/opencv.hpp>  
#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <QImage>
#include <QDebug>
#include <pthread.h>
#include "usermngr.h"

using namespace cv;
using namespace std;
using namespace cv::face;


#define ROW_MIN			60
#define MAT_SCALE		3


#define LBPH_REC_THRES_0		125.0	// LBPH设定阈值/相似度0
#define LBPH_REC_THRES_80		85.0	// LBPH识别过滤阈值/相似度80
#define LBPH_REC_THRES_100		65.0	// LBPH相似度100阈值

class FaceImageDeal
{
public:
	FaceImageDeal(void);
	int faceDeal_init(void);
    int detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale);
//	int face_recognize();

	QImage v4l22QIamge(uchar *data, int len);
	Mat QImage2cvMat(QImage image);
	QImage cvMat2QImage(const Mat& mat);
	void faceDeal_deInit(void);

public:
//	Ptr<LBPHFaceRecognizer> modelLBPH;		// 训练的人脸库分类器
	CascadeClassifier 	face_cascade;		//人脸检测分类器
//	Mat 				facesMat;			// 检测提取的人脸
//	int 				predictLBPH;		// 识别结果
//	string 				fn_csv;				// csv文件
	QImage				QImgV4l2;			// 从V4L2数据转化成的QImage
	Mat					MatV4l2;			// v4l2->QImage->MatV4l2
	Mat					MatOpencv;			// MatV4l2 >> opencv >> MatOpencv
	QImage				QImgOpencv;			// 经opnecv处理后转化成QImage
	pthread_mutex_t		imgV4Lock;			// QImgV4l2访问锁
	pthread_mutex_t		imgOpLock;			// QImgOpencv访问锁
	pthread_mutex_t		matV4Lock;			// MatV4l2访问锁
	int 				detectFlag;			// 检测到人脸标记 1-有
	vector<Rect>		vFacesRect;			// 人脸矩形容器

private:
	
};

class FaceRecogn
{
public:
	FaceRecogn(void);
	int faceRec_init(void);
	int face_recognize(void);
	void faceRec_deInit(void);

public:
	string						fn_csv; 					// csv文件
	Mat 						facesMat;					// 检测提取的人脸
	int 						predFace;					// 人脸编号 0-查无此人，>0-对应编号
	char						predName[USER_NAME_LEN];	// user name
	uchar						faceConfd;					// 相似度/置信度
	int							predChang;					// 更新结果

private:
	Ptr<LBPHFaceRecognizer> 	modelLBPH;			// LBPH训练的人脸模型
	Ptr<BasicFaceRecognizer> 	modelPCA;			// PCA训练的人脸模型
	Ptr<BasicFaceRecognizer> 	modelFisher;		// Fisher训练的人脸模型
	int 						predictLBPH;		// LBPH识别结果
	int 						predictPCA;			// PCA识别结果
	int 						predictFisher;		// Fisher识别结果
	
	
};


/* 图像处理/格式转化线程 */
void* faceopencv_handle(void *arg);

/* 人脸检测识别线程 */
void* faceDetRec_handle(void *arg);

/* opencv_deal模块去初始化 */
void opencvDeal_deInit(void);

/* 重启opencv两个线程 */
int opencvDeal_restart(void);

#endif // OPENCV_DEAL_H
