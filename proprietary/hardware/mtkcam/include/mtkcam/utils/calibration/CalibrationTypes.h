/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_CALIBRATION_CALIBRATIONTYPES_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_CALIBRATION_CALIBRATIONTYPES_H_
//
#include <string>
#include <vector>
#include <mtkcam/drv/mem/cam_cal_drv.h>    //For CAM_CAL_Stereo_Data_SIZE

namespace NSCam {
/******************************************************************************
 *
 ******************************************************************************/

enum ENUM_CALIBRATION_FORMAT
{
    E_CALIBRATION_UNKNOWN_FORMAT    = 0,
    E_CALIBRATION_GOOGLE_FORMAT     = 1,
    E_CALIBRATION_MTK_FORMAT        = 1<<1,
    E_CALIBRATION_3RDPARTY_FORMAT   = 1<<2,
};

enum ENUM_MTK_CLIBRATION_SOURCE
{
    E_MTK_CALIBRATION_SOURCE_UNKNOWN,
    E_MTK_CALIBRATION_SOURCE_EXTERNAL,  // Loaded from single file
    E_MTK_CALIBRATION_SOURCE_3RDPARTY,  // Loaded from 3rd party
    E_MTK_CALIBRATION_SOURCE_EEPROM,    // Loaded from sensor
    E_MTK_CALIBRATION_SOURCE_SETTING,   // Loaded from setting, only for golden
};

struct CalibrationQueryParam
{
    int                         openId = -1;
    std::vector<int>            physicalSensorIDs;
    std::vector<std::string>    physicalSensorNames;
};

struct CalibrationResult
{
    ENUM_CALIBRATION_FORMAT     format = E_CALIBRATION_UNKNOWN_FORMAT;
    ENUM_MTK_CLIBRATION_SOURCE  source = E_MTK_CALIBRATION_SOURCE_UNKNOWN;

    CalibrationResult(ENUM_CALIBRATION_FORMAT f)
    {
        format = f;
    }
};

struct CalibrationResultInGoogleFormat : public CalibrationResult
{
    int                 lensPoseReference = 0;      // PRIMARY_CAMERA: 0, GYROSCOPE: 1
    std::vector<float>  lensPoseRotation;           // [x, y, z, w]
    std::vector<float>  lensPoseTranslation;        // [x, y, z]
    std::vector<float>  lensIntrinsicCalibration;   // [f_x, f_y, c_x, c_y, s]
    std::vector<float>  lensDistortion;             // [kappa_1, kappa_2, kappa_3, kappa_4, kappa_5]

    CalibrationResultInGoogleFormat() : CalibrationResult(E_CALIBRATION_GOOGLE_FORMAT) {}
};

struct CalibrationResultInMTKFormat : public CalibrationResult
{
    float                       calibration[CAM_CAL_Stereo_Data_SIZE/sizeof(float)];
    ENUM_MTK_CLIBRATION_SOURCE  source = E_MTK_CALIBRATION_SOURCE_3RDPARTY;

    CalibrationResultInMTKFormat() : CalibrationResult(E_CALIBRATION_MTK_FORMAT) {}
};

/******************************************************************************
 *
 ******************************************************************************/
};  //  NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_CALIBRATION_CALIBRATIONTYPES_H_

