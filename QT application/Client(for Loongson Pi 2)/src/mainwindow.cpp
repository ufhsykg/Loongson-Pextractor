#include "mainwindow.h"
#include "opencv_deal.h"
#include "main.h"


/* C++ ���� C */
#ifdef __cplusplus
	extern "C" {
#endif

#include "v4l2_cap.h"
#include "clisocket.h"

#ifdef __cplusplus
	}
#endif

extern struct mainWorkStru 		gstMainWork;

extern struct v4l2capStru		gv4l2capInfo;
extern OpencvDeal				gOpcvDeal;
extern struct socketInfo		cliSocketInfo;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	QImage	tmpQImg;
	QPixmap tmpQPix;
	setWindowTitle("mainwindow");

    resize(1024, 600);
    gstMainWork.papercnt = 0;
	cenWidget = new QWidget;	// ���Ĵ���
	setCentralWidget(cenWidget);

    BGLabel = new QLabel(cenWidget);
    BGLabel->setPixmap(QPixmap(BACKGROUND_BG));
    BGLabel->setGeometry(0,0,1024,600);	// ��������
    BGLabel->show();

    QTimer::singleShot(6000,this,SLOT(start_anima()));

    sysBG = new QLabel(cenWidget);
    sysBG->setPixmap(QPixmap(SYS_BG));
    sysBG->setGeometry(0,0,1024,600);	// ��������
    sysBG->hide();

	imgLabel = new QLabel(cenWidget);
	imgLabel->setPixmap(QPixmap(WIN_BACKGRD_IMG));
    imgLabel->setGeometry(4,101,640,480);	// ��������
    imgLabel->hide();

    QRcode = new QLabel(cenWidget);
    QRcode->setPixmap(QPixmap(BACKGROUND_GROUP));
    QRcode->setGeometry(776,424,127,127);	// ��������
    QRcode->hide();

    cur_Time = new QLabel(cenWidget);
    cur_Time->setGeometry(400,29,200,100);	// ��������
    cur_Time->hide();

	// ��֤ͨ��ͼ��
	recOKLabel = new QLabel(cenWidget);
    recOKLabel->setGeometry(217,241,200,200); // ��������
	tmpQImg.load(REC_OK_IMG);
	tmpQPix = QPixmap::fromImage(tmpQImg);
	QPixmap fitpixmap = tmpQPix.scaled(recOKLabel->width(), recOKLabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // �������
	recOKLabel->setPixmap(fitpixmap);
	recOKLabel->hide();
/*
	tmpQImg.load(COPYRIGHT_IMG);
	copyrightLab = new QLabel(cenWidget);
	copyrightLab->setGeometry(640,0,160,120);
	copyrightLab->setPixmap(QPixmap::fromImage(tmpQImg));
    copyrightLab->hide();
*/
    predString = QString(QString::fromLocal8Bit("%1:  %2")).arg(PRED_FACE_LABEL).arg(cliSocketInfo.predFace);
	predLabel = new QLabel(cenWidget);
    predLabel->setGeometry(722,245,200,50);	// ��������
	predLabel->setText(predString);
    predLabel->hide();

    s_paper_cnt = QString(QString::fromLocal8Bit("%1 ��")).arg(gstMainWork.papercnt);
    paper_cnt = new QLabel(cenWidget);
    paper_cnt->setGeometry(910,135,160,50);	// ��������
    paper_cnt->setText(s_paper_cnt);
    paper_cnt->hide();

    confdString = QString("%1\%").arg(0);
    confdLabel = new QLabel(cenWidget);
    confdLabel->setGeometry(740,135,160,50);	// ��������
	confdLabel->setText(confdString);
    confdLabel->hide();

	QLabMess = new QLabel(cenWidget);
    QLabMess->setGeometry(608,28,50,50);
    QLabMess->setText(QString::fromLocal8Bit("��"));
    QLabMess->hide();
	
/*
	button = new QPushButton(cenWidget);
	button->setText(tr("Hellobutton"));
	button->setGeometry(640, 240, 100, 50);	// ��������
*/
	/* ��ʱ������ʾͼ�� */
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showImage()));
	timer->start(QTIMER_INTERV);

}

MainWindow::~MainWindow()
{

}

