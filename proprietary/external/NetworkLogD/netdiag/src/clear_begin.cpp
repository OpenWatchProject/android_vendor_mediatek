/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cutils/log.h>
#include <cutils/sockets.h>

#include <cutils/properties.h>
#include <sys/system_properties.h>
#include "../utils.h"

extern int get_sdpath_from_configfile(char * sd_path);
extern int check_dir(char *root, const char *target);
extern char mStoragePath[128];

void clear_begin(int fd, char *local) {
    LOGD(" clear_begin: fd = %d, local = %s", fd, local);
    char location[32] = { 0 };
    char sd_path_config[32] = { 0 };
    char rm_cmd[64] = { 0 };
    char buffer[4] = "co"; //clear log is done
    int ret = 0;
    int getsd_result = get_sdpath_from_configfile(sd_path_config);
    if (getsd_result == -1) {
        buffer[1] = 'd';

        LOGD(" clear_begin has some error get_sdpath_from_configfile");
        goto ret_back1;
    } else {
        /*get correct sdpath*/
        if (!strcmp(sd_path_config, "/data")) {
            strncpy(location, sd_path_config, sizeof(location) - 1);
            location[sizeof(location) - 1] = '\0';
        }
    }

    if (strlen(mStoragePath) > 2) {
        strncpy(location, mStoragePath, sizeof(location) - 1);
        location[sizeof(location) - 1] = '\0';
        LOGD("location set as mStoragePath  %s", location);
    } else {
        LOGD("location is  %s", location);
    }

    if (check_dir(location, "debuglogger") == 1) {
        strncat(location, "/debuglogger/netlog",strlen("/debuglogger/netlog"));
        sprintf(rm_cmd, "rm -r %s", location);
        system(rm_cmd);
        LOGD("clear log system_cmd %s", rm_cmd);
    } else
        buffer[1] = 'w';
    ret_back1: 
        if (fd) {
        ret = write(fd, buffer, strlen(buffer));
        LOGD("clear log is done, write to connection %s data", buffer);
    }

    return;

}

