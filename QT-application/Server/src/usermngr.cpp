#include "usermngr.h"
#include "opencv_deal.h"
#include "mainwindow.h"


struct userMngr_Stru		gstUsrMngr;

extern FaceImageDeal		gFaceDeal;


void userMngr_Init(struct userMngr_Stru *userMnStru)
{

	// gstUsrMngr 
	userMnStru->photoFlag = 0;
	userMnStru->addFaceOK = 0;

	userMnStru->userChange = 0;
	userMnStru->userSum = 0;
	userMnStru->pstUserInfo = NULL;
	
	pthread_mutex_init(&userMnStru->MatCapLock, NULL);
	pthread_mutex_init(&userMnStru->QImgFaceLock, NULL);

	get_UserList(&userMnStru->pstUserInfo, &userMnStru->userSum);
	
}

// 获取用户名单列表, ppUserList输出指针, Count数量
int get_UserList(struct userInfo_Stru **ppUserList, int *Count)
{
	struct stat statbuf;
	DIR *dir;
	struct dirent *dirp;
	char 	tmpDirPath[128] = {0};
	char	label[10] = {0};
	struct userInfo_Stru 	tmpUserInfo;
	struct userInfo_Stru 	*pUserMem;
	int 	dirCnt = 0;
	int		dirNum = 0;
	int 	i;

	// 获取文件属性
	if(lstat(FACES_LIB_PATH, &statbuf) < 0)
	{
		printf("%s: lstat(%s) failed !\n", __FUNCTION__, FACES_LIB_PATH);
		return -1;
	}

	// 判断是否为目录
	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s: %s is not dir !\n", __FUNCTION__, FACES_LIB_PATH); 
		return -1;
	}

	// 打开目录
	dir = opendir(FACES_LIB_PATH);
	if( dir ==NULL)
	{
		printf("opendir failed.\n");
		return -1;
	}
	// 遍历一级目录，统计数量
	while((dirp = readdir(dir)) != NULL)
	{
		// 忽略 '.' '..'文件（linux）
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;
		
		dirCnt ++;
	}
	closedir(dir);

	*Count = dirCnt;	// total dir count
	
	pUserMem = (struct userInfo_Stru *)malloc((*Count) *sizeof(struct userInfo_Stru));
	if(pUserMem == NULL)
	{
		printf("%s: malloc for pUserMem failed.\n", __FUNCTION__);
		return -1;
	}

	// 打开目录
	dir = opendir(FACES_LIB_PATH);
	if( dir ==NULL)
	{
		free(pUserMem);
		printf("opendir failed.\n");
		return -1;
	}
	// 遍历一级目录，获取信息
	while((dirp = readdir(dir)) != NULL)
	{
		// 忽略 '.' '..'文件（linux）
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;

		// 将1级目录与2级目录组合
		memset(tmpDirPath, 0, sizeof(tmpDirPath));
		strcat(tmpDirPath, FACES_LIB_PATH);
		strcat(tmpDirPath, "/");
		strcat(tmpDirPath, dirp->d_name);

		if(lstat(tmpDirPath, &statbuf) < 0)
		{
			printf("%s: lstat(%s) failed !\n", __FUNCTION__, tmpDirPath);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode) != 1)
		{
			printf("%s: %s is not dir !\n", __FUNCTION__, tmpDirPath); 
			continue;
		}

		memset(label, 0, sizeof(label));
		for(i=0; i<10; i++) 	// 取序号
		{
			if(dirp->d_name[i] != '_')
				label[i] = dirp->d_name[i];
			else
				break;
		}
		if(i == 10)
			continue; 

		// get seq
		memset(&tmpUserInfo, 0, sizeof(tmpUserInfo));
		tmpUserInfo.seq = atoi(label);
		strncpy(tmpUserInfo.name, dirp->d_name + i+1, strlen(dirp->d_name)-(i+1));

		memcpy(pUserMem+dirNum, &tmpUserInfo, sizeof(struct userInfo_Stru));
		dirNum ++;
