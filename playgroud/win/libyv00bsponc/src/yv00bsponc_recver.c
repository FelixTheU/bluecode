
/****************************************************************************************
 * 文 件 名	: yv00bsponc_recver.c
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向通信网络库
 * 功   能	: 单向接收接口实现
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

#define ONC_POLL_INTERVAL	(500)	/* ms */
#define ONC_RECV_BUFF_CNT	(5)		/* 接收缓冲区个数 */
#define ONC_FRAG_QUEUE_LEN	(2048)	/* 每个接收缓冲区中 frag 队列的长度 */
#define ONC_PACKET_INFO_CNT (10)	/* 缓存的消息数量 */

/* 分片结点，用来存放收到的一个分片 */
typedef struct sFragNode
{
  unsigned int lSeqNO;			/* 序号 */
  unsigned int lMsgNO;			/* 消息号 */
  int			lFragLen;		/* 本分片的长度 */
  char byBuff[ONC_MSS + 1];		/* 存放一个数据片 */

  char bUsed;					/* 该结点是否被使用 */
  struct list_head list;		/* 链表挂载点 */
} ST_FragNode, ST_FragQueue;

/* packet 描述信息 */
typedef struct sPacketInfo
{
  unsigned int lMsgNO;				/* 该 packet 的消息编号 */
  unsigned int lSeqNO_start;		/* packet 的起始序号 若为0 表示还未遇到开始 */
  unsigned int lSeqNO_end;			/* packet 的终止序号 若为0 表示还未遇到结尾 */
  int lReassedCnt;					/* 已经重组的数量  */
  struct list_head *plFragStart;	/* 指向接收到的该消息的第一个分片 */
  struct list_head *plFragContiEnd;	/* 指向接收到的一段连续 frag 的结尾 */

  char bUsed;
  struct list_head list;			/* 链表挂载点 */
} ST_PacketInfo;

/* 某 ip 的packet 接收缓冲 */
typedef struct sIpRecvBuffer
{
  in_addr_t fromIp;					/* 该 buffer 存放来自某个 ip 的 packet */

  int lFragQueueLen;					/* 队列长度 */
  struct list_head fragQueue;		/* 分片队列 */
  struct list_head *pFragNextFresh; /* 下一个空闲结点 */

  struct list_head packetInfoList;	/* packet 信息列表 */
  struct list_head *pPacketToCommit; /* 指向待提交的 packet info */
  struct list_head *pPacketInfoNextFresh;
} ST_IpRecvBuffer;

typedef struct sOncRecv
{
	int sockFd;								/* socket 描述符 */
	struct sockaddr_in stSelfSockAddr;		/* 本端地址 */
	struct sockaddr_in stPeerSockAddr;		/* 对端地址 */


	ST_PacketQueue	 sendQueue;				/* 发送队列 */
	pthread_t		 tidRecver;				/* 发送线程id */
	int				 bThreadGoOn;			/* 发送线程是否继续 */

	pthread_mutex_t		mutexQueue;			/* 互斥锁，保护发送队列 */
	pthread_cond_t		condQueue;			/* 条件变量，判断队列是否为空 */
	pthread_mutex_t		mutexCond;			/* 配合条件变量的互斥锁 */

  ST_IpRecvBuffer arrIpRecvBuffers[ONC_RECV_BUFF_CNT];
} ST_OncRecv;

int bsponc_initIpRecvBuffers(ST_OncRecv *pOnc)
{
  int lRet = YV_FAIL;
  int lSts = YV_FAIL;
  int i = 0;
  int j = 0;
  ST_IpRecvBuffer *pRecvBuff = NULL;
  ST_FragNode *pstFragNode = NULL;
  ST_PacketInfo *pstPacketInfo = NULL;
  
  for (i = 0; i < DIMEN_OF(pOnc->arrIpRecvBuffers); i++)
	{
	  pRecvBuff = &pOnc->arrIpRecvBuffers[i];
	  pRecvBuff->fromIp = 0;

	  /* 初始化 frag queuee */
	  INIT_LIST_HEAD(&pRecvBuff->fragQueue);
	  INIT_LIST_HEAD(&pRecvBuff->packetInfoList);
	  pRecvBuff->lFragQueueLen = ONC_FRAG_QUEUE_LEN;
	  for (j = 0; j < ONC_FRAG_QUEUE_LEN; j++)
		{
		  pstFragNode = (ST_FragNode *)malloc(sizeof (ST_FragNode));
		  if (NULL == pstFragNode)
			{
			  goto FREE;
			}
		  pstFragNode->bUsed = FALSE;
		  pstFragNode->lFragLen = 0;
		  pstFragNode->lMsgNO = 0;
		  pstFragNode->lSeqNO = 0;
		  INIT_LIST_HEAD(&pstFragNode->list);

		  list_add_tail(&pRecvBuff->fragQueue, &pstFragNode->list);
		}

	  
	  /* 初始化 packet info list */
	  for (j = 0; j < ONC_PACKET_INFO_CNT; j++)
		{
		  pstPacketInfo = (ST_PacketInfo *)malloc(sizeof (ST_PacketInfo));
		  pstPacketInfo->bUsed = FALSE;
		  pstPacketInfo->lMsgNO = 0;
		  pstPacketInfo->lReassedCnt = 0;
		  pstPacketInfo->lSeqNO_start = 0;
		  pstPacketInfo->lSeqNO_end = 0;
		  pstPacketInfo->plFragStart = NULL;
		  pstPacketInfo->plFragContiEnd = NULL;
		  INIT_LIST_HEAD(&pstPacketInfo->list);

		  list_add(&pRecvBuff->packetInfoList, &pstPacketInfo->list);
		}

	  pRecvBuff->pFragNextFresh = pRecvBuff->fragQueue.next;
	  pRecvBuff->pPacketInfoNextFresh = pRecvBuff->packetInfoList.next;
	  pRecvBuff->pPacketToCommit = NULL;
	  
	  bsponc_MakeListLoop(&pRecvBuff->fragQueue);
	  bsponc_MakeListLoop(&pRecvBuff->packetInfoList);
	}

 FREE:

 OUT:
  
}

