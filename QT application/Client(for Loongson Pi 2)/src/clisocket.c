#include "clisocket.h"
#include "v4l2_cap.h"
#include <time.h>
#include "main.h"


struct socketInfo		cliSocketInfo;

extern struct mainWorkStru 		gstMainWork;
extern struct v4l2capStru		gv4l2capInfo;


void* socket_handle(void *arg)
{
	int	 	ret;
	fd_set 	rdfds;
	struct timeval tv;
	
	
	ret = clisocket_init(&cliSocketInfo);

	if(ret != 0)
	{
		printf("%s: clisocket_init failed.\n", __FUNCTION__);
		return (void *)0;
	}
	printf("%s: clisocket_init success.\n", __FUNCTION__);

	ret = pthread_create(&gstMainWork.tid_sockSend, NULL, socketSend_thread, NULL);
	if(ret < 0)
	{
		printf("socketSend_thread() failed.\n");
		return (void *)0;
	}
	printf("socketSend_thread() success.\n");
	
	while(gstMainWork.workStatus)
	{
		if(!cliSocketInfo.connectSta)	// 无连接
		{
			usleep(1000);
			continue;
		}

		// 配置select, 每次都要重新配置
		FD_ZERO(&rdfds);
        FD_SET(cliSocketInfo.consockfd, &rdfds);
		
		tv.tv_sec = 2;
		tv.tv_usec = 500;
		
        ret = select(cliSocketInfo.consockfd+1, &rdfds, NULL, NULL, &tv);
        printf("select\r\n");
		if(ret < 0)
		{
			printf("%s: select failed.\n", __FUNCTION__);
		}
		else if(ret == 0)
		{
            printf("%s: select time out.\n", __FUNCTION__);
		}
		else	// can read
		{
			if(FD_ISSET(cliSocketInfo.consockfd, &rdfds))
			{
				if(cliSocketInfo.connectSta == 1)
                {
					socket_recv(&cliSocketInfo);

				}
			}
		}
	}

	cliSocket_deInit(&cliSocketInfo);
	

	return (void *)0;
}

void* draw_handle(void *arg)
{
    int val = *(int *)arg;
    gstMainWork.junbuf[0] = val;
    write(gstMainWork.door_fd,gstMainWork.junbuf,sizeof(gstMainWork.junbuf));
    pthread_detach(pthread_self());
}
void* socketSend_thread(void *arg)
{
	int	 	ret;
	int 	sendLen = 0;

    cliSocketInfo.sendBuf = (unsigned char *)malloc(RDWR_BUF_SIZE);
	if(cliSocketInfo.sendBuf == NULL)
	{
		printf("%s: malloc for tmpSendBuf failed.\n", __FUNCTION__);
		return (void *)0;
	}
	
	while(gstMainWork.workStatus)
	{
		if(cliSocketInfo.connectSta == 1)
		{
			/* 复制发送数据，避免在socket_send加锁阻塞过久 */
			pthread_mutex_lock(&gv4l2capInfo.frameLock);
			sendLen = gv4l2capInfo.frameLen;
            sendLen = gv4l2capInfo.jun;
       //   sendLen = 614989;
       //     printf("Sendlen is %d\r\n",sendLen);
            memcpy(cliSocketInfo.sendBuf, gv4l2capInfo.frameBuf, sendLen);
			pthread_mutex_unlock(&gv4l2capInfo.frameLock);
			
            ret = socket_send(cliSocketInfo.consockfd, CMD_V4L2DA, cliSocketInfo.sendBuf, sendLen);

		}
	}
	
	if(cliSocketInfo.sendBuf != NULL)
	{
        free(cliSocketInfo.sendBuf);
		cliSocketInfo.sendBuf = NULL;
	}

}

