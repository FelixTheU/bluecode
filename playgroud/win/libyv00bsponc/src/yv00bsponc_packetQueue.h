/****************************************************************************************
 * 文 件 名	: yv00bsponc_packetQueue.h
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向通信网络库
 * 功   能	: 消息包队列
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

typedef struct sPacket
{
	int lMsgNO;					/* 消息编号 */
	int lSeqNO;					/* 包序号 */
	char *pByData;				/* 包数据 */
	int  lDataLen;				/* 包数据长度 */
	unsigned int lCrcCode;		/* 校验码 */
	

	struct list_head list;		/* 链表挂载点 */
} ST_OncPacket;
#define ONC_PACKET_INITIALIZER(who) { 0, 0, NULL, 0, 0, LIST_HEAD_INIT(who.list)}

typedef struct sPacketQueue
{
	int lQueueLen;

	struct list_head *head;
	struct list_head *tail;
	struct list_head packetQueue;
	
} ST_PacketQueue;

int bsponc_queueEmpty(ST_PacketQueue *pQueue);

int bsponc_queueInit(ST_PacketQueue *pQueue, int len);

int bsponc_queueFini(ST_PacketQueue *pQueue);

int bsponc_enqueue(ST_PacketQueue *pQueue, const ST_OncPacket *pPacket);

int bsponc_dequeue(ST_PacketQueue *pQueue, ST_OncPacket *pPacket);
