/****************************************************************************************
 * 文 件 名	: yv00bsponc_netIO.c
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向网络通信库
 * 功   能	: 网络IO(分包，组包)
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
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include <yvcommon_def.h>
#include <list.h>
#include "yv00bsponc_util.h"
#include "yv00bsponc_debug.h"
#include "yv00bsponc.h"
#include "yv00bsponc_packetQueue.h"
#include "yv00bsponc_type.h"



static
int bsponc_sendMsg(int fd, struct sockaddr_in *pAddr, ST_OncProtocol *pProtoHdr, char *pByData2, int len2)
{

	int lSts = YV_FAIL;
	int lRet = YV_FAIL;
	struct msghdr	mh;
	struct iovec iov[2];
	
	yv_Assert(pByData1);
	yv_Assert(pByData2);
	if (NULL == pProtoHdr
		|| NULL == pByData2)
	{
		return -EINVAL;
	}

	/* 安排向量 */
	iov[0].iov_base	= pProtoHdr;
	iov[0].iov_len	= sizeof (ST_OncProtocol);
	iov[1].iov_base	= pByData2;
	iov[1].iov_len	= len2;

	/* 填充 msgHdr */
	mh.msg_name		  = (struct sockaddr *) (pAddr);
	mh.msg_namelen	  = sizeof (struct sockaddr_in);
	mh.msg_iov		  = (struct iovec *)iov;
	mh.msg_iovlen	  = DIMEN_OF(iov);
	mh.msg_control	  = NULL;
	mh.msg_controllen = 0;
	mh.msg_flags	  = 0;

	/* 端序转换 */
	bsponc_ProtocolHton(pProtoHdr);
	
	/* 发送 */
	lSts = sendmsg(fd, &mh, 0);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("sendmsg 发送失败");
		lRet = lSts;
		goto OUT;
	}
	
	lRet = lSts;
	
OUT:	
	return lRet;
}


int bsponc_recvMsg(int fd, struct sockaddr_in *pAddr, ST_OncProtocol *pProtoHdr, char *pByBuff, int lBuffLen)
{
	int lSts = YV_FAIL;
	int lRet = YV_FAIL;
	struct msghdr	mh;
	struct iovec iov[2];
	
	yv_Assert(pByData1);
	yv_Assert(pByData2);
	if (NULL == pProtoHdr
		|| NULL == pByBuff
		|| lBuffLen < ONC_FRAG_SIZE)
	{
		return -EINVAL;
	}

	/* 安排向量 */
	iov[0].iov_base	= pProtoHdr;
	iov[0].iov_len	= sizeof (ST_OncProtocol);
	iov[1].iov_base	= pByBuff;
	iov[1].iov_len	= lBuffLen;

	/* 填充 msgHdr */
	mh.msg_name		  = (struct sockaddr *) (pAddr);
	mh.msg_namelen	  = sizeof (struct sockaddr_in);
	mh.msg_iov		  = (struct iovec *)iov;
	mh.msg_iovlen	  = DIMEN_OF(iov);
	mh.msg_control	  = NULL;
	mh.msg_controllen = 0;
	mh.msg_flags	  = 0;

	/* 接收 */
	lSts = recvmsg(fd, &mh, 0);
	if (lSts < 0)
	{
		DEV_DEBUG_INFO("recvmsg 接收失败");
		lRet = lSts;
		goto OUT;
	}
	lRet = lSts;
	
	/* 端序转换 */
	bsponc_ProtocolNtoh(pProtoHdr);


	goto OUT;
	
OUT:	
	return lRet;

}

int bsponc_sendPacket(ST_OncSend *pOnc, ST_OncPacket *pstPacket)
{
	int i		   = 0;
	int lSts	   = YV_FAIL;
	int lRet	   = YV_FAIL;
	int lFragCnt   = 0;			/* 分片数量 */
	int lRemainder = 0;			/* 不足一个 ONC_FRAG_SIZE 的量 */

	/*  */
	static ST_OncProtocol s_stProtoHdr;
	
	yv_Assert(pOnc);
	yv_Assert(pstPacket);
	if (NULL == pOnc
		|| NULL == pstPacket
		|| pstPacket->lDataLen <= 0)
	{
		DEV_DEBUG_INFO("函数参数无效");
		lRet = -EINVAL;
		goto OUT;
	}

	lFragCnt   = pstPacket->lDataLen / ONC_FRAG_SIZE;
	lRemainder = pstPacket->lDataLen % ONC_FRAG_SIZE;

	s_stProtoHdr.MSG_NO = pstPacket->lMsgNO;	/* 一次发送中 msgNO 都相同 */

	/* 先发送余量 */
	s_stProtoHdr.MSG_NO =  ONC_MSGNO_FRAG_RESET(s_stProtoHdr.MSG_NO);	/* 分片标记归零 */
	if (lRemainder > 0)
	{	
		s_stProtoHdr.MSG_NO = ONC_MSGNO_FRAG_START(s_stProtoHdr.MSG_NO);
		if (lFragCnt == 0)
		{	/* 单包消息 */
			s_stProtoHdr.MSG_NO = ONC_MSGNO_FRAG_END(s_stProtoHdr.MSG_NO);
		}
		
		s_stProtoHdr.len   = lRemainder;
		s_stProtoHdr.seqNO = bsponc_SeqPostInc(&pOnc->lNextSeqNO);					/* 自增包序号 */
		lSts = bsponc_sendMsg(pOnc->sockFd, &pOnc->stPeerSockAddr, &s_stProtoHdr,
					   pstPacket->pByData, pstPacket->lDataLen);
		if (lSts < 0)
		{
			DEV_DEBUG_INFO("bsponc_sendMsg 失败");
			lRet = lSts;
			goto OUT;
		}
	}

	/* 发送整齐的多片 */
	s_stProtoHdr.MSG_NO = ONC_MSGNO_FRAG_RESET(s_stProtoHdr.MSG_NO);	/* 分片标记归零 */
	for (i = 0; i < lFragCnt; i++)
	{
		if (i == lFragCnt - 1)
		{	/* 最后一片 */
			s_stProtoHdr.MSG_NO = ONC_MSGNO_FRAG_END(s_stProtoHdr.MSG_NO);
		}

		s_stProtoHdr.len   = ONC_FRAG_SIZE;
		s_stProtoHdr.seqNO = bsponc_SeqPostInc(&pOnc->lNextSeqNO);					/* 自增包序号 */
		lSts = bsponc_sendMsg(pOnc->sockFd, &pOnc->stPeerSockAddr, &s_stProtoHdr,
					   pstPacket->pByData, pstPacket->lDataLen);
		if (lSts < 0)
		{
			DEV_DEBUG_INFO("bsponc_sendMsg 失败");
			lRet = lSts;
			goto OUT;
		}

		/* TODO: 需要考虑平滑 */
	}

OUT:	
	return lRet;
}