int clisocket_init(struct socketInfo *clisocket)
{
	int 	ret;
    int 	fdFlag;

	pthread_mutex_init(&clisocket->sendLock, NULL);
	pthread_mutex_init(&clisocket->recvLock, NULL);
	clisocket->detectFlag = 0;
	clisocket->predFace = 0;
	clisocket->connectSta = 0;
	clisocket->predChang = 0;
	clisocket->faceConfd = 0;
	clisocket->faceCount = 0;
	memset(&clisocket->faceRect, 0, sizeof(struct rectStru));

	clisocket->recvBLen = 0;
	clisocket->recvBuf = (unsigned char *)malloc(RDWR_BUF_SIZE);
	if(clisocket->recvBuf == NULL)
	{
		printf("%s: malloc for clisocket->recvBuf failed.\n", __FUNCTION__);
		return -1;
	}
	
	clisocket->recvOrigLen = 0;
	clisocket->recvOrigBuf = (unsigned char *)malloc(RDWR_BUF_SIZE);
	if(clisocket->recvOrigBuf == NULL)
	{
		printf("%s: malloc for clisocket->recvOrigBuf failed.\n", __FUNCTION__);
		return -1;
	}

	clisocket->consockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(clisocket->consockfd < 0)
	{
		printf("socket() failed.\n");
		return -1;
	}
	printf("socket() success.\n");
	
	clisocket->svrAddr.sin_family = AF_INET;
	inet_pton(AF_INET, (const char *)clisocket->chSvrIP, &clisocket->svrAddr.sin_addr);
	clisocket->svrAddr.sin_port = htons(clisocket->port);
	clisocket->svrlen = sizeof(clisocket->svrAddr);

	/* 设置socket非阻塞: 非阻塞会导致write失败，不完整 */
//    fdFlag=fcntl(clisocket->consockfd, F_GETFL, 0);
//    fcntl(clisocket->consockfd, F_SETFL, fdFlag | O_NONBLOCK);
	
	while(1)
	{
		ret = connect(clisocket->consockfd, (struct sockaddr *)&clisocket->svrAddr, clisocket->svrlen);
		if(ret >= 0)
			break;

		usleep(20000);
	}
	clisocket->connectSta = 1;	// 连接成功
	printf("connect() %s:%d success.\n", clisocket->chSvrIP, clisocket->port);
//    printf("jun :%d",gv4l2capInfo.frameLen);
	return 0;
}

/* 发送socket数据，自动封装 */
// indata: 有效数据
int socket_send(int sockfd, unsigned char cmd, unsigned char *indata, int inlen)
{
	static unsigned char sendSeq = 0;
	unsigned char *tmpBuf = NULL;
	int tmplen = 0;
	int ret = 0;

    if(NULL==indata){
        printf("NULL==indata");
    }

	if(NULL==indata || 0==inlen)
	{
       // printf("%s: param is illegal.\n", __FUNCTION__);
		return -1;
	}


	tmpBuf = (unsigned char *)malloc(inlen + PRO_FORMAT_LEN);
	if(tmpBuf == NULL)
	{
		printf("%s: malloc for tmpBuf failed.\n", __FUNCTION__);
		return -1;
	}

	// 协议头
	tmpBuf[tmplen +0] = PRO_HEAD_CHAR;
	memcpy(tmpBuf+tmplen +1, ZENG_STR, 4);
	tmplen += PRO_HEAD_LEN;

	// SEP
	tmpBuf[tmplen] = ++sendSeq;
	tmplen += PRO_SEQ_LEN;

	// CMD
	tmpBuf[tmplen] = cmd;
	tmplen += PRO_CMD_LEN;

	// 附带信息
	memcpy(tmpBuf+tmplen, "from zeng", PRO_INFO_LEN);
	tmplen += PRO_INFO_LEN;

	// 数据长度
	memcpy(tmpBuf+tmplen, &inlen, PRO_DLEN_LEN);
	tmplen += PRO_DLEN_LEN;

	// 数据
	memcpy(tmpBuf+tmplen, indata, inlen);
	tmplen += inlen;
	
	// 包尾
	tmpBuf[tmplen] = PRO_TAIL_CHAR;
	tmplen += PRO_TAIL_LEN;
	
    printf("%s: before write -------------------------------------\n", __FUNCTION__);
	ret = write(sockfd, tmpBuf, tmplen);
	if(ret != tmplen)
	{
        if (ret == EINTR) printf("%s: Interrupt ERROR",__FUNCTION__);
        if (ret == EPIPE) printf("%s: Opposite party to close connection",__FUNCTION__);
		printf("%s: write socket failed.\n", __FUNCTION__);
		free(tmpBuf);
		return -1;
	}
    printf("%s: write socket %d [%d] bytes[cmd: 0x%02x, seq: %d] .\n", __FUNCTION__, ret, inlen, cmd, sendSeq);

	free(tmpBuf);

	return ret;
}

