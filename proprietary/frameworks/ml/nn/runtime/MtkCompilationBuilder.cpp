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

#define LOG_TAG "MtkCompilationBuilder"

// Add on
#include "Manager.h"
#include "MtkModelBuilder.h"
#include "MtkCompilationBuilder.h"
#include "MtkExecutionBuilder.h"

// Utils
#include "MtkOptions.h"

#include <cutils/properties.h>

namespace android {
namespace nn {

MtkCompilationBuilder::MtkCompilationBuilder(const ModelBuilder* model,
        const std::vector<std::shared_ptr<Device>>& devices, bool explicitDeviceList):
        CompilationBuilder(model, devices, explicitDeviceList) {
    VLOG(COMPILATION) << "MtkCompilationBuilder::MtkCompilationBuilder";
}

MtkCompilationBuilder::~MtkCompilationBuilder() {}

int MtkCompilationBuilder::createExecution(ExecutionBuilder **execution) {
    if (!mFinished) {
        LOG(ERROR) << "ANeuroPilotExecution_create passed an unfinished compilation";
        *execution = nullptr;
        return ANEURALNETWORKS_BAD_STATE;
    }
    if (!mPlan.isValid()) {
        LOG(ERROR) << "ANeuroPilotExecution_create passed an invalid compilation";
        *execution = nullptr;
        return ANEURALNETWORKS_BAD_STATE;
    }
    *execution = new (std::nothrow) MtkExecutionBuilder(this);
    return (*execution ? ANEURALNETWORKS_NO_ERROR : ANEURALNETWORKS_OUT_OF_MEMORY);
}

// M: Partition Extension Start
int MtkCompilationBuilder::setPartitionExtType(uint32_t type) {
    if (mFinished) {
        LOG(ERROR) << "setPartitionExtType should not be executed when compilation is finished.";
        return ANEURALNETWORKS_BAD_STATE;
    }
    if (type > ANEUROPILOT_PARTITIONING_EXTENSION_MAX) {
        LOG(ERROR) << "setPartitionExtType unknown type.";
        return ANEURALNETWORKS_BAD_DATA;
    }

    mPartitionExtType = type;
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkCompilationBuilder::getPartitionExtType() const {
    // For Debug: Set partition type by system property
    char type[PROPERTY_VALUE_MAX];
    if (getPartitionExtTypeFromProperty(type)) {
        VLOG(COMPILATION) << "set partition by property " << type;
        if (strcmp(type, "OPERATION") == 0) {
            return ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION;
        }
    }
    return mPartitionExtType;
}

// Override for putting partition extension type into partition the work extension
int MtkCompilationBuilder::finish() {
    if (mFinished) {
        LOG(ERROR) << "ANeuralNetworksCompilation_finish called more than once";
        return ANEURALNETWORKS_BAD_STATE;
    }
    // TODO validate the rest

    mFinished = true;
    if (mIsCacheInfoProvided) {
        mPlan.setCaching(&mCacheDir, mToken);
    }
    if (mPartitioning) {
        int n = reinterpret_cast<const MtkModelBuilder*>(mModel)
                ->partitionTheWorkExt(mDevices, mPreference, &mPlan, getPartitionExtType());
        switch (n) {
            case ANEURALNETWORKS_NO_ERROR:
                return n;
            case ANEURALNETWORKS_UNEXPECTED_NULL:
            case ANEURALNETWORKS_BAD_DATA:
                // The two error codes above should only be used for errors in the user's
                // request. In case of a user error, we won't try any fallback.
                // TODO: Document this in NeuralNetworks.h and in the HAL. Make sure
                // driver writers know which code they can return.
                return n;
            default:
                // The error might be recoverable. Return the error only if falling back
                // is not allowed.
                if (!DeviceManager::partitioningAllowsFallback(mPartitioning)) {
                    return n;
                }
                if (mModel->hasOEMOperation()) {
                    LOG(ERROR) << "Cannot fall back to CPU because of an OEM operation";
                    return n;
                }
                if (mModel->hasExtensionOperation()) {
                    LOG(ERROR) << "Cannot fall back to CPU because of an extension operation";
                    return n;
                }
                break;
        }
    }

    // Fallback to CPU
    VLOG(COMPILATION) << "CompilationBuilder::finish with CPU fallback";
    mPlan.reset();
    mPlan.becomeSingleStep(DeviceManager::getCpuDevice(), mModel);
    return mPlan.finish(mModel, mPreference);
}
// Partition Extension End

}  // namespace nn
}  // namespace android
