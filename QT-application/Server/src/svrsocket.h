#ifndef SVRSOCKET_H
#define SVRSOCKET_H

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

#define PRO_HEAD_CHAR	0xFF	// 协议头字符
#define PRO_TAIL_CHAR	0xFE	// 协议尾字符
#define JUN_BIAN		"JUN"	// 协议头特定字符
#define CMD_DETECT		0x01	// 检测结果
#define CMD_RECOGN		0x02	// 识别结果
#define CMD_V4L2DA		0x03	// V4L2图像数据
#define CMD_DRAW        0x04    // 启动抽纸

#define PRO_HEAD_LEN	5		// 协议头长度
#define PRO_SEQ_LEN		1		// 序号长度
#define PRO_CMD_LEN		1		// 命令号长度
#define PRO_INFO_LEN	12		// 附带信息长度
#define PRO_DLEN_LEN	4		// 数据长度长度
#define PRO_TAIL_LEN	1		// 协议尾长度
// 协议包格式总长度(除有效数据外的长度)
#define PRO_FORMAT_LEN	PRO_HEAD_LEN + PRO_SEQ_LEN + PRO_INFO_LEN + PRO_CMD_LEN + \
						PRO_DLEN_LEN + PRO_TAIL_LEN


#define RDWR_BUF_SIZE		(630000 * sizeof(unsigned char))


struct socketInfo
{
	int svrsockfd;		// 无用的，server的
	int consockfd;		// 用到的，用于连接的
	int connectSta;		// 连接状态 0-disconnect , 1-connect
	struct sockaddr_in svrAddr;		// server 地址
	struct sockaddr_in cliAddr;		// client 地址
	socklen_t 	svrlen;
	socklen_t 	clilen;
	int 		port;				// 端口

	unsigned char 		*sendBuf;
	int 				sendBLen;
	unsigned char 		*recvBuf;			// 存放接收的有效数据(完整的)
	int 				recvBLen;
	pthread_mutex_t		sendLock;
	pthread_mutex_t		recvLock;
	unsigned char 		*recvOrigBuf;			// 接收socket原始数据数据，未经处理
	int 		recvOrigLen;

};


void* socket_handle(void *arg);
int svrsocket_init(struct socketInfo *svrsocket);

int socket_send(int sockfd, unsigned char cmd, unsigned char *indata, int inlen);
int socket_recv(struct socketInfo *svrsocket);

void svrSocket_deInit(struct socketInfo *stSocket);

#endif // SVRSOCKET_H
