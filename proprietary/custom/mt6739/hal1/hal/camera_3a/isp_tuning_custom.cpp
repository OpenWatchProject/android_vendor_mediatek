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

#define LOG_TAG "isp_tuning_custom"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_tuning_custom_swnr.h>
#include <isp_tuning_custom_instance.h>
#include <stdlib.h>				// For atoi()
#include <stdio.h>
#include <cutils/properties.h>	// For property_get().
#include "camera_custom_3dnr.h"

using namespace NSIspTuning;

static MINT32 customRatioLog=0;

// ais
/*
 *  disable sl2a & sl2b. enable slb2 for ais stage3/4, disable for other cases
 *  make sure shading_tuning_custom.cpp & isp_tuning_custom.cpp use the same value
 */
#define TUNING_FOR_AIS  1
#define AIS_INTERPOLATION   0

static float AIS_NORMAL_CFA_RATIO[4] = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively
static float AIS_NORMAL_YNR_RATIO[4] = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively
static float AIS_NORMAL_CNR_RATIO[4] = {0.20f, 1.00f, 0.20f, 0.20f}; //0=ais, 1=normal, for stage1-4 respectively
static float AIS_NORMAL_EE_RATIO[4]  = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively

#define IS_AIS          (rCamInfo.rAEInfo.u4OrgRealISOValue != rCamInfo.u4ISOValue)

MUINT32 WEIGHTING(MUINT32 x, MUINT32 y, float w) {
    MUINT32 z = (((x)*(w))+((y)*(1.0f-(w))));
    return z;
}

MINT32 AIS_Profile2Stage(MUINT32 profile) {
    MINT32 stage = -1;
    switch(profile) {
        case EIspProfile_MFB_Capture_EE_Off:
        case EIspProfile_VSS_MFB_Capture_EE_Off:
        case EIspProfile_MFB_Capture_EE_Off_SWNR:
        case EIspProfile_VSS_MFB_Capture_EE_Off_SWNR:
        case EIspProfile_MFB_PostProc_EE_Off:
        case EIspProfile_VSS_MFB_PostProc_EE_Off:
            stage = 1;
            break;

        case EIspProfile_MFB_Blending_All_Off:
        case EIspProfile_VSS_MFB_Blending_All_Off:
        case EIspProfile_MFB_Blending_All_Off_SWNR:
        case EIspProfile_VSS_MFB_Blending_All_Off_SWNR:
            stage = 2;
            break;

        case EIspProfile_MFB_PostProc_ANR_EE:
        case EIspProfile_VSS_MFB_PostProc_ANR_EE:
        case EIspProfile_MFB_PostProc_ANR_EE_SWNR:
        case EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR:
            stage = 3;
            break;

        case EIspProfile_MFB_PostProc_Mixing:
        case EIspProfile_VSS_MFB_PostProc_Mixing:
        case EIspProfile_MFB_PostProc_Mixing_SWNR:
        case EIspProfile_VSS_MFB_PostProc_Mixing_SWNR:
            stage = 4;
            break;

        default:
            stage = -1;
    }

    return stage;
}


MUINT32 get_normal_SWNR_ENC_enable_ISO_threshold()
{
#define NORMAL_SWNR_ENC_ENABLE_ISO_THRESHOLD (800) // enable if ISO >= THRESHOLD
    return NORMAL_SWNR_ENC_ENABLE_ISO_THRESHOLD;
}

MUINT32 get_MFB_SWNR_ENC_enable_ISO_threshold()
{
#define MFB_SWNR_ENC_ENABLE_ISO_THRESHOLD (800) // enable if ISO >= THRESHOLD
    return MFB_SWNR_ENC_ENABLE_ISO_THRESHOLD;
}

/*******************************************************************************
*
*   rCamInfo
*       [in]    ISP Camera Info for RAW sensor. Its members are as below:
*
*           eIspProfile:
*               EIspProfile_Preview = 0,          // Preview
*               EIspProfile_Video,                // Video
*               EIspProfile_Capture,              // Capture
*               EIspProfile_ZSD_Capture,          // ZSD Capture
*               EIspProfile_VSS_Capture,          // VSS Capture
*               EIspProfile_PureRAW_Capture,      // Pure RAW Capture
*               EIspProfile_N3D_Preview,          // N3D Preview
*               EIspProfile_N3D_Video,            // N3D Video
*               EIspProfile_N3D_Capture,          // N3D Capture
*               EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
*               EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
*               EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
*               EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
*               EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
*               EIspProfile_VFB_PostProc,         // VFB post process: all off + ANR + CCR + PCA
*               EIspProfile_IHDR_Preview,         // IHDR preview
*               EIspProfile_IHDR_Video,           // IHDR video
*
*           eSensorMode:
*               ESensorMode_Preview = 0,
*               ESensorMode_Capture,
*               ESensorMode_Video,
*               ESensorMode_SlimVideo1,
*               ESensorMode_SlimVideo2,
*
*           eIdx_Scene:
*               MTK_CONTROL_SCENE_MODE_UNSUPPORTED = 0,
*               MTK_CONTROL_SCENE_MODE_FACE_PRIORITY,
*               MTK_CONTROL_SCENE_MODE_ACTION,
*               MTK_CONTROL_SCENE_MODE_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_LANDSCAPE,
*               MTK_CONTROL_SCENE_MODE_NIGHT,
*               MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_THEATRE,
*               MTK_CONTROL_SCENE_MODE_BEACH,
*               MTK_CONTROL_SCENE_MODE_SNOW,
*               MTK_CONTROL_SCENE_MODE_SUNSET,
*               MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
*               MTK_CONTROL_SCENE_MODE_FIREWORKS,
*               MTK_CONTROL_SCENE_MODE_SPORTS,
*               MTK_CONTROL_SCENE_MODE_PARTY,
*               MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
*               MTK_CONTROL_SCENE_MODE_BARCODE,
*               MTK_CONTROL_SCENE_MODE_NORMAL,
*               MTK_CONTROL_SCENE_MODE_HDR,
*
*           u4ISOValue:
*               ISO value to determine eISO.
*
*           eIdx_ISO:
*               eIDX_ISO_100 = 0,
*               eIDX_ISO_200,
*               eIDX_ISO_400,
*               eIDX_ISO_800,
*               eIDX_ISO_1200,
*               eIDX_ISO_1600,
*               eIDX_ISO_2000,
*               eIDX_ISO_2400,
*               eIDX_ISO_2800,
*               eIDX_ISO_3200,
*
*           eIdx_PCA_LUT:
*               eIDX_PCA_LOW  = 0,
*               eIDX_PCA_MIDDLE,
*               eIDX_PCA_HIGH,
*               eIDX_PCA_LOW_2,    // for video HDR only
*               eIDX_PCA_MIDDLE_2, // for video HDR only
*               eIDX_PCA_HIGH_2    // for video HDR only
*
*           eIdx_CCM:
*               eIDX_CCM_D65  = 0,
*               eIDX_CCM_TL84,
*               eIDX_CCM_CWF,
*               eIDX_CCM_A,
*
*           eIdx_Shading_CCT:
*               eIDX_Shading_CCT_BEGIN = 0,
*               eIDX_Shading_CCT_ALight = eIDX_Shading_CCT_BEGIN,
*               eIDX_Shading_CCT_CWF,
*               eIDX_Shading_CCT_D65,
*               eIDX_Shading_CCT_RSVD
*
*           rAWBInfo:
*               rProb; // Light source probability
*               rLightStat; // Light source statistics
*               rLightAWBGain; // Golden sample's AWB gain for multi-CCM
*               rCurrentAWBGain; // Current preview AWB gain
*               i4NeutralParentBlkNum; // Neutral parent block number
*               i4CCT; // CCT
*               i4FluorescentIndex; // Fluorescent index
*               i4DaylightFluorescentIndex; // Daylight fluorescent index
*               i4SceneLV; // Scene LV
*               i4AWBMode; // AWB mode
*               bAWBStable; // AWB stable
*
*           rAEInfo:
*               u4AETarget;
*               u4AECurrentTarget;
*               u4Eposuretime;
*               u4AfeGain;
*               u4IspGain;
*               u4RealISOValue;
*               i4LightValue_x10;
*               u4AECondition;
*               eAEMeterMode;
*               i2FlareOffset;
*               u2Histogrm[AE_HISTOGRAM_BIN];
*           rAFInfo:
*               i4AFPos
*
*           rFlashInfo:
*               flashMode;
*               isFlash; // 0: no flash, 1: image with flash
*
*           u4ZoomRatio_x100:
*               zoom ratio (x100)
*
*           i4LightValue_x10:
*               light value (x10)
*
*******************************************************************************/
MVOID
IspTuningCustom::
refine_CamInfo(RAWIspCamInfo& rCamInfo)
{
}

