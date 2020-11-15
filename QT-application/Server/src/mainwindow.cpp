#include "mainwindow.h"
#include "main.h"

#include <QTextCodec>

/* C++ 包含 C */
#ifdef __cplusplus
	extern "C" {
#endif

#include "svrsocket.h"

#ifdef __cplusplus
	}
#endif


extern struct mainWorkStru 	gstMainWork;

extern struct socketInfo	svrSocketInfo;
extern FaceImageDeal		gFaceDeal;
extern FaceRecogn			gFaceRec;
extern struct userMngr_Stru	gstUsrMngr;;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{

	QImage	tmpQImg;
	QPixmap tmpQPix;

    setWindowTitle(("基于龙芯派的人脸识别智能物联网抽纸机"));

	resize(900, 480);

	cenWidget = new QWidget;	// 中心窗口
	setCentralWidget(cenWidget);

	QImgWinLa.load(WIN_BACKGRD_IMG);
	
	imgLabel = new QLabel(cenWidget);
	imgLabel->setPixmap(QPixmap::fromImage(QImgWinLa));
    imgLabel->setGeometry(260,0,640,480);
	imgLabel->show();

	PopUpLabel = new QLabel(cenWidget);
    PopUpLabel->setGeometry(480,140,200,200);
	PopUpLabel->hide();
	
	tmpQImg.load(REC_OK_IMG);
	tmpQPix = QPixmap::fromImage(tmpQImg);
	QPixRecOK = tmpQPix.scaled(PopUpLabel->width(), PopUpLabel->height(), 
		Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充

	tmpQImg.load(ADD_FACE_OK);
	tmpQPix = QPixmap::fromImage(tmpQImg);
	QPixAddFaOK = tmpQPix.scaled(PopUpLabel->width(), PopUpLabel->height(), 
		Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充

	tmpQImg.load(COPYRIGHT_IMG);
	copyrightLab = new QLabel(cenWidget);
    copyrightLab->setGeometry(0,0,260,80);
	copyrightLab->setPixmap(QPixmap::fromImage(tmpQImg));
	copyrightLab->show();

    tmpQImg.load(SLAB_PNG);
    SlabLabel = new QLabel(cenWidget);
    SlabLabel->setGeometry(0,80,260,400);
    SlabLabel->setPixmap(QPixmap::fromImage(tmpQImg));
    SlabLabel->show();
	
	predString = QString("%1:  %2").arg(PRED_FACE_LABEL).arg(0);
	predLabel = new QLabel(cenWidget);
    predLabel->setGeometry(20,100,250,30);
	predLabel->setText(predString);

	confdString = QString("%1: %2\%").arg(CONFIDENCE_LABEL).arg(0);
	confdLabel = new QLabel(cenWidget);
    confdLabel->setGeometry(20,140,250,30);
	confdLabel->setText(confdString);

	QLabMess = new QLabel(cenWidget);
    QLabMess->setGeometry(20,180,250,30);
	QLabMess->setText(AUTHOR_MESSAGE);

	LEditUser = new QLineEdit(cenWidget);
    LEditUser->setPlaceholderText("添加用户名称");
    LEditUser->setGeometry(10, 230, 150, 40);

	QButAddUser = new QPushButton(cenWidget);
    QButAddUser->setText(tr("添加用户"));
    connect(QButAddUser, SIGNAL(clicked()), this, SLOT(addFace()));
    QButAddUser->setGeometry(170, 230, 80, 40);

	QComDelUser = new QComboBox(cenWidget);
    QComDelUser->setGeometry(10, 290, 150, 40);
	QComDelUser->setEditable(true);

	QButDelUser = new QPushButton(cenWidget);
    QButDelUser->setText(tr("删除"));
    connect(QButDelUser, SIGNAL(clicked()), this, SLOT(delFace()));
    QButDelUser->setGeometry(170, 290, 80, 40);

    QDrawPaper = new QPushButton(cenWidget);
    QDrawPaper->setText(tr("强制出纸"));
    connect(QDrawPaper, SIGNAL(clicked()), this, SLOT(drawpaper()));
    QDrawPaper->setGeometry(140, 390, 80, 40);

	QImgFaceLa.load(WIN_FACE_IMG);
	faceLabel = new QLabel(cenWidget);
	faceLabel->setPixmap(QPixmap::fromImage(QImgFaceLa));
    faceLabel->setGeometry(10,350,92,112);
	faceLabel->show();

	/* 定时任务，显示图像 */
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showImage()));
    timer->start(QTIMER_INTERV);

}

MainWindow::~MainWindow()
{
	
}

