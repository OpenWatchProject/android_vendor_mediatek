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


#ifndef _SENSOR_CALIBRATION_H_

#define _SENSOR_CALIBRATION_H_

#include "BaseInterface.h"
#include "VendorInterface.h"
#include "MtkInterface.h"

class SensorCalibration {
public:
    static SensorCalibration *getInstance();
    ~SensorCalibration();
    int accInitCalibration();
    int accSetCalibration(struct sensorData *inputData);
    int accGetCalibration(struct sensorData *outputData);
    int accRunCalibration(struct sensorData *inputData, struct sensorData *outputData);
    int gyroInitCalibration();
    int gyroSetCalibration(struct sensorData *inputData);
    int gyroGetCalibration(struct sensorData *outputData);
    int gyroSetTempCaliParameter(struct sensorData *inputData);
    int gyroGetTempCaliParameter(struct sensorData *outputData);
    int gyroRunCalibration(struct sensorData *inputData, struct sensorData *outputData);
    int magInitCalibration();
    int magEnableCalibration(int en);
    int magSetCalibration(struct sensorData *inputData);
    int magGetCalibration(struct sensorData *outputData);
    int magSetAccData(struct sensorData *inputData);
    int magSetGyroData(struct sensorData *inputData);
    int magRunCalibration(struct sensorData *inputData, struct sensorData *outputData);
protected:
    SensorCalibration();
    SensorCalibration(const SensorCalibration& other);
    SensorCalibration& operator = (const SensorCalibration& other);

private:
    static SensorCalibration *SensorCalibrationInstance;
    VendorInterface *mVendorInterface;
    MtkInterface *mMtkInterface;
};

#endif
