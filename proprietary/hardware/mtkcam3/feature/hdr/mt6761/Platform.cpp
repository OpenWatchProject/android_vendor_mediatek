/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#define DEBUG_LOG_TAG "PLAT"

#include "Platform.h"

#ifdef USE_PERFSERVICE
#include <perfservicenative/PerfServiceNative.h>
#include <linux/mt_sched.h>
#endif
#include <mtkcam3/feature/hdr/2.0/utils/Debug.h>

using namespace NSCam;

// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(Platform);

#ifdef USE_PERFSERVICE

MINT32 Platform::enterPerfMode()
{
    HDR_TRACE_CALL();

    pid_t tid = gettid();
    int sceneHandle = -1;

    // use PerfServeice to enable quad-core CPUs to the highest frequency
    sceneHandle = PerfServiceNative_userRegBigLittle(4, 1950000, 4, 870000);
    if (sceneHandle == -1)
    {
        HDR_LOGE("register PerfService scenario failed");
        return sceneHandle;
    }

    //PerfServiceNative_userEnable(sceneHandle);

    // put current thread to big cores
    cpu_set_t cpuset;
    int cpuMask = 0xF0;
    unsigned int mask, cpu_no;
    CPU_ZERO(&cpuset);
    for (mask = 1, cpu_no = 0; mask < 0xFF; mask <<= 1, cpu_no++)
    {
        if (mask & cpuMask)
        {
            CPU_SET(cpu_no, &cpuset);
        }
    }

    int s;
    s = mt_sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
    {
        HDR_LOGE("mt_sched_setaffinity failed");
    }

lbExit:
    return sceneHandle;
}

MBOOL Platform::exitPerfMode(int sceneHandle)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    if (sceneHandle != -1)
    {
        pid_t tid = gettid();
        int status = mt_sched_exitaffinity(tid);
        if (status != 0)
        {
            HDR_LOGE("mt_sched_exitaffinity() failed: status(%d)", status);
            ret = MFALSE;
        }

        PerfServiceNative_userDisable(sceneHandle);
        PerfServiceNative_userUnreg(sceneHandle);
    }

lbExit:
    return ret;
}

#else

MINT32 Platform::enterPerfMode()
{
    return -1;
}

MBOOL Platform::exitPerfMode(int /*sceneHandle*/)
{
    return MTRUE;
}

#endif // #ifdef USE_PERFSERVICE
