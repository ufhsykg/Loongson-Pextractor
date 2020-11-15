#include "v4l2_cap.h"
#include "clisocket.h"
#include "main.h"


struct v4l2capStru			gv4l2capInfo;

extern struct mainWorkStru 		gstMainWork;
extern struct socketInfo	cliSocketInfo;

void* v4l2cap_handle(void *arg)
{
	int ret = 0;

	ret = v4l2cap_init(&gv4l2capInfo);
	if(ret != 0)
	{
		printf("%s: v4l2cap_init failed.\n", __FUNCTION__);
	}

	/* 开启采集 */
	v4l2cap_start(&gv4l2capInfo);

	while(gstMainWork.workStatus)
	{
		v4l2cap_getframe(&gv4l2capInfo);
	}
}


int v4l2cap_init(struct v4l2capStru *pv4l2Info)
{
	int i = 0;

	pthread_mutex_init(&pv4l2Info->frameLock, NULL);

	/* open camera */
    pv4l2Info->cam_fd = open("/dev/video0", O_RDWR);
	if(pv4l2Info->cam_fd < 0)
	{
		printf("open camera failed!\n");
		return -1;
	}
	printf("open camera successfully!\n");

	/* get support format */
	pv4l2Info->fmtdesc = (struct v4l2_fmtdesc *)calloc(1, sizeof(struct v4l2_fmtdesc));
	pv4l2Info->fmtdesc->index = 0;
	pv4l2Info->fmtdesc->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while(ioctl(pv4l2Info->cam_fd, VIDIOC_ENUM_FMT, pv4l2Info->fmtdesc) != -1)
	{
		pv4l2Info->fmtdesc->index ++ ;
		printf("descripton: %s\n", pv4l2Info->fmtdesc->description);
	}

	/* get camera capture format */
	pv4l2Info->format = (struct v4l2_format *)calloc(1, sizeof(struct v4l2_format));
	pv4l2Info->format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(pv4l2Info->cam_fd, VIDIOC_G_FMT, pv4l2Info->format) < 0)
	{
		printf("ioctl(cam_fd, VIDIOC_G_FMT, cam_fmt) failed ! \n");
		return -1; 
	}
	printf("camera width:  %d \n", pv4l2Info->format->fmt.pix.width);
	printf("camera height: %d \n", pv4l2Info->format->fmt.pix.height);
	switch(pv4l2Info->format->fmt.pix.pixelformat)
	{
		case V4L2_PIX_FMT_JPEG:
			printf("camera pixelformat: V4L2_PIX_FMT_JPEG\n");
			break;
		case V4L2_PIX_FMT_YUYV:
			printf("camera pixelformat: V4L2_PIX_FMT_YUYV\n");
			break;
		case V4L2_PIX_FMT_MJPEG:
			printf("camera pixelformat: V4L2_PIX_FMT_MJPEG\n");
			break;
		default:
			printf("=========== default ===========\n");
	}

	/* configure camera capture format */
	bzero(pv4l2Info->format, sizeof(struct v4l2_format));
	pv4l2Info->format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pv4l2Info->format->fmt.pix.width = LCD_XRES;
	pv4l2Info->format->fmt.pix.height = LCD_YRES;
	pv4l2Info->format->fmt.pix.pixelformat = V4L2_PIX_FMT_XXX;	//
	pv4l2Info->format->fmt.pix.field = V4L2_FIELD_INTERLACED;
	if(ioctl(pv4l2Info->cam_fd, VIDIOC_S_FMT, pv4l2Info->format) < 0)
	{
		printf("ioctl(cam_fd, VIDIOC_S_FMT, cam_fmt) failed ! \n");
		return -1; 
	}
	printf("wid * hei = %d * %d\n", pv4l2Info->format->fmt.pix.width, pv4l2Info->format->fmt.pix.height);

	// malloc memory for frameBuf to save frame
	pv4l2Info->frameBuf = (unsigned char *)calloc(1, FRAMEBUF_SIZE);
    if(NULL == pv4l2Info->frameBuf)
    {
        printf("malloc for frameBuf failure!\n");
        return -1;
    }
	
	/* check whether set success */
	pv4l2Info->format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl (pv4l2Info->cam_fd, VIDIOC_G_FMT, pv4l2Info->format) < 0){
        printf("ioctl (pv4l2Info->cam_fd, VIDIOC_G_FMT, pv4l2Info->format failed");
    }
	if(pv4l2Info->format->fmt.pix.pixelformat != V4L2_PIX_FMT_XXX)
	{
		printf("\nthe pixel format is NOT V4L2_PIX_FMT_XXX ! \n\n");
//		return -1;
	}

	printf("pixel format is V4L2_PIX_FMT_XXX ! \n");
	
	
	bzero(&pv4l2Info->reqbuf, sizeof(struct v4l2_requestbuffers));
	pv4l2Info->reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pv4l2Info->reqbuf.memory = V4L2_MEMORY_MMAP;
	pv4l2Info->reqbuf.count = BUF_NUM;
	//request 
	if(ioctl(pv4l2Info->cam_fd, VIDIOC_REQBUFS, &pv4l2Info->reqbuf) < 0)
	{
		printf("ioctl(cam_fd, VIDIOC_REQBUFS, &reqbuf) failed ! \n");
		return -1; 
	}
	printf("ioctl(cam_fd, VIDIOC_REQBUFS, &reqbuf) success ! \n");

	//根据设置的值，定义相应数量的struct v4l2_buffer
	//每struct v4l2_buffer对应内核摄像头的一个缓存
	for(i=0; i<BUF_NUM; i++)
	{
		bzero(&pv4l2Info->buffer[i], sizeof(struct v4l2_buffer));
		pv4l2Info->buffer[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		pv4l2Info->buffer[i].memory = V4L2_MEMORY_MMAP;
		pv4l2Info->buffer[i].index = i;
		if(ioctl(pv4l2Info->cam_fd, VIDIOC_QUERYBUF, &pv4l2Info->buffer[i]) < 0)
		{
			printf("ioctl(cam_fd, VIDIOC_QUERYBUF, &buffer[i]) failed ! \n");
			return -1; 
		}

		pv4l2Info->cam_buffer[i].length = pv4l2Info->buffer[i].length;
		pv4l2Info->cam_buffer[i].start = (unsigned char *)mmap(NULL, pv4l2Info->buffer[i].length, PROT_READ | PROT_WRITE, 
							MAP_SHARED, pv4l2Info->cam_fd, pv4l2Info->buffer[i].m.offset);

		if(ioctl(pv4l2Info->cam_fd, VIDIOC_QBUF, &pv4l2Info->buffer[i]) < 0)
		{
			printf("ioctl(cam_fd, VIDIOC_QBUF, &buffer[i]) failed ! \n");
			return -1; 
		}
	}	
	
	return 0;
}

