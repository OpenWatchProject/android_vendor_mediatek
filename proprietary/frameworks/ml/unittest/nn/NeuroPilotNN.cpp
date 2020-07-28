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

#include <string>
#include <vector>

// AOSP
#include "NeuralNetworks.h"

// NeuroPilot
#include "NeuroPilotPrivate.h"
#include "NeuroPilotShim.h"
#include "MtkDefinition.h"

// Unit tests
#include "NeuroPilotNN.h"
#include "utUtils.h"

// Models
#include "models/MtkTestModelFloat16AddConv.h"
#include "models/MtkTestModelFloat32AddConv.h"
#include "models/MtkTestModelQuant8AddAdd.h"

#ifndef NNTEST_ONLY_PUBLIC_API
// Set PartitionExt
#include "CompilationBuilder.h"
#include "ExecutionBuilder.h"
#include "ExecutionPlan.h"

#include "MtkSportsMode.h"
#include "Manager.h"

// Hal Utils
#include "MtkHalUtils.h"

#include "MtkOptions.h"
#endif

// Ion
#include <ion/ion.h>
#include <cutils/native_handle.h>
#include <hidlmemory/mapping.h>
#include <sys/mman.h>
#define ION_HEAP_TYPE 1024   // 1 << HEAP_TYPE_MULTI_MEDIA;

#define UNUSED(expr) (void) expr;

int utRunFloat32Model(bool relaxed) {
    MtkTestModelFloat32AddConv model;
    model.setRelaxed(relaxed);
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }
    return 1;
}

int utRunFloat16Model() {
    MtkTestModelFloat16AddConv model;
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }
    return 1;
}

int utRunFloat16ModelBurst() {
    MtkTestModelQuant8AddAdd model;
    if (model.runBurst() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }
    return 1;
}

int utRunQuant8Model() {
    MtkTestModelQuant8AddAdd model;
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }
    return 1;
}

// Partition Extension Start
int utSetPartitionExtType(int partition) {
    MtkTestModelFloat16AddConv model;
    model.setPartitionExt(partition);
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }
    return 1;
}

int utSetPartitionExtTypeInvalid(int partition) {
    MtkTestModelFloat16AddConv model;
    model.setPartitionExt(partition);
    if (model.run() == ANEURALNETWORKS_NO_ERROR) {
        printf("Set Invalid type %d, should not pass\n", partition);
        return 0;
    }
    return 1;
}

int utPartitionExtension(int partition) {
    MtkTestModelFloat16AddConv model;
    model.setPartitionExt(partition);
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }

#ifndef NNTEST_ONLY_PUBLIC_API
    android::nn::ExecutionBuilder* execution =
            reinterpret_cast<android::nn::ExecutionBuilder*>(model.getExecution());

    // Make sure we really devided graph into two sub-graphs.
    if (partition == ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION) {
        android::nn::CompilationBuilder* c =
                const_cast<android::nn::CompilationBuilder*>(execution->getCompilation());
        if (c->forTest_getExecutionPlan().forTest_getKind()
                != android::nn::ExecutionPlan::Kind::COMPOUND) {
            printf("Should be compound body\n");
            return 0;
        }

        const auto& steps = c->forTest_getExecutionPlan().forTest_compoundGetSteps();
        if (steps.size() != 2) {
            printf("steps.size %d is not 2\n", static_cast<int>(steps.size()));
            return 0;
        }
    }
#endif

    return 1;
}
// Partition Extension End

// Profiler
int utTestApiForProfiler(bool profilerOn) {
    MtkTestModelFloat16AddConv model;
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }

    if (profilerOn) {
        if (!ANeuroPilotUtilsPrivate_isProfilerSupported()) {
            printf("ANeuroPilotUtilsPrivate_isProfilerSupported error\n");
            return 0;
        }
    } else {
        if (ANeuroPilotUtilsPrivate_isProfilerSupported()) {
            printf("ANeuroPilotUtilsPrivate_isProfilerSupported error\n");
            return 0;
        }
    }

    ANeuralNetworksExecution* execution = model.getExecution();
    int ret = ANeuroPilotExecutionPrivate_setCurrentExecutionStep(execution, 0);
    if (profilerOn) {
        if (ret != ANEURALNETWORKS_NO_ERROR) {
            printf("failed to ANeuroPilotExecutionPrivate_setCurrentExecutionStep\n");
            return 0;
        }
    } else {
        if (ret == ANEURALNETWORKS_NO_ERROR) {
            printf("Should not pass ANeuroPilotExecutionPrivate_setCurrentExecutionStep\n");
            return 0;
        }
    }

    ret = ANeuroPilotExecutionPrivate_clearProfilerInfo(execution);
    if (profilerOn) {
        if (ret != ANEURALNETWORKS_NO_ERROR) {
            printf("failed to ANeuroPilotExecutionPrivate_clearProfilerInfo\n");
            return 0;
        }
    } else {
        if (ret == ANEURALNETWORKS_NO_ERROR) {
            printf("Should not pass ANeuroPilotExecutionPrivate_clearProfilerInfo\n");
            return 0;
        }
    }
    return 1;
}