int bsponc_validateProto(ST_OncProtocol *pProto)
{
	if (NULL == pProto)
	{
		return FALSE;
	}

	/* 检验前导码 */
	if (strcmp(pProto->magic, ONC_PROTO_MAGIC) != 0)
	{
		return FALSE;
	}

	/* 检验crc */
	return TRUE;
}

int bsponc_processRecvedMsgFromIp(ST_OncRecv *pOnc, struct sockaddr_in *pAddr, ST_OncProtocol *pProtoHdr, char *pByBuff, int lBuffLen)
{
	return 0;
}

int bsponc_processRecvedMsg(ST_OncRecv *pOnc, struct sockaddr_in *pAddr, ST_OncProtocol *pProtoHdr, char *pByBuff, int lBuffLen)
{
	/* 根据 ip 交给相应的 ip 消息去处理 */
	return 0;
}

static inline
void *threadFun_recver(void *pFunData)
{
	int				 lSts	= YV_FAIL;
	ST_OncRecv		*pOnc	= NULL;
	struct pollfd   pfd[1];
	struct sockaddr_in stAddrFrom;
	ST_OncProtocol  stOncProto;
	char byRecvBuff[ONC_MSS + 1] = { 0 };
	
	yv_Assert(pFunData);
	if (NULL == pFunData)
	{
		goto OUT;
	}

	pOnc = (ST_OncRecv *)pFunData;
	pfd[0].fd = pOnc->sockFd;
	pfd[0].events = POLLIN;
	
	while (pOnc->bThreadGoOn)
	{
		lSts = poll(pfd, DIMEN_OF(pfd), ONC_POLL_INTERVAL);
		if (lSts < 0)
		{
			if (EINTR == errno)
			{
				continue;
			}
			else
			{
				DEV_DEBUG_INFO("poll 接收失败");
				goto OUT;
			}
		}
		if (0 == lSts)
		{
			continue;
		}

		/* 有数据到来 */
		if (pfd[0].revents & POLLIN)
		{
			lSts = bsponc_recvMsg(pOnc->sockFd, &stAddrFrom, &stOncProto, byRecvBuff, sizeof (byRecvBuff));
			if (lSts < 0)
			{
				DEV_DEBUG_INFO("bsponc_recvMsg 出错");
				break;
			}

			/* 验证协议头 */
			if (!bsponc_validateProto(&stOncProto))
			{
				continue;
			}

			bsponc_processRecvedMsg(pOnc, &stAddrFrom, &stOncProto, byRecvBuff, lSts);

		}
	}
	/* 使用 poll 接收 */
	/* 1）根据来源ip 放入不同的接收缓冲区中 */
	/* 2）每个缓冲区中有一个接收队列，和一个msg 信息列表 */
	/* 3）根据当前到达的新的一包消息，接收的缓冲区判断是否能够提交新一包消息 */
	/* 4）可以提交便通知，使用 bsponc_recv 可用 */

	
OUT:	
	return NULL;
}



int bsponc_recvInit(ONC_RecvHandle *pHandle, unsigned short recvPort)
{
	int lSts = YV_FAIL;
	int lRet = YV_FAIL;
	ST_OncRecv *pOnc = NULL;
	
	if (NULL == pHandle)
	{
		DEV_DEBUG_INFO("参数无效");
		lRet = -EINVAL;
		goto OUT;
	}

	pOnc = (ST_OncRecv *)malloc(sizeof(ST_OncRecv));
	if (NULL == pOnc)
	{
		DEV_DEBUG_INFO("malloc ST_OncRecv 失败");
		lRet = -ENOSPC;
		goto OUT;
	}

	lSts = socket(PF_INET, SOCK_DGRAM, 0);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("创建套接字失败");
		lRet = lSts;
		goto FREE;
	}

	/* 设置端口 */
	pOnc->stSelfSockAddr.sin_family		 = AF_INET;
	pOnc->stSelfSockAddr.sin_port		 = htons(recvPort);
	pOnc->stSelfSockAddr.sin_addr.s_addr = INADDR_ANY;
	/* 绑定端口 */
	lSts = bind(pOnc->sockFd, (struct sockaddr *)&pOnc->stSelfSockAddr, sizeof (pOnc->stSelfSockAddr));
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("socket bind 失败");
		lRet = lSts;
		goto CLOSE;
	}

	/* 开启接收线程 */
	lSts = pthread_create(&pOnc->tidRecver, NULL, threadFun_recver, pOnc);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("创建接收线程失败");
		lRet = lSts;
		goto CLOSE;
	}

	lRet = YV_SUCCESS;
	*pHandle = pOnc;
	goto OUT;
	
	/* 开启接收线程 */
CLOSE:
	close(pOnc->sockFd);
FREE:
	free(pOnc);
OUT:	
	return lRet;
}

int bsponc_recvFini(ONC_RecvHandle handle)
{
	return 0;
}
	

int bsponc_recvGetFd(ONC_RecvHandle handle)
{
	return 0;
}
	
int bsponc_recv(ONC_RecvHandle handle, char *pByBuff, int lBuffLen)
{
	return 0;
}
