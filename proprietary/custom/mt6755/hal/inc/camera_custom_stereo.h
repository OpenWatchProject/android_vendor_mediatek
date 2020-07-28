/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_STEREO_IF_
#define _CAMERA_CUSTOM_STEREO_IF_
//
#include "camera_custom_types.h"
#include <vector>
//
/*******************************************************************************
* STEREO sensor baseline
******************************************************************************/
#define STEREO_BASELINE 2.0
/*******************************************************************************
* STEREO sensor position
******************************************************************************/
typedef struct customSensorPos_STEREO_s
{
    MUINT32  uSensorPos;
} customSensorPos_STEREO_t;

customSensorPos_STEREO_t const&  getSensorPosSTEREO();

/*******************************************************************************
* Return enable/disable flag of STEREO to ISP
*******************************************************************************/
MBOOL get_STEREOFeatureFlag(void);  //cotta : added for STEREO

/*******************************************************************************
* Stereo ZSD cap buffer size
*******************************************************************************/
int   get_stereo_zsd_cap_stored_frame_cnt(void);

enum ENUM_STEREO_SENSOR_PROFILE
{
    STEREO_SENSOR_PROFILE_UNKNOWN,
    STEREO_SENSOR_PROFILE_REAR_REAR,
    STEREO_SENSOR_PROFILE_FRONT_FRONT,
    STEREO_SENSOR_PROFILE_REAR_FRONT
};

enum ENUM_STEREO_SENSOR_RELATIVE_POSITION
{
    //Position definition: let phone screen face you and hold it vertically
    STEREO_SENSOR_REAR_MAIN_TOP    = 0,
    STEREO_SENSOR_REAR_MAIN_BOTTOM = 1,
    STEREO_SENSOR_FRONT_MAIN_RIGHT = 0,
    STEREO_SENSOR_FRONT_MAIN_LEFT  = 1
};

struct SensorFOV
{
    SensorFOV() {
        fov_horizontal = 0.0f;
        fov_vertical   = 0.0f;
    }

    SensorFOV(float h, float v)
        : fov_horizontal(h)
        , fov_vertical(v)
    {
    }

    float fov_horizontal;
    float fov_vertical;
};

//This order must align sensors' id
extern const std::vector<SensorFOV> FOV_LIST;

//This order must align sensors' id
extern const std::vector<SensorFOV> TARGET_FOV_LIST;

float getStereoBaseline(ENUM_STEREO_SENSOR_PROFILE profile);

bool getStereoSensorID(ENUM_STEREO_SENSOR_PROFILE profile, int &main1Id, int &main2Id);

bool getStereoFOV(ENUM_STEREO_SENSOR_PROFILE profile, SensorFOV &main1FOV, SensorFOV &main2FOV);

bool getStereoTargetFOV(ENUM_STEREO_SENSOR_PROFILE profile, SensorFOV &main1FOV, SensorFOV &main2FOV);

#endif  //  _CAMERA_CUSTOM_STEREO_IF_