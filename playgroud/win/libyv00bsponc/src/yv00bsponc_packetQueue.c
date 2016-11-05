/****************************************************************************************
 * 文 件 名	: yv00bsponc_packetQueue.c
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向网络通信库
 * 功   能	: 消息队列
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
#include <errno.h>
#include <malloc.h>

#include <list.h>
#include <yvcommon_def.h>
#include "yv00bsponc_util.h"
#include "yv00bsponc_debug.h"
#include "yv00bsponc_packetQueue.h"

inline
int bsponc_queueEmpty(ST_PacketQueue *pQueue)
{
	return pQueue->head == pQueue->tail;
}

int bsponc_queueInit(ST_PacketQueue *pQueue, int len)
{
	int i = 0;
	int lRet = YV_FAIL;
	ST_OncPacket *pPacket = NULL;
	
	if (NULL == pQueue
		|| len <= 0)
	{
		return -EINVAL;
	}

	
	INIT_LIST_HEAD(&pQueue->packetQueue);
	for (i = 0; i<len; i++)
	{
		pPacket = (ST_OncPacket *)malloc(sizeof (ST_OncPacket));
		if (NULL == pPacket)
		{
			DEV_DEBUG_INFO("malloc 分配内存失败");
			lRet = -ENOSPC;
			goto OUT;
		}
		INIT_LIST_HEAD(&pPacket->list);

		/* 添加到队列 */
		list_add(&pPacket->list, &pQueue->packetQueue);
	}

	pQueue->head = pQueue->packetQueue.next;
	pQueue->tail = pQueue->packetQueue.next;
	
	/* 使队列成为环形 */
	bsponc_MakeListLoop(&pQueue->packetQueue);

OUT:	
	return lRet;
}

int bsponc_queueFini(ST_PacketQueue *pQueue)
{
	ST_OncPacket *pPacket = NULL;
	struct list_head *pPos = NULL;
	struct list_head *pPosN = NULL;
	
	yv_Assert(pQueue);

	if (NULL == pQueue)
	{
		return -EINVAL;
	}

	/* 将链表恢复原状 */
	bsponc_UndoListLoop(&pQueue->packetQueue);

	list_for_each_safe(pPos, pPosN, &pQueue->packetQueue)
	{
		list_del(pPos);			/* 先移除再回收内存 */
		
		pPacket = list_entry(pPos, ST_OncPacket, list);
		if (pPacket)
		{
			free(pPacket);
		}
	}

	return 0;
}

int bsponc_queueEnqueue(ST_PacketQueue *pQueue, const ST_OncPacket *pPacket)
{
	ST_OncPacket *pNewPos = NULL;
	
	yv_Assert(pQueue);
	yv_Assert(pPacket);
	if (NULL == pQueue
		|| NULL == pPacket)
	{
		return -EINVAL;
	}

	/* TODO: 需判断队列是否还有空间 */
	
	/* 存入新 packet 到队列 */
	pNewPos = list_entry(pQueue->tail, ST_OncPacket, list); /* 添加到 tail */
	pNewPos->lDataLen = pPacket->lDataLen;
	pNewPos->pByData  = pPacket->pByData;
	pNewPos->lCrcCode = pPacket->lCrcCode;
	pNewPos->lMsgNO	  = pPacket->lMsgNO;
	pNewPos->lSeqNO	  = pPacket->lSeqNO;

	/* 后移 tail */
	pQueue->tail = pQueue->tail->next;
	
	return YV_SUCCESS;
}

int bsponc_packetDequeue(ST_PacketQueue *pQueue, ST_OncPacket *pPacket)
{
	ST_OncPacket *pHeadPkt = NULL;
	
	yv_Assert(pQueue);
	yv_Assert(pPacket);
	if (NULL == pQueue
		|| NULL == pPacket)
	{
		return -EINVAL;
	}

	if (bsponc_queueEmpty(pQueue))
	{	/* 队列为空 */
		return YV_FAIL;
	}

	/* 出队列 */
	pHeadPkt = list_entry(pQueue->head, ST_OncPacket, list); /* 从 head 取出一包 */
	pPacket->lDataLen = pHeadPkt->lDataLen;
	pPacket->pByData  = pHeadPkt->pByData;
	pPacket->lCrcCode = pHeadPkt->lCrcCode;
	pPacket->lMsgNO	  = pHeadPkt->lMsgNO;
	pPacket->lSeqNO	  = pHeadPkt->lSeqNO;

	/* 后移 head */
	pQueue->head = pQueue->head->next;
	
	return 0;
}
