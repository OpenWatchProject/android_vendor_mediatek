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

#include "camera_custom_types.h"
#include "camera_custom_nvram.h"
#include "ae_feature.h"
#include <aaa_types.h>
#include <custom/aaa/AEPlinetable.h>
#include "awb_param.h"
#include "ae_param.h"
#include "ae_tuning_custom.h"

using namespace NSIspTuning;
/*******************************************************************************
*
********************************************************************************/
template <>
AE_PARAM_T const&
getAEParam<ESensorDev_MainSecond>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        TRUE,                // bEnablePreIndex
        TRUE,                // bEnableRotateWeighting;
        TRUE,               // bEV0TriggerStrobe
        FALSE,               // bLockCamPreMeteringWin;
        FALSE,               // bLockVideoPreMeteringWin;
        TRUE,                // bLockVideoRecMeteringWin;
        TRUE,                // bSkipAEinBirghtRange;
        TRUE,                // bPreAFLockAE
        TRUE,                // bStrobeFlarebyCapture
        TRUE,                // bEnableFaceAE
        TRUE,                // bEnableMeterAE
        TRUE,                // b4FlarMaxStepGapLimitEnable
        TRUE,                // bEnableAESmoothGain
        TRUE,                // bEnableLongCaptureCtrl
        FALSE,                //bEnableTouchSmoothRatio : enable Touch mode smooth converge
        FALSE,               // bEnableTouchSmooth : enable Touch mode smooth with peframe
        TRUE,                // bEnablePixelBaseHist
        TRUE,                // bEnableHDRSTATconfig   : enable HDR Y LSB config
        TRUE,                // bEnableAEOVERCNTconfig  : enable AE over-exposure count config
        TRUE,                // bEnableTSFSTATconfig    : enable TSF RGB STAT config
        1,                   // bEnableHDRLSB
        FALSE,                 //bEnableFlareFastConverge
        FALSE,                 //bEnableRAFastConverge
        FALSE,                 //bEnableFaceFastConverge
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
        79,                     // u4MinYLowBound;        // metering and face boundary min Y value
      256,                     // u4MaxYHighBound;      // metering and face boundary max Y value
     1024,                     // u4MeterWeight 1024 base.100%=>1024
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
        1,                   //uCycleNumAESkipAfterExit;
        254,                //overexposure cnt threshold
        11,                 //u4HSSmoothTHD
        1,                  //u4FinerEvIndexBase 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
    };

    static strWeightTable  g_Weight_Matrix =
    {
        AE_WEIGHTING_CENTRALWEIGHT,
        {
            {6 ,19 ,28 ,19 ,6 },
            {19 ,60 ,88 ,60 ,19 },
            {28 ,88 ,128+32 ,88 ,28 },
            {19 ,60 ,88 ,60 ,19 },
            {6 ,19 ,28 ,19 ,6 },
        }
    };

    static strWeightTable  g_Weight_Spot =
    {
        AE_WEIGHTING_SPOT,
        {
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
        }
    };

    static strWeightTable  g_Weight_Average =
    {
        AE_WEIGHTING_AVERAGE,
        {
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
        }
    };

    static strAFPlineInfo g_strAFPLineTable =     // from high lv to low lv
    {
         0,                  // 1 enable the AF Pline, 0 disable the AF Pline (bypass)
         {
             {73, 30},       // {LV value, AF frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strAFZSDPLineTable =     // from high lv to low lv
    {
        0,                  // 1 enable the AF ZSD Pline, 0 disable the AF ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, AF frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static strAFPlineInfo g_strStrobePLineTable =     // from high lv to low lv
    {
         1,                  // 1 enable the Strobe Pline, 0 disable the Strobe Pline (bypass)
         {
             {73, 30},       // {LV value, Strobe frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strStrobeZSDPLineTable =     // from high lv to low lv
    {
        1,                  // 1 enable the Strobe ZSD Pline, 0 disable the Strobe ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, Strobe frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static MUINT32 g_strEVValueArray[LIB3A_AE_EV_COMP_MAX] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEVideoMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEFaceMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AETrackingMoveTable[] =
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
    };


    // for AE limiter
    static strAELimiterTable g_AELimiterDataTable =
    {
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
    };

    // for video dynamic frame rate
    static VdoDynamicFrameRate_T g_VdoDynamicFpsTable =
    {
       TRUE, // isEnableDFps
       50,  // EVThresNormal
       50,  // EVThresNight
    };
    static strFaceLocSize g_FaceLocSizeCheck =
    {
        FALSE,       //bFaceLocSizeCheck;
        {20,    5},//u4Size_X[2];
        {1024, 0},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 1024}, //u4Loc_Y[2];

        FALSE,                                    //bFaceOECheck;
        {10240,    20480},            //u4OE_X[2];
        {1024, 768},                    //u4OE_Y[2];
        {10,  30},        //u4FCY_DX[2];
        {110,  95},        //u4FCY_BX[2];
        {1024, 0},        //u4FCY_Y[2];

        FALSE,                                    //bLandmarkSize;
    };
    static strPerframeCFG g_PerframeCFG =
    {
        200,
        80,
        80,
        240,
        4,
        254,
        510,
        1,
        TRUE,
    };
	static strAETgTuningPara g_TgIntTuningParam =
	{
		//B2T
		245,
		70,
		200,

		{80, 300, 500},  // STD = 1, 2, 3
		{100, 85, 60, 40},  // 100%, 85%, 60%, 50%


		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{-30, -24, -20, -20},

		200,
        245, //OverExpoLowBound
        16,  //OverExpLightAcc1
        16,  //OverExpLightAcc2
        16,  //OverExpLightAcc3
        8,   //OverExpLightAcc4
		//D2T
		7, //245
		95, //70%
		20,

		{80, 250, 500},  // STD = 1, 2, 3
		{100, 85, 60, 35},	// 100%, 85%, 60%, 50%

		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{20, 15, 10, 10},

		30
	};


    static MINT32 g_TgEVCompJumpRatioOverExp[LIB3A_AE_EV_COMP_MAX] =
    {
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
    };

	static MINT32 g_TgEVCompJumpRatioUnderExp[LIB3A_AE_EV_COMP_MAX] =
    {
       100,  // EV   0
       100,   //EV    0.1
       100,   //EV    0.2
       100,   //EV    0.3
       100,   //EV    0.4
       100,   //EV    0.5
       100,   //EV    0.6
       100,   //EV    0.7
       100,   //EV    0.8
       100,   //EV    0.9
       110,   //EV    1
       110,   //EV    1.1
       110,   //EV    1.2
       110,   //EV    1.3
       110,   //EV    1.4
       110,   //EV    1.5
       110,   //EV    1.6
       110,   //EV    1.7
       110,   //EV    1.8
       110,   //EV    1.9
       120,   //EV    2
       120,   //EV    2.1
       120,   //EV    2.2
       120,   //EV    2.3
       120,   //EV    2.4
       120,   //EV    2.5
       120,   //EV    2.6
       120,   //EV    2.7
       120,   //EV    2.8
       120,   //EV    2.9
       130,   //EV    3
       130,   //EV    3.1
       130,   //EV    3.2
       130,  //EV    3.3
       130,  //EV    3.4
       130,  //EV    3.5
       130,  //EV    3.6
       130,  //EV    3.7
       130,  //EV    3.8
       130,  //EV    3.9
       130,  //EV    4
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
    };



    static strBVAccRatio  g_AEBVAccRatio = {
        TRUE,
        90,
        220,
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)
        50,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
        4000,
        -4000
    };
    static strNonCWRAcc g_AENonCWRAcc =
    {
        FALSE,          //bNonCWRAccEnable enable
        255,
        160,
        160,            //it should  be <=hs_THD
        58,             //it should keep in target~hs_flat
    };
    static strFaceSmooth g_AEFaceSmooth = {
         10,                  // u4MinYLowBound;        // metering and face boundary min Y value
        256,                  // u4MaxYHighBound;      // metering and face boundary max Y value
          0,                  // i4FaceBVLowBound;     // face boundary min BV value
       4000,                  // i4FaceBVHighBound;    // face boundary max BV value
        235,                  // tempFDY HighBound
          1,                  // tempFDY LowBound
         20,                  // face lost max count
         100,                  // face window scale ratio range 1~400% default 90%
         35,                  // face NS lowbound
         10,                  // u4FD_Lock_MaxCnt
          5,                  // u4FD_MixCWR_MaxCnt
          0,                  // u4FD_TemporalSmoothY
         75,                  // u4FD_FaceWidthCropRat
         75,                  // u4FD_FaceHeightCropRat
          5,                  // u4FD_InToOutThdMaxCnt
        220,                  // u4FD_OTLockUpperBnd
          5,                  // u4FD_OTLockLowerBnd
          1,                  // u4FD_ContinueTrustCnt
          0,                  // u4FD_PerframeAntiOverFlag
          0,                  // u4FD_SideFaceLock
          1,                  // u4FD_LimitStableThdLowBnd
          1,                  // u4FD_LimitStableThdLowBndNum
          1,                  // u4FD_ReConvergeWhenFaceChange
          0,                  // u4FD_FaceMotionLockRat
          4,                  // u4FD_ImpulseLockCnt
         25,                  // u4FD_ImpulseUpperThd
         18,                  // u4FD_ImpulseLowerThd
          2,                  // u4FD_ROPTrustCnt
         30,                  // u4FD_ReCovergeWhenSizeChangeRat
        120,                  // u4FD_ReCovergeWhenPosChangeDist
          1,                  // u4FD_ReCovergeWhenAFDone;
          1                   // u4FD_OTFaceLock;
    };
    static strFaceWeight g_AEFaceWeight = {
        MFALSE,
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
    };
    static strAECWRTempSmooth g_AECWRTS =
    {
        TRUE,                                // enable AE HS Target temp smooth
        TRUE,                                // enable AE ABL Target temp smooth
        TRUE,                                // enable AE AOE Target temp smooth
        TRUE,                                // enable AE NS Target temp smooth
        TRUE,                                // enable AE Face Target temp smooth
        TRUE,                                // enable AE Video Target temp smooth
        TRUE,                               // enable AE Main Target temp smooth
        TRUE,                                // enable AE HSV4p0 Target temp smooth
    };
    static strFaceLandMarkCtrl g_AEFaceLandmark = {
        FALSE,                //bLandmarkCtrlFlag
        0,                    //u4LandmarkCV_Thd
        3,                    //u4LandmarkWeight
      100,                    //i4LandmarkExtRatW;
      100,                    //i4LandmarkExtRatH;
       90,                    //i4LandmarkTrustRopDegree;
        5,                    //i4RoundXYPercent;
        3,                    //i4RoundSZPercent;
        9,                    //i4THOverlap;
        3,                    //i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
        8,                    //i4SMOOTH_LEVEL;
        0,                    //i4MOMENTUM;
       50                     //u4LandmarkWeightPercent;
    };
    static strAEStableThd g_AEStableThd =
    {
        0,                      // u4InStableThd;  // 0.08EV
        1,                      // u4OutStableThd

        TRUE,             // enable ae different mode stable threshold setting
        1,                      // u4VideoInStableThd;  // 0.08EV
        1,                      // u4VideoOutStableThd
        1,                      // u4FaceInStableThd;  // 0.08EV
        2,                      // u4FaceOutStableThd
        5,                     // u4FaceOutB2TStableThd
        5,                     // u4FaceOutD2TStableThd
        0,                      // u4TouchInStableThd;  // 0.08EV
        1,                      // u4TouchOutStableThd
        FALSE,                  // Face Ae lock option
        FALSE                   // enable zero stable thd
    };

    static strAEMovingRatio g_AETouchMovingRatio =
    {
        100, //u4SpeedUpRatio
        100, //u4GlobalRatio
        190, //u4Bright2TargetEnd
        20,   //u4Dark2TargetStart
        80, //u4B2TEnd
        60,  //u4B2TStart
        40,  //u4D2TEnd
        80,  //u4D2TStart
    };

    static strHSFlatSkyCFG g_HSbyRealBV =
    {
        TRUE,                   //bEnableFlatSkyEnhance
        2,                          //uSkyDetInThd  : 2%
        5,                          //uSkyDetOutThd : 5%
        160,                    //u4FlatSkyTHD
        1500,                   //u4FlatSkyEVD
        {8000,  11000}, //u4BVRatio_X : BV
        {0, 1024}               //u4BVRatio_Y : Ratio
    };

    static strHSMultiStep g_HSMultiStep =
    {
       TRUE,                                            // bEnableMultiStepHS
       4,                                                   // u4TblLength
       {1100, 2600, 4500,6000},      // u4MS_EVDiff
       {75, 160, 160, 195},                 // u4MS_OutdoorTHD
       {70, 145, 150, 160},                  // u4MS_IndoorTHD
       {5500,  8000},                           // i4MS_BVRatio
    };
    static AE_HIST_WIN_CFG_T g_AEBlockHistWINCFG =
    {
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
    };

    static AE_HIST_WIN_CFG_T g_AEPixelHistWINCFG =
    {
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
    };
    static strAOEBVRef g_strAOEByBV =
    {
       TRUE,                                       // bEnable
       4,                                               // u4TblLength
       {-2000, 0, 3000,5500},              // i4BV
       {150, 170, 180, 180}       // u4THD
    };
    static strABL_absDiff g_ABLabsDiff =
    {
        TRUE,                           //enable enhance ABL(absDiff)
        {2500,  5000},            //u4EVDiffRatio_X
        {0, 1024},                    //u4EVDiffRatio_Y
        {9000,  11000},         //u4BVRatio_X
        {0, 1024}                     //u4BVRatio_Y
    };
    static strNS_CDF g_strNSCDF =
    {
       TRUE,                                       // bEnable
       600,
       {-2000, -1000},
       {1024, 0}
    };
    static strHistStableCFG g_HistStableCFG =
    {
        TRUE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
        TRUE,                                           //bEnableHistOverExpoCnt
        6,                                                //u4OElevel : pseudo-high strength 6x
        2,                                                  //u4OERatio_LowBnd : 2% over exposure area
        20,                                                 //u4OERatio_HighBnd : 20% overexposure area
        {4000,  6000},                                   //i4OE_BVRatio_X[2]
        {0,     1024},                                     //u4OE_BVRatio_Y[2]
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
    };

    static strMainTargetCFG g_MainTargetCFG =
    {
        TRUE,               //bEnableAEMainTarget
        1024,               //u4MainTargetWeight
        {                         //u4MainTargetWeightTbl[15][15] : Gau weight table
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        },
        {   // TargetBVRatioTbl
            -2000,                   // TargetBVRatioTbl.i4X1 :BV
            42,                          // TargetBVRatioTbl.i4Y1 :Target
            4000,                   // TargetBVRatioTbl.i4X2  :BV
            52                          // TargetBVRatioTbl.i4Y2 :Target
        },
        TRUE,                   //bEnableColorWTRatio
        {   //ColorWTRatioTbl
            130,                            // ColorWTRatioTbl.i4X1 :ColorDiff
            1024,                          // ColorWTRatioTbl.i4Y1 :u4MainTargetWeight
            300,                        // ColorWTRatioTbl.i4X2  :ColorDiff
            768                          // ColorWTRatioTbl.i4Y2 :u4MainTargetWeight / 2
        },
        100,                           //uColorSupRatioR : 0~100
        100,                           //uColorSupRatioG : 0~100
        50,                               //uColorSupRatioB : 0~100
        {   //ColorSupBVRatioTbl
            5000,                            // ColorSupBVRatioTbl.i4X1 :BV
            0,                          // ColorSupBVRatioTbl.i4Y1 :Ratio
            6500,                        // ColorSupBVRatioTbl.i4X2  :BV
            1024                          // ColorSupBVRatioTbl.i4Y2 :Ratio
        },
    };

    static strHSv4p0CFG g_HSv4p0CFG =
    {
        TRUE,                   //bEnableHSv4p0
        1024,                   //u4HSv4p0Weight
        3,                          //u4BVSize ,max16
        4,                          //u4EVDSize ,max16
        {1000, 4500, 8000},     //i4HS_BVRatio, length = u4BVSize
        {80     ,      60,      15},    //u4HS_PcntRatio, length = u4BVSize, 80 equal 8%
        {1100, 2600, 4000,6500},    //u4HS_EVDRatio , length = u4EVDSize
        {                                       //u4HSTHDRatioTbl[u4BVSize][u4EVDSize] , HS THD
            //BV0-THD
            {90,    140,   160,  160},
             //BV1-THD
            {130, 150,   175,  200},
             //BV2-THD
            {140, 160,   195,  210},
        },
        TRUE,                   //bEnableDynWTRatio
        {       //DynWTRatioTbl
            0,                            // DynWTRatioTbl.i4X1 : min thd
            0,                              // DynWTRatioTbl.i4Y1 :add delta weight 1024base
            30,                         // DynWTRatioTbl.i4X2  :max thd
            256,                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        },
        FALSE,         //bEnableGreenSupress;
        120,          //uGreenGRRatio;
        130,          //uGreenGBRatio;
        70,           //uSupRatio;
        {256,512},    //u4HS_GCountRatio_X
        {0,1024},     //u4HS_GCountRatio_Y
    };

    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
       {-3500, -2000, 1000},       //i4BV[16]
       {68, 72, 75},   //u4BTTHD[16]
       {140, 160, 170},   //u4NSTHD[16]
    };

    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eMain2HDR_OFF,          // eHdrParamID

	      {
			      MTRUE,              // bHdrDetectorEnable
            80, // 20,                       // i4HdrConfidenceThr
			      {
                TRUE,           // bHistDetectEnable

                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },
				
                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                60,             // i4ContrastHistRatioH
                450,            // i4ContrastHistRatioL
                50,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },
				        {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            {
                {
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight

                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                {
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    {           // rCenterDarkDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    }
                }
            },
            {
                TRUE,           // bLVDetectEnable
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },

        {
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            20,                 // i4RMGtRatio

            {                   // rRMGt
                160,            // i4X1
                512,            // i4Y1
                220,            // i4X2
                384             // i4Y2
            },

            //Not used
            /**************************************************/
            2,                  // i4SupportHdrNum
            1024,               // i4AutoEnhanceProb
            1024,               // i4HdrWeight
            /**************************************************/

            // rAutoBright
            {
               //LV 0  1  2  3  4  5  6  7  8  9  10 11 12 13  14  15 16 17 18    i4LVRatio50
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},
               //LV 0  1  2  3  4  5  6  7  8  9  10 11 12 13  14  15 16 17 18    i4LVRatio60
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},

                {               // rContrastRatio
                    3500,       // i4X1
                    200,        // i4Y1
                    4500,       // i4X2
                    400         // i4Y2
                },

                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                {               // rNightProbRatio
                    0,          // i4X1
                    400,        // i4Y1
                    512,        // i4X2
                    200         // i4Y2
                },

                {
                    {               // rAcceProb
                        1500,       // i4X1
                        0,          // i4Y1
                        2500,       // i4X2
                        1024        // i4Y2
                    },

                    {               // rGyroProb
                        100,        // i4X1
                        0,          // i4Y1
                        700,        // i4X2
                        1024        // i4Y2
                    },

                    {               // rMotionRatio
                        512,        // i4X1
                        400,        // i4Y1
                        960,        // i4X2
                        100         // i4Y2
                    },

					768,            // i4MotionOnThr
                    5               // i4MotionOnCntThr
                }
            },

            {
                {130,130},        // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, 	// i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     	// i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/

                {               	// rIsoThr
                    14336,      	//i4X1:1024 * 14
                    1,          	//i4Y1
                    12288,      	//i4X2:1024 * 12
                    4           	//i4Y2
                },
            },

            {
                TRUE,           	// bHdrWaitAEStable
                6,              // i4HdrInfoCounterThr
				//Not used
                /**************************************************/
                FALSE,          // bRatioSmoothEnable TRUE for platform / FALSE for simulation
                /**************************************************/
                0,              // i4AutoWeightMode
                4,              // i4AutoWeightSmooth
                4,              // i4AutoWeightStableThr
                0,              // i4RMGtStableMode
                4,              // i4RMGtStableThr
                0,              // i4HdrRatioStableMode
                4,              // i4HdrRatioStableThr
                { 5,   2},      // i4RatioSmoothThr
                {30, 60},       // i4RoughRatioUnit
                {15, 60},       // i4FinerRatioUnit
                //Not used
                /**************************************************/
                {64, 128}       // i4RMGtUnit
                /**************************************************/
            },
            {                   // rAutoDark
                // TBD
            },
            //Not used
            /**************************************************/
            {                   // rAutoTuning
                10,             // i4RmmExpoDiffThr %
                24              // i4LscRatio
            },
            /**************************************************/
            100              // i4FlashRatio
        },

        {
            47,                 // i4DefTarget
            256,                // i4DefWeight
            {                   	// rAutoEVWeight
                4000,          	 	// rAutoEVWeight.i4X1
                256,            	// rAutoEVWeight.i4Y1
                5000,           	// rAutoEVWeight.i4X2
                512             	// rAutoEVWeight.i4Y2
            },

            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            {                   	// rAutoABLWeight
                0,              	// rAutoABLWeight.i4X1
                1024,           	// rAutoABLWeight.i4Y1
                512,            	// rAutoABLWeight.i4X2
                1024            	// rAutoABLWeight.i4Y2
            },

            {                       // rAutoLVWeightL
                 30,                // rAutoLVWeightL.i4X1
                768,                // rAutoLVWeightL.i4Y1
                 40,                // rAutoLVWeightL.i4X2
               1024                 // rAutoLVWeightL.i4Y2
            },

            {                       // rAutoLVWeightH
                100,                // rAutoLVWeightH.i4X1
                256,                // rAutoLVWeightH.i4Y1
                130,                // rAutoLVWeightH.i4X2
                512                 // rAutoLVWeightH.i4Y2
            },

            {                       // rAutoNightWeight
                1024,               // rAutoNightWeight.i4X1
                512,                // rAutoNightWeight.i4Y1
                2048,               // rAutoNightWeight.i4X2
                256                 // rAutoNightWeight.i4Y2
            },

            {
                25, //20,                // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            {
                150,             // i4NightPercent

                {               // rNightTarget
                    0,          // i4X1
                    80,        // i4Y1
                    1024,       // i4X2
                    60         // i4Y2
                }
            },

            {
                60,             // rTargetLimitL.i4LVLimit

                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            {
                80,             // rTargetLimitM.i4LVLimit

                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            {
                120,            // rTargetLimitH.i4LVLimit

                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitH.rTargetMaxLimit
                    4000,       // rTargetLimitH.rTargetMaxLimit.i4X1
                    47,         // rTargetLimitH.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMaxLimit.i4X2
                    90          // rTargetLimitH.rTargetMaxLimit.i4Y2
                }
            }
        },
        {                       // rHdrMiscProp
            {                   // rFaceAEProp
                {               // rFaceEVProb
                   3800, //2000,        // i4X1
                    128, //512, //384, // 512,        // i4Y1
                   5000, //2700,        // i4X2
                   1024  //128 //256         // i4Y2
                },
                {               // rFaceHdrProb
                   2000,        // i4X1
                   1024, //512, //384, // 512,        // i4Y1
                   3200,        // i4X2
                    128  //128 //256         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        {
            40,                 // i4LEThr
            20,                 // i4SERatio
            180,                // i4SETarget
            1024                // i4BaseGain
        }

    };
    static strAESceneMapping g_strStereoPlineMapping =
    {
    {
    {LIB3A_AE_SCENE_AUTO, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
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
    {LIB3A_AE_SCENE_ISO100, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    },
    };

    static strAESceneMapping g_strHDRPlineMapping =
    {
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
    };

    static strDynamicFlare g_strDynamicFlareCFG =
    {
        8,
        4,
        2
    };

    static AE_PARAM_T strAEInitPara =
    {
        g_AEParasetting,
        {
            &g_Weight_Matrix,
            &g_Weight_Spot,
            &g_Weight_Average,
            NULL
        },
        g_strAFPLineTable,
        g_strAFZSDPLineTable,
        g_strStrobePLineTable,
        g_strStrobeZSDPLineTable,
        &g_strEVValueArray[0],
        g_AEMoveTable,
        g_AEVideoMoveTable,
        g_AEFaceMoveTable,
        g_AETrackingMoveTable,
        g_AELimiterDataTable,
        g_VdoDynamicFpsTable,
        &g_AEBlockHistWINCFG,
        &g_AEPixelHistWINCFG,

        // v1.2
        FALSE,              // bOldAESmooth
        TRUE,               // bEnableSubPreIndex
        0,                      // u4VideoLPFWeight; // 0~23

        &g_FaceLocSizeCheck,
        &g_PerframeCFG,
        FALSE,               // Perframe AE smooth option
        FALSE,              // Perframe HDR AE smooth option

        FALSE,               // TG interrupt option
		TRUE, //Speedup Escape Overexposure
		TRUE, //Speedup Escape Underexposure
		&g_TgIntTuningParam,
		&g_TgEVCompJumpRatioOverExp[0],
		&g_TgEVCompJumpRatioUnderExp[0],
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

        //Open AE
        &g_AEStableThd,
        &g_AEBVAccRatio,
        &g_AEPsoConverge,
        &g_AENonCWRAcc,

        &g_AEFaceSmooth,
        &g_AEFaceWeight,
        &g_AECWRTS,
        &g_AEFaceLandmark,
        &g_strAOEByBV,
        &g_strNSCDF,

        &g_AETouchMovingRatio,
        &g_HSbyRealBV,
        &g_ABLabsDiff,
        &g_HSMultiStep,
        &g_HistStableCFG,
        &g_strOverExpoAOECFG,
        //v4.0
        TRUE,               //bAEv4p0MeterEnable;
        &g_MainTargetCFG,
        &g_HSv4p0CFG,
        &g_NSBVCFG,
        &g_strHdrAECFG,
        g_strStereoPlineMapping,
        g_strHDRPlineMapping,
        &g_strDynamicFlareCFG
    };

    return strAEInitPara;
}

template <>
AE_PARAM_T const&
getHDRAEParam<ESensorDev_MainSecond>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        TRUE,                // bEnablePreIndex
        TRUE,                // bEnableRotateWeighting;
        TRUE,               // bEV0TriggerStrobe
        FALSE,               // bLockCamPreMeteringWin;
        FALSE,               // bLockVideoPreMeteringWin;
        TRUE,                // bLockVideoRecMeteringWin;
        TRUE,                // bSkipAEinBirghtRange;
        TRUE,                // bPreAFLockAE
        TRUE,                // bStrobeFlarebyCapture
        TRUE,                // bEnableFaceAE
        TRUE,                // bEnableMeterAE
        TRUE,                // b4FlarMaxStepGapLimitEnable
        TRUE,                // bEnableAESmoothGain
        TRUE,                // bEnableLongCaptureCtrl
        FALSE,                //bEnableTouchSmoothRatio : enable Touch mode smooth converge
        FALSE,               // bEnableTouchSmooth : enable Touch mode smooth with peframe
        TRUE,                // bEnablePixelBaseHist
        TRUE,                // bEnableHDRSTATconfig   : enable HDR Y LSB config
        TRUE,                // bEnableAEOVERCNTconfig  : enable AE over-exposure count config
        TRUE,                // bEnableTSFSTATconfig    : enable TSF RGB STAT config
        1,                   // bEnableHDRLSB
        FALSE,                 //bEnableFlareFastConverge
        FALSE,                 //bEnableRAFastConverge
        FALSE,                 //bEnableFaceFastConverge
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
        79,                     // u4MinYLowBound;        // metering and face boundary min Y value
      256,                     // u4MaxYHighBound;      // metering and face boundary max Y value
     1024,                     // u4MeterWeight 1024 base.100%=>1024
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
        1,                   //uCycleNumAESkipAfterExit;
        254,                //overexposure cnt threshold
        11,                 //u4HSSmoothTHD
        1,                  //u4FinerEvIndexBase 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
    };

    static strWeightTable  g_Weight_Matrix =
    {
        AE_WEIGHTING_CENTRALWEIGHT,
        {
            {6 ,19 ,28 ,19 ,6 },
            {19 ,60 ,88 ,60 ,19 },
            {28 ,88 ,128+32 ,88 ,28 },
            {19 ,60 ,88 ,60 ,19 },
            {6 ,19 ,28 ,19 ,6 },
        }
    };

    static strWeightTable  g_Weight_Spot =
    {
        AE_WEIGHTING_SPOT,
        {
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
        }
    };

    static strWeightTable  g_Weight_Average =
    {
        AE_WEIGHTING_AVERAGE,
        {
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
        }
    };

    static strAFPlineInfo g_strAFPLineTable =     // from high lv to low lv
    {
         0,                  // 1 enable the AF Pline, 0 disable the AF Pline (bypass)
         {
             {73, 30},       // {LV value, AF frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strAFZSDPLineTable =     // from high lv to low lv
    {
        0,                  // 1 enable the AF ZSD Pline, 0 disable the AF ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, AF frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static strAFPlineInfo g_strStrobePLineTable =     // from high lv to low lv
    {
         1,                  // 1 enable the Strobe Pline, 0 disable the Strobe Pline (bypass)
         {
             {73, 30},       // {LV value, Strobe frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strStrobeZSDPLineTable =     // from high lv to low lv
    {
        1,                  // 1 enable the Strobe ZSD Pline, 0 disable the Strobe ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, Strobe frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static MUINT32 g_strEVValueArray[LIB3A_AE_EV_COMP_MAX] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEVideoMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEFaceMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AETrackingMoveTable[] =
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
    };


    // for AE limiter
    static strAELimiterTable g_AELimiterDataTable =
    {
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
    };

    // for video dynamic frame rate
    static VdoDynamicFrameRate_T g_VdoDynamicFpsTable =
    {
       TRUE, // isEnableDFps
       50,  // EVThresNormal
       50,  // EVThresNight
    };
    static strFaceLocSize g_FaceLocSizeCheck =
    {
        FALSE,       //bFaceLocSizeCheck;
        {20,    5},//u4Size_X[2];
        {1024, 0},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 1020}, //u4Loc_Y[2];

        FALSE,                                    //bFaceOECheck;
        {10240,    20480},            //u4OE_X[2];
        {1024, 768},                    //u4OE_Y[2];
        {10,  30},        //u4FCY_DX[2];
        {110,  95},        //u4FCY_BX[2];
        {1024, 0},         //u4FCY_Y[2];

        FALSE,                                    //bLandmarkSize;
    };
    static strPerframeCFG g_PerframeCFG =
    {
        200,
        80,
        80,
        240,
        4,
        254,
        510,
        1,
        TRUE,
    };

	static strAETgTuningPara g_TgIntTuningParam =
	{
		//B2T
		245,
		70,
		200,

		{80, 300, 500},  // STD = 1, 2, 3
		{100, 85, 60, 40},  // 100%, 85%, 60%, 50%


		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{-30, -24, -20, -20},

		200,
        245, //OverExpoLowBound
        0,   //OverExpLightAcc1
        0,   //OverExpLightAcc2
        0,   //OverExpLightAcc3
        0,   //OverExpLightAcc4
		//D2T
		7, //245
		95, //70%
		20,

		{80, 250, 500},  // STD = 1, 2, 3
		{100, 85, 60, 35},	// 100%, 85%, 60%, 50%

		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{20, 15, 10, 10},

		30
	};


    static MINT32 g_TgEVCompJumpRatioOverExp[LIB3A_AE_EV_COMP_MAX] =
    {
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
    };

	static MINT32 g_TgEVCompJumpRatioUnderExp[LIB3A_AE_EV_COMP_MAX] =
    {
       100,  // EV   0
       100,   //EV    0.1
       100,   //EV    0.2
       100,   //EV    0.3
       100,   //EV    0.4
       100,   //EV    0.5
       100,   //EV    0.6
       100,   //EV    0.7
       100,   //EV    0.8
       100,   //EV    0.9
       110,   //EV    1
       110,   //EV    1.1
       110,   //EV    1.2
       110,   //EV    1.3
       110,   //EV    1.4
       110,   //EV    1.5
       110,   //EV    1.6
       110,   //EV    1.7
       110,   //EV    1.8
       110,   //EV    1.9
       120,   //EV    2
       120,   //EV    2.1
       120,   //EV    2.2
       120,   //EV    2.3
       120,   //EV    2.4
       120,   //EV    2.5
       120,   //EV    2.6
       120,   //EV    2.7
       120,   //EV    2.8
       120,   //EV    2.9
       130,   //EV    3
       130,   //EV    3.1
       130,   //EV    3.2
       130,  //EV    3.3
       130,  //EV    3.4
       130,  //EV    3.5
       130,  //EV    3.6
       130,  //EV    3.7
       130,  //EV    3.8
       130,  //EV    3.9
       130,  //EV    4
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
    };
    static strBVAccRatio  g_AEBVAccRatio = {
        TRUE,
        90,
        220,
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)
        50,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
        4000,
       -4000
    };
    static strNonCWRAcc g_AENonCWRAcc =
    {
        FALSE,          //bNonCWRAccEnable enable
        255,
        160,
        160,            //it should  be <=hs_THD
        58,             //it should keep in target~hs_flat
    };
    static strFaceSmooth g_AEFaceSmooth = {
         10,                  // u4MinYLowBound;        // metering and face boundary min Y value
        256,                  // u4MaxYHighBound;      // metering and face boundary max Y value
          0,                  // i4FaceBVLowBound;     // face boundary min BV value
       4000,                  // i4FaceBVHighBound;    // face boundary max BV value
        235,                  // tempFDY HighBound
          1,                  // tempFDY LowBound
         20,                  // face lost max count
         100,                  // face window scale ratio range 1~400% default 90%
         35,                  // face NS lowbound
         10,                  // u4FD_Lock_MaxCnt
          5,                  // u4FD_MixCWR_MaxCnt
          0,                  // u4FD_TemporalSmoothY
         75,                  // u4FD_FaceWidthCropRat
         75,                  // u4FD_FaceHeightCropRat
          5,                  // u4FD_InToOutThdMaxCnt
        220,                  // u4FD_OTLockUpperBnd
          5,                  // u4FD_OTLockLowerBnd
          1,                  // u4FD_ContinueTrustCnt
          0,                  // u4FD_PerframeAntiOverFlag
          0,                  // u4FD_SideFaceLock
          1,                  // u4FD_LimitStableThdLowBnd
          1,                  // u4FD_LimitStableThdLowBndNum
          1,                  // u4FD_ReConvergeWhenFaceChange
          0,                  // u4FD_FaceMotionLockRat
          4,                  // u4FD_ImpulseLockCnt
         25,                  // u4FD_ImpulseUpperThd
         18,                  // u4FD_ImpulseLowerThd
          2,                  // u4FD_ROPTrustCnt
         30,                  // u4FD_ReCovergeWhenSizeChangeRat
        120,                  // u4FD_ReCovergeWhenPosChangeDist
          1,                  // u4FD_ReCovergeWhenAFDone;
          1                   // u4FD_OTFaceLock;
    };
    static strFaceWeight g_AEFaceWeight = {
        MFALSE,
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
    };
    static strAECWRTempSmooth g_AECWRTS =
    {
        TRUE,                                // enable AE HS Target temp smooth
        TRUE,                                // enable AE ABL Target temp smooth
        TRUE,                                // enable AE AOE Target temp smooth
        TRUE,                                // enable AE NS Target temp smooth
        TRUE,                                // enable AE Face Target temp smooth
        TRUE,                                // enable AE Video Target temp smooth
        TRUE,                               // enable AE Main Target temp smooth
        TRUE,                                // enable AE HSV4p0 Target temp smooth
    };
    static strFaceLandMarkCtrl g_AEFaceLandmark = {
        FALSE,                //bLandmarkCtrlFlag
        0,                    //u4LandmarkCV_Thd
        3,                    //u4LandmarkWeight
      100,                    //i4LandmarkExtRatW;
      100,                    //i4LandmarkExtRatH;
       90,                    //i4LandmarkTrustRopDegree;
        5,                    //i4RoundXYPercent;
        3,                    //i4RoundSZPercent;
        9,                    //i4THOverlap;
        3,                    //i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
        8,                    //i4SMOOTH_LEVEL;
        0,                    //i4MOMENTUM;
       50                     //u4LandmarkWeightPercent;
    };
    static strAEStableThd g_AEStableThd =
    {
        0,                      // u4InStableThd;  // 0.08EV
        1,                      // u4OutStableThd

        TRUE,             // enable ae different mode stable threshold setting
        1,                      // u4VideoInStableThd;  // 0.08EV
        1,                      // u4VideoOutStableThd
        1,                      // u4FaceInStableThd;  // 0.08EV
        2,                      // u4FaceOutStableThd
        15,                     // u4FaceOutB2TStableThd
        30,                     // u4FaceOutD2TStableThd
        0,                      // u4TouchInStableThd;  // 0.08EV
        1,                      // u4TouchOutStableThd
        FALSE,                  // Face Ae lock option
        FALSE                   // enable zero stable thd
    };

    static strAEMovingRatio g_AETouchMovingRatio =
    {
        100, //u4SpeedUpRatio
        100, //u4GlobalRatio
        190, //u4Bright2TargetEnd
        20,   //u4Dark2TargetStart
        80, //u4B2TEnd
        60,  //u4B2TStart
        40,  //u4D2TEnd
        80,  //u4D2TStart
    };

    static strHSFlatSkyCFG g_HSbyRealBV =
    {
        TRUE,                   //bEnableFlatSkyEnhance
        2,                          //uSkyDetInThd  : 2%
        5,                          //uSkyDetOutThd : 5%
        160,                    //u4FlatSkyTHD
        1500,                   //u4FlatSkyEVD
        {8000,  11000}, //u4BVRatio_X : BV
        {0, 1024}               //u4BVRatio_Y : Ratio
    };

    static strHSMultiStep g_HSMultiStep =
    {
       TRUE,                                            // bEnableMultiStepHS
       4,                                                   // u4TblLength
       {1100, 2600, 4500,6000},      // u4MS_EVDiff
       {75, 160, 160, 195},                 // u4MS_OutdoorTHD
       {75, 144, 160,160},                  // u4MS_IndoorTHD
       {5500,  8000},                           // i4MS_BVRatio
    };
    static AE_HIST_WIN_CFG_T g_AEBlockHistWINCFG =
    {
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
    };

    static AE_HIST_WIN_CFG_T g_AEPixelHistWINCFG =
    {
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
    };
    static strAOEBVRef g_strAOEByBV =
    {
       TRUE,                                       // bEnable
       4,                                               // u4TblLength
       {-2000, 0, 3000,5500},              // i4BV
       {150, 170, 180, 180}       // u4THD
    };
    static strABL_absDiff g_ABLabsDiff =
    {
        TRUE,                           //enable enhance ABL(absDiff)
        {2500,  5000},            //u4EVDiffRatio_X
        {0, 1024},                    //u4EVDiffRatio_Y
        {9000,  11000},         //u4BVRatio_X
        {0, 1024}                     //u4BVRatio_Y
    };
    static strNS_CDF g_strNSCDF =
    {
       TRUE,                                       // bEnable
       600,
       {-2000, -1000},
       {1024, 0}
    };
    static strHistStableCFG g_HistStableCFG =
    {
        TRUE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
        TRUE,                                           //bEnableHistOverExpoCnt
        6,                                                //u4OElevel : pseudo-high strength 6x
        2,                                                  //u4OERatio_LowBnd : 2% over exposure area
        20,                                                 //u4OERatio_HighBnd : 20% overexposure area
        {4000,  6000},                                   //i4OE_BVRatio_X[2]
        {0,     1024},                                     //u4OE_BVRatio_Y[2]
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
    };

    static strMainTargetCFG g_MainTargetCFG =
    {
        TRUE,               //bEnableAEMainTarget
        1024,               //u4MainTargetWeight
        {                         //u4MainTargetWeightTbl[15][15] : Gau weight table
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        },
        {   // TargetBVRatioTbl
            -2000,                   // TargetBVRatioTbl.i4X1 :BV
            42,                          // TargetBVRatioTbl.i4Y1 :Target
            4000,                   // TargetBVRatioTbl.i4X2  :BV
            52                          // TargetBVRatioTbl.i4Y2 :Target
        },
        TRUE,                   //bEnableColorWTRatio
        {   //ColorWTRatioTbl
            100,                            // ColorWTRatioTbl.i4X1 :ColorDiff
            1024,                          // ColorWTRatioTbl.i4Y1 :u4MainTargetWeight
            180,                        // ColorWTRatioTbl.i4X2  :ColorDiff
            512                          // ColorWTRatioTbl.i4Y2 :u4MainTargetWeight / 2
        },
        100,                           //uColorSupRatioR : 0~100
        100,                           //uColorSupRatioG : 0~100
        0,                               //uColorSupRatioB : 0~100
        {   //ColorSupBVRatioTbl
            4000,                            // ColorSupBVRatioTbl.i4X1 :BV
            0,                          // ColorSupBVRatioTbl.i4Y1 :Ratio
            7000,                        // ColorSupBVRatioTbl.i4X2  :BV
            1024                          // ColorSupBVRatioTbl.i4Y2 :Ratio
        },
    };
    static strHSv4p0CFG g_HSv4p0CFG =
    {
        TRUE,                   //bEnableHSv4p0
        1024,                   //u4HSv4p0Weight
        3,                          //u4BVSize ,max16
        4,                          //u4EVDSize ,max16
        {1000, 4500, 8000},     //i4HS_BVRatio, length = u4BVSize
        {80     ,      60,      15},    //u4HS_PcntRatio, length = u4BVSize, 80 equal 8%
        {1100, 2600, 4000,6500},    //u4HS_EVDRatio , length = u4EVDSize
        {                                       //u4HSTHDRatioTbl[u4BVSize][u4EVDSize] , HS THD
            //BV0-THD
            {90,    140,   160,  160},
             //BV1-THD
            {130, 150,   175,  200},
             //BV2-THD
            {140, 160,   195,  210},
        },
        TRUE,                   //bEnableDynWTRatio
        {       //DynWTRatioTbl
            0,                            // DynWTRatioTbl.i4X1 : min thd
            0,                              // DynWTRatioTbl.i4Y1 :add delta weight 1024base
            30,                         // DynWTRatioTbl.i4X2  :max thd
            256,                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        },
        FALSE,         //bEnableGreenSupress;
        120,          //uGreenGRRatio;
        130,          //uGreenGBRatio;
        70,           //uSupRatio;
        {256,512},    //u4HS_GCountRatio_X
        {0,1024},     //u4HS_GCountRatio_Y
    };

    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,       80,   80},   //u4BTTHD[16]
        {140, 160, 170},   //u4NSTHD[16]
    };

    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eMain2HDR_ON,           // eHdrParamID
		    {
            MTRUE,              // bHdrDetectorEnable
            80, // 20,          // i4HdrConfidenceThr
			      {
                TRUE,           // bHistDetectEnable

                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },


                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                60,             // i4ContrastHistRatioH
                450,            // i4ContrastHistRatioL
                50,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },

                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            {
                {
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight

                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                {
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight
                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    {           // rCenterDarkDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    }
                }
            },

        	/**************************************************/
        	// Feature name: HDR LV Information (for HDR detector)
        	// Calculating LV information for HDR detector
        	// Judge HDR scene by LV information, it helps to exclude the probability of night scene
        	// Influence: LV probability, HDR on/off
        	// rHdrLVInfo
        	/**************************************************/
            {
                TRUE,           // bLVDetectEnable
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },

        {
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            40, //20,           // i4RMGtRatio

            {                   // rRMGt
                160,            // i4X1
                512, //768,            // i4Y1
                220,            // i4X2
                384, //256, //512             // i4Y2
            },
            //Not used
            /**************************************************/
            2,                  // i4SupportHdrNum
            1024,               // i4AutoEnhanceProb
            1024,               // i4HdrWeight
            /**************************************************/

            // rAutoBright
            {

               	//LV 0  1  2  3  4  5  6  7  8  9  10 11 12 13  14  15 16 17 18    i4LVRatio50
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},
               	//LV 0  1  2  3  4  5  6  7  8  9  10 11 12 13  14  15 16 17 18    i4LVRatio60
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},

				{               	// rContrastRatio
                    3500, //4000, 	// i4X1
                    200,        	// i4Y1
                    4500, //5000, 	// i4X2
                    400         	// i4Y2
                },


                {               // rBrightProbRatio
                    600,        // i4X1
                    400,        // i4Y1
                   1100,        // i4X2
                    400         // i4Y2
                },

                {               // rNightProbRatio
                    0,          // i4X1
                    400,        // i4Y1
                    512,        // i4X2
                    200         // i4Y2
                },

                {
                    {               // rAcceProb
                        1100, //980,// i4X1
                        0,          // i4Y1
                        1500,       // i4X2
                        1024        // i4Y2
                    },
                    {               // rGyroProb
                        50, //100,  // i4X1
                        0,          // i4Y1
                        200, //700, // i4X2
                        1024        // i4Y2
                    },

                    {               // rMotionRatio
                        512,        // i4X1
                        400,        // i4Y1
                        960,        // i4X2
                        100         // i4Y2
                    },

                    512, //768,         // i4MotionOnThr
                    2               	//i4MotionOnCntThr
                }
            },

            {
                {130,130},      // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio
				//Not used
                /**************************************************/
                {70007, 66670}, // i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     // i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
				/**************************************************/

                {               // rIsoThr
                    12288,      //i4X1:1024 * 12
                    400,        //i4Y1
                    14336,      //i4X2:1024 * 10
                    100         //i4Y2
                },
            },

            {
                TRUE,           // bHdrWaitAEStable
                6,              // i4HdrInfoCounterThr

				//Not used
                /**************************************************/
                TRUE,           // bRatioSmoothEnable TRUE for platform / FALSE for simulation
				/**************************************************/
                0,              // i4AutoWeightMode
                4,              // i4AutoWeightSmooth
                4,              // i4AutoWeightStableThr
                0,              // i4RMGtStableMode
                4,              // i4RMGtStableThr
                0,              // i4HdrRatioStableMode
                4,              // i4HdrRatioStableThr
                { 2,  2},       // i4RatioSmoothThr
                {30, 60},       // i4RoughRatioUnit
                {15, 60},       // i4FinerRatioUnit

                //Not used
                /**************************************************/
                {128,384}       // i4RMGtUnit
				/**************************************************/
            },
            {                   // rAutoDark
                // TBD
            },
			//Not used
        	/**************************************************/
            {                   // rAutoTuning
                10,             // i4RmmExpoDiffThr %
                24              // i4LscRatio
            },
			/**************************************************/
            100              // i4FlashRatio
        },

        {
            47,                 // i4DefTarget
            0, //256,           // i4DefWeight

            {                   // rAutoEVWeight
                4000,           // rAutoEVWeight.i4X1
                256,            // rAutoEVWeight.i4Y1
                5000,           // rAutoEVWeight.i4X2
                512             // rAutoEVWeight.i4Y2
            },

            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            {                   // rAutoABLWeight
                0,              // rAutoABLWeight.i4X1
                1024,           // rAutoABLWeight.i4Y1
                512,            // rAutoABLWeight.i4X2
                1024            // rAutoABLWeight.i4Y2
            },

            {                   // rAutoLVWeightL
                 30,            // rAutoLVWeightL.i4X1
                768,            // rAutoLVWeightL.i4Y1
                 40,            // rAutoLVWeightL.i4X2
               1024             // rAutoLVWeightL.i4Y2
            },

            {                   // rAutoLVWeightH
                100,            // rAutoLVWeightH.i4X1
                256,            // rAutoLVWeightH.i4Y1
                130,            // rAutoLVWeightH.i4X2
                512             // rAutoLVWeightH.i4Y2
            },

            {                   // rAutoNightWeight
                1024,           // rAutoNightWeight.i4X1
                512,            // rAutoNightWeight.i4Y1
                2048,           // rAutoNightWeight.i4X2
                256             // rAutoNightWeight.i4Y2
            },

            {
                25, //20,       // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            {
                150,            // i4NightPercent
                {               // rNightTarget
                    0,          // i4X1
                    80,        	// i4Y1
                    1024,       // i4X2
                    60         	// i4Y2
                }
            },

            {
                60,             // rTargetLimitL.i4LVLimit

                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            {
                80,             // rTargetLimitM.i4LVLimit

                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            {
                120,            // rTargetLimitH.i4LVLimit
                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitH.rTargetMaxLimit
                    4000,       // rTargetLimitH.rTargetMaxLimit.i4X1
                    47,         // rTargetLimitH.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMaxLimit.i4X2
                    90          // rTargetLimitH.rTargetMaxLimit.i4Y2
                }
            }
        },
        {                       // rHdrMiscProp
            {                   // rFaceAEProp
                {               // rFaceEVProb
                   3800, //2000,        // i4X1
                    128, //512, //384, // 512,        // i4Y1
                   5000, //2700,        // i4X2
                   1024  //128 //256         // i4Y2
                },
                {               // rFaceHdrProb
                   2000,        // i4X1
                   1024, //512, //384, // 512,        // i4Y1
                   3200,        // i4X2
                    128  //128 //256         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        {
            40,                 // i4LEThr
            20,                 // i4SERatio
            180,                // i4SETarget
            1024                // i4BaseGain
        }
    };
    static strAESceneMapping g_strStereoPlineMapping =
    {
    {
    {LIB3A_AE_SCENE_AUTO, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
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
    {LIB3A_AE_SCENE_ISO100, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    },
    };

    static strAESceneMapping g_strHDRPlineMapping =
    {
    {
    {LIB3A_AE_SCENE_AUTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_NIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ACTION, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_BEACH, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_CANDLELIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_FIREWORKS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_LANDSCAPE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_NIGHT_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_PARTY, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_SNOW, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_SPORTS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_STEADYPHOTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_SUNSET, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_THEATRE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO_ANTI_SHAKE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO100, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO3200,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15,  AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO6400,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15,  AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO12800,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_VIDEO_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15,  AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    },
    };

    static strDynamicFlare g_strDynamicFlareCFG =
    {
        8,
        4,
        2
    };

    static AE_PARAM_T strAEInitPara =
    {
        g_AEParasetting,
        {
            &g_Weight_Matrix,
            &g_Weight_Spot,
            &g_Weight_Average,
            NULL
        },
        g_strAFPLineTable,
        g_strAFZSDPLineTable,
        g_strStrobePLineTable,
        g_strStrobeZSDPLineTable,
        &g_strEVValueArray[0],
        g_AEMoveTable,
        g_AEVideoMoveTable,
        g_AEFaceMoveTable,
        g_AETrackingMoveTable,
        g_AELimiterDataTable,
        g_VdoDynamicFpsTable,
        &g_AEBlockHistWINCFG,
        &g_AEPixelHistWINCFG,

        // v1.2
        FALSE,              // bOldAESmooth
        TRUE,               // bEnableSubPreIndex
        0,                      // u4VideoLPFWeight; // 0~23

        &g_FaceLocSizeCheck,
        &g_PerframeCFG,
        FALSE,              // Perframe AE smooth option
        FALSE,              // Perframe HDR AE smooth option

        FALSE,               // TG interrupt option
        FALSE, //Speedup Escape Overexposure
        FALSE, //Speedup Escape Underexposure
        &g_TgIntTuningParam,
        &g_TgEVCompJumpRatioOverExp[0],
	&g_TgEVCompJumpRatioUnderExp[0],
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

        //Open AE
        &g_AEStableThd,
        &g_AEBVAccRatio,
        &g_AEPsoConverge,
        &g_AENonCWRAcc,

        &g_AEFaceSmooth,
        &g_AEFaceWeight,
        &g_AECWRTS,
        &g_AEFaceLandmark,
        &g_strAOEByBV,
        &g_strNSCDF,

        &g_AETouchMovingRatio,
        &g_HSbyRealBV,
        &g_ABLabsDiff,
        &g_HSMultiStep,
        &g_HistStableCFG,
        &g_strOverExpoAOECFG,
        //v4.0
        FALSE,               //bAEv4p0MeterEnable;
        &g_MainTargetCFG,
        &g_HSv4p0CFG,
        &g_NSBVCFG,
        &g_strHdrAECFG,
        g_strStereoPlineMapping,
        g_strHDRPlineMapping,
        &g_strDynamicFlareCFG
    };

    return strAEInitPara;
}

template <>
AE_PARAM_T const&
getAUTOHDRAEParam<ESensorDev_MainSecond>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        TRUE,                // bEnablePreIndex
        TRUE,                // bEnableRotateWeighting;
        TRUE,               // bEV0TriggerStrobe
        FALSE,               // bLockCamPreMeteringWin;
        FALSE,               // bLockVideoPreMeteringWin;
        TRUE,                // bLockVideoRecMeteringWin;
        TRUE,                // bSkipAEinBirghtRange;
        TRUE,                // bPreAFLockAE
        TRUE,                // bStrobeFlarebyCapture
        TRUE,                // bEnableFaceAE
        TRUE,                // bEnableMeterAE
        TRUE,                // b4FlarMaxStepGapLimitEnable
        TRUE,                // bEnableAESmoothGain
        TRUE,                // bEnableLongCaptureCtrl
        FALSE,                //bEnableTouchSmoothRatio : enable Touch mode smooth converge
        FALSE,               // bEnableTouchSmooth : enable Touch mode smooth with peframe
        TRUE,                // bEnablePixelBaseHist
        TRUE,                // bEnableHDRSTATconfig   : enable HDR Y LSB config
        TRUE,                // bEnableAEOVERCNTconfig  : enable AE over-exposure count config
        TRUE,                // bEnableTSFSTATconfig    : enable TSF RGB STAT config
        1,                   // bEnableHDRLSB
        FALSE,                 //bEnableFlareFastConverge
        FALSE,                 //bEnableRAFastConverge
        FALSE,                 //bEnableFaceFastConverge
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
        79,                     // u4MinYLowBound;        // metering and face boundary min Y value
      256,                     // u4MaxYHighBound;      // metering and face boundary max Y value
     1024,                     // u4MeterWeight 1024 base.100%=>1024
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
        1,                   //uCycleNumAESkipAfterExit;
        254,                //overexposure cnt threshold
        11,                 //u4HSSmoothTHD
        1,                  //u4FinerEvIndexBase 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
    };

    static strWeightTable  g_Weight_Matrix =
    {
        AE_WEIGHTING_CENTRALWEIGHT,
        {
            {6 ,19 ,28 ,19 ,6 },
            {19 ,60 ,88 ,60 ,19 },
            {28 ,88 ,128+32 ,88 ,28 },
            {19 ,60 ,88 ,60 ,19 },
            {6 ,19 ,28 ,19 ,6 },
        }
    };

    static strWeightTable  g_Weight_Spot =
    {
        AE_WEIGHTING_SPOT,
        {
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
        }
    };

    static strWeightTable  g_Weight_Average =
    {
        AE_WEIGHTING_AVERAGE,
        {
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
        }
    };

    static strAFPlineInfo g_strAFPLineTable =     // from high lv to low lv
    {
         0,                  // 1 enable the AF Pline, 0 disable the AF Pline (bypass)
         {
             {73, 30},       // {LV value, AF frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strAFZSDPLineTable =     // from high lv to low lv
    {
        0,                  // 1 enable the AF ZSD Pline, 0 disable the AF ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, AF frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static strAFPlineInfo g_strStrobePLineTable =     // from high lv to low lv
    {
         1,                  // 1 enable the Strobe Pline, 0 disable the Strobe Pline (bypass)
         {
             {73, 30},       // {LV value, Strobe frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strStrobeZSDPLineTable =     // from high lv to low lv
    {
        1,                  // 1 enable the Strobe ZSD Pline, 0 disable the Strobe ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, Strobe frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static MUINT32 g_strEVValueArray[LIB3A_AE_EV_COMP_MAX] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEVideoMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEFaceMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AETrackingMoveTable[] =
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
    };


    // for AE limiter
    static strAELimiterTable g_AELimiterDataTable =
    {
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
    };

    // for video dynamic frame rate
    static VdoDynamicFrameRate_T g_VdoDynamicFpsTable =
    {
       TRUE, // isEnableDFps
       50,  // EVThresNormal
       50,  // EVThresNight
    };
    static strFaceLocSize g_FaceLocSizeCheck =
    {
        FALSE,       //bFaceLocSizeCheck;
        {20,    5},//u4Size_X[2];
        {1024, 0},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 1020}, //u4Loc_Y[2];

        FALSE,                                    //bFaceOECheck;
        {10240,    20480},            //u4OE_X[2];
        {1024, 768},                    //u4OE_Y[2];
        {10,  30},        //u4FCY_DX[2];
        {110,  95},        //u4FCY_BX[2];
        {1024, 0},         //u4FCY_Y[2];

        FALSE,                                    //bLandmarkSize;
    };
    static strPerframeCFG g_PerframeCFG =
    {
        200,
        80,
        80,
        240,
        4,
        254,
        510,
        1,
        TRUE,
    };
	static strAETgTuningPara g_TgIntTuningParam =
	{
		//B2T
		245,
		70,
		200,

		{80, 300, 500},  // STD = 1, 2, 3
		{100, 85, 60, 40},  // 100%, 85%, 60%, 50%


		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{-30, -24, -20, -20},

		200,
        245, //OverExpoLowBound
        0,   //OverExpLightAcc1
        0,   //OverExpLightAcc2
        0,   //OverExpLightAcc3
        0,   //OverExpLightAcc4
		//D2T
		7, //245
		95, //70%
		20,

		{80, 250, 500},  // STD = 1, 2, 3
		{100, 85, 60, 35},	// 100%, 85%, 60%, 50%

		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{20, 15, 10, 10},

		30
	};


    static MINT32 g_TgEVCompJumpRatioOverExp[LIB3A_AE_EV_COMP_MAX] =
    {
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
    };

	static MINT32 g_TgEVCompJumpRatioUnderExp[LIB3A_AE_EV_COMP_MAX] =
    {
       100,  // EV   0
       100,   //EV    0.1
       100,   //EV    0.2
       100,   //EV    0.3
       100,   //EV    0.4
       100,   //EV    0.5
       100,   //EV    0.6
       100,   //EV    0.7
       100,   //EV    0.8
       100,   //EV    0.9
       110,   //EV    1
       110,   //EV    1.1
       110,   //EV    1.2
       110,   //EV    1.3
       110,   //EV    1.4
       110,   //EV    1.5
       110,   //EV    1.6
       110,   //EV    1.7
       110,   //EV    1.8
       110,   //EV    1.9
       120,   //EV    2
       120,   //EV    2.1
       120,   //EV    2.2
       120,   //EV    2.3
       120,   //EV    2.4
       120,   //EV    2.5
       120,   //EV    2.6
       120,   //EV    2.7
       120,   //EV    2.8
       120,   //EV    2.9
       130,   //EV    3
       130,   //EV    3.1
       130,   //EV    3.2
       130,  //EV    3.3
       130,  //EV    3.4
       130,  //EV    3.5
       130,  //EV    3.6
       130,  //EV    3.7
       130,  //EV    3.8
       130,  //EV    3.9
       130,  //EV    4
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
    };



    static strBVAccRatio  g_AEBVAccRatio = {
        TRUE,
        90,
        220,
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)
        50,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
       4000,
      -4000
    };
    static strNonCWRAcc g_AENonCWRAcc =
    {
        FALSE,          //bNonCWRAccEnable enable
        255,
        160,
        160,            //it should  be <=hs_THD
        58,             //it should keep in target~hs_flat
    };
    static strFaceSmooth g_AEFaceSmooth = {
         10,                  // u4MinYLowBound;        // metering and face boundary min Y value
        256,                  // u4MaxYHighBound;      // metering and face boundary max Y value
          0,                  // i4FaceBVLowBound;     // face boundary min BV value
       4000,                  // i4FaceBVHighBound;    // face boundary max BV value
        235,                  // tempFDY HighBound
          1,                  // tempFDY LowBound
         20,                  // face lost max count
         100,                  // face window scale ratio range 1~400% default 90%
         35,                  // face NS lowbound
         10,                  // u4FD_Lock_MaxCnt
          5,                  // u4FD_MixCWR_MaxCnt
          0,                  // u4FD_TemporalSmoothY
         75,                  // u4FD_FaceWidthCropRat
         75,                  // u4FD_FaceHeightCropRat
          5,                  // u4FD_InToOutThdMaxCnt
        220,                  // u4FD_OTLockUpperBnd
          5,                  // u4FD_OTLockLowerBnd
          1,                  // u4FD_ContinueTrustCnt
          0,                  // u4FD_PerframeAntiOverFlag
          0,                  // u4FD_SideFaceLock
          1,                  // u4FD_LimitStableThdLowBnd
          1,                  // u4FD_LimitStableThdLowBndNum
          1,                  // u4FD_ReConvergeWhenFaceChange
          0,                  // u4FD_FaceMotionLockRat
          4,                  // u4FD_ImpulseLockCnt
         25,                  // u4FD_ImpulseUpperThd
         18,                  // u4FD_ImpulseLowerThd
          2,                  // u4FD_ROPTrustCnt
         30,                  // u4FD_ReCovergeWhenSizeChangeRat
        120,                  // u4FD_ReCovergeWhenPosChangeDist
          1,                  // u4FD_ReCovergeWhenAFDone;
          1                   // u4FD_OTFaceLock;
    };
    static strFaceWeight g_AEFaceWeight = {
        MFALSE,
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
    };
    static strAECWRTempSmooth g_AECWRTS =
    {
        TRUE,                                // enable AE HS Target temp smooth
        TRUE,                                // enable AE ABL Target temp smooth
        TRUE,                                // enable AE AOE Target temp smooth
        TRUE,                                // enable AE NS Target temp smooth
        TRUE,                                // enable AE Face Target temp smooth
        TRUE,                                // enable AE Video Target temp smooth
        TRUE,                               // enable AE Main Target temp smooth
        TRUE,                                // enable AE HSV4p0 Target temp smooth
    };
    static strFaceLandMarkCtrl g_AEFaceLandmark = {
        FALSE,                //bLandmarkCtrlFlag
        0,                    //u4LandmarkCV_Thd
        3,                    //u4LandmarkWeight
      100,                    //i4LandmarkExtRatW;
      100,                    //i4LandmarkExtRatH;
       90,                    //i4LandmarkTrustRopDegree;
        5,                    //i4RoundXYPercent;
        3,                    //i4RoundSZPercent;
        9,                    //i4THOverlap;
        3,                    //i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
        8,                    //i4SMOOTH_LEVEL;
        0,                    //i4MOMENTUM;
       50                     //u4LandmarkWeightPercent;
    };
    static strAEStableThd g_AEStableThd =
    {
        0,                      // u4InStableThd;  // 0.08EV
        1,                      // u4OutStableThd

        TRUE,             // enable ae different mode stable threshold setting
        1,                      // u4VideoInStableThd;  // 0.08EV
        1,                      // u4VideoOutStableThd
        1,                      // u4FaceInStableThd;  // 0.08EV
        2,                      // u4FaceOutStableThd
        15,                     // u4FaceOutB2TStableThd
        30,                     // u4FaceOutD2TStableThd
        0,                      // u4TouchInStableThd;  // 0.08EV
        1,                      // u4TouchOutStableThd
        FALSE,                  // Face Ae lock option
        FALSE                   // enable zero stable thd
    };

    static strAEMovingRatio g_AETouchMovingRatio =
    {
        100, //u4SpeedUpRatio
        100, //u4GlobalRatio
        190, //u4Bright2TargetEnd
        20,   //u4Dark2TargetStart
        80, //u4B2TEnd
        60,  //u4B2TStart
        40,  //u4D2TEnd
        80,  //u4D2TStart
    };

    static strHSFlatSkyCFG g_HSbyRealBV =
    {
        TRUE,                   //bEnableFlatSkyEnhance
        2,                          //uSkyDetInThd  : 2%
        5,                          //uSkyDetOutThd : 5%
        160,                    //u4FlatSkyTHD
        1500,                   //u4FlatSkyEVD
        {8000,  11000}, //u4BVRatio_X : BV
        {0, 1024}               //u4BVRatio_Y : Ratio
    };

    static strHSMultiStep g_HSMultiStep =
    {
       TRUE,                                            // bEnableMultiStepHS
       4,                                                   // u4TblLength
       {1100, 2600, 4500,6000},      // u4MS_EVDiff
       {75, 160, 160, 195},                 // u4MS_OutdoorTHD
       {75, 144, 160,160},                  // u4MS_IndoorTHD
       {5500,  8000},                           // i4MS_BVRatio
    };

    static AE_HIST_WIN_CFG_T g_AEBlockHistWINCFG =
    {
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
    };

    static AE_HIST_WIN_CFG_T g_AEPixelHistWINCFG =
    {
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
    };
    static strAOEBVRef g_strAOEByBV =
    {
       TRUE,                                       // bEnable
       4,                                               // u4TblLength
       {-2000, 0, 3000,5500},              // i4BV
       {150, 170, 180, 180}       // u4THD
    };
    static strABL_absDiff g_ABLabsDiff =
    {
        TRUE,                           //enable enhance ABL(absDiff)
        {2500,  5000},            //u4EVDiffRatio_X
        {0, 1024},                    //u4EVDiffRatio_Y
        {9000,  11000},         //u4BVRatio_X
        {0, 1024}                     //u4BVRatio_Y
    };
    static strNS_CDF g_strNSCDF =
    {
       TRUE,                                       // bEnable
       600,
       {-2000, -1000},
       {1024, 0}
    };
    static strHistStableCFG g_HistStableCFG =
    {
        TRUE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
        TRUE,                                           //bEnableHistOverExpoCnt
        6,                                                //u4OElevel : pseudo-high strength 6x
        2,                                                  //u4OERatio_LowBnd : 2% over exposure area
        20,                                                 //u4OERatio_HighBnd : 20% overexposure area
        {4000,  6000},                                   //i4OE_BVRatio_X[2]
        {0,     1024},                                     //u4OE_BVRatio_Y[2]
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
    };

    static strMainTargetCFG g_MainTargetCFG =
    {
        TRUE,               //bEnableAEMainTarget
        1024,               //u4MainTargetWeight
        {                         //u4MainTargetWeightTbl[15][15] : Gau weight table
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        },
        {   // TargetBVRatioTbl
            -2000,                   // TargetBVRatioTbl.i4X1 :BV
            42,                          // TargetBVRatioTbl.i4Y1 :Target
            4000,                   // TargetBVRatioTbl.i4X2  :BV
            52                          // TargetBVRatioTbl.i4Y2 :Target
        },
        TRUE,                   //bEnableColorWTRatio
        {   //ColorWTRatioTbl
            100,                            // ColorWTRatioTbl.i4X1 :ColorDiff
            1024,                          // ColorWTRatioTbl.i4Y1 :u4MainTargetWeight
            180,                        // ColorWTRatioTbl.i4X2  :ColorDiff
            512                          // ColorWTRatioTbl.i4Y2 :u4MainTargetWeight / 2
        },
        100,                           //uColorSupRatioR : 0~100
        100,                           //uColorSupRatioG : 0~100
        0,                               //uColorSupRatioB : 0~100
        {   //ColorSupBVRatioTbl
            4000,                            // ColorSupBVRatioTbl.i4X1 :BV
            0,                          // ColorSupBVRatioTbl.i4Y1 :Ratio
            7000,                        // ColorSupBVRatioTbl.i4X2  :BV
            1024                          // ColorSupBVRatioTbl.i4Y2 :Ratio
        },
    };
    static strHSv4p0CFG g_HSv4p0CFG =
    {
        TRUE,                   //bEnableHSv4p0
        1024,                   //u4HSv4p0Weight
        3,                          //u4BVSize ,max16
        4,                          //u4EVDSize ,max16
        {1000, 4500, 8000},     //i4HS_BVRatio, length = u4BVSize
        {80     ,      60,      15},    //u4HS_PcntRatio, length = u4BVSize, 80 equal 8%
        {1100, 2600, 4000,6500},    //u4HS_EVDRatio , length = u4EVDSize
        {                                       //u4HSTHDRatioTbl[u4BVSize][u4EVDSize] , HS THD
            //BV0-THD
            {90,    140,   160,  160},
             //BV1-THD
            {130, 150,   175,  200},
             //BV2-THD
            {140, 160,   195,  210},
        },
        TRUE,                   //bEnableDynWTRatio
        {       //DynWTRatioTbl
            0,                            // DynWTRatioTbl.i4X1 : min thd
            0,                              // DynWTRatioTbl.i4Y1 :add delta weight 1024base
            30,                         // DynWTRatioTbl.i4X2  :max thd
            256,                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        },
        FALSE,         //bEnableGreenSupress;
        120,          //uGreenGRRatio;
        130,          //uGreenGBRatio;
        70,           //uSupRatio;
        {256,512},    //u4HS_GCountRatio_X
        {0,1024},     //u4HS_GCountRatio_Y
    };
    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,       80,   80},   //u4BTTHD[16]
        {140,       160,   170},   //u4NSTHD[16]
    };

    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eMain2HDR_AUTO,         // eHdrParamID

        {
            MTRUE,              // bHdrDetectorEnable
            220, // 20,                       // i4HdrConfidenceThr

            {
                TRUE,           // bHistDetectEnable

                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },


                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                60,             // i4ContrastHistRatioH
                450,            // i4ContrastHistRatioL
                50,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },

                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            {
                {
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight

                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                {
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    {           // rCenterDarkDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    }
                }
            },
            {
                TRUE,           // bLVDetectEnable
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },

        {
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            40, //20,                 // i4RMGtRatio

            {                   // rRMGt
                160,            // i4X1
                512, //768,            // i4Y1
                220,            // i4X2
                384 //256, //512             // i4Y2
            },

            //Not used
            /**************************************************/
            2,                  // i4SupportHdrNum
            1024,               // i4AutoEnhanceProb
            1024,               // i4HdrWeight
            /**************************************************/

            // rAutoBright
            {
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio50
                {   100, 100, 100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200,  200,  200,  200, 200, 200, 200},
			   //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio60
                {   100, 100, 100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200,  200,  200,  200, 200, 200, 200},

                {               // rContrastRatio
                    3500, //4000, // i4X1
                    100,        // i4Y1
                    4500, //5000, // i4X2
                    200         // i4Y2
                },

                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                {               // rNightProbRatio
                    0,          // i4X1
                    200,          // i4Y1
                    512,        // i4X2
                    100           // i4Y2
                },

                {
                    {               // rAcceProb
                        1100, //980,        // i4X1
                        0,          // i4Y1
                        1500,       // i4X2
                        1024        // i4Y2
                    },

                    {               // rGyroProb
                        50, //100,        // i4X1
                        0,          // i4Y1
                        200, //700,        // i4X2
                        1024        // i4Y2
                    },

                    {               // rMotionRatio
                        512,        // i4X1
                        400,          // i4Y1
                        960,        // i4X2
                        100           // i4Y2
                    },

                    512, //768,            // i4MotionOnThr
                    2               //i4MotionOnCntThr
                }
            },

            {
                {130,130},        // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, // i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     // i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/


                {               // rIsoThr
                    12288,      //i4X1:1024 * 12
                    400,        //i4Y1
                    14336,      //i4X2:1024 * 10
                    100         //i4Y2
                },
            },

            {
                TRUE,           // bHdrWaitAEStable
                6, //6,         // i4HdrInfoCounterThr

                //Not used
                /**************************************************/
                TRUE,           // bRatioSmoothEnable TRUE for platform / FALSE for simulation
                /**************************************************/
                0,              // i4AutoWeightMode
                4,              // i4AutoWeightSmooth
                4,              // i4AutoWeightStableThr
                0,              // i4RMGtStableMode
                4,              // i4RMGtStableThr
                0,              // i4HdrRatioStableMode
                4,              // i4HdrRatioStableThr
                { 2,  2},      // i4RatioSmoothThr
                {30, 60},       // i4RoughRatioUnit
                {15, 60},       // i4FinerRatioUnit
                //Not used
                /**************************************************/
                {128,384}       // i4RMGtUnit
                /**************************************************/
            },
            {                   // rAutoDark
                // TBD
            },

            //Not used
            /**************************************************/
            {                   // rAutoTuning
                10,             // i4RmmExpoDiffThr %
                24              // i4LscRatio
            },
            /**************************************************/
            100              // i4FlashRatio
        },

        {
            47,                 // i4DefTarget
            0, //256,                // i4DefWeight

            {                   // rAutoEVWeight
                4000,           // rAutoEVWeight.i4X1
                256,            // rAutoEVWeight.i4Y1
                5000,           // rAutoEVWeight.i4X2
                512             // rAutoEVWeight.i4Y2
            },

            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            {                   // rAutoABLWeight
                0,              // rAutoABLWeight.i4X1
                1024,           // rAutoABLWeight.i4Y1
                512,            // rAutoABLWeight.i4X2
                1024            // rAutoABLWeight.i4Y2
            },

            {                   // rAutoLVWeightL
                 30,            // rAutoLVWeightL.i4X1
                768,            // rAutoLVWeightL.i4Y1
                 40,            // rAutoLVWeightL.i4X2
               1024             // rAutoLVWeightL.i4Y2
            },

            {                   // rAutoLVWeightH
                100,            // rAutoLVWeightH.i4X1
                256,            // rAutoLVWeightH.i4Y1
                130,            // rAutoLVWeightH.i4X2
                512             // rAutoLVWeightH.i4Y2
            },

            {                   // rAutoNightWeight
                1024,           // rAutoNightWeight.i4X1
                512,            // rAutoNightWeight.i4Y1
                2048,           // rAutoNightWeight.i4X2
                256             // rAutoNightWeight.i4Y2
            },

            {
                25, //20,                // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            {
                150,             // i4NightPercent

                {               // rNightTarget
                    0,          // i4X1
                    80,        // i4Y1
                    1024,       // i4X2
                    60         // i4Y2
                }
            },

            {
                60,             // rTargetLimitL.i4LVLimit

                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            {
                80,             // rTargetLimitM.i4LVLimit

                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            {
                120,            // rTargetLimitH.i4LVLimit

                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitH.rTargetMaxLimit
                    4000,       // rTargetLimitH.rTargetMaxLimit.i4X1
                    47,         // rTargetLimitH.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMaxLimit.i4X2
                    90          // rTargetLimitH.rTargetMaxLimit.i4Y2
                }
            }
        },
        {                       // rHdrMiscProp
            {                   // rFaceAEProp
                {               // rFaceEVProb
                   3800, //2000,        // i4X1
                    128, //512, //384, // 512,        // i4Y1
                   5000, //2700,        // i4X2
                   1024  //128 //256         // i4Y2
                },
                {               // rFaceHdrProb
                   2000,        // i4X1
                   1024, //512, //384, // 512,        // i4Y1
                   3200,        // i4X2
                    128  //128 //256         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        {
            40,                 // i4LEThr
            20,                 // i4SERatio
            180,                // i4SETarget
            1024                // i4BaseGain
        }
    };
    static strAESceneMapping g_strStereoPlineMapping =
    {
    {
    {LIB3A_AE_SCENE_AUTO, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
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
    {LIB3A_AE_SCENE_ISO100, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    },
    };

    static strAESceneMapping g_strHDRPlineMapping =
    {
    {
    {LIB3A_AE_SCENE_AUTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_NIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ACTION, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_BEACH, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_CANDLELIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_FIREWORKS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_LANDSCAPE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_NIGHT_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_PARTY, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_SNOW, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_SPORTS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_STEADYPHOTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_SUNSET, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_THEATRE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO_ANTI_SHAKE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO100, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO3200,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17,  AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO6400,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17,  AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_ISO12800,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX18, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17,  AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17, AETABLE_SCENE_INDEX17}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    },
    };

    static strDynamicFlare g_strDynamicFlareCFG =
    {
        8,
        4,
        2
    };

    static AE_PARAM_T strAEInitPara =
    {
        g_AEParasetting,
        {
            &g_Weight_Matrix,
            &g_Weight_Spot,
            &g_Weight_Average,
            NULL
        },
        g_strAFPLineTable,
        g_strAFZSDPLineTable,
        g_strStrobePLineTable,
        g_strStrobeZSDPLineTable,
        &g_strEVValueArray[0],
        g_AEMoveTable,
        g_AEVideoMoveTable,
        g_AEFaceMoveTable,
        g_AETrackingMoveTable,
        g_AELimiterDataTable,
        g_VdoDynamicFpsTable,
        &g_AEBlockHistWINCFG,
        &g_AEPixelHistWINCFG,

        // v1.2
        FALSE,              // bOldAESmooth
        TRUE,               // bEnableSubPreIndex
        0,                      // u4VideoLPFWeight; // 0~23

        &g_FaceLocSizeCheck,
        &g_PerframeCFG,
        FALSE,              // Perframe AE smooth option
        FALSE,              // Perframe HDR AE smooth option

        FALSE,               // TG interrupt option
        TRUE, //Speedup Escape Overexposure
        TRUE, //Speedup Escape Underexposure
	&g_TgIntTuningParam,
	&g_TgEVCompJumpRatioOverExp[0],
	&g_TgEVCompJumpRatioUnderExp[0],
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
        //Open AE
        &g_AEStableThd,
        &g_AEBVAccRatio,
        &g_AEPsoConverge,
        &g_AENonCWRAcc,

        &g_AEFaceSmooth,
        &g_AEFaceWeight,
        &g_AECWRTS,
        &g_AEFaceLandmark,
        &g_strAOEByBV,
        &g_strNSCDF,

        &g_AETouchMovingRatio,
        &g_HSbyRealBV,
        &g_ABLabsDiff,
        &g_HSMultiStep,
        &g_HistStableCFG,
        &g_strOverExpoAOECFG,
        //v4.0
        FALSE,               //bAEv4p0MeterEnable;
        &g_MainTargetCFG,
        &g_HSv4p0CFG,
        &g_NSBVCFG,
        &g_strHdrAECFG,
        g_strStereoPlineMapping,
        g_strHDRPlineMapping,
        &g_strDynamicFlareCFG
    };

    return strAEInitPara;
}

template <>
AE_PARAM_T const&
getVTAEParam<ESensorDev_MainSecond>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        TRUE,                // bEnablePreIndex
        TRUE,                // bEnableRotateWeighting;
        TRUE,               // bEV0TriggerStrobe
        FALSE,               // bLockCamPreMeteringWin;
        FALSE,               // bLockVideoPreMeteringWin;
        TRUE,                // bLockVideoRecMeteringWin;
        TRUE,                // bSkipAEinBirghtRange;
        TRUE,                // bPreAFLockAE
        TRUE,                // bStrobeFlarebyCapture
        TRUE,                // bEnableFaceAE
        TRUE,                // bEnableMeterAE
        TRUE,                // b4FlarMaxStepGapLimitEnable
        TRUE,                // bEnableAESmoothGain
        TRUE,                // bEnableLongCaptureCtrl
        FALSE,                //bEnableTouchSmoothRatio : enable Touch mode smooth converge
        FALSE,               // bEnableTouchSmooth : enable Touch mode smooth with peframe
        TRUE,                // bEnablePixelBaseHist
        TRUE,                // bEnableHDRSTATconfig   : enable HDR Y LSB config
        TRUE,                // bEnableAEOVERCNTconfig  : enable AE over-exposure count config
        TRUE,                // bEnableTSFSTATconfig    : enable TSF RGB STAT config
        1,                   // bEnableHDRLSB
        FALSE,                 //bEnableFlareFastConverge
        FALSE,                 //bEnableRAFastConverge
        FALSE,                 //bEnableFaceFastConverge
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
        79,                     // u4MinYLowBound;        // metering and face boundary min Y value
      256,                     // u4MaxYHighBound;      // metering and face boundary max Y value
     1024,                     // u4MeterWeight 1024 base.100%=>1024
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
        1,                   //uCycleNumAESkipAfterExit;
        254,                //overexposure cnt threshold
        11,                 //u4HSSmoothTHD
        1,                  //u4FinerEvIndexBase 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
    };

    static strWeightTable  g_Weight_Matrix =
    {
        AE_WEIGHTING_CENTRALWEIGHT,
        {
            {6 ,19 ,28 ,19 ,6 },
            {19 ,60 ,88 ,60 ,19 },
            {28 ,88 ,128+32 ,88 ,28 },
            {19 ,60 ,88 ,60 ,19 },
            {6 ,19 ,28 ,19 ,6 },
        }
    };

    static strWeightTable  g_Weight_Spot =
    {
        AE_WEIGHTING_SPOT,
        {
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0},
            {0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0},
        }
    };

    static strWeightTable  g_Weight_Average =
    {
        AE_WEIGHTING_AVERAGE,
        {
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
        }
    };

    static strAFPlineInfo g_strAFPLineTable =     // from high lv to low lv
    {
         0,                  // 1 enable the AF Pline, 0 disable the AF Pline (bypass)
         {
             {73, 30},       // {LV value, AF frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strAFZSDPLineTable =     // from high lv to low lv
    {
        0,                  // 1 enable the AF ZSD Pline, 0 disable the AF ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, AF frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static strAFPlineInfo g_strStrobePLineTable =     // from high lv to low lv
    {
         1,                  // 1 enable the Strobe Pline, 0 disable the Strobe Pline (bypass)
         {
             {73, 30},       // {LV value, Strobe frame rate}
             {65, 25},
             {55, 19},
             {45, 17},
             {0, 17},
         }
    };

    static strAFPlineInfo g_strStrobeZSDPLineTable =     // from high lv to low lv
    {
        1,                  // 1 enable the Strobe ZSD Pline, 0 disable the Strobe ZSD Pline (bypass)
        {
            {73, 15},       // {LV value, Strobe frame rate}
            {65, 15},
            {55, 15},
            {45, 15},
            {0, 10},
        }
    };

    static MUINT32 g_strEVValueArray[LIB3A_AE_EV_COMP_MAX] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEVideoMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AEFaceMoveTable[] =
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
    };

    // total 24 sets
    static strAEMOVE  g_AETrackingMoveTable[] =
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
    };


    // for AE limiter
    static strAELimiterTable g_AELimiterDataTable =
    {
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
    };

    // for video dynamic frame rate
    static VdoDynamicFrameRate_T g_VdoDynamicFpsTable =
    {
       TRUE, // isEnableDFps
       50,  // EVThresNormal
       50,  // EVThresNight
    };
    static strFaceLocSize g_FaceLocSizeCheck =
    {
        FALSE,       //bFaceLocSizeCheck;
        {20,    5},//u4Size_X[2];
        {1024, 0},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 1024}, //u4Loc_Y[2];

        FALSE,                                    //bFaceOECheck;
        {10240,    20480},            //u4OE_X[2];
        {1024, 768},                    //u4OE_Y[2];
        {10,  30},        //u4FCY_DX[2];
        {110,  95},        //u4FCY_BX[2];
        {1024, 0},         //u4FCY_Y[2];

        FALSE,                                    //bLandmarkSize;
    };
    static strPerframeCFG g_PerframeCFG =
    {
        200,
        80,
        80,
        240,
        4,
        254,
        510,
        1,
        TRUE,
    };
	static strAETgTuningPara g_TgIntTuningParam =
	{
		//B2T
		245,
		70,
		200,

		{80, 300, 500},  // STD = 1, 2, 3
		{100, 85, 60, 40},  // 100%, 85%, 60%, 50%


		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{-30, -24, -20, -20},

		200,
        245, //OverExpoLowBound
        0,   //OverExpLightAcc1
        0,   //OverExpLightAcc2
        0,   //OverExpLightAcc3
        0,   //OverExpLightAcc4
		//D2T
		7, //245
		95, //70%
		20,

		{80, 250, 500},  // STD = 1, 2, 3
		{100, 85, 60, 35},	// 100%, 85%, 60%, 50%

		{30, 40, 50},  //Acc EV = 3EV, 4EV, 5EV
		{20, 15, 10, 10},

		30
	};


    static MINT32 g_TgEVCompJumpRatioOverExp[LIB3A_AE_EV_COMP_MAX] =
    {
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
    };

	static MINT32 g_TgEVCompJumpRatioUnderExp[LIB3A_AE_EV_COMP_MAX] =
    {
       100,  // EV   0
       100,   //EV    0.1
       100,   //EV    0.2
       100,   //EV    0.3
       100,   //EV    0.4
       100,   //EV    0.5
       100,   //EV    0.6
       100,   //EV    0.7
       100,   //EV    0.8
       100,   //EV    0.9
       110,   //EV    1
       110,   //EV    1.1
       110,   //EV    1.2
       110,   //EV    1.3
       110,   //EV    1.4
       110,   //EV    1.5
       110,   //EV    1.6
       110,   //EV    1.7
       110,   //EV    1.8
       110,   //EV    1.9
       120,   //EV    2
       120,   //EV    2.1
       120,   //EV    2.2
       120,   //EV    2.3
       120,   //EV    2.4
       120,   //EV    2.5
       120,   //EV    2.6
       120,   //EV    2.7
       120,   //EV    2.8
       120,   //EV    2.9
       130,   //EV    3
       130,   //EV    3.1
       130,   //EV    3.2
       130,  //EV    3.3
       130,  //EV    3.4
       130,  //EV    3.5
       130,  //EV    3.6
       130,  //EV    3.7
       130,  //EV    3.8
       130,  //EV    3.9
       130,  //EV    4
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
    };


    static strBVAccRatio  g_AEBVAccRatio = {
        TRUE,
        90,
        220,
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)
        50,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
       4000,
      -4000
    };
    static strNonCWRAcc g_AENonCWRAcc =
    {
        FALSE,          //bNonCWRAccEnable enable
        255,
        160,
        160,            //it should  be <=hs_THD
        58,             //it should keep in target~hs_flat
    };
    static strFaceSmooth g_AEFaceSmooth = {
         10,                  // u4MinYLowBound;        // metering and face boundary min Y value
        256,                  // u4MaxYHighBound;      // metering and face boundary max Y value
          0,                  // i4FaceBVLowBound;     // face boundary min BV value
       4000,                  // i4FaceBVHighBound;    // face boundary max BV value
        235,                  // tempFDY HighBound
          1,                  // tempFDY LowBound
         20,                  // face lost max count
         100,                  // face window scale ratio range 1~400% default 90%
         35,                  // face NS lowbound
         10,                  // u4FD_Lock_MaxCnt
          5,                  // u4FD_MixCWR_MaxCnt
          0,                  // u4FD_TemporalSmoothY
         75,                  // u4FD_FaceWidthCropRat
         75,                  // u4FD_FaceHeightCropRat
          5,                  // u4FD_InToOutThdMaxCnt
        220,                  // u4FD_OTLockUpperBnd
          5,                  // u4FD_OTLockLowerBnd
          1,                  // u4FD_ContinueTrustCnt
          0,                  // u4FD_PerframeAntiOverFlag
          0,                  // u4FD_SideFaceLock
          1,                  // u4FD_LimitStableThdLowBnd
          1,                  // u4FD_LimitStableThdLowBndNum
          1,                  // u4FD_ReConvergeWhenFaceChange
          0,                  // u4FD_FaceMotionLockRat
          4,                  // u4FD_ImpulseLockCnt
         25,                  // u4FD_ImpulseUpperThd
         18,                  // u4FD_ImpulseLowerThd
          2,                  // u4FD_ROPTrustCnt
         30,                  // u4FD_ReCovergeWhenSizeChangeRat
        120,                  // u4FD_ReCovergeWhenPosChangeDist
          1,                  // u4FD_ReCovergeWhenAFDone;
          1                   // u4FD_OTFaceLock;
    };
    static strFaceWeight g_AEFaceWeight = {
        MFALSE,
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
    };
    static strAECWRTempSmooth g_AECWRTS =
    {
        TRUE,                                // enable AE HS Target temp smooth
        TRUE,                                // enable AE ABL Target temp smooth
        TRUE,                                // enable AE AOE Target temp smooth
        TRUE,                                // enable AE NS Target temp smooth
        TRUE,                                // enable AE Face Target temp smooth
        TRUE,                                // enable AE Video Target temp smooth
        TRUE,                               // enable AE Main Target temp smooth
        TRUE,                                // enable AE HSV4p0 Target temp smooth
    };
    static strFaceLandMarkCtrl g_AEFaceLandmark = {
        FALSE,                //bLandmarkCtrlFlag
        0,                    //u4LandmarkCV_Thd
        3,                    //u4LandmarkWeight
      100,                    //i4LandmarkExtRatW;
      100,                    //i4LandmarkExtRatH;
       90,                    //i4LandmarkTrustRopDegree;
        5,                    //i4RoundXYPercent;
        3,                    //i4RoundSZPercent;
        9,                    //i4THOverlap;
        3,                    //i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
        8,                    //i4SMOOTH_LEVEL;
        0,                    //i4MOMENTUM;
       50                     //u4LandmarkWeightPercent;
    };
    static strAEStableThd g_AEStableThd =
    {
        0,                      // u4InStableThd;  // 0.08EV
        1,                      // u4OutStableThd

        TRUE,             // enable ae different mode stable threshold setting
        1,                      // u4VideoInStableThd;  // 0.08EV
        1,                      // u4VideoOutStableThd
        1,                      // u4FaceInStableThd;  // 0.08EV
        2,                      // u4FaceOutStableThd
        15,                     // u4FaceOutB2TStableThd
        30,                     // u4FaceOutD2TStableThd
        0,                      // u4TouchInStableThd;  // 0.08EV
        1,                      // u4TouchOutStableThd
        FALSE,                  // Face Ae lock option
        FALSE                   // enable zero stable thd
    };

    static strAEMovingRatio g_AETouchMovingRatio =
    {
        100, //u4SpeedUpRatio
        100, //u4GlobalRatio
        190, //u4Bright2TargetEnd
        20,   //u4Dark2TargetStart
        80, //u4B2TEnd
        60,  //u4B2TStart
        40,  //u4D2TEnd
        80,  //u4D2TStart
    };

    static strHSFlatSkyCFG g_HSbyRealBV =
    {
        TRUE,                   //bEnableFlatSkyEnhance
        2,                          //uSkyDetInThd  : 2%
        5,                          //uSkyDetOutThd : 5%
        160,                    //u4FlatSkyTHD
        1500,                   //u4FlatSkyEVD
        {8000,  11000}, //u4BVRatio_X : BV
        {0, 1024}               //u4BVRatio_Y : Ratio
    };

    static strHSMultiStep g_HSMultiStep =
    {
       TRUE,                                            // bEnableMultiStepHS
       4,                                                   // u4TblLength
       {1100, 2600, 4500,6000},      // u4MS_EVDiff
       {75, 160, 160, 195},                 // u4MS_OutdoorTHD
       {75, 144, 160,160},                  // u4MS_IndoorTHD
       {5500,  8000},                           // i4MS_BVRatio
    };

    static AE_HIST_WIN_CFG_T g_AEBlockHistWINCFG =
    {
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
    };

    static AE_HIST_WIN_CFG_T g_AEPixelHistWINCFG =
    {
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
    };
    static strAOEBVRef g_strAOEByBV =
    {
       TRUE,                                       // bEnable
       4,                                               // u4TblLength
       {-2000, 0, 3000,5500},              // i4BV
       {150, 170, 180, 180}       // u4THD
    };
    static strABL_absDiff g_ABLabsDiff =
    {
        TRUE,                           //enable enhance ABL(absDiff)
        {2500,  5000},            //u4EVDiffRatio_X
        {0, 1024},                    //u4EVDiffRatio_Y
        {9000,  11000},         //u4BVRatio_X
        {0, 1024}                     //u4BVRatio_Y
    };
    static strNS_CDF g_strNSCDF =
    {
       TRUE,                                       // bEnable
       600,
       {-2000, -1000},
       {1024, 0}
    };
    static strHistStableCFG g_HistStableCFG =
    {
        TRUE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
        TRUE,                                           //bEnableHistOverExpoCnt
        6,                                                //u4OElevel : pseudo-high strength 6x
        2,                                                  //u4OERatio_LowBnd : 2% over exposure area
        20,                                                 //u4OERatio_HighBnd : 20% overexposure area
        {4000,  6000},                                   //i4OE_BVRatio_X[2]
        {0,     1024},                                     //u4OE_BVRatio_Y[2]
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
    };

    static strMainTargetCFG g_MainTargetCFG =
    {
        TRUE,               //bEnableAEMainTarget
        1024,               //u4MainTargetWeight
        {                         //u4MainTargetWeightTbl[15][15] : Gau weight table
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
            {0,	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        },
        {   // TargetBVRatioTbl
            -2000,                   // TargetBVRatioTbl.i4X1 :BV
            42,                          // TargetBVRatioTbl.i4Y1 :Target
            4000,                   // TargetBVRatioTbl.i4X2  :BV
            52                          // TargetBVRatioTbl.i4Y2 :Target
        },
        FALSE,                   //bEnableColorWTRatio
        {   //ColorWTRatioTbl
            100,                            // ColorWTRatioTbl.i4X1 :ColorDiff
            1024,                          // ColorWTRatioTbl.i4Y1 :u4MainTargetWeight
            300,                           // ColorWTRatioTbl.i4X2  :ColorDiff
            768                            // ColorWTRatioTbl.i4Y2 :u4MainTargetWeight / 2
        },
        100,                               //uColorSupRatioR : 0~100
        100,                               //uColorSupRatioG : 0~100
        50,                                //uColorSupRatioB : 0~100
        {   //ColorSupBVRatioTbl
            5000,                        // ColorSupBVRatioTbl.i4X1 :BV
            0,                          // ColorSupBVRatioTbl.i4Y1 :Ratio
            6500,                       // ColorSupBVRatioTbl.i4X2  :BV
            1024                          // ColorSupBVRatioTbl.i4Y2 :Ratio
        },
    };
    static strHSv4p0CFG g_HSv4p0CFG =
    {
        TRUE,                   //bEnableHSv4p0
        1024,                   //u4HSv4p0Weight
        3,                          //u4BVSize ,max16
        4,                          //u4EVDSize ,max16
        {1000, 4500, 8000},     //i4HS_BVRatio, length = u4BVSize
        {80     ,      60,      15},    //u4HS_PcntRatio, length = u4BVSize, 80 equal 8%
        {1100, 2600, 4000,6500},    //u4HS_EVDRatio , length = u4EVDSize
        {                                       //u4HSTHDRatioTbl[u4BVSize][u4EVDSize] , HS THD
            //BV0-THD
            {90,    140,   160,  160},
             //BV1-THD
            {130, 150,   175,  200},
             //BV2-THD
            {140, 160,   195,  210},
        },
        TRUE,                   //bEnableDynWTRatio
        {       //DynWTRatioTbl
            0,                            // DynWTRatioTbl.i4X1 : min thd
            0,                              // DynWTRatioTbl.i4Y1 :add delta weight 1024base
            30,                         // DynWTRatioTbl.i4X2  :max thd
            256,                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        },
        FALSE,         //bEnableGreenSupress;
        120,          //uGreenGRRatio;
        130,          //uGreenGBRatio;
        70,           //uSupRatio;
        {256,512},    //u4HS_GCountRatio_X
        {0,1024},     //u4HS_GCountRatio_Y
    };
    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,    80,   80},   //u4BTTHD[16]
        {140,   160,   170},   //u4NSTHD[16]
    };
    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eMain2HDR_VT,           // eHdrParamID

        {
            MTRUE,              // bHdrDetectorEnable
            80, // 20,                       // i4HdrConfidenceThr

            {
                TRUE,           // bHistDetectEnable

                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },


                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                30,             // i4ContrastHistRatioH
                300,            // i4ContrastHistRatioL
                30,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },

                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            {
                {
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight

                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                {
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },


                    {           // rCenterDarkDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    }
                }
            },

            {
                TRUE,           // bLVDetectEnable
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },

        {
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            20,                 // i4RMGtRatio

            {                   // rRMGt
                160,            // i4X1
                512,            // i4Y1
                220,            // i4X2
                384             // i4Y2
            },

            //Not used
            /**************************************************/
            2,                  // i4SupportHdrNum
            1024,               // i4AutoEnhanceProb
            1024,               // i4HdrWeight
            /**************************************************/

            // rAutoBright
            {
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio50
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio60
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},

                {               // rContrastRatio
                    3000,       // i4X1
                    200,        // i4Y1
                    4000,       // i4X2
                    400         // i4Y2
                },

                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                {               // rNightProbRatio
                    0,          // i4X1
                    400,        // i4Y1
                    512,        // i4X2
                    200         // i4Y2
                },

                {
                    {               // rAcceProb
                        1500,       // i4X1
                        0,          // i4Y1
                        2500,       // i4X2
                        1024        // i4Y2
                    },

                    {               // rGyroProb
                        100,        // i4X1
                        0,          // i4Y1
                        700,        // i4X2
                        1024        // i4Y2
                    },

                    {               // rMotionRatio
                        512,        // i4X1
                        400,          // i4Y1
                        960,        // i4X2
                        100           // i4Y2
                    },


					768,            // i4MotionOnThr
                    5               // i4MotionOnCntThr
                }
            },

            {
                {130, 130},     // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, // i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     // i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/

                {               // rIsoThr
                    14336,      //i4X1:1024 * 12
                    100,        //i4Y1
                    12288,      //i4X2:1024 * 10
                    400         //i4Y2
                },
            },

            {
                TRUE,           // bHdrWaitAEStable
                6,              // i4HdrInfoCounterThr

                //Not used
                /**************************************************/
                FALSE,          // bRatioSmoothEnable TRUE for platform / FALSE for simulation
               /**************************************************/
                0,              // i4AutoWeightMode
                4,              // i4AutoWeightSmooth
                4,              // i4AutoWeightStableThr
                0,              // i4RMGtStableMode
                4,              // i4RMGtStableThr
                0,              // i4HdrRatioStableMode
                4,              // i4HdrRatioStableThr
                { 5,   2},      // i4RatioSmoothThr
                {30, 60},       // i4RoughRatioUnit
                {15, 60},       // i4FinerRatioUnit

                //Not used
                /**************************************************/
                {64, 128}       // i4RMGtUnit
                /**************************************************/
            },
            {                   // rAutoDark
                // TBD
            },

            //Not used
            /**************************************************/
            {                   // rAutoTuning
                10,             // i4RmmExpoDiffThr %
                24              // i4LscRatio
            },
		 	/**************************************************/
            100              // i4FlashRatio
        },

        {
            47,                 // i4DefTarget
            256,                // i4DefWeight

            {                   // rAutoEVWeight
                4000,           // rAutoEVWeight.i4X1
                256,            // rAutoEVWeight.i4Y1
                5000,           // rAutoEVWeight.i4X2
                512             // rAutoEVWeight.i4Y2
            },

            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            {                   // rAutoABLWeight
                0,              // rAutoABLWeight.i4X1
                1024,           // rAutoABLWeight.i4Y1
                512,            // rAutoABLWeight.i4X2
                1024            // rAutoABLWeight.i4Y2
            },

            {                   // rAutoLVWeightL
                 30,            // rAutoLVWeightL.i4X1
                768,            // rAutoLVWeightL.i4Y1
                 40,            // rAutoLVWeightL.i4X2
               1024             // rAutoLVWeightL.i4Y2
            },

            {                   // rAutoLVWeightH
                100,            // rAutoLVWeightH.i4X1
                256,            // rAutoLVWeightH.i4Y1
                130,            // rAutoLVWeightH.i4X2
                512             // rAutoLVWeightH.i4Y2
            },

            {                   // rAutoNightWeight
                1024,           // rAutoNightWeight.i4X1
                512,            // rAutoNightWeight.i4Y1
                2048,           // rAutoNightWeight.i4X2
                256             // rAutoNightWeight.i4Y2
            },

            {
                25, //20,                // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            {
                150,             // i4NightPercent

                {               // rNightTarget
                    0,          // i4X1
                    80,        // i4Y1
                    1024,       // i4X2
                    60         // i4Y2
                }
            },

            {
                60,             // rTargetLimitL.i4LVLimit

                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            {
                80,             // rTargetLimitM.i4LVLimit

                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            {
                120,            // rTargetLimitH.i4LVLimit

                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                {               // rTargetLimitH.rTargetMaxLimit
                    4000,       // rTargetLimitH.rTargetMaxLimit.i4X1
                    47,         // rTargetLimitH.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMaxLimit.i4X2
                    90          // rTargetLimitH.rTargetMaxLimit.i4Y2
                }
            }
        },
        {                       // rHdrMiscProp
            {                   // rFaceAEProp
                {               // rFaceEVProb
                   3800, //2000,        // i4X1
                    128, //512, //384, // 512,        // i4Y1
                   5000, //2700,        // i4X2
                   1024  //128 //256         // i4Y2
                },
                {               // rFaceHdrProb
                   2000,        // i4X1
                   1024, //512, //384, // 512,        // i4Y1
                   3200,        // i4X2
                    128  //128 //256         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        {
            40,                 // i4LEThr
            20,                 // i4SERatio
            180,                // i4SETarget
            1024                // i4BaseGain
        }
    };
    static strAESceneMapping g_strStereoPlineMapping =
    {
    {
    {LIB3A_AE_SCENE_AUTO, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX12}},
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
    {LIB3A_AE_SCENE_ISO100, {AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO200}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO400}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO800}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600, AETABLE_SCENE_INDEX13, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_SCENE_INDEX11, AETABLE_CAPTURE_ISO1600}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO,AETABLE_CAPTURE_AUTO,AETABLE_VIDEO_AUTO,AETABLE_VIDEO1_AUTO,AETABLE_VIDEO2_AUTO,AETABLE_CUSTOM1_AUTO,AETABLE_CUSTOM2_AUTO,AETABLE_CUSTOM3_AUTO,AETABLE_CUSTOM4_AUTO,AETABLE_CUSTOM5_AUTO,AETABLE_CAPTURE_AUTO}},    //reserve for future
    },
    };

    static strAESceneMapping g_strHDRPlineMapping =
    {
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
    };

    static strDynamicFlare g_strDynamicFlareCFG =
    {
        8,
        4,
        2
    };

    static AE_PARAM_T strAEInitPara =
    {
        g_AEParasetting,
        {
            &g_Weight_Matrix,
            &g_Weight_Spot,
            &g_Weight_Average,
            NULL
        },
        g_strAFPLineTable,
        g_strAFZSDPLineTable,
        g_strStrobePLineTable,
        g_strStrobeZSDPLineTable,
        &g_strEVValueArray[0],
        g_AEMoveTable,
        g_AEVideoMoveTable,
        g_AEFaceMoveTable,
        g_AETrackingMoveTable,
        g_AELimiterDataTable,
        g_VdoDynamicFpsTable,
        &g_AEBlockHistWINCFG,
        &g_AEPixelHistWINCFG,

        // v1.2
        FALSE,              // bOldAESmooth
        TRUE,               // bEnableSubPreIndex
        0,                      // u4VideoLPFWeight; // 0~23

        &g_FaceLocSizeCheck,
        &g_PerframeCFG,
        FALSE,              // Perframe AE smooth option
        FALSE,              // Perframe HDR AE smooth option

        FALSE,               // TG interrupt option
        TRUE, //Speedup Escape Overexposure
        TRUE, //Speedup Escape Underexposure
        &g_TgIntTuningParam,
        &g_TgEVCompJumpRatioOverExp[0],
	&g_TgEVCompJumpRatioUnderExp[0],
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
        //Open AE
        &g_AEStableThd,
        &g_AEBVAccRatio,
        &g_AEPsoConverge,
        &g_AENonCWRAcc,

        &g_AEFaceSmooth,
        &g_AEFaceWeight,
        &g_AECWRTS,
        &g_AEFaceLandmark,
        &g_strAOEByBV,
        &g_strNSCDF,

        &g_AETouchMovingRatio,
        &g_HSbyRealBV,
        &g_ABLabsDiff,
        &g_HSMultiStep,
        &g_HistStableCFG,
        &g_strOverExpoAOECFG,
        //v4.0
        FALSE,               //bAEv4p0MeterEnable;
        &g_MainTargetCFG,
        &g_HSv4p0CFG,
        &g_NSBVCFG,
        &g_strHdrAECFG,
        g_strStereoPlineMapping,
        g_strHDRPlineMapping,
        &g_strDynamicFlareCFG
    };

    return strAEInitPara;
}



/*******************************************************************************
*
********************************************************************************/
template <>
MBOOL
isAEEnabled<ESensorDev_MainSecond>()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AE_CYCLE_NUM (3)

template <>
const MINT32*
getAEActiveCycle<ESensorDev_MainSecond>()
{
    // Default AE cycle
    static MINT32 i4AEActiveCycle[AE_CYCLE_NUM] =
    {
        MTRUE,
        MFALSE,
        MFALSE,
    };

    return (&i4AEActiveCycle[0]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
MINT32
getAECycleNum<ESensorDev_MainSecond>()
{
    return AE_CYCLE_NUM;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Param Mapping Table by Scenario
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_PREVIEW>() {
    return getAEParam<ESensorDev_MainSecond>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_VIDEO>() {
    return getAEParam<ESensorDev_MainSecond>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CAPTURE>() {
    return getAEParam<ESensorDev_MainSecond>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM1>() {
    return getHDRAEParam<ESensorDev_MainSecond>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM2>() {
    return getAUTOHDRAEParam<ESensorDev_MainSecond>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM3>() {
    return getVTAEParam<ESensorDev_MainSecond>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM4>() {
    return getAEParam<ESensorDev_MainSecond>();
}
