/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "mtkpower@1.0-impl"
#define ATRACE_TAG (ATRACE_TAG_POWER | ATRACE_TAG_HAL)

#include <log/log.h>
#include <cutils/trace.h>
#include <string.h>
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <time.h>

#include "power_util.h"

#include "power_cus_types.h"

#include "MtkPerf.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mtkpower {
namespace V1_0 {
namespace implementation {

MtkPerf::MtkPerf() {
}

MtkPerf::~MtkPerf() {
}

Return<int32_t> MtkPerf::perfLockAcquire(int32_t hdl, uint32_t duration, const hidl_vec<int32_t>& boostList, int32_t reserved)  {
    ALOGI("perfLockAcquire hdl:%d, duration:%d, reserved:%d", hdl, duration, reserved);

    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tPerfLockData vPerfLockData;
    const int pid = IPCThreadState::self()->getCallingPid();
    const int uid = IPCThreadState::self()->getCallingUid();
    int i;

    vPerfLockData.hdl = hdl;
    vPerfLockData.duration = duration;
    vPerfLockData.rscList = (int*)boostList.data();
    vPerfLockData.size = boostList.size();
    vPerfLockData.reserved = reserved;
    vPerfLockData.pid = pid;
    vPerfLockData.uid = uid;
    vPowerData.msg  = POWER_MSG_PERF_LOCK_ACQ;
    vPowerData.pBuf = (void*)&vPerfLockData;

    ALOGD("%s data size:%d", __func__, vPerfLockData.size);
    if (vPerfLockData.size % 2 != 0) {
        ALOGE("%s wrong data size:%d", __func__, vPerfLockData.size);
        return -1;
    }

    for (i=0; i<vPerfLockData.size; i+=2)
        ALOGD("rsc:0x%08x, value:%d", vPerfLockData.rscList[i], vPerfLockData.rscList[i+1]);

    //ALOGI("%s %p", __func__, &vPowerData);
    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);
    if (vpRspData) {
        if(vpRspData->pBuf) {
            vPerfLockData.hdl = ((tPerfLockData*)(vpRspData->pBuf))->hdl;
            free(vpRspData->pBuf);
        }
        free(vpRspData);
    }

    return vPerfLockData.hdl;
    //return 0;
}

Return<void> MtkPerf::perfLockRelease(int32_t hdl, int32_t reserved) {
    ALOGI("perfLockRelease hdl:%d reserved:%d", hdl, reserved);

    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tPerfLockData vPerfLockData;

    vPerfLockData.hdl = hdl;
    vPerfLockData.reserved = reserved;
    vPowerData.msg  = POWER_MSG_PERF_LOCK_REL;
    vPowerData.pBuf = (void*)&vPerfLockData;

    //ALOGI("%s %p", __func__, &vPowerData);
    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);
    if (vpRspData) {
        if(vpRspData->pBuf) {
            vPerfLockData.hdl = ((tPerfLockData*)(vpRspData->pBuf))->hdl;
            free(vpRspData->pBuf);
        }
        free(vpRspData);
    }

    return Void();
}

IMtkPerf* HIDL_FETCH_IMtkPerf(const char* /* name */) {
    ALOGE("HIDL_FETCH_IMtkPerf");
    return new MtkPerf();
}

} // namespace implementation
}  // namespace V1_0
}  // namespace mtkpower
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

