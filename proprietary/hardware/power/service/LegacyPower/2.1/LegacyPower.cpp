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

#define LOG_TAG "legacy_power@2.1-impl"
#define ATRACE_TAG (ATRACE_TAG_POWER | ATRACE_TAG_HAL)

#include <log/log.h>
#include <cutils/trace.h>
#include <string.h>
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <time.h>

#include "power_util.h"

#include "power_cus_types.h"
#include "mtkpower_types.h"

#include "LegacyPower.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace power {
namespace V2_1 {
namespace implementation {

using MtkPowerCmd_2_0 = ::vendor::mediatek::hardware::power::V2_0::MtkPowerCmd;
using MtkPowerCmd_2_1 = ::vendor::mediatek::hardware::power::V2_1::MtkPowerCmd;
using MtkQueryCmd_2_0 = ::vendor::mediatek::hardware::power::V2_0::MtkQueryCmd;

enum spm_mode {
    DEEPIDLE = 0,
    SODI3 = 1,
    SPM_MODE_NUM
};

enum spm_voter {
    AP = 0,
    SPM_VOTER_NUM
};

enum sub_spm {
    CONSYS = 0,
    SUB_SPM_NUM
};

static int suspend_count = 0;
struct timespec suspend_t;

static int switchLegacyQueryCmd(MtkQueryCmd_2_0 cmd)

{
    int mtkPowerCmd = -1;

    switch(cmd) {
    case MtkQueryCmd_2_0::CMD_GET_CLUSTER_NUM:
        mtkPowerCmd = MTKPOWER_CMD_GET_CLUSTER_NUM;
        break;

    case MtkQueryCmd_2_0::CMD_GET_CLUSTER_CPU_NUM:
        mtkPowerCmd = MTKPOWER_CMD_GET_CLUSTER_CPU_NUM;
        break;

    case MtkQueryCmd_2_0::CMD_GET_CLUSTER_CPU_FREQ_MIN:
        mtkPowerCmd = MTKPOWER_CMD_GET_CLUSTER_CPU_FREQ_MIN;
        break;

    case MtkQueryCmd_2_0::CMD_GET_CLUSTER_CPU_FREQ_MAX:
        mtkPowerCmd = MTKPOWER_CMD_GET_CLUSTER_CPU_FREQ_MAX;
        break;

    case MtkQueryCmd_2_0::CMD_GET_GPU_FREQ_COUNT:
        mtkPowerCmd = MTKPOWER_CMD_GET_GPU_FREQ_COUNT;
        break;

    case MtkQueryCmd_2_0::CMD_GET_FOREGROUND_PID:
        mtkPowerCmd = MTKPOWER_CMD_GET_FOREGROUND_PID;
        break;

    default:
        break;
    }

    return mtkPowerCmd;
}

static std::string PowerHintToString(int32_t hint, int32_t data) {
    char powerHintString[50];
    switch (hint) {
        /* hint 1.0 */
        case (int32_t)::android::hardware::power::V1_0::PowerHint::VSYNC:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "VSYNC",
                     data ? "requested" : "not requested");
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::INTERACTION:
            snprintf(powerHintString, sizeof(powerHintString), "%s %d ms", "INTERACTION", data);
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::VIDEO_ENCODE:
            snprintf(powerHintString, sizeof(powerHintString), "%s", "VIDEO_ENCODE");
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::VIDEO_DECODE:
            snprintf(powerHintString, sizeof(powerHintString), "%s", "VIDEO_DECODE");
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::LOW_POWER:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "LOW_POWER",
                     data ? "activated" : "deactivated");
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::SUSTAINED_PERFORMANCE:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "SUSTAINED_PERFORMANCE",
                     data ? "activated" : "deactivated");
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::VR_MODE:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "VR_MODE",
                     data ? "activated" : "deactivated");
            break;
        case (int32_t)::android::hardware::power::V1_0::PowerHint::LAUNCH:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "LAUNCH",
                     data ? "start" : "end");
            break;

        /* hint 1.2 */
        case (int32_t)::android::hardware::power::V1_2::PowerHint::AUDIO_STREAMING:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "AUDIO_STREAMING",
                     data ? "start" : "end");
            break;
        case (int32_t)::android::hardware::power::V1_2::PowerHint::AUDIO_LOW_LATENCY:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "AUDIO_LOW_LATENCY",
                     data ? "start" : "end");
            break;
        case (int32_t)::android::hardware::power::V1_2::PowerHint::CAMERA_LAUNCH:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "CAMERA_LAUNCH",
                     data ? "start" : "end");
            break;
        case (int32_t)::android::hardware::power::V1_2::PowerHint::CAMERA_STREAMING:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "CAMERA_STREAMING",
                     data ? "start" : "end");
            break;
        case (int32_t)::android::hardware::power::V1_2::PowerHint::CAMERA_SHOT:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "CAMERA_SHOT",
                     data ? "start" : "end");
            break;

        default:
            snprintf(powerHintString, sizeof(powerHintString), "%s", "UNKNOWN");
            break;
    }
    return powerHintString;
}

