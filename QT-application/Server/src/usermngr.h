#ifndef USERMNGR_H
#define USERMNGR_H

#include "opencv2/objdetect.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <QImage>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

using namespace cv;
using namespace std;


#define FACES_LIB_PATH		"faces"

#define USER_NAME_LEN		64

struct userInfo_Stru
{
	int 	seq;					// seq number
	char	name[USER_NAME_LEN];	// user name
};

struct userMngr_Stru
{
	int 	getFaceSta;		// 添加人脸 1-进行中 0-无
	int 	photoFlag;		// 拍照开关
	int 	addFaceOK;		// add face success

	Mat					MatCapture;			// capture cvMat format
	QImage				QImgCapture;		// capture QImage format
	QImage				QImgFace;			// face QImage format
	pthread_mutex_t		MatCapLock;			// QImgCapture lock
	pthread_mutex_t		QImgFaceLock;		// QImgCapture lock

	int 	userChange;		// user infomation change flag
	int		userSum;		// user list sum
	struct userInfo_Stru	*pstUserInfo;		// user name information
};


void userMngr_Init(struct userMngr_Stru *userMnStru);
void userMngr_DeInit(struct userMngr_Stru *userMnStru);

void* addFace_pthread(void *arg);

int checkDirExist(char *basePath, char *headDirStr);
int get_dir(char *inDir, char *inName, char *outDir);
int get_face(char *path);
int resize_save(Mat& faceIn, char *path, int FaceSeq);
int Create_CSV(char *dir_path);

int get_UserList(struct userInfo_Stru **ppUserList, int *Count);
int del_UserFace(char *facePath);

int remove_dir(const char *dir);

#endif // USERMNGR_H

