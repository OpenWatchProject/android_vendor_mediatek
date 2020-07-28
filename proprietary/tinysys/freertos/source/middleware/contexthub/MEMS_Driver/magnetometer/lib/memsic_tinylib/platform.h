/*****************************************************************************
 * 平台相关代码定义
 * Author: Dong Xiaoguang
 * Created on 2015/12/14
 *****************************************************************************/
#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

//-----------------------------choose platform---------------------------------
//#define MY_WINDOWS
#define MY_ANDROID

//---------------------------enable log or printf------------------------------
#define ALGO_DEBUG                      1
#if ALGO_DEBUG
#define myprintf(format, ...) OutputDebugLog(format, ##__VA_ARGS__)
#else
#define myprintf(format, ...)
#endif // ALGO_DEBUG

#ifdef MY_ANDROID

void OutputDebugLog(const char *format, ...);
#define DLL_API
#endif


//----------------------------------------------------------------------------
//平台定义检测
#if (defined MY_WINDOWS) && (defined MY_ANDROID)
#error Two platforms are defined, choose ONLY one platform in platform.h
#endif
#if (!defined MY_WINDOWS) && (!defined MY_ANDROID)
#error No platform is defined, choose one platform in platform.h
#endif

#endif

//PLATFORM_H_INCLUDED