/*******************************************************************************
*
*   rCamInfo
*       [in]    ISP Camera Info for RAW sensor. Its members are as below:
*
*           eIspProfile:
*               EIspProfile_Preview = 0,          // Preview
*               EIspProfile_Video,                // Video
*               EIspProfile_Capture,              // Capture
*               EIspProfile_ZSD_Capture,          // ZSD Capture
*               EIspProfile_VSS_Capture,          // VSS Capture
*               EIspProfile_PureRAW_Capture,      // Pure RAW Capture
*               EIspProfile_N3D_Preview,          // N3D Preview
*               EIspProfile_N3D_Video,            // N3D Video
*               EIspProfile_N3D_Capture,          // N3D Capture
*               EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
*               EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
*               EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
*               EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
*               EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
*               EIspProfile_VFB_PostProc,         // VFB post process: all off + ANR + CCR + PCA
*               EIspProfile_IHDR_Preview,         // IHDR preview
*               EIspProfile_IHDR_Video,           // IHDR video
*
*           eSensorMode:
*               ESensorMode_Preview = 0,
*               ESensorMode_Capture,
*               ESensorMode_Video,
*               ESensorMode_SlimVideo1,
*               ESensorMode_SlimVideo2,
*
*           eIdx_Scene:
*               MTK_CONTROL_SCENE_MODE_UNSUPPORTED = 0,
*               MTK_CONTROL_SCENE_MODE_FACE_PRIORITY,
*               MTK_CONTROL_SCENE_MODE_ACTION,
*               MTK_CONTROL_SCENE_MODE_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_LANDSCAPE,
*               MTK_CONTROL_SCENE_MODE_NIGHT,
*               MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_THEATRE,
*               MTK_CONTROL_SCENE_MODE_BEACH,
*               MTK_CONTROL_SCENE_MODE_SNOW,
*               MTK_CONTROL_SCENE_MODE_SUNSET,
*               MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
*               MTK_CONTROL_SCENE_MODE_FIREWORKS,
*               MTK_CONTROL_SCENE_MODE_SPORTS,
*               MTK_CONTROL_SCENE_MODE_PARTY,
*               MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
*               MTK_CONTROL_SCENE_MODE_BARCODE,
*               MTK_CONTROL_SCENE_MODE_NORMAL,
*               MTK_CONTROL_SCENE_MODE_HDR,
*
*           u4ISOValue:
*               ISO value to determine eISO.
*
*           eIdx_ISO:
*               eIDX_ISO_100 = 0,
*               eIDX_ISO_200,
*               eIDX_ISO_400,
*               eIDX_ISO_800,
*               eIDX_ISO_1200,
*               eIDX_ISO_1600,
*               eIDX_ISO_2000,
*               eIDX_ISO_2400,
*               eIDX_ISO_2800,
*               eIDX_ISO_3200,
*
*           eIdx_PCA_LUT:
*               eIDX_PCA_LOW  = 0,
*               eIDX_PCA_MIDDLE,
*               eIDX_PCA_HIGH,
*               eIDX_PCA_LOW_2,    // for video HDR only
*               eIDX_PCA_MIDDLE_2, // for video HDR only
*               eIDX_PCA_HIGH_2    // for video HDR only
*
*           eIdx_CCM:
*               eIDX_CCM_D65  = 0,
*               eIDX_CCM_TL84,
*               eIDX_CCM_CWF,
*               eIDX_CCM_A,
*
*           eIdx_Shading_CCT:
*               eIDX_Shading_CCT_BEGIN = 0,
*               eIDX_Shading_CCT_ALight = eIDX_Shading_CCT_BEGIN,
*               eIDX_Shading_CCT_CWF,
*               eIDX_Shading_CCT_D65,
*               eIDX_Shading_CCT_RSVD
*
*           rAWBInfo:
*               rProb; // Light source probability
*               rLightStat; // Light source statistics
*               rLightAWBGain; // Golden sample's AWB gain for multi-CCM
*               rCurrentAWBGain; // Current preview AWB gain
*               i4NeutralParentBlkNum; // Neutral parent block number
*               i4CCT; // CCT
*               i4FluorescentIndex; // Fluorescent index
*               i4DaylightFluorescentIndex; // Daylight fluorescent index
*               i4SceneLV; // Scene LV
*               i4AWBMode; // AWB mode
*               bAWBStable; // AWB stable
*
*           rAEInfo:
*               u4AETarget;
*               u4AECurrentTarget;
*               u4Eposuretime;
*               u4AfeGain;
*               u4IspGain;
*               u4RealISOValue;
*               i4LightValue_x10;
*               u4AECondition;
*               eAEMeterMode;
*               i2FlareOffset;
*               u2Histogrm[AE_HISTOGRAM_BIN];
*           rAFInfo:
*               i4AFPos
*
*           rFlashInfo:
*               flashMode;
*               isFlash; // 0: no flash, 1: image with flash
*
*           u4ZoomRatio_x100:
*               zoom ratio (x100)
*
*           i4LightValue_x10:
*               light value (x10)
*
*   rIdxMgr:
*       [in]    The default ISP tuning index manager.
*       [out]   The ISP tuning index manager after customizing.
*
*
*******************************************************************************/
MVOID
IspTuningCustom::
evaluate_nvram_index(RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr)
{
//..............................................................................
    //  (1) Dump info. before customizing.
#if 0
//#if ENABLE_MY_LOG
    rCamInfo.dump();
#endif

#if 0
    LOGD("[+evaluate_nvram_index][before customizing]");
    rIdxMgr.dump();
#endif
//..............................................................................
    //  (2) Modify each index based on conditions.
    //
    //  setIdx_XXX() returns:
    //      MTURE: if successful
    //      MFALSE: if the input index is out of range.
    //
#if 0
    fgRet = rIdxMgr.setIdx_OBC(XXX);
    fgRet = rIdxMgr.setIdx_BPC(XXX);
    fgRet = rIdxMgr.setIdx_NR1(XXX);
    fgRet = rIdxMgr.setIdx_CFA(XXX);
    fgRet = rIdxMgr.setIdx_GGM(XXX);
    fgRet = rIdxMgr.setIdx_ANR(XXX);
    fgRet = rIdxMgr.setIdx_CCR(XXX);
    fgRet = rIdxMgr.setIdx_EE(XXX);
#endif
/*
if(rCamInfo.u4ISOValue > 500 && rCamInfo.u4ISOValue < 700)
	{
		switch(rCamInfo.eIspProfile)
		{
			case EIspProfile_Preview:
				if( rCamInfo.eSensorMode == ESensorMode_Preview)
				{
				    rIdxMgr.setIdx_CFA(9);
				    rIdxMgr.setIdx_ANR(9);
				    rIdxMgr.setIdx_EE(9);
				}
				else if( rCamInfo.eSensorMode == ESensorMode_Capture)
				{
					rIdxMgr.setIdx_CFA(19);
					rIdxMgr.setIdx_ANR(19);
					rIdxMgr.setIdx_EE(19);
				}
				break;
			case EIspProfile_Capture:
			case EIspProfile_ZSD_Capture:
				if( rCamInfo.eSensorMode == ESensorMode_Capture)
				{
					rIdxMgr.setIdx_CFA(119);
					rIdxMgr.setIdx_ANR(119);
					rIdxMgr.setIdx_EE(119);
				}
				break;
			case EIspProfile_MFB_PostProc_ANR_EE:
			case EIspProfile_MFB_PostProc_ANR_EE_SWNR:
				rIdxMgr.setIdx_ANR(199);
			break;
			case EIspProfile_MFB_PostProc_Mixing:
			case EIspProfile_MFB_PostProc_Mixing_SWNR:
				rIdxMgr.setIdx_EE(189);
			break;
			default:
				break;
		}
	}


	if(rCamInfo.eIdx_ISO == eIDX_ISO_3200)
	{
		switch(rCamInfo.eIspProfile)
		{
			case EIspProfile_Preview:
				if( rCamInfo.eSensorMode == ESensorMode_Preview)
				{
				    rIdxMgr.setIdx_CFA(8);
				    rIdxMgr.setIdx_ANR(8);
				    rIdxMgr.setIdx_EE(8);
				}
				else if( rCamInfo.eSensorMode == ESensorMode_Capture)
				{
					rIdxMgr.setIdx_CFA(18);
					rIdxMgr.setIdx_ANR(18);
					rIdxMgr.setIdx_EE(18);
				}
				break;
			case EIspProfile_Capture:
			case EIspProfile_ZSD_Capture:
				if( rCamInfo.eSensorMode == ESensorMode_Capture)
				{
					rIdxMgr.setIdx_CFA(118);
					rIdxMgr.setIdx_ANR(118);
					rIdxMgr.setIdx_EE(118);
				}
				break;
			default:
				break;
		}

	}

	if( rCamInfo.eSensorMode == ESensorMode_Capture)
	{
		if(rCamInfo.rAEInfo.u4AfeGain >= 6144)
		{
			rIdxMgr.setIdx_OBC(3);
		}
	}
	*/


    if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            if(rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_EE_Off
                //|| rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off
                //|| rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR
            )
            {
                MUINT32 stage = AIS_Profile2Stage(rCamInfo.eIspProfile);

                MUINT32 normalIso = rCamInfo.rAEInfo.u4OrgRealISOValue;
                MUINT32 aisIso = rCamInfo.rAEInfo.u4RealISOValue;
                MUINT32 cfaIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_CFA_RATIO[stage-1]);
                MUINT32 ynrIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_YNR_RATIO[stage-1]);
                MUINT32 eeIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_EE_RATIO[stage-1]);

                MINT32 normalIndex = map_ISO_value_to_index(normalIso);
                MINT32 aisIndex = map_ISO_value_to_index(aisIso);

                // base on AIS index
                MINT32 deltaCfa = map_ISO_value_to_index(cfaIso) - aisIndex;
                MINT32 deltaYnr = map_ISO_value_to_index(ynrIso) - aisIndex;
                MINT32 deltaEE = map_ISO_value_to_index(eeIso) - aisIndex;

                if(rIdxMgr.getIdx_CFA() != 0)   rIdxMgr.setIdx_CFA(rIdxMgr.getIdx_CFA() + deltaCfa);
                if(rIdxMgr.getIdx_ANR() != 0)   rIdxMgr.setIdx_ANR(rIdxMgr.getIdx_ANR() + deltaYnr);
                //if(rIdxMgr.getIdx_EE() != 0)    rIdxMgr.setIdx_EE(rIdxMgr.getIdx_EE() + deltaEE);
            }
        }
    }
//..............................................................................
    //  (3) Finally, dump info. after modifying.
