/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_SENSOR_CONTROL_TYPE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_SENSOR_CONTROL_TYPE_H_
//
#include <mtkcam/def/common.h>
//
#include <vector>
#include <unordered_map>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace sensorcontrol {
enum SensorStatus
{
    E_NONE,
    E_STANDBY,
    E_STREAMING,
};

enum FeatureMode
{
    E_None,
    E_Zoom,
    E_Bokeh,
    E_Multicam,
};

struct SensorControlInfo
{
    SensorStatus iSensorStatus = SensorStatus::E_NONE;
    MBOOL  bAFDone      = false;
    MINT32 iAEIso       = -1;
    MINT32 iAELV_x10    = -1;
    MINT32 eSensorStatus = -1;
    //
    float fSensorFov_H  = .0f;
    float fSensorFov_V  = .0f;
    MSize msSensorFullSize = MSize(0,0);
    MRect mActiveArrayRegion; // active array size, store in android.sensor.info.activeArraySize.
    MFLOAT mFocalLength = .0f;
};

struct SensorControlResult
{
    SensorStatus iSensorControl = SensorStatus::E_NONE;
    float fAlternavtiveFov_H = .0f;
    float fAlternavtiveFov_V = .0f;
    MRect mrAlternactiveCropRegion;
    MRect mrAlternactiveCropRegion_Cap;
    bool isMaster = false;
};

struct SensorControlParamIn
{
    FeatureMode mFeatureMode = FeatureMode::E_None;
    //
    uint32_t mRequestNo = 0;
    //
    std::vector<int32_t> vSensorIdList;
    // <sensor id, sensor info>
    std::unordered_map<int32_t, std::shared_ptr<SensorControlInfo> > vInfo;
    //
    MRect mrCropRegion; // active domain and pass by android.scaler.cropRegion.
    //
    std::vector<MFLOAT> mAvailableFocalLength;
    //
    MFLOAT mFocalLength;
    //
    MFLOAT mZoomRatio = 0;
};

struct SensorControlParamOut
{
    float fZoomRatio;
    // <sensor id, sensor info>
    std::unordered_map<int32_t, std::shared_ptr<SensorControlResult> > vResult;
};

};  //namespace sensorcontrol
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_SENSOR_CONTROL_TYPE_H_

