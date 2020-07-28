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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "camera_custom_stereo.h"
#include <vector>
#include <mtkcam/drv/IHalSensor.h>
#include <cutils/properties.h>
#include "camera_custom_stereo_tuning.h"
#include "camera_custom_stereo_setting.h"

using namespace NSCam;

const ENUN_DEPTHMAP_SIZE CUSTOM_DEPTHMAP_SIZE = STEREO_DEPTHMAP_2X;

const ENUM_DENOISE_MODE DENOISE_MODE = E_DENOISE_MODE_NORMAL;

const size_t DPE_CAPTURE_ROUND = 3;

const ENUM_WMF_CHANNEL WPE_INPUT_CHANNEL = E_WMF_Y_CHANNEL;

const std::vector<int> VSDOF_PREVIEW_CPU_CORE_TABLE =
{
    4, 4 // LL, L
};

const std::vector<int> VSDOF_PREVIEW_CPU_MAX_FREQUENCY_TABLE =
{
    -2, -2
};

const std::vector<int> VSDOF_PREVIEW_CPU_MIN_FREQUENCY_TABLE =
{
    -2, -2
};

const std::vector<int> VSDOF_RECORD_CPU_CORE_TABLE =
{
    4, 4
};

const std::vector<int> VSDOF_RECORD_CPU_MAX_FREQUENCY_TABLE =
{
    1066000, 1469000
};

const std::vector<int> VSDOF_RECORD_CPU_MIN_FREQUENCY_TABLE =
{
    1066000, 1469000
};

const std::vector<int> VSDOF_CAPTURE_CPU_CORE_TABLE =
{
    4, 4
};

const std::vector<int> VSDOF_CAPTURE_CPU_MAX_FREQUENCY_TABLE =
{
    -1, -1
};

const std::vector<int> VSDOF_CAPTURE_CPU_MIN_FREQUENCY_TABLE =
{
    -1, -1
};

const std::vector<int> BMDENOISE_PREVIEW_CPU_CORE_TABLE =
{
    4, 0
};

const std::vector<int> BMDENOISE_PREVIEW_CPU_MAX_FREQUENCY_TABLE =
{
    -1, 0
};

const std::vector<int> BMDENOISE_PREVIEW_CPU_MIN_FREQUENCY_TABLE =
{
    0, 0
};

const std::vector<int> BMDENOISE_CAPTURE_CPU_CORE_TABLE =
{
    4, 4
};

const std::vector<int> BMDENOISE_CAPTURE_CPU_MAX_FREQUENCY_TABLE =
{
    1690000, 2392000
};

const std::vector<int> BMDENOISE_CAPTURE_CPU_MIN_FREQUENCY_TABLE =
{
    1690000, 2392000
};

const std::set<DCMF_CAMERA_OUTPUTS> DCMF_CAMERA_OUTPUT_LIST =
{
    DCMF_CAMERA_OUTPUTS::eFullSize_YUV_1,
    DCMF_CAMERA_OUTPUTS::eFullSize_YUV_2,
    DCMF_CAMERA_OUTPUTS::eReSized_YUV_1,
    DCMF_CAMERA_OUTPUTS::eReSized_YUV_2
};

int get_stereo_zsd_cap_stored_frame_cnt(){
    return 3;
}

DeNoiseVersion getBMDeNoiseVersion(){
    return VERSION_1;
}

DeNoiseMode getBMDeNoiseMode(int ISO){
    if(ISO < 400){
        return eDeNoiseMode_NONE;
    }else if(ISO >= 400 && ISO < 800){
        return eDeNoiseMode_BM_MNR;
        // return eDeNoiseMode_BM;
    }else if(ISO >= 800){
        return eDeNoiseMode_BM_SWNR;
        // return eDeNoiseMode_BM;
    }else{
        return eDeNoiseMode_NONE;
    }
}

/*******************************************************************************
* Stereo BM-DeNoise BMDN mode
*******************************************************************************/

int get_stereo_bmdenoise_zsd_cap_stored_frame_cnt(){
    return 3;
}

size_t get_stereo_bmdenoise_capture_buffer_cnt(){
    return 2;
}

size_t getBMDNCapturePhaseCnt(){
    return 3;
}

size_t getHWSyncStableCnt(){
    return 3;
}

size_t getBMDNCaptureCnt(){
    return 1;
}

size_t getBMDNDelayFrameCnt(){
    return 0;
}

int getBMDNBitMode(){
    return 0;
}

bool getIsPureZSD_BMDN(){
    return false;
}

/*******************************************************************************
* Stereo BM-DeNoise MFHR mode
*******************************************************************************/

