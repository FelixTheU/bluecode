/****************************************************************************************
 * 文 件 名	: yv00bspown.h
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向网络通信库--接口定义（One-way Network Communication library）
 * 功   能	: 提供用于单向网络通信的基本收发功能的接口
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
#ifndef YV00BSPONC_H
#define YV00BSPONC_H

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif	

typedef struct sOncSend *ONC_SendHandle;
typedef struct sOncRecv *ONC_RecvHandle;

/* 发送接口 */
int bsponc_sendInit(ONC_SendHandle *pHandle, unsigned short sendPort, char *pszPeerIp, unsigned short peerPort);

int bsponc_sendFini(ONC_SendHandle handle);

int bsponc_send(ONC_SendHandle handle, char *pByData, int lDataLen);	

/* 接收接口 */
int bsponc_recvInit(ONC_RecvHandle *pHandle, unsigned short recvPort);	

int bsponc_recvFini(ONC_RecvHandle handle);

/* 获取handle 对应的文件描述符 */
int bsponc_recvGetFd(ONC_RecvHandle handle);
	
int bsponc_recv(ONC_RecvHandle handle, char *pByBuff, int lBuffLen);
	
#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif	/* YV00BSPONC_H */
