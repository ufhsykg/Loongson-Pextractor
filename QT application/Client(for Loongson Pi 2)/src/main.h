#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>


#define DEV_ON  	0x10000
#define DEV_OFF 	0x10001
#define DOOR_DEV	0

// ����ȫ�ֹ�����Ϣ
struct mainWorkStru
{
	int			workStatus;		// 0-exit, 1-work
	pthread_t	tid_socket;		// socket pthread
	pthread_t	tid_v4l2;		// v4l2 capture pthread
	pthread_t	tid_sockSend;		// socket send pthread
    pthread_t   tid_draw;
    unsigned int papercnt;

	// client ˽��
	int 		door_fd;		// door device file describe
    unsigned char junbuf[3];
};






#endif // MAIN_H
