#ifndef V4L2_CAP_H
#define V4L2_CAP_H

#include <stdio.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>


#define V4L2_PIX_FMT_XXX	V4L2_PIX_FMT_MJPEG	// ����ͷ�ɼ���ʽ
#define BUF_NUM		1		// �ɼ���������
#define LCD_XRES	640		// LCD����
#define LCD_YRES	480
#define FRAMEBUF_SIZE	LCD_XRES*LCD_YRES*3		//


struct v4l2capStru
{
	int 	cam_fd;
	struct v4l2_fmtdesc 	*fmtdesc;
	struct v4l2_format 		*format;
	struct v4l2_buffer 		v4l2buf;
	struct v4l2_requestbuffers 	reqbuf;
	struct v4l2_buffer buffer[BUF_NUM];
	struct
	{
		int length;
		unsigned char *start;	// = {NULL, NULL, NULL}
	}cam_buffer[BUF_NUM];
	
	unsigned char 		*frameBuf;		// �洢ͼ������ 
	unsigned int 		frameLen;
    unsigned int        jun;
	pthread_mutex_t		frameLock;
	
};

void* v4l2cap_handle(void *arg);
int v4l2cap_init(struct v4l2capStru *pv4l2Info);
int v4l2cap_start(struct v4l2capStru *pv4l2Info);
int v4l2cap_getframe(struct v4l2capStru *pv4l2Info);
void v4l2cap_deInit(struct v4l2capStru *pv4l2Info);


#endif // V4L2_CAP_H

