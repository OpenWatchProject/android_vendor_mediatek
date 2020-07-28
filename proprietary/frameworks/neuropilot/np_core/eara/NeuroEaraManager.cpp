/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#define LOG_TAG "NeuroEaraMgr"

#include "NeuroEaraManager.h"
#include "NeuroEaraUtils.h"
#include <dlfcn.h>
#include <inttypes.h>
#include <thread>
#include <unistd.h>

inline void* getEaraLibHandle() {
    static void *eara_handle = nullptr;

    if (eara_handle == nullptr) {
        eara_handle = dlopen("libperfctl_vendor.so", RTLD_NOW);
        if (eara_handle == NULL) {
            LOG(ERROR) << "Can't load libperfctl_vendor.so";
        }
    }

    return eara_handle;
}

typedef int (*earaNotifyJobBegin_fn)(int tid, uint64_t modelId,
        vector<vector<int32_t>> planStruct, vector<vector<uint64_t>> *suggestExecTime);

typedef int (*earaNotifyJobEnd_fn)(int tid, uint64_t modelId,
        int32_t *jobPriority, int execSuccess, vector<vector<int32_t>> planStruct,
        vector<vector<uint64_t>> nanoSec, vector<vector<int32_t>> bandwidth,
        vector<vector<int32_t>> boostValue);

typedef int (*earaGetUsage_fn)(int device, int *deviceLoading, int *bandwidth);

static unordered_map<string, DeviceComponent> s_DeviceComponentHash;

inline static DeviceComponent compStringToEnum(string &comp) {
    if (s_DeviceComponentHash.count(comp)) {
        return s_DeviceComponentHash[comp];
    } else if (comp == "cpu") {
        s_DeviceComponentHash[comp] = DeviceComponent::DEVICE_COMP_CPU;
    } else if (comp == "gpu") {
        s_DeviceComponentHash[comp] = DeviceComponent::DEVICE_COMP_GPU;
    } else if (comp == "vpu") {
        s_DeviceComponentHash[comp] = DeviceComponent::DEVICE_COMP_VPU;
    } else if (comp == "mdla") {
        s_DeviceComponentHash[comp] = DeviceComponent::DEVICE_COMP_MDLA;
    } else {
        LOG(ERROR) << "[compStringToEnum] invalid component:" << comp;
        return DeviceComponent::DEVICE_COMP_UNKNOWN;
    }
    LOG(INFO) << "[compStringToEnum] map " << comp << " -> " << s_DeviceComponentHash[comp];
    return s_DeviceComponentHash[comp];
}

NeuroEaraManager::NeuroEaraManager(vector<string> &plan) {
    mModelTag = " (MTag_" + std::to_string(reinterpret_cast<uint64_t>(this)) + ")";
    LOG(INFO) << "Construct" << mModelTag << " - contains " << plan.size() << " steps";
    mJobPriority = static_cast<int32_t>(JOB_PRIORITY_HIGH);
    mPreference = ANEUROEARA_PREFER_FAST_SINGLE_ANSWER;
    setCompilation(plan);
    for (unsigned int c = 0; c < plan.size(); c++) {
        DeviceComponent compEnum = compStringToEnum(plan[c]);
        mUsageThreshold[compEnum] = DEVICE_BOUND_THRESHOLD;
        mSleepTimeInMs[compEnum] = DEVICE_BOUND_SLEEP_TIME_MS;
    }
    mJobTimer.resetTime();
    mBoostValResetTimer.resetTime();
}

NeuroEaraManager::~NeuroEaraManager() {
    LOG(INFO) << "Destory" << mModelTag;

    std::lock_guard<std::mutex> guard(mMutex);

    mPlanStructs.clear();
    mSuggestExecTime.clear();
    mModelExecTime.clear();
    mModelMinExecTime.clear();
    mBandwidth.clear();
    mBoostValue.clear();
}

void NeuroEaraManager::setCompilation(vector<string> &plan) {
    mPlanStructs.resize(plan.size());
    mSuggestExecTime.resize(plan.size());
    mModelExecTime.resize(plan.size());
    mModelMinExecTime.resize(plan.size());
    mBandwidth.resize(plan.size());
    mBoostValue.resize(plan.size());
    for (unsigned int i = 0; i < plan.size(); i++) {
        VLOG << "[setCompilation] step[" << i << "] exec by " << plan[i] << mModelTag;
        mPlanStructs[i].emplace_back(static_cast<int32_t>(compStringToEnum(plan[i])));
        mSuggestExecTime[i].resize(1);
        mModelExecTime[i].resize(1);
        mModelMinExecTime[i].resize(1);
        mBandwidth[i].resize(1);
        mBoostValue[i].resize(1);
    }
}

