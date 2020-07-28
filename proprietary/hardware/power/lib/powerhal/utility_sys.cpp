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

#define LOG_TAG "libPowerHal"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <errno.h>
#include <sys/stat.h>

#include "common.h"
#include "utility_sys.h"


//using namespace std;

/* variable */
static int cpu_prefer_support = 0;


/* function */
int initTaskPreferCpu(int power_on)
{
    struct stat stat_buf;

    ALOGV("cpu_prefer_init: %d", power_on);
    cpu_prefer_support = (0 == stat(PATH_TASK_PREFER_CPU, &stat_buf)) ? 1 : 0;
    return 0;
}

int setTaskPreferCpu_big(int tid, void *scn)
{
    char str[64];
    if(!cpu_prefer_support)
        return -1;

    ALOGI("setTaskPreferCpu_big:%d", tid);
    snprintf(str, sizeof(str)-1, "%d 1", tid);
    set_value(PATH_TASK_PREFER_CPU, str);
    return 0;
}

int unsetTaskPreferCpu_big(int tid, void *scn)
{
    char str[64];
    if(!cpu_prefer_support)
        return -1;

    ALOGI("unsetTaskPreferCpu_big:%d", tid);
    snprintf(str, sizeof(str)-1, "%d 0", tid);
    set_value(PATH_TASK_PREFER_CPU, str);
    return 0;
}

int setTaskPreferCpu_little(int tid, void *scn)
{
    char str[64];
    if(!cpu_prefer_support)
        return -1;

    ALOGI("setTaskPreferCpu_little:%d", tid);
    snprintf(str, sizeof(str)-1, "%d 2", tid);
    set_value(PATH_TASK_PREFER_CPU, str);
    return 0;
}

int unsetTaskPreferCpu_little(int tid, void *scn)
{
    char str[64];
    if(!cpu_prefer_support)
        return -1;

    ALOGI("unsetTaskPreferCpu_little:%d", tid);
    snprintf(str, sizeof(str)-1, "%d 0", tid);
    set_value(PATH_TASK_PREFER_CPU, str);
    return 0;
}