#if 0
    LOGD("[-evaluate_nvram_index][after customizing]");
    rIdxMgr.dump();
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   rIspRegMgr
//       m_rRegs: ISP NVRAM register
//           ISP_NVRAM_OBC_T    OBC[NVRAM_OBC_TBL_NUM];
//           ISP_NVRAM_BPC_T    BPC[NVRAM_BPC_TBL_NUM];
//           ISP_NVRAM_NR1_T    NR1[NVRAM_NR1_TBL_NUM];
//           ISP_NVRAM_LSC_T    LSC[NVRAM_LSC_TBL_NUM];
//           ISP_NVRAM_SL2_T    SL2[NVRAM_SL2_TBL_NUM];
//           ISP_NVRAM_CFA_T    CFA[NVRAM_CFA_TBL_NUM];
//           ISP_NVRAM_CCM_T    CCM[NVRAM_CCM_TBL_NUM];
//           ISP_NVRAM_GGM_T    GGM[NVRAM_GGM_TBL_NUM];
//           ISP_NVRAM_GGM_T    IHDR_GGM[NVRAM_IHDR_GGM_TBL_NUM];
//           ISP_NVRAM_ANR_T    ANR[NVRAM_ANR_TBL_NUM];
//           ISP_NVRAM_CCR_T    CCR[NVRAM_CCR_TBL_NUM];
//           ISP_NVRAM_EE_T     EE[NVRAM_EE_TBL_NUM];
//           ISP_NVRAM_NR3D_T   NR3D[NVRAM_NR3D_TBL_NUM];
//           ISP_NVRAM_MFB_T    MFB[NVRAM_MFB_TBL_NUM];
//       m_rIdx: current NVRAM index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_DBS(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_DBS_T& rDBS)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    if (getSensorDev() == ESensorDev_Main) { // main
    }
    else if (getSensorDev() == ESensorDev_Sub) { // sub
    }
    else { // main2
    }

    MY_LOG("rOBC.offst0 = 0x%8x", rOBC.offst0);
    MY_LOG("rOBC.offst1 = 0x%8x", rOBC.offst1);
    MY_LOG("rOBC.offst2 = 0x%8x", rOBC.offst2);
    MY_LOG("rOBC.offst3 = 0x%8x", rOBC.offst3);
    MY_LOG("rOBC.gain0 = 0x%8x", rOBC.gain0);
    MY_LOG("rOBC.gain1 = 0x%8x", rOBC.gain1);
    MY_LOG("rOBC.gain2 = 0x%8x", rOBC.gain2);
    MY_LOG("rOBC.gain3 = 0x%8x", rOBC.gain3);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_OBC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_OBC_T& rOBC)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    if (getSensorDev() == ESensorDev_Main) { // main
    }
    else if (getSensorDev() == ESensorDev_Sub) { // sub
    }
    else { // main2
    }

    MY_LOG("rOBC.offst0 = 0x%8x", rOBC.offst0);
    MY_LOG("rOBC.offst1 = 0x%8x", rOBC.offst1);
    MY_LOG("rOBC.offst2 = 0x%8x", rOBC.offst2);
    MY_LOG("rOBC.offst3 = 0x%8x", rOBC.offst3);
    MY_LOG("rOBC.gain0 = 0x%8x", rOBC.gain0);
    MY_LOG("rOBC.gain1 = 0x%8x", rOBC.gain1);
    MY_LOG("rOBC.gain2 = 0x%8x", rOBC.gain2);
    MY_LOG("rOBC.gain3 = 0x%8x", rOBC.gain3);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_BPC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_BPC_T& rBPC)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rBPC.con = 0x%8x", rBPC.con);
    MY_LOG("rBPC.th1 = 0x%8x", rBPC.th1);
    MY_LOG("rBPC.th2 = 0x%8x", rBPC.th2);
    MY_LOG("rBPC.th3 = 0x%8x", rBPC.th3);
    MY_LOG("rBPC.th4 = 0x%8x", rBPC.th4);
    MY_LOG("rBPC.dtc = 0x%8x", rBPC.dtc);
    MY_LOG("rBPC.cor = 0x%8x", rBPC.cor);
    MY_LOG("rBPC.tbli1 = 0x%8x", rBPC.tbli1);
    MY_LOG("rBPC.tbli2 = 0x%8x", rBPC.tbli2);
    MY_LOG("rBPC.th1_c = 0x%8x", rBPC.th1_c);
    MY_LOG("rBPC.th2_c = 0x%8x", rBPC.th2_c);
    MY_LOG("rBPC.th3_c = 0x%8x", rBPC.th3_c);
    #endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_NR1(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_NR1_T& rNR1)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_PDC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_PDC_T& rPDC)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_SL2(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_SL2_T& rSL2)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rSL2.cen = 0x%8x", rSL2.cen);
    MY_LOG("rSL2.max0_rr = 0x%8x", rSL2.max0_rr);
    MY_LOG("rSL2.max1_rr = 0x%8x", rSL2.max1_rr);
    MY_LOG("rSL2.max2_rr = 0x%8x", rSL2.max2_rr);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_NSL2A(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_NSL2A_T& rSL2)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rSL2.cen = 0x%8x", rSL2.cen);
    MY_LOG("rSL2.max0_rr = 0x%8x", rSL2.max0_rr);
    MY_LOG("rSL2.max1_rr = 0x%8x", rSL2.max1_rr);
    MY_LOG("rSL2.max2_rr = 0x%8x", rSL2.max2_rr);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_RPG(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_RPG_T& rRPG)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rRPG.satu_1 = 0x%8x", rRPG.satu_1);
    MY_LOG("rRPG.satu_2 = 0x%8x", rRPG.satu_2);
    MY_LOG("rRPG.gain_1 = 0x%8x", rRPG.gain_1);
    MY_LOG("rRPG.gain_2 = 0x%8x", rRPG.gain_2);
    MY_LOG("rRPG.ofst_1 = 0x%8x", rRPG.ofst_1);
    MY_LOG("rRPG.ofst_2 = 0x%8x", rRPG.ofst_2);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_PGN(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_PGN_T& rPGN)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rPGN.satu_1 = 0x%8x", rPGN.satu_1);
    MY_LOG("rPGN.satu_2 = 0x%8x", rPGN.satu_2);
    MY_LOG("rPGN.gain_1 = 0x%8x", rPGN.gain_1);
    MY_LOG("rPGN.gain_2 = 0x%8x", rPGN.gain_2);
    MY_LOG("rPGN.ofst_1 = 0x%8x", rPGN.ofst_1);
    MY_LOG("rPGN.ofst_2 = 0x%8x", rPGN.ofst_2);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CFA(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CFA_T& rCFA)
{
    if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN0 = 8;
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN1 = 8;
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN2 = 8;
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN3 = 8;
        }
    }

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCFA.byp = 0x%8x", rCFA.byp);
    MY_LOG("rCFA.ed_flat = 0x%8x", rCFA.ed_flat);
    MY_LOG("rCFA.ed_nyq = 0x%8x", rCFA.ed_nyq);
    MY_LOG("rCFA.ed_step = 0x%8x", rCFA.ed_step);
    MY_LOG("rCFA.rgb_hf = 0x%8x", rCFA.rgb_hf);
    MY_LOG("rCFA.dot = 0x%8x", rCFA.dot);
    MY_LOG("rCFA.f1_act = 0x%8x", rCFA.f1_act);
    MY_LOG("rCFA.f2_act = 0x%8x", rCFA.f2_act);
    MY_LOG("rCFA.f3_act = 0x%8x", rCFA.f3_act);
    MY_LOG("rCFA.f4_act = 0x%8x", rCFA.f4_act);
    MY_LOG("rCFA.f1_l = 0x%8x", rCFA.f1_l);
    MY_LOG("rCFA.f2_l = 0x%8x", rCFA.f2_l);
    MY_LOG("rCFA.f3_l = 0x%8x", rCFA.f3_l);
    MY_LOG("rCFA.f4_l = 0x%8x", rCFA.f4_l);
    MY_LOG("rCFA.hf_rb = 0x%8x", rCFA.hf_rb);
    MY_LOG("rCFA.hf_gain = 0x%8x", rCFA.hf_gain);
    MY_LOG("rCFA.hf_comp = 0x%8x", rCFA.hf_comp);
    MY_LOG("rCFA.hf_coring_th = 0x%8x", rCFA.hf_coring_th);
    MY_LOG("rCFA.act_lut = 0x%8x", rCFA.act_lut);
    MY_LOG("rCFA.spare = 0x%8x", rCFA.spare);
    MY_LOG("rCFA.bb = 0x%8x", rCFA.bb);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MINT32 Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCM_T& rCCM)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCM.cnv_1 = 0x%8x", rCCM.cnv_1);
    MY_LOG("rCCM.cnv_2 = 0x%8x", rCCM.cnv_2);
    MY_LOG("rCCM.cnv_3 = 0x%8x", rCCM.cnv_3);
    MY_LOG("rCCM.cnv_4 = 0x%8x", rCCM.cnv_4);
    MY_LOG("rCCM.cnv_5 = 0x%8x", rCCM.cnv_5);
    MY_LOG("rCCM.cnv_6 = 0x%8x", rCCM.cnv_6);
    #endif

    if(rCamInfo.eIspProfile == EIspProfile_IHDR_Preview || rCamInfo.eIspProfile == EIspProfile_IHDR_Video)
    {
        char debugFlag[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.vhdr.tuning", debugFlag, "0");

        MINT32 digit_00 = Complement2((rCCM.cnv_1.val & 0x1FFF),13);
        MINT32 digit_01 = Complement2(((rCCM.cnv_1.val & 0x1FFF0000) >> 16),13);
        MINT32 digit_02 = Complement2((rCCM.cnv_2.val & 0x1FFF),13);
        MINT32 digit_10 = Complement2((rCCM.cnv_3.val & 0x1FFF),13);
        MINT32 digit_11 = Complement2(((rCCM.cnv_3.val & 0x1FFF0000) >> 16),13);
        MINT32 digit_12 = Complement2((rCCM.cnv_4.val & 0x1FFF),13);
        MINT32 digit_20 = Complement2((rCCM.cnv_5.val & 0x1FFF),13);
        MINT32 digit_21 = Complement2(((rCCM.cnv_5.val & 0x1FFF0000) >> 16),13);
        MINT32 digit_22 = Complement2((rCCM.cnv_6.val & 0x1FFF),13);


#if 0
        if(debugFlag[0] == '1')
        {
            MY_LOG("ori:00(%d),01(%d),02(%d)",digit_00,digit_01,digit_02);
            MY_LOG("ori:10(%d),11(%d),12(%d)",digit_10,digit_11,digit_12);
            MY_LOG("ori:20(%d),21(%d),22(%d)",digit_20,digit_21,digit_22);
        }

#endif

        MFLOAT temp_01 = 0.7 * (MFLOAT)digit_01;
        MFLOAT temp_02 = 0.7 * (MFLOAT)digit_02;
        MFLOAT temp_10 = 0.7 * (MFLOAT)digit_10;
        MFLOAT temp_12 = 0.7 * (MFLOAT)digit_12;
        MFLOAT temp_20 = 0.7 * (MFLOAT)digit_20;
        MFLOAT temp_21 = 0.7 * (MFLOAT)digit_21;

        MINT32 final_00 = ((MFLOAT)digit_00-512.0) * 0.7 + 0.5 + 512.0;
        MINT32 final_01 = (temp_01 > 0) ? temp_01 + 0.5 : temp_01 - 0.5;
        MINT32 final_02 = (temp_02 > 0) ? temp_02 + 0.5 : temp_02 - 0.5;
        MINT32 final_10 = (temp_10 > 0) ? temp_10 + 0.5 : temp_10 - 0.5;
        MINT32 final_11 = ((MFLOAT)digit_11-512.0) * 0.7 + 0.5 + 512.0;
        MINT32 final_12 = (temp_12 > 0) ? temp_12 + 0.5 : temp_12 - 0.5;
        MINT32 final_20 = (temp_20 > 0) ? temp_20 + 0.5 : temp_20 - 0.5;
        MINT32 final_21 = (temp_21 > 0) ? temp_21 + 0.5 : temp_21 - 0.5;
        MINT32 final_22 = ((MFLOAT)digit_22-512.0) * 0.7 + 0.5 + 512.0;

        //> check sum = 512 at each raw

        const MINT32 sumValue = 512;

        final_00 += sumValue - (final_00 + final_01 + final_02);
        final_11 += sumValue - (final_10 + final_11 + final_12);
        final_22 += sumValue - (final_20 + final_21 + final_22);

#if 0

        if(debugFlag[0] == '1')
        {
            MY_LOG("final:00(%d),01(%d),02(%d)",final_00,final_01,final_02);
            MY_LOG("final:10(%d),11(%d),12(%d)",final_10,final_11,final_12);
            MY_LOG("final:20(%d),21(%d),22(%d)",final_20,final_21,final_22);
        }
#endif
        rCCM.cnv_1.val = 0 | (final_00 & 0x1FFF) | ((final_01 & 0x1FFF) << 16);
        rCCM.cnv_2.val = 0 | (final_02 & 0x1FFF);
        rCCM.cnv_3.val = 0 | (final_10 & 0x1FFF) | ((final_11 & 0x1FFF) << 16);
        rCCM.cnv_4.val = 0 | (final_12 & 0x1FFF);
        rCCM.cnv_5.val = 0 | (final_20 & 0x1FFF) | ((final_21 & 0x1FFF) << 16);
        rCCM.cnv_6.val = 0 | (final_22 & 0x1FFF);


#if 0

        if(debugFlag[0] == '1')
        {
            MY_LOG("final:rCCM.cnv_1 = 0x%08x", rCCM.cnv_1.val);
            MY_LOG("final:rCCM.cnv_2 = 0x%08x", rCCM.cnv_2.val);
            MY_LOG("final:rCCM.cnv_3 = 0x%08x", rCCM.cnv_3.val);
            MY_LOG("final:rCCM.cnv_4 = 0x%08x", rCCM.cnv_4.val);
            MY_LOG("final:rCCM.cnv_5 = 0x%08x", rCCM.cnv_5.val);
            MY_LOG("final:rCCM.cnv_6 = 0x%08x", rCCM.cnv_6.val);
        }
#endif
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_GGM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_GGM_T& rGGM)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rGGM.lut_rb.lut[0] = 0x%8x", rGGM.lut_rb.lut[0]);
    MY_LOG("rGGM.lut_g.lut[0] = 0x%8x", rGGM.lut_g.lut[0]);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_ANR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_ANR_T& rANR)
{
/*
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.refine_ANR", InputValue, "0");
    MUINT32 debugEn = atoi(InputValue);

    if (debugEn) MY_LOG("rANR.con1.val(0x%08x)", rANR.con1.val);

    if (rCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
        rANR.con1.bits.ANR_ENC = 0;
        rANR.con1.bits.ANR_ENY = 1;
        rANR.con1.bits.ANR_SCALE_MODE = 1;
        rANR.con1.bits.ANR_FLT_MODE = 0;
        rANR.con1.bits.ANR_MODE = 0;
        rANR.con1.bits.ANR_Y_LUMA_SCALE = 4;
        rANR.con1.bits.ANR_LCE_LINK = 0;

        rANR.con3.bits.ANR_IMPL_MODE = 0;
        rANR.con3.bits.ANR_C_MED_EN = 0;
        rANR.con3.bits.ANR_C_SM_EDGE = 1;
        rANR.con3.bits.ANR_QEC = 0;
        rANR.con3.bits.ANR_QEC_VAL = 2;

        rANR.yad1.bits.ANR_CEN_GAIN_LO_TH = 1;
        rANR.yad1.bits.ANR_CEN_GAIN_HI_TH = 5;
        rANR.yad1.bits.ANR_K_LO_TH = 0;
        rANR.yad1.bits.ANR_K_HI_TH = 9;

        rANR.yad2.bits.ANR_PTY_VGAIN = 10;
        rANR.yad2.bits.ANR_PTY_GAIN_TH = 10;

        rANR.lut1.bits.ANR_Y_CPX1 = 40;
        rANR.lut1.bits.ANR_Y_CPX2 = 100;
        rANR.lut1.bits.ANR_Y_CPX3 = 160;

        rANR.lut2.bits.ANR_Y_SCALE_CPY0 = 16;
        rANR.lut2.bits.ANR_Y_SCALE_CPY1 = 16;
        rANR.lut2.bits.ANR_Y_SCALE_CPY2 = 16;
        rANR.lut2.bits.ANR_Y_SCALE_CPY3 = 8;

        rANR.lut3.bits.ANR_Y_SCALE_SP0 = 0;
        rANR.lut3.bits.ANR_Y_SCALE_SP1 = 0;
        rANR.lut3.bits.ANR_Y_SCALE_SP2 = 0x1C;
        rANR.lut3.bits.ANR_Y_SCALE_SP3 = 0x1E;

        rANR.pty.bits.ANR_PTY1 = 12;
        rANR.pty.bits.ANR_PTY2 = 34;
        rANR.pty.bits.ANR_PTY3 = 58;
        rANR.pty.bits.ANR_PTY4 = 72;

        rANR.cad.bits.ANR_PTC_VGAIN = 10;
        rANR.cad.bits.ANR_PTC_GAIN_TH = 6;
        rANR.cad.bits.ANR_C_L_DIFF_TH = 28;

        rANR.ptc.bits.ANR_PTC1 = 2;
        rANR.ptc.bits.ANR_PTC2 = 3;
        rANR.ptc.bits.ANR_PTC3 = 4;
        rANR.ptc.bits.ANR_PTC4 = 6;

        rANR.lce1.bits.ANR_LCE_C_GAIN = 6;
        rANR.lce1.bits.ANR_LCE_SCALE_GAIN = 0;

        rANR.lce2.bits.ANR_LCE_GAIN0 = 8;
        rANR.lce2.bits.ANR_LCE_GAIN1 = 12;
        rANR.lce2.bits.ANR_LCE_GAIN2 = 16;
        rANR.lce2.bits.ANR_LCE_GAIN3 = 20;

        rANR.hp1.bits.ANR_HP_A = 120;
        rANR.hp1.bits.ANR_HP_B = 0x3C;
        rANR.hp1.bits.ANR_HP_C = 0x19;
        rANR.hp1.bits.ANR_HP_D = 0x9;
        rANR.hp1.bits.ANR_HP_E = 0xB;

        rANR.hp2.bits.ANR_HP_S1 = 0;
        rANR.hp2.bits.ANR_HP_S2 = 0;
        rANR.hp2.bits.ANR_HP_X1 = 0;
        rANR.hp2.bits.ANR_HP_F = 0x2;

        rANR.hp3.bits.ANR_HP_Y_GAIN_CLIP = 119;
        rANR.hp3.bits.ANR_HP_Y_SP = 6;
        rANR.hp3.bits.ANR_HP_Y_LO = 120;
        rANR.hp3.bits.ANR_HP_CLIP = 0;

        rANR.acty.bits.ANR_ACT_TH_Y = 16;
        rANR.acty.bits.ANR_ACT_BLD_BASE_Y = 48;
        rANR.acty.bits.ANR_ACT_SLANT_Y = 14;
        rANR.acty.bits.ANR_ACT_BLD_TH_Y = 48;

        rANR.actc.bits.ANR_ACT_TH_C = 0;
        rANR.actc.bits.ANR_ACT_BLD_BASE_C = 0;
        rANR.actc.bits.ANR_ACT_SLANT_C = 0;
        rANR.actc.bits.ANR_ACT_BLD_TH_C = 0;

    }

    if(TUNING_FOR_AIS) {
        // enable sl2b for ais stage3/4
        if(IS_AIS) {
            if (   rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR  )
            {
                    rANR.con1.bits.ANR_LCE_LINK = 1;
            }
        }

        //stage1 use high iso, stage2/3/4 use low iso
        // use low iso luma anr for ais stage3/4
        // use high iso chroma anr for ais stage3/4 <- this
        if(IS_AIS) {
            if (   rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR   )
            {
                MUINT32 stage = AIS_Profile2Stage(rCamInfo.eIspProfile);

                MUINT32 aisIso = rCamInfo.u4ISOValue;
                MUINT32 normalIso = rCamInfo.rAEInfo.u4OrgRealISOValue;
                MUINT32 ynrIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_YNR_RATIO[stage-1]);
                MUINT32 cnrIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_CNR_RATIO[stage-1]);

                MINT32 aisIndex = map_ISO_value_to_index(aisIso);
                MINT32 normalIndex = map_ISO_value_to_index(normalIso);
                MINT32 ynrIndex = map_ISO_value_to_index(ynrIso);

                //base on YNR
                MINT32 deltaCnr = map_ISO_value_to_index(cnrIso) - ynrIndex;

                ISP_NVRAM_ANR_T rAnrSettingForCnr = rIspRegMgr.getANR(rIspRegMgr.getIdx_ANR() + deltaCnr);

                if (debugEn) MY_LOG("rANR.con1.val(0x%08x), rAnrSettingForCnr.con1.val(0x%08x)", rANR.con1.val, rAnrSettingForCnr.con1.val);
                //con1
                rANR.con1.bits.ANR_ENC = rAnrSettingForCnr.con1.bits.ANR_ENC;
                rANR.con1.bits.ANR_SCALE_MODE = rAnrSettingForCnr.con1.bits.ANR_SCALE_MODE;
                //con3
                rANR.con3.bits.ANR_C_SM_EDGE = rAnrSettingForCnr.con3.bits.ANR_C_SM_EDGE;
                //cad(all)
                //rANR.cad.bits.ANR_PTC_VGAIN = rAnrSettingForCnr.cad.bits.ANR_PTC_VGAIN;
                //rANR.cad.bits.ANR_PTC_GAIN_TH = rAnrSettingForCnr.cad.bits.ANR_PTC_GAIN_TH;
                //rANR.cad.bits.ANR_C_L_DIFF_TH = rAnrSettingForCnr.cad.bits.ANR_C_L_DIFF_TH;
                rANR.cad.val = rAnrSettingForCnr.cad.val;
                //ptc(all)
                //rANR.ptc.bits.ANR_PTC1 = rAnrSettingForCnr.ptc.bits.ANR_PTC1;
                //rANR.ptc.bits.ANR_PTC2 = rAnrSettingForCnr.ptc.bits.ANR_PTC2;
                //rANR.ptc.bits.ANR_PTC3 = rAnrSettingForCnr.ptc.bits.ANR_PTC3;
                //rANR.ptc.bits.ANR_PTC4 = rAnrSettingForCnr.ptc.bits.ANR_PTC4;
                rANR.ptc.val = rAnrSettingForCnr.ptc.val;
                //lce1
                rANR.lce1.bits.ANR_LCE_C_GAIN = rAnrSettingForCnr.lce1.bits.ANR_LCE_C_GAIN;
                //lce2(all)
                //rANR.lce2.bits.ANR_LCE_GAIN0 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN0;
                //rANR.lce2.bits.ANR_LCE_GAIN1 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN1;
                //rANR.lce2.bits.ANR_LCE_GAIN2 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN2;
                //rANR.lce2.bits.ANR_LCE_GAIN3 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN3;
                rANR.lce2.val = rAnrSettingForCnr.lce2.val;
                //actc(all)
                //rANR.actc.bits.ANR_ACT_TH_C = rAnrSettingForCnr.actc.bits.ANR_ACT_TH_C;
                //rANR.actc.bits.ANR_ACT_BLD_BASE_C = rAnrSettingForCnr.actc.bits.ANR_ACT_BLD_BASE_C;
                //rANR.actc.bits.ANR_ACT_SLANT_C = rAnrSettingForCnr.actc.bits.ANR_ACT_SLANT_C;
                //rANR.actc.bits.ANR_ACT_BLD_TH_C = rAnrSettingForCnr.actc.bits.ANR_ACT_BLD_TH_C;
                rANR.actc.val = rAnrSettingForCnr.actc.val;
            }
        }
    }

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rANR.con1 = 0x%8x", rANR.con1);
    MY_LOG("rANR.con2 = 0x%8x", rANR.con2);
    MY_LOG("rANR.con3 = 0x%8x", rANR.con3);
    MY_LOG("rANR.yad1 = 0x%8x", rANR.yad1);
    MY_LOG("rANR.yad2 = 0x%8x", rANR.yad2);
    MY_LOG("rANR.lut1 = 0x%8x", rANR.lut1);
    MY_LOG("rANR.lut2 = 0x%8x", rANR.lut2);
    MY_LOG("rANR.lut3 = 0x%8x", rANR.lut3);
    MY_LOG("rANR.pty = 0x%8x", rANR.pty);
    MY_LOG("rANR.cad = 0x%8x", rANR.cad);
    MY_LOG("rANR.ptc = 0x%8x", rANR.ptc);
    MY_LOG("rANR.lce1 = 0x%8x", rANR.lce1);
    MY_LOG("rANR.lce2 = 0x%8x", rANR.lce2);
    MY_LOG("rANR.hp1 = 0x%8x", rANR.hp1);
    MY_LOG("rANR.hp2 = 0x%8x", rANR.hp2);
    MY_LOG("rANR.hp3 = 0x%8x", rANR.hp3);
    MY_LOG("rANR.acty = 0x%8x", rANR.acty);
    MY_LOG("rANR.actc = 0x%8x", rANR.actc);
    #endif
    */
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCR_T& rCCR)
{
    if (rCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
       rCCR.con.bits.CCR_EN = 0;
    }


    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCR.con = 0x%8x", rCCR.con);
    MY_LOG("rCCR.ylut = 0x%8x", rCCR.ylut);
    MY_LOG("rCCR.uvlut = 0x%8x", rCCR.uvlut);
    MY_LOG("rCCR.ylut2 = 0x%8x", rCCR.ylut2);
    MY_LOG("rCCR.sat_ctrl = 0x%8x", rCCR.sat_ctrl);
    MY_LOG("rCCR.uvlut_sp = 0x%8x", rCCR.uvlut_sp);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_EE(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_EE_T& rEE)
{
     if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            rEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN = 0;
        }
    }

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rEE.srk_ctrl = 0x%8x", rEE.srk_ctrl);
    MY_LOG("rEE.clip_ctrl = 0x%8x", rEE.clip_ctrl);
    MY_LOG("rEE.flt_ctrl_1 = 0x%8x", rEE.flt_ctrl_1);
    MY_LOG("rEE.flt_ctrl_2 = 0x%8x", rEE.flt_ctrl_2);
    MY_LOG("rEE.glut_ctrl_01 = 0x%8x", rEE.glut_ctrl_01);
    MY_LOG("rEE.glut_ctrl_02 = 0x%8x", rEE.glut_ctrl_02);
    MY_LOG("rEE.glut_ctrl_03 = 0x%8x", rEE.glut_ctrl_03);
    MY_LOG("rEE.glut_ctrl_04 = 0x%8x", rEE.glut_ctrl_04);
    MY_LOG("rEE.glut_ctrl_05 = 0x%8x", rEE.glut_ctrl_05);
    MY_LOG("rEE.glut_ctrl_06 = 0x%8x", rEE.glut_ctrl_06);
    MY_LOG("rEE.edtr_ctrl = 0x%8x", rEE.edtr_ctrl);
    MY_LOG("rEE.glut_ctrl_07 = 0x%8x", rEE.glut_ctrl_07);
    MY_LOG("rEE.glut_ctrl_08 = 0x%8x", rEE.glut_ctrl_08);
    MY_LOG("rEE.glut_ctrl_09 = 0x%8x", rEE.glut_ctrl_09);
    MY_LOG("rEE.glut_ctrl_10 = 0x%8x", rEE.glut_ctrl_10);
    MY_LOG("rEE.glut_ctrl_11 = 0x%8x", rEE.glut_ctrl_11);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Use 3DNR for frame rate improvement
