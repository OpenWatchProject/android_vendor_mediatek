/* 
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FG_LOG_H_
#define _FG_LOG_H_

#include <sys/cdefs.h>
#include <stdarg.h>

#include <utils/Log.h>


__BEGIN_DECLS

void fgdlog_init(void);
int  fgdlog_get_level(void);
void fgdlog_set_level(int level);
void fgdlog_write(int level, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));
void fgdlog_vwrite(int level, const char *fmt, va_list ap);
void fgdlog_exit(void);

__END_DECLS

#define FGLOG_ERROR_LEVEL   3
#define FGLOG_WARNING_LEVEL 4
#define FGLOG_NOTICE_LEVEL  5
#define FGLOG_INFO_LEVEL    6
#define FGLOG_DEBUG_LEVEL   7

#define FGLOG_DEFAULT_LEVEL  3  /* messages <= this level are logged */

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MTK_FG"
#endif


#define FGLOG_ERROR(x...)   	\
	do { \
		if (fgdlog_ctrl == FGLOG_CTRL_TO_KERNEL)	\
			fgdlog_write(FGLOG_ERROR_LEVEL, "<3>" LOG_TAG ": " x); \
		else if (fgdlog_ctrl == FGLOG_CTRL_TO_ANDROID)	\
			ALOGE(x);	\
	} while (0)
#define FGLOG_WARNING(x...)   	\
	do { \
		if (fgdlog_ctrl == FGLOG_CTRL_TO_KERNEL)	\
			fgdlog_write(FGLOG_WARNING_LEVEL, "<4>" LOG_TAG ": " x); \
		else if (fgdlog_ctrl == FGLOG_CTRL_TO_ANDROID)	\
			ALOGW(x);	\
	} while (0)
#define FGLOG_NOTICE(x...)   	\
	do { \
		if (fgdlog_ctrl == FGLOG_CTRL_TO_KERNEL)	\
			fgdlog_write(FGLOG_NOTICE_LEVEL, "<5>" LOG_TAG ": " x); \
		else if (fgdlog_ctrl == FGLOG_CTRL_TO_ANDROID)	\
			ALOGI(x);	\
	} while (0)
#define FGLOG_INFO(x...)   	\
	do { \
		if (fgdlog_ctrl == FGLOG_CTRL_TO_KERNEL)	\
			fgdlog_write(FGLOG_INFO_LEVEL, "<6>" LOG_TAG ": " x); \
		else if (fgdlog_ctrl == FGLOG_CTRL_TO_ANDROID)	\
			ALOGD(x);	\
	} while (0)
#define FGLOG_DEBUG(x...)   	\
	do { \
		if (fgdlog_ctrl == FGLOG_CTRL_TO_KERNEL)	\
			fgdlog_write(FGLOG_DEBUG_LEVEL, "<7>" LOG_TAG ": " x); \
		else if (fgdlog_ctrl == FGLOG_CTRL_TO_ANDROID)	\
			ALOGV(x);	\
	} while (0)

enum FGLOG_CTRL {
	FGLOG_CTRL_NONE,
	FGLOG_CTRL_TO_ANDROID,
	FGLOG_CTRL_TO_KERNEL
};

extern FGLOG_CTRL fgdlog_ctrl;

#endif
