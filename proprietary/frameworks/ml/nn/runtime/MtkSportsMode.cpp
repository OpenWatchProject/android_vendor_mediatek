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

#define LOG_TAG "MtkSportsMode"

#include "MtkSportsMode.h"
#include "MtkOptions.h"

#include "MtkModelBuilder.h"
#include "MtkExecutionBuilder.h"

#include <utils/String16.h>
#include <binder/Parcel.h>

namespace android {
namespace nn {

// Dim
const std::vector<std::vector<std::vector<uint32_t>>> kInputOperands {
        {{1, 224, 224, 3}},                // AI Benchmark Quant
        {{1, 224, 224, 3}},                // AI Benchmark Float
};

const std::vector<std::vector<std::vector<uint8_t>>> kInputRawData[2] {
        // Input Data Set 1
        {{{0, 0, 119, 67, 0, 0, 15, 67, 0, 0}},               // AI Benchmark Quant
        {{247, 143, 80, 249, 145, 82, 248, 144, 81, 247}}},   // AI Benchmark Float

        // Input Data Set 2
        {{{0, 0, 74, 67, 0, 0, 19, 67, 0, 0}},                // AI Benchmark Quant
        {{202, 147, 90, 197, 145, 88, 195, 144, 89, 192}}},   // AI Benchmark Float
};

MtkSportsMode::MtkSportsMode() {
    NP_VLOG << "MtkSportsMode::MtkSportsMode";
    mStages.resize(BENCHMARK_MODEL_COUNT);
    for (int i = 0; i < BENCHMARK_MODEL_COUNT; i++) {
        mStages[i] = STAGE_INITIAL_STAGE;
    }
}

int MtkSportsMode::getSportsMode() {
    // Don't check again since we already found
    if (mSportMode != SPORTS_MODE_NOT_SURE) {
        return mSportMode;
    }

    int ongoing = 0;
    for (int i = 0; i < BENCHMARK_MODEL_COUNT; i++) {
        if (mStages[i] == STAGE_FOUND_PATTERN) {
            mSportMode = SPORTS_MODE_TURN_ON;
            return mSportMode;
        } else if (mStages[i] == STAGE_FOUND_NOT_PATTERN) {
            ongoing = ongoing | (1 << i);
        }
    }

    if (ongoing == BENCHMARK_SCAN_COMPLETE) {
        mSportMode = SPORTS_MODE_TURN_OFF;
    }

    return mSportMode;
}

void MtkSportsMode::comparePatterns(uint32_t inputIndex, SportModeBufferInfo info) {
    if (getSportsMode() == SPORTS_MODE_TURN_OFF) {
        return;
    } else if (getSportsMode() == SPORTS_MODE_TURN_ON) {
        if (!mNotify) {
            notifySportsMode();
        }
        return;
    }

    for (int i = 0; i < BENCHMARK_MODEL_COUNT; i++) {
        NP_VLOG << "The pattern " << i << " :" << std::to_string(mStages[i]);
        if (mStages[i] == STAGE_FOUND_NOT_PATTERN) {
            NP_VLOG << "Not match: " << i;
            return;
        }

        int stage = mStages[i];
        NP_VLOG << "current stage is:" << stage;

        if (stage == (sizeof(kInputRawData)/ sizeof(*kInputRawData))) {
            NP_VLOG << "Sports Mode Pattern Hit";
            mStages[i] = STAGE_FOUND_PATTERN;
            continue;
        }
        std::vector<std::vector<std::vector<uint8_t>>> table = kInputRawData[stage];
        std::vector<std::vector<uint8_t>> operand = table[i];
        std::vector<uint8_t> raw = operand[inputIndex];

        if (inputIndex > operand.size() || info.length < raw.size()) {
            LOG(ERROR) << "Parameter error! index: " << inputIndex << ", length:" << info.length;
            mStages[i] = STAGE_FOUND_NOT_PATTERN;
            continue;
        }

        uint8_t *buffer = nullptr;
        buffer = new uint8_t[raw.size()];
        if (!buffer) {
            LOG(ERROR) << "out of memory";
            mStages[i] = STAGE_FOUND_NOT_PATTERN;
            continue;
        }
        if (info.source == BUFFER_SOURCE_POINTER) {
            memcpy(buffer, reinterpret_cast<uint8_t*>(info.buffer), raw.size());
        } else {
            uint8_t* tmp = nullptr;
            Memory *mem = reinterpret_cast<Memory*>(info.buffer);
            int ret = mem->getPointer(&tmp);
            if (ret != ANEURALNETWORKS_NO_ERROR) {
                LOG(ERROR) << "failed to get pointer from memory";
                mStages[i] = STAGE_FOUND_NOT_PATTERN;
                if (buffer) {
                    delete buffer;
                }
                continue;
            }
            memcpy(buffer, &tmp[info.offset], raw.size());
        }

        for (uint32_t k = 0; k < raw.size(); k++) {
            NP_VLOG << "Input operand " << i
                    << " raw: " << k << " in table:" << std::to_string(raw[k])
                    << " true: " << std::to_string(buffer[k]);

            if (raw[k] != buffer[k]) {
                NP_VLOG << "data not match!";
                mStages[i] = STAGE_FOUND_NOT_PATTERN;
                if (buffer) {
                    delete buffer;
                }
                return;
            }
        }
        mStages[i]++;
        if (buffer) {
            delete buffer;
        }
    }
}

void MtkSportsMode::setInput(uint32_t inputIndex, const void* buffer, size_t length) {
    SportModeBufferInfo info = {.source = BUFFER_SOURCE_POINTER,
                                .buffer = const_cast<void*>(buffer),
                                .offset = 0,
                                .length = length,
                               };
    comparePatterns(inputIndex, info);
}

void MtkSportsMode::setInputFromMemory(
        uint32_t inputIndex, const Memory* memory, size_t offset, size_t length) {
    SportModeBufferInfo info = {.source = BUFFER_SOURCE_MEMORYT,
                                .buffer = reinterpret_cast<void*>(const_cast<Memory*>(memory)),
                                .offset = offset,
                                .length = length
                               };
    comparePatterns(inputIndex, info);
}

void MtkSportsMode::compareModels(MtkModelBuilder* model, int modelIdx) {
    if (mStages[modelIdx] == STAGE_FOUND_NOT_PATTERN) {
        NP_VLOG << "Not match: " << modelIdx;
        return;
    }

    std::vector<std::vector<uint32_t>> operands = kInputOperands[modelIdx];
    uint32_t size = operands.size();

    NP_VLOG << "Model " << modelIdx << " input operand size :" << size
            << ", Truly input count : " << model->inputCount();

    if (model->inputCount() != size) {
        NP_VLOG << "size not match!";
        mStages[modelIdx] = STAGE_FOUND_NOT_PATTERN;
        return;
    }

    for (uint32_t i = 0; i < size; i++) {
        std::vector<uint32_t> dim = operands[i];
        const Operand& inputOperand = model->getInputOperand(i);

        if (isNeuroPilotVLogSupported()) {
            LOG(INFO) << "Input operand " << i << " dim size: "<< dim.size()
                    << ", Truly dim size : " << inputOperand.dimensions.size();
            for (uint32_t k = 0; k < dim.size(); k++) {
                LOG(INFO) << "Input operand " << i << " dim in table" << k << " :"
                        << std::to_string(dim[k])
                        << ", value: " << std::to_string(inputOperand.dimensions[k]);
            }
        }

        if (inputOperand.dimensions != dim) {
            NP_VLOG << "dimension not match!";
            mStages[modelIdx] = STAGE_FOUND_NOT_PATTERN;
            return;
        }
    }
}

void MtkSportsMode::notifySportsMode() {
    if (mPowerHalMgrBinder == nullptr) {
        android::sp<android::IServiceManager> sm(android::defaultServiceManager());
        mPowerHalMgrBinder = sm->checkService(android::String16("power_hal_mgr_service"));
        if (mPowerHalMgrBinder == nullptr) {
            LOG(ERROR) << "[power_hal_mgr_service_test] null binder";
            return;
        }
    }

    android::Parcel data;
    android::Parcel reply;
    data.writeInterfaceToken(android::String16("com.mediatek.powerhalmgr.IPowerHalMgr"));
    data.writeStrongBinder(NULL);
    mPowerHalMgrBinder->transact(13, data, &reply);
    mNotify = true;
    NP_VLOG << "Notify sports mode";
}

}  // namespace nn
}  // namespace android