int v4l2cap_start(struct v4l2capStru *pv4l2Info)
{
	enum v4l2_buf_type vtype;

	//turn on camera to capture 
	vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(pv4l2Info->cam_fd, VIDIOC_STREAMON, &vtype) < 0)
	{
		printf("ioctl(cam_fd, VIDIOC_STREAMON, &vtype) failed ! \n");
		return -1; 
	}
	printf("start Capture success.\n");

	return 0;

}

int v4l2cap_getframe(struct v4l2capStru * pv4l2Info)
{
	static int i = 0;
	
	bzero(&pv4l2Info->v4l2buf, sizeof(struct v4l2_buffer));
	pv4l2Info->v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pv4l2Info->v4l2buf.memory = V4L2_MEMORY_MMAP;
	
	pv4l2Info->v4l2buf.index = i % BUF_NUM;
	if(ioctl(pv4l2Info->cam_fd, VIDIOC_DQBUF, &pv4l2Info->v4l2buf) < 0)		//will block if no data
	{
		printf("ioctl(cam_fd, VIDIOC_DQBUF, &v4l2buf) failed ! \n");
		return -1; 
	}
	
	pthread_mutex_lock(&pv4l2Info->frameLock);
	memset(pv4l2Info->frameBuf, 0, FRAMEBUF_SIZE);
	memcpy(pv4l2Info->frameBuf, pv4l2Info->cam_buffer[pv4l2Info->v4l2buf.index].start, pv4l2Info->cam_buffer[pv4l2Info->v4l2buf.index].length); 
	pv4l2Info->frameLen = pv4l2Info->cam_buffer[pv4l2Info->v4l2buf.index].length;
  // printf("pv4l2Info->frameLen : %d\r\n",pv4l2Info->frameLen);
	pthread_mutex_unlock(&pv4l2Info->frameLock);
    if(pv4l2Info->frameBuf == NULL){
        printf("nmsl");
    }
    if(pv4l2Info->frameLen <= 0){
        printf("nmsl");
    }
    pv4l2Info->jun = pv4l2Info->frameLen;
	ioctl(pv4l2Info->cam_fd, VIDIOC_QBUF, &pv4l2Info->v4l2buf);

	i++;
	if(i > 60000)
		i=0;

	return 0;
}


void v4l2cap_deInit(struct v4l2capStru *pv4l2Info)
{
	int i;

	pthread_mutex_destroy(&pv4l2Info->frameLock);

	if(pv4l2Info->fmtdesc != NULL)
	{
		free(pv4l2Info->fmtdesc);
		pv4l2Info->fmtdesc = NULL;
	}

	if(pv4l2Info->format != NULL)
	{
		free(pv4l2Info->format);
		pv4l2Info->format = NULL;
	}

	if(pv4l2Info->frameBuf != NULL)
	{
		free(pv4l2Info->frameBuf);
		pv4l2Info->frameBuf = NULL;
	}

	for(i=0; i<BUF_NUM; i++)
	{
		munmap(pv4l2Info->cam_buffer[i].start, pv4l2Info->cam_buffer[i].length);
	}

	if(pv4l2Info->cam_fd > 0)
	{
		close(pv4l2Info->cam_fd);
		pv4l2Info->cam_fd = -1;
	}
	
}

