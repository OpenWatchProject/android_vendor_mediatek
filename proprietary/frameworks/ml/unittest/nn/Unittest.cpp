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

#include "NeuralNetworks.h"
#include "NeuroPilotNN.h"
#include "MtkDefinition.h"
#include "utUtils.h"

TEST(NeuroPilotUnittest, utRunFloat32Model) {
    int ret = utRunFloat32Model(false);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utRunFloat32ModelRelaxed) {
    int ret = utRunFloat32Model(true);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utRunFloat16Model) {
    int ret = utRunFloat16Model();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utRunFloat16ModelBurst) {
    utUtils::enableProfiler(true);
    int ret = utRunFloat16ModelBurst();
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utRunQuant8Model) {
    int ret = utRunQuant8Model();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utSetPartitionExtTypeNone) {
    int ret = utSetPartitionExtType(ANEUROPILOT_PARTITIONING_EXTENSION_NONE);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utSetPartitionExtTypePerOp) {
    int ret = utSetPartitionExtType(ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utSetPartitionExtTypeInvaild) {
    int ret = utSetPartitionExtTypeInvalid(9999);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utPartitionExtensionNone) {
    int ret = utPartitionExtension(ANEUROPILOT_PARTITIONING_EXTENSION_NONE);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utPartitionExtensionPerOp) {
    int ret = utPartitionExtension(ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION);
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utTestExecutionForProfilerOn) {
    utUtils::enableProfiler(true);
    int ret = utRunFloat32Model(false);
    int ret2 = utRunFloat16Model();
    int ret3 = utRunQuant8Model();
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret & ret2 & ret3);
}

TEST(NeuroPilotUnittest, utTestExecutionForProfilerOff) {
    utUtils::enableProfiler(false);
    int ret = utRunFloat32Model(false);
    int ret2 = utRunFloat16Model();
    int ret3 = utRunQuant8Model();
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret & ret2 & ret3);
}

TEST(NeuroPilotUnittest, utTestProfilerApiWhenOn) {
    utUtils::enableProfiler(true);
    int ret = utTestApiForProfiler(true);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utTestProfilerApiWhenOff) {
    utUtils::enableProfiler(false);
    int ret = utTestApiForProfiler(false);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utProfilerDefaultPartition) {
    utUtils::enableProfiler(true);
    int ret = utProfiler(ANEUROPILOT_PARTITIONING_EXTENSION_NONE);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utProfilerPerOperation) {
    utUtils::enableProfiler(true);
    int ret = utProfiler(ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utOperationResultDefaultPartition) {
    utUtils::enableProfiler(true);
    int ret = utOperationResult(ANEUROPILOT_PARTITIONING_EXTENSION_NONE);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utOperationResultPerOperation) {
    utUtils::enableProfiler(true);
    int ret = utOperationResult(ANEUROPILOT_PARTITIONING_EXTENSION_PER_OPERATION);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utCreateAshmemMemory) {
    int ret = utCreateHidlMemory();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utGetAshmemMemoryPointer) {
    int ret = utGetMemoryPointer();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utGetIonMemoryPointer) {
    int ret = utGetMemoryPointer();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utMapAshmemMemory) {
    int ret = utMapMemory();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utMapIonMemory) {
    int ret = utMapMemory();
    ASSERT_EQ(1, ret);
}


TEST(NeuroPilotUnittest, utGetMtkFeatureOption) {
    int ret = utGetMtkFeatureOption();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utTestMtkFeatureOption) {
    int ret = utTestMtkFeatureOption();
    ASSERT_EQ(1, ret);
}


TEST(NeuroPilotUnittest, utTestSportModeInitFlow) {
    int ret = utTestSportModeInitFlow();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utTestMtkExtensions) {
    int ret = utTestMtkExtensions();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utSetCpuOnly) {
    utUtils::enableProfiler(true);
    bool cpuOnly = true;
    utUtils::setCpuOnly(cpuOnly);
    int ret = utSetCpuOnly(cpuOnly);

    cpuOnly = false;
    utUtils::setCpuOnly(cpuOnly);
    int ret2 = utSetCpuOnly(cpuOnly);
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, (ret & ret2));
}

TEST(NeuroPilotUnittest, utGetDevice) {
    utUtils::enableProfiler(true);
    int ret = utGetDevice();
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utGetDeviceApis) {
    int ret = utGetDeviceApis();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utMeasureTiming) {
    utUtils::enableProfiler(true);
    int ret = utMeasureTiming();
    utUtils::clearDebugProperties();
    ASSERT_EQ(1, ret);
}

TEST(NeuroPilotUnittest, utSetPreference) {
    int ret = utSetPreference(ANEURALNETWORKS_PREFER_LOW_POWER);
    int ret2 = utSetPreference(ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER);
    int ret3 = utSetPreference(ANEURALNETWORKS_PREFER_SUSTAINED_SPEED);
    ASSERT_EQ(1, ret & ret2 & ret3);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    int ret;

    ret = RUN_ALL_TESTS();

    return 0;
}