Power::Power() {}

Power::~Power() {}

void Power::powerHintInternal(int32_t hint, int32_t data)
{
    struct tPowerData vPowerData;
    struct tHintData  vHintData;
    struct tPowerData *vpRspData = NULL;

    //ALOGI("powerHintInternal hint:%d, data:%d", hint, data);

    switch(hint) {

    /* Add switch case here to support more hint */
    case (int32_t)::android::hardware::power::V1_0::PowerHint::LAUNCH:
    case (int32_t)::android::hardware::power::V1_2::PowerHint::CAMERA_LAUNCH:

        ALOGI("@Deprecated powerHintInternal hint:%d, data:%d", hint, data);

        if (ATRACE_ENABLED()) {
            ATRACE_BEGIN(PowerHintToString(hint, data).c_str());
        }

        vHintData.hint = (int)hint;
        vHintData.data = data;
        vPowerData.msg  = POWER_MSG_AOSP_HINT;
        vPowerData.pBuf = (void*)&vHintData;

        power_msg(&vPowerData, (void **) &vpRspData);

        ATRACE_END();
        break;

    default:
        ALOGD("powerHintInternal hint:%d data:%d, not support", hint, data);
        break;
    }

    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }
}

Return<void> Power::setInteractive(bool interactive)  {
    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tScnData vScnData;

    if (interactive) {
      ALOGI("@Deprecated %s %s", __func__, "Restore All");
      vPowerData.msg  = POWER_MSG_SCN_RESTORE_ALL;
    } else {
      vPowerData.msg  = POWER_MSG_SCN_DISABLE_ALL;
      suspend_count++;
      clock_gettime(CLOCK_BOOTTIME, &suspend_t);
      ALOGI("@Deprecated %s %s %d %ld", __func__, "Disable All", suspend_count, suspend_t.tv_sec);
    }
    vPowerData.pBuf = (void*)&vScnData;

    power_msg(&vPowerData, (void **) &vpRspData);

    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }

    return Void();
}

Return<void> Power::powerHint(::android::hardware::power::V1_0::PowerHint hint, int32_t data)  {
    powerHintInternal((int32_t)hint, data);
    return Void();
}

Return<void> Power::setFeature(Feature feature, bool activate)  {
    ALOGV("setFeature, %d, %d", (int)feature, (int)activate);
    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb)  {
    hidl_vec<PowerStatePlatformSleepState> states;
    struct PowerStatePlatformSleepState *state;

    states.resize(spm_mode::SPM_MODE_NUM);

    /* Update statistics for XO_shutdown */
    state = &states[spm_mode::DEEPIDLE];
    state->name = "DEEPIDLE";

    /*
    state->residencyInMsecSinceBoot = stats[platform_param_id::ACCUMULATED_VLOW_TIME];
    state->totalTransitions = stats[platform_param_id::VLOW_COUNT];
    */
    state->residencyInMsecSinceBoot = ((uint64_t)suspend_t.tv_sec * 1000);
    state->totalTransitions = suspend_count;
    state->supportedOnlyInSuspend = false;
    state->voters.resize(spm_voter::SPM_VOTER_NUM);

    /* Update statistics for AP voter */
    state->voters[spm_voter::AP].name = "AP";
    /*
    state->voters[0].totalTimeInMsecVotedForSinceBoot =
        stats[platform_param_id::XO_ACCUMULATED_DURATION_APSS] / RPM_CLK;
    state->voters[0].totalNumberOfTimesVotedSinceBoot = stats[platform_param_id::XO_COUNT_APSS];
    */
    state->voters[spm_voter::AP].totalTimeInMsecVotedForSinceBoot = ((uint64_t)suspend_t.tv_sec * 1000);
    state->voters[spm_voter::AP].totalNumberOfTimesVotedSinceBoot = suspend_count;

    state = &states[spm_mode::SODI3];
    state->name = "SODI3";

    state->residencyInMsecSinceBoot = ((uint64_t)suspend_t.tv_sec * 1000);
    state->totalTransitions = suspend_count;
    state->supportedOnlyInSuspend = false;
    state->voters.resize(0);

    _hidl_cb(states, Status::SUCCESS);
    return Void();
}

