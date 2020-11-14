#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QTime>
#include <QString>
#include <time.h>


#define QTIMER_INTERV		1		// ms
#define REC_OK_SHOWTIME		3	// 识别成功显示时间 S
#define NO_DRAW             30

#define PRED_FACE_LABEL			"当前人脸标签"
#define CONFIDENCE_LABEL		"Confidence"

#define WIN_BACKGRD_IMG				"images/junbian.jpg"		// 界面背景图
#define REC_OK_IMG					"images/rec_ok.jpg"
#define COPYRIGHT_IMG				"images/copyright-1.png"

#define SYS_BG                      "images/SYS_BG.png"
#define BACKGROUND_BG               "images/Background.png"
#define BACKGROUND_TITLE_1          ""
#define BACKGROUND_TITLE_2          ""
#define BACKGROUND_GROUP            "images/QRcode.png"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
    unsigned char junb_uf[3];
	~MainWindow();

public slots:
    void start_anima();
    void refresh_time();

private slots:
	void showImage();

private:
    QLabel          *BGLabel;           //启动画面
    QLabel          *sysBG;             //物联网抽纸背景
    QLabel          *QRcode;
    QLabel          *cur_Time;          //显示系统时间
    QLabel          *usr_profile;       //用户头像
    QLabel          *paper_cnt;         //抽纸次数
    QString         s_paper_cnt;
	QWidget 		*cenWidget;			// 中心窗口
	QLabel 			*imgLabel;			// 图像显示
	QImage 			camImage;			// 摄像头图像
	QLabel			*copyrightLab;		// copyright info label
	QLabel 			*predLabel;			// 识别结果
	QString			predString;			// 识别结果显示字符
	QLabel 			*confdLabel;		// 识别置信度/相似度
	QString			confdString;		// 识别置信度显示字符
	QLabel			*recOKLabel;		// 验证通过图标
	QTimer 			*timer;				// 刷新显示定时器
	QLabel			*QLabMess;			// some message 
	QPushButton 	*button;
};

#endif