int utProfiler(int partitionType) {
    MtkTestModelFloat16AddConv model;
    model.setPartitionExt(partitionType);
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }

    ANeuralNetworksExecution* execution = model.getExecution();
    std::vector<ProfilerResult> infos;
    if (ANeuroPilotExecutionWrapper_getProfilerInfo(execution, &infos)
            != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_getProfilerInfo\n");
        return 0;
    }

    int stepCount;
    for (auto info : infos) {
        printf("========== Profiler Start ==========\n");
        printf("Execution Step   : %d\n", info.step);
        printf("Execution Result : %d\n", info.success);
        printf("Device Name      : %s\n", info.deviceTime.devName.c_str());
        printf("Operations       : %s\n", info.deviceTime.opName.c_str());
        printf("Spent Time       : %f us\n", info.deviceTime.delta);
        printf("========== Profiler End ============\n");
        stepCount = info.step;
    }

    if (partitionType == ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION && stepCount != 1) {
        printf("failed to partition, step: %d\n", stepCount);
        return 0;
    }

    return 1;
}

// Operation Result
int utOperationResult(int partitionType) {
    MtkTestModelFloat16AddConv model;
    model.setPartitionExt(partitionType);
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }

    ANeuralNetworksExecution* execution = model.getExecution();
    std::vector<ProfilerResult> infos;
    if (ANeuroPilotExecutionWrapper_getProfilerInfo(execution, &infos)
            != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_getProfilerInfo\n");
        return 0;
    }

    uint32_t size;
    auto printOperationResult = [&](auto out) {
        for (uint32_t k = 0; k < size; k++) {
            printf("%s,", std::to_string(out[k]).c_str());
        }
        printf("\n");
    };

    for (auto info : infos) {
        printf("========== Profiler Start ==========\n");
        printf("Execution Step   : %d\n", info.step);
        printf("Execution Result : %d\n", info.success);
        for (auto& result : info.opResults) {
            size = result.size;
            printf("------ Output [0] ----------\n");
            printf("Operand Type     : %d\n", result.operandType);
            printf("Size             : %d\n", size);
            printf("Result           : ");
            printOperationResult((float *) result.buffer);
        }
        printf("========== Profiler End ============\n\n");
     }

    return 1;
}

int utCreateHidlMemory() {
#ifndef NNTEST_ONLY_PUBLIC_API
    android::nn::Memory memory;
    int n = memory.create(10);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        printf("Create memory error\n");
        return 0;
    }

    if (memory.getHidlMemory().name() != "ashmem") {
        printf("Hidl memory name is not ashmem.\n");
        return 0;
    }
#else
    printf("Only support for static test\n");
#endif
    return 1;
}

int utGetMemoryPointer() {
#ifndef NNTEST_ONLY_PUBLIC_API
    android::nn::Memory memory;
    int n = memory.create(10);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        printf("Create memory error\n");
        return 0;
    }

    uint8_t *data = nullptr;
    n = memory.getPointer(&data);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        printf("Get Pointer error\n");
        return 0;
    }
    uint8_t test[4] = {'t', 'e', 's', 't'};
    memcpy(data, test, 4);

    printf("%s\n", data);
    std::string str(data, data+4);
    if (str != "test") {
        printf("memory copy fail\n");
        return 0;
    }
#else
    printf("Only support for static test\n");
#endif
    return 1;
}

int utMapMemory() {
#ifndef NNTEST_ONLY_PUBLIC_API
    android::nn::Memory memory;
    int n = memory.create(10);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        printf("Create memory error\n");
        return 0;
    }

    const auto& hidlMemory = memory.getHidlMemory();
    uint8_t* buffer = nullptr;
    android::sp<IMemory> iMemory;
    iMemory = mapMemory(hidlMemory);
    if (iMemory == nullptr) {
        printf("Can't map ashmem shared memory.\n");
        return 0;
    }
    buffer = reinterpret_cast<uint8_t*>(static_cast<void*>(iMemory->getPointer()));
    if (buffer == nullptr) {
        printf("Can't access ashmem shared memory.\n");
        return 0;
    }

    uint8_t test[4] = {'t', 'e', 's', 't'};
    memcpy(buffer, test, 4);
    iMemory->commit();
    iMemory->update();

    uint8_t *data = nullptr;
    n = memory.getPointer(&data);

    std::string str(data, data + 4);
    printf("%s\n", str.c_str());
    if (str != "test") {
        printf("memory map fail\n");
        return 0;
    }
