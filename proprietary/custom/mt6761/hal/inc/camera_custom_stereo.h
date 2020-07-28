/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
    eStereoBMDnoise,
    eStereoBMDnoiseMFHR
};

struct BMDeNoiseISODependentParam
{
    int ISO            = 1;
    int BW_SingleRange = 1;
    int BW_OccRange    = 1;
    int BW_Range       = 1;
    int BW_Kernel      = 4;
    int B_Range        = 1;
    int B_Kernel       = 1;
    int W_Range        = 1;
    int W_Kernel       = 1;
    int VSL            = 2598;
    int VOFT           = 28635;
    int VGAIN          = 8192;

    BMDeNoiseISODependentParam(
        int iso,
        int bw_SingleRange, int bw_OccRange, int bw_Range, int bw_Kernel,
        int b_Range, int b_Kernel, int w_Range, int w_Kernel,
        int vsl, int voft, int vgain
    )
    : ISO(iso)
    , BW_SingleRange(bw_SingleRange)
    , BW_OccRange(bw_OccRange)
    , BW_Range(bw_Range)
    , BW_Kernel(bw_Kernel)
    , B_Range(b_Range)
    , B_Kernel(b_Kernel)
    , W_Range(w_Range)
    , W_Kernel(w_Kernel)
    , VSL(vsl)
    , VOFT(voft)
    , VGAIN(vgain)
    {}
};

struct BMDeNoiseSensorDependentParam
{
    float v_scale = 1.2685f;
    float v_offset = 55.927f;
};

struct BMDeNoiseQualityPerformanceParam
{
    int dsH = 4;
    int dsV = 4;
    int FPREPROC = 1;
    int FSSTEP = 4;
    int DblkRto = 0;
    int DblkTH = 0;
};

struct BMDeNoiseFOVCrop
{
    float theta_delta = 6.0;
    float ratio_left = 0.5;
    float ratio_top = 0.9;
};

enum DeNoiseMode
{
    eDeNoiseMode_NONE = 0,
    eDeNoiseMode_BM,
    eDeNoiseMode_BM_MNR,
    eDeNoiseMode_BM_SWNR,
};

enum DeNoiseVersion
{
    VERSION_1 = 0,
    VERSION_2
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

enum BMDeNoiseMain2Switch
{
    Off = 0,
    On  = 1
};

typedef std::vector<NSCam::MSize> SIZE_LIST_T;

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

/*******************************************************************************
* Stereo ZSD cap buffer size
*******************************************************************************/
int   get_stereo_zsd_cap_stored_frame_cnt(void);

/*******************************************************************************
* Stereo BM-DeNoise BMDN mode
*******************************************************************************/
DeNoiseVersion getBMDeNoiseVersion();

DeNoiseMode getBMDeNoiseMode(int ISO);

int   get_stereo_bmdenoise_zsd_cap_stored_frame_cnt(void);

// The working buffer count to allocate in bmdenoise feature pipe
size_t   get_stereo_bmdenoise_capture_buffer_cnt(void);

// The maximun number of concurrent capture requests
size_t   getBMDNCapturePhaseCnt(void);

// The number of hwsync base frame count
size_t   getHWSyncStableCnt(void);
// The number of capture "pairs" we used for BMDN shot
size_t   getBMDNCaptureCnt(void);
// The number of delay frame to resume original 3A setting
size_t   getBMDNDelayFrameCnt(void);

int getBMDNBitMode(void);

bool getIsPureZSD_BMDN(void);

/*******************************************************************************
* Stereo BM-DeNoise MFHR mode
*******************************************************************************/
// The maximun number of concurrent capture requests
size_t   getMFHRCapturePhaseCnt(void);
// The number of capture "pairs" we used for MFHR shot
size_t   getMFHRCaptureCnt(void);
// The number of delay frame to resume original 3A setting
size_t   getMFHRDelayFrameCnt(void);

bool getIsPureZSD_MFHR(void);

/*******************************************************************************
* Stereo BM-DeNoise tuning params
*******************************************************************************/
BMDeNoiseISODependentParam getBMDeNoiseISODePendentParams(int ISO);

BMDeNoiseSensorDependentParam getBMDeNoiseSensorDependentParam();

BMDeNoiseQualityPerformanceParam getBMDeNoiseQualityPerformanceParam();

BMDeNoiseFOVCrop getBMDeNoiseFOVCropParam();

int getMain2SwichISO(BMDeNoiseMain2Switch main2Switch);
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

enum ENUM_DENOISE_MODE
{
    E_DENOISE_MODE_NORMAL,
    E_DENOISE_MODE_PERFORMANCE
};

extern const ENUM_DENOISE_MODE DENOISE_MODE;

// tuning parameter for denoise shot mode select
struct BMDeNoiseCriteriaSettings
{
    int CUSTOMER_VAR_L_ISO_MID;    // mid iso 1600~3200
    int CUSTOMER_VAR_H_ISO_MID;
    int CUSTOMER_VAR_L_ISO_HIG;    // high iso 3200~5200
    int CUSTOMER_VAR_H_ISO_HIG;
    int CUSTOMER_VAR_L_ISO_EXT;    // extreme iso 5200~6400
    int CUSTOMER_VAR_H_ISO_EXT;

    int CUSTOMER_INT_L_ISO_MID;
    int CUSTOMER_INT_H_ISO_MID;
    int CUSTOMER_INT_L_ISO_HIG;
    int CUSTOMER_INT_H_ISO_HIG;
    int CUSTOMER_INT_L_ISO_EXT;
    int CUSTOMER_INT_H_ISO_EXT;

    int CUSTOMER_RECT_ISO_MID;
    int CUSTOMER_RECT_ISO_HIG;
    int CUSTOMER_RECT_ISO_EXT;

    int CUSTOMER_VAR_TOL_DEG_MID;
    int CUSTOMER_VAR_TOL_DEG_HIG;
    int CUSTOMER_VAR_TOL_DEG_EXT;

    int CUSTOMER_ISO_MID;
    int CUSTOMER_ISO_HIG;
    int CUSTOMER_ISO_BM_MIN;
    int CUSTOMER_ISO_HR_MIN;

    int CUSTOMER_FE_NUM_MIN;

    int CUSTOMER_DECISION_SMOOTH_THR_AE;
    int CUSTOMER_DECISION_SMOOTH_THR_AF;

    int CUSTOMER_DECISION_MARGIN_VAR;
    int CUSTOMER_DECISION_MARGIN_INT;

    BMDeNoiseCriteriaSettings();
} ;

MUINT8  get_vsdof_cap_queue_size();

MUINT8 canSupportBGService();

extern const char *DEFAULT_STEREO_TUNING;

int getStereoModeType();

int getStereoCaptureModeType();

bool supportMain2FullRaw();

bool supportDramControl();

bool needControlMmdvfs();

MINT64 getFrameSyncToleranceTime();

bool usingMultithreadForPipelineContext();
#endif  //  _CAMERA_CUSTOM_STEREO_IF_
