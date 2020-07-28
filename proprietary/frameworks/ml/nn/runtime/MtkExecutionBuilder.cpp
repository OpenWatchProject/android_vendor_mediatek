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

#define LOG_TAG "MtkExecutionBuilder"

#include "MtkExecutionBuilder.h"
#include "MtkCompilationBuilder.h"
#include "MtkModelBuilder.h"

#include "MtkOptions.h"

// Profiler
#include "MtkProfiler.h"

// Version
#include "MtkUtils.h"

namespace android {
namespace nn {
MtkExecutionBuilder::MtkExecutionBuilder(const CompilationBuilder* compilation) :
        ExecutionBuilder(compilation) {
    VLOG(EXECUTION) << "MtkExecutionBuilder::MtkExecutionBuilder";
    VLOG(EXECUTION) << "NeuroPilot Runtime Hash: " << get_nn_hash();
}

MtkExecutionBuilder::~MtkExecutionBuilder() {
    clearProfilerInfo();
}

// Profiler Start
int MtkExecutionBuilder::startProfile(const char* device) {
    VLOG(EXECUTION) << "Start Profile: this = " << this;

    size_t count = mProfilerData.size();
    mProfilerData.resize(count+1);

    auto& data = mProfilerData[count];
    data.step = mCurrentStep;
    if (device != NULL) {
        data.deviceTime.devName = device;
    } else {
        data.deviceTime.devName = "unKnown";
    }
    data.deviceTime.delta = getCurrentTimeUs();

    VLOG(EXECUTION) << "startProfile end";
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkExecutionBuilder::stopProfile(StepExecutor* executor, Request *request, int err) {
    const ModelBuilder* model = nullptr;
    MemoryTracker memories;
    std::vector<ModelArgumentInfo> outputs;
    double currentTime = getCurrentTimeUs();

    VLOG(EXECUTION) << "Stop Profile: this = " << this;

    const ExecutionPlan *plan = getPlan();
    if (plan->forTest_getKind() == ExecutionPlan::Kind::SIMPLE) {
        VLOG(EXECUTION) << "SIMPLE BODY";

        // Got Model
        model = getModel();

        // For Execution Result
        outputs = getOutputs();
        memories = getMemories();
    } else if (plan->forTest_getKind() == ExecutionPlan::Kind::COMPOUND) {
        VLOG(EXECUTION) << "COMPOUND BODY";

        const auto& steps = plan->forTest_compoundGetSteps();
        // Got Model
        model = steps[mCurrentStep]->getSubModel();

        // For Execution Result
        outputs = executor->getOutputs();
        memories = executor->getMemories();
    } else {
        LOG(ERROR) << "Error: unknown body";
    }

    size_t count = mProfilerData.size();
    auto& data = mProfilerData[count-1];
    if (err == ANEURALNETWORKS_NO_ERROR) {
        data.success = true;
    } else {
        data.success = false;
    }

    // Model
    Model hidlModel;
    if (model == nullptr) {
        LOG(ERROR) << "model is nullptr";
        return ANEURALNETWORKS_UNEXPECTED_NULL;
    }
    model->setHidlModel(&hidlModel);

    // Execution Time
    prepareExecutionTime(model, currentTime);

    // Execution Result
    prepareExecutionResult(model, memories, outputs);

    std::vector<std::string> outputOperand;
    outputOperand.resize(data.opResults.size());

    for (uint32_t i = 0; i < data.opResults.size(); i++) {
        outputOperand[i] = toString(model->getOutputOperand(i));
    }
    dumpProfilerData(mProfilerData, request, hidlModel, outputOperand);

    return ANEURALNETWORKS_NO_ERROR;
}

int MtkExecutionBuilder::clearProfilerInfo() {
    mCurrentStep = 0;
    for (auto& data : mProfilerData) {
        for (auto& result : data.opResults) {
            if (result.buffer) {
                delete result.buffer;
            }
        }
        data.opResults.clear();
    }
    mProfilerData.clear();
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkExecutionBuilder::getProfilerResult(std::vector<ProfilerResult> *result) {
    *result = mProfilerData;
    return ANEURALNETWORKS_NO_ERROR;
}

void MtkExecutionBuilder::prepareExecutionTime(const ModelBuilder* model, double currentTime) {
    size_t count = mProfilerData.size();
    auto& data = mProfilerData[count-1];

    VLOG(EXECUTION) << "saveExecutionTime";

    // Execution time
    auto& operations = model->getOperations();
    std::string ops = "";
    uint32_t opCount = operations.size();
    for (uint32_t i = 0; i < opCount; i++) {
        ops += getOperationName(operations[i].type);
        if (i < opCount - 1) {
            ops += ":";
        }
    }
    data.deviceTime.opName = ops;

    if (data.success) {
        data.deviceTime.delta = currentTime - data.deviceTime.delta;
    } else {
        data.deviceTime.delta = 0;
    }
}

void MtkExecutionBuilder::prepareExecutionResult(const ModelBuilder* model,
                                                 const MemoryTracker memories,
                                                 std::vector<ModelArgumentInfo> outputs) {
    size_t count = mProfilerData.size();
    auto& data = mProfilerData[count-1];

    if (!data.success) {
        VLOG(EXECUTION) << "Previous operation is failed, don't store";
        return;
    }

    VLOG(EXECUTION) << "prepareExecutionResult";

    for (uint32_t i = 0; i < model->outputCount(); i++) {
        Operand operand = model->getOutputOperand(i);
        ModelArgumentInfo arg = outputs[i];

        ExecResult result;
        result.operandType = static_cast<int32_t>(operand.type);

        if (copyArgumentBuffer(&result.buffer, memories, arg) != ANEURALNETWORKS_NO_ERROR) {
            result.size = 0;
            continue;
        }

        switch (operand.type) {
            case OperandType::TENSOR_FLOAT32:
            case OperandType::FLOAT32: {
                result.size = arg.locationAndLength.length / sizeof(float);
            } break;
            case OperandType::TENSOR_FLOAT16:
            case OperandType::FLOAT16:
            case OperandType::TENSOR_QUANT16_SYMM:
            case OperandType::TENSOR_QUANT16_ASYMM: {
                result.size = arg.locationAndLength.length / sizeof(uint16_t);
            } break;
            case OperandType::TENSOR_INT32:
            case OperandType::INT32: {
                result.size = arg.locationAndLength.length / sizeof(int);
            } break;
            case OperandType::UINT32: {
                result.size = arg.locationAndLength.length / sizeof(uint32_t);
            } break;
            case OperandType::TENSOR_QUANT8_ASYMM:
            case OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL:
            case OperandType::TENSOR_QUANT8_SYMM:
            case OperandType::BOOL:
            case OperandType::TENSOR_BOOL8: {
                result.size = arg.locationAndLength.length / sizeof(uint8_t);
            } break;
            default:
                result.size = arg.locationAndLength.length / sizeof(uint8_t);
                break;
        }
        data.opResults.push_back(result);
    }
}

int MtkExecutionBuilder::copyArgumentBuffer(uint8_t** buffers,
        const MemoryTracker memories, ModelArgumentInfo arg) {
    if (arg.state == ModelArgumentInfo::POINTER) {
        LOG(DEBUG) << "POINTER";
        *buffers = new uint8_t[arg.locationAndLength.length];
        if (!*buffers) {
            LOG(ERROR) << "out of memory";
            return ANEURALNETWORKS_UNEXPECTED_NULL;
        } else {
            memcpy(*buffers, reinterpret_cast<uint8_t*>(arg.buffer),
                    arg.locationAndLength.length);
        }
    } else if (arg.state == ModelArgumentInfo::MEMORY) {
        LOG(DEBUG) << "MEMORY";
        if (arg.locationAndLength.poolIndex >= memories.size()) {
            LOG(ERROR) << "illegal arg.locationAndLength.poolIndex("
                    << arg.locationAndLength.poolIndex
                    << ") >= memories.size()(" << memories.size() << ")";
            return ANEURALNETWORKS_BAD_DATA;
        }
        uint8_t* buffer = nullptr;
        const Memory *mem = memories[arg.locationAndLength.poolIndex];
        int ret = mem->getPointer(&buffer);
        if (ret != ANEURALNETWORKS_NO_ERROR) {
            LOG(ERROR) << "failed to get pointer from memory";
            return ret;
        }
        *buffers = new uint8_t[arg.locationAndLength.length];
        if (!*buffers) {
            LOG(ERROR) << "out of memory";
            return ANEURALNETWORKS_UNEXPECTED_NULL;
        } else {
            memcpy(*buffers, &buffer[arg.locationAndLength.offset],
                    arg.locationAndLength.length);
        }
    }
    return ANEURALNETWORKS_NO_ERROR;
}
/// M: Profiler @}

/// M: Sports Mode @{
int MtkExecutionBuilder::setInput(uint32_t index, const ANeuralNetworksOperandType* type,
        const void* buffer, size_t length) {
    int ret = ExecutionBuilder::setInput(index, type, buffer, length);
    if (isSportsModeSupported()) {
        const_cast<MtkModelBuilder*>(reinterpret_cast<const MtkModelBuilder*>(getModel()))
                ->getMtkSportsMode()->setInput(index, buffer, length);
    }
    return ret;
}

int MtkExecutionBuilder::setInputFromMemory(uint32_t index, const ANeuralNetworksOperandType* type,
        const Memory* memory, size_t offset, size_t length) {
    int ret = ExecutionBuilder::setInputFromMemory(index, type, memory, offset, length);
    if (isSportsModeSupported()) {
        const_cast<MtkModelBuilder*>(reinterpret_cast<const MtkModelBuilder*>(getModel()))
                ->getMtkSportsMode()->setInputFromMemory(index, memory, offset, length);
    }
    return ret;
}
/// M: Sports Mode @}

/// M: Eara Qos @{
int MtkExecutionBuilder::addExecutionExtraParam(MemoryTracker *memories, Memory* memory) {
    uint8_t prefix[4] = {'t', 'i', 'd', ':'};
    int32_t tid = gettid();
    int total = 4 + sizeof(int32_t);

    NP_VLOG << "addExecutionExtraParam, tid: " << tid;

    memory->create(total);
    uint8_t* data = nullptr;
    int n = memory->getPointer(&data);
    if (n != ANEURALNETWORKS_NO_ERROR) {
        LOG(ERROR) << "addExecutionExtraParam: getPointer error";
        return n;
    }

    // Copy Prefix to first four bytes.
    memcpy(data, prefix, 4);

    // Copy tid to next four bytes
    memcpy(data + 4, &tid, sizeof(int32_t));
    memories->add(memory);
    return ANEURALNETWORKS_NO_ERROR;
}
/// M: Eara Qos @}

}  // namespace nn
}  // namespace android