#else
    printf("Only support for static test\n");
#endif
    return 1;
}

int utGetMtkFeatureOption() {
#ifndef NNTEST_ONLY_PUBLIC_API
    property_set("debug.nn.mtk_nn.option", "");
    android::nn::forceOverwriteOptions();

    for (int i = 0; i <= 25; i++) {
        if (android::nn::isFeatureSupported(i)) {
            printf("Error: feature %d should not supported\n", i);
            property_set("debug.nn.mtk_nn.option", "0");
            android::nn::forceOverwriteOptions();
            return 0;
        }
    }

    property_set("debug.nn.mtk_nn.option", "A,C,Z");
    for (int i = 0; i <= 25; i++) {
        if (i == 0 || i == 2 || i == 25) {
            if (!android::nn::isFeatureSupported(i)) {
                printf("Error: feature %d should supported\n", i);
                property_set("debug.nn.mtk_nn.option", "0");
                android::nn::forceOverwriteOptions();
                return 0;
            }
        } else {
            if (android::nn::isFeatureSupported(i)) {
                printf("Error: feature %d should not supported\n", i);
                property_set("debug.nn.mtk_nn.option", "0");
                android::nn::forceOverwriteOptions();
                return 0;
            }
        }
    }

    property_set("debug.nn.mtk_nn.option", "a,c,z");
    for (int i = 0; i <= 25; i++) {
        if (i == 0 || i == 2 || i == 25) {
            if (!android::nn::isFeatureSupported(i)) {
                printf("Error: feature %d should supported\n", i);
                property_set("debug.nn.mtk_nn.option", "0");
                android::nn::forceOverwriteOptions();
                return 0;
            }
        } else {
            if (android::nn::isFeatureSupported(i)) {
                printf("Error: feature %d should not supported\n", i);
                property_set("debug.nn.mtk_nn.option", "0");
                android::nn::forceOverwriteOptions();
                return 0;
            }
        }
    }

    property_set("debug.nn.mtk_nn.option", " A, c,z ");
    for (int i = 0; i <= 25; i++) {
        if (i == 0 || i == 2 || i == 25) {
            if (!android::nn::isFeatureSupported(i)) {
                printf("Error: feature %d should supported\n", i);
                property_set("debug.nn.mtk_nn.option", "0");
                android::nn::forceOverwriteOptions();
                return 0;
            }
        } else {
            if (android::nn::isFeatureSupported(i)) {
                printf("Error: feature %d should not supported\n", i);
                property_set("debug.nn.mtk_nn.option", "0");
                android::nn::forceOverwriteOptions();
                return 0;
            }
        }
    }

    property_set("debug.nn.mtk_nn.option", " -5, dc,z~,,11,aa,fg   ,");
    for (int i = 0; i <= 25; i++) {
        if (android::nn::isFeatureSupported(i)) {
            printf("Error: feature %d should not supported\n", i);
            property_set("debug.nn.mtk_nn.option", "0");
            android::nn::forceOverwriteOptions();
            return 0;
        }
    }

    if (android::nn::isFeatureSupported(26)) {
        printf("Error: invaild feature 26 should not supported\n");
        property_set("debug.nn.mtk_nn.option", "0");
        android::nn::forceOverwriteOptions();
        return 0;
    }

    if (android::nn::isFeatureSupported(-1)) {
        printf("Error: invaild feature -1 should not supported\n");
        property_set("debug.nn.mtk_nn.option", "0");
        android::nn::forceOverwriteOptions();
        return 0;
    }
#else
    printf("Only support for static test\n");
#endif
    return 1;
}

