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

#define LOG_TAG "MtkModelBuilder"

// Utils
#include "MtkOptions.h"

// Add-on
#include "MtkModelBuilder.h"
#include "Manager.h"
#include "MtkCompilationBuilder.h"

// Utils
#include "MtkOptions.h"
#include "MtkHalUtils.h"

// Definition
#include "MtkDefinition.h"

#include "TypeManager.h"

namespace android {
namespace nn {

MtkModelBuilder::MtkModelBuilder() {
    VLOG(MODEL) << "MtkModelBuilder::MtkModelBuilder";
    /// M: Sports Mode @{
    if (isSportsModeSupported()) {
        mSportsMode = new MtkSportsMode();
    }
    /// M: Sports Mode @}
}

MtkModelBuilder::~MtkModelBuilder() {
    /// M: Sports Mode @{
    if (mSportsMode != nullptr) {
        delete mSportsMode;
    }
    /// M: Sports Mode @}
}

int MtkModelBuilder::createCompilation(CompilationBuilder** compilation,
                                       const std::vector<std::shared_ptr<Device>>& devices,
                                       bool explicitDeviceList) {
    if (!mCompletedModel || mInvalidModel) {
        LOG(ERROR) << "ANeuralNetworksCompilation_create passed an unfinished or invalid model";
        *compilation = nullptr;
        return ANEURALNETWORKS_BAD_STATE;
    }
    *compilation = new (std::nothrow) MtkCompilationBuilder(this, devices, explicitDeviceList);
    return (*compilation ? ANEURALNETWORKS_NO_ERROR : ANEURALNETWORKS_OUT_OF_MEMORY);
}

bool MtkModelBuilder::getSupportedOperations(hidl_vec<bool>* outSupportedOperations,
                                             std::shared_ptr<Device> device) const {
    if (isCpuOnly()) {
        std::string cpuName = DeviceManager::getCpuDevice()->getName();
        std::string deviceName = device->getName();
        if (cpuName != deviceName) {
            uint32_t count = operationCount();
            std::vector<bool> supported(count, false);
            *outSupportedOperations = supported;
            NP_VLOG << "Cpu only is set, ignore " << device->getName();
            return true;
        }
    }

    if (isPreCheckOperandTypeSupported()) {
        // Check the operand type before ask hal service. Since we know about the capability our device
        // we can save some time if we know the hal service can't handle the input model.
        // If we found the device can't handle this model, retuen false.
        if (!betterThanCpu(device)) {
            uint32_t count = operationCount();
            std::vector<bool> supported(count, false);
            *outSupportedOperations = supported;
            NP_VLOG << "Skip get supported operation from " << device->getName();
            return true;
        }
    }

    // No special rule, use AOSP policy directly.
    return false;
}

bool MtkModelBuilder::betterThanCpu(std::shared_ptr<Device> device) const {
    bool ret = true;
    if (!isPreCheckOperandTypeSupported()) {
        return ret;
    }

    for (uint32_t i = 0; i < operationCount(); i++) {
        const Operation& op = getOperation(i);
        if (op.type == OperationType::OEM_OPERATION) {
            NP_VLOG << "Skip checking OEM OP";
            continue;
        }
        if (isExtensionOperationType(op.type)) {
            NP_VLOG << "Skip Extension Type";
            continue;
        }

        PerformanceInfo perf = getPerformanceInfo(device, i);
        // Currently, we have same capacity in powerUsage and execTime.
        // TODO: Use preference to get different capabity.
        const float perfVal = perf.execTime;
        if (perfVal > 1.0f) {
            NP_VLOG << device->getName() << " capability [" << perfVal << "] is worse than CPU";
            ret = false;
            break;
        }
    }

    NP_VLOG << "[betterThanCpu] Device " << device->getName() << " is better than CPU? "
            << (ret ? "Yes" : "No");
    return ret;
}

void MtkModelBuilder::sortIntoRunOrder() {
    if (ignoreSortRunOrder()) {
        VLOG(MODEL) << "Ignore sortIntoRunOrder";
        return;
    }
    ModelBuilder::sortIntoRunOrder();
}

// Partition Extension Start
int MtkModelBuilder::partitionTheWorkExt(const std::vector<std::shared_ptr<Device>>& devices,
        uint32_t preference, ExecutionPlan* plan, uint32_t partitionExtType) const {
    int ret = ANEURALNETWORKS_NO_ERROR;

    // No Preference, fallback to AOSP
    if (partitionExtType == ANEUROPILOT_PARTITIONING_EXTENSION_NONE) {
        return ModelBuilder::partitionTheWork(devices, preference, plan);
    }

    // MTK policy
    if (partitionExtType == ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION) {
        const size_t deviceCount = devices.size();
        const size_t operationCount = getOperations().size();

        VLOG(COMPILATION) << "MtkModelBuilder::partitionTheWorkExt: deviceCount = " << deviceCount
                          << ", operationCount = " << operationCount;

        // Figure out where each operation will best execute.
        // The value of the vector is the index in the devices vector.
        std::vector<int> bestDeviceForOperation(operationCount);
        NN_RETURN_IF_ERROR(
                findBestDeviceForEachOperation(preference, devices, &bestDeviceForOperation));

        // If one device will run all the operations, we don't need to split the work.
        if (operationCount == 1) {
            const int bestDeviceIndex = bestDeviceForOperation[0];
            VLOG(COMPILATION) << "MtkModelBuilder::partitionTheWorkExt: only one best device: "
                              << bestDeviceIndex << " = " << devices[bestDeviceIndex]->getName();

            plan->becomeSingleStep(devices[bestDeviceIndex], this);
            return plan->finish(this, preference);
        }

        // For each iteration of this loop, we'll create an execution step.
        for (size_t operationIndex = 0; operationIndex < operationCount; operationIndex++) {
            int deviceIndex = bestDeviceForOperation[operationIndex];
            // Assign as much as possible to this device.
            std::shared_ptr<ExecutionStep> step = plan->createNewStep(devices[deviceIndex]);
            int n = step->addOperation(operationIndex, *this);
            if (n != ANEURALNETWORKS_NO_ERROR) {
                LOG(ERROR) << "failed to add operation " << operationIndex << " to step";
                return n;
            }
        }

        ret = plan->finish(this, preference);
    } else {
        LOG(ERROR) << "MtkModelBuilder::partitionTheWorkExt: Unknown partition type "
                << partitionExtType << ", use AOSP policy as error handling.";
        return ModelBuilder::partitionTheWork(devices, preference, plan);
    }

    if (VLOG_IS_ON(COMPILATION)) {
        Model model;
        setHidlModel(&model);
        VLOG(COMPILATION) << "MtkModelBuilder::partitionTheWorkExt: original model: ";
        logModelToInfo(model);
        logModelToInfoExt(model);
        plan->dump();
    }
    return ret;
}
// Partition Extension End

// Debug Start
int MtkModelBuilder::checkSportsMode() {
    // Check if sports mode is needed
    if (mSportsMode != nullptr) {
        for (int i = 0; i < BENCHMARK_MODEL_COUNT; i++) {
            mSportsMode->compareModels(this, i);
        }
    }
    return ANEURALNETWORKS_NO_ERROR;
}
// Debug End

int MtkModelBuilder::replicateExtensionInfo(const char* extensionName,
            uint16_t typeWithinExtension, int32_t* type) {
    VLOG(MODEL) << "replicateExtensionInfo for " << extensionName
            << "'s type:" << typeWithinExtension;
    return TypeManager::get()->getExtensionType(extensionName, typeWithinExtension, type)
                   ? ANEURALNETWORKS_NO_ERROR
                   : ANEURALNETWORKS_BAD_DATA;
}

}  // namespace nn
}  // namespace android
