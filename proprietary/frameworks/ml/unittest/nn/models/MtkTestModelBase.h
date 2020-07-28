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

#ifndef __MEDIATEK_TEST_MODEL_BASE_H__
#define __MEDIATEK_TEST_MODEL_BASE_H__

#include "NeuralNetworks.h"
#include <stdio.h>
#include <vector>

#define ADD_OPERAND(m, t) \
    {\
        int r = ANeuralNetworksModel_addOperand(m, &t);\
        if (r != ANEURALNETWORKS_NO_ERROR) { \
            printf("failed to ANeuralNetworksModel_addOperand\n");\
            return r;\
        }\
    }

#define SET_INT32_OPERAND_VALUE(v, i, m)\
    {\
        int32_t val(v); \
        int r = ANeuralNetworksModel_setOperandValue(m, i, &val, sizeof(val));\
        if (r != ANEURALNETWORKS_NO_ERROR) {\
            printf("failed to ANeuralNetworksModel_setOperandValue\n"); \
            return r;\
        }\
    }

#define SET_FP32_OPERAND_VALUE(v, l, i, m)\
    {\
        int r = ANeuralNetworksModel_setOperandValue(m, i, &v, sizeof(float) * l);\
        if (r != ANEURALNETWORKS_NO_ERROR) {\
            printf("failed to ANeuralNetworksModel_setOperandValue\n"); \
            return r; \
        }\
    }

#define SET_FP16_OPERAND_VALUE(v, l, i, m)\
    {\
        int r = ANeuralNetworksModel_setOperandValue(m, i, &v, sizeof(_Float16) * l);\
        if (r != ANEURALNETWORKS_NO_ERROR) {\
            printf("failed to ANeuralNetworksModel_setOperandValue\n");\
            return r;\
        }\
    }

class MtkTestModelBase {
public:
    MtkTestModelBase() {}
    virtual ~MtkTestModelBase() {};
    int run();
    int runBurst();

    ANeuralNetworksModel* getModel() { return mModel; }
    ANeuralNetworksExecution* getExecution() { return mExecution; }
    void setRelaxed(bool relaxed) { mRelaxed = relaxed; }
    void setMeasure(bool measure) { mMeasure = measure; }
    void setPreference(int preference) { mPreference = preference; }
    void setPartitionExt(int partitionExt) { mPartitionExt = partitionExt; }
    void addDevice(ANeuralNetworksDevice* device) { mDevices.push_back(device); }
    int getOperationCount() { return mOpCount;}

protected:
    virtual int createModelForTest() { return ANEURALNETWORKS_BAD_STATE; }
    int createCompilationForTest();
    int createExecutionForTest();
    int createBurstForTest();
    virtual int createRequestForTest() { return ANEURALNETWORKS_BAD_STATE; }
    int startCompute();
    virtual int CompareMatrices() { return ANEURALNETWORKS_OP_FAILED; }
    void setOperationCount(int opCount) { mOpCount = opCount; }

protected:
    ANeuralNetworksModel* mModel = nullptr;
    ANeuralNetworksCompilation* mCompilation = nullptr;
    ANeuralNetworksExecution* mExecution = nullptr;
    ANeuralNetworksBurst* mBurst = nullptr;

    std::vector<ANeuralNetworksDevice*> mDevices;

    bool mRelaxed = false;
    bool mMeasure = false;
    bool mPreference = false;
    int mPartitionExt = 0;
    int mOpCount = 0;
};

#endif  // __MEDIATEK_TEST_MODEL_BASE_H__