size_t getMFHRCapturePhaseCnt(){
    return 2;
}

size_t getMFHRCaptureCnt(){
    int value = property_get_int32("debug.bmdenoise.mfhr.cnt", -1);
    if(value != -1){
        return value;
    }else{
        return 3;
    }
}

size_t getMFHRDelayFrameCnt(){
    return 0;
}

bool getIsPureZSD_MFHR(){
    return false;
}

bool operator <(BMDeNoiseISODependentParam const& ms, int const i)
{
    return ms.ISO < i;
}

const std::vector<BMDeNoiseISODependentParam> AVAILABLE_ISO_SET =
{
    BMDeNoiseISODependentParam(100,   16,64,640,4,16,0,16,0, 2598,28635,8192),
    BMDeNoiseISODependentParam(200,   16,64,640,4,64,1,16,0, 2598,28635,8192),
    BMDeNoiseISODependentParam(400,   16,64,640,4,32,1,16,0, 2598,28635,8192),
    BMDeNoiseISODependentParam(800,   16,64,640,6,32,1,16,0, 2598,28635,8192),

    BMDeNoiseISODependentParam(1200,  16,64,640,6,16,1,16,0, 2598,28635,8192),
    BMDeNoiseISODependentParam(1600,  16,64,640,8,16,2,16,0, 2598,28635,8192),
    BMDeNoiseISODependentParam(2000,  16,64,640,8,16,3,16,0, 2598,28635,8192),
    BMDeNoiseISODependentParam(2400,  20,64,640,10,16,3,16,0,2598,28635,8192),

    BMDeNoiseISODependentParam(2800,  24,64,640,12,16,3,16,1,2598,28635,8192),
    BMDeNoiseISODependentParam(3200,  28,64,640,14,16,4,16,1,2598,28635,8192),
    BMDeNoiseISODependentParam(4000,  36,64,640,16,12,4,16,1,2598,28635,8192),
    BMDeNoiseISODependentParam(4800,  40,64,640,16,12,4,16,1,2598,28635,8192),

    BMDeNoiseISODependentParam(6400,  16,64,640,16,8,4,16,2, 2598,28635,8192),
    BMDeNoiseISODependentParam(12800, 16,64,640,16,4,4,16,3, 2598,28635,8192)
};

BMDeNoiseISODependentParam getBMDeNoiseISODePendentParams(int targrtISO){

    // find the lower bound
    auto const it = std::lower_bound(AVAILABLE_ISO_SET.begin(), AVAILABLE_ISO_SET.end(), targrtISO);

    // if boundary, return directly
    if(it == AVAILABLE_ISO_SET.begin() || it == AVAILABLE_ISO_SET.end() - 1){
        return (*it);
    }

    // compare with the previous element too
    int lower_bound_diff = abs((*it).ISO - targrtISO);
    int prv_element_diff = abs( (*(it-1)).ISO - targrtISO);

    // return the nearest one
    if(lower_bound_diff <= prv_element_diff){
        return (*it);
    }else{
        return (*(it-1));
    }
}

BMDeNoiseSensorDependentParam getBMDeNoiseSensorDependentParam(){
    BMDeNoiseSensorDependentParam sensorDependentParam;

    // use default value
    return sensorDependentParam;
}

BMDeNoiseQualityPerformanceParam getBMDeNoiseQualityPerformanceParam(){
    BMDeNoiseQualityPerformanceParam ret;

    // use default value
    return ret;
}

BMDeNoiseFOVCrop getBMDeNoiseFOVCropParam(){
    BMDeNoiseFOVCrop ret;
    return ret;
}

int getMain2SwichISO(BMDeNoiseMain2Switch main2Switch)
{
    if(main2Switch == BMDeNoiseMain2Switch::Off)
        return 800;
    else
        return 1600;
}

SensorModuleConfig getSensorModuleConfig()
{
    return SensorModuleConfig::MainAtBottom;
}

float getWorkingRangeNear()
{
    return 20.0f;
}

float getAdditionalToleranceCoeff()
{
    return 1.0f;
}

float getRelRotToleranceXY()
{
    return 1.0f;
}

