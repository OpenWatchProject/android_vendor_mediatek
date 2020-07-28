/* 
 * Copyright (C) 2008 The Android Open Source Project
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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/Log.h>

#include "fg_log.h"


static int fgdlog_fd = -1;
static int fgdlog_level = FGLOG_DEFAULT_LEVEL;
FGLOG_CTRL fgdlog_ctrl = FGLOG_CTRL_TO_KERNEL;

int fgdlog_get_level(void) {
    return fgdlog_level;
}

void fgdlog_set_level(int level) {
    fgdlog_level = level;
}

void fgdlog_init(void)
{
		static const char *name = "/dev/kmsg";
	int ret = 0;

    if (fgdlog_fd >= 0) return; /* Already initialized */

    fgdlog_fd = open(name, O_WRONLY);
    if (fgdlog_fd < 0) {
        /* ALOGE("open fd failed, error = %s\n", strerror(errno)); */
        return;
    }
    ret = fcntl(fgdlog_fd, F_SETFD, FD_CLOEXEC);
	if(ret != 0) {
		//ALOGE("open fgdlog_fd failed\n");
		return;
	}
}

#define LOG_BUF_MAX 512

void fgdlog_vwrite(int level, const char *fmt, va_list ap)
{
    char buf[LOG_BUF_MAX];

    if (level > fgdlog_level) {
    	ALOGI("log level too low, fgdlog_level = %d\n", fgdlog_level);
    	return;
    }
    if (fgdlog_fd < 0) {
    	ALOGW("fd < 0, init first!\n");    	
    	fgdlog_init();
    }
    if (fgdlog_fd < 0) {
    	ALOGE("init failed, return!\n");     	
    	return;
    }

    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;

    write(fgdlog_fd, buf, strlen(buf));
}

void fgdlog_write(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fgdlog_vwrite(level, fmt, ap);
    va_end(ap);
}

void fgdlog_exit(void)
{
		close(fgdlog_fd);
		fgdlog_fd = -1;
}