// Ratio|
//  100%|         /-----
//      |        /
//      |       /
//      |      /
//      |     /
//   0% +----|----|------
//      TH_LOW  TH_HIGH
#define LIMITER(Input, LowerBound, UpperBound)  do { if (Input > UpperBound){Input = UpperBound;} if (Input < LowerBound){Input = LowerBound;} } while (0)

// Note: X2 must be larger than or equal to X1.
inline MINT32 Nr3dLmtInterpolation(MINT32 TargetX, MINT32 X1, MINT32 Y1, MINT32 X2, MINT32 Y2)
{
    MINT32 TargetY = 0;
	MINT32 TempValue = 0;
	MINT32 RoundingValue = 0;

    if (X1 == X2)
    {
        TargetY = Y1;
    }
	else if ( TargetX <= X1 )
	{
		TargetY = Y1;
	}
	else if ( TargetX >= X2 )
	{
		TargetY = Y2;
	}
	else    // if (X1 <= TargetX <= X2), then interpolation.
	{
		TempValue = (TargetX - X1) * (Y2 - Y1);
		RoundingValue = (X2 - X1) >> 1;
		TargetY = (TempValue + RoundingValue) / (X2 - X1) + Y1;
	}

    return TargetY;

}

/*
MVOID
IspTuningCustom::
refine_NR3D(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_NR3D_T& rNR3D)
{
    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    unsigned int i4TempInputValue = 0;

    MUINT32 MaxIsoIncreasePercentage    = get_3dnr_max_iso_increase_percentage();   // E.g. 130 means that Raise Max ISO to 130% when 3DNR on.;
    MUINT32 i4TempIso = 0;


    ISP_NVRAM_NR3D_BLEND_T u4OldNr3dblend = rNR3D.blend;   // Record old rNR3D.blend.
    FIELD u4OldNr3dGain  = rNR3D.blend.bits.NR3D_GAIN;   // Record old rNR3D.blend.bits.NR3D_GAIN.
    MUINT32 u4InterpolatedRatio = 0; // 1000 * the ratio that will be multiplied to NR3D_GAIN.
    #if 1   // RealIsoforThrLow/High use fix ISO.
    MUINT32 IsoEnableThresholdLow       = get_3dnr_iso_enable_threshold_low();      // E.g. 60 means that use 60% of Max Current ISO as THRESHOLD_LOW;
    MUINT32 IsoEnableThresholdHigh      = get_3dnr_iso_enable_threshold_high();     // E.g. 80 means that use 80% of Max Current ISO as THRESHOLD_HIGH;
    MUINT32 u4RealIsoforThrLow  = IsoEnableThresholdLow  ;
    MUINT32 u4RealIsoforThrHigh = IsoEnableThresholdHigh;
    #else   // RealIsoforThrLow/High use Max ISO percentage.
    MUINT32 IsoEnableThresholdLow       = get_3dnr_iso_enable_threshold_low_percentage();      // E.g. 60 means that use 60% of Max Current ISO as THRESHOLD_LOW;
    MUINT32 IsoEnableThresholdHigh      = get_3dnr_iso_enable_threshold_high_percentage();     // E.g. 80 means that use 80% of Max Current ISO as THRESHOLD_HIGH;
    MUINT32 u4RealIsoforThrLow  = rCamInfo.rAEInfo.u4MaxISO * IsoEnableThresholdLow  / MaxIsoIncreasePercentage;  // /MAX_ISO_INCREASE_PERCENTAGE: Should use MaxISO before raise max ISO limit.
    MUINT32 u4RealIsoforThrHigh = rCamInfo.rAEInfo.u4MaxISO * IsoEnableThresholdHigh / MaxIsoIncreasePercentage;  // /MAX_ISO_INCREASE_PERCENTAGE: Should use MaxISO before raise max ISO limit.
    #endif  // Diff RealIsoforThrLow/High decision.
    // Force change u4RealIsoforThrLow/u4RealIsoforThrHigh.
    property_get("vendor.camera.3dnr.fixlimit", InputValue, "0");
    if (InputValue[0] == '1')   // Use fixed limit
    {
        u4RealIsoforThrLow  = 400;  // /MAX_ISO_INCREASE_PERCENTAGE: Should use MaxISO before raise max ISO limit.
        u4RealIsoforThrHigh = 600;  // /MAX_ISO_INCREASE_PERCENTAGE: Should use MaxISO before raise max ISO limit.

        property_get("vendor.camera.3dnr.lowiso", InputValue, "0");
        i4TempInputValue = atoi(InputValue);
        if (i4TempInputValue != 0)
        {
            u4RealIsoforThrLow = i4TempInputValue;
        }
        property_get("vendor.camera.3dnr.highiso", InputValue, "0");
        i4TempInputValue = atoi(InputValue);
        if (i4TempInputValue != 0)
        {
            u4RealIsoforThrHigh = i4TempInputValue;
        }
    }

    // Use interpolation to calculate NR3D_GAIN ratio.
    u4InterpolatedRatio = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4RealIsoforThrLow, 0, u4RealIsoforThrHigh, 1000);
    // Apply ratio to NR3D_GAIN.
	rNR3D.blend.bits.NR3D_GAIN = (u4InterpolatedRatio * rNR3D.blend.bits.NR3D_GAIN + 500) / 1000;   // +500: for rounding.

    EIndex_ISO_T eIsoIdxforThrLow  = eIDX_ISO_100; //map_ISO_value_to_lower_index(rCamInfo.u4ISOValue);
    EIndex_ISO_T eIsoIdxforThrHigh = eIDX_ISO_100; //map_ISO_value_to_upper_index(rCamInfo.u4ISOValue);
    i4TempIso = map_ISO_index_to_value(map_ISO_value_to_index(rCamInfo.u4ISOValue));
    if (rCamInfo.u4ISOValue > i4TempIso)
    {
        if (rIspRegMgr.getIdx_NR3D() == eNUM_OF_ISO_IDX - 1)    // Already max index, can't +1.
        {
            eIsoIdxforThrLow  = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D());
            eIsoIdxforThrHigh = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D());
        }
        else
        {
            eIsoIdxforThrLow  = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D());
            eIsoIdxforThrHigh = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D() + 1);
        }
    }
    else    // u4ISOValue < i4TempIso
    {
        if (rIspRegMgr.getIdx_NR3D() == 0)    // Already min index, can't -1.
        {
            eIsoIdxforThrLow  = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D());
            eIsoIdxforThrHigh = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D());
        }
        else
        {
            eIsoIdxforThrLow  = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D() - 1);
            eIsoIdxforThrHigh = (EIndex_ISO_T)(rIspRegMgr.getIdx_NR3D());
        }
    }
    //MY_LOG("i4TempIso(%d), u4ISOValue(%d), getIdx_NR3D(%d)- (%d, %d)", i4TempIso, rCamInfo.u4ISOValue, rIspRegMgr.getIdx_NR3D(), eIsoIdxforThrLow, eIsoIdxforThrHigh);

    ISP_NVRAM_NR3D_T rNr3dSettingforThrLow  = rIspRegMgr.getNR3D(eIsoIdxforThrLow );
    ISP_NVRAM_NR3D_T rNr3dSettingforThrHigh = rIspRegMgr.getNR3D(eIsoIdxforThrHigh);

    MUINT32 u4MappedIsoforThrLow  = map_ISO_index_to_value(eIsoIdxforThrLow );
    MUINT32 u4MappedIsoforThrHigh = map_ISO_index_to_value(eIsoIdxforThrHigh);


    rNR3D.blend.bits.NR3D_RND_Y           = 31 - rNR3D.blend.bits.NR3D_GAIN;
    rNR3D.blend.bits.NR3D_RND_U           = 31 - rNR3D.blend.bits.NR3D_GAIN;
    rNR3D.blend.bits.NR3D_RND_V           = 31 - rNR3D.blend.bits.NR3D_GAIN;
    rNR3D.lmt_cpx.bits.NR3D_LMT_CPX1      = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX1      , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX1      );
    rNR3D.lmt_cpx.bits.NR3D_LMT_CPX2      = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX2      , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX2      );
    rNR3D.lmt_cpx.bits.NR3D_LMT_CPX3      = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX3      , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX3      );
    rNR3D.lmt_cpx.bits.NR3D_LMT_CPX4      = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX4      , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX4      );
    rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_y_con1.bits.NR3D_LMT_Y0     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_y_con1.bits.NR3D_LMT_Y0     );
    rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0_TH  = rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0 * 2;
    rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_y_con1.bits.NR3D_LMT_Y1     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_y_con1.bits.NR3D_LMT_Y1     );
    rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1_TH  = rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1 * 2;
    rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_y_con2.bits.NR3D_LMT_Y2     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_y_con2.bits.NR3D_LMT_Y2     );
    rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2_TH  = rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2 * 2;
    rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_y_con2.bits.NR3D_LMT_Y3     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_y_con2.bits.NR3D_LMT_Y3     );
    rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3_TH  = rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3 * 2;
    rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_y_con3.bits.NR3D_LMT_Y4     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_y_con3.bits.NR3D_LMT_Y4     );
    rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4_TH  = rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4 * 2;
    rNR3D.lmt_u_con1.bits.NR3D_LMT_U0     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_u_con1.bits.NR3D_LMT_U0     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_u_con1.bits.NR3D_LMT_U0     );
    rNR3D.lmt_u_con1.bits.NR3D_LMT_U0_TH  = rNR3D.lmt_u_con1.bits.NR3D_LMT_U0 * 2;
    rNR3D.lmt_u_con1.bits.NR3D_LMT_U1     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_u_con1.bits.NR3D_LMT_U1     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_u_con1.bits.NR3D_LMT_U1     );
    rNR3D.lmt_u_con1.bits.NR3D_LMT_U1_TH  = rNR3D.lmt_u_con1.bits.NR3D_LMT_U1 * 2;
    rNR3D.lmt_u_con2.bits.NR3D_LMT_U2     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_u_con2.bits.NR3D_LMT_U2     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_u_con2.bits.NR3D_LMT_U2     );
    rNR3D.lmt_u_con2.bits.NR3D_LMT_U2_TH  = rNR3D.lmt_u_con2.bits.NR3D_LMT_U2 * 2;
    rNR3D.lmt_u_con2.bits.NR3D_LMT_U3     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_u_con2.bits.NR3D_LMT_U3     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_u_con2.bits.NR3D_LMT_U3     );
    rNR3D.lmt_u_con2.bits.NR3D_LMT_U3_TH  = rNR3D.lmt_u_con2.bits.NR3D_LMT_U3 * 2;
    rNR3D.lmt_u_con3.bits.NR3D_LMT_U4     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_u_con3.bits.NR3D_LMT_U4     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_u_con3.bits.NR3D_LMT_U4     );
    rNR3D.lmt_u_con3.bits.NR3D_LMT_U4_TH  = rNR3D.lmt_u_con3.bits.NR3D_LMT_U4 * 2;
    rNR3D.lmt_v_con1.bits.NR3D_LMT_V0     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_v_con1.bits.NR3D_LMT_V0     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_v_con1.bits.NR3D_LMT_V0     );
    rNR3D.lmt_v_con1.bits.NR3D_LMT_V0_TH  = rNR3D.lmt_v_con1.bits.NR3D_LMT_V0 * 2;
    rNR3D.lmt_v_con1.bits.NR3D_LMT_V1     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_v_con1.bits.NR3D_LMT_V1     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_v_con1.bits.NR3D_LMT_V1     );
    rNR3D.lmt_v_con1.bits.NR3D_LMT_V1_TH  = rNR3D.lmt_v_con1.bits.NR3D_LMT_V1 * 2;
    rNR3D.lmt_v_con2.bits.NR3D_LMT_V2     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_v_con2.bits.NR3D_LMT_V2     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_v_con2.bits.NR3D_LMT_V2     );
    rNR3D.lmt_v_con2.bits.NR3D_LMT_V2_TH  = rNR3D.lmt_v_con2.bits.NR3D_LMT_V2 * 2;
    rNR3D.lmt_v_con2.bits.NR3D_LMT_V3     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_v_con2.bits.NR3D_LMT_V3     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_v_con2.bits.NR3D_LMT_V3     );
    rNR3D.lmt_v_con2.bits.NR3D_LMT_V3_TH  = rNR3D.lmt_v_con2.bits.NR3D_LMT_V3 * 2;
    rNR3D.lmt_v_con3.bits.NR3D_LMT_V4     = Nr3dLmtInterpolation(rCamInfo.u4ISOValue, u4MappedIsoforThrLow, rNr3dSettingforThrLow.lmt_v_con3.bits.NR3D_LMT_V4     , u4MappedIsoforThrHigh, rNr3dSettingforThrHigh.lmt_v_con3.bits.NR3D_LMT_V4     );
    rNR3D.lmt_v_con3.bits.NR3D_LMT_V4_TH  = rNR3D.lmt_v_con3.bits.NR3D_LMT_V4 * 2;


    property_get("vendor.camera.3dnr.tuning", InputValue, "0");
	if (InputValue[0] == '1')
	{
        rNR3D.blend.val      = 0x0000001F;
        rNR3D.lmt_cpx.val    = 0xbf906036;
        rNR3D.lmt_y_con1.val = 0x1e0f1e0f;
        rNR3D.lmt_y_con2.val = 0x10081a0d;
        rNR3D.lmt_y_con3.val = 0x00000a05;
        rNR3D.lmt_u_con1.val = 0x0a050a05;
        rNR3D.lmt_u_con2.val = 0x06030c06;
        rNR3D.lmt_u_con3.val = 0x00000402;
        rNR3D.lmt_v_con1.val = 0x0a050a05;
        rNR3D.lmt_v_con2.val = 0x06030c06;
        rNR3D.lmt_v_con3.val = 0x00000402;

        //MY_LOG("rNR3D.blend      = 0x%8x", rNR3D.blend);
    	//MY_LOG("rNR3D.lmt_cpx    = 0x%8x", rNR3D.lmt_cpx);
    	//MY_LOG("rNR3D.lmt_y_con1 = 0x%8x", rNR3D.lmt_y_con1);
    	//MY_LOG("rNR3D.lmt_y_con2 = 0x%8x", rNR3D.lmt_y_con2);
    	//MY_LOG("rNR3D.lmt_y_con3 = 0x%8x", rNR3D.lmt_y_con3);
    	//MY_LOG("rNR3D.lmt_u_con1 = 0x%8x", rNR3D.lmt_u_con1);
    	//MY_LOG("rNR3D.lmt_u_con2 = 0x%8x", rNR3D.lmt_u_con2);
    	//MY_LOG("rNR3D.lmt_u_con3 = 0x%8x", rNR3D.lmt_u_con3);
    	//MY_LOG("rNR3D.lmt_v_con1 = 0x%8x", rNR3D.lmt_v_con1);
    	//MY_LOG("rNR3D.lmt_v_con2 = 0x%8x", rNR3D.lmt_v_con2);
    	//MY_LOG("rNR3D.lmt_v_con3 = 0x%8x", rNR3D.lmt_v_con3);

	}
	else if (InputValue[0] == '2')
	{
	    property_get("vendor.camera.3dnr.nr3dgain", InputValue, "0");
		MUINT32 u4Nr3dGain = atoi(InputValue);

	    rNR3D.blend.bits.NR3D_GAIN = u4Nr3dGain;
        rNR3D.blend.bits.NR3D_RND_Y           = 31 - rNR3D.blend.bits.NR3D_GAIN;
        rNR3D.blend.bits.NR3D_RND_U           = 31 - rNR3D.blend.bits.NR3D_GAIN;
        rNR3D.blend.bits.NR3D_RND_V           = 31 - rNR3D.blend.bits.NR3D_GAIN;

        //MY_LOG("rNR3D.blend: 0x%8x. NR3D_GAIN: %d", rNR3D.blend, rNR3D.blend.bits.NR3D_GAIN);
	}
	else if (InputValue[0] == '3')
	{
	    property_get("vendor.camera.3dnr.lmtyratio", InputValue, "0");
		MUINT32 u4LmtYRatio = atoi(InputValue);
	    property_get("vendor.camera.3dnr.lmtcratio", InputValue, "0");
		MUINT32 u4LmtCRatio = atoi(InputValue);
	    property_get("vendor.camera.3dnr.nr3dgain", InputValue, "256");
		MUINT32 u4Nr3dGain = atoi(InputValue);

        if (u4Nr3dGain != 256)
        {
    	    rNR3D.blend.bits.NR3D_GAIN  = u4Nr3dGain;
            rNR3D.blend.bits.NR3D_RND_Y = 31 - rNR3D.blend.bits.NR3D_GAIN;
            rNR3D.blend.bits.NR3D_RND_U = 31 - rNR3D.blend.bits.NR3D_GAIN;
            rNR3D.blend.bits.NR3D_RND_V = 31 - rNR3D.blend.bits.NR3D_GAIN;
        }

        rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0     = rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0 * u4LmtYRatio / 100;
        rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1     = rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1 * u4LmtYRatio / 100;
        rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2     = rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2 * u4LmtYRatio / 100;
        rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3     = rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3 * u4LmtYRatio / 100;
        rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4     = rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4 * u4LmtYRatio / 100;
        rNR3D.lmt_u_con1.bits.NR3D_LMT_U0     = rNR3D.lmt_u_con1.bits.NR3D_LMT_U0 * u4LmtCRatio / 100;
        rNR3D.lmt_u_con1.bits.NR3D_LMT_U1     = rNR3D.lmt_u_con1.bits.NR3D_LMT_U1 * u4LmtCRatio / 100;
        rNR3D.lmt_u_con2.bits.NR3D_LMT_U2     = rNR3D.lmt_u_con2.bits.NR3D_LMT_U2 * u4LmtCRatio / 100;
        rNR3D.lmt_u_con2.bits.NR3D_LMT_U3     = rNR3D.lmt_u_con2.bits.NR3D_LMT_U3 * u4LmtCRatio / 100;
        rNR3D.lmt_u_con3.bits.NR3D_LMT_U4     = rNR3D.lmt_u_con3.bits.NR3D_LMT_U4 * u4LmtCRatio / 100;
        rNR3D.lmt_v_con1.bits.NR3D_LMT_V0     = rNR3D.lmt_v_con1.bits.NR3D_LMT_V0 * u4LmtCRatio / 100;
        rNR3D.lmt_v_con1.bits.NR3D_LMT_V1     = rNR3D.lmt_v_con1.bits.NR3D_LMT_V1 * u4LmtCRatio / 100;
        rNR3D.lmt_v_con2.bits.NR3D_LMT_V2     = rNR3D.lmt_v_con2.bits.NR3D_LMT_V2 * u4LmtCRatio / 100;
        rNR3D.lmt_v_con2.bits.NR3D_LMT_V3     = rNR3D.lmt_v_con2.bits.NR3D_LMT_V3 * u4LmtCRatio / 100;
        rNR3D.lmt_v_con3.bits.NR3D_LMT_V4     = rNR3D.lmt_v_con3.bits.NR3D_LMT_V4 * u4LmtCRatio / 100;

        LIMITER(rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0, 0, 15);
        LIMITER(rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1, 0, 15);
        LIMITER(rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2, 0, 15);
        LIMITER(rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3, 0, 15);
        LIMITER(rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4, 0, 15);
        LIMITER(rNR3D.lmt_u_con1.bits.NR3D_LMT_U0, 0, 15);
        LIMITER(rNR3D.lmt_u_con1.bits.NR3D_LMT_U1, 0, 15);
        LIMITER(rNR3D.lmt_u_con2.bits.NR3D_LMT_U2, 0, 15);
        LIMITER(rNR3D.lmt_u_con2.bits.NR3D_LMT_U3, 0, 15);
        LIMITER(rNR3D.lmt_u_con3.bits.NR3D_LMT_U4, 0, 15);
        LIMITER(rNR3D.lmt_v_con1.bits.NR3D_LMT_V0, 0, 15);
        LIMITER(rNR3D.lmt_v_con1.bits.NR3D_LMT_V1, 0, 15);
        LIMITER(rNR3D.lmt_v_con2.bits.NR3D_LMT_V2, 0, 15);
        LIMITER(rNR3D.lmt_v_con2.bits.NR3D_LMT_V3, 0, 15);
        LIMITER(rNR3D.lmt_v_con3.bits.NR3D_LMT_V4, 0, 15);

        rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0_TH  = rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0 * 2;
        rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1_TH  = rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1 * 2;
        rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2_TH  = rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2 * 2;
        rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3_TH  = rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3 * 2;
        rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4_TH  = rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4 * 2;
        rNR3D.lmt_u_con1.bits.NR3D_LMT_U0_TH  = rNR3D.lmt_u_con1.bits.NR3D_LMT_U0 * 2;
        rNR3D.lmt_u_con1.bits.NR3D_LMT_U1_TH  = rNR3D.lmt_u_con1.bits.NR3D_LMT_U1 * 2;
        rNR3D.lmt_u_con2.bits.NR3D_LMT_U2_TH  = rNR3D.lmt_u_con2.bits.NR3D_LMT_U2 * 2;
        rNR3D.lmt_u_con2.bits.NR3D_LMT_U3_TH  = rNR3D.lmt_u_con2.bits.NR3D_LMT_U3 * 2;
        rNR3D.lmt_u_con3.bits.NR3D_LMT_U4_TH  = rNR3D.lmt_u_con3.bits.NR3D_LMT_U4 * 2;
        rNR3D.lmt_v_con1.bits.NR3D_LMT_V0_TH  = rNR3D.lmt_v_con1.bits.NR3D_LMT_V0 * 2;
        rNR3D.lmt_v_con1.bits.NR3D_LMT_V1_TH  = rNR3D.lmt_v_con1.bits.NR3D_LMT_V1 * 2;
        rNR3D.lmt_v_con2.bits.NR3D_LMT_V2_TH  = rNR3D.lmt_v_con2.bits.NR3D_LMT_V2 * 2;
        rNR3D.lmt_v_con2.bits.NR3D_LMT_V3_TH  = rNR3D.lmt_v_con2.bits.NR3D_LMT_V3 * 2;
        rNR3D.lmt_v_con3.bits.NR3D_LMT_V4_TH  = rNR3D.lmt_v_con3.bits.NR3D_LMT_V4 * 2;


#if 0

        MY_LOG("rNR3D.blend.bits.NR3D_GAIN           = %d", rNR3D.blend.bits.NR3D_GAIN           );
        MY_LOG("rNR3D.blend.bits.NR3D_RND_Y          = %d", rNR3D.blend.bits.NR3D_RND_Y          );
        MY_LOG("rNR3D.blend.bits.NR3D_RND_U          = %d", rNR3D.blend.bits.NR3D_RND_U          );
        MY_LOG("rNR3D.blend.bits.NR3D_RND_V          = %d", rNR3D.blend.bits.NR3D_RND_V          );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0    = %d", rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0    );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0_TH = %d", rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0_TH );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1    = %d", rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1    );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1_TH = %d", rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1_TH );
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2    = %d", rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2    );
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2_TH = %d", rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2_TH );
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3    = %d", rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3    );
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3_TH = %d", rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3_TH );
        MY_LOG("rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4    = %d", rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4    );
        MY_LOG("rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4_TH = %d", rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4_TH );
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U0    = %d", rNR3D.lmt_u_con1.bits.NR3D_LMT_U0    );
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U0_TH = %d", rNR3D.lmt_u_con1.bits.NR3D_LMT_U0_TH );
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U1    = %d", rNR3D.lmt_u_con1.bits.NR3D_LMT_U1    );
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U1_TH = %d", rNR3D.lmt_u_con1.bits.NR3D_LMT_U1_TH );
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U2    = %d", rNR3D.lmt_u_con2.bits.NR3D_LMT_U2    );
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U2_TH = %d", rNR3D.lmt_u_con2.bits.NR3D_LMT_U2_TH );
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U3    = %d", rNR3D.lmt_u_con2.bits.NR3D_LMT_U3    );
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U3_TH = %d", rNR3D.lmt_u_con2.bits.NR3D_LMT_U3_TH );
        MY_LOG("rNR3D.lmt_u_con3.bits.NR3D_LMT_U4    = %d", rNR3D.lmt_u_con3.bits.NR3D_LMT_U4    );
        MY_LOG("rNR3D.lmt_u_con3.bits.NR3D_LMT_U4_TH = %d", rNR3D.lmt_u_con3.bits.NR3D_LMT_U4_TH );
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V0    = %d", rNR3D.lmt_v_con1.bits.NR3D_LMT_V0    );
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V0_TH = %d", rNR3D.lmt_v_con1.bits.NR3D_LMT_V0_TH );
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V1    = %d", rNR3D.lmt_v_con1.bits.NR3D_LMT_V1    );
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V1_TH = %d", rNR3D.lmt_v_con1.bits.NR3D_LMT_V1_TH );
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V2    = %d", rNR3D.lmt_v_con2.bits.NR3D_LMT_V2    );
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V2_TH = %d", rNR3D.lmt_v_con2.bits.NR3D_LMT_V2_TH );
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V3    = %d", rNR3D.lmt_v_con2.bits.NR3D_LMT_V3    );
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V3_TH = %d", rNR3D.lmt_v_con2.bits.NR3D_LMT_V3_TH );
        MY_LOG("rNR3D.lmt_v_con3.bits.NR3D_LMT_V4    = %d", rNR3D.lmt_v_con3.bits.NR3D_LMT_V4    );
        MY_LOG("rNR3D.lmt_v_con3.bits.NR3D_LMT_V4_TH = %d", rNR3D.lmt_v_con3.bits.NR3D_LMT_V4_TH );
#endif
	}

    // Print log for debug.
    property_get("vendor.camera.3dnr.tuning.debuglog", InputValue, "0");
	if (InputValue[0] == '1')
	{

#if 0
    	MY_LOG("<NR3D Tuning Parameters>");
    	MY_LOG("rCamInfo.rAEInfo.u4MaxISO         = %d", rCamInfo.rAEInfo.u4MaxISO);
    	MY_LOG("rCamInfo.eIdx_ISO                 = %d", rCamInfo.eIdx_ISO);
//    	MY_LOG("rCamInfo.IsoEnableThresholdLow    = %d", IsoEnableThresholdLow);
//    	MY_LOG("rCamInfo.IsoEnableThresholdHigh   = %d", IsoEnableThresholdHigh);
    	MY_LOG("rCamInfo.MaxIsoIncreasePercentage = %d", MaxIsoIncreasePercentage);
    	MY_LOG("rCamInfo.u4ISOValue               = %d (L/R ISO Idx: %d - %d)", rCamInfo.u4ISOValue, eIsoIdxforThrLow, eIsoIdxforThrHigh);
    	MY_LOG("rCamInfo.u4RealIsoforThrLow       = %d", u4RealIsoforThrLow );
    	MY_LOG("rCamInfo.u4RealIsoforThrHigh      = %d", u4RealIsoforThrHigh);
    	MY_LOG("rCamInfo.getIdx_NR3D              = %d", rIspRegMgr.getIdx_NR3D());
        MY_LOG("rNR3D.blend      = 0x%8x. NR3D_GAIN: %d (before)", u4OldNr3dblend, u4OldNr3dGain);
        MY_LOG("rNR3D.blend      = 0x%8x. NR3D_GAIN: %d (after)",  rNR3D.blend, rNR3D.blend.bits.NR3D_GAIN);
    	MY_LOG("rNR3D.lmt_cpx    = 0x%8x", rNR3D.lmt_cpx);
    	MY_LOG("rNR3D.lmt_y_con1 = 0x%8x", rNR3D.lmt_y_con1);
    	MY_LOG("rNR3D.lmt_y_con2 = 0x%8x", rNR3D.lmt_y_con2);
    	MY_LOG("rNR3D.lmt_y_con3 = 0x%8x", rNR3D.lmt_y_con3);
    	MY_LOG("rNR3D.lmt_u_con1 = 0x%8x", rNR3D.lmt_u_con1);
    	MY_LOG("rNR3D.lmt_u_con2 = 0x%8x", rNR3D.lmt_u_con2);
    	MY_LOG("rNR3D.lmt_u_con3 = 0x%8x", rNR3D.lmt_u_con3);
    	MY_LOG("rNR3D.lmt_v_con1 = 0x%8x", rNR3D.lmt_v_con1);
    	MY_LOG("rNR3D.lmt_v_con2 = 0x%8x", rNR3D.lmt_v_con2);
    	MY_LOG("rNR3D.lmt_v_con3 = 0x%8x", rNR3D.lmt_v_con3);

        // For Interpolation result check.
        MY_LOG("rNR3D.blend.bits.NR3D_RND_Y                           = %d (%d, %d)x", rNR3D.blend.bits.NR3D_RND_Y          , rNr3dSettingforThrLow.blend.bits.NR3D_RND_Y          , rNr3dSettingforThrHigh.blend.bits.NR3D_RND_Y         );
        MY_LOG("rNR3D.blend.bits.NR3D_RND_U                           = %d (%d, %d)x", rNR3D.blend.bits.NR3D_RND_U          , rNr3dSettingforThrLow.blend.bits.NR3D_RND_U          , rNr3dSettingforThrHigh.blend.bits.NR3D_RND_U         );
        MY_LOG("rNR3D.blend.bits.NR3D_RND_V                           = %d (%d, %d)x", rNR3D.blend.bits.NR3D_RND_V          , rNr3dSettingforThrLow.blend.bits.NR3D_RND_V          , rNr3dSettingforThrHigh.blend.bits.NR3D_RND_V         );
        MY_LOG("rNR3D.lmt_cpx.bits.NR3D_LMT_CPX1                      = %d (%d, %d)",  rNR3D.lmt_cpx.bits.NR3D_LMT_CPX1     , rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX1     , rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX1    );
        MY_LOG("rNR3D.lmt_cpx.bits.NR3D_LMT_CPX2                      = %d (%d, %d)",  rNR3D.lmt_cpx.bits.NR3D_LMT_CPX2     , rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX2     , rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX2    );
        MY_LOG("rNR3D.lmt_cpx.bits.NR3D_LMT_CPX3                      = %d (%d, %d)",  rNR3D.lmt_cpx.bits.NR3D_LMT_CPX3     , rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX3     , rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX3    );
        MY_LOG("rNR3D.lmt_cpx.bits.NR3D_LMT_CPX4                      = %d (%d, %d)",  rNR3D.lmt_cpx.bits.NR3D_LMT_CPX4     , rNr3dSettingforThrLow.lmt_cpx.bits.NR3D_LMT_CPX4     , rNr3dSettingforThrHigh.lmt_cpx.bits.NR3D_LMT_CPX4    );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0                     = %d (%d, %d)",  rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0    , rNr3dSettingforThrLow.lmt_y_con1.bits.NR3D_LMT_Y0    , rNr3dSettingforThrHigh.lmt_y_con1.bits.NR3D_LMT_Y0   );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0_TH                  = %d (%d, %d)x", rNR3D.lmt_y_con1.bits.NR3D_LMT_Y0_TH , rNr3dSettingforThrLow.lmt_y_con1.bits.NR3D_LMT_Y0_TH , rNr3dSettingforThrHigh.lmt_y_con1.bits.NR3D_LMT_Y0_TH);
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1                     = %d (%d, %d)",  rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1    , rNr3dSettingforThrLow.lmt_y_con1.bits.NR3D_LMT_Y1    , rNr3dSettingforThrHigh.lmt_y_con1.bits.NR3D_LMT_Y1   );
        MY_LOG("rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1_TH                  = %d (%d, %d)x", rNR3D.lmt_y_con1.bits.NR3D_LMT_Y1_TH , rNr3dSettingforThrLow.lmt_y_con1.bits.NR3D_LMT_Y1_TH , rNr3dSettingforThrHigh.lmt_y_con1.bits.NR3D_LMT_Y1_TH);
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2                     = %d (%d, %d)",  rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2    , rNr3dSettingforThrLow.lmt_y_con2.bits.NR3D_LMT_Y2    , rNr3dSettingforThrHigh.lmt_y_con2.bits.NR3D_LMT_Y2   );
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2_TH                  = %d (%d, %d)x", rNR3D.lmt_y_con2.bits.NR3D_LMT_Y2_TH , rNr3dSettingforThrLow.lmt_y_con2.bits.NR3D_LMT_Y2_TH , rNr3dSettingforThrHigh.lmt_y_con2.bits.NR3D_LMT_Y2_TH);
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3                     = %d (%d, %d)",  rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3    , rNr3dSettingforThrLow.lmt_y_con2.bits.NR3D_LMT_Y3    , rNr3dSettingforThrHigh.lmt_y_con2.bits.NR3D_LMT_Y3   );
        MY_LOG("rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3_TH                  = %d (%d, %d)x", rNR3D.lmt_y_con2.bits.NR3D_LMT_Y3_TH , rNr3dSettingforThrLow.lmt_y_con2.bits.NR3D_LMT_Y3_TH , rNr3dSettingforThrHigh.lmt_y_con2.bits.NR3D_LMT_Y3_TH);
        MY_LOG("rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4                     = %d (%d, %d)",  rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4    , rNr3dSettingforThrLow.lmt_y_con3.bits.NR3D_LMT_Y4    , rNr3dSettingforThrHigh.lmt_y_con3.bits.NR3D_LMT_Y4   );
        MY_LOG("rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4_TH                  = %d (%d, %d)x", rNR3D.lmt_y_con3.bits.NR3D_LMT_Y4_TH , rNr3dSettingforThrLow.lmt_y_con3.bits.NR3D_LMT_Y4_TH , rNr3dSettingforThrHigh.lmt_y_con3.bits.NR3D_LMT_Y4_TH);
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U0                     = %d (%d, %d)",  rNR3D.lmt_u_con1.bits.NR3D_LMT_U0    , rNr3dSettingforThrLow.lmt_u_con1.bits.NR3D_LMT_U0    , rNr3dSettingforThrHigh.lmt_u_con1.bits.NR3D_LMT_U0   );
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U0_TH                  = %d (%d, %d)x", rNR3D.lmt_u_con1.bits.NR3D_LMT_U0_TH , rNr3dSettingforThrLow.lmt_u_con1.bits.NR3D_LMT_U0_TH , rNr3dSettingforThrHigh.lmt_u_con1.bits.NR3D_LMT_U0_TH);
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U1                     = %d (%d, %d)",  rNR3D.lmt_u_con1.bits.NR3D_LMT_U1    , rNr3dSettingforThrLow.lmt_u_con1.bits.NR3D_LMT_U1    , rNr3dSettingforThrHigh.lmt_u_con1.bits.NR3D_LMT_U1   );
        MY_LOG("rNR3D.lmt_u_con1.bits.NR3D_LMT_U1_TH                  = %d (%d, %d)x", rNR3D.lmt_u_con1.bits.NR3D_LMT_U1_TH , rNr3dSettingforThrLow.lmt_u_con1.bits.NR3D_LMT_U1_TH , rNr3dSettingforThrHigh.lmt_u_con1.bits.NR3D_LMT_U1_TH);
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U2                     = %d (%d, %d)",  rNR3D.lmt_u_con2.bits.NR3D_LMT_U2    , rNr3dSettingforThrLow.lmt_u_con2.bits.NR3D_LMT_U2    , rNr3dSettingforThrHigh.lmt_u_con2.bits.NR3D_LMT_U2   );
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U2_TH                  = %d (%d, %d)x", rNR3D.lmt_u_con2.bits.NR3D_LMT_U2_TH , rNr3dSettingforThrLow.lmt_u_con2.bits.NR3D_LMT_U2_TH , rNr3dSettingforThrHigh.lmt_u_con2.bits.NR3D_LMT_U2_TH);
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U3                     = %d (%d, %d)",  rNR3D.lmt_u_con2.bits.NR3D_LMT_U3    , rNr3dSettingforThrLow.lmt_u_con2.bits.NR3D_LMT_U3    , rNr3dSettingforThrHigh.lmt_u_con2.bits.NR3D_LMT_U3   );
        MY_LOG("rNR3D.lmt_u_con2.bits.NR3D_LMT_U3_TH                  = %d (%d, %d)x", rNR3D.lmt_u_con2.bits.NR3D_LMT_U3_TH , rNr3dSettingforThrLow.lmt_u_con2.bits.NR3D_LMT_U3_TH , rNr3dSettingforThrHigh.lmt_u_con2.bits.NR3D_LMT_U3_TH);
        MY_LOG("rNR3D.lmt_u_con3.bits.NR3D_LMT_U4                     = %d (%d, %d)",  rNR3D.lmt_u_con3.bits.NR3D_LMT_U4    , rNr3dSettingforThrLow.lmt_u_con3.bits.NR3D_LMT_U4    , rNr3dSettingforThrHigh.lmt_u_con3.bits.NR3D_LMT_U4   );
        MY_LOG("rNR3D.lmt_u_con3.bits.NR3D_LMT_U4_TH                  = %d (%d, %d)x", rNR3D.lmt_u_con3.bits.NR3D_LMT_U4_TH , rNr3dSettingforThrLow.lmt_u_con3.bits.NR3D_LMT_U4_TH , rNr3dSettingforThrHigh.lmt_u_con3.bits.NR3D_LMT_U4_TH);
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V0                     = %d (%d, %d)",  rNR3D.lmt_v_con1.bits.NR3D_LMT_V0    , rNr3dSettingforThrLow.lmt_v_con1.bits.NR3D_LMT_V0    , rNr3dSettingforThrHigh.lmt_v_con1.bits.NR3D_LMT_V0   );
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V0_TH                  = %d (%d, %d)x", rNR3D.lmt_v_con1.bits.NR3D_LMT_V0_TH , rNr3dSettingforThrLow.lmt_v_con1.bits.NR3D_LMT_V0_TH , rNr3dSettingforThrHigh.lmt_v_con1.bits.NR3D_LMT_V0_TH);
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V1                     = %d (%d, %d)",  rNR3D.lmt_v_con1.bits.NR3D_LMT_V1    , rNr3dSettingforThrLow.lmt_v_con1.bits.NR3D_LMT_V1    , rNr3dSettingforThrHigh.lmt_v_con1.bits.NR3D_LMT_V1   );
        MY_LOG("rNR3D.lmt_v_con1.bits.NR3D_LMT_V1_TH                  = %d (%d, %d)x", rNR3D.lmt_v_con1.bits.NR3D_LMT_V1_TH , rNr3dSettingforThrLow.lmt_v_con1.bits.NR3D_LMT_V1_TH , rNr3dSettingforThrHigh.lmt_v_con1.bits.NR3D_LMT_V1_TH);
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V2                     = %d (%d, %d)",  rNR3D.lmt_v_con2.bits.NR3D_LMT_V2    , rNr3dSettingforThrLow.lmt_v_con2.bits.NR3D_LMT_V2    , rNr3dSettingforThrHigh.lmt_v_con2.bits.NR3D_LMT_V2   );
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V2_TH                  = %d (%d, %d)x", rNR3D.lmt_v_con2.bits.NR3D_LMT_V2_TH , rNr3dSettingforThrLow.lmt_v_con2.bits.NR3D_LMT_V2_TH , rNr3dSettingforThrHigh.lmt_v_con2.bits.NR3D_LMT_V2_TH);
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V3                     = %d (%d, %d)",  rNR3D.lmt_v_con2.bits.NR3D_LMT_V3    , rNr3dSettingforThrLow.lmt_v_con2.bits.NR3D_LMT_V3    , rNr3dSettingforThrHigh.lmt_v_con2.bits.NR3D_LMT_V3   );
        MY_LOG("rNR3D.lmt_v_con2.bits.NR3D_LMT_V3_TH                  = %d (%d, %d)x", rNR3D.lmt_v_con2.bits.NR3D_LMT_V3_TH , rNr3dSettingforThrLow.lmt_v_con2.bits.NR3D_LMT_V3_TH , rNr3dSettingforThrHigh.lmt_v_con2.bits.NR3D_LMT_V3_TH);
        MY_LOG("rNR3D.lmt_v_con3.bits.NR3D_LMT_V4                     = %d (%d, %d)",  rNR3D.lmt_v_con3.bits.NR3D_LMT_V4    , rNr3dSettingforThrLow.lmt_v_con3.bits.NR3D_LMT_V4    , rNr3dSettingforThrHigh.lmt_v_con3.bits.NR3D_LMT_V4   );
        MY_LOG("rNR3D.lmt_v_con3.bits.NR3D_LMT_V4_TH                  = %d (%d, %d)x", rNR3D.lmt_v_con3.bits.NR3D_LMT_V4_TH , rNr3dSettingforThrLow.lmt_v_con3.bits.NR3D_LMT_V4_TH , rNr3dSettingforThrHigh.lmt_v_con3.bits.NR3D_LMT_V4_TH);
#endif
	}

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_MFB(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_MFB_T& rMFB)
{


    if((rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing) ||
       (rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)||
       (rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing) ||
       (rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)) {
	    rMFB.ll_con3.bits.BLD_LL_DB_EN = 0;
	    rMFB.ll_con3.bits.BLD_LL_BRZ_EN = 0;
    }

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    MY_LOG("rMFB.profile = %d", rCamInfo.eIspProfile);
    MY_LOG("rMFB.ll_con2 = 0x%8x", rMFB.ll_con2);
    MY_LOG("rMFB.ll_con3 = 0x%8x", rMFB.ll_con3);
    MY_LOG("rMFB.ll_con4 = 0x%8x", rMFB.ll_con4);
    MY_LOG("rMFB.ll_con5 = 0x%8x", rMFB.ll_con5);
    MY_LOG("rMFB.ll_con6 = 0x%8x", rMFB.ll_con6);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_MIXER3(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_MIXER3_T& rMIXER3)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    rMIXER3.ctrl_0.bits.MIX3_WT_SEL = 1;
    rMIXER3.ctrl_0.bits.MIX3_B0 = 0;
    rMIXER3.ctrl_0.bits.MIX3_B1 = 0xFF;
    rMIXER3.ctrl_0.bits.MIX3_DT = 1;

    rMIXER3.ctrl_1.bits.MIX3_M0 = 0;
    rMIXER3.ctrl_1.bits.MIX3_M1 = 0xFF;

    MY_LOG("rMIXER3.ctrl_0 = 0x%8x", rMIXER3.ctrl_0);
    MY_LOG("rMIXER3.ctrl_1 = 0x%8x", rMIXER3.ctrl_1);
    MY_LOG("rMIXER3.spare = 0x%8x", rMIXER3.spare);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_LCE(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_LCE_T& rLCE)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    MY_LOG("rLCE.qua = 0x%8x", rLCE.qua);
    #endif
}
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_CCM_T
IspTuningCustom::
evaluate_CCM_index(RAWIspCamInfo const& rCamInfo)
{

#if 0
    MY_LOG("%s()\n", __FUNCTION__);

    MY_LOG(
        "[+evaluate_CCM_index]"
        "(eIdx_CCM, i4CCT, i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_CCM
        , rCamInfo.rAWBInfo.i4CCT
        , rCamInfo.rAWBInfo.i4FluorescentIndex);

#endif

    EIndex_CCM_T eIdx_CCM_new = rCamInfo.eIdx_CCM;

//    -----------------|---|---|--------------|---|---|------------------
//                                THA TH1 THB              THC TH2  THD

    MINT32 const THA = 3318;
    MINT32 const TH1 = 3484;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const TH2 = 5050;
    MINT32 const THD = 5316;
    MINT32 const F_IDX_TH1 = 25;
    MINT32 const F_IDX_TH2 = -25;

    switch  (rCamInfo.eIdx_CCM)
    {
    case eIDX_CCM_TL84:
        if  ( rCamInfo.rAWBInfo.i4CCT < THB )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex < F_IDX_TH2 )
                eIdx_CCM_new = eIDX_CCM_CWF;
            else
                eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_CWF:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH1 )
                eIdx_CCM_new = eIDX_CCM_TL84;
            else
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_D65:
        if  ( rCamInfo.rAWBInfo.i4CCT > THC )
        {
	        eIdx_CCM_new = eIDX_CCM_D65;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT > TH1 )
        {
            if(rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH2)
                eIdx_CCM_new = eIDX_CCM_TL84;
            else
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        break;
    default:
        break;
    }

    if  ( rCamInfo.eIdx_CCM != eIdx_CCM_new )
    {
        MY_LOG(
            "[-evaluate_CCM_index] CCM Idx(old,new)=(%d,%d)"
            , rCamInfo.eIdx_CCM, eIdx_CCM_new
        );
    }

    return  eIdx_CCM_new;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_smooth_ccm_with_preference_gain(RAWIspCamInfo const& rCamInfo)
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_isp_interpolation(RAWIspCamInfo const& rCamInfo)
{
    if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            if(
                   rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR

            )
			{
	            return MFALSE;
			}
       }
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IspTuningCustom::
get_CCM_smooth_method(RAWIspCamInfo const& rCamInfo)
{
    // 0: CCM (without flash info)
    // 1: enable flash CCM
    return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
get_SWNR_ENC_enable_ISO_threshold(RAWIspCamInfo const& rCamInfo)
{
    if ((rCamInfo.eIspProfile == EIspProfile_Capture_SWNR) ||
        (rCamInfo.eIspProfile == EIspProfile_VSS_Capture_SWNR) ||
        (rCamInfo.eIspProfile == EIspProfile_PureRAW_Capture_SWNR) ||
        (rCamInfo.eIspProfile == EIspProfile_PureRAW_TPipe_Capture_SWNR)) {
         return get_normal_SWNR_ENC_enable_ISO_threshold();
    }
    else if (
             (rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR) ||
             (rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR) ||
             (rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR) ||
             (rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR) ||
             (rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR) ||
             (rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)

            )
    {
         return get_MFB_SWNR_ENC_enable_ISO_threshold();
    }

    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_PCA_LUT_T
IspTuningCustom::
evaluate_PCA_LUT_index(RAWIspCamInfo const& rCamInfo)
{
    //MY_LOG("%s()\n", __FUNCTION__);

    // TODO: Add your code below...

/*
    MY_LOG(
        "[+evaluate_PCA_LUT_index]"
        "(rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex
    );
*/
    EIndex_PCA_LUT_T eIdx_PCA_LUT_new = rCamInfo.eIdx_PCA_LUT;

