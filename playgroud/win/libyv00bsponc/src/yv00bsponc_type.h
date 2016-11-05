/****************************************************************************************
 * 文 件 名	: yv00bsponc_type.h
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向通信网络库
 * 功   能	: 类型定义
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
#define ONC_FRAG_SIZE		(1300)			/* 需要分包的大小 */
#define ONC_MSS				ONC_FRAG_SIZE
#define ONC_PROTO_MAGIC		"YV"			/* 前导码 */

typedef struct sOncSend
{
	int sockFd;								/* socket 描述符 */
	struct sockaddr_in stSelfSockAddr;		/* 本端地址 */
	struct sockaddr_in stPeerSockAddr;		/* 对端地址 */

	unsigned int lNextMsgNO;				/* 下一个可用的消息编号，在获得新消息后增加 */
	unsigned int lNextSeqNO;				/* 下一个可用的包序号, 在发送后增加 */

	ST_PacketQueue	 sendQueue;				/* 发送队列 */
	pthread_t		 tidSender;				/* 发送线程id */
	int				 bThreadGoOn;			/* 发送线程是否继续 */

	pthread_mutex_t		mutexQueue;			/* 互斥锁，保护发送队列 */
	pthread_cond_t		condQueue;			/* 条件变量，判断队列是否为空 */
	pthread_mutex_t		mutexCond;			/* 配合条件变量的互斥锁 */
} ST_OncSend;

/* 单向通信协议 */
typedef struct sOncProtocol
{
	char magic[2];				/* 前导码："YV" */
	unsigned char version :4;	/* 版本：1 */
	unsigned char flag    :4;	/* 标志 */
	char reserverd;				/* 保留 */

	union
	{
		struct
		{
			int msgS :1;				/* 分包开始标记 */
			int msgE :1;				/* 分包结束标记 */
			int msgNO :30;				/* 消息编号，同一包消息的各个分包消息编号相同 */
		};
		int MSG_NO;	
	};
	int seqNO;					/* 包序号 */

	int len;					/* 载荷长度 */
	unsigned int crcCode;		/* crc 校验码 */
	char content[0];
} ST_OncProtocol;

/* 分包标记操作 */
#define ONC_MSGNO_FRAG_START(msgNO)		((msgNO) | 1 << 31)
#define ONC_MSGNO_FRAG_END(msgNO)		((msgNO) | 1 << 30)
#define ONC_MSGNO_FRAG_RESET(msgNO)		((msgNO) & ~(0x3 << 30))

inline static
int bsponc_ProtocolHton(ST_OncProtocol *pOncProto)
{
	yv_Assert(pOncProto);
	if (NULL == pOncProto)
	{
		return -EINVAL;
	}
	
	pOncProto->MSG_NO = htonl(pOncProto->MSG_NO);
	pOncProto->seqNO = htonl(pOncProto->MSG_NO);
	pOncProto->len = htonl(pOncProto->len);
	pOncProto->crcCode = htonl(pOncProto->crcCode);

	return 0;
}

inline static
int bsponc_ProtocolNtoh(ST_OncProtocol *pOncProto)
{
	yv_Assert(pOncProto);
	if (NULL == pOncProto)
	{
		return -EINVAL;
	}

	pOncProto->MSG_NO = ntohl(pOncProto->MSG_NO);
	pOncProto->seqNO = ntohl(pOncProto->MSG_NO);
	pOncProto->len = ntohl(pOncProto->len);
	pOncProto->crcCode = ntohl(pOncProto->crcCode);

	return 0;
}
