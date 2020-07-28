/*****************************************************************************
 * 平台相关代码定义
 * Author: Dong Xiaoguang
 * Created on 2015/12/14
 *****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

#define PROMPT "memsic9: "

#ifdef MY_ANDROID
void OutputDebugLog(const char *format,...)
{
#if 0
	static int newLine = 1;
	static char strBuffer[128]={0};
	int nFormat = strlen(format);
	if(newLine)
	{
		strcat(strBuffer,PROMPT);
	}
	va_list vlArgs;
	va_start(vlArgs,format);
	int nBuffer = strlen(strBuffer);
	vsnprintf(strBuffer+nBuffer,sizeof(strBuffer)-1-nBuffer,format,vlArgs);
	va_end(vlArgs);
	newLine = format[nFormat-1]=='\n';
	if(newLine)
	{
		strBuffer[0] = 0;
	}
#endif
}

#endif