std::vector<int> get_stereo_cam_cpu_num(
    ENUM_STEREO_CAM_FEATURE featureId,
    ENUM_STEREO_CAM_SCENARIO scenario
)
{
    switch(featureId)
    {
        case eStereoCamVsdof:
            {
                if(eStereoCamPreview == scenario)
                {
                    return VSDOF_PREVIEW_CPU_CORE_TABLE;
                }
                else if(eStereoCamRecord == scenario)
                {
                    return VSDOF_RECORD_CPU_CORE_TABLE;
                }
                else if(eStereoCamCapture == scenario)
                {
                    return VSDOF_CAPTURE_CPU_CORE_TABLE;
                }
                else
                {
                    return std::vector<int>();
                }
            }
            break;
        case eStereoCamStereo_Capture:
            {
                if(eStereoCamPreview == scenario)
                {
                    return VSDOF_PREVIEW_CPU_CORE_TABLE;
                }
                else if(eStereoCamCapture == scenario)
                {
                    return VSDOF_CAPTURE_CPU_CORE_TABLE;
                }
                else
                {
                    return std::vector<int>();
                }
            }
            break;
        case eStereoBMDnoise:
            {
                if(eStereoCamPreview == scenario)
                {
                    return BMDENOISE_PREVIEW_CPU_CORE_TABLE;
                }
                else if(eStereoCamCapture == scenario)
                {
                    std::vector<int> cpu_core = BMDENOISE_CAPTURE_CPU_CORE_TABLE;
                    char cProperty[PROPERTY_VALUE_MAX];
                    MINT32 value;

                    property_get("debug.bmdenoise.coreLL", cProperty, "-1");
                    value = ::atoi(cProperty);
                    if(value != -1){
                        cpu_core[0] = value;
                    }
                    property_get("debug.bmdenoise.coreL", cProperty, "-1");
                    value = ::atoi(cProperty);
                    if(value != -1){
                        cpu_core[1] = value;
                    }
                    property_get("debug.bmdenoise.coreB", cProperty, "-1");
                    value = ::atoi(cProperty);
                    if(value != -1){
                        cpu_core[2] = value;
                    }
                    return cpu_core;
                }
                else
                {
                    return std::vector<int>();
                }
            }
            break;
        default:
                return std::vector<int>();
            break;
    }
}

bool get_stereo_cam_cpu_frequency(
    ENUM_STEREO_CAM_FEATURE featureId,
    ENUM_STEREO_CAM_SCENARIO scenario,
    std::vector<int>& min,
    std::vector<int>& max
)
{
    min.clear();
    max.clear();
    switch(featureId)
    {
        case eStereoCamVsdof:
            {
                if(eStereoCamPreview == scenario)
                {
                    min = VSDOF_PREVIEW_CPU_MIN_FREQUENCY_TABLE;
                    max = VSDOF_PREVIEW_CPU_MAX_FREQUENCY_TABLE;
                }
                else if(eStereoCamRecord == scenario)
                {
                    min = VSDOF_RECORD_CPU_MIN_FREQUENCY_TABLE;
                    max = VSDOF_RECORD_CPU_MAX_FREQUENCY_TABLE;
                }
                else if(eStereoCamCapture == scenario)
                {
                    min = VSDOF_CAPTURE_CPU_MIN_FREQUENCY_TABLE;
                    max = VSDOF_CAPTURE_CPU_MAX_FREQUENCY_TABLE;
                }
                else
                {
                    return false;
                }
            }
            break;
        case eStereoCamStereo_Capture:
            {
                if(eStereoCamPreview == scenario)
                {
                    min = VSDOF_PREVIEW_CPU_MIN_FREQUENCY_TABLE;
                    max = VSDOF_PREVIEW_CPU_MAX_FREQUENCY_TABLE;
                }
                else if(eStereoCamCapture == scenario)
                {
                    min = VSDOF_CAPTURE_CPU_MIN_FREQUENCY_TABLE;
                    max = VSDOF_CAPTURE_CPU_MAX_FREQUENCY_TABLE;
                }
                else
                {
                    return false;
                }
            }
            break;
        case eStereoBMDnoise:
            {
                if(eStereoCamPreview == scenario)
                {
                    min = BMDENOISE_PREVIEW_CPU_MIN_FREQUENCY_TABLE;
                    max = BMDENOISE_PREVIEW_CPU_MAX_FREQUENCY_TABLE;
                }
                else if(eStereoCamCapture == scenario)
                {
                    min = BMDENOISE_CAPTURE_CPU_MIN_FREQUENCY_TABLE;
                    max = BMDENOISE_CAPTURE_CPU_MAX_FREQUENCY_TABLE;
                    char cProperty[PROPERTY_VALUE_MAX];
                    MINT32 value;

                    property_get("debug.bmdenoise.freqMinLL", cProperty, "-2");
                    value = ::atoi(cProperty);
                    if(value != -2){
                        min[0] = value;
                    }
                    property_get("debug.bmdenoise.freqMaxLL", cProperty, "-2");
                    value = ::atoi(cProperty);
                    if(value != -2){
                        max[0] = value;
                    }
                    property_get("debug.bmdenoise.freqMinL", cProperty, "-2");
                    value = ::atoi(cProperty);
                    if(value != -2){
                        min[1] = value;
                    }
                    property_get("debug.bmdenoise.freqMaxL", cProperty, "-2");
                    value = ::atoi(cProperty);
                    if(value != -2){
                        max[1] = value;
                    }
                    property_get("debug.bmdenoise.freqMinB", cProperty, "-2");
                    value = ::atoi(cProperty);
                    if(value != -2){
                        min[2] = value;
                    }
                    property_get("debug.bmdenoise.freqMaxB", cProperty, "-2");
                    value = ::atoi(cProperty);
                    if(value != -2){
                        max[2] = value;
                    }
                }
                else
                {
                    return false;
                }
            }
            break;
        default:
                return false;
            break;
    }
    return true;
}

