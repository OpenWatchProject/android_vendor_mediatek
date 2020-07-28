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

#ifndef _TA_LOG_H_
#define _TA_LOG_H_

#include <sys/cdefs.h>
#include <stdarg.h>
#include <cutils/log.h>
#include <utils/Log.h>


__BEGIN_DECLS

void talog_init(void);
int  talog_get_level(void);
void talog_set_level(int level);
void talog_write(int level, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));
void talog_vwrite(int level, const char *fmt, va_list ap);
void talog_exit(void);

__END_DECLS

#define TA_LOG_LEVEL_PATH "/sys/devices/platform/battery_meter/TA_daemon_log_level"

#define TALOG_ERROR_LEVEL   3
#define TALOG_WARNING_LEVEL 4
#define TALOG_NOTICE_LEVEL  5
#define TALOG_INFO_LEVEL    6
#define TALOG_DEBUG_LEVEL   7

#define TALOG_DEFAULT_LEVEL  6  /* messages <= this level are logged */

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MTK_FG"
#endif

#define TA_LOG  "thermal_src"

#define TALOG_ERROR(_fmt_, args...)   LOG_PRI(ANDROID_LOG_ERROR, TA_LOG, _fmt_, ##args)

#define TALOG_WARNING(_fmt_, args...) LOG_PRI(ANDROID_LOG_WARN, TA_LOG, _fmt_, ##args)

#define TALOG_NOTICE(_fmt_, args...)  LOG_PRI(ANDROID_LOG_INFO, TA_LOG, _fmt_, ##args)

#define TALOG_INFO(_fmt_, args...)    LOG_PRI(ANDROID_LOG_INFO, TA_LOG, _fmt_, ##args)

#define TALOG_DEBUG(_fmt_, args...)   LOG_PRI(ANDROID_LOG_DEBUG, TA_LOG, _fmt_, ##args)

#define TALOG_VERB(_fmt_, args...)   LOG_PRI(ANDROID_LOG_VERBOSE, TA_LOG, _fmt_, ##args)


enum TALOG_CTRL {
	TALOG_CTRL_NONE,
	TALOG_CTRL_TO_ANDROID,
	TALOG_CTRL_TO_KERNEL
};

extern TALOG_CTRL talog_ctrl;

#endif
