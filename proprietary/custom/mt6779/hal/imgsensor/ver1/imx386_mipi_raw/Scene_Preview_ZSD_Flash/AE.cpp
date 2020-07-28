/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#include "camera_custom_isp_nvram.h"

#include "inc.h"

#define AE_BASE                                                      AE_AE_BASE

const AE_NVRAM_T AE_BASE[1] = {
    {
        .rDevicesInfo = 
        {    //rDevicesInfo
            1024,       // u4MinGain, 1024 base =  1x
            16384,      // u4MaxGain, 16x
            100,        // u4MiniISOGain, ISOxx
            16,         // u4GainStepUnit, 1x/8
            11840,      // u4PreExpUnit
            30,         // u4PreMaxFrameRate
            12261,      // u4VideoExpUnit
            30,         // u4VideoMaxFrameRate
            1024,       // u4Video2PreRatio, 1024 base = 1x
            9250,       // u4CapExpUnit
            30,         // u4CapMaxFrameRate
            1024,       // u4Cap2PreRatio, 1024 base = 1x
            6693,       // u4Video1ExpUnit
            120,        // u4Video1MaxFrameRate
            1024,       // u4Video12PreRatio, 1024 base = 1x
            15047,      // u4Video2ExpUnit
            30,         // u4Video2MaxFrameRate
            1024,       // u4Video22PreRatio, 1024 base = 1x
            19770,      // u4Custom1ExpUnit
            30,         // u4Custom1MaxFrameRate
            1024,       // u4Custom12PreRatio, 1024 base = 1x
            19770,      // u4Custom2ExpUnit
            30,         // u4Custom2MaxFrameRate
            1024,       // u4Custom22PreRatio, 1024 base = 1x
            19770,      // u4Custom3ExpUnit
            30,         // u4Custom3MaxFrameRate
            1024,       // u4Custom32PreRatio, 1024 base = 1x
            19770,      // u4Custom4ExpUnit
            30,         // u4Custom4MaxFrameRate
            1024,       // u4Custom42PreRatio, 1024 base = 1x
            19770,      // u4Custom5ExpUnit
            30,         // u4Custom5MaxFrameRate
            1024,       // u4Custom52PreRatio, 1024 base = 1x
            18,         // u4LensFno, Fno = 2.0
            350         // u4FocusLength_100x
        },
        .rHistConfig = 
        {  // rHistConfig
            1,      // u4HistHighThres
            90,     // u4HistLowThres
            200,    // u4MostBrightRatio
            255,    // u4MostDarkRatio
            1024,   // u4CentralHighBound
            1024,   // u4CentralLowBound
            {1024, 1024, 13, 210, 100},     // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {200, 255, 1024, 0, 15},        // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {308, 308, 308, 308, 308 }       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        .rCCTConfig = 
        {
            MTRUE,            // bEnableBlackLight
            MTRUE,            // bEnableHistStretch
            MTRUE,            // bEnableAntiOverExposure
            MTRUE,            // bEnableTimeLPF
            MTRUE,            // bEnableCaptureThres
            MTRUE,            // bEnableVideoThres
            MTRUE,            // bEnableVideo1Thres
            MTRUE,            // bEnableVideo2Thres
            MTRUE,            // bEnableCustom1Thres
            MTRUE,            // bEnableCustom2Thres
            MTRUE,            // bEnableCustom3Thres
            MTRUE,            // bEnableCustom4Thres
            MTRUE,            // bEnableCustom5Thres
            MTRUE,            // bEnableStrobeThres
            47,              // u4AETarget
            47,              // u4StrobeAETarget
            50,              // u4InitIndex
            4,               // u4BackLightWeight
            32,              // u4HistStretchWeight
            4,               // u4AntiOverExpWeight
            2,               // u4BlackLightStrengthIndex
            2,               // u4HistStretchStrengthIndex
            2,               // u4AntiOverExpStrengthIndex
            2,               // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,              // u4InDoorEV = 9.0, 10 base
            -12,             // i4BVOffset delta BV = value/10
            32,              // u4PreviewFlareOffset
            32,              // u4CaptureFlareOffset
            10,              // u4CaptureFlareThres
            32,              // u4VideoFlareOffset
            10,              // u4VideoFlareThres
            32,              // u4CustomFlareOffset
            10,              //  u4CustomFlareThres
            32,              // u4StrobeFlareOffset //12 bits
            3,               // u4StrobeFlareThres // 0.5%
            32,              // u4PrvMaxFlareThres //12 bit
            0,               // u4PrvMinFlareThres
            32,              // u4VideoMaxFlareThres // 12 bit
            0,               // u4VideoMinFlareThres
            18,              // u4FlatnessThres              // 10 base for flatness condition.
            75,    // u4FlatnessStrength
            //rMeteringSpec
            {
                //rHS_Spec
                {
                    MTRUE,   //bEnableHistStretch           // enable histogram stretch
                    1024,   //u4HistStretchWeight          // Histogram weighting value
                    60,     //u4Pcent                      // 1%=10, 0~1000
                    160,    //u4Thd                        // 0~255
                    75,     //u4FlatThd                    // 0~255
                    120,    //u4FlatBrightPcent
                    120,    //u4FlatDarkPcent
                    //sFlatRatio
                    {
                        1000,  //i4X1
                        1024,  //i4Y1
                        2400, //i4X2
                        0     //i4Y2
                    },
                    MFALSE, //bEnableGreyTextEnhance
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
                    MTRUE,   //bEnableAntiOverExposure
                    1024,   //u4AntiOverExpWeight
                    10,     //u4Pcent
                    220,    //u4Thd
    
                    MTRUE,   //bEnableCOEP
                    1,      //u4COEPcent
                    98,     //u4COEThd
                    0,      //u4BVCompRatio
                    //sCOEYRatio;     //the outer y ratio
                    {
                        13,     //i4X1
                        1024,   //i4Y1
                        47,     //i4X2
                        0       //i4Y2
                    },
                    //sCOEDiffRatio;  // inner/outer y difference ratio
                    {
                        1000, //i4X1
                        0,    //i4Y1
                        2500, //i4X2
                        500   //i4Y2
                    }
                },
                //rABL_Spec
                {
                    MTRUE,   //bEnableBlackLigh
                    1024,   //u4BackLightWeigh
                    500,    //u4Pcent
                    35,     //u4Thd,
                    255,    //center luminance
                    228,    //final target limitation, 256/128 = 2x
                    //sFgBgEVRatio
                    {
                        3000,   //i4X1
                        0,      //i4Y1
                        6200,   //i4X2
                        1024    //i4Y2
                    },
                    //sBVRatio
                    {
                        4000,   //i4X1
                        0,      //i4Y1
                        8000,   //i4X2
                        1024    //i4Y2
                    },
                    MTRUE,         //Enable TargetStrength change along BV
                    4,                          //Max size : 8   
                    {1100, 2600, 4500,6000},    //size must large than u4TblLength
                    { 65, 150, 165, 180 },      //TargetStrength TBL along BV
                },
                //rNS_Spec
                {
                    MTRUE,   //bEnableNightScene
                    10,     //u4Pcent
                    150,    //u4Thd
                    50,     //u4FlatThd
    
                    205,    //u4BrightTonePcent
                    50,     //u4BrightToneThd
    
                    500,   //u4LowBndPcent
                    10,    //u4LowBndThd
                    24,    //u4LowBndThdLimit
    
                    50,    //u4FlatBrightPcent;
                    300,   //u4FlatDarkPcent;
                    //sFlatRatio
                    {
                        1200,   //i4X1
                        1024,   //i4Y1
                        2700,   //i4X2
                        0       //i4Y2
                    },
                    //sBVRatio
                    {
                        -1200,  //i4X1
                        1024,   //i4Y1
                        2500,   //i4X2
                        0       //i4Y2
                    },
                    MTRUE, // bEnableNightSkySuppresion
                    //sSkyBVRatio
                    {
                        -4000,  //i4X1
                        1024,   //i4X2
                        -1200,  //i4Y1
                        0       //i4Y2
                    }
                },
                // rTOUCHFD_Spec
                {
                    60,  // u4CwrLowBound;        // metering boundary min CWR value
                    245, // u4CwrHighBound;      // metering boundary max CWR value
                    400, // u4MeterWeight 1024 base.100%=>1024
                    40, //uMeteringYLowBound;
                    50, //uMeteringYHighBound;
                    120,//u4MeteringStableMax;
                    80, //u4MeteringStableMin;
                }
            }, //End rMeteringSpec
            // rFlareSpec
            {
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                96,                                               //u4FlareStdThrHigh;
                0,                                               //u4FlareStdThrLow;
                0,                                                //u4PrvCapFlareDiff;
                2,                                                //u4FlareMaxStepGap_Fast;
                0,                                                //u4FlareMaxStepGap_Slow;
                1800,                                             //u4FlarMaxStepGapLimitBV;
                2,                                                //u4FlareAEStableCount;
            },
            //rAEMoveRatio =
            {
                100,    //u4SpeedUpRatio
                100,    //u4GlobalRatio
                190,    //u4Bright2TargetEnd
                10,     //u4Dark2TargetStart
                100,    //u4B2TEnd
                90,     //u4B2TStart
                90,     //u4D2TEnd
                90,     //u4D2TStart
            },
    
            //rAEVideoMoveRatio =
            {
                100,    //u4SpeedUpRatio
                100,    //u4GlobalRatio
                190,    //u4Bright2TargetEnd
                10,     //u4Dark2TargetStart
                70,     //u4B2TEnd
                30,     //u4B2TStart
                20,     //u4D2TEnd
                60,     //u4D2TStart
            },
    
            //rAEVideo1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                40,  //u4B2TStart
                30,  //u4D2TEnd
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
                30,  //u4D2TEnd
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
                30,  //u4D2TEnd
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
                30,  //u4D2TEnd
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
                30,  //u4D2TEnd
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
                30,  //u4D2TEnd
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
                30,  //u4D2TEnd
                90,  //u4D2TStart
            },
    
            //rAEFaceMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                30, //u4B2TEnd
                30,  //u4B2TStart
                30,  //u4D2TEnd
                30,  //u4D2TStart
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
                130,    // u4BVCompRatio
                {
                    {
                        28,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        10,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        1,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        28,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        15,  //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        3,      //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        28,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        25,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        8,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    }
                }
            }
        },
        .rAeParam = 
        {
            {     //strAEParasetting
                MFALSE,               // bEnableSaturationCheck
                MTRUE,                // bEnablePreIndex
                MTRUE,                // bEnableRotateWeighting;
                MTRUE,               // bEV0TriggerStrobe
                MFALSE,               // bLockCamPreMeteringWin;
                MFALSE,               // bLockVideoPreMeteringWin;
                MTRUE,                // bLockVideoRecMeteringWin;
                MTRUE,                // bSkipAEinBirghtRange;
                MTRUE,                // bPreAFLockAE
                MTRUE,                // bStrobeFlarebyCapture
                MTRUE,                // bEnableFaceAE
                MTRUE,                // bEnableMeterAE
                MTRUE,                // b4FlarMaxStepGapLimitEnable
                MFALSE,                // bEnableAESmoothGain
                MTRUE,                // bEnableLongCaptureCtrl
                MFALSE,                //bEnableTouchSmoothRatio : enable Touch mode smooth converge
                MFALSE,               // bEnableTouchSmooth : enable Touch mode smooth with peframe
                MTRUE,                // bEnablePixelBaseHist
                MTRUE,                // bEnableHDRSTATconfig   : enable HDR Y LSB config
                MTRUE,                // bEnableAEOVERCNTconfig  : enable AE over-exposure count config
                MTRUE,                // bEnableTSFSTATconfig    : enable TSF RGB STAT config
                1,                // bEnableHDRLSB
                MFALSE,                 //bEnableFlareFastConverge
                MFALSE,                 //bEnableRAFastConverge
                MTRUE,                 //bEnableFaceFastConverge
                256,                  // u4BackLightStrength : strength of backlight condtion
                256,                  // u4OverExpStrength : strength of anti over exposure
                256,                  // u4HistStretchStrength : strength of  histogram stretch
                0,                     // u4SmoothLevel : time LPF smooth level , internal use
                4,                     // u4TimeLPFLevel : time LOW pass filter level
                120,                  // u4AEWinodwNumX;                   // AE statistic winodw number X
                90,                   // u4AEWinodwNumY;                   // AE statistic winodw number Y
                AE_BLOCK_NO,  // uBockXNum : AE X block number;
                AE_BLOCK_NO,  // uBockYNum : AE Yblock number;
                AE_TG_BLOCK_NO_X, // uTgBlockNumX : AE X tg block number;
                AE_TG_BLOCK_NO_Y, // uTgBlockNumY : AE Y tg block number;
                20,                    // uSatBlockCheckLow : saturation block check , low thres
                50,                     // uSatBlockCheckHigh : sturation  block check , hight thres
                50,                     // uSatBlockAdjustFactor : adjust factore , to adjust central weighting target value
    
                80,                     // uMeteringYLowSkipRatio : metering area min Y value to skip AE
                120,                   // uMeteringYHighSkipRatio : metering area max Y value to skip AE
                1,                     // u4MinCWRecommend;    // mini target value
                250,                     // u4MaxCWRecommend;    // max target value
                -50,                     // iMiniBVValue;               // mini BV value.
                2,                      // uAEShutterDelayCycle;         // for AE smooth used.
                2,                      // uAESensorGainDelayCycleWShutter;
                1,                      // uAESensorGainDelayCycleWOShutter;
                0,                      // uAEIspGainDelayCycle;
    
                5,                  // u4AEYCoefR
                9,                  // u4AEYCoefG
                2,                  // u4AEYCoefB
    
                200000,             // u4LongCaptureThres 100ms
                1,                  //u4CenterFaceExtraWeighting;
                7,                  //u2AEStatThrs;
                1,                  //uCycleNumAESkipAfterExit;
                254,                //overexposure cnt threshold
                11,                 //u4HSSmoothTHD
                3,                  //u4FinerEvIndexBase 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
                MFALSE,             //bNewConstraintForMeteringEnable, MFALSE: Disable New constraint for metering
            },
            {
                {
                    AE_WEIGHTING_CENTRALWEIGHT,
                    {
                        {6 ,19 ,28 ,19 ,6 },
                        {19 ,60 ,88 ,60 ,19 },
                        {28 ,88 ,160 ,88 ,28 },
                        {19 ,60 ,88 ,60 ,19 },
                        {6 ,19 ,28 ,19 ,6 },
                    }
                },
                {
                    AE_WEIGHTING_SPOT,
                    {
                        {0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0},
                        {0, 0, 1, 0, 0},
                        {0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0},
                    }
                },
                {
                    AE_WEIGHTING_AVERAGE,
                    {
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                    }
                },
                {
                    AE_WEIGHTING_AVERAGE,
                    {
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                        {1, 1, 1, 1, 1},
                    }
                },
            },
            {    // strAFPLineTable from high lv to low lv
                0,                  // 1 enable the AF Pline, 0 disable the AF Pline (bypass)
                {
                    {73, 30},       // {LV value, AF frame rate}
                    {65, 25},
                    {55, 19},
                    {45, 17},
                    {0, 17},
                }
            },
            {    //strAFZSDPLineTable from high lv to low lv
                0,                  // 1 enable the AF ZSD Pline, 0 disable the AF ZSD Pline (bypass)
                {
                    {73, 15},       // {LV value, AF frame rate}
                    {65, 15},
                    {55, 15},
                    {45, 15},
                    {0, 10},
                }
            },
            {    //strStrobePLineTable from high lv to low lv
                1,                  // 1 enable the Strobe Pline, 0 disable the Strobe Pline (bypass)
                {
                    {73, 30},       // {LV value, Strobe frame rate}
                    {65, 25},
                    {55, 19},
                    {45, 17},
                    {0, 17},
                }
            },
            {    //strStrobeZSDPLineTable from high lv to low lv
                1,                  // 1 enable the Strobe ZSD Pline, 0 disable the Strobe ZSD Pline (bypass)
                {
                    {73, 15},       // {LV value, Strobe frame rate}
                    {65, 15},
                    {55, 15},
                    {45, 15},
                    {0, 10},
                }
            },
            {
                {
                    1024,  // EV   0
                    1097,   //EV    0.1
                    1176,   //EV    0.2
                    1261,   //EV    0.3
                    1351,   //EV    0.4
                    1448,   //EV    0.5
                    1552,   //EV    0.6
                    1663,   //EV    0.7
                    1783,   //EV    0.8
                    1911,   //EV    0.9
                    2048,   //EV    1
                    2195,   //EV    1.1
                    2353,   //EV    1.2
                    2521,   //EV    1.3
                    2702,   //EV    1.4
                    2896,   //EV    1.5
                    3104,   //EV    1.6
                    3327,   //EV    1.7
                    3566,   //EV    1.8
                    3822,   //EV    1.9
                    4096,   //EV    2
                    4390,   //EV    2.1
                    4705,   //EV    2.2
                    5043,   //EV    2.3
                    5405,   //EV    2.4
                    5793,   //EV    2.5
                    6208,   //EV    2.6
                    6654,   //EV    2.7
                    7132,   //EV    2.8
                    7643,   //EV    2.9
                    8192,   //EV    3
                    8780,   //EV    3.1
                    9410,   //EV    3.2
                    10086,  //EV    3.3
                    10809,  //EV    3.4
                    11585,  //EV    3.5
                    12417,  //EV    3.6
                    13308,  //EV    3.7
                    14263,  //EV    3.8
                    15287,  //EV    3.9
                    16384,  //EV    4
                    955,    //EV    -0.1
                    891,    //EV    -0.2
                    832,    //EV    -0.3
                    776,    //EV    -0.4
                    724,    //EV    -0.5
                    676,    //EV    -0.6
                    630,    //EV    -0.7
                    588,    //EV    -0.8
                    549,    //EV    -0.9
                    512,    //EV    -1
                    478,    //EV    -1.1
                    446,    //EV    -1.2
                    416,    //EV    -1.3
                    388,    //EV    -1.4
                    362,    //EV    -1.5
                    338,    //EV    -1.6
                    315,    //EV    -1.7
                    294,    //EV    -1.8
                    274,    //EV    -1.9
                    256,    //EV    -2
                    239,    //EV    -2.1
                    223,    //EV    -2.2
                    208,    //EV    -2.3
                    194,    //EV    -2.4
                    181,    //EV    -2.5
                    169,    //EV    -2.6
                    158,    //EV    -2.7
                    147,    //EV    -2.8
                    137,    //EV    -2.9
                    128,    //EV    -3
                    119,    //EV    -3.1
                    111,    //EV    -3.2
                    104,    //EV    -3.3
                    97, //EV    -3.4
                    91, //EV    -3.5
                    84, //EV    -3.6
                    79, //EV    -3.7
                    74, //EV    -3.8
                    69, //EV    -3.9
                    64, //EV    -4
                }
            },
            {
                {
                    {-20,   17,    24}, //   mean below -2.5  move increase 25 index
                    {-20,   25,    19}, //   -2.5~-2  move increase 20 index
                    {-15,   33,    15}, //   -2~-1.6
                    {-15,   40,    12}, //   -1.6~-1.3
                    {-10,   50,    9}, //   -1.3~-1
                    { -8,   57,     7}, //   -1~-0.8
                    { -5,   71,     4}, //   -0.8~-0.5
                    { -4,   75,     3}, //   -0.5~-0.4
                    { -3,   81,     2}, //   -0.4~-0.3
                    { -1,   90,     1}, //   -0.3~-0.1
                    {   0,  100,     0}, //   -0.1~0
                    {   1,  110,     0}, //     0~0.1
                    {   2,  114,    -1}, //    0.1~0.2       move decrease 1 index
                    {   3,  123,    -1}, //    0.2~0.3
                    {   4,  131,    -2}, //    0.3~0.4
                    {   5,  141,    -3}, //    0.4~0.5
                    {   7,  162,    -4}, //    0.5~0.7
                    {   9,  186,    -6}, //    0.7~0.9
                    { 10,  200,   -8}, //    0.9~1.0
                    { 13,  246,   -9}, //    1.0~1.3
                    { 16,  303,   -12}, //    1.3~1.6
                    { 20,  400,   -15}, //    1.6~2       move decrease 10  index
                    { 25,  566,   -19}, //    2~2.5       move decrease 20  index
                    { 30,  800,   -22}, //    2.5~3      move decrease 30  index
                }
            },   
            {//AEVideoMoveTable
                {
                    {-20,   17,    20}, //   mean below -2.5  move increase 25 index
                    {-20,   25,    15}, //   -2.5~-2  move increase 20 index
                    {-15,   33,    10}, //   -2~-1.6
                    {-15,   40,    6}, //   -1.6~-1.3
                    {-10,   50,    4}, //   -1.3~-1
                    { -8,   57,     3}, //   -1~-0.8
                    { -5,   71,     2}, //   -0.8~-0.5
                    { -4,   75,     1}, //   -0.5~-0.4
                    { -3,   81,     1}, //   -0.4~-0.3
                    { -1,   90,     1}, //   -0.3~-0.1
                    {   0,  100,     0}, //   -0.1~0
                    {   1,  110,     0}, //     0~0.1
                    {   2,  114,    -1}, //    0.1~0.2       move decrease 1 index
                    {   3,  123,    -1}, //    0.2~0.3
                    {   4,  131,    -1}, //    0.3~0.4
                    {   5,  141,    -2}, //    0.4~0.5
                    {   7,  162,    -2}, //    0.5~0.7
                    {   9,  186,    -3}, //    0.7~0.9
                    { 10,  200,   -4}, //    0.9~1.0
                    { 13,  246,   -4}, //    1.0~1.3
                    { 16,  303,   -6}, //    1.3~1.6
                    { 20,  400,   -7}, //    1.6~2       move decrease 10  index
                    { 25,  566,   -9}, //    2~2.5       move decrease 20  index
                    { 30,  800,   -11}, //    2.5~3      move decrease 30  index
                }
            },
            { //AEFaceMoveTable
                {
                    {-20,   17,    20}, //   mean below -2.5  move increase 25 index
                    {-20,   25,    15}, //   -2.5~-2  move increase 20 index
                    {-15,   33,    10}, //   -2~-1.6
                    {-15,   40,    6}, //   -1.6~-1.3
                    {-10,   50,    4}, //   -1.3~-1
                    { -8,   57,     3}, //   -1~-0.8
                    { -5,   71,     2}, //   -0.8~-0.5
                    { -4,   75,     1}, //   -0.5~-0.4
                    { -3,   81,     1}, //   -0.4~-0.3
                    { -1,   90,     1}, //   -0.3~-0.1
                    {   0,  100,     0}, //   -0.1~0
                    {   1,  110,     0}, //     0~0.1
                    {   2,  114,    -1}, //    0.1~0.2       move decrease 1 index
                    {   3,  123,    -1}, //    0.2~0.3
                    {   4,  131,    -1}, //    0.3~0.4
                    {   5,  141,    -2}, //    0.4~0.5
                    {   7,  162,    -2}, //    0.5~0.7
                    {   9,  186,    -3}, //    0.7~0.9
                    { 10,  200,   -4}, //    0.9~1.0
                    { 13,  246,   -4}, //    1.0~1.3
                    { 16,  303,   -6}, //    1.3~1.6
                    { 20,  400,   -7}, //    1.6~2       move decrease 10  index
                    { 25,  566,   -9}, //    2~2.5       move decrease 20  index
                    { 30,  800,   -11}, //    2.5~3      move decrease 30  index
                }
            },
            { //AETrackingMoveTable
                {
                    {-20,   17,    20}, //   mean below -2.5  move increase 25 index
                    {-20,   25,    15}, //   -2.5~-2  move increase 20 index
                    {-15,   33,    10}, //   -2~-1.6
                    {-15,   40,    6}, //   -1.6~-1.3
                    {-10,   50,    4}, //   -1.3~-1
                    { -8,   57,     3}, //   -1~-0.8
                    { -5,   71,     2}, //   -0.8~-0.5
                    { -4,   75,     1}, //   -0.5~-0.4
                    { -3,   81,     1}, //   -0.4~-0.3
                    { -1,   90,     1}, //   -0.3~-0.1
                    {   0,  100,     0}, //   -0.1~0
                    {   1,  110,     0}, //     0~0.1
                    {   2,  114,    -1}, //    0.1~0.2       move decrease 1 index
                    {   3,  123,    -1}, //    0.2~0.3
                    {   4,  131,    -1}, //    0.3~0.4
                    {   5,  141,    -2}, //    0.4~0.5
                    {   7,  162,    -2}, //    0.5~0.7
                    {   9,  186,    -3}, //    0.7~0.9
                    { 10,  200,   -4}, //    0.9~1.0
                    { 13,  246,   -4}, //    1.0~1.3
                    { 16,  303,   -6}, //    1.3~1.6
                    { 20,  400,   -7}, //    1.6~2       move decrease 10  index
                    { 25,  566,   -9}, //    2~2.5       move decrease 20  index
                    { 30,  800,   -11}, //    2.5~3      move decrease 30  index
                }
            },
            {   //AELimiterDataTable for AE limiter
                2,    //  iLEVEL1_GAIN
                10,  //  iLEVEL2_GAIN
                18,  //  iLEVEL3_GAIN
                28,  //  iLEVEL4_GAIN
                40,  //  iLEVEL5_GAIN
                50,  //  iLEVEL6_GAIN
                0,    //  iLEVEL1_TARGET_DIFFERENCE
                4,    //  iLEVEL2_TARGET_DIFFERENCE
                7,    //  iLEVEL3_TARGET_DIFFERENCE
                7,    //  iLEVEL4_TARGET_DIFFERENCE
                7,    //  iLEVEL5_TARGET_DIFFERENCE
                7,    //  iLEVEL6_TARGET_DIFFERENCE
                2,    //  iLEVEL1_GAINH
                -2,  //  iLEVEL1_GAINL
                6,    //  iLEVEL2_GAINH
                -7,  //  iLEVEL2_GAINL
                8,    // iLEVEL3_GAINH
                -9,  //  iLEVEL3_GAINL
                10,  //  iLEVEL4_GAINH
                -12, // iLEVEL4_GAINL
                12,   // iLEVEL5_GAINH
                -16, // iLEVEL5_GAINL
                15,   // iLEVEL6_GAINH
                -20, // iLEVEL6_GAINL
                1,     // iGAIN_DIFFERENCE_LIMITER
            },
            {  //VdoDynamicFpsTable for video dynamic frame rate
                MTRUE, // isEnableDFps
                50,  // EVThresNormal
                50,  // EVThresNight
            },   
            {//AEBlockHistWINCFG
                0,                      // uHist0StartBlockXRatio : Histogram 0 window config start block X ratio (0~100)
                100,                   // uHist0EndBlockXRatio : Histogram 0 window config end block X ratio (0~100)
                0,                      // uHist0StartBlockYRatio : Histogram 0 window config start block Y ratio (0~100)
                100,                   // uHist0EndBlockYRatio : Histogram 0 window config end block Y ratio (0~100)
                3,                      // uHist0OutputMode : Histogram 0 output source mode
                0,                      // uHist0BinMode : Histogram 0 bin mode range
                0,                      // uHist1StartBlockXRatio : Histogram 1 window config start block X ratio (0~100)
                100,                   // uHist1EndBlockXRatio : Histogram 1 window config end block X ratio (0~100)
                0,                      // uHist1StartBlockYRatio : Histogram 1 window config start block Y ratio (0~100)
                100,                   // uHist1EndBlockYRatio : Histogram 1 window config end block Y ratio (0~100)
                3,                      // uHist1OutputMode : Histogram 1 output source mode
                0,                      // uHist1BinMode : Histogram 1 bin mode range
                0,                      // uHist2StartBlockXRatio : Histogram 2 window config start block X ratio (0~100)
                100,                   // uHist2EndBlockXRatio : Histogram 2 window config end block X ratio (0~100)
                0,                      // uHist2StartBlockYRatio : Histogram 2 window config start block Y ratio (0~100)
                100,                   // uHist2EndBlockYRatio : Histogram 2 window config end block Y ratio (0~100)
                4,                      // uHist2OutputMode : Histogram 2 output source mode
                0,                      // uHist2BinMode : Histogram 2 bin mode range
                25,                     // uHist3StartBlockXRatio : Histogram 3 window config start block X ratio (0~100)
                75,                    // uHist3EndBlockXRatio : Histogram 3 window config end block X ratio (0~100)
                25,                     // uHist3StartBlockYRatio : Histogram 3 window config start block Y ratio (0~100)
                75,                    // uHist3EndBlockYRatio : Histogram 3 window config end block Y ratio (0~100)
                4,                      // uHist3OutputMode : Histogram 3 output source mode
                0,                      // uHist3BinMode : Histogram 3 bin mode range
    
                // No use, just for build pass
                25,//25 for isp5.0 CY   // uHist4StartBlockXRatio : Histogram 4 window config start block X ratio (0~100)
                75,//75 for isp5.0 CY   // uHist4EndBlockXRatio : Histogram 4 window config end block X ratio (0~100)
                25,//25 for isp5.0 CY   // uHist4StartBlockYRatio : Histogram 4 window config start block Y ratio (0~100)
                75,//75 for isp5.0 CY   // uHist4EndBlockYRatio : Histogram 4 window config end block Y ratio (0~100)
                4,//4 for isp5.0 CY     // uHist4OutputMode : Histogram 4 output source mode
                1,//1 for isp5.0 256bin // uHist4BinMode : Histogram 4 bin mode range
    
                25,//reserved           // uHist5StartBlockXRatio : Histogram 5 window config start block X ratio (0~100)
                75,                     // uHist5EndBlockXRatio : Histogram 5 window config end block X ratio (0~100)
                25,                     // uHist5StartBlockYRatio : Histogram 5 window config start block Y ratio (0~100)
                75,                     // uHist5EndBlockYRatio : Histogram 5 window config end block Y ratio (0~100)
                4,                      // uHist5OutputMode : Histogram 5 output source mode
                1,                      // uHist5BinMode : Histogram 5 bin mode range
            },
            {//AEPixelHistWINCFG
                0,                      // uHist0StartBlockXRatio : Histogram 0 window config start block X ratio (0~100)
                100,                    // uHist0EndBlockXRatio : Histogram 0 window config end block X ratio (0~100)
                0,                      // uHist0StartBlockYRatio : Histogram 0 window config start block Y ratio (0~100)
                100,                    // uHist0EndBlockYRatio : Histogram 0 window config end block Y ratio (0~100)
                3,//0 for isp6.0 RGB    // uHist0OutputMode : Histogram 0 output source mode
                1,//1 for isp6.0 256bin // uHist0BinMode : Histogram 0 bin mode range
    
                0,                      // uHist1StartBlockXRatio : Histogram 1 window config start block X ratio (0~100)
                100,                    // uHist1EndBlockXRatio : Histogram 1 window config end block X ratio (0~100)
                0,                      // uHist1StartBlockYRatio : Histogram 1 window config start block Y ratio (0~100)
                100,                    // uHist1EndBlockYRatio : Histogram 1 window config end block Y ratio (0~100)
                0,//1 for isp6.0 R      // uHist1OutputMode : Histogram 1 output source mode
                1,//1 for isp6.0 256bin // uHist1BinMode : Histogram 1 bin mode range
    
                0,                      // uHist2StartBlockXRatio : Histogram 2 window config start block X ratio (0~100)
                100,                    // uHist2EndBlockXRatio : Histogram 2 window config end block X ratio (0~100)
                0,                      // uHist2StartBlockYRatio : Histogram 2 window config start block Y ratio (0~100)
                100,                    // uHist2EndBlockYRatio : Histogram 2 window config end block Y ratio (0~100)
                1,//2 for isp6.0 G      // uHist2OutputMode : Histogram 2 output source mode
                1,//1 for isp6.0 256bin // uHist2BinMode : Histogram 2 bin mode range
    
                0,                      // uHist3StartBlockXRatio : Histogram 3 window config start block X ratio (0~100)
                100,                    // uHist3EndBlockXRatio : Histogram 3 window config end block X ratio (0~100)
                0,                      // uHist3StartBlockYRatio : Histogram 3 window config start block Y ratio (0~100)
                100,                    // uHist3EndBlockYRatio : Histogram 3 window config end block Y ratio (0~100)
                2,//3 for isp6.0 B      // uHist3OutputMode : Histogram 3 output source mode
                1,//1 for isp6.0 256bin // uHist3BinMode : Histogram 3 bin mode range
    
                0,                      // uHist4StartBlockXRatio : Histogram 4 window config start block X ratio (0~100)
                100,                    // uHist4EndBlockXRatio : Histogram 4 window config end block X ratio (0~100)
                0,                      // uHist4StartBlockYRatio : Histogram 4 window config start block Y ratio (0~100)
                100,                    // uHist4EndBlockYRatio : Histogram 4 window config end block Y ratio (0~100)
                4,//4 for isp6.0 Y      // uHist4OutputMode : Histogram 4 output source mode
                1,//1 for isp6.0 256bin // uHist4BinMode : Histogram 4 bin mode range
    
                25,                     // uHist5StartBlockXRatio : Histogram 5 window config start block X ratio (0~100)
                75,                     // uHist5EndBlockXRatio : Histogram 5 window config end block X ratio (0~100)
                25,                     // uHist5StartBlockYRatio : Histogram 5 window config start block Y ratio (0~100)
                75,                     // uHist5EndBlockYRatio : Histogram 5 window config end block Y ratio (0~100)
                4,//4 for isp6.0 CY     // uHist5OutputMode : Histogram 5 output source mode
                1,//1 for isp6.0 256bin // uHist5BinMode : Histogram 5 bin mode range
            },
            {//PSPixelHistWINCFG
                0,                      // uHistStartBlockXRatio : Histogram window config start block X ratio (0~100)
                100,                    // uHistEndBlockXRatio : Histogram window config end block X ratio (0~100)
                0,                      // uHistStartBlockYRatio : Histogram window config start block Y ratio (0~100)
                100,                    // uHistEndBlockYRatio : Histogram window config end block Y ratio (0~100)
            },
            // v1.2
            MFALSE,         // bOldAESmooth
            MTRUE,         // bEnableSubPreIndex
            0,         // u4VideoLPFWeight; // 0~23   
            { //PerframeCFG
                200,
                80,
                80,
                240,
                4,
                254,
                510,
                1,
                MFALSE,
            },
            MFALSE,             // Perframe AE smooth option
            MFALSE,             // Perframe HDR AE smooth option
    
            MFALSE,             // TG interrupt option
            MFALSE, //Speedup Escape Overexposure
            MFALSE, //Speedup Escape Underexposure
            {//TgIntTuningParam
                //B2T
                252,
                70,
                200,
    
                {80, 300, 500},  // STD = 1, 2, 3
                {100, 85, 60, 40},  // 100%, 85%, 60%, 50%
    
    
                {30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
                {-30, -24, -20, -10},
    
                200,
                165, //OverExpoLowBound
                24,  //OverExpLightAcc1
                20,  //OverExpLightAcc2
                18,  //OverExpLightAcc3
                16,  //OverExpLightAcc4
    
                //D2T
                7, //245
                80, //80%
                20,
    
                {80, 250, 500},  // STD = 1, 2, 3
                {100, 85, 60, 35},  // 100%, 85%, 60%, 50%
    
                {30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
                {20, 15, 10, 10},
    
                20
            },
            {//TgEVCompJumpRatioOverExp[LIB3A_AE_EV_COMP_MAX]
                100,  // EV   0
                0,   //EV    0.1
                0,   //EV    0.2
                0,   //EV    0.3
                0,   //EV    0.4
                0,   //EV    0.5
                0,   //EV    0.6
                0,   //EV    0.7
                0,   //EV    0.8
                0,   //EV    0.9
                0,   //EV    1
                0,   //EV    1.1
                0,   //EV    1.2
                0,   //EV    1.3
                0,   //EV    1.4
                0,   //EV    1.5
                0,   //EV    1.6
                0,   //EV    1.7
                0,   //EV    1.8
                0,   //EV    1.9
                0,   //EV    2
                0,   //EV    2.1
                0,   //EV    2.2
                0,   //EV    2.3
                0,   //EV    2.4
                0,   //EV    2.5
                0,   //EV    2.6
                0,   //EV    2.7
                0,   //EV    2.8
                0,   //EV    2.9
                0,   //EV    3
                0,   //EV    3.1
                0,   //EV    3.2
                0,  //EV    3.3
                0,  //EV    3.4
                0,  //EV    3.5
                0,  //EV    3.6
                0,  //EV    3.7
                0,  //EV    3.8
                0,  //EV    3.9
                0,  //EV    4
                100,    //EV    -0.1
                100,    //EV    -0.2
                100,    //EV    -0.3
                100,    //EV    -0.4
                100,    //EV    -0.5
                110,    //EV    -0.6
                110,    //EV    -0.7
                110,    //EV    -0.8
                110,    //EV    -0.9
                120,    //EV    -1
                120,    //EV    -1.1
                120,    //EV    -1.2
                130,    //EV    -1.3
                130,    //EV    -1.4
                140,    //EV    -1.5
                140,    //EV    -1.6
                140,    //EV    -1.7
                150,    //EV    -1.8
                150,    //EV    -1.9
                170,    //EV    -2
                170,    //EV    -2.1
                170,    //EV    -2.2
                170,    //EV    -2.3
                170,    //EV    -2.4
                170,    //EV    -2.5
                170,    //EV    -2.6
                170,    //EV    -2.7
                170,    //EV    -2.8
                170,    //EV    -2.9
                200,    //EV    -3
                200,    //EV    -3.1
                200,    //EV    -3.2
                200,    //EV    -3.3
                200, //EV    -3.4
                200, //EV    -3.5
                200, //EV    -3.6
                200, //EV    -3.7
                200, //EV    -3.8
                200, //EV    -3.9
                200, //EV    -4
            },
            {//TgEVCompJumpRatioUnderExp[LIB3A_AE_EV_COMP_MAX]
                100,//100,  // EV   0
                100,//100,   //EV    0.1
                100,//100,   //EV    0.2
                100,//100,   //EV    0.3
                100,//100,   //EV    0.4
                100,//100,   //EV    0.5
                100,//100,   //EV    0.6
                100,//100,   //EV    0.7
                100,//100,   //EV    0.8
                100,//100,   //EV    0.9
                100,//110,   //EV    1
                100,//110,   //EV    1.1
                100,//110,   //EV    1.2
                100,//110,   //EV    1.3
                100,//110,   //EV    1.4
                100,//110,   //EV    1.5
                100,//110,   //EV    1.6
                100,//110,   //EV    1.7
                100,//110,   //EV    1.8
                100,//110,   //EV    1.9
                100,//120,   //EV    2
                100,//120,   //EV    2.1
                100,//120,   //EV    2.2
                100,//120,   //EV    2.3
                100,//120,   //EV    2.4
                100,//120,   //EV    2.5
                100,//120,   //EV    2.6
                100,//120,   //EV    2.7
                100,//120,   //EV    2.8
                100,//120,   //EV    2.9
                100,//130,   //EV    3
                100,//130,   //EV    3.1
                100,//130,   //EV    3.2
                100,//130,  //EV    3.3
                100,//130,  //EV    3.4
                100,//130,  //EV    3.5
                100,//130,  //EV    3.6
                100,//130,  //EV    3.7
                100,//130,  //EV    3.8
                100,//130,  //EV    3.9
                100,//130,  //EV    4
                0,    //EV    -0.1
                0,    //EV    -0.2
                0,    //EV    -0.3
                0,    //EV    -0.4
                0,    //EV    -0.5
                0,    //EV    -0.6
                0,    //EV    -0.7
                0,    //EV    -0.8
                0,    //EV    -0.9
                0,    //EV    -1
                0,    //EV    -1.1
                0,    //EV    -1.2
                0,    //EV    -1.3
                0,    //EV    -1.4
                0,    //EV    -1.5
                0,    //EV    -1.6
                0,    //EV    -1.7
                0,    //EV    -1.8
                0,    //EV    -1.9
                0,    //EV    -2
                0,    //EV    -2.1
                0,    //EV    -2.2
                0,    //EV    -2.3
                0,    //EV    -2.4
                0,    //EV    -2.5
                0,    //EV    -2.6
                0,    //EV    -2.7
                0,    //EV    -2.8
                0,    //EV    -2.9
                0,    //EV    -3
                0,    //EV    -3.1
                0,    //EV    -3.2
                0,    //EV    -3.3
                0, //EV    -3.4
                0, //EV    -3.5
                0, //EV    -3.6
                0, //EV    -3.7
                0, //EV    -3.8
                0, //EV    -3.9
                0, //EV    -4       
            },
            {
                {55, 55, 69, 83, 83, 83, 83, 83, 83, 69, 55, 55},
                {69, 69, 86, 104, 104, 113, 143, 104, 104, 86, 69, 69},
                {83, 83, 104, 125, 125, 143, 143, 125, 125, 104, 83, 83},
                {83, 83, 123, 134, 134, 143, 143, 134, 134, 123, 83, 83},
                {83, 83, 143, 143, 143, 143, 143, 143, 143, 143, 83, 83},
                {83, 83, 123, 134, 134, 143, 143, 134, 134, 123, 83, 83},
                {83, 83, 104, 125, 125, 143, 143, 125, 125, 104, 83, 83},
                {69, 69, 86, 104, 104, 113, 143, 104, 104, 86, 69, 69},
                {55, 55, 69, 83, 83, 83, 83, 83, 83, 69, 55, 55},
            },  
            {//AEStableThd
                0,                      // u4InStableThd;  // 0.08EV
                1,                      // u4OutStableThd
    
                MTRUE,             // enable ae different mode stable threshold setting
                3,                      // u4VideoInStableThd;  // 0.08EV
                12,                      // u4VideoOutStableThd
                3,                      // u4FaceInStableThd;  // 0.08EV
                3,                      // u4FaceOutStableThd
                6,                     // u4FaceOutB2TStableThd
                6,                     // u4FaceOutD2TStableThd
                0,                      // u4TouchInStableThd;  // 0.08EV
                3,                      // u4TouchOutStableThd
                MFALSE,                  // Face Ae lock option
                MFALSE,                   // enable zero stable thd
                MTRUE,                  //bEnableFaceStableTolerance
                4,                      //u4FaceStableTolCnt
               {-2000, 0, 3000,5500},   //u4FaceStableBVThd
               {1,1,1,1},               //u4FaceStableCWVDiffThd
               {2,2,2,2},               //u4FaceStableFDYDiffThd
                99,                     //u4FaceStableOutThd
            },  
            { //AEBVAccRatio
                MFALSE,
                90,  //i4DeltaBVRatio
                220, //u4B2T_Target
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    255, //u4Bright2TargetEnd
                    20, //u4Dark2TargetStart
                    200, //u4B2TEnd
                    100, //u4B2TStart
                    40, //u4D2TEnd
                    90, //u4D2TStart
                },
            },
            {  //AEPsoConverge
                MFALSE,
                100, //Speed control by PSO under wash-out (100 means 100%, fastest)
                100,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
                4000,
                -4000
            }, 
            {//AENonCWRAcc
                MFALSE,          //bNonCWRAccEnable enable
                255,
                170,
                250,            //it should  be <=hs_THD
                58,             //it should keep in target~hs_flat
            },
            {//AEFaceSmooth
                235,                  // tempFDY HighBound
                  1,                  // tempFDY LowBound
                 0,                  // face lost max count
                100,                  // face window scale ratio range 1~400% default 90%
                  1,                  // u4FD_Lock_MaxCnt
                  0,                  // u4FD_TemporalSmoothY
             MFALSE,                  // bFD_FaceSizeExt
                 10,                  // i4FD_FaceXMiniBnd
                 10,                  // i4FD_FaceYMiniBnd
                 70,                  // u4FD_FaceTopCropRat
                100,                  // u4FD_FaceBottomCropRat
                100,                  // u4FD_FaceLeftCropRat
                100,                  // u4FD_FaceRightCropRat
                  1,                  // u4FD_InToOutThdMaxCnt
                255,                  // u4FD_OTLockUpperBnd
                  0,                  // u4FD_OTLockLowerBnd
                  1,                  // u4FD_ContinueTrustCnt
                  0,                  // u4FD_PerframeAntiOverFlag
                  0,                  // u4FD_SideFaceLock
                  0,                  // u4FD_LimitStableThdLowBnd
                  0,                  // u4FD_LimitStableThdLowBndNum
                  0,                  // u4FD_ReConvergeWhenFaceChange
                  0,                 // u4FD_FaceMotionLockRat
                  3,                  // u4FD_ImpulseLockCnt
                  2,                  // u4FD_ImpulseUpperThd
                  2,                  // u4FD_ImpulseLowerThd
                  0,                  // u4FD_ROPTrustCnt
                 30,                  // u4FD_ReCovergeWhenSizeChangeRat
                120,                  // u4FD_ReCovergeWhenPosChangeDist
                3000,                  // u4FD_ReCovergeWhenRealBVx1000DiffLarge
                  1,                  // u4FD_ReCovergeWhenAFDone;
                  0,                  // u4FD_OTFaceLock
                MFALSE,                  // bCnnFaceEnable
                MFALSE,                  // bReConvergeWhenCNNFaceBack
                  4,                  // i4PortraitTolerance
                  0,                  // i4FD_CNN_Lock_MaxCnt
                MFALSE,                  // bEVCompExceptionEnable
                10000,                  // u4EVCompTouchMeteringStableMax
                  1,                  // u4EVCompTouchMeteringStableMin
                 20,                  // u4EVCompTouchOutStableThd
                 15,                  // u4EVCompTouchInStableThd
                MFALSE,                  // bFaceAccLockEnable
                100,                  // u4FaceAccLockThd
                  0,                  // u4FaceAccLock_MaxCnt
                MFALSE,                  // bFaceGyroDiffLockEnable
                100,                  // u4FaceGyroDiffLockThd
                  0                   // u4FaceGyroDiffLock_MaxCnt
            },
            {//AEFaceWeight
                MTRUE,
                FACE_PRIOR_TIME,
                {
                    1,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0
                }
            },
            { //AECWRTS
                MFALSE,                                // enable AE HS Target temp smooth
                MFALSE,                                // enable AE ABL Target temp smooth
                MFALSE,                                // enable AE AOE Target temp smooth
                MFALSE,                                // enable AE NS Target temp smooth
                MFALSE,                                // enable AE Face Target temp smooth
                MFALSE,                                // enable AE Video Target temp smooth
                MFALSE,                                // enable AE Main Target temp smooth
                MFALSE,                               // enable AE HSv4p0 Target temp smooth
            },
            { //AEFaceLandmark
                MFALSE,                //bLandmarkCtrlFlag
                30,                    //u4LandmarkCV_Thd
               -60,                   //i4LandmarkCV_BV
                30,                    //u4LandmarkROP_Thd
               -60,                   //i4LandmarkROP_BV
                4,                    //u4LandmarkWeight
              100,                    //i4LandmarkExtRatW;
              100,                    //i4LandmarkExtRatH;
               90,                    //i4LandmarkTrustRopDegree;
                5,                    //i4RoundXYPercent;
                3,                    //i4RoundSZPercent;
                9,                    //i4THOverlap;
                3,                    //i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
                8,                    //i4SMOOTH_LEVEL;
                0,                    //i4MOMENTUM;
                60,                    //u4LandmarkWeightPercent;
                MFALSE,                //u4LandmarkBKMode;
                40,                   //u4LandmarkBK_CVthd;
                20,                   //u4LandmarkBK_FYthd;
                70,                   //u4LandmarkBK_Weight;
                40,                   //i4LandmarkBK_BV;
            },
            {//strAOEByBV
                MTRUE,                                       // bEnable
                4,                                               // u4TblLength
                {-2000, 0, 3000,5500},         // i4BV
                {150, 170, 180, 180}            // u4THD
            },
            {//strNSCDF
                MTRUE,                                       // bEnable
                700,
                {-2200, -1700},
                {1024,   0},
            },
            {//AETouchMovingRatio
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190, //u4Bright2TargetEnd
                20,   //u4Dark2TargetStart
                80, //u4B2TEnd
                60,  //u4B2TStart
                40,  //u4D2TEnd
                80,  //u4D2TStart
            },
            {//HSbyRealBV
                MTRUE,                   //bEnableFlatSkyEnhance
                2,                          //uSkyDetInThd  : 2%
                5,                          //uSkyDetOutThd : 5%
                160,                    //u4FlatSkyTHD
                1500,                   //u4FlatSkyEVD
                {8000,  11000},         //u4BVRatio_X : BV
                {0, 1024}               //u4BVRatio_Y : Ratio
            },
            {//ABLabsDiff
                MTRUE,                           //enable enhance ABL(absDiff)
                {2500,  5000},            //u4EVDiffRatio_X
                {0, 1024},                    //u4EVDiffRatio_Y
                {9000,  11000},         //u4BVRatio_X
                {0, 1024}                     //u4BVRatio_Y
            },   
            {//HistStableCFG
                MFALSE,                                            // bEnableStablebyHist
                250,                                                //u4HistEVDiff : 0.25EV
                242,                                                //u4OverexpoTHD  : 95%
                13,                                                 //u4UnderexpoTHD : 5%
                140,                                                //u4HistStableTHD : 15%(1024base)
            },
            {//strOverExpoAOECFG
                MTRUE,                                              // bEnableOverExpoAOE
                MTRUE,                                              //bEnableHistOverExpoCnt
                MTRUE,                                              //bEnableLTMOE_Link
                MTRUE,                                              //bEnableLTMOE_Decrease;
                  1.0,                                               //fLtm_Cap_Weight; 
                 16,                                                //u4OElevel : pseudo-high strength 6x
                 2,                                                 //u4OEHboundTblLength
                {40,80},                                            //u4OE_Ratio_Hbound_X[u4OEHboundTblLength]
                {512,768},                                          //u4OE_Ratio_Hbound_Y[u4OEHboundTblLength]             
                1,                                                  //u4OERatio_LowBnd : 2% over exposure area
                15,                                                 //u4OERatio_HighBnd : 20% overexposure area
                {1000,  4600},                                   //i4OE_BVRatio_X[2]
                {800,   1024},                                     //u4OE_BVRatio_Y[2]
                5,                                                  //u4OETblLength
                {20,30,40,85,108},                                  //u4OE_Ratio_X[u4OETblLength]
                {128,256,512,896,1024},                             //u4OE_Ratio_Y[u4OETblLength]        
                {                                                   //u4OEWeight[15][15] : Spatial OE weight (avg mode)
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                }
            },
            //v4.0
            MTRUE,              //bAEv4p0MeterEnable;
            {//MainTargetCFG
                MTRUE,               //bEnableAEMainTarget
                1024,               //u4MainTargetWeight
                {                         //u4MainTargetWeightTbl[15][15] : Gau weight table
                    {0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 43, 50,  57,  65,  68,  71,  74,  71,  68,  65,  57, 50, 43, 0},
                    {0, 50, 58,  67,  76,  79,  83,  86,  83,  79,  76,  67, 58, 50, 0},
                    {0, 57, 67,  77,  87,  91,  95,  99,  95,  91,  87,  77, 67, 57, 0},
                    {0, 65, 76,  87,  98, 102, 107, 112, 107, 102,  98,  87, 76, 65, 0},
                    {0, 68, 79,  91, 102, 107, 112, 117, 112, 107, 102,  91, 79, 68, 0},
                    {0, 71, 83,  95, 107, 112, 117, 122, 117, 112, 107,  95, 83, 71, 0},
                    {0, 74, 86,  99, 112, 117, 122, 128, 122, 117, 112,  99, 86, 74, 0},
                    {0, 71, 83,  95, 107, 112, 117, 122, 117, 112, 107,  95, 83, 71, 0},
                    {0, 68, 79,  91, 102, 107, 112, 117, 112, 107, 102,  91, 79, 68, 0},
                    {0, 65, 76,  87,  98, 102, 107, 112, 107, 102,  98,  87, 76, 65, 0},
                    {0, 57, 67,  77,  87,  91,  95,  99,  95,  91,  87,  77, 67, 57, 0},
                    {0, 50, 58,  67,  76,  79,  83,  86,  83,  79,  76,  67, 58, 50, 0},
                    {0, 43, 50,  57,  65,  68,  71,  74,  71,  68,  65,  57, 50, 43, 0},
                    {0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                },
                {   // TargetBVRatioTbl
                    -2000,                   // TargetBVRatioTbl.i4X1 :BV
                    55,                          // TargetBVRatioTbl.i4Y1 :Target
                    8000,                   // TargetBVRatioTbl.i4X2  :BV
                    66                          // TargetBVRatioTbl.i4Y2 :Target
                },
                MTRUE,                   //bEnableColorWTRatio
                {   //ColorWTRatioTbl
                    0,                            // ColorWTRatioTbl.i4X1 :ColorDiff
                    600,                          // ColorWTRatioTbl.i4Y1 :u4MainTargetWeight
                    100,                        // ColorWTRatioTbl.i4X2  :ColorDiff
                    1024                          // ColorWTRatioTbl.i4Y2 :u4MainTargetWeight / 2
                },
                100,                           //uColorSupRatioR : 0~100
                100,                           //uColorSupRatioG : 0~100
                0,                               //uColorSupRatioB : 0~100
                {   //ColorSupBVRatioTbl
                    1000,                            // ColorSupBVRatioTbl.i4X1 :BV
                    700,                          // ColorSupBVRatioTbl.i4Y1 :Ratio
                    6000,                        // ColorSupBVRatioTbl.i4X2  :BV
                    1024                          // ColorSupBVRatioTbl.i4Y2 :Ratio
                },
            },
            {//HSv4p0CFG
                TRUE,                   //bEnableHSv4p0
                1024,                   //u4HSv4p0Weight
                6,                          //u4BVSize ,max16
                7,                          //u4EVDSize ,max16
                {-500, 500, 2500, 5000, 8000, 10000},     //i4HS_BVRatio, length = u4BVSize
                {50, 50, 45, 30, 20, 20},    //u4HS_PcntRatio, length = u4BVSize, 80 equal 8%
                {500, 1100, 2500, 4800, 5800, 6300, 7000},    //u4HS_EVDRatio , length = u4EVDSize
                {                                       //u4HSTHDRatioTbl[u4BVSize][u4EVDSize] , HS THD
                   //BV0-THD
                   {160, 170, 190, 190, 190, 190, 190},
                   //BV1-THD
                   {160, 190, 205, 195, 230, 230, 215},
                   //BV2-THD
                   {150, 210, 225, 230, 240, 240, 230},
                   //BV3-THD
                   {150, 200, 220, 230, 240, 240, 240},
                   //BV4-THD
                   {150, 210, 230, 235, 240, 245, 210},
                   //BV5-THD
                   {150, 235, 230, 230, 225, 230, 200},
                },
                TRUE,                   //bEnableDynWTRatio
                {       //DynWTRatioTbl
                    0,                            // DynWTRatioTbl.i4X1 : min thd
                    0,                              // DynWTRatioTbl.i4Y1 :add delta weight 1024base
                    30,                         // DynWTRatioTbl.i4X2  :max thd
                    500,                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
                },
            }, 
            {//ColorSuppression
                {//GreenSuppression
                    TRUE, //   bEnableGreenSupress;
                    90,   // u4GreenDetectGRRatio;       //  Green Detect G/R Ratio
                    120,  // u4GreenDetectGBRatio;       //  Green Detect G/B Ratio
                    {0,1024},// u4GreenRatio[2];           // Green Detect count weighting TBL X
                    {0, 1024 }, // u4GreenProb[2];            // Green Detect count weighting TBL Y
                    {1000, 6000},// u4GreenBv[2];              // Green Detect BV range
                    {700, 1024},// u4GreenBvProb[2];            // Green Detect BV Prob
                    10,    //u4GreenDetectMin;      // HS4p0 green supress Transition range  HS to Supression 
                    130,   // u4Hs4p0SupRatio;           // HS4p0 green supress stregth ratio
                    120,// u4MainTargetSupRatio;      // MainTarget green supress stregth ratio
                    {0},// u4TH[6];                  // Reserve for HWTH
                },
            },
            { //NSBVCFG
                TRUE,   //bEnableNSBVCFG
                4,      //u4TblLength
               {-4500, -3000, -1000, 0},       //i4BV[16]
               {70, 70, 70, 75},   //u4BTTHD[16]
               { 140, 150, 190, 195},   //u4NSTHD[16]
            },
            { //AEACCMovingRatio
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                255, //u4Bright2TargetEnd
                20,   //u4Dark2TargetStart
                100, //u4B2TEnd
                100,  //u4B2TStart
                100,  //u4D2TEnd
                100,  //u4D2TStart
            },
            {//FaceLocSizeCheck
                TRUE,          //bFaceLocSizeCheck;
                { 3, 18 },      // u4SizeSmall2Big[2];
                { 380, 700 },    //u4ProbSizeSmall2Big[2];
                { 200, 600 },   // u4Loc_Near2Far[2];
                { 1024, 1024 }, //u4ProbLocNear2Far[2];
                FALSE,          //bLandmarkSize;
                false,           //bFaceLocSizeByBv;
                4,               //Max size 16
                {2000, 4000, 6000,8000},   // i4Bv[AE_BVDR_MAXSIZE]
                //Size Prob change along Bv
                {380, 480, 480 ,380},  //ProbSizeSmallByBv[AE_BVDR_MAXSIZE];
                {700, 700, 700,700},  //ProbSizeBigByBv[AE_BVDR_MAXSIZE];
                //Loc Prob change along Bv
                {1024,1024,1024,1024}, //u4ProbLocNearByBv[AE_BVDR_MAXSIZE];
                {1024,1024,1024,1024}, //u4ProbLocFarByBv[AE_BVDR_MAXSIZE];
                {0}, //Reserved
            },
            {
                10,     // u4FdCwrLowBound;        // metering and face boundary min Y value
                1600,   // u4FdCwrHighBound;      // metering and face boundary max Y value
                -1000, // i4FaceBVLowBound;     // face boundary min BV value
                4000, // i4FaceBVHighBound;    // face boundary max BV value
                3, //uFaceCentralWeight;
                56, //uFaceYLowBound;
                60, //uFaceYHighBound;
                49, //face NS lowbound
            },
            { //FaceBalancedTargetCFG
                TRUE,	//bEnableFBT
                TRUE,  //bFloatingPcentDR
                { // Normal Face 
                    2,      //BV Table, max 16
                    4,      //DR index Table size, max 16
                    { -1000, 4000 , 6000, 7000 },  // Parameter index BV
                    { 10, 10 ,10,10 }, // Tolerance for overexposure
                    { 20, 20 ,20,20 }, // OE region behind tolerance region
                    { -500,0,3000,4000 }, // Parameter index DR
                    {// u4_OE_TH_Tbl: OE brightness target
                        { 250,250,250,250},  // BV0
                        { 250,250,250,250}, // BV1
                        { 250,250,250,250 },  // BV2
                        { 250,250,250,250 }, // BV3
                    },
                    { //u4_FD_TH: FD brightness target
                        { 48,60,60,60 }, // BV0
                        { 52,60,60,60 }, // BV1
                        { 48,58,58,58 }, // BV2
                        { 52,52,52,52 }, // BV3
                    },
                    { //u4_FD_MIN_TH: FD brightness target
                        { 48,60,60,60 }, // BV0
                        { 52,60,60,60 }, // BV1
                        { 48,58,58,58 }, // BV2
                        { 52,52,52,52 }, // BV3
                    },
                },
                { // Night Scene Face
                    2,      //BV Table, max 16
                    4,      //DR index Table size, max 16
                    { -1000, 4000 , 6000, 7000 },  // Parameter index BV
                    { 10, 10 ,10,10 }, // Tolerance for overexposure
                    { 20, 20 ,20,20 }, // OE region behind tolerance region
                    { -500,0,3000,4000 }, // Parameter index DR
                    {// u4_OE_TH_Tbl: OE brightness target
                        { 250,250,250,250 }, // BV0
                        { 250,250,250,250 }, // BV1
                        { 250,250,250,250 }, // BV2
                        { 250,250,250,250 }, // BV3
                    },
                    { //u4_FD_TH: FD brightness target
                        { 48,60,60,60 }, // BV0
                        { 52,60,60,60 }, // BV1
                        { 48,58,58,58 }, // BV2
                        { 52,52,52,52 }, // BV3
                    },
                    { //u4_FD_MIN_TH: FD brightness target
                        { 48,60,60,60 }, // BV0
                        { 52,60,60,60 }, // BV1
                        { 48,58,58,58 }, // BV2
                        { 52,52,52,52 }, // BV3
                    },
                },
            },
            {
                25, //u4FaceSortRa
                5,  //u4FaceSortStbCnt
            },
            {
                true, //bEnableFaceMotionDetect
                25, //u4FaceMotionRa_FDIN
                3,  //u4FaceMotionRa_FDCONTINUE
                3,  //u4FDMotionFrame_FDIN
                5,  //u4FDMotionFrame_FDCONTINUE
            },
            {
                true, //bEnableFaceSizeDetect
                3, //u4FaceSizeDiffTh_W_FDIN
                3, //u4FaceSizeDiffTh_H_FDIN
                3,  //u4FaceSizeDiffTh_W_FDCONTINUE
                3,  //u4FaceSizeDiffTh_H_FDCONTINUE
                3,  //u4FDSizeDiffFrame_FDIN
                5,  //u4FDSizeDiffFrame_FDCONTINUE
                true,//bEnableFaceBndLock
                0,  //u4EndBnkUpperBound
            },
            { //ContrastLink
                true,  //bEnableContrastLink total control of contrast link
                { //FaceLCELink
                    2, //u4FaceRbTrustCnt
                    3, //u4FaceDropTempSmoothCnt
                    3, //u4OTFaceTrustCnt_FDIN
                    0, //u4OTFaceTrustCnt_FDCONTINUE
                    MTRUE, //bOTFaceTimeOutLockAE
                    0,0,0,0,0,
                },
                1000, // i4LTMFaceProtectPercent
                100, // i4LTMFaceBrightTonePercent
                70, // i4LTMTopCropRatio
                70, // i4LTMBottomCropRatio
                70, // i4LTMLeftCropRatio
                70, // i4LTMRightCropRatio
                {308, 308, 308, 308, 308}, //m_u4LCEMaxGain
                false,                      //Enable dynamic ltm weight
                5,                         //Max size : 8
                {0,1000,2000,3000,6000},   //u4OE_LTM_Weight_BV_Tbll[u4OELTM_Weight_TblLength]
                {20,30,40,60,80},          //u4OE_LTM_Weight_OE_Ratio_Tbl[u4OELTM_Weight_TblLength]
                {                          //u4OE_LTM_Weight_OE_Prob_Tbl[u4OELTM_Weight_TblLength][u4OELTM_Weight_TblLength]
                   //BV0-OE
                   {128, 160, 192, 224, 256},
                   //BV1-OE
                   {128, 160, 192, 224, 256},
                   //BV2-OE
                   {128, 256, 512, 640, 720},
                   //BV3-OE
                   {720, 720, 720, 720, 720},
                   //BV4-OE
                   {720, 720, 720, 720, 720},
                },       
                {0}, //Reserve for other contrast Link param
            },
            { //FolivoraEffect
                false, //bEnableFolivoraEffect; 
                {-8000, -2000, 0, 6000},//i4BvTable[AE_STABLE_BV_MAXSIZE];
                {300,300,300,300},//i4FolivoraBMovingTH[AE_STABLE_BV_MAXSIZE];
                {300,300,300,300},// i4FolivoraFMovingTH[AE_STABLE_BV_MAXSIZE];
                {65,65,65,65},//i4FolivoraFMovingQuantityTH[AE_STABLE_BV_MAXSIZE];
                {0,0,0,0},//i4FolivoraDarkMotionTH[AE_STABLE_BV_MAXSIZE];
                {0,0,0,0},//i4FolivoraDarkMinMotionTH[AE_STABLE_BV_MAXSIZE];
                {300,300,300,300},//i4FolivoraTrackingTH[AE_STABLE_BV_MAXSIZE]; 
                {0,0,0,0},//i4FolivoraHesitateTH[AE_STABLE_BV_MAXSIZE];
            },
            {//strMvhdrTargetCFG
                5,                            // u4TblLengthnv
                { -1500, -500, 1000, 3500, 8000 },  // i4BV4HDRTarget
                {    50,   58,   60,   63,   67 },   // u4HDRTarget
            },
            {//strMvhdrFDTargetCFG
                3,                            // u4TblLength
                { -2000,    0,  4000 },  // i4BV4HDRFDTarget
                {    40,   35,    30 },   // u4HDRFDTarget
            },
            {//strHdrSmoothCFG
                950,           //u4B2DLinearThd
                -45,           //i4B2DBVACCExtreamBound
                -22,           //i4B2DBVACCNonlinearBound
                -16,           //i4B2DNonlinearEVDBound;
                -12,           //i4B2DLinearEVDBound;
                
                750,           //u4D2BLinearThd;
                50,            //i4D2BNonlinearEVDBound;
                50,            //i4D2BLinearEVDBound;
            },
            {//str3ExpomVHdrCFG
                {   //str3ExpomVHdrRatioSmoothTbl
                    MTRUE,                                       // bEnable
                    4,                                          // u4TblLength
                    {     30,   100,   400,   800 },            // u4RatioPcent_X
                    {    512,   512,   256,   128 },            // u4RatioPcent_Y
                },
                {   //str3ExpomVHdrRatioLimitTbl
                    MTRUE,                                       // bEnable
                    4,                                          // u4TblLength
                    {     100,   300,  1000, 2000},             // u4RatioLimit_X
                    {    1024,   128,     0,    0},             // u4RatioLimit_Y
                },
            },
            {//strHdrRatioCFG
                {                                 // HdrMaxRatioInfo
                    6400,                         // i4MaxHDRRatio
                    6400,                         // i4MaxHDRRatio_Face
                    4,
                    { -1000, 1000, 3500, 8000 },  // i4BV4MaxHDRRatio
                    {  6400, 6400, 6400, 6400 }   // u4MaxHDRRatio_BV
                },
                {                                 // HdrRatioConvergeInfo
                    20,                           // i4HDRRatioInTolerance
                    30,                           // i4HDRRatioOutTolerance
                    3,                            // i4HDRStableCountTHD
                },
                {                                  // HdrRatioTargetInfo
                    MFALSE,                         // bEnableHLAvgRatioControl;
                    200,                           // u4HDRHLRatio
                    120,                           // u4HDRHLTarget
                    MTRUE,                          // bEnableHistOERatioControl
                    5,                             // u4TblLengthOE1
                    {    0,   3, 10,  20,   25 },  // i4OE1Pct
                    {    0,   0,100, 150,  200 },  // i4RatioMovOE1
                    5,                             // u4TblLengthOE2
                    {    0,   3, 10,  20,   25 },  // i4OE2Pct
                    { -100, -30,  0,   0,    0 },  // i4RatioMovOE2
                    {    1,  129}                  // u4OEBin
                },
                {                                 // HdrRatioDecreaseInfo
                    MTRUE,                         // bEnableBVDecrease
                    4,                            // u4BVTblLength
                    { -1000, 1000, 3500, 8000 },  // i4BV4HDRRatioDecrease
                    {   100,  100,  100,  100 },  // u4RatioDecresaeRate4BV
    
                    MTRUE,                         // bEnableOEDecrease
                    10,                           // u4OEBinN
                    4,                            // u4OETblLength
                    {    10,   20,   30,   40},   // u4OERate4HDRDecrease
                    {   100,   100,  100, 100}   // u4RatioDecreaseRate4OE
                },
                0,  // i4HdrExpTransMethod
            },
            {//MHDRHSMultiStep
                MTRUE,
                4, // u4TblLength
                { 1100, 2600, 4500, 6000 },      // u4MS_EVDiff
                { 65, 150, 165, 180 },                 // u4MS_OutdoorTHD
                { 65, 150, 165, 180 },                  // u4MS_IndoorTHD
                { 4500,  8000 },                           // i4MS_BVRatio
            },
            {//HdrPre2CapComp
                MFALSE, //bHdrPre2CapComp
                0, //u4HdrPre2CapCompMethod
                {
                    { 1024, 4096}, //u4FDAvgRatio_X
                    { 1024, 1300}, //u4FDAvgRatio_Y
                }
            },
            {//HdrReserve[256]
                0,
            },
            {   //strStereoPlineMapping
                {
                    {LIB3A_AE_SCENE_AUTO, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11}},
                    {LIB3A_AE_SCENE_NIGHT, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_ACTION, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_BEACH, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_CANDLELIGHT, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_FIREWORKS, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_LANDSCAPE, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_PORTRAIT, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_NIGHT_PORTRAIT, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_PARTY, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_SNOW, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_SPORTS, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_STEADYPHOTO, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_SUNSET, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_THEATRE, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_ISO_ANTI_SHAKE, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
                    {LIB3A_AE_SCENE_ISO100, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO100, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO100}},
                    {LIB3A_AE_SCENE_ISO200, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200}},
                    {LIB3A_AE_SCENE_ISO400, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400}},
                    {LIB3A_AE_SCENE_ISO800, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800}},
                    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600}},
                    {LIB3A_AE_SCENE_ISO3200, {AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX6,AETABLE_SCENE_INDEX13,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX6}},
                    {LIB3A_AE_SCENE_ISO6400, {AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX7,AETABLE_SCENE_INDEX13,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX7}},
                    {LIB3A_AE_SCENE_ISO12800, {AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX8,AETABLE_SCENE_INDEX13,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX11,AETABLE_SCENE_INDEX8}},
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                },
            },
            {   //strHDRPlineMapping
                {
                    {LIB3A_AE_SCENE_AUTO, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX5}},
                    {LIB3A_AE_SCENE_NIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
                    {LIB3A_AE_SCENE_ACTION, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
                    {LIB3A_AE_SCENE_BEACH, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
                    {LIB3A_AE_SCENE_CANDLELIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
                    {LIB3A_AE_SCENE_FIREWORKS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX4, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX4}},
                    {LIB3A_AE_SCENE_LANDSCAPE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
                    {LIB3A_AE_SCENE_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
                    {LIB3A_AE_SCENE_NIGHT_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
                    {LIB3A_AE_SCENE_PARTY, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
                    {LIB3A_AE_SCENE_SNOW, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
                    {LIB3A_AE_SCENE_SPORTS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
                    {LIB3A_AE_SCENE_STEADYPHOTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
                    {LIB3A_AE_SCENE_SUNSET, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
                    {LIB3A_AE_SCENE_THEATRE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
                    {LIB3A_AE_SCENE_ISO_ANTI_SHAKE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
                    {LIB3A_AE_SCENE_ISO100, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO100, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO100}},
                    {LIB3A_AE_SCENE_ISO200, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO200, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO200}},
                    {LIB3A_AE_SCENE_ISO400, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO400, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO400}},
                    {LIB3A_AE_SCENE_ISO800, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO800, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO800}},
                    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO1600, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_SCENE_INDEX6, AETABLE_SCENE_INDEX7, AETABLE_SCENE_INDEX8, AETABLE_SCENE_INDEX9, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO1600}},
                    {LIB3A_AE_SCENE_ISO3200,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX6, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO,  AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX6}},
                    {LIB3A_AE_SCENE_ISO6400,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX7, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO,  AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX7}},
                    {LIB3A_AE_SCENE_ISO12800,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX8, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO,  AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX8}},
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
                },
            },
            {//strDynamicFlareCFG
                4,
                2,
                2,
                MTRUE,
                32,               //u4OutdoorMaxFlareThres
                {3500, 4500}      //i4BVRatio[2]
            },
            /*ccu_ae_smooth_v6p0_cfg*/
            {   
                2,            /*ae_cycle*/
                -1,           /*small_range_idx*/
				-1,           /*face_small_range_idx*/
                2,            /*inverse_protect_ratio*/
                80,           /*extreme_over_perframe_ratio*/
                { 100, 180 }, /*extreme_over_non_linear_cwv_ratio_x*/
                { 100, 180 }, /*extreme_over_non_linear_cwv_ratio_y*/
                50,           /*extreme_under_perframe_ratio*/
                { 100, 180 }, /*extreme_under_non_linear_cwv_ratio_x*/
                { 100, 180 }, /*extreme_under_non_linear_cwv_ratio_y*/
                
                1,              /*over_bv_th_enable*/
                2,              /*over_tbl_length*/
                { 8000, 9000 }, /*over_bv_th[16]*/
                { 90, 60 },     /*over_perframe_ratio[16]*/
                90,           /*over_low_bv_perframe_ratio_default*/
                
                480,          /*over_limit_delta_index*/
                { 30, 100 },{ 90, 70 },   /*over_prob_ratio*/
                { 105, 150 },{ 100, 75 }, /*over_non_linear_cwv_ratio*/
                { 105, 150 },{ 100, 85 }, /*over_non_linear_avg_ratio*/
                
                85,           /*over_pointlight_perframe_ratio*/
                300,          /*over_pointlight_limit_delta_index*/
                { 30, 70 },{ 70,  50 },   /*over_pointlight_prob_ratio*/
                { 105, 150 },{ 100, 60 }, /*over_pointlight_non_linear_cwv_ratio*/
                { 105, 150 },{ 100, 60 }, /*over_pointlight_non_linear_avg_ratio*/
                
                -48, /*pointlight_enable_delta_idx*/
                50,  /*pointlight_perframe_ratio*/
                130,  /*pointlight_limit_delta_index*/
                { 20, 60 },{ 115,  130 },  /*pointlight_prob_ratio*/
                { 120, 140 },{ 100, 140 }, /*pointlight_non_linear_cwv_ratio*/
                { 100, 120 },{ 55, 100 },  /*pointlight_non_linear_cwv_stop_ratio*/
                
                1,/*normal_bv_th_enable*/
                2,/*normal_tbl_length*/
                { 9000, 10000 },/*normal_bv_th_b2t[16]*/
                { 65, 45 },/*normal_perframe_ratio_b2t[16]*/
                { 9000, 10000 },/*normal_bv_th_d2t[16]*/
                { 40, 30 },/*normal_perframe_ratio_d2t[16]*/
                65,  /*normal_perframe_ratio_b2t_default*/
                40,  /*normal_perframe_ratio_d2t_default*/
                { 105, 150 },{ 100,  60 },  /*normal_non_linear_cwv_ratio*/
                { 105, 150 },{ 100,  60 }, /*normal_non_linear_avg_ratio*/
                { 50, 100 },{ 100, 70 },  /*normal_delta_idx_ratio*/
                80,/*normal_enable_delta_idx_ratio_thd*/
    
                95,/*extreme_overexp_perc*/
                5,/*extreme_underexp_perc*/
                
                10,/*overexp_prob_high_bound_prec*/
                120,/*overexp_prob_non_linear_ratio_th*/
                40,/*overexp_prob_light_pixel_th*/
                190,/*overexp_prob_cwv_th*/
                
                10,/*overexp_pl_prob_high_bound_prec*/
                30,/*overexp_pl_prob_low_bound_prec*/
                9000,/*overexp_pl_prob_bv_th*/
                140,/*overexp_pl_prob_non_linear_ratio_th*/
                200,/*overexp_pl_prob_linear_cwv_th*/
                16,/*overexp_pl_prob_over_prec*/
                25,/*overexp_pl_prob_light_pixel_prec*/
                30,/*overexp_pl_prob_dark_pixel_prec*/
                
                30,/*pl_prob_low_bound_prec*/
                7,/*pl_prob_high_bound_prec*/
                4,/*pl_prob_black_bound_prec*/
                660,/*pl_prob_EVD_th*/
                8000,/*pl_prob_bv_th*/
                50,/*pl_prob_mid_pixel_prec*/
                30,/*pl_prob_dark_pixel_prec*/
                10,/*pl_prob_light_pixel_dw_prec*/
                40,/*pl_prob_light_pixel_up_prec*/
                30,/*pl_prob_black_pixel_prec*/
                100,
                { 120, 200 },{ 20, 40 },
                { 120, 200 },{ 20, 40 },/*face_perframe_ratio_d2t*/
                1,/*smooth_gyro_enable*/
                80,/*gyro_on_th*/
                { 80, 220 },{ 60, 10 },/*gyro_x_y*/
            80,           /*touch_extreme_over_perframe_ratio*/
            { 100, 180 }, /*touch_extreme_over_non_linear_cwv_ratio_x*/
            { 100, 180 }, /*touch_extreme_over_non_linear_cwv_ratio_y*/
            50,           /*touch_extreme_under_perframe_ratio*/
            { 100, 180 }, /*touch_extreme_under_non_linear_cwv_ratio_x*/
            { 100, 180 }, /*touch_extreme_under_non_linear_cwv_ratio_y*/

            1,/*touch_normal_bv_th_enable*/
            2,/*touch_normal_tbl_length*/
            { 9000, 10000 },/*touch_normal_bv_th_b2t[8]*/
            { 65, 45 },/*touch_normal_perframe_ratio_b2t[8]*/
            { 9000, 10000 },/*touch_normal_bv_th_b2t[8]*/
            { 40, 30 },/*touch_normal_perframe_ratio_b2t[8]*/
            65,  /*touch_normal_perframe_ratio_b2t_default*/
            40,  /*touch_normal_perframe_ratio_d2t_default*/

            { 105, 150 },{ 100,  60 },  /*touch_normal_non_linear_cwv_ratio*/
            { 105, 150 },{ 100,  60 }, /*touch_normal_non_linear_avg_ratio*/
            { 50, 100 },{ 100, 70 },  /*touch_normal_delta_idx_ratio*/
            80,/*touch_normal_enable_delta_idx_ratio_thd*/
                {0} /*reserved*/
            },
            //strLightSensor
            {
                0,      // bEnableLS
                1,      // u4TblLength
                {0},    // i4LS_Bv
                {0},    // i4LS_Offset
            },
            //strAisControl
            {
                0,      //bEnableAisControl
                0,      //u4Mode
                3,      //u4Bv_TblLength
                1,      //u4Motion_TblLength
                {-1000, 3000, 7000, 0, 0}, //i4Bv_Tbl
                {0}, //i4Motion_Tbl
                {    //u4Shutter_Thd
                    {0},
                    {0},
                    {0},
                    {0},
                    {0}
                },
                {    //u4Iso_Thd 
                    {0},
                    {0},
                    {0},
                    {0},
                    {0}
                },
            },
            
            NVRAM_CUSTOM_AE_REVISION
        },
    },
};
