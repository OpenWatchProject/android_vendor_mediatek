//Gionee <Amigo_Skip> <fangrui> <20170228> SW17G04 Rear Main Camera S5K2P7 Tuning begin
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

#include "camera_ae_tuning_para_s5k2p7mipiraw.h"

template <>
AE_NVRAM_T const&
getAENVRAM_s5k2p7mipiraw<CAM_SCENARIO_CUSTOM4>() // STEREO
{
	// rDevicesInfo
	static AE_DEVICES_INFO_T  g_rDevicesInfo =
	{
		1144,  // u4MinGain
		16384,  // u4MaxGain
		100,  // u4MiniISOGain
		32,  // u4GainStepUnit
		9144,  // u4PreExpUnit
		30,  // u4PreMaxFrameRate
		9144,  // u4VideoExpUnit
		30,  // u4VideoMaxFrameRate
		1024,  // u4Video2PreRatio
		9144,  // u4CapExpUnit
		30,  // u4CapMaxFrameRate
		1024,  // u4Cap2PreRatio
		9144,  // u4Video1ExpUnit
		121,  // u4Video1MaxFrameRate
		1024,  // u4Video12PreRatio
		9144,  // u4Video2ExpUnit
		30,  // u4Video2MaxFrameRate
		1024,  // u4Video22PreRatio
		9144,  // u4Custom1ExpUnit
		30,  // u4Custom1MaxFrameRate
		1024,  // u4Custom12PreRatio
		9144,  // u4Custom2ExpUnit
		30,  // u4Custom2MaxFrameRate
		1024,  // u4Custom22PreRatio
		9144,  // u4Custom3ExpUnit
		30,  // u4Custom3MaxFrameRate
		1024,  // u4Custom32PreRatio
		9144,  // u4Custom4ExpUnit
		30,  // u4Custom4MaxFrameRate
		1024,  // u4Custom42PreRatio
		9144,  // u4Custom5ExpUnit
		30,  // u4Custom5MaxFrameRate
		1024,  // u4Custom52PreRatio
		18,  // u4LensFno
		395  // u4FocusLength_100x
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
		
		
       {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
		
	
       {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
		
	
		
        {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
		
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
		50,  // u4AETarget
		48,  // u4StrobeAETarget
		50,  // u4InitIndex
		4,  // u4BackLightWeight
		32,  // u4HistStretchWeight
		4,  // u4AntiOverExpWeight
		2,  // u4BlackLightStrengthIndex
		2,  // u4HistStretchStrengthIndex
		2,  // u4AntiOverExpStrengthIndex
		2,  // u4TimeLPFStrengthIndex
		
        {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
		
		90,  // u4InDoorEV
		-19,  //-4,  // i4BVOffset
		64,  // u4PreviewFlareOffset
		64,  // u4CaptureFlareOffset
		5,  // u4CaptureFlareThres
		64,  // u4VideoFlareOffset
		3,  // u4VideoFlareThres
		64,  // u4CustomFlareOffset
		3,  // u4CustomFlareThres
		64,  // u4StrobeFlareOffset
		3,  // u4StrobeFlareThres
		32,//150,  // u4PrvMaxFlareThres
		0,  // u4PrvMinFlareThres
		32,//150,  // u4VideoMaxFlareThres
		0,  // u4VideoMinFlareThres
		18,  // u4FlatnessThres
		75,  // u4FlatnessStrength
		// rMeteringSpec
		{
			// rHS_Spec
			{
				1,  // bEnableHistStretch
				1024,  // u4HistStretchWeight
				30,  // u4Pcent
				137,  // u4Thd
				68,  // u4FlatThd
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
				210,  // u4Thd
				0,  // bEnableCOEP
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
				0,  // bEnableBlackLight
				1024,  // u4BackLightWeight
				400,  // u4Pcent
				12,  // u4Thd
				255,  // u4CenterHighBnd
				256,  // u4TargetStrength
				// sFgBgEVRatio
				{
					2200,  // u4X1
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
				168,//175,  // u4Thd
				50,//60,  // u4FlatThd
				160,//200,//180,  // u4BrightTonePcent
				45,//50,//60,  // u4BrightToneThd
				500,  // u4LowBndPcent
				3,//5,  // u4LowBndThd
				25,//30,  // u4LowBndThdLimit
				50,  // u4FlatBrightPcent
				300,  // u4FlatDarkPcent
				// sFlatRatio
				{
					1200,  // u4X1
					1024,  // u4Y1
					2400,  // u4X2
					0  // u4Y2
				},
				// sBVRatio
				{
					-2000,//-1000,//-500,  // u4X1
					1024,  // u4Y1
					1500,//2000,  // u4X2
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
				40,  // uMeteringYLowBound
				50,  // uMeteringYHighBound
				50,//45,//40,  // uFaceYLowBound
				60,//55,//50,  // uFaceYHighBound
				3,  // uFaceCentralWeight
				120,  // u4MeteringStableMax
				80  // u4MeteringStableMin
			}
		},
		// rFlareSpec
		{
			
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
		
			
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
			
			96,  // u4FlareStdThrHigh
			48,  // u4FlareStdThrLow
			0,  // u4PrvCapFlareDiff
			4,  // u4FlareMaxStepGap_Fast
			0,  // u4FlareMaxStepGap_Slow
			1800,  // u4FlarMaxStepGapLimitBV
			2  // u4FlareAEStableCount
		},
		// rAEMovingRatio
		{
			110,  // u4SpeedUpRatio
			110,  // u4GlobalRatio
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

		{
         // Contrast:
         //  0   1   2   3   4   5   6   7   8   9  10
			3, 3, 3, 3, 3, 2, 1, 1, 1, 1, 1       // i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
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
