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
#include <hardware/sensors.h>
#include <utils/Log.h>
#include <linux/hwmsensor.h>

#include "MtkInterface.h"
#include "API_sensor_calibration.h"

#undef LOG_TAG
#define LOG_TAG "MtkInterface"

MtkInterface *MtkInterface::MtkInterfaceInstance = nullptr;
MtkInterface *MtkInterface::getInstance() {
    if (MtkInterfaceInstance == nullptr) {
        MtkInterface *mInterface = new MtkInterface;
        MtkInterfaceInstance = mInterface;
    }
    return MtkInterfaceInstance;
}

MtkInterface::MtkInterface() {
    accLastTimeStamp = 0;
    gyroLastTimeStamp = 0;
}

MtkInterface::~MtkInterface() {
}

int MtkInterface::setGyroData(struct sensorData * /*inputData*/) {
    return 0;
}

int MtkInterface::setAccData(struct sensorData * /*inputData*/) {
    return 0;
}

int MtkInterface::setMagData(struct sensorData * /*inputData*/) {
    return 0;
}

int MtkInterface::getGravity(struct sensorData * /*outputData*/) {
    return 0;
}

int MtkInterface::getRotationVector(struct sensorData * /*outputData*/) {
    return 0;
}

int MtkInterface::getOrientation(struct sensorData * /*outputData*/) {
    return 0;
}

int MtkInterface::getLinearaccel(struct sensorData * /*outputData*/) {
    return 0;
}

int MtkInterface::getGameRotationVector(struct sensorData * /*outputData*/) {
    return 0;
}

int MtkInterface::getGeoMagnetic(struct sensorData * /*outputData*/) {
    return 0;
}

int MtkInterface::accInitCalibration() {
    float bias[3] = {0};

    Acc_init_calibration(bias);
    return 0;
}

int MtkInterface::accSetCalibration(struct sensorData *inputData) {
    float bias[3] = {0};

    bias[0] = inputData->data[0];
    bias[1] = inputData->data[1];
    bias[2] = inputData->data[2];
    Acc_init_calibration(bias);
    return 0;
}

int MtkInterface::accGetCalibration(struct sensorData *outputData) {
    float bias[3] = {0};

    Acc_get_calibration_parameter(bias);
    outputData->data[0] = bias[0];
    outputData->data[1] = bias[1];
    outputData->data[2] = bias[2];
    return 0;
}

int MtkInterface::accRunCalibration(struct sensorData *inputData,
        struct sensorData *outputData) {
    int dt = 0;
    int calibrate_acc = 0;
    float raw_data_input[3] = {0};
    float calibrated_data_output[3] = {0};

    if (accLastTimeStamp) {
        dt = int(inputData->timeStamp - accLastTimeStamp);
    }
    accLastTimeStamp = inputData->timeStamp;

    raw_data_input[0] = inputData->data[0];
    raw_data_input[1] = inputData->data[1];
    raw_data_input[2] = inputData->data[2];
    Acc_run_calibration(dt, raw_data_input, calibrated_data_output, &calibrate_acc);
    outputData->timeStamp = inputData->timeStamp;
    outputData->data[0] = calibrated_data_output[0];
    outputData->data[1] = calibrated_data_output[1];
    outputData->data[2] = calibrated_data_output[2];
    outputData->status = calibrate_acc;
    return 0;
}

int MtkInterface::gyroInitCalibration() {;
    float slope[3] = {0};
    float intercept[3] = {0};

    Gyro_init_calibration(slope, intercept);
    return 0;
}

int MtkInterface::gyroSetCalibration(struct sensorData * /*inputData*/) {
    return 0;
}

int MtkInterface::gyroGetCalibration(struct sensorData *outputData) {
    float bias[3] = {0};
    float slope[3] = {0};
    float intercept[3] = {0};

    Gyro_get_calibration_parameter(bias, slope, intercept);
    outputData->data[0] = bias[0];
    outputData->data[1] = bias[1];
    outputData->data[2] = bias[2];
    return 0;
}

int MtkInterface::gyroSetTempCaliParameter(struct sensorData *inputData) {
    float slope[3] = {0};
    float intercept[3] = {0};

    slope[0] = inputData->data[0];
    slope[1] = inputData->data[1];
    slope[2] = inputData->data[2];
    intercept[0] = inputData->data[3];
    intercept[1] = inputData->data[4];
    intercept[2] = inputData->data[5];
    Gyro_init_calibration(slope, intercept);
    return 0;
}

int MtkInterface::gyroGetTempCaliParameter(struct sensorData *outputData) {
    float bias[3] = {0};
    float slope[3] = {0};
    float intercept[3] = {0};

    Gyro_get_calibration_parameter(bias, slope, intercept);
    outputData->data[0] = slope[0];
    outputData->data[1] = slope[1];
    outputData->data[2] = slope[2];
    outputData->data[3] = intercept[0];
    outputData->data[4] = intercept[1];
    outputData->data[5] = intercept[2];
    return 0;
}

int MtkInterface::gyroRunCalibration(struct sensorData *inputData,
        struct sensorData *outputData) {
    int dt = 0;
    int calibrate_acc = 0;
    float temperature = 0;
    float raw_data_input[3] = {0};
    float calibrated_data_output[3] = {0};

    if (gyroLastTimeStamp) {
        dt = (int)(inputData->timeStamp - accLastTimeStamp);
    }
    gyroLastTimeStamp = inputData->timeStamp;

    raw_data_input[0] = inputData->data[0];
    raw_data_input[1] = inputData->data[1];
    raw_data_input[2] = inputData->data[2];
    temperature = inputData->data[3];
    Gyro_run_calibration(dt, raw_data_input, calibrated_data_output, &calibrate_acc, temperature);
    outputData->timeStamp = inputData->timeStamp;
    outputData->data[0] = calibrated_data_output[0];
    outputData->data[1] = calibrated_data_output[1];
    outputData->data[2] = calibrated_data_output[2];
    outputData->status = calibrate_acc;
    //ALOGI("run cali: [%d %d %f, %f %f %f %f %f]\n", dt, calibrate_acc, raw_data_input[0], raw_data_input[1], raw_data_input[2], calibrated_data_output[0], calibrated_data_output[1], calibrated_data_output[2]);
    return 0;
}
