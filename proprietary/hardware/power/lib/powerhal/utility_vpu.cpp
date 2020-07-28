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
#include "utility_vpu.h"

#define VPU_CORE_MAX    2
#define VPU_BOOST_MAX   100
#define MDLA_BOOST_MAX  100

#define PATH_VPU_BOOST_CTRL   "/sys/kernel/debug/vpu/power"
#define PATH_MDLA_BOOST_CTRL  "/sys/kernel/debug/mdla/power"

static int vpu_min_now[VPU_CORE_MAX];
static int vpu_max_now[VPU_CORE_MAX];
static int mdla_min_now;
static int mdla_max_now;


static void setVPUboost(int core, int boost_h, int boost_l)
{
    static int vpu_support = -1;
    struct stat stat_buf;
    char str[256], buf[256];

    if (vpu_support == -1) {
        vpu_support = (0 == stat(PATH_VPU_BOOST_CTRL, &stat_buf)) ? 1 : 0;
    }

    if (vpu_support != 1)
        return;

    ALOGI("set vpu_boost: %d %d %d", core, boost_l, boost_h);

    boost_h = (boost_h == -1) ? VPU_BOOST_MAX : boost_h;
    boost_l = (boost_l == -1) ? 0 : boost_l;

    /* check ceiling must higher than floor*/
    if (boost_h < boost_l) {
        boost_h = boost_l;
        ALOGI("update set vpu_boost: %d %d %d", core, boost_l, boost_h);
    }

    str[0] = '\0';
    sprintf(buf, "power_hal %d %d %d ", core+1, boost_l, boost_h);
    strncat(str, buf, strlen(buf));
    str[strlen(str)-1] = '\0';
    ALOGV("set vpu_boost: %s", str);
    set_value(PATH_VPU_BOOST_CTRL, str);
}

static void setMDLAboost(int boost_h, int boost_l)
{
    static int mdla_support = -1;
    struct stat stat_buf;
    char str[256], buf[256];

    if (mdla_support == -1) {
        mdla_support = (0 == stat(PATH_VPU_BOOST_CTRL, &stat_buf)) ? 1 : 0;
    }

    if (mdla_support != 1)
        return;

    ALOGI("set mdla_boost: %d  %d", boost_l, boost_h);

    boost_h = (boost_h == -1) ? MDLA_BOOST_MAX : boost_h;
    boost_l = (boost_l == -1) ? 0 : boost_l;

      /* check ceiling must higher than floor*/
    if (boost_h < boost_l) {
        boost_h = boost_l;
        ALOGI("update set mdla_boost: %d %d", boost_l, boost_h);
    }
    str[0] = '\0';
    sprintf(buf, "power_hal %d %d ", boost_l, boost_h);
    strncat(str, buf, strlen(buf));
    str[strlen(str)-1] = '\0';
    ALOGV("set mdla_boost: %s", str);
    set_value(PATH_MDLA_BOOST_CTRL, str);
}

int vpu_init(int poweron_init)
{
    int i;
    ALOGV("vpu_init: %d", poweron_init);

    for(i=0; i<VPU_CORE_MAX; i++) {
        vpu_min_now[i] = -1;
        vpu_max_now[i] = -1;
    }
    return 0;
}

int mdla_init(int poweron_init)
{
    ALOGV("mdla_init: %d", poweron_init);
    mdla_min_now = mdla_max_now = -1;
    return 0;
}

int setVpuFreqMin_core_0(int value, void *scn)
{
    ALOGV("setVpuFreqMin_core_0: %p", scn);
    vpu_min_now[0] = value;
    setVPUboost(0, vpu_max_now[0], vpu_min_now[0]);
    return 0;
}

int setVpuFreqMax_core_0(int value, void *scn)
{
    ALOGV("setVpuFreqMax_core_0: %p", scn);
    vpu_max_now[0] = value;
    setVPUboost(0, vpu_max_now[0], vpu_min_now[0]);
    return 0;
}

int setVpuFreqMin_core_1(int value, void *scn)
{
    ALOGV("setVpuFreqMin_core_1: %p", scn);
    vpu_min_now[1] = value;
    setVPUboost(1, vpu_max_now[1], vpu_min_now[1]);
    return 0;
}

int setVpuFreqMax_core_1(int value, void *scn)
{
    ALOGV("setVpuFreqMax_core_1: %p", scn);
    vpu_max_now[1] = value;
    setVPUboost(1, vpu_max_now[1], vpu_min_now[1]);
    return 0;
}

int setMdlaFreqMin(int value, void *scn)
{
    ALOGV("setMdlaFreqMin: %p", scn);
    mdla_min_now = value;
    setMDLAboost(mdla_max_now, mdla_min_now);
    return 0;
}

int setMdlaFreqMax(int value, void *scn)
{
    ALOGV("setMdlaFreqMax: %p", scn);
    mdla_max_now = value;
    setMDLAboost(mdla_max_now, mdla_min_now);
    return 0;
}


