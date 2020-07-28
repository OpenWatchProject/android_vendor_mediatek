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

const std::set<DCMF_CAMERA_OUTPUTS> DCMF_CAMERA_OUTPUT_LIST =
{
    DCMF_CAMERA_OUTPUTS::eFullSize_YUV_1,
    DCMF_CAMERA_OUTPUTS::eFullSize_YUV_2,
    DCMF_CAMERA_OUTPUTS::eReSized_YUV_1,
    DCMF_CAMERA_OUTPUTS::eReSized_YUV_2
};

size_t getHWSyncStableCnt(){
    return 3;
}

int get_stereo_zsd_cap_stored_frame_cnt(){
    return 3;
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
