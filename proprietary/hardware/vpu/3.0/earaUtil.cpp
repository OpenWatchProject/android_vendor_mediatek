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

#include "earaUtil.h"
#include "vpu.h"
#include <vpu_drv.h>

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
    uint64_t targetTime = 0;
    struct earaComm * handle = getEaraComm();
    fnEaraNotifyJobBegin fnJobBegin = nullptr;

    EARACOM_DEBUG("earaNotifyCvJobBegin: jobid = 0x%x",jobId);

    std::lock_guard<std::mutex> gLock(handle->refMtx);

    if(handle->startFn == nullptr)
    {
        EARACOM_DEBUG("Warn: function earaNotifyJobBegin is not init");
        return -1;
    }

    fnJobBegin = reinterpret_cast<fnEaraNotifyJobBegin>(handle->startFn);

    // call EARA's API
    EARACOM_VER("Send job begin to eara");
    if(fnJobBegin(gettid(), jobId, &targetTime))
    {
        EARACOM_DEBUG("Warn: fnJobBegin fail");
        return -1;
    }
    suggestExecTime = targetTime;

    return 0;
}

int earaNotifyJobEnd(int jobId, uint64_t executeTime, uint32_t bandwidth, int32_t errorStatus, int32_t &jobPriority, int32_t boostVal)
{
    int32_t priority = 0;
    struct earaComm * handle = getEaraComm();
    std::vector<std::vector<int32_t>> boost;
    std::vector<int32_t> boostInner;
    fnEearaNotifyJobEnd fnJobEnd = nullptr;

    EARACOM_DEBUG("earaNotifyCvJobEnd: jobid = 0x%x, boost = %d", jobId, boostVal);

    std::lock_guard<std::mutex> gLock(handle->refMtx);

    if(handle->startFn == nullptr)
    {
        EARACOM_DEBUG("Warn: function earaNotifyJobBegin is not init");
        return -1;
    }

    fnJobEnd = reinterpret_cast<fnEearaNotifyJobEnd>(handle->endFn);

    boostInner.push_back(boostVal);
    boost.push_back(boostInner);

    // call EARA's API
    EARACOM_VER("Send job end to eara");
    if(fnJobEnd(gettid(), jobId, &priority, errorStatus, executeTime, bandwidth, boost))
    {
        EARACOM_DEBUG("Warn: fnJobEnd fail");
        return -1;
    }
    jobPriority = priority;

    return 0;
}

int getJobIdFromPointer(void *addr)
{
    int jobId = static_cast<int>(reinterpret_cast<uintptr_t>(addr));

    return jobId;
}

int startEaraInf(void)
{
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
                EARACOM_WARN("Warn: Can't open library: %s, ignore eara", dlerror());
                return -1;
            }
            handle->startFn = dlsym(handle->libHandle, "earaNotifyCVJobBegin");
            handle->endFn = dlsym(handle->libHandle, "earaNotifyCVJobEnd");
            if(handle->startFn == nullptr || handle->endFn == nullptr)
            {
                EARACOM_WARN("Warn: Can't open function: %s, ignore eara", dlerror());
                return -1;
            }
        }
    }
    handle->refCnt++;

    return 0;
}

int closeEaraInf(void)
{
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

    return 0;
}

uint8_t calcOppFromBoostValue(uint8_t boostValue)
{
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

    return ePowerOppUnrequest;
}

uint8_t calcBoostvalueFromOpp(uint8_t opp)
{
    int boostValue = -1;

    if(opp < VPU_OPP_0 || opp >= VPU_OPP_NUM)
    {
        return ePowerBoostUnrequest;
    }

    return g_vpu_opp_table[opp] * 100 / g_vpu_opp_table[VPU_OPP_0];
}