static int get_consys_low_power_stats(struct PowerStateSubsystem &subsystem) {
    ALOGV("get_consys_low_power_stats %s", subsystem.name.c_str());
    return 0;
}

// Methods from ::android::hardware::power::V1_1::IPower follow.
Return<void> Power::getSubsystemLowPowerStats(getSubsystemLowPowerStats_cb _hidl_cb) {
    hidl_vec<PowerStateSubsystem> subsystems;
    int ret;

    subsystems.resize(sub_spm::SUB_SPM_NUM);

    //We currently have only one Subsystem for WLAN
    ret = get_consys_low_power_stats(subsystems[sub_spm::CONSYS]);
    if (ret != 0) {
        goto done;
    }

    //Add query for other subsystems here

done:
    _hidl_cb(subsystems, Status::SUCCESS);
    return Void();
}

Return<void> Power::powerHintAsync(::android::hardware::power::V1_0::PowerHint hint, int32_t data) {
    ALOGI("@Deprecated powerHintAsync hint:%d, data:%d", hint, data);
    powerHintInternal((int32_t)hint, data);
    return Void();
}

Return<void> Power::powerHintAsync_1_2(::android::hardware::power::V1_2::PowerHint hint, int32_t data) {
    ALOGI("@Deprecated powerHintAsync_1_2 hint:%d, data:%d", hint, data);
    powerHintInternal((int32_t)hint, data);
    return Void();
}

Return<void> Power::mtkPowerHint(::vendor::mediatek::hardware::power::V2_0::MtkPowerHint hint, int32_t data)  {
    ALOGI("@Deprecated mtkPowerHint hint:%d, data:%d", hint, data);
    return Void();
}

Return<void> Power::mtkCusPowerHint(int32_t hint, int32_t data)  {
    ALOGI("@Deprecated mtkCusPowerHint hint:%d, data:%d", hint, data);
    return Void();
}

Return<void> Power::notifyAppState(const hidl_string& packName, const hidl_string& actName, int32_t pid, MtkActState state) {
    ALOGI("@Deprecated notifyAppState pack:%s, act:%s, pid:%d, state:%d", packName.c_str(), actName.c_str(), pid, state);
    return Void();
}

Return<void> Power::notifyAppState_2_1(const hidl_string& packName, const hidl_string& actName, int32_t pid, MtkActState state, int32_t uid) {
    ALOGI("@Deprecated notifyAppState_2_1 pack:%s, act:%s, pid:%d, uid:%d, state:%d", packName.c_str(), actName.c_str(), pid, uid, state);
    return Void();
}

Return<int32_t> Power::querySysInfo(::vendor::mediatek::hardware::power::V2_0::MtkQueryCmd cmd, int32_t param)  {
    ALOGD("@Deprecated querySysInfo cmd:%d, param:%d", (int)cmd, param);

    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tQueryInfoData vQueryData;

    vQueryData.cmd = switchLegacyQueryCmd(cmd);
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
}

Return<int32_t> Power::scnReg() {
    ALOGD("@Deprecated scnReg");

    const int pid = IPCThreadState::self()->getCallingPid();
    const int uid = IPCThreadState::self()->getCallingUid();

    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tScnData   vScnData;

    vScnData.param1 = pid;
    vScnData.param2 = uid;
    vPowerData.msg  = POWER_MSG_SCN_REG;
    vPowerData.pBuf = (void*)&vScnData;

    //ALOGI("%s %p", __func__, &vScnData);
    vScnData.handle = -1;
    power_msg(&vPowerData, (void **) &vpRspData);

    //ALOGI("%s %p", __func__, vpRspData);

    if (vpRspData) {
        if(vpRspData->pBuf) {
            vScnData.handle = ((tScnData*)(vpRspData->pBuf))->handle;
            ALOGI("%s hdl:%d", __func__, vScnData.handle);
            free(vpRspData->pBuf);
        }
        free(vpRspData);
    }

    return vScnData.handle;
}