//    -----------------|-------|--------------|-------|------------------
//                    THA     THB            THC     THD

    MINT32 const THA = 3318;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const THD = 5316;

    switch  (rCamInfo.eIdx_PCA_LUT)
    {
    case eIDX_PCA_HIGH_2:
        eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        break;
    case eIDX_PCA_MIDDLE_2:
        eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        break;
    case eIDX_PCA_LOW_2:
        eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        break;
    case eIDX_PCA_HIGH:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THC )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        break;
    case eIDX_PCA_MIDDLE:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        break;
    case eIDX_PCA_LOW:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
	        eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT > THB )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        break;
    }

    if  ( rCamInfo.eIdx_PCA_LUT != eIdx_PCA_LUT_new )
    {
        MY_LOG(
            "[-evaluate_PCA_LUT_index] PCA_LUT_index(old,new)=(%d,%d)"
            , rCamInfo.eIdx_PCA_LUT, eIdx_PCA_LUT_new
        );
    }

    return eIdx_PCA_LUT_new;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_PCA_LUT_T
IspTuningCustom::
evaluate_PCA_LUT_index_for_IHDR(RAWIspCamInfo const& rCamInfo)
{
    //MY_LOG("%s()\n", __FUNCTION__);

    // TODO: Add your code below...

#if 0
    MY_LOG(
        "[+evaluate_PCA_LUT_index]"
        "(rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex
    );

#endif

    EIndex_PCA_LUT_T eIdx_PCA_LUT_new = rCamInfo.eIdx_PCA_LUT;

//    -----------------|-------|--------------|-------|------------------
//                    THA     THB            THC     THD

    MINT32 const THA = 3318;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const THD = 5316;

    switch  (rCamInfo.eIdx_PCA_LUT)
    {
    case eIDX_PCA_HIGH:
        eIdx_PCA_LUT_new = eIDX_PCA_HIGH_2;
        break;
    case eIDX_PCA_MIDDLE:
        eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE_2;
        break;
    case eIDX_PCA_LOW:
        eIdx_PCA_LUT_new = eIDX_PCA_LOW_2;
        break;
    case eIDX_PCA_HIGH_2:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW_2;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THC )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE_2;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH_2;
        }
        break;
    case eIDX_PCA_MIDDLE_2:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH_2;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW_2;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE_2;
        }
        break;
    case eIDX_PCA_LOW_2:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
	        eIdx_PCA_LUT_new = eIDX_PCA_HIGH_2;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT > THB )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE_2;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW_2;
        }
        break;
    }

    if  ( rCamInfo.eIdx_PCA_LUT != eIdx_PCA_LUT_new )
    {
        MY_LOG(
            "[-evaluate_PCA_LUT_index] PCA_LUT_index(old,new)=(%d,%d)"
            , rCamInfo.eIdx_PCA_LUT, eIdx_PCA_LUT_new
        );
    }

    return eIdx_PCA_LUT_new;
}

