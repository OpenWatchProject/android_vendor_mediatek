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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov5670mipiraw.h"
#include "camera_info_ov5670mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"

const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    	}
    },
    ISPPca: {
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
            0x01FF0001, // MIX3_CTRL_0
            0x00FF0000, // MIX3_CTRL_1
            0xFFFF0000  // MIX3_SPARE
    }},
    ISPMulitCCM:{
            Poly22:{
        95075, // i4R_AVG
        1, // i4R_STD
        65782, // i4B_AVG
        1, // i4B_STD
        4095,      // i4R_MAX
        512,       // i4R_MIN
        4095,      // i4G_MAX
        512,       // i4G_MIN
        4095,      // i4B_MAX
        512,       // i4B_MIN
        { // i4P00[9]
            5516324, -2829410, -126914, -609281, 4057480, -888199, -3238, -1810090, 4373328
        },
        { // i4P10[9]
            0,  0, 0, 0,  0,  0, 0,   0, 0
        },
        { // i4P01[9]
            0,  0, 0, 0, 0,  0, 0,  0,  0
        },
        { // i4P20[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
        },
        { // i4P11[9]
            0,  0,  0,  0,   0, 0, 0,  0,  0
        },
        { // i4P02[9]
            0,    0,  0,  0,   0, 0,  0,    0,  0
        }
            },
            AWBGain:{
                // Strobe
		           {
                    512,	// i4R
                    512,	// i4G
                    512	// i4B
		           },
                // A
                {
                    575,	// i4R
                    512,	// i4G
                    1159	// i4B
		           },
                // TL84
		           {
                    702,	// i4R
                    512,	// i4G
                    935	// i4B
		           },
                // CWF
                {
                    810,	// i4R
                    512,	// i4G
                    997	// i4B
		           },
                // D65
		           {
                    971,	// i4R
                    512,	// i4G
                    649 // i4B
		           },
                // Reserved 1
		           {
                    512,	// i4R
                    512,	// i4G
                    512 // i4B
		           },
                // Reserved 2
		           {
                    512,	// i4R
                    512,	// i4G
                    512 // i4B
		           },
                // Reserved 3
		           {
                    512,	// i4R
                    512,	// i4G
                    512 // i4B
		           }
            },
            Weight:{
    1, // Strobe
    1, // A
    1, // TL84
    1, // CWF
    1, // D65
    1, // Reserved 1
    1, // Reserved 2
    1  // Reserved 3
}
    },
    //bInvokeSmoothCCM
    bInvokeSmoothCCM: MTRUE,
    DngMetadata:
    {
        0,  //i4RefereceIlluminant1
        3,  //i4RefereceIlluminant2
        {
            // rNoiseProfile[4]
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
        },
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1144,    // u4MinGain, 1024 base = 1x
            8192,    // u4MaxGain, 16x
            93,    // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            17,    // u4PreExpUnit
            30,    // u4PreMaxFrameRate
            17,    // u4VideoExpUnit
            30,    // u4VideoMaxFrameRate
            1024,    // u4Video2PreRatio, 1024 base = 1x
            17,    // u4CapExpUnit
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            16,     // u4Video1ExpUnit
            60,     // u4Video1MaxFrameRate
            1024,   // u4Video12PreRatio, 1024 base = 1x
            17,     // u4Video2ExpUnit
            30,     // u4Video2MaxFrameRate
            1024,   // u4Video22PreRatio, 1024 base = 1x
            17,     // u4Custom1ExpUnit
            30,     // u4Custom1MaxFrameRate
            1024,   // u4Custom12PreRatio, 1024 base = 1x
            17,     // u4Custom2ExpUnit
            30,     // u4Custom2MaxFrameRate
            1024,   // u4Custom22PreRatio, 1024 base = 1x
            17,     // u4Custom3ExpUnit
            30,     // u4Custom3MaxFrameRate
            1024,   // u4Custom32PreRatio, 1024 base = 1x
            17,     // u4Custom4ExpUnit
            30,     // u4Custom4MaxFrameRate
            1024,   // u4Custom42PreRatio, 1024 base = 1x
            17,     // u4Custom5ExpUnit
            30,     // u4Custom5MaxFrameRate
            1024,   // u4Custom52PreRatio, 1024 base = 1x
            22,    // u4LensFno, Fno = 2.8
            220    // u4FocusLength_100x
         },
         // rHistConfig
        {
            4, // 2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            TRUE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            TRUE,            // bEnableCaptureThres
            TRUE,            // bEnableVideoThres
            TRUE,            // bEnableVideo1Thres
            TRUE,            // bEnableVideo2Thres
            TRUE,            // bEnableCustom1Thres
            TRUE,            // bEnableCustom2Thres
            TRUE,            // bEnableCustom3Thres
            TRUE,            // bEnableCustom4Thres
            TRUE,            // bEnableCustom5Thres
            TRUE,            // bEnableStrobeThres
            47,                // u4AETarget
            47,                // u4StrobeAETarget

            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2, // 2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -6,               // i4BVOffset delta BV = -2.3
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            3,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            3,                 // u4VideoFlareThres
            64,                 // u4CustomFlareOffset
            3,                 // u4CustomFlareThres
            64,                 // u4StrobeFlareOffset
            3,                 // u4StrobeFlareThres
            160,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            160,                 // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75,                // u4FlatnessStrength
                        //rMeteringSpec
                        {
                                //rHS_Spec
                                {
                                        TRUE,//bEnableHistStretch           // enable histogram stretch
                                        1024,//u4HistStretchWeight          // Histogram weighting value
                                        40,//u4Pcent                      // 1%=10, 0~1000
                                        160,//u4Thd                        // 0~255
                                        75,//u4FlatThd                    // 0~255

                                        120,//u4FlatBrightPcent
                                        120,//u4FlatDarkPcent
                                        //sFlatRatio
                                        {
                                            1000, //i4X1
                                            1024,  //i4Y1
                                            2400, //i4X2
                                            0     //i4Y2
                                        },
                    TRUE, //bEnableGreyTextEnhance
                    1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                    {
                        10,     //i4X1
                        1024,   //i4Y1
                        80,     //i4X2
                        0       //i4Y2
                    }
                                },
                                //rAOE_Spec
                                {
                                        TRUE,//bEnableAntiOverExposure
                                        1024,//u4AntiOverExpWeight
                                        10,//u4Pcent
                                        200,//u4Thd
                                        TRUE,//bEnableCOEP
                                        1,//u4COEPcent
                                        106,//u4COEThd
                                        0,  // u4BVCompRatio
                                        //sCOEYRatio;     // the outer y ratio
                                        {
                                               23,   //i4X1
                                                1024,  //i4Y1
                                                47,   //i4X2
                                                0     //i4Y2
                                        },
                                       //sCOEDiffRatio;  // inner/outer y difference ratio
                                        {
                                                1500, //i4X1
                                                0,    //i4Y1
                                                2100, //i4X2
                                                1024   //i4Y2
                                        }
                                },
                                //rABL_Spec
                                {
                                        TRUE,//bEnableBlackLigh
                                        1024,//u4BackLightWeigh
                                        400,//u4Pcent
                                        22,//u4Thd,
                                        255, // center luminance
                                        256, // final target limitation, 256/128 = 2x
                                        //sFgBgEVRatio
                                        {
                                                2200, //i4X1
                                                0,    //i4Y1
                                                4000, //i4X2
                                                1024   //i4Y2
                                        },
                                        //sBVRatio
                                        {
                                                3800,//i4X1
                                                0,   //i4Y1
                                                5000,//i4X2
                                                1024  //i4Y2
                                        }
                                },
                                //rNS_Spec
                                {
                                        TRUE, // bEnableNightScene
                                        5,    //u4Pcent
                                        174,  //u4Thd
                                        72,   //u4FlatThd
                                        200,  //u4BrightTonePcent
                                        90,   //u4BrightToneThdMul;      // bright tone THD 1x=1024, m_u4AETarget*u4BrightToneThdMul/1024
                                        500,  //u4LowBndPcent
                                        5,    //u4LowBndThdMul, <1024, u4AETarget*u4LowBndThdMul/1024
                                        20,    //u4LowBndThdLimitMul, <1024, u4AETarget*u4LowBndThdLimitMul/1024

                                        50,  //u4FlatBrightPcent;
                                        300,   //u4FlatDarkPcent;
                                        //sFlatRatio
                                        {
                                                1200, //i4X1
                                                1024, //i4Y1
                                                2400,//i4X2
                                                0    //i4Y2
                                        },
                                        //sBVRatio
                                        {
                                                -500, //i4X1
                                                1024,  //i4Y1
                                                2000, //i4X2
                                                0     //i4Y2
                                        },
                                        TRUE, // bEnableNightSkySuppresion
                                        //sSkyBVRatio
                                        {
                                                -4000, //i4X1
                                                1024, //i4X2
                                                -2000,  //i4Y1
                                                0     //i4Y2
                                        }
                            },
                            // rTOUCHFD_Spec
                            {
                                40,
                                50,
                                40,
                                50,
                                3,
                                120,
                                80,
                            }
                        }, //End rMeteringSpec
                        // rFlareSpec
                        {
                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                            96,
                            48,
                            0,
                            4,
                            0,
                            1800,
                            0,
                        },
                        //rAEMoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            190, //u4Bright2TargetEnd
                            20,   //u4Dark2TargetStart
                            90, //u4B2TEnd
                            70,  //u4B2TStart
                            60,  //u4D2TEnd
                            90,  //u4D2TStart
                        },

                        //rAEVideoMoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAEVideo1MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAEVideo2MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAECustom1MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAECustom2MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAECustom3MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAECustom4MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },
                        //rAECustom5MoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            150,  //u4Bright2TargetEnd
                            20,    //u4Dark2TargetStart
                            90, //u4B2TEnd
                            40,  //u4B2TStart
                            40,  //u4D2TEnd
                            90,  //u4D2TStart
                        },

                        //rAEFaceMoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            190,  //u4Bright2TargetEnd
                            10,    //u4Dark2TargetStart
                            80, //u4B2TEnd
                            40,  //u4B2TStart
                            30,  //u4D2TEnd
                            60,  //u4D2TStart
                        },

                        //rAETrackingMoveRatio =
                        {
                            100, //u4SpeedUpRatio
                            100, //u4GlobalRatio
                            190,  //u4Bright2TargetEnd
                            10,    //u4Dark2TargetStart
                            80, //u4B2TEnd
                            40,  //u4B2TStart
                            30,  //u4D2TEnd
                            60,  //u4D2TStart
                        },
            //rAEAOENVRAMParam =
            {
                1,      // i4AOEStrengthIdx: 0 / 1 / 2
                130,    // u4BVCompRatio
                {
                {
                     47,  //u4Y_Target
                     10,  //u4AOE_OE_percent
                     160,  //u4AOE_OEBound
                     10,    //u4AOE_DarkBound
                     950,    //u4AOE_LowlightPrecent
                     1,    //u4AOE_LowlightBound
                     135,    //u4AOESceneLV_L
                     170,    //u4AOESceneLV_H
                     40,    //u4AOE_SWHdrLE_Bound
                },
                {
                     47,  //u4Y_Target
                     10,  //u4AOE_OE_percent
                     180,  //u4AOE_OEBound
                     15,    //u4AOE_DarkBound
                     950,    //u4AOE_LowlightPrecent
                     3,    //u4AOE_LowlightBound
                     135,    //u4AOESceneLV_L
                     170,    //u4AOESceneLV_H
                     40,    //u4AOE_SWHdrLE_Bound
                },
                {
                     47,  //u4Y_Target
                     10,  //u4AOE_OE_percent
                     200,  //u4AOE_OEBound
                     25,    //u4AOE_DarkBound
                     950,    //u4AOE_LowlightPrecent
                     8,    //u4AOE_LowlightBound
                     135,    //u4AOESceneLV_L
                     170,    //u4AOESceneLV_H
                     40,    //u4AOE_SWHdrLE_Bound
                }
              }
          }
        }
    },
    // AWB NVRAM
   {
        {
   {
	// AWB calibration data
	{
		// rUnitGain (unit gain: 1.0 = 512)
		{
			0,	// i4R
			0,	// i4G
			0	// i4B
		},
		// rGoldenGain (golden sample gain: 1.0 = 512)
		{
			0,	// i4R
			0,	// i4G
			0	// i4B
		},
		// rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
		{
			0,	// i4R
			0,	// i4G
			0	// i4B
		},
		// rD65Gain (D65 WB gain: 1.0 = 512)
		{
			971,	// i4R
			512,	// i4G
			649	// i4B
		}
	},
	// Original XY coordinate of AWB light source
	{
		// Strobe
		{
			0,	// i4X
			0	// i4Y
		},
		// Horizon
		{
		-368,	// i4X
		-328	// i4Y
		},
		// A
		{
			-259,	// i4X
			-345	    // i4Y
		},
		// TL84
		{
			-106,	// i4X
			-339	// i4Y
		},
		// CWF
		{
			-77,	// i4X
			-416	// i4Y
		},
		// DNP
		{
			19,	// i4X
			-349	// i4Y
		},
		// D65
		{
			149,	// i4X
			-324	// i4Y
		},
		// DF
		{
			0,    // i4X
			0	// i4Y
		}
	},
	// Rotated XY coordinate of AWB light source
	{
		// Strobe
		{
			0,     // i4X
			0	    // i4Y
		},
		// Horizon
		{
			-380,	// i4X
			-315	// i4Y
		},
		// A
		{
			-271,	// i4X
			-336	// i4Y
		},
		// TL84
		{
			-118,	// i4X
			-335	// i4Y
		},
		// CWF
		{
			-92,	// i4X
			-413	// i4Y
		},
		// DNP
		{
			7,	// i4X
			-350	// i4Y
		},
		// D65
		{
			138,	// i4X
			-329	// i4Y
		},
		// DF
		{
			122,	// i4X
			-403	// i4Y
		}
	},
	// AWB gain of AWB light source
	{
		// Strobe
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
		},
		// Horizon
		{
			512,	// i4R
			540,	// i4G
			1387	// i4B
		},
		// A
		{
			575,	// i4R
			512,	// i4G
			1159	// i4B
		},
		// TL84
		{
			702,	// i4R
			512,	// i4G
			935	// i4B
		},
		// CWF
		{
			810,	// i4R
			512,	// i4G
			997	// i4B
		},
		// DNP
		{
			843,	// i4R
			512,	// i4G
			800	// i4B
		},
		// D65
		{
			971,	// i4R
			512,	// i4G
			649	// i4B
		},
		// DF
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
		}
	},
	// Rotation matrix parameter
	{
		2,	// i4RotationAngle
		256,	// i4Cos
		9	// i4Sin
	},
	// Daylight locus parameter
	{
		-135,	// i4SlopeNumerator
		128	// i4SlopeDenominator
	},
 // Predictor gain
 {
                        // i4PrefRatio100
                        151,

                        // DaylightLocus_L
                        {
                            971,    // i4R
                            512,    // i4G
                            649     // i4B
                        },
                        // DaylightLocus_H
                        {
                            718,    // i4R
                            512,    // i4G
                            896     // i4B
                        },
                        // Temporal General
                        {
                            971,    // i4R
                            512,    // i4G
                            649     // i4B
                        },
                        // AWB LSC Gain
                    {
                        718,    // i4R
                            512,    // i4G
                            896     // i4B
                    }
 },
	// AWB light area
	{
		// Strobe
		{
			0,	// StrobeRightBound
			0,	// StrobeLeftBound
			0,	// StrobeUpperBound
			0	// StrobeLowerBound
		},
		// Tungsten
		{
			-179,	// TungRightBound
			-780,	// TungLeftBound
			-280,	// TungUpperBound
			-336	// TungLowerBound
		},
		// Warm fluorescent
		{
			-179,	// WFluoRightBound
			-780,	// WFluoLeftBound
			-336,	// WFluoUpperBound
			-443	// WFluoLowerBound
		},
		// Fluorescent
		{
			-24,	// FluoRightBound
			-179,	// FluoLeftBound
			-289,	// FluoUpperBound
			-384	// FluoLowerBound
		},
		// CWF
		{
			-6,	// CWFRightBound
			-179,	// CWFLeftBound
			-384,	// CWFUpperBound
			-458	// CWFLowerBound
		},
		// Daylight
		{
			168,	// DayRightBound
			-24,	// DayLeftBound
			-289,	// DayUpperBound
			-384	// DayLowerBound
		},
		// Shade
		{
			498,	// ShadeRightBound
			168,	// ShadeLeftBound
			-289,	// ShadeUpperBound
			-363	// ShadeLowerBound
		},
		// Daylight Fluorescent
		{
			168,	// DFRightBound
			-6,	// DFLeftBound
			-384,	// DFUpperBound
			-458	// DFLowerBound
		}
	},
	// PWB light area
	{
		// Reference area
		{
			498,	// PRefRightBound
			-780,	// PRefLeftBound
			-255,	// PRefUpperBound
			-458	// PRefLowerBound
		},
		// Daylight
		{
			193,	// PDayRightBound
			-24,	// PDayLeftBound
			-289,	// PDayUpperBound
			-384	// PDayLowerBound
		},
		// Cloudy daylight
		{
			293,	// PCloudyRightBound
			118,	// PCloudyLeftBound
			-289,	// PCloudyUpperBound
			-384	// PCloudyLowerBound
		},
		// Shade
		{
			393,	// PShadeRightBound
			118,	// PShadeLeftBound
			-289,	// PShadeUpperBound
			-384	// PShadeLowerBound
		},
		// Twilight
		{
			-24,	// PTwiRightBound
			-184,	// PTwiLeftBound
			-289,	// PTwiUpperBound
			-384	// PTwiLowerBound
		},
		// Fluorescent
		{
			188,	// PFluoRightBound
			-218,	// PFluoLeftBound
			-279,	// PFluoUpperBound
			-463	// PFluoLowerBound
		},
		// Warm fluorescent
		{
			-171,	// PWFluoRightBound
			-371,	// PWFluoLeftBound
			-279,	// PWFluoUpperBound
			-463	// PWFluoLowerBound
		},
		// Incandescent
		{
			-171,	// PIncaRightBound
			-371,	// PIncaLeftBound
			-289,	// PIncaUpperBound
			-384	// PIncaLowerBound
		},
		// Gray World
		{
			5000,	// PGWRightBound
			-5000,	// PGWLeftBound
			5000,	// PGWUpperBound
			-5000	// PGWLowerBound
		}
	},
	// PWB default gain
	{
		// Daylight
		{
			916,	// PWB_Day_R
			512,	// PWB_Day_G
			706	// PWB_Day_B
		},
		// Cloudy daylight
		{
			1072,	// PWB_Cloudy_R
			512,	// PWB_Cloudy_G
			596	// PWB_Cloudy_B
		},
		// Shade
		{
			1144,	// PWB_Shade_R
			512,	// PWB_Shade_G
			555	// PWB_Shade_B
		},
		// Twilight
		{
			716,	// PWB_Twi_R
			512,	// PWB_Twi_G
			919	// PWB_Twi_B
		},
		// Fluorescent
		{
			844,	// PWB_Fluo_R
			512,	// PWB_Fluo_G
			848	// PWB_Fluo_B
		},
		// Warm fluorescent
		{
			604,	// PWB_WFluo_R
			512,	// PWB_WFluo_G
			1214	// PWB_WFluo_B
		},
		// Incandescent
		{
			576,	// PWB_Inca_R
			512,	// PWB_Inca_G
			1161	// PWB_Inca_B
		},
		// Gray World
		{
			512,	// PWB_GW_R
			512,	// PWB_GW_G
			512	// PWB_GW_B
		}
	},
	// AWB preference color
	{
		// Tungsten
		{
			60,	// TUNG_SLIDER
			4120	// TUNG_OFFS
		},
		// Warm fluorescent
		{
			50,	// WFluo_SLIDER
			4320	// WFluo_OFFS
		},
		// Shade
		{
			50,	// Shade_SLIDER
			909	// Shade_OFFS
		},
		// Sunset Area
                    {
                        38, //64,   // i4Sunset_BoundXr_Thr
                        -350 //-463     // i4Sunset_BoundYr_Thr
                    },
                    // Sunset Vertex
                    {
                        38, //64,   // i4Sunset_BoundXr_Thr
                        -350 //-463     // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {

                        -87, //-135, //-82,   // i4BoundXrThr
                        -329 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade F Vertex
                    {

                        -87, //-135, //-82,   // i4BoundXrThr
                        -329 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade CWF Area
                    {
                        -92,//-109, //-89,   // i4BoundXrThr
                        -413//-475 //-509    // i4BoundYrThr
                    },
                    // Shade CWF Vertex
                    {
                        -92,//-109, //-89,   // i4BoundXrThr
                        -413//-475 //-509    // i4BoundYrThr
                    },
                    // Low CCT Area
                    {
                        -400, //-432, //-450,       // i4BoundXrThr
                        142 //450         // i4BoundYrThr
                    },
                    // Low CCT Vertex
                    {
                        -400, //-432, //-450,       // i4BoundXrThr
                        142 //450         // i4BoundYrThr
                    }
	},
	// CCT estimation
	{
		// CCT
		{
			2300,	// CCT0
			2850,	// CCT1
			3750,	// CCT2
			5100,	// CCT3
			6500     // CCT4
		},
		// Rotated X coordinate
		{
			-518,	// RotatedXCoordinate0
			-409,	// RotatedXCoordinate1
			-256,	// RotatedXCoordinate2
			-131,	// RotatedXCoordinate3
			0  // RotatedXCoordinate4
		}
	}
            },

            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

                // Daylight locus offset LUTs for tungsten
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                    //{0, 500, 1000, 1333, 1667, 2000, 2333, 2667, 3000, 3333, 3667, 4000, 4333, 4667, 5000, 5333, 5667, 6000, 6333, 6667,  7000} // i4LUTOut
                    //{0, 500, 1000, 1500, 2000, 2313, 2625, 2938, 3250, 3563, 3875, 4188, 4500, 4813, 5125, 5438, 5750, 6063, 6375, 6688,  7000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  700,  850, 1200, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2500, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8000, 8000, 8500, 9000, 9500, 10000}  // i4LUTOut
                },
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
          	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // STROBE
        	        {
          	            {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {520, 520, 496}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
           	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // TUNGSTEN
        	        {
                        {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508},
           	            {512, 512, 508}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // WARM F
        	        {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    115, //100,    // i4InitLVThr_L
                    155, //140,    // i4InitLVThr_H
                    100 //80      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                        65,     // i4Neutral_ParentBlk_Thr
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    // Non neutral
	                { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Flurescent
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // CWF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Daylight
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
	                // DF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                },
                // AWB feature detection
                {
                    // Sunset Prop
                    {
                        1,          // i4Enable
                        120,        // i4LVThr_L
                        130,        // i4LVThr_H
                        10,         // i4SunsetCountThr
                        0,          // i4SunsetCountRatio_L
                        171         // i4SunsetCountRatio_H
                    },

                    // Shade F Detection
                    {
                        1,          // i4Enable
                        90,        // i4LVThr
                        105,        // i4LVThr
                        96 //10         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        90,        // i4LVThr
                        105,        // i4LVThr
                        208 //224 //128 //35         // i4DaylightProb
                    },

                    // Low CCT
                    {
                        0,          // i4Enable
                        256        // i4SpeedRatio
                    }

                },

                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {  90, 90,  90,  90,  90,  90,  90,  90,  90,  90,   90,  50,  30,  20,  10,   0,   0,   0,   0}
                },

                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    {100, 100, 100, 100, 100, 100, 100, 100, 100,  95,   95,   70,  50,  25,  25,  25,  0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                }
            }
        },
 {
   {
	// AWB calibration data
	{
		// rUnitGain (unit gain: 1.0 = 512)
		{
			0,	// i4R
			0,	// i4G
			0	// i4B
		},
		// rGoldenGain (golden sample gain: 1.0 = 512)
		{
			0,	// i4R
			0,	// i4G
			0	// i4B
		},
		// rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
		{
			0,	// i4R
			0,	// i4G
			0	// i4B
		},
		// rD65Gain (D65 WB gain: 1.0 = 512)
		{
			971,	// i4R
			512,	// i4G
			649	// i4B
		}
	},
	// Original XY coordinate of AWB light source
	{
		// Strobe
		{
			0,	// i4X
			0	// i4Y
		},
		// Horizon
		{
		-368,	// i4X
		-328	// i4Y
		},
		// A
		{
			-259,	// i4X
			-345	    // i4Y
		},
		// TL84
		{
			-106,	// i4X
			-339	// i4Y
		},
		// CWF
		{
			-77,	// i4X
			-416	// i4Y
		},
		// DNP
		{
			19,	// i4X
			-349	// i4Y
		},
		// D65
		{
			149,	// i4X
			-324	// i4Y
		},
		// DF
		{
			0,    // i4X
			0	// i4Y
		}
	},
	// Rotated XY coordinate of AWB light source
	{
		// Strobe
		{
			0,     // i4X
			0	    // i4Y
		},
		// Horizon
		{
			-380,	// i4X
			-315	// i4Y
		},
		// A
		{
			-271,	// i4X
			-336	// i4Y
		},
		// TL84
		{
			-118,	// i4X
			-335	// i4Y
		},
		// CWF
		{
			-92,	// i4X
			-413	// i4Y
		},
		// DNP
		{
			7,	// i4X
			-350	// i4Y
		},
		// D65
		{
			138,	// i4X
			-329	// i4Y
		},
		// DF
		{
			122,	// i4X
			-403	// i4Y
		}
	},
	// AWB gain of AWB light source
	{
		// Strobe
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
		},
		// Horizon
		{
			512,	// i4R
			540,	// i4G
			1387	// i4B
		},
		// A
		{
			575,	// i4R
			512,	// i4G
			1159	// i4B
		},
		// TL84
		{
			702,	// i4R
			512,	// i4G
			935	// i4B
		},
		// CWF
		{
			810,	// i4R
			512,	// i4G
			997	// i4B
		},
		// DNP
		{
			843,	// i4R
			512,	// i4G
			800	// i4B
		},
		// D65
		{
			971,	// i4R
			512,	// i4G
			649	// i4B
		},
		// DF
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
		}
	},
	// Rotation matrix parameter
	{
		2,	// i4RotationAngle
		256,	// i4Cos
		9	// i4Sin
	},
	// Daylight locus parameter
	{
		-135,	// i4SlopeNumerator
		128	// i4SlopeDenominator
	},
 // Predictor gain
 {
                        // i4PrefRatio100
                        151,

                        // DaylightLocus_L
                        {
                            971,    // i4R
                            512,    // i4G
                            649     // i4B
                        },
                        // DaylightLocus_H
                        {
                            718,    // i4R
                            512,    // i4G
                            896     // i4B
                        },
                        // Temporal General
                        {
                            971,    // i4R
                            512,    // i4G
                            649     // i4B
                        },
                        // AWB LSC Gain
                    {
                        718,    // i4R
                            512,    // i4G
                            896     // i4B
                    }
 },
	// AWB light area
	{
		// Strobe
		{
			0,	// StrobeRightBound
			0,	// StrobeLeftBound
			0,	// StrobeUpperBound
			0	// StrobeLowerBound
		},
		// Tungsten
		{
			-179,	// TungRightBound
			-780,	// TungLeftBound
			-280,	// TungUpperBound
			-336	// TungLowerBound
		},
		// Warm fluorescent
		{
			-179,	// WFluoRightBound
			-780,	// WFluoLeftBound
			-336,	// WFluoUpperBound
			-443	// WFluoLowerBound
		},
		// Fluorescent
		{
			-24,	// FluoRightBound
			-179,	// FluoLeftBound
			-289,	// FluoUpperBound
			-384	// FluoLowerBound
		},
		// CWF
		{
			-6,	// CWFRightBound
			-179,	// CWFLeftBound
			-384,	// CWFUpperBound
			-458	// CWFLowerBound
		},
		// Daylight
		{
			168,	// DayRightBound
			-24,	// DayLeftBound
			-289,	// DayUpperBound
			-384	// DayLowerBound
		},
		// Shade
		{
			498,	// ShadeRightBound
			168,	// ShadeLeftBound
			-289,	// ShadeUpperBound
			-363	// ShadeLowerBound
		},
		// Daylight Fluorescent
		{
			168,	// DFRightBound
			-6,	// DFLeftBound
			-384,	// DFUpperBound
			-458	// DFLowerBound
		}
	},
	// PWB light area
	{
		// Reference area
		{
			498,	// PRefRightBound
			-780,	// PRefLeftBound
			-255,	// PRefUpperBound
			-458	// PRefLowerBound
		},
		// Daylight
		{
			193,	// PDayRightBound
			-24,	// PDayLeftBound
			-289,	// PDayUpperBound
			-384	// PDayLowerBound
		},
		// Cloudy daylight
		{
			293,	// PCloudyRightBound
			118,	// PCloudyLeftBound
			-289,	// PCloudyUpperBound
			-384	// PCloudyLowerBound
		},
		// Shade
		{
			393,	// PShadeRightBound
			118,	// PShadeLeftBound
			-289,	// PShadeUpperBound
			-384	// PShadeLowerBound
		},
		// Twilight
		{
			-24,	// PTwiRightBound
			-184,	// PTwiLeftBound
			-289,	// PTwiUpperBound
			-384	// PTwiLowerBound
		},
		// Fluorescent
		{
			188,	// PFluoRightBound
			-218,	// PFluoLeftBound
			-279,	// PFluoUpperBound
			-463	// PFluoLowerBound
		},
		// Warm fluorescent
		{
			-171,	// PWFluoRightBound
			-371,	// PWFluoLeftBound
			-279,	// PWFluoUpperBound
			-463	// PWFluoLowerBound
		},
		// Incandescent
		{
			-171,	// PIncaRightBound
			-371,	// PIncaLeftBound
			-289,	// PIncaUpperBound
			-384	// PIncaLowerBound
		},
		// Gray World
		{
			5000,	// PGWRightBound
			-5000,	// PGWLeftBound
			5000,	// PGWUpperBound
			-5000	// PGWLowerBound
		}
	},
	// PWB default gain
	{
		// Daylight
		{
			916,	// PWB_Day_R
			512,	// PWB_Day_G
			706	// PWB_Day_B
		},
		// Cloudy daylight
		{
			1072,	// PWB_Cloudy_R
			512,	// PWB_Cloudy_G
			596	// PWB_Cloudy_B
		},
		// Shade
		{
			1144,	// PWB_Shade_R
			512,	// PWB_Shade_G
			555	// PWB_Shade_B
		},
		// Twilight
		{
			716,	// PWB_Twi_R
			512,	// PWB_Twi_G
			919	// PWB_Twi_B
		},
		// Fluorescent
		{
			844,	// PWB_Fluo_R
			512,	// PWB_Fluo_G
			848	// PWB_Fluo_B
		},
		// Warm fluorescent
		{
			604,	// PWB_WFluo_R
			512,	// PWB_WFluo_G
			1214	// PWB_WFluo_B
		},
		// Incandescent
		{
			576,	// PWB_Inca_R
			512,	// PWB_Inca_G
			1161	// PWB_Inca_B
		},
		// Gray World
		{
			512,	// PWB_GW_R
			512,	// PWB_GW_G
			512	// PWB_GW_B
		}
	},
	// AWB preference color
	{
		// Tungsten
		{
			60,	// TUNG_SLIDER
			4120	// TUNG_OFFS
		},
		// Warm fluorescent
		{
			50,	// WFluo_SLIDER
			4320	// WFluo_OFFS
		},
		// Shade
		{
			50,	// Shade_SLIDER
			909	// Shade_OFFS
		},
		// Sunset Area
                    {
                        38, //64,   // i4Sunset_BoundXr_Thr
                        -350 //-463     // i4Sunset_BoundYr_Thr
                    },
                    // Sunset Vertex
                    {
                        38, //64,   // i4Sunset_BoundXr_Thr
                        -350 //-463     // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {

                        -87, //-135, //-82,   // i4BoundXrThr
                        -329 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade F Vertex
                    {

                        -87, //-135, //-82,   // i4BoundXrThr
                        -329 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade CWF Area
                    {
                        -92,//-109, //-89,   // i4BoundXrThr
                        -413//-475 //-509    // i4BoundYrThr
                    },
                    // Shade CWF Vertex
                    {
                        -92,//-109, //-89,   // i4BoundXrThr
                        -413//-475 //-509    // i4BoundYrThr
                    },
                    // Low CCT Area
                    {
                        -400, //-432, //-450,       // i4BoundXrThr
                        142 //450         // i4BoundYrThr
                    },
                    // Low CCT Vertex
                    {
                        -400, //-432, //-450,       // i4BoundXrThr
                        142 //450         // i4BoundYrThr
                    }
	},
	// CCT estimation
	{
		// CCT
		{
			2300,	// CCT0
			2850,	// CCT1
			3750,	// CCT2
			5100,	// CCT3
			6500     // CCT4
		},
		// Rotated X coordinate
		{
			-518,	// RotatedXCoordinate0
			-409,	// RotatedXCoordinate1
			-256,	// RotatedXCoordinate2
			-131,	// RotatedXCoordinate3
			0  // RotatedXCoordinate4
		}
	}
            },

            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

                // Daylight locus offset LUTs for tungsten
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                    //{0, 500, 1000, 1333, 1667, 2000, 2333, 2667, 3000, 3333, 3667, 4000, 4333, 4667, 5000, 5333, 5667, 6000, 6333, 6667,  7000} // i4LUTOut
                    //{0, 500, 1000, 1500, 2000, 2313, 2625, 2938, 3250, 3563, 3875, 4188, 4500, 4813, 5125, 5438, 5750, 6063, 6375, 6688,  7000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  700,  850, 1200, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2500, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8000, 8000, 8500, 9000, 9500, 10000}  // i4LUTOut
                },
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
          	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // STROBE
        	        {
          	            {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {520, 520, 496}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
           	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // TUNGSTEN
        	        {
                        {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508},
           	            {512, 512, 508}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // WARM F
        	        {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    115, //100,    // i4InitLVThr_L
                    155, //140,    // i4InitLVThr_H
                    100 //80      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                        65,     // i4Neutral_ParentBlk_Thr
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    // Non neutral
	                { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Flurescent
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // CWF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Daylight
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
	                // DF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                },
                // AWB feature detection
                {
                    // Sunset Prop
                    {
                        1,          // i4Enable
                        120,        // i4LVThr_L
                        130,        // i4LVThr_H
                        10,         // i4SunsetCountThr
                        0,          // i4SunsetCountRatio_L
                        171         // i4SunsetCountRatio_H
                    },

                    // Shade F Detection
                    {
                        1,          // i4Enable
                        90,        // i4LVThr
                        105,        // i4LVThr
                        96 //10         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        90,        // i4LVThr
                        105,        // i4LVThr
                        208 //224 //128 //35         // i4DaylightProb
                    },

                    // Low CCT
                    {
                        0,          // i4Enable
                        256        // i4SpeedRatio
                    }

                },

                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {  90, 90,  90,  90,  90,  90,  90,  90,  90,  90,   90,  50,  30,  20,  10,   0,   0,   0,   0}
                },

                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    {100, 100, 100, 100, 100, 100, 100, 100, 100,  95,   95,   70,  50,  25,  25,  25,  0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                }
            }
        }
},
        // Flash AWB NVRAM
        {
            #include INCLUDE_FILENAME_FLASH_AWB_PARA
        },

	{0}  //FIXED
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace
const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
        {
                0,  // isTsfEn
                2,  // tsfCtIdx
                {20, 2000, -110, -110, 512, 512, 512, 0}    // rAWBInput[8]
        },
    #include INCLUDE_FILENAME_TSF_PARA
    #include INCLUDE_FILENAME_TSF_DATA
};
const NVRAM_CAMERA_FEATURE_STRUCT CAMERA_FEATURE_DEFAULT_VALUE =
{
#include INCLUDE_FILENAME_FEATURE_PARA
};

typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T),
                                             0,
                                             sizeof(CAMERA_TSF_TBL_STRUCT),
                                             0,
                                             sizeof(NVRAM_CAMERA_FEATURE_STRUCT)
                                            };

    if (CameraDataType > CAMERA_NVRAM_DATA_FEATURE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_FEATURE:
            memcpy(pDataBuf,&CAMERA_FEATURE_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
            break;
        default:
            break;
    }
    return 0;
}};  //  NSFeature