Return<void> Power::scnConfig(int32_t hdl, MtkPowerCmd_2_0 cmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4) {
    ALOGI("@Deprecated scnConfig hdl:%d, cmd:%d, param1:%d, param2:%d, param3:%d, param4:%d", hdl, cmd, param1, param2, param3, param4);

    struct tPowerData vPowerData;
    struct tScnData vScnData;
    struct tPowerData *vpRspData = NULL;

    if ((int)cmd > (int32_t)MtkPowerCmd::CMD_SET_END_JUMP) {
        ALOGI("scnConfig err cmd:%d", (int)cmd);
        return Void();
    }

    vScnData.handle = hdl;
    vScnData.command = (int)cmd;
    vScnData.param1 = param1;
    vScnData.param2 = param2;
    vScnData.param3 = param3;
    vScnData.param4 = param4;
    vScnData.timeout = 0;
    vPowerData.msg  = POWER_MSG_SCN_CONFIG;
    vPowerData.pBuf = (void*)&vScnData;

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

Return<void> Power::scnConfig_2_1(int32_t hdl, MtkPowerCmd_2_1 cmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4) {
    ALOGI("@Deprecated scnConfig_2_1 hdl:%d, cmd:%d, param1:%d, param2:%d, param3:%d, param4:%d", hdl, cmd, param1, param2, param3, param4);

    struct tPowerData vPowerData;
    struct tScnData vScnData;
    struct tPowerData *vpRspData = NULL;

    if ((int)cmd > (int32_t)MtkPowerCmd_2_1::CMD_SET_END_JUMP_2_1) {
        ALOGI("scnConfig err cmd:%d", (int)cmd);
        return Void();
    }

    vScnData.handle = hdl;
    vScnData.command = (int)cmd;
    vScnData.param1 = param1;
    vScnData.param2 = param2;
    vScnData.param3 = param3;
    vScnData.param4 = param4;
    vScnData.timeout = 0;
    vPowerData.msg  = POWER_MSG_SCN_CONFIG;
    vPowerData.pBuf = (void*)&vScnData;

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


Return<void> Power::scnUnreg(int32_t hdl) {
    ALOGI("@Deprecated scnUnreg hdl:%d", hdl);

    struct tPowerData vPowerData;
    struct tScnData vScnData;
    struct tPowerData *vpRspData = NULL;

    vScnData.handle = hdl;
    vScnData.command = 0;
    vScnData.param1 = 0;
    vScnData.param2 = 0;
    vScnData.param3 = 0;
    vScnData.param4 = 0;
    vScnData.timeout = 0;
    vPowerData.msg  = POWER_MSG_SCN_UNREG;
    vPowerData.pBuf = (void*)&vScnData;

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

Return<void> Power::scnEnable(int32_t hdl, int32_t timeout) {
    ALOGI("@Deprecated scnEnable hdl:%d, timeout:%d", hdl, timeout);

    struct tPowerData vPowerData;
    struct tScnData vScnData;
    struct tPowerData *vpRspData = NULL;

    vScnData.handle = hdl;
    vScnData.timeout = timeout;
    vPowerData.msg  = POWER_MSG_SCN_ENABLE;
    vPowerData.pBuf = (void*)&vScnData;

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

Return<void> Power::scnUltraCfg(int32_t hdl, int32_t ultracmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4) {
    ALOGD("@Deprecated scnUltaCfg hdl:%d, cmd:%d, param1:%d, param2:%d, param3:%d, param4:%d", hdl, ultracmd, param1, param2, param3, param4);

    struct tPowerData vPowerData;
    struct tScnData vScnData;
    struct tPowerData *vpRspData = NULL;

    if (ultracmd < (int32_t)MtkPowerCmd_2_1::CMD_SET_END_JUMP_2_1) {
        ALOGI("scnUltraCfg err cmd:%d", ultracmd);
        return Void();
    }

    vScnData.handle = hdl;
    vScnData.command = ultracmd;
    vScnData.param1 = param1;
    vScnData.param2 = param2;
    vScnData.param3 = param3;
    vScnData.param4 = param4;
    vScnData.timeout = 0;
    vPowerData.msg  = POWER_MSG_SCN_ULTRA_CFG;
    vPowerData.pBuf = (void*)&vScnData;

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

Return<void> Power::scnDisable(int32_t hdl) {
    ALOGI("@Deprecated scnDisable hdl:%d", hdl);

    struct tPowerData vPowerData;
    struct tScnData vScnData;
    struct tPowerData *vpRspData = NULL;

    vScnData.handle = hdl;
    vPowerData.msg  = POWER_MSG_SCN_DISABLE;
    vPowerData.pBuf = (void*)&vScnData;

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

Return<void> Power::setSysInfo(int32_t type, const hidl_string& data) {
    ALOGI("@Deprecated setSysInfo type:%d data:%s", type, data.c_str());
    return Void();
}

IPower* HIDL_FETCH_IPower(const char* /* name */) {
    return new Power();
}

} // namespace implementation
}  // namespace V2_1
}  // namespace power
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

