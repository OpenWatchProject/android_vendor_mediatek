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

#include "MtkPower.h"
#include "mtkpower_hint.h"
#include "mtkpower_types.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mtkpower {
namespace V1_0 {
namespace implementation {

MtkPower::MtkPower() {
}

MtkPower::~MtkPower() {
}

Return<void> MtkPower::mtkPowerHint(int32_t hint, int32_t data)  {
    ALOGD("mtkPowerHint hint:%d, data:%d", hint, data);

    struct tPowerData vPowerData;
    struct tHintData  vHintData;
    struct tPowerData *vpRspData = NULL;

    if(hint < MTKPOWER_HINT_BASE || hint >= MTKPOWER_HINT_NUM) {
        ALOGI("mtkPowerHint unsupport hint:%d", hint);
        return Void();
    }

    vHintData.hint = (int)hint;
    vHintData.data = data;
    vPowerData.msg  = POWER_MSG_MTK_HINT;
    vPowerData.pBuf = (void*)&vHintData;

    //ALOGI("%s %p", __func__, &vPowerData);

    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);
    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }

    return Void();
}

Return<void> MtkPower::mtkCusPowerHint(int32_t hint, int32_t data)  {
    ALOGD("mtkCusPowerHint hint:%d, data:%d", hint, data);

    struct tPowerData vPowerData;
    struct tHintData  vHintData;
    struct tPowerData *vpRspData = NULL;

    //if(hint >= (int32_t)MtkCusPowerHintInternal::MTK_CUS_HINT_NUM)
    //    return Void();

    vHintData.hint = (int)hint;
    vHintData.data = data;
    vPowerData.msg  = POWER_MSG_MTK_CUS_HINT;
    vPowerData.pBuf = (void*)&vHintData;

    //ALOGI("%s %p", __func__, &vPowerData);

    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);
    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }

    return Void();
}

Return<void> MtkPower::notifyAppState(const hidl_string& packName, const hidl_string& actName, int32_t pid, int32_t state, int32_t uid) {
    ALOGD("notifyAppState pack:%s, act:%s, pid:%d, state:%d, uid:%d", packName.c_str(), actName.c_str(), pid, state, uid);

    struct tPowerData vPowerData;
    struct tAppStateData vStateData;
    struct tPowerData *vpRspData = NULL;

    strncpy(vStateData.pack, packName.c_str(), (MAX_NAME_LEN - 1));
    vStateData.pack[(MAX_NAME_LEN-1)] = '\0';
    strncpy(vStateData.activity, actName.c_str(), (MAX_NAME_LEN - 1));
    vStateData.activity[(MAX_NAME_LEN-1)] = '\0';
    vStateData.pid = pid;
    vStateData.uid = uid;
    vStateData.state = (int)state;
    vPowerData.msg  = POWER_MSG_NOTIFY_STATE;
    vPowerData.pBuf = (void*)&vStateData;

    //ALOGI("%s %p", __func__, &vScnData);

    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);

    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }

    return Void();
}

Return<int32_t> MtkPower::querySysInfo(int32_t cmd, int32_t param)  {
    ALOGD("querySysInfo cmd:%d, param:%d", (int)cmd, param);

    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tQueryInfoData vQueryData;

    vQueryData.cmd = (int)cmd;
    vQueryData.param = param;
    vPowerData.msg  = POWER_MSG_QUERY_INFO;
    vPowerData.pBuf = (void*)&vQueryData;

    //ALOGI("%s %p", __func__, &vPowerData);
    vQueryData.value = -1;
    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);
    if (vpRspData) {
        if(vpRspData->pBuf) {
            vQueryData.value = ((tQueryInfoData*)(vpRspData->pBuf))->value;
            free(vpRspData->pBuf);
        }
        free(vpRspData);
    }

    return vQueryData.value;
    //return 0;
}

Return<int32_t> MtkPower::setSysInfo(int32_t type, const hidl_string& data) {
    ALOGD("setSysInfo type:%d data:%s", type, data.c_str());

    struct tPowerData vPowerData;
    struct tSysInfoData vSysInfoData;
    struct tPowerData *vpRspData = NULL;

    vSysInfoData.type = type;
    strncpy(vSysInfoData.data, data.c_str(), (MAX_SYSINFO_LEN - 1));
    vSysInfoData.data[(MAX_SYSINFO_LEN-1)] = '\0';
    vPowerData.msg  = POWER_MSG_SET_SYSINFO;
    vPowerData.pBuf = (void*)&vSysInfoData;

    power_msg(&vPowerData, (void **) &vpRspData);

    if (vpRspData) {
        if(vpRspData->pBuf) {
            vSysInfoData.ret = ((tSysInfoData*)(vpRspData->pBuf))->ret;
            free(vpRspData->pBuf);
        }
        free(vpRspData);
    }

    return vSysInfoData.ret;
}

Return<void> MtkPower::setSysInfoAsync(int32_t type, const hidl_string& data) {
    ALOGD("setSysInfoAsync type:%d data:%s", type, data.c_str());

    struct tPowerData vPowerData;
    struct tSysInfoData vSysInfoData;
    struct tPowerData *vpRspData = NULL;

    vSysInfoData.type = type;
    strncpy(vSysInfoData.data, data.c_str(), (MAX_SYSINFO_LEN - 1));
    vSysInfoData.data[(MAX_SYSINFO_LEN-1)] = '\0';
    vPowerData.msg  = POWER_MSG_SET_SYSINFO;
    vPowerData.pBuf = (void*)&vSysInfoData;

    power_msg(&vPowerData, (void **) &vpRspData);

    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }

    return Void();
}

IMtkPower* HIDL_FETCH_IMtkPower(const char* /* name */) {
    ALOGE("HIDL_FETCH_IMtkPower");
    return new MtkPower();
}

} // namespace implementation
}  // namespace V1_0
}  // namespace mtkpower
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