/*******************************************************************************
*
* eIdx_Shading_CCT_old:
*   [in] the previous color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
* i4CCT:
*   [in] the current color temperature from 3A.
*
*
* return:
*   [out] the current color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
*******************************************************************************/
EIndex_Shading_CCT_T
IspTuningCustom::
evaluate_Shading_CCT_index  (
        RAWIspCamInfo const& rCamInfo
)   const
{
    MINT32 i4CCT = rCamInfo.rAWBInfo.i4CCT;

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 3257;
    MINT32 const THH1 = 3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  (rCamInfo.eIdx_Shading_CCT)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THL2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    }

    if  ( rCamInfo.eIdx_Shading_CCT != eIdx_Shading_CCT_new )
    {
        MY_LOG(
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , rCamInfo.eIdx_Shading_CCT, eIdx_Shading_CCT_new,i4CCT
        );
    }

    return  eIdx_Shading_CCT_new;
}

MVOID
IspTuningCustom::
reset_ISO_SmoothBuffer()
{
    total_RA_num_frames_= 0;
    MY_LOG("reset_ISO total_RA_num_frames_=0");
    memset(ISO_Buffer_, 6, sizeof(ISO_Buffer_));
    MY_LOG("[%s] total_RA_num_frames_(%d)", __FUNCTION__, total_RA_num_frames_ );
    MY_LOG("[%s] ISO_Buffer_[] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n}", __FUNCTION__,
        ISO_Buffer_[0], ISO_Buffer_[1], ISO_Buffer_[2], ISO_Buffer_[3], ISO_Buffer_[4],
        ISO_Buffer_[5], ISO_Buffer_[6], ISO_Buffer_[7], ISO_Buffer_[8], ISO_Buffer_[9] );
}

