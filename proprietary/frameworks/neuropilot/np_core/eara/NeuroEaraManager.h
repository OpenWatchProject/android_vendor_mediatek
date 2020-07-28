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

#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cutils/properties.h>

#include <android-base/logging.h>
#include <android-base/macros.h>

#include "NeuroEara.h"
#include "NeuroEaraUtils.h"

using std::string;
using std::unordered_map;
using std::vector;

typedef struct EaraTimer {
    // express in nano second
    long long jobStartTime_;
    long long jobEndTime_;
    inline long long getSysTime() {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return (tp.tv_sec * 1e6 + tp.tv_usec) * 1e3;
    }
    inline bool isValid() {
        return (jobStartTime_ > -1);
    }
    inline void resetTime() {
        jobStartTime_ = -1;
        jobEndTime_ = -1;
    }
    inline long long startTimer() {
        jobEndTime_ = -1;
        jobStartTime_ = getSysTime();
        return jobStartTime_;
    }
    inline long long stopTimer() {
        jobEndTime_ = getSysTime();
        return jobEndTime_ - jobStartTime_;
    }
    EaraTimer() {
        jobStartTime_ = -1;
        jobEndTime_ = -1;
    }
} EaraTimer;

class NeuroEaraManager {
public:
    NeuroEaraManager(vector<string> &plan);
    ~NeuroEaraManager();
    int setExecutionPreference(ExecPreference preference);
    int getEaraUsage(string compStr, int *deviceLoading, int *bandwidth);
    int suggestOffload(string compStr, bool *execOffloadPlan);
    int configSuspendParams(string &compStr, int usageThreshold, uint32_t sleepTimeInMs);
    int suggestSuspendTime(string &compStr, uint32_t *time);
    int notifyExecutionBegin(int tid, uint64_t modelId, vector<int32_t> &boostValue);
    int notifyExecutionEnd(int tid, uint64_t modelId, vector<bool> &hasFallback,
            vector<uint64_t> &execTime, vector<int32_t> &bandwidth);
    int notifyExecutionFail(int tid, uint64_t modelId, uint32_t stepId);
    int getJobPriority(int *jobPriority);
private:
    std::mutex mMutex;
    const static int DEVICE_BOUND_THRESHOLD = 50;
    const static int DEVICE_BOUND_SLEEP_TIME_MS = 200;
    const static int BANDWIDTH_BOUND_THRESHOLD = 60;
    const static int BANDWIDTH_BOUND_SLEEP_TIME_MS = 50;
    const static int32_t MIN_BOOST_VALUE = 1;
    const static int32_t MAX_BOOST_VALUE = 100;
    const static uint64_t BOOST_VALUE_RESET_TIME_THRES_IN_NS = 1e9;

    string mModelTag;
    int32_t mJobPriority;
    ExecPreference mPreference;
    vector<vector<int32_t>> mPlanStructs;
    vector<vector<uint64_t>> mModelExecTime;
    vector<vector<uint64_t>> mModelMinExecTime;
    vector<vector<uint64_t>> mSuggestExecTime;
    vector<vector<int32_t>> mBandwidth;
    vector<vector<int32_t>> mBoostValue;
    unordered_map<DeviceComponent, int> mUsageThreshold;
    unordered_map<DeviceComponent, uint32_t> mSleepTimeInMs;
    EaraTimer mJobTimer;
    EaraTimer mBoostValResetTimer;

    inline const string componentToString(DeviceComponent comp) const {
        switch (comp) {
            case DeviceComponent::DEVICE_COMP_CPU:
                return string("CPU");
            case DeviceComponent::DEVICE_COMP_GPU:
                return string("GPU");
            case DeviceComponent::DEVICE_COMP_VPU:
                return string("VPU");
            case DeviceComponent::DEVICE_COMP_MDLA:
                return string("MDLA");
            case DeviceComponent::DEVICE_COMP_NEON:
                return string("NEON");
            default:
                break;
        }
        return string("UNKNOWN");
    }
    void setCompilation(vector<string> &plan);
    int getEaraUsage(DeviceComponent comp, int *deviceLoading, int *bandwidth);
    NeuroEaraResultCode notifyEaraJobBegin(int tid, uint64_t modelId,
            vector<int32_t> &suggestedBoostValue);
    NeuroEaraResultCode updateBoostValue(vector<int32_t> &boostValue);
    inline int32_t getDefaultBoostValue() {
        return (mPreference == ANEUROEARA_PREFER_LOW_POWER) ? MIN_BOOST_VALUE : MAX_BOOST_VALUE;
    }
    NeuroEaraResultCode notifyEaraJobEnd(int tid, uint64_t modelId, bool execSuccess);
};