int utTestMtkFeatureOption() {
    int opt1 = 1, opt2 = 1, opt3 = 1, opt4 = 1;
#ifndef NNTEST_ONLY_PUBLIC_API
    property_set("debug.nn.mtk_nn.option", "A,B,D,F");
    android::nn::forceOverwriteOptions();

    // A: ION
    if (!android::nn::isIonMemorySupported()) {
        printf("Error: Should support ion\n");
        opt1 = 0;
    }

    // B: PreCheck Operand Type
    if (!android::nn::isPreCheckOperandTypeSupported()) {
        printf("Error: Should support PreCheckOperandType\n");
        opt2 = 0;
    }

    // D: Sports Mode
    if (!android::nn::isSportsModeSupported()) {
        printf("Error: Should support Sports Mode\n");
        opt3 = 0;
    }

    // F: Sports Mode
    if (!android::nn::isSportsModeSupported()) {
        printf("Error: Should support Sports Mode\n");
        opt4 = 0;
    }

    property_set("debug.nn.mtk_nn.option", "");
    android::nn::forceOverwriteOptions();
#else
    printf("Only support for static test\n");
#endif
    return opt1 & opt2 & opt3 & opt4;
}

int utTestSportModeInitFlow() {
#ifdef NNTEST_ONLY_PUBLIC_API
    printf("Only support static test\n");
    return 1;
#else
    android::nn::MtkSportsMode *sports = new android::nn::MtkSportsMode();
    std::vector<int8_t> stages = sports->getStages();
    // Test 1
    if (stages.size() != BENCHMARK_MODEL_COUNT) {
        printf("Stage size %d not equal to benchmark count %d error! \n",
                stages.size(), BENCHMARK_MODEL_COUNT);
        delete sports;
        return 0;
    }

    // Test 2
    for (int i = 0; i < BENCHMARK_MODEL_COUNT; i++) {
        if (stages[i] != SPORTS_MODE_NOT_SURE) {
            printf("Stage init error\n");
            delete sports;
            return 0;
        }
    }

    // Test 3
    if (sports->getSportsMode() != SPORTS_MODE_NOT_SURE) {
        printf("Sport mode error\n");
        delete sports;
        return 0;
    }

    delete sports;
    return 1;
#endif
}

