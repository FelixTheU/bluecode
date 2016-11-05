/****************************************************************************************
 * 文 件 名	: yv00bsponc.c
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向网络通信库--实现文件
 * 功   能	: 提供用于单向网络通信的收发功能
 * 操作系统	: LINUX
 * 修改记录	: 无
 * 版   本	: Rev 0.1.0
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 设    计	: zhengsw      '2016-11-03
 * 编    码	: zhengsw      '2016-11-03
 * 修    改	: 
 ****************************************************************************************
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 公司介绍及版权说明
 *
 *           (C)Copyright 2012 YView    Corporation All Rights Reserved.
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 ***************************************************************************************/
#include "yv00bsponc.h"

#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include <list.h>
#include <yvcommon_def.h>
#include "yv00bsponc_util.h"
#include "yv00bsponc_debug.h"
#include "yv00bsponc_packetQueue.h"
#include "yv00bsponc_type.h"
#include "yv00bsponc_netIO.h"

#define ONC_SEND_QUEUE_LEN			(500)		/* 默认发送队长长度 */


static inline
void *threadFun_sender(void *pFunData)
{
	int				 lSts	= YV_FAIL;
	ST_OncSend		*pOnc	= NULL;
	ST_OncPacket	 packet = ONC_PACKET_INITIALIZER(packet);
	
	yv_Assert(pFunData);
	if (NULL == pFunData)
	{
		goto OUT;
	}

	while (pOnc->bThreadGoOn)
	{
		/* 条件变量阻塞 */
		pthread_mutex_lock(&pOnc->mutexCond);
		while (bsponc_queueEmpty(&pOnc->sendQueue))
		{
			pthread_cond_wait(&pOnc->condQueue, &pOnc->mutexCond);
		}
		pthread_mutex_unlock(&pOnc->mutexCond);
		
		/* 取一包消息出队列 */
		bsponc_dequeue(&pOnc->sendQueue, &packet);

		/* 发送一包消息，可能会分包 */
		lSts = bsponc_sendPacket(pOnc, &packet);
		if (lSts < 0)
		{
			DEV_DEBUG_INFO("sendPacket 发送失败");
		}

		/* 回收消息的内存 */
		free(packet.pByData);						/* [free ByData] */
	}

OUT:	
	return NULL;
}


/* 发送接口 */
int bsponc_sendInit(ONC_SendHandle *pHandle, unsigned short sendPort, char *pszPeerIp, unsigned short peerPort)
{
	int			 lRet		= YV_FAIL;
	int			 lSts		= YV_FAIL;
	ST_OncSend	*pOnc		= NULL;
	in_addr_t	 peerIpAddr = 0;
	
	yv_Assert(pHandle);
	yv_Assert(pszPeerIp);
	if (NULL == pHandle
		|| NULL == pszPeerIp)
	{
		DEV_DEBUG_INFO("传入参数有误");
		lRet = -EINVAL;
		goto OUT;
	}

	/* 将点分十进制 ip 地址转换为 32位数值 */
	peerIpAddr = inet_addr(pszPeerIp);
	if (INADDR_NONE == peerIpAddr)
	{
		DEV_DEBUG_INFO("传入参数有误");
		lRet = -EINVAL;
		goto OUT;
	}

	pOnc = (ST_OncSend *)malloc(sizeof (ST_OncSend));				/* [malloc ST_Onc],  */
	if (NULL == pOnc)
	{
		DEV_DEBUG_INFO("malloc 为 ST_Onc 分配内存失败");
		lRet = -ENOSPC;
		goto OUT;
	}

	bzero(pOnc, sizeof(ST_OncSend));

	/* 初始化包序号与消息编号 */
	pOnc->lNextMsgNO = bsponc_SeqNoInit();
	pOnc->lNextSeqNO = bsponc_MsgNoInit();
	
	/* 本端地址信息 */
	pOnc->stSelfSockAddr.sin_family		 = AF_INET;
	pOnc->stSelfSockAddr.sin_port		 = htons(sendPort);		/* 端口可以指定，也可以给0 让系统分配 */
	pOnc->stSelfSockAddr.sin_addr.s_addr = INADDR_ANY;			/* 地址默认选择 */

	/* 对端地址信息 */
	pOnc->stPeerSockAddr.sin_family		 = AF_INET;
	pOnc->stPeerSockAddr.sin_port		 = htons(peerPort);
	pOnc->stPeerSockAddr.sin_addr.s_addr = peerIpAddr;
	
	/* 创建 udp socket */
	lSts = socket(PF_INET, SOCK_DGRAM, 0);			/* udp socket */
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("socket 创建失败");
		lRet = lSts;
		goto FREE;
	}

	pOnc->sockFd = lSts;
	lSts = bind(pOnc->sockFd, (struct sockaddr *)&pOnc->stSelfSockAddr, sizeof (pOnc->stSelfSockAddr));
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("socket bind 失败");
		lRet = lSts;
		goto CLOSE;
	}

	/* 初始化发送队列 */
	lSts = bsponc_queueInit(&pOnc->sendQueue, ONC_SEND_QUEUE_LEN);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("初始化发送队列失败");
		lRet = lSts;
		goto CLOSE;
	}

	/* 初始化队列互斥器与变量变量 */
	lSts = pthread_mutex_init(&pOnc->mutexQueue, NULL);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("队列互斥器创建失败");
		lRet = lSts;
		goto CLEAN_QUEUE;
	}

	lSts = pthread_mutex_init(&pOnc->mutexCond, NULL);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("条件变量互斥器创建失败");
		lRet = lSts;
		goto CLEAN_MUTEX_QUEUE;
	}

	lSts = pthread_cond_init(&pOnc->condQueue, NULL);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("队列条件变量创建失败");
		lRet = lSts;
		goto CLEAN_MUTEX_COND;
	}

	/* 创建发送线程 */
	pOnc->bThreadGoOn = TRUE;
	lSts = pthread_create(&pOnc->tidSender, NULL, threadFun_sender, pOnc);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("发送线程创建失败");
		lRet = lSts;
		goto CLEAN_COND;
	}

	*pHandle = pOnc;
	lRet = YV_SUCCESS;
	goto OUT;

