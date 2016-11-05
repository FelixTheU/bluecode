/****************************************************************************************
 * 文 件 名	: yv00bsponc_util.c
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向网络通信库
 * 功   能	: 工具
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
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <list.h>
#include "yv00bsponc_util.h"
#include "yv00bsponc_debug.h"

/* 包序号 */
#define ONC_SEQ_CMP_TH						0x7FFFFFF           /* 包序号比较差值的阀值，用来判断是否发生回绕 */
#define ONC_SEQ_MAX							0xFFFFFFF          	/* 包序号最大值 */

/* 包消息编号 */
#define ONC_MSGID_CMP_TH						0xFFFFFFF          	/* 消息号比较差值的阀值，用来判断是否发生回绕 */
#define ONC_MSGID_MAX							0x1FFFFFFF        	/* 消息号最大值 */

unsigned int bsponc_SeqNoInit()
{
	/* 初始化包序号 */
	srand((unsigned)time(NULL));
    return  (rand() % 65535) + 1;	/* random value 1-----65536，保证不可能为 0 */
}

unsigned int bsponc_MsgNoInit()
{
	/* 初始化消息编号 */
	srand((unsigned)time(NULL));
    return  (rand() % 65535) + 1;	/* random value 1-----65536，保证不可能为 0 */
}


/****************************************************************************************
 * 函 数 名	: bsponc_SeqInc
 * 功   能	: 包序号递增操作
 * 输入参数	: seq: 序号
 * 输出参数	: 无
 * 返 回 值	: 递加后的包序号
 ***************************************************************************************/
unsigned int bsponc_SeqPostInc(unsigned int *pSeq)
{
	unsigned int lRet = *pSeq;
	if (ONC_SEQ_MAX == ++*pSeq)
	{	/* 回绕 */
		*pSeq = 1;
	}

	return lRet;
}

/****************************************************************************************
 * 函 数 名	: bsponc_SeqDec
 * 功   能	: 包序号递减操作
 * 输入参数	: seq: 序号
 * 输出参数	: 无
 * 返 回 值	: 减小后的包序号
 ***************************************************************************************/
unsigned int bsponc_SeqPostDec(unsigned int *pSeq)
{
	unsigned int lRet = *pSeq;
	if (0 == --*pSeq)
	{
		*pSeq = ONC_SEQ_MAX;
	}

	return lRet;
}

/****************************************************************************************
 * 函 数 名	: bsponc_SeqCmp
 * 功   能	: 比较两个包序号，判断先后关系，序号小的表示在前
 * 输入参数	: seq1: 序号 1
 * 输入参数	: seq2: 序号 2
 * 输出参数	: 无
 * 返 回 值	: = 0：两个序号相同，< 0:序号 1 在前面， > 0:序号 1 在后面
 ***************************************************************************************/
int bsponc_SeqCmp(unsigned int seq1, unsigned int seq2)
{
	int diff = seq1 - seq2;
	if (abs(diff) > ONC_SEQ_CMP_TH)
	{	/* 发生回绕，
		 * 比如：seq1 为 TM_SEQ_NO_MAX - 5, seq2 为 3
		 * seq1 依然应该是在 seq2 前面 ，差值应该取反 */
		diff = -diff;
	}
	return diff;
}

/****************************************************************************************
 * 函 数 名	: bsponc_MsgIdInc
 * 功   能	: 消息编号自增操作
 * 输入参数	: msgId: 消息编号
 * 输出参数	: 无
 * 返 回 值	: 增加后的消息编号
 ***************************************************************************************/
unsigned int bsponc_MsgNO_PostInc(unsigned int *pMsgNO)
{
	unsigned int lRet = *pMsgNO;
	if (ONC_MSGID_MAX == ++*pMsgNO)
	{	/* 回绕 */
		*pMsgNO = 1;
	}

	return lRet;
}


/* 使链表成为真正的环形，没有头结点 */
int bsponc_MakeListLoop(struct list_head *pList)
{
	yv_Assert(NULL != pList);
	if (NULL == pList)
	{
		return -EINVAL;
	}

	pList->prev->next = pList->next;
	pList->next->prev = pList->prev;
	return 0;
}

/* 为环形链表加入头结点 */
int bsponc_UndoListLoop(struct list_head *pList)
{
	yv_Assert(NULL != pList);
	if (NULL == pList)
	{
		return -EINVAL;
	}

	pList->prev->next = pList;
	pList->next->prev = pList;

	return 0;
}
