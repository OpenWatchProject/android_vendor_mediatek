/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "VpuStream"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <thread>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h> //offsetof
#include <mutex>

#include "vpuCommon.h"
#include "earaUtil.h"
#include "vpu.h"
#include <vpu_drv.h>

//#define VPU_EARA_TEST
#define PROPERTY_DEBUG_EARATIME "debug.apu.earatime"
#define PROPERTY_DEBUG_EARAPRIORITY "debug.apu.earapriority"
#define PROPERTY_DEBUG_EARABOOSTVALUE "debug.apu.userboostvalue"

static int gEaraDisable = 0;
#define PROPERTY_DEBUG_EARADISABLE "debug.apu.earadisable"

#ifdef __ANDROID__
#include <cutils/properties.h>  // For property_get().
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGV_IF(_x, ...)  do { if (_x) ALOGV(__VA_ARGS__); } while (0)
#define ALOGD_IF(_x, ...)  do { if (_x) ALOGD(__VA_ARGS__); } while (0)
#define EARACOM_VER(fmt, arg...) ALOGV_IF(getVpuLogLevel(), LOG_TAG "[eara] (%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define EARACOM_DEBUG(fmt, arg...) ALOGD_IF(getVpuLogLevel(), LOG_TAG "[eara] (%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define EARACOM_INFO(fmt, arg...) ALOGI(LOG_TAG "[eara] (%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define EARACOM_WARN(fmt, arg...) ALOGW(LOG_TAG "[eara] (%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#define EARACOM_ERR(fmt, arg...) ALOGE(LOG_TAG "[eara] (%d)[%s] " fmt "\n", gettid(), __FUNCTION__, ##arg)
#else
#define EARACOM_VER printf
#define EARACOM_DEBUG printf
#define EARACOM_INFO printf
#define EARACOM_WARN printf
#define EARACOM_ERR printf
#endif

typedef int (*fnEaraNotifyJobBegin)(int tid, int mid, uint64_t *suggestExecTime);
typedef int (*fnEearaNotifyJobEnd)(int tid, uint64_t mid, int32_t *jobPriority, int errorStatus, uint64_t execTimeNanoSecs, int32_t bandWidth, std::vector<std::vector<int32_t>> boost);

#define MTK_EARA_LIB "libperfctl_vendor.so"

struct earaComm{
    void * libHandle;
    void * startFn;
    void * endFn;

    uint32_t refCnt;
    std::mutex refMtx;
};

struct earaComm gHandle;

static inline struct earaComm * getEaraComm(void)
{
    return &gHandle;
}

int earaNotifyJobBegin(int jobId, uint64_t &suggestExecTime)
{
#if 0
    uint64_t targetTime = 0;
    struct earaComm * handle = getEaraComm();
    fnEaraNotifyJobBegin fnJobBegin = nullptr;

    EARACOM_DEBUG("earaNotifyCvJobBegin: jobid = %d",jobId);

    std::lock_guard<std::mutex> gLock(handle->refMtx);

    if(handle->startFn == nullptr)
    {
        EARACOM_WARN("Warn: function earaNotifyJobBegin is not init");
        return -1;
    }

    fnJobBegin = reinterpret_cast<fnEaraNotifyJobBegin>(handle->startFn);

    // call EARA's API
    EARACOM_VER("Send job begin to eara");
    if(fnJobBegin(gettid(), jobId, &targetTime))
    {
        EARACOM_WARN("Warn: fnJobBegin fail");
        return -1;
    }
    suggestExecTime = targetTime;
#endif
    return 0;
}

int earaNotifyJobEnd(int jobId, uint64_t executeTime, uint32_t bandwidth, int32_t errorStatus, int32_t &jobPriority, int32_t boostVal)
{
#if 0
    int32_t priority = 0;
    struct earaComm * handle = getEaraComm();
    std::vector<std::vector<int32_t>> boost;
    std::vector<int32_t> boostInner;
    fnEearaNotifyJobEnd fnJobEnd = nullptr;

    EARACOM_DEBUG("earaNotifyCvJobEnd: jobid = %d, boost = %d", jobId, boostVal);

    std::lock_guard<std::mutex> gLock(handle->refMtx);

    if(handle->startFn == nullptr)
    {
        EARACOM_WARN("Warn: function earaNotifyJobBegin is not init");
        return -1;
    }

    fnJobEnd = reinterpret_cast<fnEearaNotifyJobEnd>(handle->endFn);

    boostInner.push_back(boostVal);
    boost.push_back(boostInner);

    // call EARA's API
    EARACOM_VER("Send job end to eara");
    if(fnJobEnd(gettid(), jobId, &priority, errorStatus, executeTime, bandwidth, boost))
    {
        EARACOM_WARN("Warn: fnJobEnd fail");
        return -1;
    }
    jobPriority = priority;
#endif
    return 0;
}

