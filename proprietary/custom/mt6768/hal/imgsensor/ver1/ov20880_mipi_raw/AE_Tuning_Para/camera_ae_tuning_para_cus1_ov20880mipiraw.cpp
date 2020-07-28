//Gionee <Amigo_Skip> <wanguocheng> <20170210> SWW1617 Front Camera OV20880 Tuning begin
/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2016. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "camera_custom_nvram.h"

#include "camera_ae_tuning_para_ov20880mipiraw.h"

template <>
AE_NVRAM_T const&
getAENVRAM_ov20880mipiraw<CAM_SCENARIO_CUSTOM1>() // HDR
{
	// rDevicesInfo
	static AE_DEVICES_INFO_T  g_rDevicesInfo =
	{
		1195,  // u4MinGain
		15872,  // u4MaxGain
		100,  // u4MiniISOGain
		64,  // u4GainStepUnit
		13500,  // u4PreExpUnit
		30,  // u4PreMaxFrameRate
		13500,  // u4VideoExpUnit
		30,  // u4VideoMaxFrameRate
		1024,  // u4Video2PreRatio
		8667,  // u4CapExpUnit
		30,  // u4CapMaxFrameRate
		1024,  // u4Cap2PreRatio
		13500,  // u4Video1ExpUnit
		120,  // u4Video1MaxFrameRate
		1024,  // u4Video12PreRatio
		13500,  // u4Video2ExpUnit
		30,  // u4Video2MaxFrameRate
		1024,  // u4Video22PreRatio
		13500,  // u4Custom1ExpUnit
		30,  // u4Custom1MaxFrameRate
		1024,  // u4Custom12PreRatio
		13500,  // u4Custom2ExpUnit
		30,  // u4Custom2MaxFrameRate
		1024,  // u4Custom22PreRatio
		13500,  // u4Custom3ExpUnit
		30,  // u4Custom3MaxFrameRate
		1024,  // u4Custom32PreRatio
		13500,  // u4Custom4ExpUnit
		30,  // u4Custom4MaxFrameRate
		1024,  // u4Custom42PreRatio
		13500,  // u4Custom5ExpUnit
		30,  // u4Custom5MaxFrameRate
		1024,  // u4Custom52PreRatio
		20,  // u4LensFno
		350  // u4FocusLength_100x
	};

	// rHistConfig
	static AE_HIST_CFG_T  g_rHistConfig =
	{
		4,  // u4HistHighThres
		40,  // u4HistLowThres
		2,  // u4MostBrightRatio
		1,  // u4MostDarkRatio
		160,  // u4CentralHighBound
		20,  // u4CentralLowBound
		// u4OverExpThres
		{
			240, 230, 220, 210, 200
		},
		// u4HistStretchThres
		{
			62, 70, 82, 108, 141
		},
		// u4BlackLightThres
		{
			18, 22, 26, 30, 34
		}
	};

	// rCCTConfig
	static AE_CCT_CFG_T  g_rCCTConfig =
	{
		1,  // bEnableBlackLight
		1,  // bEnableHistStretch
		1,  // bEnableAntiOverExposure
		1,  // bEnableTimeLPF
		1,  // bEnableCaptureThres
		1,  // bEnableVideoThres
		1,  // bEnableVideo1Thres
		1,  // bEnableVideo2Thres
		1,  // bEnableCustom1Thres
		1,  // bEnableCustom2Thres
		1,  // bEnableCustom3Thres
		1,  // bEnableCustom4Thres
		1,  // bEnableCustom5Thres
		1,  // bEnableStrobeThres
		66,  // u4AETarget
		66,  // u4StrobeAETarget
		50,  // u4InitIndex
		4,  // u4BackLightWeight
		32,  // u4HistStretchWeight
		4,  // u4AntiOverExpWeight
		2,  // u4BlackLightStrengthIndex
		2,  // u4HistStretchStrengthIndex
		2,  // u4AntiOverExpStrengthIndex
		2,  // u4TimeLPFStrengthIndex
		// u4LPFConvergeLevel
		{
			1, 3, 5, 7, 8
		},
		90,  // u4InDoorEV
		5,//-6,  // i4BVOffset
		48,  // u4PreviewFlareOffset
		48,  // u4CaptureFlareOffset
		3,  // u4CaptureFlareThres
		64,  // u4VideoFlareOffset
		3,  // u4VideoFlareThres
		64,  // u4CustomFlareOffset
		3,  // u4CustomFlareThres
		64,  // u4StrobeFlareOffset
		3,  // u4StrobeFlareThres
		64, //16,  // u4PrvMaxFlareThres
		32, //8,  // u4PrvMinFlareThres
		30,  // u4VideoMaxFlareThres
		0,  // u4VideoMinFlareThres
		18,  // u4FlatnessThres
		75,  // u4FlatnessStrength
		// rMeteringSpec
		{
			// rHS_Spec
			{
				1,  // bEnableHistStretch
				1024,  // u4HistStretchWeight
				50,  // u4Pcent
				165,  // u4Thd
				80,  // u4FlatThd
				120,  // u4FlatBrightPcent
				120,  // u4FlatDarkPcent
				// sFlatRatio
				{
					1000,  // u4X1
					1024,  // u4Y1
					2400,  // u4X2
					0  // u4Y2
				},
				1,  // bEnableGreyTextEnhance
				1800,  // u4GreyTextFlatStart
				// sGreyTextRatio
				{
					10,  // u4X1
					1024,  // u4Y1
					80,  // u4X2
					0  // u4Y2
				}
			},
			// rAOE_Spec
			{
				1,  // bEnableAntiOverExposure
				1024,  // u4AntiOverExpWeight
				10,  // u4Pcent
				220,  // u4Thd
				1,  // bEnableCOEP
				1,  // u4COEPcent
				106,  // u4COEThd
				0,  // u4BVCompRatio
				// sCOEYRatio
				{
					23,  // u4X1
					1024,  // u4Y1
					47,  // u4X2
					0  // u4Y2
				},
				// sCOEDiffRatio
				{
					1500,  // u4X1
					0,  // u4Y1
					2100,  // u4X2
					1024  // u4Y2
				}
			},
			// rABL_Spec
			{
				1,  // bEnableBlackLight
				1024,  // u4BackLightWeight
				500,//400,  // u4Pcent
				65,//25,  // u4Thd
				255,  // u4CenterHighBnd
				250,  // u4TargetStrength
				// sFgBgEVRatio
				{
					2100,  // u4X1
					0,  // u4Y1
					4000,  // u4X2
					1024  // u4Y2
				},
				// sBVRatio
				{
					3800,  // u4X1
					0,  // u4Y1
					5000,  // u4X2
					1024  // u4Y2
				}
			},
			// rNS_Spec
			{
				1,  // bEnableNightScene
				10,  // u4Pcent
				150,  // u4Thd
				72,  // u4FlatThd
				180,  // u4BrightTonePcent
				80,  // u4BrightToneThd
				500,  // u4LowBndPcent
				5,  // u4LowBndThd
				26,  // u4LowBndThdLimit
				50,  // u4FlatBrightPcent
				300,  // u4FlatDarkPcent
				// sFlatRatio
				{
					1200,  // u4X1
					1024,  // u4Y1
					2800,  // u4X2
					0  // u4Y2
				},
				// sBVRatio
				{
					-500,  // u4X1
					1024,  // u4Y1
					3000,  // u4X2
					0  // u4Y2
				},
				1,  // bEnableNightSkySuppresion
				// sSkyBVRatio
				{
					-4000,  // u4X1
					1024,  // u4Y1
					-2000,  // u4X2
					0  // u4Y2
				}
			},
			// rTOUCHFD_Spec
			{
				66,  // uMeteringYLowBound
				68,  // uMeteringYHighBound
				68,  // uFaceYHighBound
				70,  // uFaceYHighBound
				5,  // uFaceCentralWeight
				120,  // u4MeteringStableMax
				100  // u4MeteringStableMin
			}
		},
		// rFlareSpec
		{
			// uPrvFlareWeightArr
			{
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1
			},
			// uVideoFlareWeightArr
			{
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1
			},
			96,  // u4FlareStdThrHigh
			48,  // u4FlareStdThrLow
			0,  // u4PrvCapFlareDiff
			2,  // u4FlareMaxStepGap_Fast
			0,  // u4FlareMaxStepGap_Slow
			1800,  // u4FlarMaxStepGapLimitBV
			2  // u4FlareAEStableCount
		},
		// rAEMovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			190,  // u4Bright2TargetEnd
			10,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			85,  // u4B2TStart
			70,  // u4D2TEnd
			85  // u4D2TStart
		},
		// rAEVideoMovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAEVideo1MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAEVideo2MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAECustom1MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAECustom2MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAECustom3MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAECustom4MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAECustom5MovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			150,  // u4Bright2TargetEnd
			20,  // u4Dark2TargetStart
			90,  // u4B2TEnd
			40,  // u4B2TStart
			30,  // u4D2TEnd
			90  // u4D2TStart
		},
		// rAEFaceMovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			190,  // u4Bright2TargetEnd
			10,  // u4Dark2TargetStart
			80,  // u4B2TEnd
			30,  // u4B2TStart
			20,  // u4D2TEnd
			60  // u4D2TStart
		},
		// rAETrackingMovingRatio
		{
			100,  // u4SpeedUpRatio
			100,  // u4GlobalRatio
			190,  // u4Bright2TargetEnd
			10,  // u4Dark2TargetStart
			80,  // u4B2TEnd
			30,  // u4B2TStart
			20,  // u4D2TEnd
			60  // u4D2TStart
		},
		// rAEAOENVRAMParam
		{
			1,  // i4AOEStrengthIdx
			130,  // u4BVCompRatio
			// rAEAOEAlgParam
			{
				{
					47,  // u4Y_Target
					25,  // u4AOE_OE_percent
					210,  // u4AOE_OEBound
					10,  // u4AOE_DarkBound
					950,  // u4AOE_LowlightPrecent
					1,  // u4AOE_LowlightBound
					145,  // u4AOESceneLV_L
					180,  // u4AOESceneLV_H
					40  // u4AOE_SWHdrLE_Bound
				},
				{
					47,  // u4Y_Target
					25,  // u4AOE_OE_percent
					210,  // u4AOE_OEBound
					15,  // u4AOE_DarkBound
					950,  // u4AOE_LowlightPrecent
					3,  // u4AOE_LowlightBound
					145,  // u4AOESceneLV_L
					180,  // u4AOESceneLV_H
					40  // u4AOE_SWHdrLE_Bound
				},
				{
					47,  // u4Y_Target
					25,  // u4AOE_OE_percent
					210,  // u4AOE_OEBound
					25,  // u4AOE_DarkBound
					950,  // u4AOE_LowlightPrecent
					8,  // u4AOE_LowlightBound
					145,  // u4AOESceneLV_L
					180,  // u4AOESceneLV_H
					40  // u4AOE_SWHdrLE_Bound
				}
			}
		}
	};

	// rAEHDRConfig
	static HDR_AE_CFG_T  g_rHDRAEConfig =
	{
		3072,  // i4RMGSeg
		768,  // i4RMGTurn
		35,  // i4HDRTarget_L
		40,  // i4HDRTarget_H
		100,  // i4HDRTargetLV_L
		120,  // i4HDRTargetLV_H
		20,  // i4OverExpoRatio
		212,  // i4OverExpoTarget
		120,  // i4OverExpoLV_L
		180,  // i4OverExpoLV_H
		4,  // i4UnderExpoContrastThr
		// i4UnderExpoTargetTbl
		{
         //  0   1   2   3   4   5   6   7   8   9  10
             3,  3,  3,  3,  3,  2,  1,  1,  1,  1,  1    // i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
		},
		950,  // i4UnderExpoRatio
		1000,  // i4AvgExpoRatio
		8,  // i4AvgExpoTarget
		768,  // i4HDRAESpeed
		40,  // i4HDRConvergeThr
		20,  // i4SWHdrLEThr
		180,  // i4SWHdrSERatio
		1024,  // i4SWHdrSETarget
		0  // i4SWHdrBaseGain
	};

	static AE_NVRAM_T strAENVRAM =
	{
		g_rDevicesInfo,
		g_rHistConfig,
		g_rCCTConfig,
		g_rHDRAEConfig
	};
	return strAENVRAM;
};