/* 读取socket数据 */
// outdata: 提取的有效数据
int socket_recv(struct socketInfo *stSocket)
{
	int i = 0;
	unsigned char Seq;
	unsigned char Cmd;
	int ret = 0;
	int pos = 0;
	int datalen = 0;
	static int dataNotCom = 0;		// 数据接收未完整

	if(NULL == stSocket)
	{
		printf("%s: param is illegal.\n", __FUNCTION__);
		return -1;
	}

	// 数据不足一包或无数据则接收，否则先处理已接收的，以防在read阻塞导致处理不及时
	if(dataNotCom || stSocket->recvOrigLen==0)
	{
		// 读出socket数据到recvSockBuf
		ret = read(stSocket->consockfd, stSocket->recvOrigBuf+stSocket->recvOrigLen, 
					RDWR_BUF_SIZE - stSocket->recvOrigLen);
		if(ret < 0)
		{
			printf("%s: read socket failed[ret=%d].\n", __FUNCTION__, ret);
			return -1;
		}
		
		stSocket->recvOrigLen += ret;
        printf("%s: recv %d b, want %d, total: %d.\n", __FUNCTION__, ret, RDWR_BUF_SIZE-stSocket->recvOrigLen, stSocket->recvOrigLen);
		if(stSocket->recvOrigLen <= 0)
		{
			return ret;
		}
	}

	/* 检测包头 */
	while(pos < stSocket->recvOrigLen)
	{
		if(stSocket->recvOrigBuf[pos] == PRO_HEAD_CHAR)	// 检测到包头
		{
			/* 将包头开始处移到0处 */
			memcpy(stSocket->recvOrigBuf, stSocket->recvOrigBuf+pos, stSocket->recvOrigLen-pos);
			stSocket->recvOrigLen -= pos;
            //printf("%s: find packet head [recvOrigLen: %d].\n", __FUNCTION__, svrsocket->recvOrigLen);
			break;
		}
		
		pos ++;
		if(pos >= stSocket->recvOrigLen)	// 遍历完成，无包头
		{
			stSocket->recvOrigLen = 0;
			return ret;		// 退出
		}
	}

	pos = 0;
	/* 进一步检测是否协议包 */
	if(stSocket->recvOrigBuf[0] == PRO_HEAD_CHAR)	// 检测到包头
	{
		if(stSocket->recvOrigLen <= PRO_FORMAT_LEN)	// 数据不够(要>格式大小)，等下次
		{
			dataNotCom = 1;
			return ret;
		}
		
		/* 且验证正确 */
		if(strncmp((char *)stSocket->recvOrigBuf+1, ZENG_STR, 4) == 0)
		{
			pos += PRO_HEAD_LEN;	// 包头
			
			Seq = stSocket->recvOrigBuf[pos];		// SEQ
			pos += PRO_SEQ_LEN;
			
			Cmd = stSocket->recvOrigBuf[pos];		// CMD
			pos += PRO_CMD_LEN;
			
			pos += PRO_INFO_LEN;	// 附带信息
			memcpy(&datalen, stSocket->recvOrigBuf+pos, PRO_DLEN_LEN);	// 数据长度
			pos += PRO_DLEN_LEN;	// 数据长度
			if(pos+datalen+PRO_TAIL_LEN > stSocket->recvOrigLen)	// 数据不足一包
			{
				dataNotCom = 1;
                //printf("%s: remain not enough[%d : %d].\n", __FUNCTION__, pos+datalen+PRO_TAIL_LEN, svrsocket->recvOrigLen);
				return ret;
			}

			if(stSocket->recvOrigBuf[pos+datalen] == PRO_TAIL_CHAR)	// 包尾，数据完整
			{
				/* 有效数据 */
				switch(Cmd)
				{
					case CMD_DETECT:	// 人脸检测结果
						/* 数量 */
						cliSocketInfo.faceCount = stSocket->recvOrigBuf[pos];
						pos += 1;

						/* 矩形参数 */
						for(i=0; i<cliSocketInfo.faceCount; i++)
						{
							memcpy(&cliSocketInfo.faceRect[i].x, stSocket->recvOrigBuf+pos +0, 4);
							memcpy(&cliSocketInfo.faceRect[i].y, stSocket->recvOrigBuf+pos +4, 4);
							memcpy(&cliSocketInfo.faceRect[i].w, stSocket->recvOrigBuf+pos +8, 4);
							memcpy(&cliSocketInfo.faceRect[i].h, stSocket->recvOrigBuf+pos +12, 4);
							pos += 4*4;
                            printf("face[%d]: %d, %d, %d, %d\n", i, cliSocketInfo.faceRect[i].x, cliSocketInfo.faceRect[i].y,
                                        cliSocketInfo.faceRect[i].w, cliSocketInfo.faceRect[i].h);
						}
						cliSocketInfo.detectFlag = 1;
                        //printf("face: %d, %d, %d, %d\n", cliSocketInfo.face_x, cliSocketInfo.face_y,
                        //        cliSocketInfo.face_w, cliSocketInfo.face_h);
						
						break;

					case CMD_RECOGN:		// 人脸识别结果
						pos += 1;			// count
                        //printf("faces num: %d.\n", svrsocket->recvOrigBuf[pos]);

						// face label
						memcpy(&cliSocketInfo.predFace, stSocket->recvOrigBuf+pos, 4);
						pos += 4;

						// user name
						memcpy(cliSocketInfo.predName, stSocket->recvOrigBuf+pos, USER_NAME_LEN);
						pos += USER_NAME_LEN;
						
						stSocket->faceConfd = stSocket->recvOrigBuf[pos];		// 相似度
						pos += 1;		

						if(cliSocketInfo.predFace > 0)
						{
							stSocket->predChang = 1;
						}
						break;
						
					case CMD_V4L2DA:
						break;
						
                    case CMD_DRAW:
                        gstMainWork.junbuf[0] = 1;
                        write(gstMainWork.door_fd,gstMainWork.junbuf,sizeof(gstMainWork.junbuf));
                        gstMainWork.papercnt++;
                        break;

					default:
						break;
				}
				
				pos += PRO_TAIL_LEN;

				dataNotCom = 0;
                printf("%s: recv socket %d bytes data[cmd: 0x%02x, seq: %d].\n", __FUNCTION__, datalen, Cmd, Seq);
			}

			/* 清空已验数据 */
			memcpy(stSocket->recvOrigBuf, stSocket->recvOrigBuf+pos, stSocket->recvOrigLen-pos);
			stSocket->recvOrigLen -= pos;
		}
		else	// 验证失败，移除包头
		{
			printf("%s: check failed and remove head.\n", __FUNCTION__);
			memcpy(stSocket->recvOrigBuf, stSocket->recvOrigBuf+1, 
						stSocket->recvOrigLen - 1);
			stSocket->recvOrigLen = stSocket->recvOrigLen - 1;
		}
	}

	return ret;
}

void cliSocket_deInit(struct socketInfo *stSocket)
{
	printf("%s: enter.\n", __FUNCTION__);
	
	stSocket->connectSta = 0;
	
	pthread_mutex_destroy(&stSocket->sendLock);
	pthread_mutex_destroy(&stSocket->recvLock);

	if(stSocket->consockfd > 0)
	{
		close(stSocket->consockfd);
		stSocket->consockfd = -1;
	}

	if(stSocket->recvOrigBuf != NULL)
	{
		free(stSocket->recvOrigBuf);
		stSocket->recvOrigBuf = NULL;
	}

	if(stSocket->recvBuf != NULL)
	{
		free(stSocket->recvBuf);
		stSocket->recvBuf = NULL;
	}

	if(stSocket->sendBuf != NULL)
	{
		free(stSocket->sendBuf);
		stSocket->sendBuf = NULL;
	}
	
}

