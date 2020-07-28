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

#include <mtkcam3/feature/hdr/Platform.h>

#ifdef USE_PERFSERVICE
#include <perfservicenative/PerfServiceNative.h>
#include <cutils/properties.h>
#endif

#ifdef USE_AFFINITY
#include <mtkcam3/feature/hdr/utils/Debug.h>
#include <linux/mt_sched.h>
#endif

#ifdef EARLY_PORTING
#undef USE_PERFSERVICE
#undef USE_AFFINITY
#endif

using namespace NSCam;

// ---------------------------------------------------------------------------

class PerfImpl final : public IPerf
{
public:
    PerfImpl();
    ~PerfImpl();

    int enableAffinity(pid_t pid) override;
    int disableAffinity(pid_t pid) override;

private:
    int mSceneHandle;
};

#ifdef USE_PERFSERVICE

PerfImpl::PerfImpl()
    : mSceneHandle(-1)
{
    HDR_TRACE_CALL();

    mSceneHandle = PerfServiceNative_userRegScn();
    if (mSceneHandle == -1)
    {
        HDR_LOGE("register PerfService scenario failed");
        return;
    }

    // configure CPU DVFS via adb
    int32_t minCores = 0;
    int32_t maxCores = 0;
    int32_t minFreq  = 0;
    int32_t maxFreq  = 0;

    // config cluster 0 (CA53 x4 up to 1.638G)
    {
        minCores = property_get_int32("mediatek.hdr.debug.mincore0", 4);
        maxCores = property_get_int32("mediatek.hdr.debug.maxcore0", 4);
        minFreq  = property_get_int32("mediatek.hdr.debug.minfreq0", 1638000);
        maxFreq  = property_get_int32("mediatek.hdr.debug.maxfreq0", 1638000);

        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MIN, 0, minCores, 0, 0);
        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MAX, 0, maxCores, 0, 0);
        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, minFreq, 0, 0);
        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MAX, 0, maxFreq, 0, 0);
    }

    // config cluster 1 (CA53 x4 up to 2.34G)
    {
        minCores = property_get_int32("mediatek.hdr.debug.mincore1", 4);
        maxCores = property_get_int32("mediatek.hdr.debug.maxcore1", 4);
        minFreq  = property_get_int32("mediatek.hdr.debug.minfreq1", 2340000);
        maxFreq  = property_get_int32("mediatek.hdr.debug.maxfreq1", 2340000);

        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MIN, 1, minCores, 0, 0);
        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MAX, 1, maxCores, 0, 0);
        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, minFreq, 0, 0);
        PerfServiceNative_userRegScnConfig(
                mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MAX, 1, maxFreq, 0, 0);
    }

    //PerfServiceNative_userEnable(mSceneHandle);
}

PerfImpl::~PerfImpl()
{
    HDR_TRACE_CALL();

    if (mSceneHandle != -1)
    {
        PerfServiceNative_userDisable(mSceneHandle);
        PerfServiceNative_userUnreg(mSceneHandle);
    }
}

#else

PerfImpl::PerfImpl()
    : mSceneHandle(-1)
{
}

PerfImpl::~PerfImpl()
{
}

#endif // #ifdef USE_PERFSERVICE

int PerfImpl::enableAffinity(pid_t pid)
{
    int status = 0;

#ifdef USE_AFFINITY
    // cluster 0 + 1
    int cpuMask = 0xFF;
    // put current thread to bigger cores
    cpu_set_t cpuset;
    unsigned int mask, cpu_no;
    CPU_ZERO(&cpuset);
    for (mask = 1, cpu_no = 0; mask < 0xFF; mask <<= 1, cpu_no++)
    {
        if (mask & cpuMask)
        {
            CPU_SET(cpu_no, &cpuset);
        }
    }
    status = mt_sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
    HDR_LOGE_IF(status, "mt_sched_setaffinity() failed: status(%d)", status);
#endif

    return status;
}

int PerfImpl::disableAffinity(pid_t pid)
{
    int status = 0;

#ifdef USE_AFFINITY
    status = mt_sched_exitaffinity(pid);
    HDR_LOGE_IF(status, "mt_sched_exitaffinity() failed: status(%d)", status);
#endif

    return status;
}

// ---------------------------------------------------------------------------

IPerf* IPlatform::getPerf()
{
    return new PerfImpl;
}