//		printf("seq: %d\t name: %s\n", (pUserMem+dirNum)->seq , (pUserMem+dirNum)->name);
	}

	closedir(dir);

	if(*ppUserList != NULL)		// free last time source
	{
		free(*ppUserList);
		*ppUserList = NULL;
	}
	*ppUserList = pUserMem;

	gstUsrMngr.userChange = 1;
	
	return 0;
}

int del_UserFace(char *facePath)
{
	struct stat statbuf;
	int ret;

	if(facePath == NULL)
		return -1;

	// 获取文件属性
	if(lstat(facePath, &statbuf) < 0)
	{
		printf("lstat(%s) failed !\n", facePath);
		return -1;
	}

	// 判断是否为目录
	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s is not dir !\n", facePath); 
		return -1;
	}

	ret = remove_dir(facePath);
	if(ret != 0)
	{
		printf("%s: remove_dir %s failed.\n", __FUNCTION__, facePath);
		return -1;
	}
	printf("remove_dir %s success.\n", facePath);

	return 0;
}

// 参数: char *名字
void* addFace_pthread(void *arg)
{
	struct stat statbuf;
	char 	chName[USER_NAME_LEN] = {0};
	char 	getDirPath[128] = {0};
	int		ret = 0;

	if(arg == NULL)
	{
		printf("%s: arg is null !\n", __FUNCTION__);
		return (void*)0;
	}

	pthread_mutex_lock(&gstUsrMngr.QImgFaceLock);
	gstUsrMngr.QImgFace.load(WIN_FACE_IMG);
	pthread_mutex_unlock(&gstUsrMngr.QImgFaceLock);

	pthread_mutex_lock(&gstUsrMngr.MatCapLock);
	gstUsrMngr.MatCapture = imread(WIN_BACKGRD_IMG);
	pthread_mutex_unlock(&gstUsrMngr.MatCapLock);
	
	gstUsrMngr.getFaceSta = 1;		// last init

	strncpy(chName, (const char*)arg, strlen((const char*)arg));
	printf("add user name: %s\n", chName);

	ret = get_dir((char *)FACES_LIB_PATH, chName, getDirPath);
	if(ret != 0)
	{
		printf("get_dir failed!\n");
		goto addFace_exit;
//		return (void*)0;
	}
	
	/* 检测目录是否创建成功 */
	if(lstat(getDirPath, &statbuf) < 0)	// 获取文件属性
	{
		printf("lstat(%s) failed !\n", getDirPath);
		goto addFace_exit;
	}
	if(S_ISDIR(statbuf.st_mode) != 1)	// 判断是否为目录
	{
		printf("%s is not dir !\n", getDirPath); 
		goto addFace_exit;
	}
	
	ret = get_face(getDirPath);
	if(ret < 0)
	{
		printf("get_face failed!\n");
		ret = remove(getDirPath);
		if(ret != 0)
			printf("remove %s failed!\n", getDirPath);
		
		goto addFace_exit;
	}

	gstUsrMngr.addFaceOK = 1;		// 录入成功标记

	ret = Create_CSV((char *)FACES_LIB_PATH);

	// restart 2 opencv deal pthread
	ret = opencvDeal_restart();

addFace_exit:
	gstUsrMngr.getFaceSta = 0;

	// 关闭所有opencv窗口
//	destroyAllWindows();

	printf("%s: exit ---------.\n", __FUNCTION__);
	
	return (void*)0;
}



// 判断通配目录是否已存在，如:1_* ，寻找以1_开头，只支持通配开头 
int checkDirExist(char *basePath, char *dirHeadStr)
{
	struct stat statbuf;
	DIR *dir;
	struct dirent *dirp;
	char 	tmpDirPath[128] = {0};

	if(basePath[strlen(basePath)-1] == '/')		// 统一输入不以'/'结束，如 "/mnt/" 改为 "/mnt"
		basePath[strlen(basePath)-1] = 0;
	
	if(dirHeadStr[strlen(dirHeadStr)-1] == '*')
		dirHeadStr[strlen(dirHeadStr)-1] = 0;

	// 获取文件属性
	if(lstat(basePath, &statbuf) < 0)
	{
		printf("lstat(%s) failed !\n", basePath);
		return -1;
	}
	
	// 判断是否为目录
	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s is not dir !\n", basePath); 
		return -1;
	}

	// 打开目录
	dir = opendir(basePath);
	if( dir ==NULL)
	{
		printf("opendir failed.\n");
		return -1;
	}

	// 遍历一级目录
	while((dirp = readdir(dir)) != NULL)
	{
		// 忽略 '.' '..'文件（linux）
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;
		
		// 将1级目录与2级目录组合
		memset(tmpDirPath, 0, sizeof(tmpDirPath));
		strcat(tmpDirPath, basePath);
		strcat(tmpDirPath, "/");
		strcat(tmpDirPath, dirp->d_name);

		if(strncmp(dirp->d_name, dirHeadStr, strlen(dirHeadStr)) == 0)	// 匹配，证明存在
		{
			return -1;
		}

	}
	
	// 到此代表以dirHeadStr开头的目录可用

	return 0;
}