// tuning parameter for denoise shot mode select
BMDeNoiseCriteriaSettings::
BMDeNoiseCriteriaSettings()
{
    CUSTOMER_VAR_L_ISO_MID =  4;    // mid iso 1600~3200
    CUSTOMER_VAR_H_ISO_MID = 25;
    CUSTOMER_VAR_L_ISO_HIG =  8;    // high iso 3200~5200
    CUSTOMER_VAR_H_ISO_HIG = 49;
    CUSTOMER_VAR_L_ISO_EXT = 12;    // extreme iso 5200~6400
    CUSTOMER_VAR_H_ISO_EXT = 81;

    CUSTOMER_INT_L_ISO_MID =  25;
    CUSTOMER_INT_H_ISO_MID =  60;
    CUSTOMER_INT_L_ISO_HIG =  28;
    CUSTOMER_INT_H_ISO_HIG =  80;
    CUSTOMER_INT_L_ISO_EXT =  31;
    CUSTOMER_INT_H_ISO_EXT = 100;

    CUSTOMER_RECT_ISO_MID =  4 ;
    CUSTOMER_RECT_ISO_HIG =  5 ;
    CUSTOMER_RECT_ISO_EXT =  5 ;

    CUSTOMER_VAR_TOL_DEG_MID = 0;
    CUSTOMER_VAR_TOL_DEG_HIG = 1;
    CUSTOMER_VAR_TOL_DEG_EXT = 1;

    CUSTOMER_ISO_MID = 3200;
    CUSTOMER_ISO_HIG = 5200;
    CUSTOMER_ISO_BM_MIN = 1600;
    CUSTOMER_ISO_HR_MIN = 3200;

    CUSTOMER_FE_NUM_MIN = 5;

    CUSTOMER_DECISION_SMOOTH_THR_AE = 6;
    CUSTOMER_DECISION_SMOOTH_THR_AF = 12;

    CUSTOMER_DECISION_MARGIN_VAR = 3;
    CUSTOMER_DECISION_MARGIN_INT = 3;

    CUSTOMER_MACRO_BOUND_THR    = 0;
    CUSTOMER_BOUNDARY_THR       = 10;
    CUSTOMER_VAR_THR            = 10000;
}

MUINT8 get_vsdof_cap_queue_size()
{
    return 3;
}

MUINT8 canSupportBGService()
{
    return 1 ;
}


/*******************************************************************************
* Stereo DCMF(Dual-Cam Multi-Frame) tuning params
*******************************************************************************/
int getDCMFCaptureCnt()
{
    return 1;
}

int getDCMFCapturePhaseCnt()
{
    return 1;
}

bool getIsPureZSD_DCMF()
{
    return true;
}

bool getIsCameraCallback()
{
    return false;
}

int getSWNRExecutedRealIsoThreshold()
{
    return 200;
}

std::vector<int> getMFLLExecutedRealIsoThresholds()
{
    return {200, 1600};
}

bool getIsSuportedFDToHDRInHDRBokehMode()
{
    return false;
}

bool getIsSuportedFDToDepthInMFNRBokehMode()
{
    return false;
}

// ===============================================
// 0: tk
// 1: pure 3rd
// 2: tk depth + 3rd
// ===============================================
int getStereoModeType()
{
    return 1;
}

// ===============================================
// 0: tk
// 1: pure 3rd
// ===============================================
int getStereoCaptureModeType()
{
    return 1;
}

bool supportMain2FullRaw()
{
    return true;
}

bool supportDramControl()
{
    return true;
}

// for isp 3.0 & 4.0, it has to set mmdvfs level to high for hwsync.
bool needControlMmdvfs()
{
    return false;
}

// unit: microsecond
MINT64 getFrameSyncToleranceTime()
{
    return 1000;
}

bool usingMultithreadForPipelineContext()
{
    return true;
}