void MainWindow::showImage()
{
	int 	i = 0;
    int     err = 0;
    int     jun = 0;
	Mat 	detectMat;
	QImage	detectImg;
	static time_t 	beTim = 0;
	time_t 	curTim = 0;
    static time_t 	beTim2 = 0;
	static int 	recOKshow = 0;
    static int    candraw = 0;

	curTim = time(NULL);
	timer->stop();
    refresh_time();
//    printf("print init");
	if(gv4l2capInfo.frameLen > 0)
	{
		/* ��V4L2ͼ������ת��ΪQImage */
		pthread_mutex_lock(&gv4l2capInfo.frameLock);
		camImage.loadFromData(gv4l2capInfo.frameBuf, gv4l2capInfo.frameLen);
		gv4l2capInfo.frameLen = 0;
		pthread_mutex_unlock(&gv4l2capInfo.frameLock);
		if(camImage.isNull())
		{
			printf("loadFromData failed[camImage is null].\n");
			return ;
		}

 //   cliSocketInfo.detectFlag=1;
		if(cliSocketInfo.detectFlag == 1)	// ��⵽������������ʾ
		{
			/* ��QImageת��ΪcvMat */
			gOpcvDeal.MatV4l2 = gOpcvDeal.QImage2cvMat(camImage).clone();
          //  printf("jun");
        /* ��ȡ��ǰͼ����Ͼ��ο򣬴�������: ��ʱ */
			detectMat = gOpcvDeal.MatV4l2.clone();

			/* �����ο������  */
			for(i=0; i<cliSocketInfo.faceCount; i++)
			{
				rectangle(detectMat, Point(cliSocketInfo.faceRect[i].x, cliSocketInfo.faceRect[i].y), 
				Point(cliSocketInfo.faceRect[i].x + cliSocketInfo.faceRect[i].w, 
					cliSocketInfo.faceRect[i].y + cliSocketInfo.faceRect[i].h), 
					Scalar(0, 0, 255), 3, 8);	 // �������
			}
			
			cliSocketInfo.detectFlag = 0;
			cliSocketInfo.faceCount = 0;
			memset(&cliSocketInfo.faceRect, 0, sizeof(struct rectStru));

			/* cmMat >> QImage */
			detectImg = gOpcvDeal.cvMat2QImage(detectMat);

			/* qt image show */
			imgLabel->setPixmap(QPixmap::fromImage(detectImg));

		}
		else	// ������
		{
			if(!cliSocketInfo.predChang)
			{
				cliSocketInfo.faceConfd = 0;
			}
			if(cliSocketInfo.connectSta)
			{
				imgLabel->setPixmap(QPixmap::fromImage(camImage));
			}
           // printf("bu junbian ");
		}

		/* ˢ����ʾʶ���� */
		if(cliSocketInfo.predChang && !recOKshow)
		{
			QString prdNameStr =  QString("%1").arg(cliSocketInfo.predName);
			QLabMess->setText(prdNameStr);
			beTim = time(NULL);
			predString = QString("%1:  %2").arg(PRED_FACE_LABEL).arg(cliSocketInfo.predFace);
			predLabel->setText(predString);
			recOKLabel->show();
			recOKshow = 1;
            beTim2 = time(NULL);
            //ioctl(gstMainWork.door_fd, DEV_ON, DOOR_DEV);		// open door
            //write(gstMainWork.door_fd,1,sizeof(int));
            //junb_uf[0] = 1;
            //write(gstMainWork.door_fd,junb_uf,sizeof(junb_uf));
            if(!candraw){
            candraw = 1;
            jun = 1;
            err = pthread_create(&gstMainWork.tid_draw, NULL,draw_handle, &jun);
            if(err != 0)
            {
                printf("%s: draw paper failed %s\n", __FUNCTION__,strerror(err));
            }
            gstMainWork.papercnt++;
            }



		}
		else if(curTim - beTim >= REC_OK_SHOWTIME)		// ��ʾ3S,��ʱ��0
		{
            //QLabMess->setText(QString::fromLocal8Bit("��"));
			beTim = curTim + 10000;		// ��ֹ��������
			cliSocketInfo.predChang = 0;
			cliSocketInfo.predFace = 0;
			recOKLabel->hide();
			recOKshow = 0;
			
          //  gstMainWork.junbuf[0] = 0;
           // write(gstMainWork.door_fd,gstMainWork.junbuf,sizeof(gstMainWork.junbuf));
            /*
			predString = QString("%1:  %2").arg(PRED_FACE_LABEL).arg(cliSocketInfo.predFace);
            predLabel->setText(predString);
            */
		}
        else if(curTim - beTim2 >= NO_DRAW){
            candraw = 0;
        }
		
		// ��ʾ���ƶ�
        confdString = QString("%1\%").arg(cliSocketInfo.faceConfd);
		confdLabel->setText(confdString);

	}

	timer->start(QTIMER_INTERV);

}
void MainWindow::start_anima()
{
      BGLabel->clear();

      sysBG->show();
      imgLabel->show();
  //    copyrightLab->show();
      paper_cnt->show();
      predLabel->show();
      confdLabel->show();
      cur_Time->show();
      QLabMess->show();
      QRcode->show();
}

void MainWindow::refresh_time()
{
    QDateTime time;
    s_paper_cnt = QString(QString::fromLocal8Bit("%1 ��")).arg(gstMainWork.papercnt);
    paper_cnt->setText(s_paper_cnt);
    cur_Time->setText(time.currentDateTime().toString("yyyy") + "/" + \
                      time.currentDateTime().toString("M") + "/" + \
                      time.currentDateTime().toString("d") + "<br>" + \
                      time.currentDateTime().toString("h") + ":" + \
                      time.currentDateTime().toString("m"));
    cur_Time->setWordWrap(true);
    cur_Time->setAlignment(Qt::AlignHCenter);
}