// 获取可用目录 序号_名字 .../ 序号唯一
int get_dir(char *inDir, char *inName, char *outDir)
{
	struct stat statbuf;
	char 	tmpDirPath[128] = {0};
	int		nDirSeq = 0;
	char 	chDirSeq[10] = {0};
	int 	ret = 0;


	if(inDir[strlen(inDir)-1] == '/') 	// 统一输入不以'/'结束，如 "/mnt/" 改为 "/mnt"
		inDir[strlen(inDir)-1] = 0;

	if(lstat(inDir, &statbuf) < 0)	// 获取文件属性
	{
		printf("lstat(%s) failed !\n", inDir);
		return -1;
	}
	
	if(S_ISDIR(statbuf.st_mode) != 1)	// 判断是否为目录
	{
		printf("%s is not dir !\n", inDir); 
		return -1;
	}

	// 从s1 s2 ...开始寻找未用的目录名
	while(1)
	{
		nDirSeq ++;

		// 序号转换，1_ 2_ ...
		memset(chDirSeq, 0, sizeof(chDirSeq));
		sprintf(chDirSeq, "%d_", nDirSeq);

		// 检查序号是否存在
		ret = checkDirExist(inDir, chDirSeq);
		if(ret != 0)	// 存在
		{
			continue;
		}

		// 组合目录
		memset(tmpDirPath, 0, sizeof(tmpDirPath));
		strcat(tmpDirPath, inDir);
		strcat(tmpDirPath, "/");
		strcat(tmpDirPath, chDirSeq);
		strcat(tmpDirPath, inName);

		if(lstat(tmpDirPath, &statbuf) < 0)		// 不存在，证明可用
		{
			printf("lstat: %s is not used !\n", tmpDirPath); 
			memcpy(outDir, tmpDirPath, sizeof(tmpDirPath));
			break;
		}

		if(S_ISDIR(statbuf.st_mode) != 1)	// 不是目录，证明可用
		{
			printf("S_ISDIR: %s is not used !\n", tmpDirPath); 
			memcpy(outDir, tmpDirPath, sizeof(tmpDirPath));
			break;
		}

	}

	// make directory
	ret = mkdir(tmpDirPath, 0777);
	if(ret == -1)
	{
		printf("mkdir failed.\n");
		return -1;
	}

	return 0;
}


