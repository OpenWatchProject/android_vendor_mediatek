/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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

#include <unistd.h>
#include <string.h>
#include <utils/Log.h>

#include "SensorCalibration.h"

#undef LOG_TAG
#define LOG_TAG "SensorCalibration"

SensorCalibration *SensorCalibration::SensorCalibrationInstance = nullptr;
SensorCalibration *SensorCalibration::getInstance() {
    if (SensorCalibrationInstance == nullptr) {
        SensorCalibration *mInterface = new SensorCalibration;
        SensorCalibrationInstance = mInterface;
    }
    return SensorCalibrationInstance;
}

SensorCalibration::SensorCalibration() {
    mVendorInterface = VendorInterface::getInstance();
    mMtkInterface = MtkInterface::getInstance();
}

SensorCalibration::~SensorCalibration() {
}

int SensorCalibration::accInitCalibration() {
    return mMtkInterface->accInitCalibration();
}

int SensorCalibration::accSetCalibration(struct sensorData *inputData) {
    return mMtkInterface->accSetCalibration(inputData);
}

int SensorCalibration::accGetCalibration(struct sensorData *outputData) {
    return mMtkInterface->accGetCalibration(outputData);
}

int SensorCalibration::accRunCalibration(struct sensorData *inputData,
        struct sensorData *outputData) {
    return mMtkInterface->accRunCalibration(inputData, outputData);
}

int SensorCalibration::gyroInitCalibration() {
    return mMtkInterface->gyroInitCalibration();
}

int SensorCalibration::gyroSetCalibration(struct sensorData *inputData) {
    return mMtkInterface->gyroSetCalibration(inputData);
}

int SensorCalibration::gyroGetCalibration(struct sensorData *outputData) {
    return mMtkInterface->gyroGetCalibration(outputData);
}

int SensorCalibration::gyroSetTempCaliParameter(struct sensorData *inputData) {
    return mMtkInterface->gyroSetTempCaliParameter(inputData);
}

int SensorCalibration::gyroGetTempCaliParameter(struct sensorData *outputData) {
    return mMtkInterface->gyroGetTempCaliParameter(outputData);
}

int SensorCalibration::gyroRunCalibration(struct sensorData *inputData,
        struct sensorData *outputData) {
    return mMtkInterface->gyroRunCalibration(inputData, outputData);
}

int SensorCalibration::magInitCalibration() {
    return mVendorInterface->magInitLib();
}

int SensorCalibration::magEnableCalibration(int en) {
    return mVendorInterface->magEnableLib(en);
}

int SensorCalibration::magSetCalibration(struct sensorData *inputData) {
    float offset[3] = {0};

    offset[0] = inputData->data[0];
    offset[1] = inputData->data[1];
    offset[2] = inputData->data[2];
    return mVendorInterface->setMagOffset(offset);
}

int SensorCalibration::magGetCalibration(struct sensorData *outputData) {
    float offset[3] = {0};
    int ret = 0;

    ret = mVendorInterface->getMagOffset(offset);
    outputData->data[0] = offset[0];
    outputData->data[1] = offset[1];
    outputData->data[2] = offset[2];
    return ret;
}

int SensorCalibration::magSetAccData(struct sensorData *inputData) {
    return mVendorInterface->setAccData(inputData);
}

int SensorCalibration::magSetGyroData(struct sensorData *inputData) {
    return mVendorInterface->setGyroData(inputData);
}

int SensorCalibration::magRunCalibration(struct sensorData *inputData,
        struct sensorData *outputData) {
    return mVendorInterface->magCalibration(inputData, outputData);
}
