#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QString>
#include <time.h>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>

#include "opencv_deal.h"
#include "usermngr.h"


#define QTIMER_INTERV		1		// ms
#define REC_OK_SHOWTIME		3		// 识别成功显示时间 S

#define PRED_FACE_LABEL			"人脸标签"
#define CONFIDENCE_LABEL		"相似度"
#define	WORKS_MESSAGE			"Solution: Embedded Linux intelligent IOT paper extractor Base Loongson pie"
#define	AUTHOR_MESSAGE			"Contingency 肃正协议"

#define WIN_BACKGRD_IMG				"images/junbian.jpg"		// 界面背景图
#define WIN_FACE_IMG				"images/face.png"
#define REC_OK_IMG					"images/rec_ok.jpg"
#define ADD_FACE_OK					"images/addFace_ok.jpg"
#define COPYRIGHT_IMG				"images/copyright-0.png"
#define SLAB_PNG                    "images/slab_BG.png"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void showImage();
	void addFace();
	void delFace();
    void drawpaper();

private:
	QWidget 		*cenWidget;			// 中心窗口
	QLabel 			*imgLabel;			// 图像显示
	QLabel			*faceLabel;			// 人脸显示框
	QImage			QImgWinLa;			// 初始背景图
	QImage 			QImgFaceLa;			// 初始人脸框图
    QImage          QImgSLAB;
    QLabel          *SlabLabel;
	QLabel			*copyrightLab;		// copyright info label
	QLabel 			*predLabel;			// 识别结果
	QString			predString;			// 识别结果显示字符
	QLabel 			*confdLabel;		// 识别置信度/相似度
	QString			confdString;		// 识别置信度显示字符
	QLabel			*PopUpLabel;		// 弹出提示窗口
	QPixmap			QPixRecOK;			// 验证通过图标
	QPixmap			QPixAddFaOK;		// 录入成功图标
	QTimer 			*timer;				// 刷新显示定时器
	QLabel			*QLabMess;			// some message 
	QLineEdit		*LEditUser;			// edit add user name
	QPushButton 	*QButAddUser;		// add face key
	QComboBox		*QComDelUser;		// delete user list box
	QPushButton 	*QButDelUser;		// delete user key
    QPushButton     *QDrawPaper;        // 启动抽纸
};

#endif