int utTestMtkExtensions() {
#ifndef NNTEST_ONLY_PUBLIC_API
    std::vector<Model::ExtensionNameAndPrefix> extensionNameToPrefixVec;
    extensionNameToPrefixVec.push_back({
            .name = android::nn::ANEUROPILOT_EXTENSION_NAME,
            .prefix = 1,
            });

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x00010000)
            != android::nn::ANEUROPILOT_OPERATION_ELU) {
        printf("getNeuroPilotExtensionType 0x00010000 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x00010001)
            != android::nn::ANEUROPILOT_OPERATION_MINPOOL) {
        printf("getNeuroPilotExtensionType 0x00010000 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x00010002)
            != android::nn::ANEUROPILOT_OPERATION_REQUANT) {
        printf("getNeuroPilotExtensionType 0x00010002 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x00010003)
            != android::nn::ANEUROPILOT_OPERATION_REVERSE) {
        printf("getNeuroPilotExtensionType 0x00010003 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x00010BB8)
            != android::nn::ANEUROPILOT_OPERATION_INJECTION) {
        printf("getNeuroPilotExtensionType 0x00010BB8 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x00010A03)
            != android::nn::ANEUROPILOT_OPERATION_INVAILD) {
        printf("getNeuroPilotExtensionType 0x00010A03 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x01010000)
            != android::nn::ANEUROPILOT_OPERATION_INVAILD) {
        printf("getNeuroPilotExtensionType 0x01010000 fail\n");
        return 0;
    }

    if (android::nn::getNeuroPilotExtensionType(extensionNameToPrefixVec, 0x10100000)
            != android::nn::ANEUROPILOT_OPERATION_INVAILD) {
        printf("getNeuroPilotExtensionType 0x01010000 fail\n");
        return 0;
    }

#else
    printf("Only Support in static test.\n");
#endif
    return 1;
}

int utSetCpuOnly(bool cpuOnly) {
    uint32_t numDevices = 0;
    if (ANeuralNetworks_getDeviceCount(&numDevices) != ANEURALNETWORKS_NO_ERROR) {
        printf("Get device count error\n");
        return 0;
    }
    if (numDevices <= 1) {
        printf("Not tested since only CPU is existed.\n");
        return 1;
    }

    MtkTestModelFloat16AddConv model;
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }

    ANeuralNetworksExecution* execution = model.getExecution();
    std::vector<ProfilerResult> infos;
    std::string name = "nnapi-reference";

#ifndef NNTEST_ONLY_PUBLIC_API
    name = android::nn::DeviceManager::getCpuDevice()->getName();
#endif
    if (ANeuroPilotExecutionWrapper_getProfilerInfo(execution, &infos)
            != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_getProfilerInfo\n");
        return 0;
    }

    if (infos.size() != 1) {
        printf("Should be simple body\n");
        return 0;
    }

    for (auto info : infos) {
        if (cpuOnly) {
            if (info.deviceTime.devName != name) {
                printf("Should be Cpu name! %s\n", info.deviceTime.devName.c_str());
                return 0;
            }
        } else {
            if (info.deviceTime.devName == name) {
                printf("Should not be Cpu name! %s\n", info.deviceTime.devName.c_str());
                return 0;
            }
        }
    }
    return 1;
}

int utGetDevice() {
    uint32_t numDevices = 0;
    if (ANeuralNetworks_getDeviceCount(&numDevices) != ANEURALNETWORKS_NO_ERROR) {
        printf("Get device count error\n");
        return 0;
    }

    for (uint32_t i = 0; i < numDevices; i++) {
        ANeuralNetworksDevice* device = nullptr;
        if (ANeuralNetworks_getDevice(i, &device) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get device error\n");
            return 0;
        }

        MtkTestModelFloat16AddConv model;
        model.addDevice(device);
        if (model.run() != ANEURALNETWORKS_NO_ERROR) {
            return 0;
        }

        bool supportedOps[99] = {false};
        if (ANeuralNetworksModel_getSupportedOperationsForDevices(
                model.getModel(), &device, 1, supportedOps) != ANEURALNETWORKS_NO_ERROR) {
                printf("Get supported operation error!\n");
            return 0;
        }

        for (int i = 0; i < model.getOperationCount(); i++) {
            if (supportedOps[i] != true) {
                printf("Supported[%d] is not true.\n", i);
                return 0;
            }
        }
    }
    return 1;
}

int utGetDeviceApis() {
    uint32_t numDevices = 0;
    if (ANeuralNetworks_getDeviceCount(&numDevices) != ANEURALNETWORKS_NO_ERROR) {
        printf("Get device count error\n");
        return 0;
    }

    std::vector<ANeuralNetworksDevice*> devices;
    for (uint32_t i = 0; i < numDevices; i++) {
        ANeuralNetworksDevice* device = nullptr;
        if (ANeuralNetworks_getDevice(i, &device) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get device error\n");            
            return 0;
        }

        const char* buffer = nullptr;
        if (ANeuralNetworksDevice_getName(device, &buffer) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get Name error!\n");
            return 0;
        }

        const char* version = nullptr;
        if (ANeuralNetworksDevice_getVersion(device, &version) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get version error!\n");
            return 0;
        }

        int64_t featureLevel = 0;
        if (ANeuralNetworksDevice_getFeatureLevel(device, &featureLevel) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get type error!\n");
            return 0;
        }

        int32_t type = 0;
        if (ANeuralNetworksDevice_getType(device, &type) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get type error!\n");
            return 0;
        }
    }
    return 1;
}

int utMeasureTiming() {
    uint32_t numDevices = 0;
    if (ANeuralNetworks_getDeviceCount(&numDevices) != ANEURALNETWORKS_NO_ERROR) {
        printf("Get device count error\n");
        return 0;
    }
    if (numDevices <= 1) {
        printf("Not tested since only CPU is existed.\n");
        return 1;
    }

    for (uint32_t i = 0; i < numDevices; i++) {
        ANeuralNetworksDevice* device = nullptr;
        if (ANeuralNetworks_getDevice(i, &device) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get device error\n");
            return 0;
        }

        MtkTestModelFloat16AddConv model;
        model.addDevice(device);
        model.setMeasure(true);
        if (model.run() != ANEURALNETWORKS_NO_ERROR) {
            return 0;
        }

        ANeuralNetworksExecution* execution = model.getExecution();
        uint64_t duration = 0;
        if (ANeuralNetworksExecution_getDuration(
                execution,
                ANEURALNETWORKS_DURATION_ON_HARDWARE,
                &duration) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get duration error\n");
            return 0;
        }
        if (duration == UINT64_MAX) {
            printf("Get duration on hardware error\n");
            return 0;
        }
        printf("Get duration on hardware %s ns\n", std::to_string(duration).c_str());

        if (ANeuralNetworksExecution_getDuration(
                execution,
                ANEURALNETWORKS_DURATION_IN_DRIVER,
                &duration) != ANEURALNETWORKS_NO_ERROR) {
            printf("Get duration error\n");
            return 0;
        }
        if (duration == UINT64_MAX) {
            printf("Get duration in driver error\n");
            return 0;
        }
        printf("Get duration in driver %s ns\n", std::to_string(duration).c_str());
    }
    return 1;
}

int utSetPreference(int prefer) {
    MtkTestModelFloat16AddConv model;
    model.setPreference(prefer);
    if (model.run() != ANEURALNETWORKS_NO_ERROR) {
        return 0;
    }
    return 1;
}

