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

#define BUFF_SIZE 128

int shell(char * cmd) {
    LOGD(" shell cmd = %s", cmd);
    return 1;

// phase out this feature
/*
    FILE * fp, *fp_exit_code;
    int bufflen;
    char * buffer = (char *) malloc((BUFF_SIZE));
    if (buffer == NULL) {
        LOGD("run shell command buffer is null");
        return 1;
    }
    char * buffer_retcode = NULL;
    char * cmd_exit_code = (char *) malloc((BUFF_SIZE));
    int ret_code = 1;
    if (cmd_exit_code == NULL) {
        LOGD("alloc cmd_exit_code failed ");
        goto ret_sec;
    }
    if (cmd == NULL) {
        LOGD("run shell command is null");
        goto ret_fir;
    }
    buffer[0] = 0;
    strncpy(cmd_exit_code, cmd);
    strncat(cmd_exit_code, ";echo ret_code:$?");
    fp = popen(cmd_exit_code, "r");
    if (fp == NULL) {
        LOGD("can't run shell command");
        goto ret_fir;
    }
    LOGD("run shell command successfully");
    while (fgets(buffer, BUFF_SIZE, fp) != NULL) {
        LOGD("%s", buffer);
    }
    buffer_retcode = strstr(buffer, "ret_code:");
    if (buffer_retcode) {
        ret_code = atoi(buffer_retcode + strlen("ret_code:"));
        LOGD("no processing%d,%s", ret_code,
                buffer_retcode + strlen("ret_code:"));
    }
    pclose(fp);
    LOGD("run shell command fp:%s", fp);

    ret_fir: if (cmd_exit_code)
        free(cmd_exit_code);
    ret_sec: if (buffer)
        free(buffer);
    return ret_code;*/
}
