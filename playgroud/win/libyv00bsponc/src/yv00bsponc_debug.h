/****************************************************************************************
 * 文 件 名	: debug.h
 * 项目名称	: YVGA1207001A
 * 模 块 名	: 设备通信模块
 * 功   能	: 调试相关辅助工具
 * 操作系统	: LINUX
 * 修改记录	:
 *--------------------------------------------------------------------------------------
 * 版   本	: 1.1.0
 * 设   计	: zhengsw  '2016-8-25
 * 编   码	: zhengsw  '2016-8-25
 * 修   改	:
 ****************************************************************************************
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 公司介绍及版权说明
 *
 *               (C)Copyright 2016 YView    Corporation All Rights Reserved.
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 ***************************************************************************************/
#ifndef DEBUG_H_
#define DEBUG_H_

/* 调试 */
#define DEBUG

#ifdef DEBUG
#	include <stdio.h>

static inline
void positionInfo(const char *file, int line, const char *func)
{
	printf("%s:%d:%s", file, line, func);
}
	/* 开发阶段调试输出宏，与日志中调试级别区分 */
#	define DEV_DEBUG_INFO(...)	positionInfo(__FILE__, __LINE__, __func__);\
								printf(__VA_ARGS__);\
								printf("\n")

#else
#	define DEV_DEBUG_INFO(...)	void
#endif

/* 终端彩色输出颜色值定义 */
#define TC_NONE                 "\e[0m"
#define TC_BLACK                "\e[0;30m"
#define TC_L_BLACK              "\e[1;30m"
#define TC_RED                  "\e[0;31m"
#define TC_L_RED                "\e[1;31m"
#define TC_GREEN                "\e[0;32m"
#define TC_L_GREEN              "\e[1;32m"
#define TC_BROWN                "\e[0;33m"
#define TC_YELLOW               "\e[1;33m"
#define TC_BLUE                 "\e[0;34m"
#define TC_L_BLUE               "\e[1;34m"
#define TC_PURPLE               "\e[0;35m"
#define TC_L_PURPLE             "\e[1;35m"
#define TC_CYAN                 "\e[0;36m"
#define TC_L_CYAN               "\e[1;36m"
#define TC_GRAY                 "\e[0;37m"
#define TC_WHITE                "\e[1;37m"

#define TC_BOLD                 "\e[1m"
#define TC_UNDERLINE            "\e[4m"
#define TC_BLINK                "\e[5m"
#define TC_REVERSE              "\e[7m"
#define TC_HIDE                 "\e[8m"
#define TC_CLEAR                "\e[2J"
#define TC_CLRLINE              "\r\e[K"

/* 终端彩色输出，color 值来自 term_color.h */
#define DEV_COLOR_INFO(color, ...) 		DEV_DEBUG_INFO(color); 			\
										DEV_DEBUG_INFO(__VA_ARGS__); 	\
										DEV_DEBUG_INFO(TC_NONE)
/* 默认使用红色进行重要信息高亮 */
#define DEV_COLOR_INFO_DEF(...)			DEV_COLOR_INFO(TC_RED, __VA_ARGS__)

#endif /* DEBUG_H_ */
