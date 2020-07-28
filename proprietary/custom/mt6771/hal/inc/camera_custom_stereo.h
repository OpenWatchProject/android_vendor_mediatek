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

#include "camera_custom_types.h"
#include "mtkcam/def/UITypes.h" //For MSize
#include <vector>
#include <map>
#include <set>

/*******************************************************************************
* Return enable/disable flag of STEREO to ISP
*******************************************************************************/
enum ENUM_STEREO_SENSOR_PROFILE
{
    STEREO_SENSOR_PROFILE_UNKNOWN,
    STEREO_SENSOR_PROFILE_REAR_REAR,
    STEREO_SENSOR_PROFILE_FRONT_FRONT,
    STEREO_SENSOR_PROFILE_REAR_FRONT,
    TOTAL_STEREO_SENSOR_PROFILE
};

enum ENUM_STEREO_SENSOR_RELATIVE_POSITION
{
    //Position definition: let phone screen face you and hold it vertically
    STEREO_SENSOR_REAR_MAIN_TOP    = 0,
    STEREO_SENSOR_REAR_MAIN_BOTTOM = 1,
    STEREO_SENSOR_FRONT_MAIN_RIGHT = 0,
    STEREO_SENSOR_FRONT_MAIN_LEFT  = 1
};

enum ENUM_ROTATION
{
    //Rotation is defined as clock-wised
    eRotate_0   = 0,
    eRotate_90  = 90,
    eRotate_180 = 180,
    eRotate_270 = 270
};

enum ENUN_DEPTHMAP_SIZE
{
    STEREO_DEPTHMAP_1X = 1, //240x136
    STEREO_DEPTHMAP_2X = 2, //480x272
    STEREO_DEPTHMAP_4X = 4  //960x544
};

enum ENUM_STEREO_CAM_SCENARIO
{
    eStereoCamPreview,
    eStereoCamRecord,
    eStereoCamCapture
};

enum ENUM_STEREO_CAM_FEATURE
{
    eStereoCamVsdof,
    eStereoCamStereo_Capture,
    eStereoBMDnoise // phase out
};

enum class DCMF_CAMERA_OUTPUTS
{
    eFullSize_YUV_1,
    eFullSize_YUV_2,
    eReSized_YUV_1,
    eReSized_YUV_2,
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

/**
 * Ratio of width:height
 */
#define STEREO_RATIO_CONSEQUENT_BITS (16)
const int STEREO_RATIO_CONSEQUENT_MASK = (1<<STEREO_RATIO_CONSEQUENT_BITS)-1;

enum ENUM_STEREO_RATIO
{
    eRatio_Unknown   = 0
    , eRatio_16_9    = (16<<STEREO_RATIO_CONSEQUENT_BITS)|9
    , eRatio_4_3     = ( 4<<STEREO_RATIO_CONSEQUENT_BITS)|3
    , eRatio_Default = eRatio_16_9  //Never set to eRatio_Unknown
    , eRatio_Sensor  = eRatio_4_3   //Never set to eRatio_Unknown
};

extern const char *DEFAULT_STEREO_SETTING;

typedef std::vector<NSCam::MSize> SIZE_LIST_T;
typedef std::map<ENUM_STEREO_RATIO, SIZE_LIST_T> STEREO_CAPTURE_SIZE_T;

extern const ENUN_DEPTHMAP_SIZE CUSTOM_DEPTHMAP_SIZE;

typedef std::vector<int> GF_TUNING_T;

struct CUST_FOV_CROP_T
{
    float FOV_CROP_DEGREE;
    float FOV_CROP_RATIO;

    CUST_FOV_CROP_T(float degree = 0.0f, float ratio = 0.0f)
        : FOV_CROP_DEGREE(degree)
        , FOV_CROP_RATIO(ratio)
    {
    }
};

extern const size_t DPE_CAPTURE_ROUND;

enum ENUM_WMF_CHANNEL
{
    E_WMF_Y_CHANNEL,
    E_WMF_U_CHANNEL,
    E_WMF_V_CHANNEL,
};
extern const ENUM_WMF_CHANNEL WPE_INPUT_CHANNEL;

extern const std::set<DCMF_CAMERA_OUTPUTS> DCMF_CAMERA_OUTPUT_LIST;

/*******************************************************************************
* Stereo common properties
*******************************************************************************/
// The number of hwsync base frame count
size_t   getHWSyncStableCnt(void);

/*******************************************************************************
* Stereo ZSD cap buffer size
*******************************************************************************/
int   get_stereo_zsd_cap_stored_frame_cnt(void);

/*******************************************************************************
* Stereo scenario control for cpu and frequency usage
*******************************************************************************/
std::vector<int>
get_stereo_cam_cpu_num(
    ENUM_STEREO_CAM_FEATURE featureId,
    ENUM_STEREO_CAM_SCENARIO scenario
);

bool get_stereo_cam_cpu_frequency(
    ENUM_STEREO_CAM_FEATURE featureId,
    ENUM_STEREO_CAM_SCENARIO scenario,
    std::vector<int>& min,
    std::vector<int>& max
);

MUINT8  get_vsdof_cap_queue_size();

MUINT8 canSupportBGService();

extern const char *DEFAULT_STEREO_TUNING;

/*******************************************************************************
* Stereo DCMF(Dual-Cam Multi-Frame) tuning params
*******************************************************************************/
int getDCMFCaptureCnt(void);

int getDCMFCapturePhaseCnt(void);

bool getIsPureZSD_DCMF(void);

bool getIsCameraCallback(void);

int getSWNRExecutedRealIsoThreshold();

std::vector<int> getMFLLExecutedRealIsoThresholds();

bool getIsSuportedFDToHDRInHDRBokehMode();

bool getIsSuportedFDToDepthInMFNRBokehMode();

int getStereoModeType();

int getStereoCaptureModeType();

bool supportMain2FullRaw();

bool supportDramControl();

bool needControlMmdvfs();

MINT64 getFrameSyncToleranceTime();

bool usingMultithreadForPipelineContext();
#endif  //  _CAMERA_CUSTOM_STEREO_IF_