static MINT32 ratioMapping(MINT32 i4Iso)
{
#define LERP(x, lo_x, lo_y, hi_x, hi_y)\
    (((hi_x) - (x))*(lo_y) + ((x) - (lo_x))*(hi_y)) / ((hi_x) - (lo_x))

    static const MINT32 iso[10] =
    {100, 200, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200};

    static const MINT32 rto[10] =
    {24, 22, 20, 18, 16, 14, 12, 10, 8, 6}; //Tower modify for iso1600 Noise 2014-12-26

    MINT32 i = 0;
    MINT32 i4Rto = 32;

    if (i4Iso < iso[0])
    {
        i4Rto = rto[0];
    }
    else if (i4Iso >= iso[9])
    {
        i4Rto = rto[9];
    }
    else
    {
        for (i = 1; i < 10; i++)
        {
            if (i4Iso < iso[i])
                break;
        }
        i4Rto = LERP(i4Iso, iso[i-1], rto[i-1], iso[i], rto[i]);
    }
    return i4Rto;
}

MINT32
IspTuningCustom::
evaluate_Shading_Ratio  (
        RAWIspCamInfo const& rCamInfo
)
{
    /*
        Sample code for evaluate shading ratio.
        The shading ratio is an integer ranging from 0(0%) to 32(100%).
        All informations can be obtained via rCamInfo.
        The following sample code shows a shading ratio evaluated by ISO value with temporal smoothness.
    */
    customRatioLog = property_get_int32("vendor.debug.shading.custom.ratio.log", 0);

    MINT32 Avg_Frm_Cnt = 5;
    MINT32 i = 0;
    MINT32 i4Rto = 8; //32;
    MINT32 i4Iso = rCamInfo.rAEInfo.u4RealISOValue;

    int idx = total_RA_num_frames_ % Avg_Frm_Cnt;
    int *p_global_Ra = ISO_Buffer_;
    int n_frames, avgISO;

    ISO_Buffer_[idx] = i4Iso;

    // to prevent total frames overflow
    if (total_RA_num_frames_ >= 65535){
        total_RA_num_frames_ = 0;
    }
    total_RA_num_frames_++;
    if (total_RA_num_frames_ < 20){
        avgISO = 8;
        CAM_LOGD_IF(customRatioLog,"[%s] first avgISO = %d\n", __FUNCTION__, avgISO);
    } else {
        // smooth
        n_frames = ( total_RA_num_frames_ <  Avg_Frm_Cnt) ? (total_RA_num_frames_) : (Avg_Frm_Cnt);
        avgISO = 0;
        for (int k = 0; k < n_frames; k++) {
            avgISO += ISO_Buffer_[k];
        }
        avgISO /= n_frames;
        CAM_LOGD_IF(customRatioLog,"[%s] ISO_Buffer_[] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n}", __FUNCTION__,
        ISO_Buffer_[0], ISO_Buffer_[1], ISO_Buffer_[2], ISO_Buffer_[3], ISO_Buffer_[4],
        ISO_Buffer_[5], ISO_Buffer_[6], ISO_Buffer_[7], ISO_Buffer_[8], ISO_Buffer_[9] );
        CAM_LOGD_IF(customRatioLog,"[%s] avgISO = %d", __FUNCTION__, avgISO);
        if (rCamInfo.rFlashInfo.isFlash == 2)
        {
            i4Rto = ratioMapping(i4Iso);
            MY_LOG("[%s] Main flash iso(%d), ratio(%d)", __FUNCTION__, i4Iso, i4Rto);
        }
        else
        {
            i4Rto = ratioMapping(avgISO);
        }
    }
    return i4Rto;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sample code for sub sensor customization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
EIndex_Shading_CCT_T
CTIspTuningCustom<ESensorDev_Sub>::
evaluate_Shading_CCT_index(RAWIspCamInfo const& rCamInfo) const
{
    MY_LOG("CTIspTuningCustom<ESensorDev_Main> %s()\n", __FUNCTION__);

    MINT32 i4CCT = rCamInfo.rAWBInfo.i4CCT;

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 2500;//3257;
    MINT32 const THH1 = 2800;//3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  (rCamInfo.eIdx_Shading_CCT)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
         eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THL2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    }

    if  ( rCamInfo.eIdx_Shading_CCT != eIdx_Shading_CCT_new )
    {
        MY_LOG(
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , rCamInfo.eIdx_Shading_CCT, eIdx_Shading_CCT_new,i4CCT
        );
    }

    return  eIdx_Shading_CCT_new;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_index(MUINT32 const u4Iso) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso < 150 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso < 300 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso < 600 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso < 1000 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso < 1400 )
    {
        return  eIDX_ISO_1200;
    }
    else if ( u4Iso < 1800 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso < 2200 )
    {
        return  eIDX_ISO_2000;
    }
    else if ( u4Iso < 2600 )
    {
        return  eIDX_ISO_2400;
    }
    else if ( u4Iso < 3000 )
    {
        return  eIDX_ISO_2800;
    }

    return  eIDX_ISO_3200;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