CLEAN_COND:
	pthread_cond_destroy(&pOnc->condQueue);	

CLEAN_MUTEX_COND:
	pthread_mutex_destroy(&pOnc->mutexCond);

CLEAN_MUTEX_QUEUE:
	pthread_mutex_destroy(&pOnc->mutexQueue);
	
CLEAN_QUEUE:
	bsponc_queueFini(&pOnc->sendQueue);

CLOSE:
	close(pOnc->sockFd);

FREE:
	free(pOnc);

OUT:	
	return lRet;
}


/****************************************************************************************
 * 函 数 名	: bsponc_sendFini
 * 功   能	: 发送功能 finish
 * 输入参数	: handle: 
 * 输出参数	: 无
 * 返 回 值	: YV_SUCCSS: 成功; YV_FAIL: 失败
 ***************************************************************************************/
int bsponc_sendFini(ONC_SendHandle handle)
{
	int lRet = YV_FAIL;
	void *threadRet = NULL;
	ST_OncSend *pOnc = NULL;
	
	yv_Assert(handle);
	if (NULL == handle)
	{
		DEV_DEBUG_INFO("函数参数无效");
		lRet = -EINVAL;
		goto OUT;
	}

	handle->bThreadGoOn = FALSE;
	pthread_join(handle->tidSender, &threadRet);

	pOnc = handle;
	pthread_cond_destroy(&pOnc->condQueue);	
	pthread_mutex_destroy(&pOnc->mutexQueue);
	pthread_mutex_destroy(&pOnc->mutexCond);
	
	bsponc_queueFini(&pOnc->sendQueue);
	
	close(pOnc->sockFd);
	free(pOnc);									/* [free ST_Onc] */
OUT:	
	return lRet;
}

int bsponc_send(ONC_SendHandle handle, char *pByData, int lDataLen)
{
	int				lSts					 = YV_FAIL;
	int				lRet					 = YV_FAIL;
	ST_OncPacket	packet					 = ONC_PACKET_INITIALIZER(packet);
	int				bQueueEmptyBefore = FALSE;

	yv_Assert(handle);
	yv_Assert(pByData);
	yv_Assert(lDataLen >= 0);
	if (NULL == handle
		|| NULL == pByData
		|| lDataLen < 0)
	{
		DEV_DEBUG_INFO("函数参数无效");
		lRet = -EINVAL;
		goto OUT;
	}

	/* 缓存新的一包消息，分配空间，后面需要替换成内存池 */
	packet.pByData = (char *)malloc(lDataLen);				/* 为新的一包消息分配内存 [malloc ByData]*/
	if (NULL == packet.pByData)
	{
		DEV_DEBUG_INFO("malloc 分配失败");
		lRet = -ENOSPC;
		goto OUT;
	}

	/* 准备 packet  */
	memcpy(packet.pByData, pByData, lDataLen);
	packet.lDataLen = lDataLen;
	packet.lMsgNO = bsponc_MsgNO_PostInc(&handle->lNextMsgNO); /* 此处只增加 msgNO, seqNo 需要在每发一包就即增加 */

	/* 加入一包消息到队列中 */
	pthread_mutex_lock(&handle->mutexQueue);					/* [队列加锁] */
	bQueueEmptyBefore = bsponc_queueEmpty(&handle->sendQueue);	/* 记录下在加入本包之前队列是否为空 */
	lSts = bsponc_enqueue(&handle->sendQueue, &packet);			/* 将新的一包消息加入队列 */
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("packet 加入队列失败");
		lRet = lSts;
		goto CLEAN;
	}
	pthread_mutex_unlock(&handle->mutexQueue);				/* [队列解锁] */
	
	/* 条件变量唤醒发送线程 */
	pthread_mutex_lock(&handle->mutexCond);					/* [条件变量加锁] */
	if (bQueueEmptyBefore)
	{	/* 只在队列为空时唤醒 */
		pthread_cond_signal(&handle->condQueue);			/* 唤醒 */
	}
	pthread_mutex_unlock(&handle->mutexCond);				/* [条件变量解锁] */
	goto OUT;
	
CLEAN:
	free(packet.pByData);									/* [free ByData] */
	pthread_mutex_unlock(&handle->mutexQueue);				/* [队列解锁] */

OUT:	
	return lRet;
}