int getJobIdFromPointer(void *addr)
{
#if 0
    int jobId = static_cast<int>(reinterpret_cast<uintptr_t>(addr));
    return jobId;
#else
    return 0;
#endif
}

int startEaraInf(void)
{
#if 0
    struct earaComm * handle = getEaraComm();
    EARACOM_VER("startInf");

    std::lock_guard<std::mutex> gLock(handle->refMtx);
    if(handle->refCnt == 0)
    {
        if(handle->libHandle == nullptr)
        {
            EARACOM_DEBUG("Open eara lib");
            handle->libHandle = dlopen(MTK_EARA_LIB, RTLD_NOW);
            if(handle->libHandle == nullptr)
            {
                EARACOM_WARN("Warn: Can't open library: %s", dlerror());
                return -1;
            }
            handle->startFn = dlsym(handle->libHandle, "earaNotifyCVJobBegin");
            handle->endFn = dlsym(handle->libHandle, "earaNotifyCVJobEnd");
            if(handle->startFn == nullptr || handle->endFn == nullptr)
            {
                EARACOM_WARN("Warn: Can't open function: %s", dlerror());
                return -1;
            }
        }
    }
    handle->refCnt++;
#endif
    return 0;
}

int closeEaraInf(void)
{
#if 0
    struct earaComm * handle = getEaraComm();
    std::lock_guard<std::mutex> gLock(handle->refMtx);
    EARACOM_VER("endInf");

    handle->refCnt--;
    if(handle->refCnt == 0)
    {
        EARACOM_DEBUG("Close eara lib");
        dlclose(handle->libHandle);
        handle->libHandle = nullptr;
        handle->startFn = nullptr;
        handle->endFn = nullptr;
    }
#endif
    return 0;
}

uint8_t calcOppFromBoostValue(uint8_t boostValue)
{
#if 0
    int opp = -1;
    int targetFreq = 0;

    if(boostValue < 0 || boostValue > 100)
    {
        return ePowerOppUnrequest;
    }

    targetFreq = g_vpu_opp_table[VPU_OPP_0] * boostValue / 100;

    for(opp = VPU_OPP_0; opp < VPU_OPP_NUM; opp++)
    {
        if(opp == VPU_OPP_NUM-1)
        {
            return opp;
        }

        if(targetFreq <= g_vpu_opp_table[opp] && targetFreq > g_vpu_opp_table[opp+1])
        {
            return opp;
        }
    }
#endif
    return ePowerOppUnrequest;
}

uint8_t calcBoostvalueFromOpp(uint8_t opp)
{
#if 0
    if(opp < VPU_OPP_0 || opp >= VPU_OPP_NUM)
    {
        return ePowerBoostUnrequest;
    }

    return g_vpu_opp_table[opp] * 100 / g_vpu_opp_table[VPU_OPP_0];
#else
    return ePowerBoostUnrequest;
#endif
}

#if 0  // unused functions
uint64_t getEaraSuggestTime(void)
{
    char prop[100];
    uint64_t suggestTime = -2;

#ifdef __ANDROID__
    property_get(PROPERTY_DEBUG_EARATIME, prop, "-2");
    suggestTime = (uint64_t)atoi(prop);
#else
    suggestTime = -2;
#endif

    VPU_LOGD("[earaVpu][test] change eara earatime = %lu/%s",
        (unsigned long)suggestTime, prop);

    return suggestTime;
}

int getEaraPriority(void)
{
    char prop[100];
    int jobPriority = -2;

#ifdef __ANDROID__
    property_get(PROPERTY_DEBUG_EARAPRIORITY, prop, "0");
    jobPriority = atoi(prop);
#else
    jobPriority = 0;
#endif

    VPU_LOGD("[earaVpu][test] change eara earapriority = %d/%s", jobPriority, prop);

    return jobPriority;
}

int getUserBoostValue(void)
{
    char prop[100];
    int boostValue = -1;

#ifdef __ANDROID__
    property_get(PROPERTY_DEBUG_EARABOOSTVALUE, prop, "-1");
    boostValue = atoi(prop);
#else
    boostValue = -1;
#endif

    VPU_LOGD("[earaVpu][test] change user boostvalue = %d/%s", boostValue, prop);

    return boostValue;
}
#endif

void initEaraDisable(void)
{
#if 0
    char prop[100];

#ifdef __ANDROID__
    property_get(PROPERTY_DEBUG_EARADISABLE, prop, "0");
    gEaraDisable = atoi(prop);
#else
    gEaraDisable = 1;
#endif
#endif
    return;
}

int isEaraDisabled(void) {
    return gEaraDisable;
}

