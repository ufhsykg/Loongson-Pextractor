#include <QApplication>
#include <QTextCodec>
#include <QFontDatabase>
#include <signal.h>

#include "main.h"
#include "mainwindow.h"
#include "opencv_deal.h"

/* C++ 包含C */
#ifdef __cplusplus
	extern "C" {
#endif

#include "clisocket.h"
#include "v4l2_cap.h"

#ifdef __cplusplus
	}
#endif

// 管理全局工作信息
struct mainWorkStru 		gstMainWork;

extern struct socketInfo	cliSocketInfo;
extern OpencvDeal			gOpcvDeal;


/* 释放资源 */
void main_deInit(void);

/* 结束程序回调 */
void signal_exit_handler(int sig)
{
	printf("\n--------------- byebye client ---------------\n");
	printf("%s: signal[%d].\n", __FUNCTION__, sig);
	
    main_deInit();

	exit(0);
}


int main(int argc, char *argv[])
{
	printf("--------------- welcome client ---------------\n");

	int ret = 0;
	
    if(argc != 3)
	{
		printf("usage: %s <ip> <port>\n", argv[0]);
		return -1;
	}




	// 管理全局工作结构体初始化
	gstMainWork.workStatus = 1;
	gstMainWork.tid_socket = 0;
	gstMainWork.tid_sockSend = 0;
	gstMainWork.tid_v4l2 = 0;
	gstMainWork.door_fd = 0;
	// 注册异常信号回调
	signal(SIGINT, signal_exit_handler);
/*	
	signal(SIGTERM, signal_exit_handler);
	signal(SIGBUS, signal_exit_handler);
	signal(SIGSEGV, signal_exit_handler);
	signal(SIGABRT, signal_exit_handler);

	signal(SIGFPE, signal_exit_handler);
	signal(SIGILL, signal_exit_handler);
	signal(SIGIOT, signal_exit_handler);
	signal(SIGKILL, signal_exit_handler);
	signal(SIGPWR, signal_exit_handler);
	signal(SIGSTOP, signal_exit_handler);
	signal(SIGSYS, signal_exit_handler);
*/
	memset(cliSocketInfo.chSvrIP, 0, sizeof(cliSocketInfo.chSvrIP));
	strncpy(cliSocketInfo.chSvrIP, argv[1], strlen(argv[1]));
	cliSocketInfo.port = atoi(argv[2]);
	printf("server: %s:%d\n", cliSocketInfo.chSvrIP, cliSocketInfo.port);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	QApplication app(argc, argv);
    QApplication::setOverrideCursor(Qt::BlankCursor);

    int id = QFontDatabase::addApplicationFont("/home/loongson/SourceHanSansCN-Regular.otf");
    QString msyh = QFontDatabase::applicationFontFamilies (id).at(0);
    QFont font(msyh,10);
    qDebug()<<msyh<<endl;
    font.setPointSize(13);
    app.setFont(font);


	MainWindow win;
	win.show();


    gstMainWork.door_fd = open("/dev/junbian",O_RDWR,O_NONBLOCK);
	if(gstMainWork.door_fd < 0)
	{
		printf("open door dev failed!\n");
		return -1;
	}

	printf("open door dev [fd = %d] success.\n", gstMainWork.door_fd);
//	ioctl(gstMainWork.door_fd, DEV_OFF, DOOR_DEV);		// close door
    gstMainWork.junbuf[0] = 0;
    write(gstMainWork.door_fd,gstMainWork.junbuf,sizeof(gstMainWork.junbuf));

	ret = gOpcvDeal.opencvDeal_init();
	if(ret < 0)
	{
		printf("opencvDeal_init() failed.\n");
		return -1;
	}
	printf("opencvDeal_init() success.\n");

    ret = pthread_create(&gstMainWork.tid_v4l2, NULL, v4l2cap_handle, NULL);
    if(ret < 0)
    {
        printf("v4l2cap_handle() failed.\n");
        return -1;
    }
    printf("v4l2cap_handle() success.\n");

	ret = pthread_create(&gstMainWork.tid_socket, NULL, socket_handle, NULL);
	if(ret < 0)
	{
		printf("socket_handle() failed.\n");
		return -1;
	}
	printf("socket_handle() success.\n");




	return app.exec();
}


void main_deInit(void)
{
	gstMainWork.workStatus = 0; 	// stop working

	usleep(1000);		// 1 ms
	
	gOpcvDeal.opencvDeal_DeInit();
	
	cliSocket_deInit(&cliSocketInfo);

	close(gstMainWork.door_fd);
}

