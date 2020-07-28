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

#define LOG_TAG "MtkOptions"

#include "MtkOptions.h"

#include <cutils/properties.h>
#include <android-base/properties.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

namespace android {
namespace nn {

static bool sOptions[26] = {false};
static bool sReadProperty = false;

void initOptions(const char* key) {
    const std::string vOptionSetting = android::base::GetProperty(key, "");
    if (vOptionSetting.empty()) {
        LOG(INFO) << "Empty property";
        return;
    }

    std::vector<std::string> elements = android::base::Split(vOptionSetting, " ,");
    for (const auto& elem : elements) {
        if (elem.length() != 1) {
            LOG(ERROR) << "Unknown property: " << elem;
            continue;
        }

        int index = -1;
        char c = elem.at(0);
        index = tolower(c) - 'a';
        if (index < 0 || index > 25) {
            LOG(ERROR) << "Invaild index: " << index;
            continue;
        }

        NP_VLOG << "Property Index: " << index;
        sOptions[index] = true;
    }
}

bool overwriteOptions() {
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.mtk_nn.option", var, "") != 0) {
        LOG(INFO) << "For debug: feature support by debug proerty: " << var;

        // Reset old data first.
        for (int i = 0; i <= 25; i++) {
            sOptions[i] = false;
        }
        initOptions("debug.nn.mtk_nn.option");
        return true;
    }
    return false;
}

bool forceOverwriteOptions() {
    char var[PROPERTY_VALUE_MAX];
    LOG(INFO) << "For debug: feature support by debug proerty: " << var;

    // Reset old data first.
    for (int i = 0; i <= 25; i++) {
        sOptions[i] = false;
    }
    initOptions("debug.nn.mtk_nn.option");
    return true;
}

bool isFeatureSupported(int feature) {
    if (feature < 0 || feature > 25) {
        LOG(ERROR) << "Unknown feature: " << feature;
        return false;
    }

#ifdef NN_DEBUGGABLE
    if (overwriteOptions()) {
        return sOptions[feature];
    }
#endif

    if (!sReadProperty) {
        initOptions("ro.vendor.mtk_nn.option");
        sReadProperty = true;
    }

    return sOptions[feature];
}

// Utils
bool isNeuroPilotVLogSupported() {
    bool ret = false;
    if (property_get_bool("debug.neuropilot.vlog", false)) {
        ret = true;
    }
    return ret;
}

bool isFallbackCpuSupported() {
    bool ret = true;
#ifdef NN_DEBUGGABLE
    if (!property_get_bool("debug.nn.fallback.cpu.supported", true)) {
        ret = false;
    }
    NP_VLOG << "isFallbackCpuSupported : " << ret;
#endif
    return ret;
}

bool isCpuOnly() {
    bool ret = false;
#ifdef NN_DEBUGGABLE
    if (property_get_bool("debug.np.cpu.only", false)) {
        ret = true;
    }
    NP_VLOG << "isCpuOnly : " << ret;
#endif
    return ret;
}

// Profiler
bool isProfilerSupported() {
    bool ret = false;
    if (property_get_bool("debug.nn.profiler.supported", false)) {
        ret = true;
    }
    LOG(DEBUG) << "isProfilerSupported : " << ret;
    return ret;
}

// Debug
bool ignoreSortRunOrder() {
    bool ret = false;
    if (property_get_bool("debug.nn.ignore.sort.order", false)) {
        ret = true;
    }
    LOG(DEBUG) << "ignoreSortRunOrder : " << ret;
    return ret;
}

// Partition Extension
bool getPartitionExtTypeFromProperty(char* type) {
    bool ret = false;
    if (property_get("debug.nn.partitionext.type", type, "") != 0) {
        ret = true;
    }
    LOG(DEBUG) << "getPartitionExtTypeFromProperty : " << type;
    return ret;
}

// Ion
bool isIonMemorySupported() {
    bool ret = false;
    if (isFeatureSupported(ANEUROPILOT_FEATURE_ION)) {
        ret = true;
    }
#ifdef NN_DEBUGGABLE
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.ion.supported", var, "") != 0) {
        ret = atoi(var) == 1 ? true : false;
        LOG(INFO) << "Override isIonMemorySupported by debug property";
    }
#endif
    NP_VLOG << "isIonMemorySupported : " << ret;
    return ret;
}

// Performance
bool isPreCheckOperandTypeSupported() {
    bool ret = false;
    if (isFeatureSupported(ANEUROPILOT_FEATURE_ENHANCE_PERFORMANCE)) {
        ret = true;
    }
#ifdef NN_DEBUGGABLE
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.precheck.operand.supported", var, "") != 0) {
        ret = atoi(var) == 1 ? true : false;
        LOG(INFO) << "Override isPreCheckOperandTypeSupported by debug property";
    }
#endif
    NP_VLOG << "isPreCheckOperandTypeSupported : " << ret;
    return ret;
}

// Sports Mode
bool isSportsModeSupported() {
    bool ret = false;
    if (isFeatureSupported(ANEUROPILOT_FEATURE_SPORTS_MODE)) {
        ret = true;
    }
#ifdef NN_DEBUGGABLE
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.sportsmode.supported", var, "") != 0) {
        ret = atoi(var) == 1 ? true : false;
        LOG(INFO) << "Override isSportsModeSupported by debug property";
    }
#endif
    NP_VLOG << "isSportsModeSupported: " << ret;
    return ret;
}

// EARA Qos
bool isEaraQosSupported() {
    bool ret = false;
    if (isFeatureSupported(ANEUROPILOT_FEATURE_EARA_QOS)) {
        ret = true;
    }
#ifdef NN_DEBUGGABLE
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.earaqos.supported", var, "") != 0) {
        ret = atoi(var) == 1 ? true : false;
        LOG(INFO) << "Override isEaraQosSupported by debug property";
    }
#endif
    NP_VLOG << "isEaraQosSupported: " << ret;
    return ret;
}

// NeuroPilot
bool isNeuroPilotSupported() {
    bool ret = false;
    if (isFeatureSupported(ANEUROPILOT_FEATURE_NEUROPILOT_ALL)) {
        ret = true;
    }
#ifdef NN_DEBUGGABLE
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.neuropilot.supported", var, "") != 0) {
        ret = atoi(var) == 1 ? true : false;
        LOG(INFO) << "Override isNeuroPilotSupported by debug property";
    }
#endif
    NP_VLOG << "isNeuroPilotSupported: " << ret;
    return ret;
}

}  // namespace nn
}  // namespace android
