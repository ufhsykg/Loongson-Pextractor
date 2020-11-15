#ifndef CLISOCKET_H
#define CLISOCKET_H

#include <stdio.h>
#include <string.h>
#include <asm/types.h>          /* for videodev2.h */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>


#define PRO_HEAD_CHAR	0xFF	// Э��ͷ�ַ�
#define PRO_TAIL_CHAR	0xFE	// Э��β�ַ�
#define ZENG_STR		"JUN"	// Э��ͷ�ض��ַ�
#define CMD_DETECT		0x01	// �����
#define CMD_RECOGN		0x02	// ʶ����
#define CMD_V4L2DA		0x03	// V4L2ͼ������
#define CMD_DRAW        0x04

#define PRO_HEAD_LEN	5		// Э��ͷ����
#define PRO_SEQ_LEN		1		// ��ų���
#define PRO_CMD_LEN		1		// ����ų���
#define PRO_INFO_LEN	12		// ������Ϣ����
#define PRO_DLEN_LEN	4		// ���ݳ��ȳ���
#define PRO_TAIL_LEN	1		// Э��β����
// Э�����ʽ�ܳ���(����Ч������ĳ���)
#define PRO_FORMAT_LEN	PRO_HEAD_LEN + PRO_SEQ_LEN + PRO_INFO_LEN + PRO_CMD_LEN + \
						PRO_DLEN_LEN + PRO_TAIL_LEN

#define RDWR_BUF_SIZE		(630000 * sizeof(unsigned char))

#define USER_NAME_LEN		64		// name len


struct rectStru
{
	int				x;
	int				y;
	int				w;
	int				h;
};

struct socketInfo
{
	char				chSvrIP[64];		// ������ IP
	int 				port;				// �˿�
	int svrsockfd;		// ���õģ�server��
	int consockfd;		// �õ��ģ��������ӵ�
	int connectSta;		// ����״̬ 0-disconnect , 1-connect
	struct sockaddr_in 	svrAddr;		// server ��ַ
	struct sockaddr_in 	cliAddr;		// client ��ַ
	socklen_t 			svrlen;
	socklen_t 			clilen;

	unsigned char 		*sendBuf;
	int 				sendBLen;
	unsigned char 		*recvBuf;			// ��Ž��յ���Ч����
	int 				recvBLen;
	pthread_mutex_t		sendLock;
	pthread_mutex_t		recvLock;
	unsigned char 		*recvOrigBuf;			// ����socketԭʼ�������ݣ�δ������
	int 				recvOrigLen;

	int 				detectFlag;			// ��⵽������� 1-��
	int					faceCount;			// ����
	struct rectStru		faceRect[20];		// ��������

	int 				predFace;		// ������� 0-���޴��ˣ�>0-��Ӧ���
	char 				predName[USER_NAME_LEN];	// user name
	unsigned char		faceConfd;		// ���ƶ�/���Ŷ�
	int					predChang;		// ���½��
};


void* socket_handle(void *arg);
void* draw_handle(void *arg);
int clisocket_init(struct socketInfo *clisocket);
void* socketSend_thread(void *arg);

int socket_send(int sockfd, unsigned char cmd, unsigned char *indata, int inlen);
int socket_recv(struct socketInfo *svrsocket);

void cliSocket_deInit(struct socketInfo *stSocket);


#endif // CLISOCKET_H
