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
#include <libgen.h>
#include <log/log.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <private/android_filesystem_config.h>

#include "config.h"
#include "global_var.h"
#include "mlog.h"
#include "libfunc.h"

int create_dir(const char* whole_path) {
    char temp[255], dname[255];
    mode_t mode = 0775;

    ALOGI("create dir %s", whole_path);
    strncpy(temp, whole_path, sizeof(temp)-1);
    strncpy(dname, dirname(temp), sizeof(dname)-1);

    if (access(dname, F_OK) != 0) {
        create_dir(dname);
    }

    if (0 == strncmp(whole_path, "/data/", strlen("/data/"))) {
        mode = 0770;
    }
    if (TEMP_FAILURE_RETRY(mkdir(whole_path, mode)) == -1) {
        ALOGE("mkdir %s fail(%s)", whole_path, strerror(errno));
        if (errno == EEXIST)
            return 1;
        else
            return 0;
    }

    if (0770 == mode) {
        if (chown(whole_path, AID_SHELL, AID_LOG) < 0) {
            ALOGE("chown fail for %s", whole_path);
        }
    }
    return 1;
}

int mb_open(const char* pathname) {
    int f_flags = 0;
    mode_t f_mode = 0;
    int ret;

    f_flags = O_RDWR | O_CREAT | O_APPEND;  // file status flags
    if (0 == strncmp(pathname, "/data/", strlen("/data/"))) {
        f_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;  // creation mode
    } else {
        f_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;  // creation mode
    }
    ret = TEMP_FAILURE_RETRY(open(pathname, f_flags, f_mode));
    if (f_mode == (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) {
        if (chown(pathname, AID_SHELL, AID_LOG) < 0) {
            ALOGE("chown fail for %s", pathname);
        }
    }
    return ret;
}

int notify_client(int clientfd, char *msg) {
    ALOGD("write %s to client %d ", msg, clientfd);
    write(clientfd, msg, strlen(msg));
    ALOGD("write to client over");

    return 1;
}

int is_mblog_running() {
    char is_running[PROPERTY_VALUE_MAX];
    property_get(PROP_RUNNING, is_running, "0");
    return atoi(is_running);
}

char *set_cur_logpath(char *logpath) {
    if (cur_logging_path[0] != 0x00)
        snprintf(last_logging_path, sizeof(last_logging_path), "%s", cur_logging_path);

    snprintf(cur_logging_path, sizeof(cur_logging_path), "%s", logpath);
    property_set(PROP_PATH, logpath);

    MLOGI_DATA("set cur path: %s", logpath);
    return cur_logging_path;
}

void copy_file(char *srcfile, char *desfile) {
    unsigned long ret, total_size;
    int need_exit = 0;
    FILE *desfp, *srcfp;
    char cpbuffer[8192];

    if (access(srcfile, F_OK) != 0)
        return;

    if ((desfp = fopen(desfile, "a+")) == NULL) {
        MLOGE_BOTH("open %s error when copy(%s)", desfile, strerror(errno));
        return;
    }

    total_size = 0;
    if ((srcfp = fopen(srcfile, "rb")) != NULL) {
        while (!feof(srcfp)) {
            if (g_mblog_status == STOPPED) {
                MLOGE_BOTH("status is topped");
                need_exit = 1;
                break;
            }

            memset(cpbuffer, 0x0, sizeof(cpbuffer));
            ret = fread(cpbuffer, sizeof(char), sizeof(cpbuffer), srcfp);
            if (ret != fwrite(cpbuffer, sizeof(char), ret, desfp)) {
                MLOGE_BOTH("fwrite to %s error! %s(%d)", srcfile, strerror(errno), errno);
                continue;
            }
        }

        fclose(srcfp);
        if (remove(srcfile) < 0)
            MLOGE_BOTH("remove %s fail(%s)", srcfile, strerror(errno));
    }

    fclose(desfp);
    if (need_exit)
        pthread_exit(0);
    ret = usleep(100000);
    if (ret < 0) {
        MLOGE_BOTH("usleep error %s", strerror(errno));
    }
}
