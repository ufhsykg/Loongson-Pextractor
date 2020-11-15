#include "svrsocket.h"
#include "main.h"


struct socketInfo	svrSocketInfo;

extern struct mainWorkStru 	gstMainWork;

void* socket_handle(void *arg)
{
	printf("%s: enter.\n", __FUNCTION__);
	
	int 	ret;
	fd_set 	rdfds;
	struct timeval tv;
	
	ret = svrsocket_init(&svrSocketInfo);
	if(ret != 0)
	{
		printf("%s: clisocket_init failed.\n", __FUNCTION__);
		return (void *)0;
	}

	while(gstMainWork.workStatus)
	{
		if(!svrSocketInfo.connectSta)	// 无连接
		{
			usleep(1000);
			continue;
		}

		// 配置select, 每次都要重新配置
		FD_ZERO(&rdfds);
		FD_SET(svrSocketInfo.consockfd, &rdfds);
		
		tv.tv_sec = 2;
		tv.tv_usec = 500;
		
        ret = select(svrSocketInfo.consockfd+1, &rdfds, NULL, NULL, &tv);
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
			if(FD_ISSET(svrSocketInfo.consockfd, &rdfds))
			{
				if(svrSocketInfo.connectSta == 1)
				{
					socket_recv(&svrSocketInfo);
				}
			}
		}
	}

	svrSocket_deInit(&svrSocketInfo);

	return (void *)0;
}


int svrsocket_init(struct socketInfo *svrsocket)
{
	int ret;

	pthread_mutex_init(&svrsocket->sendLock, NULL);
	pthread_mutex_init(&svrsocket->recvLock, NULL);
	svrsocket->connectSta = 0;

	svrsocket->recvBLen = 0;
	svrsocket->recvBuf = (unsigned char *)malloc(RDWR_BUF_SIZE);
	if(svrsocket->recvBuf == NULL)
	{
		printf("%s: malloc for svrsocket->recvBuf failed.\n", __FUNCTION__);
		return -1;
	}
	
	svrsocket->recvOrigLen = 0;
	svrsocket->recvOrigBuf = (unsigned char *)malloc(RDWR_BUF_SIZE);
	if(svrsocket->recvOrigBuf == NULL)
	{
		printf("%s: malloc for svrsocket->recvOrigBuf failed.\n", __FUNCTION__);
		return -1;
	}

	svrsocket->svrsockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(svrsocket->svrsockfd < 0)
	{
		printf("socket() failed.\n");
		return -1;
	}
	
	svrsocket->svrAddr.sin_family = AF_INET;
	svrsocket->svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svrsocket->svrAddr.sin_port = htons(svrsocket->port);
	svrsocket->svrlen = sizeof(svrsocket->svrAddr);

	ret = bind(svrsocket->svrsockfd, (struct sockaddr *)&svrsocket->svrAddr, svrsocket->svrlen);
	if(ret < 0)
	{
		close(svrsocket->svrsockfd);
		printf("bind() failed.\n");
		return -1;
	}
	
	ret = listen(svrsocket->svrsockfd, 3);
	if(ret < 0)
	{
		close(svrsocket->svrsockfd);
		printf("listen() failed.\n");
		return -1;
	}
	
	svrsocket->consockfd = accept(svrsocket->svrsockfd, (struct sockaddr *)&svrsocket->cliAddr, &svrsocket->clilen);
	if(ret < 0)
	{
		close(svrsocket->svrsockfd);
		printf("accept() failed.\n");
		return -1;
	}
	
	svrsocket->connectSta = 1;

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

	if(NULL==indata || 0==inlen)
	{
		printf("%s: param is illegal.\n", __FUNCTION__);
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
    memcpy(tmpBuf+tmplen +1, JUN_BIAN, 4);
	tmplen += PRO_HEAD_LEN;

	// SEP
	tmpBuf[tmplen] = ++sendSeq;
	tmplen += PRO_SEQ_LEN;

	// CMD
	tmpBuf[tmplen] = cmd;
	tmplen += PRO_CMD_LEN;

	// 附带信息
    memcpy(tmpBuf+tmplen, "from junbian", PRO_INFO_LEN);
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
	
	ret = write(sockfd, tmpBuf, tmplen);
	if(ret != tmplen)
	{
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
	unsigned char Seq;
	unsigned char Cmd;
	int ret = 0;
	int pos = 0;
	int datalen = 0;
	static int dataNotCom = 0;		// 数据接收未完整

	if(NULL==stSocket)
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
            printf("%s: find packet head [recvOrigLen: %d].\n", __FUNCTION__, stSocket->recvOrigLen);
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
        if(strncmp((char *)stSocket->recvOrigBuf+1, JUN_BIAN, 4) == 0)
		{
            printf("find head char success");
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
//				printf("%s: remain not enough[%d : %d].\n", __FUNCTION__, pos+datalen+PRO_TAIL_LEN, stSocket->recvOrigLen);
				return ret;
			}

			if(stSocket->recvOrigBuf[pos+datalen] == PRO_TAIL_CHAR)	// 包尾，数据完整
			{
				/* 有效数据 */
				switch(Cmd)
				{
					case CMD_DETECT:	break;
					case CMD_RECOGN:	break;
					case CMD_V4L2DA:
						pthread_mutex_lock(&stSocket->recvLock);
						memcpy(stSocket->recvBuf, stSocket->recvOrigBuf+pos, datalen);
						stSocket->recvBLen = datalen;
						pthread_mutex_unlock(&stSocket->recvLock);
						break;
					default:
						break;
				}
				
				pos += datalen;		// 数据
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

void svrSocket_deInit(struct socketInfo *stSocket)
{

	stSocket->connectSta = 0;

	pthread_mutex_destroy(&stSocket->sendLock);
	pthread_mutex_destroy(&stSocket->recvLock);

	if(stSocket->consockfd > 0)
	{
		close(stSocket->consockfd);
		stSocket->consockfd = -1;
	}
	
	if(stSocket->svrsockfd > 0)
	{
		close(stSocket->svrsockfd);
		stSocket->svrsockfd = -1;
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
	
}

