/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __MTEE_LOG_H__
#define __MTEE_LOG_H__

//#include "printf.h"

/* should match constant in ta_logctrl.h */
typedef enum {
    MTEE_LOG_LVL_INFO    = 0x00000000,
    MTEE_LOG_LVL_DEBUG   = 0x00000001,
    MTEE_LOG_LVL_PRINTF  = 0x00000002,
    MTEE_LOG_LVL_WARN    = 0x00000003,
    MTEE_LOG_LVL_BUG     = 0x00000004,
    MTEE_LOG_LVL_ASSERT  = 0x00000005,
    MTEE_LOG_LVL_DISABLE = 0x0000000f,
} MTEE_LOG_LVL;

/* mark this configure to disable all log in build time */
#define MTEE_ENABLE_LOG_IN_BUILD_TIME

/*
    MTEE_LOG_BUILD_LEVEL is the build time log level
    note:
        USER_BUILD is defined in alps\mediatek\trustzone\mtk\Setting.mk
*/
#ifdef USER_BUILD
/* user build */
#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_BUG
#else
/* eng debug build */
//#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_DISABLE			//for performance test
//#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_PRINTF			//for performance test
#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_DEBUG			//orig is this

//#define MTEE_LOG_BUILD_LEVEL   MTEE_LOG_LVL_INFO
#endif

//int _MTEE_LOG(int level, const char *fmt, ...) __PRINTFLIKE(2, 3);

//#define MTEE_LOG(lvl, args...) do { if ((lvl) >= MTEE_LOG_BUILD_LEVEL) { _MTEE_LOG(lvl, args); } } while (0)
#define MTEE_LOG(lvl, args...) do { if ((lvl) >= MTEE_LOG_BUILD_LEVEL) { printf(args); } } while (0)

#define DBG_LOG(fmt, ...) \
	MTEE_LOG(MTEE_LOG_LVL_DEBUG, "[%s] %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#define PRINTF_LOG(fmt, ...) \
			MTEE_LOG(MTEE_LOG_LVL_PRINTF, "[%s] %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#define WARN_LOG(fmt, ...) \
	MTEE_LOG(MTEE_LOG_LVL_WARN, "[%s] warning %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#define ERR_LOG(fmt, ...) \
	MTEE_LOG(MTEE_LOG_LVL_BUG, "[%s] error %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#define print_mtee_orig_msg MTEE_LOG(MTEE_LOG_LVL_WARN, "This method [%s] is not supported.\n", __func__)

extern void MTEE_EnableLogUART(int enable);
extern void MTEE_EnableLogREE(void);

/*
    change run-time log output level
    this is the run-time log level
*/
extern void MTEE_SetLogLvl(MTEE_LOG_LVL lvl);

#endif /* __MTEE_LOG_H__ */

