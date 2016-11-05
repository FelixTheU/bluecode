/****************************************************************************************
 * 文 件 名	: yv00bsponc_util.h
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 单向网络通信库
 * 功   能	: 
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
#ifndef YV00BSPONC_UTIL_H
#define YV00BSPONC_UTIL_H

# define __ASSERT_VOID_CAST (void)

#ifndef	DEBUG

# define yv_Assert(expr)		(__ASSERT_VOID_CAST (0))

#else /* DEBUG switch on  */

#ifdef __cplusplus
extern "C" {
#endif
/* This prints an "Assertion failed" message and aborts.  */
extern void __assert_fail (__const char *__assertion, __const char *__file,
			   unsigned int __line, __const char *__function);
#ifdef __cplusplus
}
#endif

# define yv_Assert(expr)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : __assert_fail (__STRING(expr), __FILE__, __LINE__, __func__))

#endif /* DEBUG  */

#define DIMEN_OF(array)		(sizeof((array)) / sizeof((array)[0]))

#define UNUSED_PARAM(prm)	(void)(prm)


#ifdef __cplusplus
extern "C" {
#endif

/* 包序号操作 */

unsigned int bsponc_SeqNoInit();

unsigned int bsponc_MsgNoInit();	
	
unsigned int bsponc_SeqPostInc(unsigned int *pSeq);

unsigned int bsponc_SeqPostDec(unsigned int *pSeq);

int bsponc_SeqCmp(unsigned int seq1, unsigned int seq2);

#define bsponc_SeqBefore(seq1, seq2)		bsponc_SeqCmp(seq1, seq2) < 0

#define bsponc_SeqAfter(seq1, seq2)			bsponc_SeqBefore(seq2, seq1)

/* 消息号操作 */
unsigned int bsponc_MsgNO_PostInc(unsigned int *pMsgNO);


int bsponc_MakeListLoop(struct list_head *pList);

int bsponc_UndoListLoop(struct list_head *pList);	
	
#ifdef __cplusplus
}
#endif

#endif	/* YV00BSPONC_UTIL_H */
