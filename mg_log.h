/* 版权声明
 * Copyright(C) 2012,国路安信息技术有限公司
 * 版权所有.
 */

#ifndef __MG_LOG_H__
#define __MG_LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>


#define LOG_MAX_LEN		1024



#define MG_LOG_DEBUG	LOG_DEBUG
#define MG_LOG_INFO		LOG_INFO
#define MG_LOG_NOTICE   LOG_NOTICE
#define MG_LOG_WARNING  LOG_WARNING
#define MG_LOG_ERROR	LOG_ERR
#define MG_LOG_CRIT     LOG_CRIT
#define MG_LOG_ALERT    LOG_ALERT
#define MG_LOG_EMERG    LOG_EMERG


void mg_log(int std_out,int flag, const char* fmt, ...);

void mg_log_def_set_gate(int argc, char **argv);

	//#define MG_LOG_WITH_STD_OUT
	//#define MG_LOG_WITH_DEBUG


	#ifdef MG_LOG_WITH_STD_OUT

		#define mg_log_ex(flag,fmt,...)  mg_log( (1),flag, fmt" [file: %s; function: %s; line:%d]", ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__)

	#else

		#define mg_log_ex(flag,fmt,...)  mg_log( (0),flag, fmt" [function: %s]", ##__VA_ARGS__, __FUNCTION__)

	#endif


	#ifdef MG_LOG_WITH_DEBUG
		#define mg_log_ex_debug(fmt,...) mg_log_ex(MG_LOG_DEBUG,fmt,##__VA_ARGS__)
	#else
		#define mg_log_ex_debug(fmt,...) /*do nothing*/
	#endif

#endif
