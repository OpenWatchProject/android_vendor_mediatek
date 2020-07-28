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

#include "MtkTestModelFloat16AddConv.h"

MtkTestModelFloat16AddConv::MtkTestModelFloat16AddConv()
        : mInputRow(4), mInputCol(4), mOutputRow(2), mOutputCol(2) {
    memset(mOutputMat, 0, sizeof(mOutputMat));
    setOperationCount(2);
}

MtkTestModelFloat16AddConv::~MtkTestModelFloat16AddConv() {
    if (mModel != nullptr) {
        ANeuralNetworksModel_free(mModel);
    }

    if (mCompilation != nullptr) {
        ANeuralNetworksCompilation_free(mCompilation);
    }

    if (mExecution != nullptr) {
        ANeuralNetworksExecution_free(mExecution);
    }

    if (mBurst != nullptr) {
        ANeuralNetworksBurst_free(mBurst);
    }
}
int MtkTestModelFloat16AddConv::createModelForTest() {
    int ret = ANeuralNetworksModel_create(&mModel);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_create\n");
        return ret;
    }

    //
    // create operand
    //
    std::vector<uint32_t> dim{1, mInputRow, mInputCol, 1};
    ANeuralNetworksOperandType op = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_TENSOR_FLOAT16),
        .dimensionCount    = static_cast<uint32_t>(dim.size()),
        .dimensions        = dim.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };

    std::vector<uint32_t> dimOut{1, mOutputRow, mOutputCol, 1};
    ANeuralNetworksOperandType opOut = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_TENSOR_FLOAT16),
        .dimensionCount    = static_cast<uint32_t>(dimOut.size()),
        .dimensions        = dimOut.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };


    std::vector<uint32_t> filterDim{1, 2, 2, 1};
    ANeuralNetworksOperandType opFilter = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_TENSOR_FLOAT16),
        .dimensionCount    = static_cast<uint32_t>(filterDim.size()),
        .dimensions        = filterDim.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };

    std::vector<uint32_t> biasDim{1};
    ANeuralNetworksOperandType opBias = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_TENSOR_FLOAT16),
        .dimensionCount    = static_cast<uint32_t>(biasDim.size()),
        .dimensions        = biasDim.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };

    std::vector<uint32_t> pad0Data;
    ANeuralNetworksOperandType pad0 = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_INT32),
        .dimensionCount    = static_cast<uint32_t>(pad0Data.size()),
        .dimensions        = pad0Data.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };

    std::vector<uint32_t> addDim{1, mOutputRow, mOutputCol, 1};
    ANeuralNetworksOperandType addOp = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_TENSOR_FLOAT16),
        .dimensionCount    = static_cast<uint32_t>(addDim.size()),
        .dimensions        = addDim.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };

    std::vector<uint32_t> addScalarDim;
    ANeuralNetworksOperandType addScalar = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_INT32),
        .dimensionCount    = static_cast<uint32_t>(addScalarDim.size()),
        .dimensions        = addScalarDim.data(),
        .scale             = 0.0f,
        .zeroPoint         = 0
    };

    //
    // max_pooling_2d operands
    // index : 0 -> input matrix
    // index : 1 -> filter matrix
    // index : 2 -> bias matrix
    // index : 3 -> padding_left
    // index : 4 -> padding_right
    // index : 5 -> padding_top
    // index : 6 -> padding_bottom
    // index : 7 -> stride_width
    // index : 8 -> stride_height
    // index : 9 -> activation
    // index : 10 -> conv2D output (add input)
    // index : 11 -> add input
    // index : 12 -> add activation
    // index : 13 -> output
    //
    ADD_OPERAND(mModel, op)
    ADD_OPERAND(mModel, opFilter)
    ADD_OPERAND(mModel, opBias)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, pad0)
    ADD_OPERAND(mModel, opOut)
    ADD_OPERAND(mModel, addOp)
    ADD_OPERAND(mModel, addScalar)
    ADD_OPERAND(mModel, addOp)

    //
    // set operand value
    //
    SET_INT32_OPERAND_VALUE(0, 3, mModel)
    SET_INT32_OPERAND_VALUE(0, 4, mModel)
    SET_INT32_OPERAND_VALUE(0, 5, mModel)
    SET_INT32_OPERAND_VALUE(0, 6, mModel)
    SET_INT32_OPERAND_VALUE(2, 7, mModel)
    SET_INT32_OPERAND_VALUE(2, 8, mModel)
    SET_INT32_OPERAND_VALUE(0, 9, mModel)
    SET_INT32_OPERAND_VALUE(0, 12, mModel)

    SET_FP16_OPERAND_VALUE(mFilterMat, filterDim.size(), 1, mModel)
    SET_FP16_OPERAND_VALUE(mBiasMat, biasDim.size(), 2, mModel)


    //
    // add operation
    //
    std::vector<uint32_t> in{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<uint32_t> out{10};
    ret = ANeuralNetworksModel_addOperation(
            mModel, ANEURALNETWORKS_CONV_2D, static_cast<uint32_t>(in.size()), in.data(),
            static_cast<uint32_t>(out.size()), out.data());
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_addOperation\n");
        return ret;
    }

    std::vector<uint32_t> in2{10, 11, 12};
    std::vector<uint32_t> out2{13};
    ret = ANeuralNetworksModel_addOperation(
            mModel, ANEURALNETWORKS_ADD, static_cast<uint32_t>(in2.size()), in2.data(),
            static_cast<uint32_t>(out2.size()), out2.data());
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_addOperation\n");
        return ret;
    }

    //
    // set inputs and outputs
    //
    std::vector<uint32_t> in3{0, 11};
    std::vector<uint32_t> out3{13};
    ret = ANeuralNetworksModel_identifyInputsAndOutputs(
            mModel, static_cast<uint32_t>(in3.size()), in3.data(),
            static_cast<uint32_t>(out3.size()), out3.data());
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_identifyInputsAndOutputs\n");
        return ret;
    }

    ret = ANeuralNetworksModel_relaxComputationFloat32toFloat16(mModel, mRelaxed);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("fail to ANeuralNetworksModel_relaxComputationFloat32toFloat16\n");
        return ret;
    }

    ret = ANeuralNetworksModel_finish(mModel);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_finish\n");
        return ret;
    }

    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelFloat16AddConv::createRequestForTest() {
    //
    // set buffer
    //
    int ret = ANeuralNetworksExecution_setInput(
        mExecution, 0, nullptr, mInputMat, sizeof(mInputMat));
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_setInput - input\n");
        return ret;
    }

    ret = ANeuralNetworksExecution_setInput(
        mExecution, 1, nullptr, mMatrix, sizeof(mMatrix));
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_setInput - bias\n");
        return ret;
    }

    ret = ANeuralNetworksExecution_setOutput(
            mExecution, 0, nullptr, mOutputMat, sizeof(mOutputMat));
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_setOutput\n");
        return ret;
    }

    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelFloat16AddConv::CompareMatrices() {
    int errors = 0;
    int ret = ANEURALNETWORKS_NO_ERROR;
    for (int i = 0; i < mOutputRow; i++) {
        for (int j = 0; j < mOutputCol; j++) {
            if (mExpectedMat[i][j] != mOutputMat[i][j]) {
                printf("expected[%d][%d] != actual[%d][%d], %f != %f\n",
                        i, j, i, j,
                        static_cast<double>(mExpectedMat[i][j]),
                        static_cast<double>(mOutputMat[i][j]));
                errors++;
            }
        }
    }
    if (errors != 0) {
        ret = ANEURALNETWORKS_OP_FAILED;
    }
    return ret;
}