map_ISO_index_to_value(EIndex_ISO_T const u4IsoIdx) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4IsoIdx == eIDX_ISO_100 )
    {
        return  100;
    }
    else if ( u4IsoIdx == eIDX_ISO_200 )
    {
        return  200;
    }
    else if ( u4IsoIdx == eIDX_ISO_400 )
    {
        return  400;
    }
    else if ( u4IsoIdx == eIDX_ISO_800 )
    {
        return  800;
    }
    else if ( u4IsoIdx == eIDX_ISO_1200 )
    {
        return  1200;
    }
    else if ( u4IsoIdx == eIDX_ISO_1600 )
    {
        return  1600;
    }
    else if ( u4IsoIdx == eIDX_ISO_2000 )
    {
        return  2000;
    }
    else if ( u4IsoIdx == eIDX_ISO_2400 )
    {
        return  2400;
    }
    else if ( u4IsoIdx == eIDX_ISO_2800 )
    {
        return  2800;
    }
    else if ( u4IsoIdx == eIDX_ISO_3200 )
    {
        return  3200;
    }

    return  0;  // If no ISO Index matched, return 0.

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
remap_ISO_value(MUINT32 const u4Iso) const
{
    MUINT32 remapIso = u4Iso;

    //add your remap ISO code here

    MY_LOG_IF(fgLogEn, "[%s] ISO: in(%d), out(%d)", __FUNCTION__, u4Iso, remapIso);
    return remapIso;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_upper_index(MUINT32 const u4Iso) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso <= 100 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso <= 200 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso <= 400 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso <= 800 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso <= 1200 )
    {
        return  eIDX_ISO_1200;
    }
    else if ( u4Iso <= 1600 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso <= 2000 )
    {
        return  eIDX_ISO_2000;
    }
    else if ( u4Iso <= 2400 )
    {
        return  eIDX_ISO_2400;
    }
    else if ( u4Iso <= 2800 )
    {
        return  eIDX_ISO_2800;
    }

    return  eIDX_ISO_3200;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_lower_index(MUINT32 const u4Iso) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso < 200 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso < 400 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso < 800 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso < 1200 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso < 1600 )
    {
        return  eIDX_ISO_1200;
    }
    else if ( u4Iso < 2000 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso < 2400 )
    {
        return  eIDX_ISO_2000;
    }
    else if ( u4Iso < 2800 )
    {
        return  eIDX_ISO_2400;
    }
    else if ( u4Iso < 3200 )
    {
        return  eIDX_ISO_2800;
    }

    return  eIDX_ISO_3200;

}

EIndex_ANR_TBL_ISO_T
IspTuningCustom::
evaluate_ANR_TBL_ISO_index(RAWIspCamInfo const& rCamInfo)
{

    //    -----------------|-------|--------------|--------
    //                    THA     THB            THC


    MUINT32 const THA = 100;
    MUINT32 const THB = 400;
    MUINT32 const THC = 1600;
    MUINT32 const THD = 3200;
    MUINT32 const THE = 6400;
    MUINT32 const THF = 12800;


    EIndex_ANR_TBL_ISO_T eIdx_ANR_TBL_ISO;

    if (rCamInfo.u4ISOValue < THA){
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_0;
    }
    else if (rCamInfo.u4ISOValue < THB){
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_1;
    }
    else if (rCamInfo.u4ISOValue < THC){
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_2;
    }
    else if (rCamInfo.u4ISOValue < THD){
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_3;
    }
    else if (rCamInfo.u4ISOValue < THE){
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_4;
    }
    else if (rCamInfo.u4ISOValue < THF){
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_5;
    }
    else{
        eIdx_ANR_TBL_ISO = eIDX_ANR_TBL_ISO_5;
    }

    return eIdx_ANR_TBL_ISO;
}

EIndex_ANR_TBL_CT_T
IspTuningCustom::
evaluate_ANR_TBL_CT_index(RAWIspCamInfo const& rCamInfo)
{
    //MY_LOG("%s()\n", __FUNCTION__);

    // TODO: Add your code below...

/*
    MY_LOG(
        "[+evaluate_PCA_LUT_index]"
        "(rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex
    );
*/
    EIndex_ANR_TBL_CT_T eIdx_ANR_TBL_new;

//    -----------------|-------|--------------|-------|------------------
//                    THA     THB            THC     THD

    MINT32 const THA = 4317;

        if (rCamInfo.rAWBInfo.i4CCT < THA){
            eIdx_ANR_TBL_new = eIDX_ANR_TBL_CCT_0;
        }
        else{
            eIdx_ANR_TBL_new = eIDX_ANR_TBL_CCT_1;
        }

    return eIdx_ANR_TBL_new;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_capture_in_color(RAWIspCamInfo const& rCamInfo)
{
    if (// normal capture
        rCamInfo.eIspProfile == EIspProfile_Capture)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

