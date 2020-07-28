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

#define LOG_TAG "NeuroEara"
#include "NeuroEara.h"
#include "NeuroEaraManager.h"
#include <android-base/properties.h>

static int sEaraSupport = -1;
bool isEaraQosSupported() {
    if (sEaraSupport != -1) {
        LOG(ERROR) << "isEaraQosSupported: " << sEaraSupport;
        return sEaraSupport ? true : false;
    }
    sEaraSupport = 1;
    void *handle = dlopen("libperfctl_vendor.so", RTLD_LAZY | RTLD_LOCAL);
    if (handle == NULL) {
        // without FPSGO, no need to check any more
        sEaraSupport = 0;
        return false;
    }
    dlclose(handle);
#ifdef NN_DEBUGGABLE
    char var[PROPERTY_VALUE_MAX];
    if (property_get("debug.nn.earaqos.supported", var, "1") != 0) {
        sEaraSupport = atoi(var);
    }
#endif
    return (sEaraSupport == 0) ? false : true;
}

int ANeuroEara_isEaraQosSupported(bool *isEaraSupport) {
    *isEaraSupport = isEaraQosSupported();
    return ANEUROEARA_NO_ERROR;
}

int ANeuroEara_create(ANeuroEara **eara, vector<string> &plan) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    NeuroEaraManager *earaMgr = new (std::nothrow) NeuroEaraManager(plan);
    if (earaMgr == nullptr) {
        *eara = nullptr;
        return ANEUROEARA_OUT_OF_MEMORY;
    }
    (*eara) = reinterpret_cast<ANeuroEara *>(earaMgr);
    return ANEUROEARA_NO_ERROR;
}

int ANeuroEara_setExecutionPreference(ANeuroEara *eara,
        ExecPreference preference) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_setExecutionPreference passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->setExecutionPreference(preference);
}

int ANeuroEara_getEaraUsage(ANeuroEara *eara, string comp,
        int *deviceUsage, int *bandwidth) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_getEaraUsage passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->getEaraUsage(comp, deviceUsage, bandwidth);
}

int ANeuroEara_suggestOffload(ANeuroEara *eara, string comp, bool *execOffloadPlan) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_suggestOffload passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->suggestOffload(comp, execOffloadPlan);
}

int ANeuroEara_configSuspendParams(ANeuroEara *eara, string comp,
        int usageThreshold, uint32_t sleepTimeInMs) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_configSuspendParams passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->configSuspendParams(comp, usageThreshold, sleepTimeInMs);
}

int ANeuroEara_suggestSuspendTime(ANeuroEara *eara, string comp,
        uint32_t *time) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_suggestSuspendTime passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->suggestSuspendTime(comp, time);
}

int ANeuroEara_notifyExecutionBegin(ANeuroEara *eara, int tid,
        uint64_t modelId, vector<int32_t> &suggestedBoostValue) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_notifyExecutionBegin passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->notifyExecutionBegin(tid, modelId, suggestedBoostValue);
}

int ANeuroEara_notifyExecutionEnd(ANeuroEara *eara, int tid, uint64_t modelId,
        vector<bool> &hasFallback, vector<uint64_t> &execTime, vector<int32_t> &bandwidth) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_notifyExecutionEnd passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->notifyExecutionEnd(tid, modelId, hasFallback, execTime, bandwidth);
}

int ANeuroEara_notifyExecutionFail(ANeuroEara *eara, int tid,
        uint64_t modelId, uint32_t stepId) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_notifyExecutionFail passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->notifyExecutionFail(tid, modelId, stepId);
}

int ANeuroEara_getJobPriority(ANeuroEara *eara, int *jobPriority) {
    if (!isEaraQosSupported()) {
        return ANEUROEARA_NOT_SUPPORT;
    }
    if (!eara) {
        LOG(ERROR) << "ANeuroEara_getJobPriority passed a nullptr";
        return ANEUROEARA_UNEXPECTED_NULL;
    }
    NeuroEaraManager *e = reinterpret_cast<NeuroEaraManager *>(eara);
    return e->getJobPriority(jobPriority);
}

void ANeuroEara_delete(ANeuroEara *eara) {
    if (!isEaraQosSupported()) {
        return;
    }
    NeuroEaraManager *m = reinterpret_cast<NeuroEaraManager *>(eara);
    delete m;
}
