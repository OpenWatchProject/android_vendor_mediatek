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

#ifndef MEDIATEK_ML_NN_RUNTIME_EXECUTION_BUILDER_H
#define MEDIATEK_ML_NN_RUNTIME_EXECUTION_BUILDER_H

#include "ExecutionBuilder.h"
#include <sys/time.h>

#include "NeuroPilotDef.h"

namespace android {
namespace nn {

class MtkExecutionBuilder : public ExecutionBuilder {
public:
    explicit MtkExecutionBuilder(const CompilationBuilder* compilation);
    virtual ~MtkExecutionBuilder();

    // Profiler Start
    int startProfile(const char* device);
    int stopProfile(StepExecutor* executor, Request *request, int err);
    int clearProfilerInfo();
    int getProfilerResult(std::vector<ProfilerResult> *result);
    void setCurrentExecutionStep(uint32_t step) { mCurrentStep = step; }
    // Profiler End

    /// M: Sports Mode @{
    int setInput(uint32_t index, const ANeuralNetworksOperandType* type, const void* buffer,
                 size_t length) override;
    int setInputFromMemory(uint32_t index, const ANeuralNetworksOperandType* type,
                           const Memory* memory, size_t offset, size_t length) override;
    /// M: Sports Mode @}

    /// M: Eara Qos @{
    int addExecutionExtraParam(MemoryTracker *mMemories, Memory* memory) override;
    /// M: Eara Qos @}

private:
    // Profiler @{
    uint32_t mCurrentStep = 0;
    std::vector<ProfilerResult> mProfilerData;

    void prepareExecutionTime(const ModelBuilder* model, double time);
    void prepareExecutionResult(const ModelBuilder* model,
                                const MemoryTracker memories,
                                std::vector<ModelArgumentInfo> outputs);
    int copyArgumentBuffer(uint8_t** buffers, const MemoryTracker memories, ModelArgumentInfo arg);
    /// Profiler @}
};

}  // namespace nn
}  // namespace android

#endif  // MEDIATEK_ML_NN_RUNTIME_EXECUTION_BUILDER_H
