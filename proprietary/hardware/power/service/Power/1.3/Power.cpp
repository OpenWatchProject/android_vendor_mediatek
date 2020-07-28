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

#define LOG_TAG "power@1.3-impl"
#define ATRACE_TAG (ATRACE_TAG_POWER | ATRACE_TAG_HAL)

#include <log/log.h>
#include <cutils/trace.h>
#include <string.h>
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <time.h>

#include "power_util.h"
#include "power_cus_types.h"
#include "mtkpower_hint.h"

#include "Power.h"

namespace android {
namespace hardware {
namespace power {
namespace V1_3 {
namespace implementation {

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

Power::Power() {}

Power::~Power() {}

//std::string PowerHintToString(::android::hardware::power::V1_0::PowerHint hint, int32_t data) {
std::string PowerHintToString(int32_t hint, int32_t data) {
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

        /* hint 1.3 */
        case (int32_t)::android::hardware::power::V1_3::PowerHint::EXPENSIVE_RENDERING:
            snprintf(powerHintString, sizeof(powerHintString), "%s %s", "EXPENSIVE_RENDERING",
                     data ? "start" : "end");
            break;


        default:
            snprintf(powerHintString, sizeof(powerHintString), "%s", "UNKNOWN");
            break;
    }
    return powerHintString;
}

// Methods from ::android::hardware::power::V1.0::IPower follow.
Return<void> Power::setInteractive(bool interactive)  {
    /*if (mModule->setInteractive)
        mModule->setInteractive(mModule, interactive ? 1 : 0);
    */
    struct tPowerData vPowerData;
    struct tPowerData *vpRspData = NULL;
    struct tScnData vScnData;

    if (interactive) {
      ALOGI("%s %s", __func__, "Restore All");
      vPowerData.msg  = POWER_MSG_SCN_RESTORE_ALL;
    } else {
      vPowerData.msg  = POWER_MSG_SCN_DISABLE_ALL;
      suspend_count++;
      clock_gettime(CLOCK_BOOTTIME, &suspend_t);
      ALOGI("%s %s %d %ld", __func__, "Disable All", suspend_count, suspend_t.tv_sec);
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

void Power::powerHintInternal(int32_t hint, int32_t data)
{
    int32_t param = data;
    struct tPowerData vPowerData;
    struct tHintData  vHintData;
    struct tPowerData *vpRspData = NULL;

    switch(hint) {

    /* Add switch case here to support more hint */
    case (int32_t)::android::hardware::power::V1_0::PowerHint::INTERACTION:
    case (int32_t)::android::hardware::power::V1_2::PowerHint::CAMERA_LAUNCH:
    case (int32_t)::android::hardware::power::V1_3::PowerHint::EXPENSIVE_RENDERING:
        ALOGD("powerHintInternal hint:%d, data:%d", hint, data);
        if (param > 0)
            vHintData.data = param;
        else
            vHintData.data = 0;
        break;

    case (int32_t)::android::hardware::power::V1_0::PowerHint::LAUNCH:
        ALOGI("powerHintInternal hint:%d, data:%d", hint, data);
        if (param > 0)
            vHintData.data = MTKPOWER_HINT_ALWAYS_ENABLE;
        else
            vHintData.data = 0;
        break;

    default:
        ALOGD("powerHintInternal hint:%d data:%d, not support", hint, data);
        return;
    }

    if (ATRACE_ENABLED()) {
        ATRACE_BEGIN(PowerHintToString(hint, data).c_str());
    }

    vHintData.hint = (int)hint;
    vPowerData.msg  = POWER_MSG_AOSP_HINT;
    vPowerData.pBuf = (void*)&vHintData;

    power_msg(&vPowerData, (void **) &vpRspData);

    ATRACE_END();

    if (vpRspData) {
        if(vpRspData->pBuf)
            free(vpRspData->pBuf);
        free(vpRspData);
    }
}

Return<void> Power::powerHint(::android::hardware::power::V1_0::PowerHint hint, int32_t data)  {
    powerHintInternal((int32_t)hint, data);
    return Void();
}

Return<void> Power::setFeature(Feature feature, bool activate)  {
#if 0
    if (mModule->setFeature)
        mModule->setFeature(mModule, static_cast<feature_t>(feature),
                activate ? 1 : 0);
#else
    ALOGI("setFeature feature:%d, feature:%d", (int)feature, (int)activate);
#endif
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

    struct PowerStateSubsystemSleepState *state;

    subsystem.name = "CONSYS";
    subsystem.states.resize(spm_mode::SPM_MODE_NUM);

    /* Update statistics for Active State */
    state = &subsystem.states[spm_mode::DEEPIDLE];
    state->name = "DpIdle";
    state->residencyInMsecSinceBoot = ((uint64_t)suspend_t.tv_sec * 1000);
    state->totalTransitions = suspend_count;
    state->lastEntryTimestampMs = 0;
    state->supportedOnlyInSuspend = false;

    /* Update statistics for Deep-Sleep state */
    state = &subsystem.states[spm_mode::SODI3];
    state->name = "SODI3";
    state->residencyInMsecSinceBoot = ((uint64_t)suspend_t.tv_sec * 1000);
    state->totalTransitions = suspend_count;
    state->lastEntryTimestampMs = 0;
    state->supportedOnlyInSuspend = false;

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
    powerHintInternal((int32_t)hint, data);
    return Void();
}

// Methods from ::android::hardware::power::V1_2::IPower follow.
Return<void> Power::powerHintAsync_1_2(::android::hardware::power::V1_2::PowerHint hint, int32_t data) {
    ALOGD("powerHintAsync_1_2 hint:%d, data:%d", hint, data);
    powerHintInternal((int32_t)hint, data);
    return Void();
}

// Methods from ::android::hardware::power::V1_2::IPower follow.
Return<void> Power::powerHintAsync_1_3(::android::hardware::power::V1_3::PowerHint hint, int32_t data) {
    ALOGD("powerHintAsync_1_3 hint:%d, data:%d", hint, data);
    powerHintInternal((int32_t)hint, data);
    return Void();
}


IPower* HIDL_FETCH_IPower(const char* /* name */) {
    return new Power();
}

} // namespace implementation
}  // namespace V1_3
}  // namespace power
}  // namespace hardware
}  // namespace android

