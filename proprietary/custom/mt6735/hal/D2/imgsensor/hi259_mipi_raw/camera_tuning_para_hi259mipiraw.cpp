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
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_hi259mipiraw.h"
#include "camera_info_hi259mipiraw.h"
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
      0x0, // MIX3_CTRL_0
      0x0, // MIX3_CTRL_1
      0x0  // MIX3_SPARE
    }},
    ISPMulitCCM:{
      Poly22:{
        57975,      // i4R_AVG
        11900,      // i4R_STD
        69525,      // i4B_AVG
        14086,      // i4B_STD
        0,      // i4R_MAX
        0,      // i4R_MIN
        0,      // i4G_MAX
        0,      // i4G_MIN
        0,      // i4B_MAX
        0,      // i4B_MIN
        {  // i4P00[9]
            5160000, -2470000, -130000, -1115000, 4427500, -752500, -340000, -2670000, 5577500
        },
        {  // i4P10[9]
            2607402, -3047921, 440519, -246712, -157949, 404661, 63905, 713701, -798409
        },
        {  // i4P01[9]
            1664875, -1813987, 149113, -362100, -305156, 667256, -78215, 147298, -82271
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
                }

      },
      AWBGain:{
        // Strobe
        {
          512,    // i4R
          512,    // i4G
          512    // i4B
        },
        // A
        {
          453,    // i4R
          512,    // i4G
          823    // i4B
        },
        // TL84
        {
          537,    // i4R
          512,    // i4G
          716    // i4B
        },
        // CWF
        {
          593,    // i4R
          512,    // i4G
          747    // i4B
        },
        // D65
        {
          736,    // i4R
          512,    // i4G
          495    // i4B
        },
        // Reserved 1
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        },
        // Reserved 2
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        },
        // Reserved 3
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
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
    bInvokeSmoothCCM: MTRUE
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1280,    // u4MinGain, 1024 base = 1x
            8192,    // u4MaxGain, 16x
            100,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            25001,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            25001,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            25001,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            25001,    // u4Video1ExpUnit
            120,    // u4Video1MaxFrameRate
            1024,    // u4Video12PreRatio, 1024 base = 1x
            25001,    // u4Video2ExpUnit
            30,    // u4Video2MaxFrameRate
            1024,    // u4Video22PreRatio, 1024 base = 1x
            25001,    // u4Custom1ExpUnit
            30,    // u4Custom1MaxFrameRate
            1024,    // u4Custom12PreRatio, 1024 base = 1x
            25001,    // u4Custom2ExpUnit
            30,    // u4Custom2MaxFrameRate
            1024,    // u4Custom22PreRatio, 1024 base = 1x
            25001,    // u4Custom3ExpUnit
            30,    // u4Custom3MaxFrameRate
            1024,    // u4Custom32PreRatio, 1024 base = 1x
            25001,    // u4Custom4ExpUnit
            30,    // u4Custom4MaxFrameRate
            1024,    // u4Custom42PreRatio, 1024 base = 1x
            25001,    // u4Custom5ExpUnit
            30,    // u4Custom5MaxFrameRate
            1024,    // u4Custom52PreRatio, 1024 base = 1x
            28,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
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
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            TRUE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            TRUE,    // bEnableCaptureThres
            TRUE,    // bEnableVideoThres
            TRUE,    // bEnableVideo1Thres
            TRUE,    // bEnableVideo2Thres
            TRUE,    // bEnableCustom1Thres
            TRUE,    // bEnableCustom2Thres
            TRUE,    // bEnableCustom3Thres
            TRUE,    // bEnableCustom4Thres
            TRUE,    // bEnableCustom5Thres
            TRUE,    // bEnableStrobeThres
            44,    // u4AETarget
            47,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -7,    // i4BVOffset delta BV = value/10 
            58,    // u4PreviewFlareOffset
            58,    // u4CaptureFlareOffset
            1,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            3,    // u4VideoFlareThres
            64,    // u4CustomFlareOffset
            3,    // u4CustomFlareThres
            96,    // u4StrobeFlareOffset
            3,    // u4StrobeFlareThres
            64,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            160,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75,    // u4FlatnessStrength
            //rMeteringSpec
            {
                //rHS_Spec
                {
                    TRUE,//bEnableHistStretch           // enable histogram stretch
                    1024,//u4HistStretchWeight          // Histogram weighting value
                    40,//u4Pcent                      // 
                    170,//u4Thd                        // 0~255
                    72, //74,//u4FlatThd                    // 0~255
                    120,//u4FlatBrightPcent
                    120,//u4FlatDarkPcent
                    //sFlatRatio
                    {
                        1000,  //i4X1
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
                    TRUE, //bEnableAntiOverExposure
                    1024, //u4AntiOverExpWeight
                    10, //u4Pcent
                    210, //u4Thd     //0401 modify
                    TRUE, //bEnableCOEP
                    1,    //u4COEPcent
                    106,  //u4COEThd
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
                    5,//u4Pcent
                    225,//u4Thd         //0406 modify for dark 
                    72,//u4FlatThd
                    205,//u4BrightTonePcent
                    125,//u4BrightToneThd
                    500,//u4LowBndPcent
                    5,//u4LowBndThd
                    26,//u4LowBndThdLimit
                    50,//u4FlatBrightPcent
                    300,//u4FlatDarkPcent
                    //sFlatRatio
                    {
                        1200, //i4X1
                        1024, //i4Y1
                        2400, //i4X2
                        0    //i4Y2
                    },
                    //sBVRatio
                    {
                        -500, //i4X1
                        1024,  //i4Y1
                        3000, //i4X2
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
                    50, //uMeteringYLowBound;
                    60, //uMeteringYHighBound;
                    40, //uFaceYLowBound;
                    50, //uFaceYHighBound;
                    3,  //uFaceCentralWeight;
                    120,//u4MeteringStableMax;
                    80, //u4MeteringStableMin;
                }
            }, //End rMeteringSpec
            // rFlareSpec
            {
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                96,                                               //u4FlareStdThrHigh;
                48,                                               //u4FlareStdThrLow;
                0,                                                //u4PrvCapFlareDiff;
                4,                                                //u4FlareMaxStepGap_Fast;
                0,                                                //u4FlareMaxStepGap_Slow;
                1800,                                             //u4FlarMaxStepGapLimitBV;
                0,                                                //u4FlareAEStableCount;
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
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
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
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom4MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom5MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                10,  //u4B2TStart
                10,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEFaceMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                80, //u4B2TEnd
                30,  //u4B2TStart
                20,  //u4D2TEnd
                60,  //u4D2TStart
            },

            //rAETrackingMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                80, //u4B2TEnd
                30,  //u4B2TStart
                20,  //u4D2TEnd
                60,  //u4D2TStart
            },
            //rAEAOENVRAMParam =
            {
                1,      // i4AOEStrengthIdx: 0 / 1 / 2
                128,    // u4BVCompRatio
                {
                    {
                        47,  //u4Y_Target
                        10,  //u4AOE_OE_percent
                        160,  //u4AOE_OEBound
                        15,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        5,    //u4AOE_LowlightBound
                        100,    //u4AOESceneLV_L
                        150,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                        10,  //u4AOE_OE_percent
                        180,  //u4AOE_OEBound
                        20, //20,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        10, //10,    //u4AOE_LowlightBound
                        100,    //u4AOESceneLV_L
                        150,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                        10,  //u4AOE_OE_percent
                        200,  //u4AOE_OEBound
                        25,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        15,    //u4AOE_LowlightBound
                        100,    //u4AOESceneLV_L
                        150,    //u4AOESceneLV_H
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
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // rGoldenGain (golden sample gain: 1.0 = 512)
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // rD65Gain (D65 WB gain: 1.0 = 512)
                {
                    749,    // D65Gain_R
                    523,    // D65Gain_G
                    512    // D65Gain_B
                }
            },
            // Original XY coordinate of AWB light source
            {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -322,    // OriX_Hor
                    -110    // OriY_Hor
                },
                // A
                {
                    -226,    // OriX_A
                    -129    // OriY_A
                },
                // TL84
                {
                    -108,    // OriX_TL84
                    -146    // OriY_TL84
                },
                // CWF
                {
                    -85,    // OriX_CWF
                    -190    // OriY_CWF
                },
                // DNP
                {
                    -23,    // OriX_DNP
                    -156    // OriY_DNP
                },
                // D65
                {
                    140,    // OriX_D65
                    -125    // OriY_D65
                },
                // DF
                {
                    0,    // OriX_DF
                    0    // OriY_DF
                }
            },
            // Rotated XY coordinate of AWB light source
            {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -322,    // RotX_Hor
                    -110    // RotY_Hor
                },
                // A
                {
                    -226,    // RotX_A
                    -129    // RotY_A
                },
                // TL84
                {
                    -108,    // RotX_TL84
                    -146    // RotY_TL84
                },
                // CWF
                {
                    -85,    // RotX_CWF
                    -190    // RotY_CWF
                },
                // DNP
                {
                    -23,    // RotX_DNP
                    -156    // RotY_DNP
                },
                // D65
                {
                    140,    // RotX_D65
                    -125    // RotY_D65
                },
                // DF
                {
                    117,    // RotX_DF
                    -182    // RotY_DF
                }
            },
            // AWB gain of AWB light source
            {
                // Strobe 
                {
                    512,    // i4R
                    512,    // i4G
                    512    // i4B
                },
                // Horizon 
                {
                    512,    // AWBGAIN_HOR_R
                    682,    // AWBGAIN_HOR_G
                    1223    // AWBGAIN_HOR_B
                },
                // A 
                {
                    512,    // AWBGAIN_A_R
                    584,    // AWBGAIN_A_G
                    943    // AWBGAIN_A_B
                },
                // TL84 
                {
                    540,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    722    // AWBGAIN_TL84_B
                },
                // CWF 
                {
                    590,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    743    // AWBGAIN_CWF_B
                },
                // DNP 
                {
                    613,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    652    // AWBGAIN_DNP_B
                },
                // D65 
                {
                    749,    // AWBGAIN_D65_R
                    523,    // AWBGAIN_D65_G
                    512    // AWBGAIN_D65_B
                },
                // DF 
                {
                    512,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    512    // AWBGAIN_DF_B
                }
            },
            // Rotation matrix parameter
            {
                0,    // RotationAngle
                256,    // Cos
                0    // Sin
            },
            // Daylight locus parameter
            {
                -128,    // i4SlopeNumerator
                128    // i4SlopeDenominator
            },
            // Predictor gain
            {
                101, // i4PrefRatio100
                // DaylightLocus_L
                {
                    719,    // i4R
                    530,    // i4G
                    532    // i4B
                },
                // DaylightLocus_H
                {
                    570,    // i4R
                    512,    // i4G
                    646    // i4B
                },
                // Temporal General
                {
                    749,    // i4R
                    523,    // i4G
                    512    // i4B
                },
                // AWB LSC Gain
                {
                    570,        // i4R
                    512,        // i4G
                    646        // i4B
                }
            },
            // AWB light area
            {
                // Strobe:FIXME
                {
                    0,    // i4RightBound
                    0,    // i4LeftBound
                    0,    // i4UpperBound
                    0    // i4LowerBound
                },
                // Tungsten
                {
                    -165,    // TungRightBound
                    -722,    // TungLeftBound
                    -45,    // TungUpperBound
                    -138    // TungLowerBound
                },
                // Warm fluorescent
                {
                    -165,    // WFluoRightBound
                    -722,    // WFluoLeftBound
                    -138,    // WFluoUpperBound
                    -330    // WFluoLowerBound
                },
                // Fluorescent
                {
                    -51,    // FluoRightBound
                    -165,    // FluoLeftBound
                    -55,    // FluoUpperBound
                    -168    // FluoLowerBound
                },
                // CWF
                {
                4,    // CWFRightBound
                -165,    // CWFLeftBound
                -168,    // CWFUpperBound
                -310    // CWFLowerBound
                },
                // Daylight
                {
                    170,    // DayRightBound
                    -51,    // DayLeftBound
                    -55,    // DayUpperBound
                    -168    // DayLowerBound
                },
                // Shade
                {
                    500,    // ShadeRightBound
                    170,    // ShadeLeftBound
                    -55,    // ShadeUpperBound
                    -200    // ShadeLowerBound
                },
                // Daylight Fluorescent
                {
                    170,    // DFRightBound
                    4,    // DFLeftBound
                    -168,    // DFUpperBound
                    -310    // DFLowerBound
                }
            },
            // PWB light area
            {
                // Reference area
                {
                    500,    // PRefRightBound
                    -722,    // PRefLeftBound
                    0,    // PRefUpperBound
                    -330    // PRefLowerBound
                },
                // Daylight
                {
                    195,    // PDayRightBound
                    -51,    // PDayLeftBound
                    -55,    // PDayUpperBound
                    -168    // PDayLowerBound
                },
                // Cloudy daylight
                {
                    295,    // PCloudyRightBound
                    120,    // PCloudyLeftBound
                    -55,    // PCloudyUpperBound
                    -168    // PCloudyLowerBound
                },
                // Shade
                {
                    395,    // PShadeRightBound
                    120,    // PShadeLeftBound
                    -55,    // PShadeUpperBound
                    -168    // PShadeLowerBound
                },
                // Twilight
                {
                    -51,    // PTwiRightBound
                    -211,    // PTwiLeftBound
                    -55,    // PTwiUpperBound
                    -168    // PTwiLowerBound
                },
                // Fluorescent
                {
                    190,    // PFluoRightBound
                    -208,    // PFluoLeftBound
                    -75,    // PFluoUpperBound
                    -240    // PFluoLowerBound
                },
                // Warm fluorescent
                {
                    -126,    // PWFluoRightBound
                    -326,    // PWFluoLeftBound
                    -75,    // PWFluoUpperBound
                    -240    // PWFluoLowerBound
                },
                // Incandescent
                {
                    -126,    // PIncaRightBound
                    -326,    // PIncaLeftBound
                    -55,    // PIncaUpperBound
                    -168    // PIncaLowerBound
                },
                // Gray World
                {
                    5000,    // PGWRightBound
                    -5000,    // PGWLeftBound
                    5000,    // PGWUpperBound
                    -5000    // PGWLowerBound
                }
            },
            // PWB default gain	
            {
                // Daylight
                {
                    656,    // PWB_Day_R
                    512,    // PWB_Day_G
                    540    // PWB_Day_B
                },
                // Cloudy daylight
                {
                    789,    // PWB_Cloudy_R
                    512,    // PWB_Cloudy_G
                    450    // PWB_Cloudy_B
                },
                // Shade
                {
                    844,    // PWB_Shade_R
                    512,    // PWB_Shade_G
                    420    // PWB_Shade_B
                },
                // Twilight
                {
                    499,    // PWB_Twi_R
                    512,    // PWB_Twi_G
                    711    // PWB_Twi_B
                },
                // Fluorescent
                {
                    626,    // PWB_Fluo_R
                    512,    // PWB_Fluo_G
                    641    // PWB_Fluo_B
                },
                // Warm fluorescent
                {
                    467,    // PWB_WFluo_R
                    512,    // PWB_WFluo_G
                    860    // PWB_WFluo_B
                },
                // Incandescent
                {
                    438,    // PWB_Inca_R
                    512,    // PWB_Inca_G
                    809    // PWB_Inca_B
                },
                // Gray World
                {
                    512,    // PWB_GW_R
                    512,    // PWB_GW_G
                    512    // PWB_GW_B
                }
            },
            // AWB preference color	
            {
                // Tungsten
                {
                    50,    // TUNG_SLIDER
                    4155    // TUNG_OFFS
                },
                // Warm fluorescent	
                {
                    50,    // WFluo_SLIDER
                    4155    // WFluo_OFFS
                },
                // Shade
                {
                    0,    // Shade_SLIDER
                    1409    // Shade_OFFS
                },
                // Sunset Area
                {
                    170,   // i4Sunset_BoundXr_Thr
                    -156    // i4Sunset_BoundYr_Thr
                },
                // Sunset Vertex
                {
                    -2,   // i4Sunset_BoundXr_Thr
                    -156    // i4Sunset_BoundYr_Thr
                },
                // Shade F Area
                {
                    -165,   // i4BoundXrThr
                    -90    // i4BoundYrThr
                },
                // Shade F Vertex
                {
                    -155,   // i4BoundXrThr
                    -159    // i4BoundYrThr
                },
                // Shade CWF Area
                {
                    -165,   // i4BoundXrThr
                    -243    // i4BoundYrThr
                },
                // Shade CWF Vertex
                {
                    -155,   // i4BoundXrThr
                    -220    // i4BoundYrThr
                },
                // Low CCT Area
                {
                    -342,   // i4BoundXrThr
                    188    // i4BoundYrThr
                },
                // Low CCT Vertex
                {
                    -342,   // i4BoundXrThr
                    188    // i4BoundYrThr
                }
            },
            // CCT estimation
            {
                // CCT
                {
                    2300,    // i4CCT[0]
                    2850,    // i4CCT[1]
                    3750,    // i4CCT[2]
                    5100,    // i4CCT[3]
                    6500     // i4CCT[4]
                },
                // Rotated X coordinate
                {
                -462,    // i4RotatedXCoordinate[0]
                -366,    // i4RotatedXCoordinate[1]
                -248,    // i4RotatedXCoordinate[2]
                -163,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
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
                {0, 350, 800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
            },
            // Daylight locus offset LUTs for WF
            {
                21, // i4Size: LUT dimension
                {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
            },
            // Daylight locus offset LUTs for Shade
            {
                21, // i4Size: LUT dimension
                {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
            },
            // Preference gain for each light source
            {
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, 
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // STROBE
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, 
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // TUNGSTEN
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {500, 512, 512}, 
                    {500, 512, 512}, {500, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // WARM F
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {514, 512, 512}, {516, 512, 512}, {518, 512, 512}, {518, 512, 512}, {516, 512, 512}, 
                    {514, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                }, // F
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, 
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // CWF
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, 
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {516, 512, 508}, {516, 512, 506}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                }, // DAYLIGHT
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, 
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                }, // SHADE
                {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, 
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 508}, {512, 512, 506}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                } // DAYLIGHT F
            },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    115,    // i4InitLVThr_L
                    155,    // i4InitLVThr_H
                    100      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                    65,     // i4Neutral_ParentBlk_Thr
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    // Non neutral
                    {   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Flurescent
                    {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // CWF
                    {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Daylight
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
                    // DF
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
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
                        50,        // i4LVThr_L
                        70,        // i4LVThr_H
                        128         // i4DaylightProb
                    },
                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        50,        // i4LVThr_L
                        70,        // i4LVThr_H
                        192         // i4DaylightProb
                    },
                    // Low CCT
                    {
                        0,          // i4Enable
                        256        // i4SpeedRatio
                    }
                },
                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                    {   0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
                },
                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   0,   0,   0,   0}, // Strobe
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90,  80,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  40,  30,  20}, // Daylight
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  90,  70,  50,  25,   0,   0}, // Shade
                    {  90,  90,  90,  90,  90,  90,  80,  80,  60,  60,  60,  30,   0,   0,   0,   0,   0,   0,   0} // Daylight fluorescent
                }
            }
        },
        {
          {
            // AWB calibration data
            {
                // rUnitGain (unit gain: 1.0 = 512)
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // rGoldenGain (golden sample gain: 1.0 = 512)
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // rD65Gain (D65 WB gain: 1.0 = 512)
                {
                    0,    // D65Gain_R
                    0,    // D65Gain_G
                    0    // D65Gain_B
                }
            },
            // Original XY coordinate of AWB light source
            {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    0,    // OriX_Hor
                    0    // OriY_Hor
                },
                // A
                {
                    0,    // OriX_A
                    0    // OriY_A
                },
                // TL84
                {
                    0,    // OriX_TL84
                    0    // OriY_TL84
                },
                // CWF
                {
                    0,    // OriX_CWF
                    0    // OriY_CWF
                },
                // DNP
                {
                    0,    // OriX_DNP
                    0    // OriY_DNP
                },
                // D65
                {
                    0,    // OriX_D65
                    0    // OriY_D65
                },
                // DF
                {
                    0,    // OriX_DF
                    0    // OriY_DF
                }
            },
            // Rotated XY coordinate of AWB light source
            {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    0,    // RotX_Hor
                    0    // RotY_Hor
                },
                // A
                {
                    0,    // RotX_A
                    0    // RotY_A
                },
                // TL84
                {
                    0,    // RotX_TL84
                    0    // RotY_TL84
                },
                // CWF
                {
                    0,    // RotX_CWF
                    0    // RotY_CWF
                },
                // DNP
                {
                    0,    // RotX_DNP
                    0    // RotY_DNP
                },
                // D65
                {
                    0,    // RotX_D65
                    0    // RotY_D65
                },
                // DF
                {
                    0,    // RotX_DF
                    0    // RotY_DF
                }
            },
            // AWB gain of AWB light source
            {
                // Strobe 
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // Horizon 
                {
                    0,    // AWBGAIN_HOR_R
                    0,    // AWBGAIN_HOR_G
                    0    // AWBGAIN_HOR_B
                },
                // A 
                {
                    0,    // AWBGAIN_A_R
                    0,    // AWBGAIN_A_G
                    0    // AWBGAIN_A_B
                },
                // TL84 
                {
                    0,    // AWBGAIN_TL84_R
                    0,    // AWBGAIN_TL84_G
                    0    // AWBGAIN_TL84_B
                },
                // CWF 
                {
                    0,    // AWBGAIN_CWF_R
                    0,    // AWBGAIN_CWF_G
                    0    // AWBGAIN_CWF_B
                },
                // DNP 
                {
                    0,    // AWBGAIN_DNP_R
                    0,    // AWBGAIN_DNP_G
                    0    // AWBGAIN_DNP_B
                },
                // D65 
                {
                    0,    // AWBGAIN_D65_R
                    0,    // AWBGAIN_D65_G
                    0    // AWBGAIN_D65_B
                },
                // DF 
                {
                    0,    // AWBGAIN_DF_R
                    0,    // AWBGAIN_DF_G
                    0    // AWBGAIN_DF_B
                }
            },
            // Rotation matrix parameter
            {
                0,    // RotationAngle
                0,    // Cos
                0    // Sin
            },
            // Daylight locus parameter
            {
                0,    // i4SlopeNumerator
                0    // i4SlopeDenominator
            },
            // Predictor gain
            {
                0, // i4PrefRatio100
                // DaylightLocus_L
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // DaylightLocus_H
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // Temporal General
                {
                    0,    // i4R
                    0,    // i4G
                    0    // i4B
                },
                // AWB LSC Gain
                {
                    0,        // i4R
                    0,        // i4G
                    0        // i4B
                }
            },
            // AWB light area
            {
                // Strobe:FIXME
                {
                    0,    // i4RightBound
                    0,    // i4LeftBound
                    0,    // i4UpperBound
                    0    // i4LowerBound
                },
                // Tungsten
                {
                    0,    // TungRightBound
                    0,    // TungLeftBound
                    0,    // TungUpperBound
                    0    // TungLowerBound
                },
                // Warm fluorescent
                {
                    0,    // WFluoRightBound
                    0,    // WFluoLeftBound
                    0,    // WFluoUpperBound
                    0    // WFluoLowerBound
                },
                // Fluorescent
                {
                    0,    // FluoRightBound
                    0,    // FluoLeftBound
                    0,    // FluoUpperBound
                    0    // FluoLowerBound
                },
                // CWF
                {
                0,    // CWFRightBound
                0,    // CWFLeftBound
                0,    // CWFUpperBound
                0    // CWFLowerBound
                },
                // Daylight
                {
                    0,    // DayRightBound
                    0,    // DayLeftBound
                    0,    // DayUpperBound
                    0    // DayLowerBound
                },
                // Shade
                {
                    0,    // ShadeRightBound
                    0,    // ShadeLeftBound
                    0,    // ShadeUpperBound
                    0    // ShadeLowerBound
                },
                // Daylight Fluorescent
                {
                    0,    // DFRightBound
                    0,    // DFLeftBound
                    0,    // DFUpperBound
                    0    // DFLowerBound
                }
            },
            // PWB light area
            {
                // Reference area
                {
                    0,    // PRefRightBound
                    0,    // PRefLeftBound
                    0,    // PRefUpperBound
                    0    // PRefLowerBound
                },
                // Daylight
                {
                    0,    // PDayRightBound
                    0,    // PDayLeftBound
                    0,    // PDayUpperBound
                    0    // PDayLowerBound
                },
                // Cloudy daylight
                {
                    0,    // PCloudyRightBound
                    0,    // PCloudyLeftBound
                    0,    // PCloudyUpperBound
                    0    // PCloudyLowerBound
                },
                // Shade
                {
                    0,    // PShadeRightBound
                    0,    // PShadeLeftBound
                    0,    // PShadeUpperBound
                    0    // PShadeLowerBound
                },
                // Twilight
                {
                    0,    // PTwiRightBound
                    0,    // PTwiLeftBound
                    0,    // PTwiUpperBound
                    0    // PTwiLowerBound
                },
                // Fluorescent
                {
                    0,    // PFluoRightBound
                    0,    // PFluoLeftBound
                    0,    // PFluoUpperBound
                    0    // PFluoLowerBound
                },
                // Warm fluorescent
                {
                    0,    // PWFluoRightBound
                    0,    // PWFluoLeftBound
                    0,    // PWFluoUpperBound
                    0    // PWFluoLowerBound
                },
                // Incandescent
                {
                    0,    // PIncaRightBound
                    0,    // PIncaLeftBound
                    0,    // PIncaUpperBound
                    0    // PIncaLowerBound
                },
                // Gray World
                {
                    0,    // PGWRightBound
                    0,    // PGWLeftBound
                    0,    // PGWUpperBound
                    0    // PGWLowerBound
                }
            },
            // PWB default gain	
            {
                // Daylight
                {
                    0,    // PWB_Day_R
                    0,    // PWB_Day_G
                    0    // PWB_Day_B
                },
                // Cloudy daylight
                {
                    0,    // PWB_Cloudy_R
                    0,    // PWB_Cloudy_G
                    0    // PWB_Cloudy_B
                },
                // Shade
                {
                    0,    // PWB_Shade_R
                    0,    // PWB_Shade_G
                    0    // PWB_Shade_B
                },
                // Twilight
                {
                    0,    // PWB_Twi_R
                    0,    // PWB_Twi_G
                    0    // PWB_Twi_B
                },
                // Fluorescent
                {
                    0,    // PWB_Fluo_R
                    0,    // PWB_Fluo_G
                    0    // PWB_Fluo_B
                },
                // Warm fluorescent
                {
                    0,    // PWB_WFluo_R
                    0,    // PWB_WFluo_G
                    0    // PWB_WFluo_B
                },
                // Incandescent
                {
                    0,    // PWB_Inca_R
                    0,    // PWB_Inca_G
                    0    // PWB_Inca_B
                },
                // Gray World
                {
                    0,    // PWB_GW_R
                    0,    // PWB_GW_G
                    0    // PWB_GW_B
                }
            },
            // AWB preference color	
            {
                // Tungsten
                {
                    0,    // TUNG_SLIDER
                    0    // TUNG_OFFS
                },
                // Warm fluorescent	
                {
                    0,    // WFluo_SLIDER
                    0    // WFluo_OFFS
                },
                // Shade
                {
                    0,    // Shade_SLIDER
                    0    // Shade_OFFS
                },
                // Sunset Area
                {
                    0,   // i4Sunset_BoundXr_Thr
                    0    // i4Sunset_BoundYr_Thr
                },
                // Sunset Vertex
                {
                    0,   // i4Sunset_BoundXr_Thr
                    0    // i4Sunset_BoundYr_Thr
                },
                // Shade F Area
                {
                    0,   // i4BoundXrThr
                    0    // i4BoundYrThr
                },
                // Shade F Vertex
                {
                    0,   // i4BoundXrThr
                    0    // i4BoundYrThr
                },
                // Shade CWF Area
                {
                    0,   // i4BoundXrThr
                    0    // i4BoundYrThr
                },
                // Shade CWF Vertex
                {
                    0,   // i4BoundXrThr
                    0    // i4BoundYrThr
                },
                // Low CCT Area
                {
                    0,   // i4BoundXrThr
                    0    // i4BoundYrThr
                },
                // Low CCT Vertex
                {
                    0,   // i4BoundXrThr
                    0    // i4BoundYrThr
                }
            },
            // CCT estimation
            {
                // CCT
                {
                    0,    // i4CCT[0]
                    0,    // i4CCT[1]
                    0,    // i4CCT[2]
                    0,    // i4CCT[3]
                    0     // i4CCT[4]
                },
                // Rotated X coordinate
                {
                0,    // i4RotatedXCoordinate[0]
                0,    // i4RotatedXCoordinate[1]
                0,    // i4RotatedXCoordinate[2]
                0,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
                }
                }
            },
            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

            // Daylight locus offset LUTs for tungsten
            {
                0, // i4Size: LUT dimension
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // i4LUTIn
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} // i4LUTOut
            },
            // Daylight locus offset LUTs for WF
            {
                0, // i4Size: LUT dimension
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // i4LUTIn
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} // i4LUTOut
            },
            // Daylight locus offset LUTs for Shade
            {
                0, // i4Size: LUT dimension
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // i4LUTIn
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} // i4LUTOut
            },
            // Preference gain for each light source
            {
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // STROBE
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // TUNGSTEN
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // WARM F
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // F
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // CWF
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // DAYLIGHT
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                }, // SHADE
                {
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 
                    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
                } // DAYLIGHT F
            },
                // Parent block weight parameter
                {
                    0,      // bEnable
                    0           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    0,    // i4InitLVThr_L
                    0,    // i4InitLVThr_H
                    0      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                    0,     // i4Neutral_ParentBlk_Thr
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    // Non neutral
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // (%)
                    // Flurescent
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // (%)
                    // CWF
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // (%)
                    // Daylight
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // (%)
                    // DF
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // (%)
                },
                // AWB feature detection
                {
                // Sunset Prop
                    {
                        0,          // i4Enable
                        0,        // i4LVThr_L
                        0,        // i4LVThr_H
                        0,         // i4SunsetCountThr
                        0,          // i4SunsetCountRatio_L
                        0         // i4SunsetCountRatio_H
                    },
                    // Shade F Detection
                    {
                        0,          // i4Enable
                        0,        // i4LVThr_L
                        0,        // i4LVThr_H
                        0         // i4DaylightProb
                    },
                    // Shade CWF Detection
                    {
                        0,          // i4Enable
                        0,        // i4LVThr_L
                        0,        // i4LVThr_H
                        0         // i4DaylightProb
                    },
                    // Low CCT
                    {
                        0,          // i4Enable
                        0        // i4SpeedRatio
                    }
                },
                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
                },
                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Strobe
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Tungsten
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Warm fluorescent
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Fluorescent
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // CWF
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Daylight
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Shade
                    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0} // Daylight fluorescent
                }
            }
        }
    },
    // Flash AWB NVRAM
    {
#include INCLUDE_FILENAME_FLASH_AWB_PARA
    },

    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    {
        1,  // isTsfEn
        2,  // tsfCtIdx
        {20, 2000, -110, -110, 512, 512, 512, 0}    // rAWBInput[8]
    },

#include INCLUDE_FILENAME_TSF_PARA
#include INCLUDE_FILENAME_TSF_DATA
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
                                             sizeof(CAMERA_TSF_TBL_STRUCT)
	};

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
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

      default:
            return 1;
    }
    return 0;
  }};  //  NSFeature


