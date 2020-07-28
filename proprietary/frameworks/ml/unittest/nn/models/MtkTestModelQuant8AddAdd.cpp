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

#include "MtkTestModelQuant8AddAdd.h"

MtkTestModelQuant8AddAdd::MtkTestModelQuant8AddAdd()
        : mInputRow(4), mInputCol(4), mOutputRow(4), mOutputCol(4) {
    memset(mOutputMat, 0, sizeof(mOutputMat));
    setOperationCount(2);
}

MtkTestModelQuant8AddAdd::~MtkTestModelQuant8AddAdd() {
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

int MtkTestModelQuant8AddAdd::createModelForTest() {
    int ret = ANeuralNetworksModel_create(&mModel);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_create\n");
        return ret;
    }

    //
    // create operand
    //
    std::vector<uint32_t> addDim{1, mInputRow, mInputCol, 1};
    ANeuralNetworksOperandType addOp = {
        .type              = static_cast<int32_t>(ANEURALNETWORKS_TENSOR_QUANT8_ASYMM),
        .dimensionCount    = static_cast<uint32_t>(addDim.size()),
        .dimensions        = addDim.data(),
        .scale             = 1.0f,
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

    // index : 0 -> ADD0 input 0 (model input 0)
    // index : 1 -> ADD0 input 1 (model input 1)
    // index : 2 -> ADD0 act
    // index : 3 -> ADD1 input 0 (ADD0 output)
    // index : 4 -> ADD1 input 1 (model input 2)
    // index : 5 -> ADD1 act
    // index : 6 -> ADD1 output (model output)

    ADD_OPERAND(mModel, addOp)
    ADD_OPERAND(mModel, addOp)
    ADD_OPERAND(mModel, addScalar)
    ADD_OPERAND(mModel, addOp)
    ADD_OPERAND(mModel, addOp)
    ADD_OPERAND(mModel, addScalar)
    ADD_OPERAND(mModel, addOp)

    //
    // set operand value
    //
    SET_INT32_OPERAND_VALUE(0, 2, mModel)
    SET_INT32_OPERAND_VALUE(0, 5, mModel)

    //
    // add operation
    //
    std::vector<uint32_t> in{0, 1, 2};
    std::vector<uint32_t> out{3};
    ret = ANeuralNetworksModel_addOperation(
            mModel, ANEURALNETWORKS_ADD, static_cast<uint32_t>(in.size()), in.data(),
            static_cast<uint32_t>(out.size()), out.data());
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksModel_addOperation\n");
        return ret;
    }

    std::vector<uint32_t> in2{3, 4, 5};
    std::vector<uint32_t> out2{6};
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
    std::vector<uint32_t> in3{0, 1, 4};
    std::vector<uint32_t> out3{6};
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

int MtkTestModelQuant8AddAdd::createRequestForTest() {
    //
    // set buffer
    //
    int ret = ANeuralNetworksExecution_setInput(mExecution, 0, nullptr, mInput1, sizeof(mInput1));
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_setInput - input 0\n");
        return ret;
    }

    ret = ANeuralNetworksExecution_setInput(mExecution, 1, nullptr, mInput2, sizeof(mInput2));
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_setInput - input 1\n");
        return ret;
    }

    ret = ANeuralNetworksExecution_setInput(mExecution, 2, nullptr, mInput3, sizeof(mInput3));
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_setInput - input 2\n");
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

int MtkTestModelQuant8AddAdd::CompareMatrices() {
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