int NeuroEaraManager::setExecutionPreference(ExecPreference preference) {
    VLOG << "[setExecutionPreference] mPreference = " << preference << mModelTag;
    mPreference = preference;
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::configSuspendParams(string &compStr,
        int usageThreshold, uint32_t sleepTimeInMs) {
    DeviceComponent comp = compStringToEnum(compStr);
    if (mUsageThreshold.count(comp) == 0 || mSleepTimeInMs.count(comp) == 0) {
        LOG(ERROR) << "[configSuspendParams] no such component:" << compStr;
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    VLOG << "[configSuspendParams] " << componentToString(comp)
            << " usageThreshold: " << usageThreshold
            << ", sleepTimeInMs: " << sleepTimeInMs << mModelTag;
    mUsageThreshold[comp] = usageThreshold;
    mSleepTimeInMs[comp] = sleepTimeInMs;
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::suggestSuspendTime(string &compStr, uint32_t *time) {
    DeviceComponent comp = compStringToEnum(compStr);
    if (mUsageThreshold.count(comp) == 0 || mSleepTimeInMs.count(comp) == 0) {
        LOG(ERROR) << "[suggestSuspendTime] no such component: " << componentToString(comp);
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    if (mJobPriority != JOB_PRIORITY_LOW) {
        *time = 0;
        return ANEUROEARA_NO_ERROR;
    }
    int deviceLoading = 0;
    int bandwidth = 0;
    getEaraUsage(comp, &deviceLoading, &bandwidth);
    if (bandwidth > BANDWIDTH_BOUND_THRESHOLD) {
        VLOG << "[suggestSuspendTime] bandwidth bound! bandwidth:"
                << bandwidth << mModelTag;
        *time = static_cast<uint32_t>(BANDWIDTH_BOUND_SLEEP_TIME_MS);
    } else if (deviceLoading > mUsageThreshold[comp]) {
        VLOG << "[suggestSuspendTime] " << componentToString(comp) << " bound! bandwidth:"
                << bandwidth << mModelTag;
        *time = mSleepTimeInMs[comp];
    }
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::getEaraUsage(string compStr, int *deviceLoading, int *bandwidth) {
    return getEaraUsage(compStringToEnum(compStr), deviceLoading, bandwidth);
}

int NeuroEaraManager::getEaraUsage(DeviceComponent comp, int *deviceLoading, int *bandwidth) {
    static earaGetUsage_fn getUsage_fn = nullptr;
    void* fn = nullptr;
    if (getEaraLibHandle() != nullptr) {
        fn = dlsym(getEaraLibHandle(), "earaGetUsage");
    }

    if (fn == nullptr) {
        LOG(ERROR) << "Error : unable to open function earaGetUsage";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    DeviceType type = DEVICE_TYPE_UNKNOWN;
    if (comp == DEVICE_COMP_CPU || comp == DEVICE_COMP_NEON) {
        type = DEVICE_TYPE_CPU;
    } else if (comp == DEVICE_COMP_GPU) {
        type = DEVICE_TYPE_GPU;
    } else if (comp == DEVICE_COMP_VPU || comp == DEVICE_COMP_MDLA) {
        type = DEVICE_TYPE_APU;
    } else {
        LOG(ERROR) << "[getEaraUsage] Invalid device enum:" << comp;
        (*deviceLoading) = 0;
        (*bandwidth) = 0;
        return ANEUROEARA_BAD_DATA;
    }
    getUsage_fn = reinterpret_cast<earaGetUsage_fn>(fn);
    getUsage_fn(static_cast<int>(type), deviceLoading, bandwidth);
    VLOG << "[getEaraUsage] " << componentToString(comp) << " usage:" << (*deviceLoading);
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::suggestOffload(string compStr, bool *execOffloadPlan) {
    (*execOffloadPlan) = false;
    if (mJobPriority == JOB_PRIORITY_LOW) {
        int deviceUsage = 0;
        int bandwidth = 0;
        NeuroEaraResultCode status = static_cast<NeuroEaraResultCode>(
                getEaraUsage(compStr, &deviceUsage, &bandwidth));
        if (status != ANEUROEARA_NO_ERROR) {
            return status;
        }
        if (deviceUsage > mUsageThreshold[compStringToEnum(compStr)]) {
            (*execOffloadPlan) = true;
            VLOG << "[suggestOffload] bandwidth:" << bandwidth
                    << ", deviceUsage:" << deviceUsage << mModelTag;
        }
    }
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::notifyExecutionBegin(int tid, uint64_t modelId,
        vector<int32_t> &suggestedBoostValue) {
    VLOG << "[notifyExecutionBegin] => tid:" << tid << ", mid:" << modelId << mModelTag;
    if (mPlanStructs.size() != suggestedBoostValue.size()) {
        LOG(ERROR) << " [notifyExecutionBegin] size not match!";
        return ANEUROEARA_BAD_DATA;
    }
    NeuroEaraResultCode status = notifyEaraJobBegin(tid, modelId, suggestedBoostValue);
    if (status == ANEUROEARA_NO_ERROR) mJobTimer.startTimer();
    return status;
}

NeuroEaraResultCode NeuroEaraManager::notifyEaraJobBegin(int tid,
        uint64_t modelId, vector<int32_t> &boostValue) {
    static earaNotifyJobBegin_fn jobBegin_fn = nullptr;
    void *fn = nullptr;
    if (getEaraLibHandle() != nullptr) {
        fn = dlsym(getEaraLibHandle(), "earaNotifyJobBegin");
    }

    if (fn == nullptr) {
        LOG(ERROR) << "Error : unable to open function earaNotifyJobBegin";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    jobBegin_fn = reinterpret_cast<earaNotifyJobBegin_fn>(fn);
    jobBegin_fn(tid, modelId, mPlanStructs, &mSuggestExecTime);
    return updateBoostValue(boostValue);
}

NeuroEaraResultCode NeuroEaraManager::updateBoostValue(vector<int32_t> &boostValue) {
    uint64_t boostValAge = static_cast<uint64_t>(mBoostValResetTimer.stopTimer());
    for (unsigned int stepId = 0; stepId < mPlanStructs.size(); stepId++) {
        if (mBoostValue[stepId][0] == 0 || mSuggestExecTime[stepId][0] == INVALID_EXEC_TIME
                || boostValAge > BOOST_VALUE_RESET_TIME_THRES_IN_NS) {
            mBoostValue[stepId][0] = getDefaultBoostValue();
        } else if (mSuggestExecTime[stepId][0] <= mModelMinExecTime[stepId][0]) {
            mBoostValue[stepId][0] = MAX_BOOST_VALUE;
        } else {
            // formula: minExecTime * 100 = suggestExecTime * new boost value
            mBoostValue[stepId][0] = std::max(
                    mModelMinExecTime[stepId][0] * 100 / mSuggestExecTime[stepId][0],
                            static_cast<uint64_t>(MIN_BOOST_VALUE));
        }
        boostValue[stepId] = mBoostValue[stepId][0];
        VLOG << "[updateBoostValue] step" << stepId
                << " @" << componentToString(static_cast<DeviceComponent>(mPlanStructs[stepId][0]))
                << ", minExecTime: " << mModelMinExecTime[stepId][0]
                << ", suggestExecTime: " << mSuggestExecTime[stepId][0]
                << ", boostValue: " << boostValue[stepId]
                << mModelTag;
    }
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::notifyExecutionEnd(int tid, uint64_t modelId,
        vector<bool> &hasFallback, vector<uint64_t> &execTime, vector<int32_t> &bandwidth) {

    std::lock_guard<std::mutex> guard(mMutex);

    uint64_t localTime = static_cast<uint64_t>(mJobTimer.stopTimer());
    for (unsigned int stepId = 0; stepId < mPlanStructs.size(); stepId++) {
        if (hasFallback[stepId]) {
            mModelExecTime[stepId][0] = INVALID_EXEC_TIME;
        } else {
            mModelExecTime[stepId][0] = (execTime[stepId] != 0) ? execTime[stepId] : localTime;
            if (mBoostValue[stepId][0] == MAX_BOOST_VALUE) {
                if (mModelMinExecTime[stepId][0] == 0) {
                    mModelMinExecTime[stepId][0] = mModelExecTime[stepId][0];
                } else {
                    mModelMinExecTime[stepId][0]
                            = (mModelMinExecTime[stepId][0] + mModelExecTime[stepId][0]) >> 1;
                }
            }
        }
        mBandwidth[stepId][0] = bandwidth[stepId];
        VLOG << "[notifyExecutionEnd] step" << stepId
                << " @" << componentToString(static_cast<DeviceComponent>(mPlanStructs[stepId][0]))
                << ", execTime:" << mModelExecTime[stepId][0]
                << ", minExecTime:" << mModelMinExecTime[stepId][0] << mModelTag;
    }
    mBoostValResetTimer.startTimer();
    return notifyEaraJobEnd(tid, modelId, true);
}

NeuroEaraResultCode NeuroEaraManager::notifyEaraJobEnd(int tid, uint64_t modelId,
        bool execSuccess) {
    static earaNotifyJobEnd_fn jobEnd_fn = nullptr;
    void *fn = nullptr;
    if (getEaraLibHandle() != nullptr) {
        fn = dlsym(getEaraLibHandle(), "earaNotifyJobEnd");
    }
    if (fn == nullptr) {
        LOG(ERROR) << "Error : unable to open function earaNotifyJobEnd";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    jobEnd_fn = reinterpret_cast<earaNotifyJobEnd_fn>(fn);
    jobEnd_fn(tid, modelId, &mJobPriority, execSuccess ? 0 : -1, mPlanStructs,
            mModelExecTime, mBandwidth, mBoostValue);
    VLOG << "[notifyEaraJobEnd] <= priority: " << mJobPriority
            << ", tid:" << tid << ", mid:" << modelId << mModelTag << '#';
    return ANEUROEARA_NO_ERROR;
}

int NeuroEaraManager::notifyExecutionFail(int tid, uint64_t modelId,
        uint32_t stepId) {
    VLOG << "[notifyExecutionFail]" << mModelTag << " fail at step " << stepId;
    return notifyEaraJobEnd(tid, modelId, false);
}

int NeuroEaraManager::getJobPriority(int *jobPriority) {
    *jobPriority = mJobPriority;
    return ANEUROEARA_NO_ERROR;
}
