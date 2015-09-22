/* 版权声明
 * Copyright(C) 2012,国路安信息技术有限公司
 * 版权所有.
 */

#include "mg_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define PRINTF 1

int mg_log_gate = MG_LOG_WARNING;

void mg_log(int std_out, int flag, const char* fmt, ...)
{
	if(flag > mg_log_gate) return;

    char log[LOG_MAX_LEN+1] = {0x00};
	
    
	va_list  args;
    va_start(args, fmt);
    vsnprintf(log, LOG_MAX_LEN, fmt, args);
    va_end(args);
    log[LOG_MAX_LEN] = '\0';
 
    syslog(LOG_LOCAL0|flag, "%s", log);

	if (std_out)
    	printf("%s\n", log);

}

void mg_log_def_set_gate(int argc, char **argv)
{
	int i, rc;
	for(i = 1; i < argc; i ++)
	{
		if(0 == strncmp("-LOG_DEBUG", argv[i], strlen("-LOG_DEBUG")))
		{
			mg_log_gate = MG_LOG_DEBUG;
			return;
		}
		
		if(0 == strncmp("-LOG_INFO", argv[i], strlen("-LOG_INFO")))
        {
            mg_log_gate = MG_LOG_INFO;
            return;
        }
    
    	if(0 == strncmp("-LOG_NOTICE", argv[i], strlen("-LOG_NOTICE")))
        {
            mg_log_gate = MG_LOG_NOTICE;
            return;
        }
    
    	if(0 == strncmp("-LOG_WARNING", argv[i], strlen("-LOG_WARNING")))
        {
            mg_log_gate = MG_LOG_WARNING;
            return;
        }
    	
		if(0 == strncmp("-LOG_ERR", argv[i],strlen("-LOG_ERR")))
        {
            mg_log_gate = MG_LOG_ERROR;
            return;
        }
    
    	if(0 == strncmp("-LOG_CRIT", argv[i],strlen("-LOG_CRIT")))
        {
            mg_log_gate = MG_LOG_CRIT;
            return;
        }
    
		if(0 == strncmp("-LOG_ALERT", argv[i], strlen("-LOG_ALERT")))
        {
            mg_log_gate = MG_LOG_ALERT;
            return;
        }
    
    	if(0 == strncmp("-LOG_EMERG", argv[i],strlen("-LOG_EMERG")))
        {
            mg_log_gate = MG_LOG_EMERG;
            return;
        }
	}
}
