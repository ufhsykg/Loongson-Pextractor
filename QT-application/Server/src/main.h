#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>


#define CSV_FILE	"faces.csv"


// 管理全局工作信息
struct mainWorkStru
{
	int			workStatus;		// 0-exit, 1-work
	pthread_t	tid_Sock;		// socket pthread
	pthread_t	tid_facOpen;		// face opencv deal pthread
	pthread_t	tid_facDetRec;		// face detect and recognize pthread
	pthread_t	tid_addFace;		// add user face

	int 		opcvDealReboot;		// opencv deal modules restart flag
};






#endif // MAIN_H
