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
#include "MtkTestModelBase.h"
#include "NeuroPilotShim.h"

int MtkTestModelBase::run() {
    // Create Model
    int ret = createModelForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        return ret;
    }

    // Create Compilation
    ret = createCompilationForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        return ret;
    }

    // Create Execution and Request Start
    ret = createExecutionForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        return ret;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelBase::runBurst() {
    // Create Model
    int ret = createModelForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        return ret;
    }

    // Create Compilation
    ret = createCompilationForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        return ret;
    }

    // Create Execution and Request Start
    ret = createBurstForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        return ret;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelBase::createCompilationForTest() {
    int ret = ANEURALNETWORKS_NO_ERROR;
    if (mDevices.size() == 0) {
        ret = ANeuralNetworksCompilation_create(mModel, &mCompilation);
        if (ret != ANEURALNETWORKS_NO_ERROR) {
            printf("failed to ANeuralNetworksCompilation_create\n");
            return ret;
        }
    } else {
        ret = ANeuralNetworksCompilation_createForDevices(
            mModel, mDevices.data(), mDevices.size(), &mCompilation);
        if (ret != ANEURALNETWORKS_NO_ERROR) {
            printf("failed to ANeuralNetworksCompilation_createForDevices\n");
            return ret;
        }
    }

    ret = ANeuralNetworksCompilation_setPreference(mCompilation, mPreference);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksCompilation_setPreference\n");
        return ret;
    }

    ret = ANeuroPilotCompilationWrapper_setPartitionExtType(mCompilation, mPartitionExt);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksCompilation_setPreference\n");
        return ret;
    }

    ret = ANeuralNetworksCompilation_finish(mCompilation);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksCompilation_finish\n");
        return ret;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelBase::createExecutionForTest() {
    int ret = ANEURALNETWORKS_NO_ERROR;
    ret = ANeuralNetworksExecution_create(mCompilation, &mExecution);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_create\n");
        return ret;
    }

    ret = createRequestForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
       return ret;
    }

    // Create Execution and Request End
    ret = startCompute();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to startCompute\n");
        return ret;
    }

    // Compare Results
    ret = CompareMatrices();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to compare matrices\n");
        return ret;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelBase::createBurstForTest() {
    // Create Burst
    int ret = ANeuralNetworksBurst_create(mCompilation, &mBurst);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksBurst_create\n");
        return ret;
    }

    // Create Execution
    ret = ANeuralNetworksExecution_create(mCompilation, &mExecution);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_create\n");
        return ret;
    }

    ret = createRequestForTest();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
       return ret;
    }

    // Burst Compute
    ret = ANeuralNetworksExecution_burstCompute(mExecution, mBurst);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksBurst_create\n");
        return ret;
    }

    // Compare Results
    ret = CompareMatrices();
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to compare matrices\n");
        return ret;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int MtkTestModelBase::startCompute() {
    int ret = ANEURALNETWORKS_NO_ERROR;

    // Reset output matrix
    if (mDevices.size() == 1) {
        // Set measure
        ret = ANeuralNetworksExecution_setMeasureTiming(mExecution, mMeasure);
        if (ret != ANEURALNETWORKS_NO_ERROR) {
            printf("failed to ANeuralNetworksExecution_setMeasureTiming\n");
            return ret;
        }
    }
    //
    // start to compute
    //
    ANeuralNetworksEvent* event = nullptr;
    ret = ANeuralNetworksExecution_startCompute(mExecution, &event);
    if (ret != ANEURALNETWORKS_NO_ERROR) {
        printf("failed to ANeuralNetworksExecution_startCompute\n");
        return ret;
    }

    ANeuralNetworksEvent_wait(event);
    ANeuralNetworksEvent_free(event);

    return ANEURALNETWORKS_NO_ERROR;
}

