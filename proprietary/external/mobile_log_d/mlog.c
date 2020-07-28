/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <log/log.h>
#include <sys/stat.h>
#include "config.h"
#include "global_var.h"
#include "mlog.h"
#include "libfunc.h"

/*
 * save msg to both 'log file' in /data/misc/mblog/
 * and current log path
 */
#define MB_LOG_FILE     "mblog_history"
#define MAX_FILE_SIZE   (10*1024*1024)

pthread_mutex_t mlog_lock = PTHREAD_MUTEX_INITIALIZER;

void __mb_log(int type, const char *fmt, ...) {
    char buf[255], ts[255], result[255];
    FILE *fp;
    int fd = -1;
    struct tm *tmptr;
    time_t lt;
    int to_sd_fail = 0;
    static int haschmod = 0, sz_overflow = 0;
    static unsigned long long total_sz = 0;

    memset(ts, 0x0, sizeof(ts));
    lt = time(NULL);
    tmptr = localtime(&lt);
    if (tmptr != NULL) {
        strftime(ts, sizeof(ts), "%Y:%m:%d %H:%M:%S ", tmptr);
    } else {
        ts[0] = 'E';
    }
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    snprintf(result, sizeof(result), "%s%s(%d)", ts, buf, getpid());

    pthread_mutex_lock(&mlog_lock);
    if (type == TO_SDCARD || type == TO_BOTH) {
        char full_path[255];

        snprintf(full_path, sizeof(full_path), "%s%s", cur_logging_path, MB_LOG_FILE);
        if (strlen(cur_logging_path) && (fd = mb_open(full_path)) >= 0
            && (fp = fdopen(fd, "a+"))) {
            fputs(result, fp);
            fputs("\n", fp);
            fclose(fp);
        } else if (type == TO_SDCARD) {
            to_sd_fail = 1;
        }
    }

    if (!sz_overflow && (type == TO_DATA || type == TO_BOTH || to_sd_fail)) {
        if (access(CONFIG_DIR, F_OK) != 0) {
            if (!create_dir(CONFIG_DIR)) {
                ALOGE("create dir %s failed(%s)", CONFIG_DIR, strerror(errno));
                goto err;
            }
        }

        if ((fp = fopen(MBLOG_HISTORY, "a+"))) {
            fputs(result, fp);
            fputs("\n", fp);
            fclose(fp);
        }
        total_sz += strlen(result);
        if (total_sz > MAX_FILE_SIZE)
            sz_overflow = 1;

        if (!haschmod) {
            if (chmod(MBLOG_HISTORY, 0664) < 0) {
                ALOGE("chmod %s fail(%s)", MBLOG_HISTORY, strerror(errno));
                goto err;
            }
            haschmod = 1;
        }
    }

err:
    pthread_mutex_unlock(&mlog_lock);

    return;
}