// 获取人脸图像
int get_face(char *path)
{
	CascadeClassifier face_cascade;  
	VideoCapture camera;
	Mat	frame;
	int ret = 0;
	int faceNum = 1;
	vector<Rect> faces;  
	Mat img_gray;  
	Mat resize_gIMG;
	Mat faceImg;
    double scale = 3.00000;
    double fx = 1.00000 / scale;

	camera.open(0);		// 打开摄像头
	if(!camera.isOpened())
	{
	  cout << "open camera failed. " << endl;
	  return -1;
	}
	cout << "open camera succeed. " << endl;

	// 加载人脸分类器
	ret = face_cascade.load("haarcascade_frontalface_alt.xml");
	if( !ret )
	{
		printf("load xml failed.\n");
		return -1;
	}

	while (1)  
	{
		camera >> frame;
		if(frame.empty())
		{
			continue;
			usleep(100);
		}
		
		cvtColor(frame, img_gray, COLOR_BGR2GRAY);  
		
		resize( img_gray, resize_gIMG, Size(), fx, fx, INTER_LINEAR);	//缩放图像
		
//		equalizeHist(img_gray, img_gray);  
		equalizeHist(resize_gIMG, resize_gIMG);  
		
		// 检测目标
//		face_cascade.detectMultiScale(img_gray, faces, 1.1, 3, 0, Size(50, 50)); 
		face_cascade.detectMultiScale(resize_gIMG, faces, 1.1, 3, 0, Size(50, 50)); 

		for(size_t i =0; i<faces.size(); i++)  
		{
			// 修正resize后的人脸位置
			faces[0].x *= scale;
			faces[0].y *= scale;
			faces[0].width *= scale;
			faces[0].height *= scale;
		
			 /* 画矩形框出目标 */
			rectangle(frame, Point(faces[0].x, faces[0].y), Point(faces[0].x + 
				faces[0].width, faces[0].y + faces[0].height),	Scalar(0, 255, 0), 1, 8);
		}

		pthread_mutex_lock(&gstUsrMngr.MatCapLock);
		gstUsrMngr.MatCapture = frame.clone();
		pthread_mutex_unlock(&gstUsrMngr.MatCapLock);
/*
		imshow("camera", frame);  // 显示
		// delay, no too loog or short , not use WaitKey() (because qt)
		usleep(800);
*/
		if(gstUsrMngr.photoFlag)
		{
			// 只限定检测一个人脸
			if(faces.size() == 1)
			{
				faceImg = frame(faces[0]);
				ret = resize_save(faceImg, path, faceNum);	// 调整大小及保存
				if(ret == 0)
				{
					printf("resize_save %s/%d success.\n", path, faceNum);
					if(faceNum >= 10)
						return 0;
					faceNum ++;
				}
			}
			gstUsrMngr.photoFlag = 0;
		}
		
	}  
	
	camera.release();

	return 0;
}


// 调整大小并保存
int resize_save(Mat& faceIn, char *path, int FaceSeq)
{
	string strName;
	Mat image;
	Mat faceOut;  
	int ret;

	if(faceIn.empty())
	{  
    	printf("faceIn is empty.\n");
      	return -1;  
	}  

	if (faceIn.cols > 100)  
	{  
		resize(faceIn, faceOut, Size(92, 112));		// 调整大小，此选择与官方人脸库兼容
		strName = format("%s/%d.jpg", path, FaceSeq);	// 先要创建文件夹
		ret = imwrite(strName, faceOut);  // 文件名后缀要正确 .jpg .bmp ...
		if(ret == false)	// 出现错误，请检测文件名后缀、文件路径是否存在
		{
			printf("imwrite failed!\n");
			printf("please check filename[%s] is legal ?!\n", strName.c_str());
			return -1;
		}
//		imshow(strName, faceOut);  
		
		pthread_mutex_lock(&gstUsrMngr.QImgFaceLock);
		gstUsrMngr.QImgFace = gFaceDeal.cvMat2QImage(faceOut); // cvMat >> QImage
		pthread_mutex_unlock(&gstUsrMngr.QImgFaceLock);

	}  

    return 0;
}


