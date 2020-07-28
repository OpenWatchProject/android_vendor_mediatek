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

const size_t DPE_CAPTURE_ROUND = 3;

const ENUM_WMF_CHANNEL WPE_INPUT_CHANNEL = E_WMF_Y_CHANNEL;

const std::vector<int> VSDOF_PREVIEW_CPU_CORE_TABLE =
{
    4, 4 // LL, L
};

const std::vector<int> VSDOF_PREVIEW_CPU_MAX_FREQUENCY_TABLE =
{
    1484000, 1050000
};

const std::vector<int> VSDOF_PREVIEW_CPU_MIN_FREQUENCY_TABLE =
{
    1484000, 1050000
};

const std::vector<int> VSDOF_RECORD_CPU_CORE_TABLE =
{
    4, 4
};

const std::vector<int> VSDOF_RECORD_CPU_MAX_FREQUENCY_TABLE =
{
    1484000, 1050000
};

const std::vector<int> VSDOF_RECORD_CPU_MIN_FREQUENCY_TABLE =
{
    1484000, 1050000
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
    4, 2
};

const std::vector<int> BMDENOISE_CAPTURE_CPU_MAX_FREQUENCY_TABLE =
{
    -1, 1625000
};

const std::vector<int> BMDENOISE_CAPTURE_CPU_MIN_FREQUENCY_TABLE =
{
    -1, 1625000
};

int get_stereo_zsd_cap_stored_frame_cnt(){
    return 3;
}

DeNoiseMode getBMDeNoiseMode(int ISO){
    if(ISO < 400){
        return eDeNoiseMode_NONE;
    }
    return eDeNoiseMode_BM;
}

int get_stereo_bmdenoise_zsd_cap_stored_frame_cnt(){
    return 3;
}

size_t get_stereo_bmdenoise_capture_buffer_cnt(){
    return 3;
}

bool operator <(BMDeNoiseISODependentParam const& ms, int const i)
{
    return ms.ISO < i;
}

const std::vector<BMDeNoiseISODependentParam> AVAILABLE_ISO_SET =
{
    BMDeNoiseISODependentParam(100, 16,32,640,2,16,0,16,0,446,31591,1024),
    BMDeNoiseISODependentParam(200, 16,32,640,4,16,0,16,0,778,32094,2048),
    BMDeNoiseISODependentParam(400, 16,32,640,4,16,1,16,0,1416,32398,4096),
    BMDeNoiseISODependentParam(800, 16,32,640,6,16,1,16,0,2743,32577,8192),

    BMDeNoiseISODependentParam(1200, 16,32,640,8,16,2,16,0,2743,32577,8192),
    BMDeNoiseISODependentParam(1600, 16,32,640,8,16,2,16,0,5270,29284,16384),
    BMDeNoiseISODependentParam(2000, 19,32,640,10,16,2,16,0,5270,29284,16384),
    BMDeNoiseISODependentParam(2400, 22,32,640,10,16,3,16,0,5270,29284,16384),

    BMDeNoiseISODependentParam(2800, 25,32,640,12,16,3,16,0,5270,29284,16384),
    BMDeNoiseISODependentParam(3200, 28,32,640,14,16,4,16,0,5270,29284,16384),
    BMDeNoiseISODependentParam(4000, 34,32,640,14,12,4,16,1,5270,29284,16384),
    BMDeNoiseISODependentParam(4800, 40,32,640,16,12,4,16,1,5270,29284,16384),
    BMDeNoiseISODependentParam(6400, 46,32,640,16,8,4,16,2,5270,29284,16384)
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
                    return BMDENOISE_PREVIEW_CPU_CORE_TABLE;
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
                    min = BMDENOISE_PREVIEW_CPU_MIN_FREQUENCY_TABLE;
                    max = BMDENOISE_PREVIEW_CPU_MAX_FREQUENCY_TABLE;
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
        default:
                return false;
            break;
    }
    return true;
}

MUINT8 get_vsdof_cap_queue_size()
{
    return 3;
}

MUINT8 canSupportBGService()
{
    return 1 ;
}

// ===============================================
// 0: tk
// 1: pure 3rd
// 2: tk depth + 3rd
// ===============================================
int getStereoModeType()
{
    return 0;
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
    return true;
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