void MainWindow::addFace()
{
	int ret = 0;
	QString qUsrName;
	QByteArray ba;
	static char arg_name[USER_NAME_LEN] = {0};		// 要用static ，不然可能会乱码

	if(!gstUsrMngr.getFaceSta && !gstUsrMngr.addFaceOK)		// 创建线程，获取人脸图像
	{
		// get text string
		memset(arg_name, 0, sizeof(arg_name));
		qUsrName = LEditUser->text();
		ba = qUsrName.toLatin1();
		strncpy(arg_name, ba.data(), strlen(ba.data()));
		if(strlen(arg_name) <= 0)
		{
            QLabMess->setText("警告：用户名称为空");
			return ;
		}

        QLabMess->setText("信息：添加用户 ...");
		
		ret = pthread_create(&gstMainWork.tid_addFace, NULL, addFace_pthread, (void*)arg_name);
		if(ret < 0)
		{
			printf("%s: socket_handle() failed.\n", __FUNCTION__);
			return ;
		}
		printf("%s: socket_handle() success.\n", __FUNCTION__);
	}
	else	// 拍照
	{
		gstUsrMngr.photoFlag = 1;
	}

	
}

void MainWindow::delFace()
{
	QString qUsrName;
	QByteArray ba;
	char usr_name[USER_NAME_LEN] = {0};
	char dir_name[128] = {0};
	char chSeq_[10] = {0};
	int ret;
	int i;

    QLabMess->setText("信息：删除用户 ...");

	// get text string
	memset(usr_name, 0, sizeof(usr_name));
	qUsrName = QComDelUser->currentText();
	ba = qUsrName.toLatin1();
	strncpy(usr_name, ba.data(), strlen(ba.data()));
	if(strlen(usr_name) <= 0)
	{
		return ;
	}

    if(QMessageBox::warning(this,"警告", "删除 "+QComDelUser->currentText()+" ?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
	{
		return ;
	}
	
	// 组合当前用户所在的文件夹名
	for(i=0; i<gstUsrMngr.userSum; i++)
	{
		if(strlen(usr_name)==strlen((gstUsrMngr.pstUserInfo+i)->name) && 
			0==strncmp((gstUsrMngr.pstUserInfo+i)->name, usr_name, strlen(usr_name)))
		{
			memset(dir_name, 0, sizeof(dir_name));
			sprintf(chSeq_, "%d_", (gstUsrMngr.pstUserInfo+i)->seq);
			
			strcat(dir_name, FACES_LIB_PATH);
			strcat(dir_name, "/");
			strcat(dir_name, chSeq_);
			strcat(dir_name, usr_name);

			break;
		}
	}

	ret = del_UserFace(dir_name);
	if(ret != 0)
	{
        QLabMess->setText("(错误)删除用户失败");
		return ;
	}

	// restart 2 opencv deal pthread
	ret = opencvDeal_restart();

    QLabMess->setText("清除（DELETE）用户成功");

}

void MainWindow::drawpaper()
{
    static unsigned char jun[] = "DRAW";
    if(QMessageBox::warning(this,"警告", "是否执行强制出纸？(请保证设备附近有工作人员值守)",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
    {
        return ;
    }
    if(!svrSocketInfo.connectSta)
    {
        QLabMess->setText("无网络连接，请检查设备连接");
        printf("%s : No TCP Connect! Please check device connection \n",__FUNCTION__);
    }
    else
    {
    pthread_mutex_lock(&svrSocketInfo.sendLock);
    socket_send(svrSocketInfo.consockfd, CMD_DRAW, jun, sizeof(jun));
    pthread_mutex_unlock(&svrSocketInfo.sendLock);
    }
}

void MainWindow::showImage()
{
	int 	i = 0;
	Mat 	detectMat;
	QImage 	detectImg;
	int 	v4l2Flag;
	static time_t 	beTim = 0;
	static time_t 	curTim = 0;
	static int 	recOKshow = 0;
	static int 	addFashow = 0;


	if(!gstMainWork.workStatus)
	{
		timer->stop();
		return ;
	}

	timer->stop();

	if(gstUsrMngr.getFaceSta)		// 录入人脸中...
	{
		pthread_mutex_lock(&gstUsrMngr.MatCapLock);
		if(!gstUsrMngr.MatCapture.empty())
		{
			gstUsrMngr.QImgCapture = gFaceDeal.cvMat2QImage(gstUsrMngr.MatCapture); // cvMat >> QImage
		}
		pthread_mutex_unlock(&gstUsrMngr.MatCapLock);
		
		imgLabel->setPixmap(QPixmap::fromImage(gstUsrMngr.QImgCapture));

		pthread_mutex_lock(&gstUsrMngr.QImgFaceLock);
		faceLabel->setPixmap(QPixmap::fromImage(gstUsrMngr.QImgFace));
		pthread_mutex_unlock(&gstUsrMngr.QImgFaceLock);
		
	}
	else if(gFaceDeal.detectFlag == 1)	// 检测到人脸
	{
		gFaceDeal.detectFlag = 0;

		/* 获取当前图像加上矩形框，存在问题: 延时 */
		pthread_mutex_lock(&gFaceDeal.matV4Lock);
		detectMat = gFaceDeal.MatV4l2.clone();
		pthread_mutex_unlock(&gFaceDeal.matV4Lock);

		/* 画矩形框出人脸  */
		for(i=0; i<(int)gFaceDeal.vFacesRect.size(); i++)
		{
			rectangle(detectMat, Point(gFaceDeal.vFacesRect[i].x, gFaceDeal.vFacesRect[i].y), 
			Point(gFaceDeal.vFacesRect[i].x + gFaceDeal.vFacesRect[i].width, 
				gFaceDeal.vFacesRect[i].y + gFaceDeal.vFacesRect[i].height), 
				Scalar(0, 0, 255), 3, 8);	 // 框出人脸
		}

		/* cmMat >> QImage */
		detectImg = gFaceDeal.cvMat2QImage(detectMat);

		/* qt image show */
		imgLabel->setPixmap(QPixmap::fromImage(detectImg));

	}
	else
	{
		pthread_mutex_lock(&svrSocketInfo.recvLock);
		v4l2Flag = svrSocketInfo.recvBLen;
		svrSocketInfo.recvBLen = 0;
		pthread_mutex_unlock(&svrSocketInfo.recvLock);
		
		if(v4l2Flag > 0)
		{
			pthread_mutex_lock(&gFaceDeal.imgV4Lock);
			imgLabel->setPixmap(QPixmap::fromImage(gFaceDeal.QImgV4l2));
			pthread_mutex_unlock(&gFaceDeal.imgV4Lock);
		}
		else if(!svrSocketInfo.connectSta)	// no tcp connect
		{
			imgLabel->setPixmap(QPixmap::fromImage(QImgWinLa));
		}
	}

	curTim = time(NULL);
	/* 刷新显示弹出窗口 */
	if((gFaceRec.predChang&&!recOKshow) || (gstUsrMngr.addFaceOK&&!addFashow))
	{
		beTim = time(NULL);

		if((gFaceRec.predChang&&!recOKshow))
		{
            QString prdNameStr =  QString("信息: %1").arg(gFaceRec.predName);
			QLabMess->setText(prdNameStr);
			predString = QString("%1:  %2").arg(PRED_FACE_LABEL).arg(gFaceRec.predFace);
			predLabel->setText(predString);
			PopUpLabel->setPixmap(QPixRecOK);
			PopUpLabel->show();
			recOKshow = 1;
		}
		if((gstUsrMngr.addFaceOK&&!addFashow))
		{
            QLabMess->setText("成功添加用户");
			PopUpLabel->setPixmap(QPixAddFaOK);
			PopUpLabel->show();
			addFashow = 1;
		}
	}
	else if(curTim - beTim >= REC_OK_SHOWTIME)		// 显示3S,超时归 0
	{
		beTim = curTim + 10000;		// 防止经常进入

		if(gFaceRec.predChang)
		{
			gFaceRec.predChang = 0;
			gFaceRec.predFace = 0;
			PopUpLabel->hide();
			recOKshow = 0;
			QLabMess->setText(AUTHOR_MESSAGE);
			predString = QString("%1:  %2").arg(PRED_FACE_LABEL).arg(gFaceRec.predFace);
			predLabel->setText(predString);
		}

		if(gstUsrMngr.addFaceOK)
		{
			gstUsrMngr.addFaceOK = 0;
			PopUpLabel->hide();
			addFashow = 0;
		}

	}

	// 显示相似度
	confdString = QString("%1: %2\%").arg(CONFIDENCE_LABEL).arg(gFaceRec.faceConfd);
	confdLabel->setText(confdString);
	
	// 添加人脸相关
	if(! gstUsrMngr.getFaceSta)
	{
        QButAddUser->setText(tr("添加人脸"));
		
		faceLabel->setPixmap(QPixmap::fromImage(QImgFaceLa));
	}
	else
	{
        QButAddUser->setText(tr("拍照"));
	}

	// update delete user list
	if(gstUsrMngr.userChange)
	{
		gstUsrMngr.userChange = 0;
		QComDelUser->clear();

		for(i=0; i<gstUsrMngr.userSum; i++)
		{
			QComDelUser->addItem((gstUsrMngr.pstUserInfo+i)->name);
		}
	}


	timer->start(QTIMER_INTERV);

}