// 创建/更新CSV文件 参数: 人脸库路径
int Create_CSV(char *dir_path)
{
	struct stat statbuf;
	DIR *dir = NULL;
	DIR *dirFile = NULL;
	struct dirent *dirp;
	struct dirent *direntFile;
	char 	dir_path2[64] = {0};
	char 	fileName[128] = {0};
	char 	witeBuf[128] = {0};
	char 	label[10] = {0};
	int 	fd;
	int 	i;

	if(dir_path[strlen(dir_path)-1] == '/')		// 统一输入不以'/'结束，如 "/mnt/" 改为 "/mnt"
		dir_path[strlen(dir_path)-1] = 0;

	// 获取文件属性
	if(lstat(dir_path, &statbuf) < 0)
	{
		printf("lstat(%s) failed !\n", dir_path);
		return -1;
	}

	// 判断是否为目录
	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s is not dir !\n", dir_path); 
		return -1;
	}

	// 打开目录
	dir = opendir(dir_path);
	if( dir ==NULL)
	{
		printf("opendir failed.\n");
		return -1;
	}

	// 创建或打开csv文件
	fd = open("faces.csv", O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
	{
		printf("open file faile.\n");
		return -1;
	}

	// 定位读写位置
	lseek(fd, 0, SEEK_SET);

	// 遍历一级目录
	while((dirp = readdir(dir)) != NULL)
	{
		// 忽略 '.' '..'文件（linux）
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;

		// 将1级目录与2级目录组合
		memset(dir_path2, 0, sizeof(dir_path2));
		strcat(dir_path2, dir_path);
		strcat(dir_path2, "/");
		strcat(dir_path2, dirp->d_name);

		if(lstat(dir_path2, &statbuf) < 0)
		{
			printf("lstat(%s) failed !\n", dir_path2);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode) != 1)
		{
			printf("%s is not dir !\n", dir_path2); 
			continue;
		}

		dirFile = opendir(dir_path2);
		if( dirFile ==NULL)
		{
			printf("opendir failed.\n");
			return -1;
		}

		// 遍历二级目录
		while((direntFile = readdir(dirFile)) != NULL)
		{
			if( strncmp(direntFile->d_name, ".", strlen(direntFile->d_name))==0 || 
				 strncmp(direntFile->d_name, "..", strlen(direntFile->d_name))==0 )
				continue;

			// 获取完整文件路径名
			memset(fileName, 0, sizeof(fileName));
			strcat(fileName, dir_path2);
			strcat(fileName, "/");
			strcat(fileName, direntFile->d_name);

			if(lstat(fileName, &statbuf) < 0)
			{
				printf("lstat(%s) failed !\n", fileName);
				continue;
			}
			if(S_ISREG(statbuf.st_mode) != 1)	// 不是普通文件
			{
				printf("%s is not reg file !\n", fileName); 
				continue;
			}

			memset(label, 0, sizeof(label));
			memset(witeBuf, 0, sizeof(witeBuf));
			memcpy(witeBuf, fileName, strlen(fileName));
			strcat(witeBuf, ";");

			for(i=0; i<10; i++)		// 取序号
			{
				if(dirp->d_name[i] != '_')
					label[i] = dirp->d_name[i];
				else
					break;
			}
			if(i == 10)
				continue; 
			
			strcat(witeBuf, label);	// 标签: '_'前的字符
			strcat(witeBuf, "\n");

			// 写入信息
			if(write(fd, witeBuf, strlen(witeBuf)) != (ssize_t)strlen(witeBuf))
			{
				printf("%s: write failed!\n", __FUNCTION__);
				closedir(dirFile);
				closedir(dir);
				close(fd);
				return -1;
			}
		}
	}

	closedir(dirFile);
	closedir(dir);
	close(fd);

	return 0;
}

void userMngr_DeInit(struct userMngr_Stru *userMnStru)
{
	if(userMnStru->pstUserInfo != NULL)
	{
		free(userMnStru->pstUserInfo);
		userMnStru->pstUserInfo = NULL;
	}
}

// 删除非空目录
int remove_dir(const char *dir)
{
    char cur_dir[] = ".";
    char up_dir[] = "..";
    char dir_name[128];
    DIR *dirp;
    struct dirent *dp;
    struct stat dir_stat;

    // 参数传递进来的目录不存在，直接返回
    if ( 0 != access(dir, F_OK) ) {
        return 0;
    }

    // 获取目录属性失败，返回错误
    if ( 0 > stat(dir, &dir_stat) ) {
        perror("get directory stat error");
        return -1;
    }

    if ( S_ISREG(dir_stat.st_mode) ) {  // 普通文件直接删除
        remove(dir);
    } else if ( S_ISDIR(dir_stat.st_mode) ) {   // 目录文件，递归删除目录中内容
        dirp = opendir(dir);
        while ( (dp=readdir(dirp)) != NULL ) {
            // 忽略 . 和 ..
            if ( (0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name)) ) {
                continue;
            }

            sprintf(dir_name, "%s/%s", dir, dp->d_name);
            remove_dir(dir_name);   // 递归调用
        }
        closedir(dirp);

        rmdir(dir);     // 删除空目录
    } else {
        perror("unknow file type!");    
    }

    return 0;
}

