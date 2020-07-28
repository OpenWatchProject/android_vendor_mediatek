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

#define LOG_TAG "NeuroPilot"

#include "MtkModelBuilder.h"
#include "MtkCompilationBuilder.h"
#include "MtkExecutionBuilder.h"

// Utils
#include "NeuroPilot.h"
#include "MtkOptions.h"

using namespace android::nn;

int ANeuroPilotModel_create(ANeuralNetworksModel** model) {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return ANEURALNETWORKS_BAD_STATE;
    }
    initVLogMask();
    if (!model) {
        LOG(ERROR) << "ANeuralNetworksModel_create passed a nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }

    ModelBuilder* m = new (std::nothrow) MtkModelBuilder();
    if (m == nullptr) {
        *model = nullptr;
        return ANEURALNETWORKS_OUT_OF_MEMORY;
    }
    *model = reinterpret_cast<ANeuralNetworksModel*>(m);
    return ANEURALNETWORKS_NO_ERROR;
}

/// Partition Extension @{
int ANeuroPilotCompilation_setPartitionExtType(ANeuralNetworksCompilation* compilation, uint32_t type) {
    MtkCompilationBuilder* c = reinterpret_cast<MtkCompilationBuilder*>(compilation);
    return c->setPartitionExtType(type);
}
/// Partition Extension @}

/// Profiler @{
int ANeuroPilotExecution_setCurrentExecutionStep(
        ANeuralNetworksExecution *execution, uint32_t step) {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (!isProfilerSupported()) {
        NP_VLOG << "SetCurrentExecutionStep: Profiler not support";
        return ANEURALNETWORKS_BAD_STATE;
    }
    MtkExecutionBuilder* e = reinterpret_cast<MtkExecutionBuilder*>(execution);
    e->setCurrentExecutionStep(step);
    return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotExecution_startProfile(ANeuralNetworksStepExecutor *executor, const char* device) {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (!isProfilerSupported()) {
        NP_VLOG << "StartProfile: Profiler not support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    MtkExecutionBuilder *execution = reinterpret_cast<MtkExecutionBuilder*>(
            reinterpret_cast<StepExecutor*>(executor)->getExecutionBuilder());

    return execution->startProfile(device);
}

int ANeuroPilotExecution_stopProfile(ANeuralNetworksStepExecutor *executor,
        ANeuralNetworksRequest *r, int err) {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (!isProfilerSupported()) {
        NP_VLOG << "StopProfile: Profiler not support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    StepExecutor* stepExecutor = reinterpret_cast<StepExecutor*>(executor);
    Request* request = reinterpret_cast<Request *>(r);
    MtkExecutionBuilder *execution = reinterpret_cast<MtkExecutionBuilder*>(
            stepExecutor->getExecutionBuilder());

    return execution->stopProfile(stepExecutor, request, err);
}

int ANeuroPilotExecution_clearProfilerInfo(ANeuralNetworksExecution *execution) {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (!isProfilerSupported()) {
        NP_VLOG << "ClearProfilerInfo: Profiler not support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    MtkExecutionBuilder* e = reinterpret_cast<MtkExecutionBuilder*>(execution);
    return e->clearProfilerInfo();
}

bool ANeuroPilotUtils_isProfilerSupported() {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return false;
    }
    return isProfilerSupported();
}

int ANeuroPilotExecution_getProfilerInfo(
        ANeuralNetworksExecution *execution, std::vector<ProfilerResult> *result) {
    if (!isProfilerSupported()) {
        NP_VLOG << "GetProfilerInfo: Profiler not support";
        return ANEURALNETWORKS_BAD_STATE;
    }

    return reinterpret_cast<MtkExecutionBuilder*>(execution)->getProfilerResult(result);
}
/// @}

/// M: Utils @{
bool ANeuroPilotUtils_forbidCpuExecution() {
    if (!isNeuroPilotSupported()) {
        NP_VLOG << "NeuroPilot Not Support";
        return false;
    }

    bool ret = false;
    if (!isFallbackCpuSupported()) {
        LOG(INFO) << "Disable CPU fallback";
        printf("Disable CPU fallback\n");
        ret = true;
    }
    return ret;
}
/// @}

