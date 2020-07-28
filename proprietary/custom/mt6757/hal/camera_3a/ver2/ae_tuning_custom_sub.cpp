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
getAEParam<ESensorDev_Sub>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        FALSE,                // bEnablePreIndex
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
        1,                    //bEnableFlareFastConverge
        1,                    //bEnableRAFastConverge
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
        1,                  //uCycleNumAESkipAfterExit;
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
        {35,    10},//u4Size_X[2];
        {1024, 256},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 256} //u4Loc_Y[2];
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)        
        70,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
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
          5,                  // face lost max count
         90,                  // face window scale ratio range 1~400% default 90%
         30,                //face NS lowbound
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
        {-2000, 0, 3000,5500},         // i4BV
        {150, 170, 180, 180}            // u4THD
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
       {-1000, -300},
       {1024, 	0},
    };
    static strHistStableCFG g_HistStableCFG =
    {
        FALSE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
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
        TRUE,                           //bEnableColorSuppressR
        TRUE,                           //bEnableColorSuppressG
        FALSE                         //bEnableColorSuppressB
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
            256                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        }
    };

    static strNSBVCFG g_NSBVCFG =
    {
        TRUE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,       80,   80},   //u4BTTHD[16]
    };

    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eSubHDR_OFF,            // eHdrParamID

        /**************************************************/
        // Feature name: HDR detector parameter information
        // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
        // rHdrDetector
        /**************************************************/
        {             
            /**************************************************/                   
            // Variable name: bHdrDetectorEnable
            // A switch to turn on/off of HDR detector
            // Default value: MTRUE.
            // Data range: MTRUE or MFALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR detector
            //
            // Variable name: i4HdrConfidenceThr
            // Threshold to turn HDR on/off
            // Unit: %
            // Default value: 80 or 220
            // Data range: Integer, i4HdrConfidenceThr > 0
            // Constraints: N/A.
            // Effect: Turn HDR on when result of HDR scene probability is larger than i4HdrConfidenceThr
            /**************************************************/
            MTRUE,              // bHdrDetectorEnable
            80, // 20,          // i4HdrConfidenceThr

            /**************************************************/
            // Feature name: rHdrHistInfo
            // HDR detector - histogram information calculation
            // Detect HDR scene by calculating full RGB histogram information
            // Find the proportion of pixel at bright/dark side of an image to check HDR characteristics in histogram
            /**************************************************/
            {                  
                /**************************************************/                
                // Variable name: bHistDetectEnable
                // A switch to turn on/off histogram analysis of HDR detector
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable histogram analysis of HDR detector
                /**************************************************/
                TRUE,           // bHistDetectEnable

                /**************************************************/
                // Feature name: HDR detector - dark weight
                // Dark side weighting table for HDR detector
                // Multiply full RGB histogram with dark side weight and add the result of bright side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the dark part of image
                // A pixel will be treated as a dark pixel, if its pixel value is lower than certain threshold
                // Here are the parameters to control the threshold of dark pixel and the corresponding weighting
                // rHdrDetectDarkWtWin
                //
                // Varaible name: i4HdrHistP0
                // Dark side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 6
                // Data range: Integer, 0 < i4HdrHistP0 < i4HdrHistP1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the more histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistP1
                // Dark side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 12
                // Data range: Integer, i4HdrHistP0 < i4HdrHistP1 <= 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the more histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistW0
                // Dark side higher pixel weight
                // Default value: 100
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins before i4HdrHistP0 will get higher fully weight
                //
                // Varaible name: i4HdrHistW1
                // Dark side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                /**************************************************/
                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },

                /**************************************************/
                // Feature name: HDR detector - bright weight
                // Bright side weighting table for HDR detector
                // Multiply full RGB histogram with bright side weight and add the result of dark side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the bright part of image
                // A pixel will be treated as a bright pixel, if its pixel value is higher than certain threshold
                // Influence: HDR probability, HDR on/off
                // rHdrDetectBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 180
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: HDR auto enhance bright weight
                // Weighting table, enhance histogram bright side
                // Multiply full RGB histogram with the bright side weight
                // It is used to calculate bright enhance probability
                // Influence: HDR auto weight, HDR ratio
                // rHdrEnhanceBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: rHdrFDBrightWtWin
                // Histogram from bright side weighting table for face detection
                // Multiply full RGB histogram with the face detection bright side weight
                // It is used to calculate face detection bright tone probability
                // Influence: HDR face probability
                // rHdrFDBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Face detection bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Face detection bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Face detection bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Face detection bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                /**************************************************/
                // Varaible name: i4HdrContrastPercentH
                // Bright side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 60
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrContrastPercentL
                // Dark side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightEVDiffPercentH
                // Bright side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 50 or 30
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher the i4HdrNightEVDiffPercentH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrNightEVDiffPercentL
                // Dark side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4HdrNightEVDiffPercentL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightDarkPercent
                // Night scene histogram Y %
                // It is used to calculate i4NightHistProb
                // Unit: 10*%
                // Default value: 500
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: It affects the calculation of histogram Y. The percentage depends on your purpose.
                /**************************************************/
                60,             // i4ContrastHistRatioH
                450,            // i4ContrastHistRatioL
                50,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                /**************************************************/
                // Feature name: HDR night scene EV difference probability
                // The probability table interpolating for night scene probability by EV difference
                // Influence: night EVD probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightEVDiffProb
                //
                // Varaible name: rHdrNightEVDiffProb.i4X1
                // Min EV difference threshold to get min night scene probability (i4Y1)
                // Unit: EV*1000
                // Default value: 4800
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X1, larger EV difference zone will get min night scene probability
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y1
                // Min night scene probability according to i4X1
                // Unit: 1024*%/100 
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 0, i4X1 would be the cutoff EV difference
                //         (EV difference < i4X1 will get zero probability)
                //         If i4Y1 > 0, we will get probability >= i4Y1
                //         (If i4Y1 > 0, we have certain faith of night scene for any EV difference input)
                //
                // Varaible name: rHdrNightEVDiffProb.i4X2
                // Max EV difference threshold to get max night scene probability (i4Y2)
                // Unit: EV*1000
                // Default value: 6500
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X2, the smaller zone of EV difference get max night scene probability (i4Y2)
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y2
                // Max night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 1024, the EV difference >= i4X2 will get night scene probability of i4Y2
                /**************************************************/
                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },
                /**************************************************/
                // Feature name: HDR night scene histogram probability
                // The probability table for night scene probability by histogram avg Y
                // Influence: night histogram probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightHistProb
                //
                // Varaible name: rHdrNightHistProb.i4X1
                // Max histogram avg Y threshold to get max night scene probability (i4Y1)
                // Unit: Y value in 8 bits
                // Default value: 2
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The avg Y <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightHistProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4X1 = 1024, the avg Y <= i4X1 will get 100% night scene probability
                //         If i4X1 < 1024, it means that we're not 100% sure for night scene, judging by avg Y
                //
                // Varaible name: rHdrNightHistProb.i4X2
                // Min histogram avg Y threshold to get min night scene probability (i4Y2)
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4X2, the higher avg Y we need to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightHistProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, then i4X2 is the cutoff avg Y to exclude the possibility of night scene
                /**************************************************/
                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                /**************************************************/
                // Feature name: HDR night scene LV probability
                // The probability table for night scene probability by scene LV
                // Influence: night LV probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                //  rHdrNightLVProb
                // Varaible name: rHdrNightLVProb.i4X1
                // Max scene LV threshold to get max night scene probability (i4Y1)
                // Unit: 10*LV
                // Default value: 50
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: Scene LV <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightLVProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 1024, scene LV <= i4X1 will get 100% night scene probability
                //         If i4Y1 < 1024, it means that we cannot 100% exclude the posibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4X2
                // Min scene LV threshold to get min night scene probability (i4Y2)
                // Unit: 10*LV
                // Default value: 100
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: The higher i4X2, higher scene LV is needed to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, i4X2 is the cutoff scene LV to exclude the possibility of night scene
                /**************************************************/
                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            /**************************************************/
            // Feature name: HDR geometry information (for HDR detector)
            // Calculating for HDR detector by geometry information
            // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
            // rHdrGeometryInfo
            /**************************************************/
            {                 
                /**************************************************/
                // Feature name: geometry center over exposure probability
                // HDR scene share the same property as center over exposure
                // rGeometryCOEProp
                /**************************************************/
                {             
                    /**************************************************/             
                    // Variable name: bGeometryCOEEnable
                    // A switch for HDR detector by geometry COE
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry COE
                    //
                    // Variable name: i4COEWeight
                    // Weight to control the importance of COE in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If COE is important to detect the HDR scene, set i4COEWeight >= 1024.
                    /**************************************************/  
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight
                    /**************************************************/
                    // Feature name: HDR center over-exposure outer ratio
                    // The outer geometry average information of AE statistics for HDR detector
                    // Influence: COE outer probability, COE outer weight probability
                    // rHdrCOEOuterRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer average threshold to get max COE probability (i4Y1)
                    // Unit: Y value in 8 bits
                    // Default value: 23
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The outer average <= i4X1 will get max COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Max COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 1024, the outer average <= i4X1 will get 100% COE probability
                    //         If i4Y1 <= 1024, it means that we're not 100% sure for COE, judging by outer average
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer average threshold to get min COE probability (i4Y2)
                    // Unit: Y value in 8 bits
                    // Default value: 47
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The higher i4X2, higher outer average is needed to exclude the possibility of COE
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Min COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 0, i4X2 is the cutoff outer average to exclude the possibility of COE
                    /**************************************************/
                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center over-exposure difference ratio
                    // The outer/inner geometry EV difference of AE statistics for HDR detector
                    // Influence: COE difference probability, COE difference weight probability, HDR on/off
                    // rHdrCOEDiffRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer/inner difference threshold to get min COE probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 1900
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger outer/inner geometry EV difference zone will get min COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Min COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff outer/inner geometry EV difference
                    //         (EV difference < i4X1 will get zero probability)
                    //         If i4Y1 > 0, we will get probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of COE for any outer/inner geometry input)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer/inner difference threshold to get max COE probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 2400
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of outer/inner geometry EV difference get max COE probability (i4Y2)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Max COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the outer/inner EV difference >= i4X2 will get COE probability of i4Y2
                    /**************************************************/
                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                /**************************************************/
                // Feature name: HDR anti-back-light(ABL) probability
                // HDR scene share the same property as back-light
                // rGeometryABLProp
                /**************************************************/
                {           
                    /**************************************************/
                    // Variable name: bGeometryABLEnable
                    // A switch for HDR detector by geometry ABL
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry ABL
                    //
                    // Variable name: i4ABLWeight
                    // Weight to control the importance of ABL in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If ABL is important to detect the HDR scene, set i4ABLWeight >= 1024.
                    /**************************************************/
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    /**************************************************/
                    // Feature name: HDR center bright difference ratio
                    // The ABL target probability table when center is brighter than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterBrightDiffRatio
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center dark difference ratio
                    // The ABL target probability table when center is darker than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterDarkDiffRatio
                    // Varaible name: rCenterDarkDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100   
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100   
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
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
                /**************************************************/
                // Variable name: bLVDetectEnable
                // A switch to use LV to detect HDR scene
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable HDR LV analysis
                //
                // Varaible name: i4LVProbTbl
                // LV table for detecting HDR scene
                // Unit: %
                // Default value: { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
                // Data range: Integer, 0 ~ 100
                // Constraints: N/A.
                // Effect: Set the probability of HDR scene according to LV information
                /**************************************************/
                TRUE,          // bLVDetectEnable
               
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },
        
        /**************************************************/
        // Feature name: HDR auto-enhance
        // Calculating information for HDR ratio
        // rHdrAutoEnhance
        /**************************************************/
        {                 
            /**************************************************/
            // Variable name: bAutoEnhanceEnable
            // A switch to calculate HDR ratio information
            // Default value: TRUE.
            // Data range: TRUE or FALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR BV analysis
            //
            // Varaible name: i4RMGk
            // Control the height of turning point to LE/SE curve
            // Default value: 4095
            // Data range: Integer, 0 ~ 4095
            // Constraints: N/A.
            // Effect: Set i4RMGk higher(lower) to make LE longer(shorter)
            //
            // Varaible name: i4RMGtRatio
            // Set the percentage of current Y for calculating RMGt
            // Unit: 10*%
            // Default value: 40
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The higher i4RMGtRatio, more percentage of histogram will be accumulated in current Y
            /**************************************************/
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            20,                 // i4RMGtRatio

            /**************************************************/
            // Feature name: RMGt
            // Interpolation table for RMGt
            // Influence: the turning point between LE/SE curve
            // If current Y is large(small), use data of SE(LE) more
            // rRMGt
            //
            // Varaible name: rRMGt.i4X1
            // Max current Y threshold to get max RMGt i4Y1
            // Unit: Y value in 8 bits
            // Default value: 160
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X1, larger current Y zone will get max RMGt setting (i4Y1)
            //         The current Y lower than i4Y1 will get longer LE curve.
            //
            // Varaible name: rRMGt.i4Y1
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 512
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y1, the longer LE curve we will get
            //
            // Varaible name: rRMGt.i4X2
            // Min current Y to get min RMGt i4Y2
            // Unit: Y value in 8 bits
            // Default value: 220
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller current Y zone will get min RMGt setting (i4Y2)
            //         The current Y higher than i4Y1 will get shorter LE curve.
            //
            // Varaible name: rRMGt.i4Y2
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 384
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y2, the longer LE curve we will gets
            /**************************************************/
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
                /**************************************************/
                // Varaible name: i4LVRatio50
                // Ratio interpolation table by scene LV, for 50 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                //
                // Varaible name: i4LVRatio60
                // Ratio interpolation table by scene LV, for 60 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                /**************************************************/
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio50
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio60
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},

                /**************************************************/
                // Feature name: rContrastRatio
                // HDR ratio interpolation table by HDR contrast EV difference
                // Influence: auto contrast ratio, HDR ratio
                // rContrastRatio
                //
                // Varaible name: rContrastRatio.i4X1
                // Max contrast EVD threshold to get min HDR ratio i4Y1
                // Unit: EV*1000
                // Default value: 3500
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger contrast EVD zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rContrastRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 100
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rContrastRatio.i4X2
                // Min contrast EVD to get max HDR ratio i4Y2
                // Unit: EV*1000
                // Default value: 4500
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller contrast EVD zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rContrastRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rContrastRatio
                    3500,       // i4X1
                    200,        // i4Y1
                    4500,       // i4X2
                    400         // i4Y2
                },

                /**************************************************/
                // Feature name: rBrightRatio
                // HDR ratio interpolation table by HDR bright side histogram information
                // Influence: auto bright probability, HDR ratio
                //rBrightRatio
                //
                // Varaible name: rBrightRatio.i4X1
                // Max bright side histogram threshold to get HDR ratio i4Y1
                // Unit: weighting average pixel count
                // Default value: 600
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger bright side histogram zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rBrightRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rBrightRatio.i4X2
                // Min bright side histogram to get max HDR ratio i4Y2
                // Unit: weighting average pixel count
                // Default value: 1100
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller bright side histogram zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rBrightRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: rNightProbRatio
                // HDR ratio interpolation table by night scene probability
                // Influence: auto night probability, HDR ratio
                // rNightProbRatio
                // Varaible name: rNightProbRatio.i4X1
                // Max night scene probability threthold to get max HDR ratio i4Y1
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rNightProbRatio.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rNightProbRatio.i4X2
                // Min night scene probability to get max HDR ratio i4Y2
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rNightProbRatio.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rNightProbRatio
                    0,          // i4X1
                    400,        // i4Y1
                    512,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: motion information
                // The motion corresponding information including gyro, accelerator,
                // Influence: HDR ratio, HDR on/off, on/off smoothness
                // rGGyroSensor
                /**************************************************/
                {                 
                    /**************************************************/
                    // Feature name: rAcceProb
                    // Acceleration probability interpolation table by accelerator information
                    // Influence: HDR ratio, motion on/off
                    // rAcceProb
                    //
                    // Varaible name: rAcceProb.i4X1
                    // Max acceleration threthold to get min acceleration probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger acceleration zone will get min acceleration probability (i4Y1)
                    //
                    // Varaible name: rAcceProb.i4Y1
                    // Min acceleration probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff acceleration to get min acceleration probability
                    //
                    // Varaible name: rAcceProb.i4X2
                    // Min acceleration to get max acceleration probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller acceleration zone will get max acceleration probability (i4Y2)
                    //
                    // Varaible name: rAcceProb.i4Y2
                    // Max acceleration probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the acceleration > i4X2 will get 100% acceleration probability
                    /**************************************************/
                    {               // rAcceProb
                        1500,       // i4X1
                        0,          // i4Y1
                        2500,       // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rGyroProb
                    // Motion probability interpolation table by gyro sensor information
                    // Influence: HDR ratio, motion on/off
                    // rGyroProb
                    //
                    // Varaible name: rGyroProb.i4X1
                    // Max motion threthold to get min motion probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion zone will get min motion probability (i4Y1)
                    //
                    // Varaible name: rGyroProb.i4Y1
                    // Min motion probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff motion to get min motion probability
                    //
                    // Varaible name: rGyroProb.i4X2
                    // Min motion to get max motion probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion zone will get max motion probability (i4Y2)
                    //
                    // Varaible name: rGyroProb.i4Y2
                    // Max motion probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the motion > i4X2 will get 100% motion probability
                    /**************************************************/
                    {               // rGyroProb
                        100,        // i4X1
                        0,          // i4Y1
                        700,        // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rMotionRatio
                    // HDR ratio interpolation table by motion (gyro + acce) probability information
                    // Influence: HDR ratio
                    // rMotionRatio
                    //
                    // Varaible name: rMotionRatio.i4X1
                    // Max motion probability threthold to get min HDR ratio i4Y1
                    // Unit: 2048*%/100
                    // Default value: 512
                    // Data range: Integer, 0 ~ 2048
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion probability zone will get max HDR ratio (i4Y1)
                    //
                    // Varaible name: rMotionRatio.i4Y1
                    // Max HDR ratio according to i4X1
                    // Unit: 100*LE/SE
                    // Default value: 400
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    //
                    // Varaible name: rMotionRatio.i4X2
                    // Min motion probability to get max HDR ratio i4Y2
                    // Unit: 2048*%/100
                    // Default value: 960
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion probability zone will get min HDR ratio (i4Y2)
                    //
                    // Varaible name: rMotionRatio.i4Y2
                    // Min HDR ratio according to i4X2
                    // Unit: 100*LE/SE
                    // Default value: 100
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    /**************************************************/
                    {               // rMotionRatio
                        512,        // i4X1
                        400,          // i4Y1
                        960,        // i4X2
                        100           // i4Y2
                    },

                    /**************************************************/
                    // Varaible name: i4MotionOnThr
                    // Threshold to determine motion on/off
                    // Unit: 1024*%/100
                    // Default value: 512 or 768
                    // Data range: Integer, i4MotionOnThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnThr, you need higher motion probability to set motion detector on
                    //
                    // Varaible name: i4MotionOnCntThr
                    // Counting threshold to update motion on/off information
                    // Unit: frame count
                    // Default value: 2 or 5
                    // Data range: Integer, i4MotionOnCntThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnCntThr, you need count more to update motion on/off
                    /**************************************************/
                    768,            // i4MotionOnThr
                    5               // i4MotionOnCntThr
                }
            },

            /**************************************************/
            // Feature name: rAutoFlicker
            // Parameter setting to automatically avoid flicker
            // rAutoFlicker
            /**************************************************/
            {                 
                /**************************************************/
                // Varaible name: i4Flkr1LVThr
                // LV threshold for deflicker enable
                // Unit: LV*10
                // Default value: {130,130}
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The higher i4Flkr1LVThr, the higher scene LV needed to enable deflicker
                //
                // Varaible name: i4MinRatioAntiFlk
                // The min acceptable ratio for anti-flicker
                // Unit: 100*LE/SE
                // Default value: 200 or 800
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The ratio lower i4MinRatioAntiFlk than could get flickered results
                /**************************************************/
                {130, 130},     // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, 	// i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     	// i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/

                /**************************************************/
                // Feature name: rIsoThr
                // HDR ratio interpolation table by ISO information
                // Influence: HDR ratio
                // rIsoThr
                //
                // Varaible name: rIsoThr.i4X1
                // Max ISO threshold to get max HDR ratio i4Y1
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger ISO zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rIsoThr.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rIsoThr.i4X2
                // Min ISO to get min HDR ratio i4Y2
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller ISO zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rIsoThr.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               	// rIsoThr
                    14336,      	//i4X1:1024 * 14
                    1,          	//i4Y1
                    12288,      	//i4X2:1024 * 12
                    4           	//i4Y2
                },
            },

            /**************************************************/
            // Feature name: rAutoSmooth
            // HDR auto smooth (HDR on/off, motion on/off, HDR ratio smooth) information
            // rAutoSmooth
            /**************************************************/
            {           
                /**************************************************/
                // Variable name: bHdrWaitAEStable
                // A switch to determine wait for AE stable or not
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable wait
                //
                // Variable name: i4HdrInfoCounterThr
                // Threshold to update HDR on/off information
                // Unit: frame count
                // Default value: 6
                // Data range: Integer, i4HdrInfoCounterThr > 0
                // Constraints: N/A.
                // Effect: HDR on/off counter larger than i4HdrInfoCounterThr can update the on/off result
                /**************************************************/
                TRUE,           	// bHdrWaitAEStable
                6,              	// i4HdrInfoCounterThr

                //Not used
                /**************************************************/
                FALSE,          	// bRatioSmoothEnable TRUE for platform / FALSE for simulation
                /**************************************************/

                /**************************************************/
                // Variable name: i4RatioSmoothThr
                // Threshold to update HDR ratio smooth information
                // Unit: frame count
                // Default value: none
                // Data range: Integer, i4RatioSmoothThr > 0
                // Constraints: N/A.
                // Effect: HDR ratio smooth counter larger than i4RatioSmoothThr can update the smoothed HDR ratio result
                //
                // Variable name: i4RoughRatioUnit
                // Threshold to check if difference of current ratio and current stable ratio is large enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4RoughRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current ratio and current stable ratio larger than i4RoughRatioUnit check for close results in incoming frames
                //
                // Variable name: i4FinerRatioUnit
                // Threshold to check if difference of current/previous ratio is small enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4FinerRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current/previous HDR ratio > i4FinerRatioUnit start to count
                /**************************************************/
                { 5,   2},      	// i4RatioSmoothThr
                {30, 60},         // i4RoughRatioUnit
                {15, 60},         // i4FinerRatioUnit
                
                //Not used
                /**************************************************/
                {64, 128}       	// i4RMGtUnit
                /**************************************************/
            },
            {                   	// rAutoDark
                // TBD
            },
            //Not used
            /**************************************************/
            {                   // rAutoTuning
                10,             // i4RmmExpoDiffThr %
                24              // i4LscRatio
            }
            /**************************************************/
        },

        /**************************************************/
        // Feature name: rHdrAETarget
        // HDR AE target parameter setting
        // rHdrAETarget
        /**************************************************/
        {   
            /**************************************************/
            // Variable name: i4DefTarget
            // Default AE target
            // Unit: Y value in 8 bits
            // Default value: 47
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: A reference of AE target, set higher to make AE target higher
            //
            // Variable name: i4DefWeight
            // Weight of default target, mixing with normal AE target
            // Default value: 47
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: Set i4DefWeight higher to make the proportion of default AE target higher
            /**************************************************/
            47,                 	// i4DefTarget
            256,                	// i4DefWeight

            /**************************************************/
            // Feature name: rAutoEVWeight
            // HDR auto EV weight interpolation table
            // Influence: auto EV weight, HDR auto weight
            // rAutoEVWeight
            //
            // Varaible name: rAutoEVWeight.i4X1
            // Max contrast EVD threshold to get min auto EV weight (i4Y1)
            // Unit: EV*1000
            // Default value: 4000 or 3750
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger contrast EVD zone will get min auto EV weight (i4Y1)
            //
            // Varaible name: rAutoEVWeight.i4Y1
            // Min auto EV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoEVWeight.i4X2
            // Min contrast EVD threshold to get max auto EV weight i4Y2
            // Unit: EV*1000
            // Default value: 4500 or 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller contrast EVD zone will get max auto EV weight (i4Y2)
            //
            // Varaible name: rAutoEVWeight.i4Y2
            // Max auto EV weight according to i4X2
            // Unit: 1024*%/100
            // Default value:  512 or 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The contrast EVD larger than i4X2 will get auto EV weight of i4Y2
            /**************************************************/
            {                   	// rAutoEVWeight
                4000,          	 	// rAutoEVWeight.i4X1
                256,            	// rAutoEVWeight.i4Y1
                5000,           	// rAutoEVWeight.i4X2
                512             	// rAutoEVWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoBrightWeight
            // HDR auto bright weight interpolation table
            // Influence: auto bright weight, auto night weight, HDR auto weight
            // rAutoBrightWeight
            //
            // Varaible name: rAutoBrightWeight.i4X1
            // Max bright enhance probability threshold to get min auto bright weight (i4Y1)
            // Unit: weighting average pixel count
            // Default value: 4000
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger bright enhance probability zone will get min auto bright weight (i4Y1)
            //
            // Varaible name: rAutoBrightWeight.i4Y1
            // Min auto bright weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoBrightWeight.i4X2
            // Min bright enhance probability threshold to get max auto bright weight i4Y2
            // Unit: weighting average pixel count
            // Default value: 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller bright enhance probability zone will get max auto bright weight (i4Y2)
            //
            // Varaible name: rAutoBrightWeight.i4Y2
            // Max auto bright weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The bright enhance probability larger than i4X2 will get max auto bright weight of i4Y2
            /**************************************************/
            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoABLWeight
            // HDR auto ABL weight interpolation table
            // Influence: auto ABL weight, auto night weight, HDR auto weight
            // rAutoABLWeight
            //
            // Varaible name: rAutoABLWeight.i4X1
            // Max ABL probability threshold to get min auto ABL weight (i4Y1)
            // Unit: 1024*%/100
            // Default value: 0
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X1, larger ABL probability zone will get min auto ABL weight (i4Y1)
            //
            // Varaible name: rAutoABLWeight.i4Y1
            // Min auto ABL weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoABLWeight.i4X2
            // Min ABL probability threshold to get max auto ABL weight i4Y2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller ABL probability zone will get max auto ABL weight (i4Y2)
            //
            // Varaible name: rAutoABLWeight.i4Y2
            // Max auto ABL weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The ABL probability larger than i4X2 will get max auto ABL weight of i4Y2
            /**************************************************/
            {                   	// rAutoABLWeight
                0,              	// rAutoABLWeight.i4X1
                1024,           	// rAutoABLWeight.i4Y1
                512,            	// rAutoABLWeight.i4X2
                1024            	// rAutoABLWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightL
            // HDR auto LV weight interpolation table
            // It is for low LV specially, LV higher than 40 will get weight 1024
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightL
            //
            // Varaible name: rAutoLVWeightL.i4X1
            // Max LV threshold to get min auto LV weight (i4Y1)
            // Unit: LV*10
            // Default value: 30
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight (i4Y1)
            //
            // Varaible name: rAutoLVWeightL.i4Y1
            // Min auto LV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoLVWeightL.i4X2
            // Min LV threshold to get max auto LV weight i4Y2
            // Unit: LV*10
            // Default value: 40
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight (i4Y2)
            //
            // Varaible name: rAutoLVWeightL.i4Y2
            // Max auto LV weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight of i4Y2              
            /**************************************************/
            {                       // rAutoLVWeightL
                 30,                // rAutoLVWeightL.i4X1
                768,                // rAutoLVWeightL.i4Y1
                 40,                // rAutoLVWeightL.i4X2
               1024                 // rAutoLVWeightL.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightH
            // HDR auto LV weight lower bound, determined by high LV
            // If scene LV is high, it raise HDR AE weight
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightH
            //
            // Varaible name: rAutoLVWeightH.i4X1
            // Max LV threshold to get min auto LV weight lower bound (i4Y1)
            // Unit: LV*10
            // Default value: 100
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight lower bound (i4Y1)
            //
            // Varaible name: rAutoLVWeightH.i4Y1
            // Min auto LV weight lower bound according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight for auto LV lower bound
            //
            // Varaible name: rAutoLVWeightH.i4X2
            // Min LV threshold to get max auto LV weight lower bound i4Y2
            // Unit: LV*10
            // Default value: 130
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight lower bound (i4Y2)
            //
            // Varaible name: rAutoLVWeightH.i4Y2
            // Max auto LV weight lower bound according to i4X2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight lower bound (i4Y2)
            /**************************************************/
            {                       // rAutoLVWeightH
                100,                // rAutoLVWeightH.i4X1
                256,                // rAutoLVWeightH.i4Y1
                130,                // rAutoLVWeightH.i4X2
                512                 // rAutoLVWeightH.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoNightWeight
            // HDR auto weight interpolation table, determined by night scene probability
            // Influence: HDR auto weight
            // rAutoNightWeight
            //
            // Varaible name: rAutoNightWeight.i4X1
            // Max night scene probability threshold to get max HDR auto weight (i4Y1)
            // Unit: LV*10
            // Default value: 1024
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger night scene probability zone will get max HDR auto weight (i4Y1)
            //
            // Varaible name: rAutoNightWeight.i4Y1
            // Max HDR auto weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y1, the higher HDR auto weight we will get
            //
            // Varaible name: rAutoNightWeight.i4X2
            // Min night scene probability threshold to get min HDR auto weight i4Y2
            // Unit: LV*10
            // Default value: 2048
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller night scene probability zone will get min HDR auto weight (i4Y2)
            //
            // Varaible name: rAutoNightWeight.i4Y2
            // Min HDR auto weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y2, the higher HDR auto weight we will get                
            /**************************************************/
            {                       // rAutoNightWeight
                1024,               // rAutoNightWeight.i4X1
                512,                // rAutoNightWeight.i4Y1
                2048,               // rAutoNightWeight.i4X2
                256                 // rAutoNightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rHdrAoeTarget
            // AOE information for HDR AE calculation
            // HDR scene share the same property as over exposure
            //
            // Varaible name: i4AoePercent
            // The percent for AOE to calculate current AOE Y from bright
            // Unit: 10*%
            // Default value: 25
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The larger i4AoePercent, more percent of histogram from the bright side will be accumulated for AOE current Y
            //
            // Varaible name: i4AoeLV
            // The LV thresholds to deliver different AOE target
            // Unit: LV*10
            // Default value: {50, 100, 150}
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: Differentiate the AOE target by LV
            //
            // Varaible name: i4AoeTarget
            // Different AOE targets differentiated by LV
            // Unit: Y value in 8 bits
            // Default value: {200, 200, 200}
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: Different LV delivers different AOE target
            /**************************************************/
            {
                25, //20,       // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            /**************************************************/
            // Feature name: rHdrNightTarget
            // Night scene information for HDR AE calculation
            // HDR scene has almost the same property as night scene,
            // but if we use higher HDR ratio in night scene, 
            // the noise will be large because of the high ISO in night scene.
            // So, night scene should use lower HDR ratio
            // rHdrNightTarget
            /**************************************************/
            {                 
                /**************************************************/
                // Varaible name: i4NightPercent
                // The percent for night scene to calculate current night scene Y from bright
                // Default value: 150
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The larger i4NightPercent, more percent of histogram from the bright side will be accumulated for night scene current Y
                /**************************************************/
                150,             // i4NightPercent

                /**************************************************/
                // Feature name: rNightTarget
                // Night scene target interpolation table, determined by night scene probability
                // Influence: HDR night scene target Y, HDR night scene target gain, HDR target gain, HDR target Y
                // rNightTarget
                //
                // Varaible name: rNightTarget.i4X1
                // Max night scene probability threshold to get max night scene target (i4Y1)
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max night scene target (i4Y1)
                //
                // Varaible name: rNightTarget.i4Y1
                // Max night scene target according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 80
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher night scene target we will get
                //
                // Varaible name: rNightTarget.i4X2
                // Min night scene probability threshold to get min night scene target i4Y2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min night scene target (i4Y2)
                //
                // Varaible name: rNightTarget.i4Y2
                // Min night scene target according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 60
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher night scene target we will get                
                /**************************************************/
                {               // rNightTarget
                    0,          // i4X1
                    80,         // i4Y1
                    1024,       // i4X2
                    60          // i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitL
            // HDR AE target clamping for low LV
            // The LV lower than the low LV threshold use this table,
            // between low LV threshold and medium LV threshold use the interpolation result of medium/low LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitL
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitL.i4LVLimit
                // Low LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 60
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV value lower than rTargetLimitL.i4LVLimit should use rTargetMinLimit interpolation table
                /**************************************************/
                60,             // rTargetLimitL.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMinLimit
                // For low LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for low LV scene
                // rTargetLimitL.rTargetMinLimit
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR target threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMaxLimit
                // For low LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for low LV scene
                // rTargetLimitL.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold 
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitM
            // HDR AE target clamping for medium LV
            // The LV between low LV threshold and medium LV threshold use the interpolation result of low/medium LV
            // The LV between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for medium LV scene
            // rTargetLimitM
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitM.i4LVLimit
                // Medium LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The rTargetLimitL.i4LVLimit < LV < rTargetLimitM.i4LVLimit should use rTargetLimitM interpolation table      
                /**************************************************/
                80,             // rTargetLimitM.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMinLimit
                // For medium LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for medium LV scene
                // rTargetLimitM.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get
                /**************************************************/
                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMaxLimit
                // For medium LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for medium LV scene
                // rTargetLimitM.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get            
                /**************************************************/
                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitH
            // HDR AE target clamping for high LV
            // The LV higher than the high LV threshold use this table,
            // between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitH
            /**************************************************/
            {                
                /**************************************************/
                // Varaible name: rTargetLimitH.i4LVLimit
                // High LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV larger than rTargetLimitH.i4LVLimit should use rTargetLimitH interpolation table               
                /**************************************************/
                120,            // rTargetLimitH.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMinLimit
                // For high LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for high LV scene
                // rTargetLimitH.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 25
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 40
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get            
                /**************************************************/
                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMaxLimit
                // For high LV, HDR AE target max threshold interpolation table
                // Influence: HDR AE target Y max clamping range for high LV scene
                // rTargetLimitH.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get                
                /**************************************************/
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
                /**************************************************/
                // Feature name: rFaceHdrProb
                // Face HDR probability interpolation table
                // rFaceHdrProb
                //
                // Varaible name: rFaceHdrProb.i4X1
                // Max face bright tone probability to get max night scene target (i4Y1)
                // Unit: weighting average pixel count
                // Default value: 2200
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger face bright tone probability zone will get max face HDR target (i4Y1)
                //
                // Varaible name: rFaceHdrProb.i4Y1
                // Max face HDR target according to i4X1
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher face HDR target we will get
                //
                // Varaible name: rFaceHdrProb.i4X2
                // Min face bright tone probability threshold to get min face HDR target i4Y2
                // Unit: weighting average pixel count
                // Default value: 3200
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller face bright tone probability zone will get min face HDR target (i4Y2)
                //
                // Varaible name: rFaceHdrProb.i4Y2
                // Min face HDR target according to i4X2
                // Unit: 1024*%/100
                // Default value: 256
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher face HDR target we will get                
                /**************************************************/
                {               // rFaceHdrProb
                    600,        // i4X1
                    400,        // i4Y1
                   1100,        // i4X2
                    400         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        /**************************************************/
        // Feature name: rHdrcHdr
        // Calculate cHDR detection information
        // rHdrcHdr      
        /**************************************************/
        {   
            /**************************************************/
            // Varaible name: i4LEThr
            // Current Y accumulated histogram from dark for LE information
            // Unit: Y value in 8 bits
            // Default value: 40
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4LEThr, more bins of histogram will be accumulated from dark side
            //
            // Varaible name: i4SERatio
            // Current Y accumulated histogram percentage from bright for SE information
            // Unit:10*%
            // Default value: 20
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SERatio, more % of histogram will be accumulated from bright side                   
            //  
            // Varaible name: i4SETarget
            // SE default target Y
            // Unit: Y value in 8 bits
            // Default value: 180
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SETarget, the higher SE target Y
            // 
            // Varaible name: i4BaseGain
            // The base gain for AE target
            // Unit: 1024*gain
            // Default value: 1024
            // Data range: Integer, i4BaseGain > 0
            // Constraints: N/A.
            // Effect: The higher i4BaseGain, the higher base for gain
            /**************************************************/
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
getHDRAEParam<ESensorDev_Sub>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        FALSE,                // bEnablePreIndex
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
        0,                    //bEnableFlareFastConverge
        0,                    //bEnableRAFastConverge
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
        1,                  //uCycleNumAESkipAfterExit;
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
        {35,    10},//u4Size_X[2];
        {1024, 256},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 256} //u4Loc_Y[2];
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)        
        70,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
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
          5,                  // face lost max count
         90,                  // face window scale ratio range 1~400% default 90%
         30,                //face NS lowbound
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
        {-2000, 0, 3000,5500},         // i4BV
        {150, 170, 180, 180}            // u4THD
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
       {-1000, -300},
       {1024, 	0},
    };
    static strHistStableCFG g_HistStableCFG =
    {
        FALSE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
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
        TRUE,                           //bEnableColorSuppressR
        TRUE,                           //bEnableColorSuppressG
        FALSE                         //bEnableColorSuppressB
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
            256                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        }
    };
    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,       80,   80},   //u4BTTHD[16]
    };
    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eSubHDR_ON,             // eHdrParamID

        /**************************************************/
        // Feature name: HDR detector parameter information
        // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
        // rHdrDetector
        /**************************************************/
        {                       
            /**************************************************/                   
            // Variable name: bHdrDetectorEnable
            // A switch to turn on/off of HDR detector
            // Default value: MTRUE.
            // Data range: MTRUE or MFALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR detector
            //
            // Variable name: i4HdrConfidenceThr
            // Threshold to turn HDR on/off
            // Unit: %
            // Default value: 80 or 220
            // Data range: Integer, i4HdrConfidenceThr > 0
            // Constraints: N/A.
            // Effect: Turn HDR on when result of HDR scene probability is larger than i4HdrConfidenceThr
            /**************************************************/
            MTRUE,              // bHdrDetectorEnable
            80, // 20,          // i4HdrConfidenceThr

            /**************************************************/
            // Feature name: rHdrHistInfo
            // HDR detector - histogram information calculation
            // Detect HDR scene by calculating full RGB histogram information
            // Find the proportion of pixel at bright/dark side of an image to check HDR characteristics in histogram
            /**************************************************/
            {                  
                /**************************************************/                
                // Variable name: bHistDetectEnable
                // A switch to turn on/off histogram analysis of HDR detector
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable histogram analysis of HDR detector
                /**************************************************/
                TRUE,           // bHistDetectEnable

                /**************************************************/
                // Feature name: HDR detector - dark weight
                // Dark side weighting table for HDR detector
                // Multiply full RGB histogram with dark side weight and add the result of bright side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the dark part of image
                // A pixel will be treated as a dark pixel, if its pixel value is lower than certain threshold
                // Here are the parameters to control the threshold of dark pixel and the corresponding weighting
                // rHdrDetectDarkWtWin
                //
                // Varaible name: i4HdrHistP0
                // Dark side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 6
                // Data range: Integer, 0 < i4HdrHistP0 < i4HdrHistP1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the more histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistP1
                // Dark side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 12
                // Data range: Integer, i4HdrHistP0 < i4HdrHistP1 <= 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the more histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistW0
                // Dark side higher pixel weight
                // Default value: 100
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins before i4HdrHistP0 will get higher fully weight
                //
                // Varaible name: i4HdrHistW1
                // Dark side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                /**************************************************/
                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },

                /**************************************************/
                // Feature name: HDR detector - bright weight
                // Bright side weighting table for HDR detector
                // Multiply full RGB histogram with bright side weight and add the result of dark side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the bright part of image
                // A pixel will be treated as a bright pixel, if its pixel value is higher than certain threshold
                // Influence: HDR probability, HDR on/off
                // rHdrDetectBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 180
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: HDR auto enhance bright weight
                // Weighting table, enhance histogram bright side
                // Multiply full RGB histogram with the bright side weight
                // It is used to calculate bright enhance probability
                // Influence: HDR auto weight, HDR ratio
                // rHdrEnhanceBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: rHdrFDBrightWtWin
                // Histogram from bright side weighting table for face detection
                // Multiply full RGB histogram with the face detection bright side weight
                // It is used to calculate face detection bright tone probability
                // Influence: HDR face probability
                // rHdrFDBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Face detection bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Face detection bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Face detection bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Face detection bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                /**************************************************/
                // Varaible name: i4HdrContrastPercentH
                // Bright side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 60
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrContrastPercentL
                // Dark side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightEVDiffPercentH
                // Bright side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 50 or 30
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher the i4HdrNightEVDiffPercentH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrNightEVDiffPercentL
                // Dark side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4HdrNightEVDiffPercentL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightDarkPercent
                // Night scene histogram Y %
                // It is used to calculate i4NightHistProb
                // Unit: 10*%
                // Default value: 500
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: It affects the calculation of histogram Y. The percentage depends on your purpose.
                /**************************************************/
                60,             // i4ContrastHistRatioH
                450,            // i4ContrastHistRatioL
                50,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                /**************************************************/
                // Feature name: HDR night scene EV difference probability
                // The probability table interpolating for night scene probability by EV difference
                // Influence: night EVD probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightEVDiffProb
                //
                // Varaible name: rHdrNightEVDiffProb.i4X1
                // Min EV difference threshold to get min night scene probability (i4Y1)
                // Unit: EV*1000
                // Default value: 4800
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X1, larger EV difference zone will get min night scene probability
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y1
                // Min night scene probability according to i4X1
                // Unit: 1024*%/100 
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 0, i4X1 would be the cutoff EV difference
                //         (EV difference < i4X1 will get zero probability)
                //         If i4Y1 > 0, we will get probability >= i4Y1
                //         (If i4Y1 > 0, we have certain faith of night scene for any EV difference input)
                //
                // Varaible name: rHdrNightEVDiffProb.i4X2
                // Max EV difference threshold to get max night scene probability (i4Y2)
                // Unit: EV*1000
                // Default value: 6500
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X2, the smaller zone of EV difference get max night scene probability (i4Y2)
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y2
                // Max night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 1024, the EV difference >= i4X2 will get night scene probability of i4Y2
                /**************************************************/
                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },
                /**************************************************/
                // Feature name: HDR night scene histogram probability
                // The probability table for night scene probability by histogram avg Y
                // Influence: night histogram probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightHistProb
                //
                // Varaible name: rHdrNightHistProb.i4X1
                // Max histogram avg Y threshold to get max night scene probability (i4Y1)
                // Unit: Y value in 8 bits
                // Default value: 2
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The avg Y <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightHistProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4X1 = 1024, the avg Y <= i4X1 will get 100% night scene probability
                //         If i4X1 < 1024, it means that we're not 100% sure for night scene, judging by avg Y
                //
                // Varaible name: rHdrNightHistProb.i4X2
                // Min histogram avg Y threshold to get min night scene probability (i4Y2)
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4X2, the higher avg Y we need to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightHistProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, then i4X2 is the cutoff avg Y to exclude the possibility of night scene
                /**************************************************/
                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                /**************************************************/
                // Feature name: HDR night scene LV probability
                // The probability table for night scene probability by scene LV
                // Influence: night LV probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                //  rHdrNightLVProb
                // Varaible name: rHdrNightLVProb.i4X1
                // Max scene LV threshold to get max night scene probability (i4Y1)
                // Unit: 10*LV
                // Default value: 50
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: Scene LV <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightLVProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 1024, scene LV <= i4X1 will get 100% night scene probability
                //         If i4Y1 < 1024, it means that we cannot 100% exclude the posibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4X2
                // Min scene LV threshold to get min night scene probability (i4Y2)
                // Unit: 10*LV
                // Default value: 100
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: The higher i4X2, higher scene LV is needed to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, i4X2 is the cutoff scene LV to exclude the possibility of night scene
                /**************************************************/
                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            /**************************************************/
            // Feature name: HDR geometry information (for HDR detector)
            // Calculating for HDR detector by geometry information
            // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
            // rHdrGeometryInfo
            /**************************************************/
            {                 
                /**************************************************/
                // Feature name: geometry center over exposure probability
                // HDR scene share the same property as center over exposure
                // rGeometryCOEProp
                /**************************************************/
                {              
                    /**************************************************/             
                    // Variable name: bGeometryCOEEnable
                    // A switch for HDR detector by geometry COE
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry COE
                    //
                    // Variable name: i4COEWeight
                    // Weight to control the importance of COE in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If COE is important to detect the HDR scene, set i4COEWeight >= 1024.
                    /**************************************************/  
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight
                    /**************************************************/
                    // Feature name: HDR center over-exposure outer ratio
                    // The outer geometry average information of AE statistics for HDR detector
                    // Influence: COE outer probability, COE outer weight probability
                    // rHdrCOEOuterRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer average threshold to get max COE probability (i4Y1)
                    // Unit: Y value in 8 bits
                    // Default value: 23
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The outer average <= i4X1 will get max COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Max COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 1024, the outer average <= i4X1 will get 100% COE probability
                    //         If i4Y1 <= 1024, it means that we're not 100% sure for COE, judging by outer average
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer average threshold to get min COE probability (i4Y2)
                    // Unit: Y value in 8 bits
                    // Default value: 47
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The higher i4X2, higher outer average is needed to exclude the possibility of COE
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Min COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 0, i4X2 is the cutoff outer average to exclude the possibility of COE
                    /**************************************************/
                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center over-exposure difference ratio
                    // The outer/inner geometry EV difference of AE statistics for HDR detector
                    // Influence: COE difference probability, COE difference weight probability, HDR on/off
                    // rHdrCOEDiffRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer/inner difference threshold to get min COE probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 1900
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger outer/inner geometry EV difference zone will get min COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Min COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff outer/inner geometry EV difference
                    //         (EV difference < i4X1 will get zero probability)
                    //         If i4Y1 > 0, we will get probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of COE for any outer/inner geometry input)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer/inner difference threshold to get max COE probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 2400
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of outer/inner geometry EV difference get max COE probability (i4Y2)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Max COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the outer/inner EV difference >= i4X2 will get COE probability of i4Y2
                    /**************************************************/
                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                /**************************************************/
                // Feature name: HDR anti-back-light(ABL) probability
                // HDR scene share the same property as back-light
                // rGeometryABLProp
                /**************************************************/
                {            
                    /**************************************************/
                    // Variable name: bGeometryABLEnable
                    // A switch for HDR detector by geometry ABL
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry ABL
                    //
                    // Variable name: i4ABLWeight
                    // Weight to control the importance of ABL in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If ABL is important to detect the HDR scene, set i4ABLWeight >= 1024.
                    /**************************************************/
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    /**************************************************/
                    // Feature name: HDR center bright difference ratio
                    // The ABL target probability table when center is brighter than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterBrightDiffRatio
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center dark difference ratio
                    // The ABL target probability table when center is darker than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterDarkDiffRatio
                    // Varaible name: rCenterDarkDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100   
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100   
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
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
                /**************************************************/
                // Variable name: bLVDetectEnable
                // A switch to use LV to detect HDR scene
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable HDR LV analysis
                //
                // Varaible name: i4LVProbTbl
                // LV table for detecting HDR scene
                // Unit: %
                // Default value: { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
                // Data range: Integer, 0 ~ 100
                // Constraints: N/A.
                // Effect: Set the probability of HDR scene according to LV information
                /**************************************************/
                TRUE,           // bLVDetectEnable
               
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },
        
        /**************************************************/
        // Feature name: HDR auto-enhance
        // Calculating information for HDR ratio
        // rHdrAutoEnhance
        /**************************************************/
        {                       
            /**************************************************/
            // Variable name: bAutoEnhanceEnable
            // A switch to calculate HDR ratio information
            // Default value: TRUE.
            // Data range: TRUE or FALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR BV analysis
            //
            // Varaible name: i4RMGk
            // Control the height of turning point to LE/SE curve
            // Default value: 4095
            // Data range: Integer, 0 ~ 4095
            // Constraints: N/A.
            // Effect: Set i4RMGk higher(lower) to make LE longer(shorter)
            //
            // Varaible name: i4RMGtRatio
            // Set the percentage of current Y for calculating RMGt
            // Unit: 10*%
            // Default value: 40
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The higher i4RMGtRatio, more percentage of histogram will be accumulated in current Y
            /**************************************************/
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            40, //20,           // i4RMGtRatio

            /**************************************************/
            // Feature name: RMGt
            // Interpolation table for RMGt
            // Influence: the turning point between LE/SE curve
            // If current Y is large(small), use data of SE(LE) more
            // rRMGt
            //
            // Varaible name: rRMGt.i4X1
            // Max current Y threshold to get max RMGt i4Y1
            // Unit: Y value in 8 bits
            // Default value: 160
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X1, larger current Y zone will get max RMGt setting (i4Y1)
            //         The current Y lower than i4Y1 will get longer LE curve.
            //
            // Varaible name: rRMGt.i4Y1
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 512
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y1, the longer LE curve we will get
            //
            // Varaible name: rRMGt.i4X2
            // Min current Y to get min RMGt i4Y2
            // Unit: Y value in 8 bits
            // Default value: 220
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller current Y zone will get min RMGt setting (i4Y2)
            //         The current Y higher than i4Y1 will get shorter LE curve.
            //
            // Varaible name: rRMGt.i4Y2
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 384
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y2, the longer LE curve we will gets
            /**************************************************/
            {                         // rRMGt
                160,                  // i4X1
                512, //768,           // i4Y1
                220,                  // i4X2
                384, //256, //512     // i4Y2
            },
            //Not used
            /**************************************************/
            2,                  // i4SupportHdrNum
            1024,               // i4AutoEnhanceProb
            1024,               // i4HdrWeight
            /**************************************************/
            
            // rAutoBright
            {                   
                /**************************************************/
                // Varaible name: i4LVRatio50
                // Ratio interpolation table by scene LV, for 50 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                //
                // Varaible name: i4LVRatio60
                // Ratio interpolation table by scene LV, for 60 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                /**************************************************/
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio50
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio60
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},

                /**************************************************/
                // Feature name: rContrastRatio
                // HDR ratio interpolation table by HDR contrast EV difference
                // Influence: auto contrast ratio, HDR ratio
                // rContrastRatio
                //
                // Varaible name: rContrastRatio.i4X1
                // Max contrast EVD threshold to get min HDR ratio i4Y1
                // Unit: EV*1000
                // Default value: 3500
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger contrast EVD zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rContrastRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 100
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rContrastRatio.i4X2
                // Min contrast EVD to get max HDR ratio i4Y2
                // Unit: EV*1000
                // Default value: 4500
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller contrast EVD zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rContrastRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {                 // rContrastRatio
                    3500, //4000, // i4X1
                    200,          // i4Y1
                    4500, //5000, // i4X2
                    400           // i4Y2
                },

                /**************************************************/
                // Feature name: rBrightRatio
                // HDR ratio interpolation table by HDR bright side histogram information
                // Influence: auto bright probability, HDR ratio
                //rBrightRatio
                //
                // Varaible name: rBrightRatio.i4X1
                // Max bright side histogram threshold to get HDR ratio i4Y1
                // Unit: weighting average pixel count
                // Default value: 600
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger bright side histogram zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rBrightRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rBrightRatio.i4X2
                // Min bright side histogram to get max HDR ratio i4Y2
                // Unit: weighting average pixel count
                // Default value: 1100
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller bright side histogram zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rBrightRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: rNightProbRatio
                // HDR ratio interpolation table by night scene probability
                // Influence: auto night probability, HDR ratio
                // rNightProbRatio
                // Varaible name: rNightProbRatio.i4X1
                // Max night scene probability threthold to get max HDR ratio i4Y1
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rNightProbRatio.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rNightProbRatio.i4X2
                // Min night scene probability to get max HDR ratio i4Y2
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rNightProbRatio.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rNightProbRatio
                    0,          // i4X1
                    400,        // i4Y1
                    512,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: motion information
                // The motion corresponding information including gyro, accelerator,
                // Influence: HDR ratio, HDR on/off, on/off smoothness
                // rGGyroSensor
                /**************************************************/
                {                 
                    /**************************************************/
                    // Feature name: rAcceProb
                    // Acceleration probability interpolation table by accelerator information
                    // Influence: HDR ratio, motion on/off
                    // rAcceProb
                    //
                    // Varaible name: rAcceProb.i4X1
                    // Max acceleration threthold to get min acceleration probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger acceleration zone will get min acceleration probability (i4Y1)
                    //
                    // Varaible name: rAcceProb.i4Y1
                    // Min acceleration probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff acceleration to get min acceleration probability
                    //
                    // Varaible name: rAcceProb.i4X2
                    // Min acceleration to get max acceleration probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller acceleration zone will get max acceleration probability (i4Y2)
                    //
                    // Varaible name: rAcceProb.i4Y2
                    // Max acceleration probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the acceleration > i4X2 will get 100% acceleration probability
                    /**************************************************/
                    {               // rAcceProb
                        1100, //980,// i4X1
                        0,          // i4Y1
                        1500,       // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rGyroProb
                    // Motion probability interpolation table by gyro sensor information
                    // Influence: HDR ratio, motion on/off
                    // rGyroProb
                    //
                    // Varaible name: rGyroProb.i4X1
                    // Max motion threthold to get min motion probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion zone will get min motion probability (i4Y1)
                    //
                    // Varaible name: rGyroProb.i4Y1
                    // Min motion probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff motion to get min motion probability
                    //
                    // Varaible name: rGyroProb.i4X2
                    // Min motion to get max motion probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion zone will get max motion probability (i4Y2)
                    //
                    // Varaible name: rGyroProb.i4Y2
                    // Max motion probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the motion > i4X2 will get 100% motion probability
                    /**************************************************/
                    {               // rGyroProb
                        50, //100,  // i4X1
                        0,          // i4Y1
                        200, //700, // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rMotionRatio
                    // HDR ratio interpolation table by motion (gyro + acce) probability information
                    // Influence: HDR ratio
                    // rMotionRatio
                    //
                    // Varaible name: rMotionRatio.i4X1
                    // Max motion probability threthold to get min HDR ratio i4Y1
                    // Unit: 2048*%/100
                    // Default value: 512
                    // Data range: Integer, 0 ~ 2048
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion probability zone will get max HDR ratio (i4Y1)
                    //
                    // Varaible name: rMotionRatio.i4Y1
                    // Max HDR ratio according to i4X1
                    // Unit: 100*LE/SE
                    // Default value: 400
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    //
                    // Varaible name: rMotionRatio.i4X2
                    // Min motion probability to get max HDR ratio i4Y2
                    // Unit: 2048*%/100
                    // Default value: 960
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion probability zone will get min HDR ratio (i4Y2)
                    //
                    // Varaible name: rMotionRatio.i4Y2
                    // Min HDR ratio according to i4X2
                    // Unit: 100*LE/SE
                    // Default value: 100
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    /**************************************************/
                    {               // rMotionRatio
                        512,        // i4X1
                        400,        // i4Y1
                        960,        // i4X2
                        100         // i4Y2
                    },

                    /**************************************************/
                    // Varaible name: i4MotionOnThr
                    // Threshold to determine motion on/off
                    // Unit: 1024*%/100
                    // Default value: 512 or 768
                    // Data range: Integer, i4MotionOnThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnThr, you need higher motion probability to set motion detector on
                    //
                    // Varaible name: i4MotionOnCntThr
                    // Counting threshold to update motion on/off information
                    // Unit: frame count
                    // Default value: 2 or 5
                    // Data range: Integer, i4MotionOnCntThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnCntThr, you need count more to update motion on/off
                    /**************************************************/
                    512, //768,     // i4MotionOnThr
                    2               //i4MotionOnCntThr
                }
            },

            /**************************************************/
            // Feature name: rAutoFlicker
            // Parameter setting to automatically avoid flicker
            // rAutoFlicker
            /**************************************************/
            {                
                /**************************************************/
                // Varaible name: i4Flkr1LVThr
                // LV threshold for deflicker enable
                // Unit: LV*10
                // Default value: {130,130}
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The higher i4Flkr1LVThr, the higher scene LV needed to enable deflicker
                //
                // Varaible name: i4MinRatioAntiFlk
                // The min acceptable ratio for anti-flicker
                // Unit: 100*LE/SE
                // Default value: 200 or 800
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The ratio lower i4MinRatioAntiFlk than could get flickered results
                /**************************************************/
                {130,130},        // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, // i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     // i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/

                /**************************************************/
                // Feature name: rIsoThr
                // HDR ratio interpolation table by ISO information
                // Influence: HDR ratio
                // rIsoThr
                //
                // Varaible name: rIsoThr.i4X1
                // Max ISO threshold to get max HDR ratio i4Y1
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger ISO zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rIsoThr.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rIsoThr.i4X2
                // Min ISO to get min HDR ratio i4Y2
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller ISO zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rIsoThr.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rIsoThr
                    12288,      //i4X1:1024 * 12
                    4,          //i4Y1
                    14336,      //i4X2:1024 * 10
                    1           //i4Y2
                },
            },

            /**************************************************/
            // Feature name: rAutoSmooth
            // HDR auto smooth (HDR on/off, motion on/off, HDR ratio smooth) information
            // rAutoSmooth
            /**************************************************/
            {                  
                /**************************************************/
                // Variable name: bHdrWaitAEStable
                // A switch to determine wait for AE stable or not
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable wait
                //
                // Variable name: i4HdrInfoCounterThr
                // Threshold to update HDR on/off information
                // Unit: frame count
                // Default value: 6
                // Data range: Integer, i4HdrInfoCounterThr > 0
                // Constraints: N/A.
                // Effect: HDR on/off counter larger than i4HdrInfoCounterThr can update the on/off result
                /**************************************************/
                TRUE,           // bHdrWaitAEStable
                6,              // i4HdrInfoCounterThr

                //Not used
                /**************************************************/
                TRUE,           // bRatioSmoothEnable TRUE for platform / FALSE for simulation
                /**************************************************/

                /**************************************************/
                // Variable name: i4RatioSmoothThr
                // Threshold to update HDR ratio smooth information
                // Unit: frame count
                // Default value: none
                // Data range: Integer, i4RatioSmoothThr > 0
                // Constraints: N/A.
                // Effect: HDR ratio smooth counter larger than i4RatioSmoothThr can update the smoothed HDR ratio result
                //
                // Variable name: i4RoughRatioUnit
                // Threshold to check if difference of current ratio and current stable ratio is large enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4RoughRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current ratio and current stable ratio larger than i4RoughRatioUnit check for close results in incoming frames
                //
                // Variable name: i4FinerRatioUnit
                // Threshold to check if difference of current/previous ratio is small enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4FinerRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current/previous HDR ratio > i4FinerRatioUnit start to count
                /**************************************************/
                { 2,   2},      // i4RatioSmoothThr
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
            }
            /**************************************************/
        },

        /**************************************************/
        // Feature name: rHdrAETarget
        // HDR AE target parameter setting
        // rHdrAETarget
        /**************************************************/
        {   
            /**************************************************/
            // Variable name: i4DefTarget
            // Default AE target
            // Unit: Y value in 8 bits
            // Default value: 47
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: A reference of AE target, set higher to make AE target higher
            //
            // Variable name: i4DefWeight
            // Weight of default target, mixing with normal AE target
            // Default value: 47
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: Set i4DefWeight higher to make the proportion of default AE target higher
            /**************************************************/
            47,                 // i4DefTarget
            0, //256,           // i4DefWeight

            /**************************************************/
            // Feature name: rAutoEVWeight
            // HDR auto EV weight interpolation table
            // Influence: auto EV weight, HDR auto weight
            // rAutoEVWeight
            //
            // Varaible name: rAutoEVWeight.i4X1
            // Max contrast EVD threshold to get min auto EV weight (i4Y1)
            // Unit: EV*1000
            // Default value: 4000 or 3750
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger contrast EVD zone will get min auto EV weight (i4Y1)
            //
            // Varaible name: rAutoEVWeight.i4Y1
            // Min auto EV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoEVWeight.i4X2
            // Min contrast EVD threshold to get max auto EV weight i4Y2
            // Unit: EV*1000
            // Default value: 4500 or 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller contrast EVD zone will get max auto EV weight (i4Y2)
            //
            // Varaible name: rAutoEVWeight.i4Y2
            // Max auto EV weight according to i4X2
            // Unit: 1024*%/100
            // Default value:  512 or 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The contrast EVD larger than i4X2 will get auto EV weight of i4Y2
            /**************************************************/
            {                   // rAutoEVWeight
                4000,           // rAutoEVWeight.i4X1
                256,            // rAutoEVWeight.i4Y1
                5000,           // rAutoEVWeight.i4X2
                512             // rAutoEVWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoBrightWeight
            // HDR auto bright weight interpolation table
            // Influence: auto bright weight, auto night weight, HDR auto weight
            // rAutoBrightWeight
            //
            // Varaible name: rAutoBrightWeight.i4X1
            // Max bright enhance probability threshold to get min auto bright weight (i4Y1)
            // Unit: weighting average pixel count
            // Default value: 4000
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger bright enhance probability zone will get min auto bright weight (i4Y1)
            //
            // Varaible name: rAutoBrightWeight.i4Y1
            // Min auto bright weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoBrightWeight.i4X2
            // Min bright enhance probability threshold to get max auto bright weight i4Y2
            // Unit: weighting average pixel count
            // Default value: 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller bright enhance probability zone will get max auto bright weight (i4Y2)
            //
            // Varaible name: rAutoBrightWeight.i4Y2
            // Max auto bright weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The bright enhance probability larger than i4X2 will get max auto bright weight of i4Y2
            /**************************************************/
            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoABLWeight
            // HDR auto ABL weight interpolation table
            // Influence: auto ABL weight, auto night weight, HDR auto weight
            // rAutoABLWeight
            //
            // Varaible name: rAutoABLWeight.i4X1
            // Max ABL probability threshold to get min auto ABL weight (i4Y1)
            // Unit: 1024*%/100
            // Default value: 0
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X1, larger ABL probability zone will get min auto ABL weight (i4Y1)
            //
            // Varaible name: rAutoABLWeight.i4Y1
            // Min auto ABL weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoABLWeight.i4X2
            // Min ABL probability threshold to get max auto ABL weight i4Y2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller ABL probability zone will get max auto ABL weight (i4Y2)
            //
            // Varaible name: rAutoABLWeight.i4Y2
            // Max auto ABL weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The ABL probability larger than i4X2 will get max auto ABL weight of i4Y2
            /**************************************************/
            {                   // rAutoABLWeight
                0,              // rAutoABLWeight.i4X1
                1024,           // rAutoABLWeight.i4Y1
                512,            // rAutoABLWeight.i4X2
                1024            // rAutoABLWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightL
            // HDR auto LV weight interpolation table
            // It is for low LV specially, LV higher than 40 will get weight 1024
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightL
            //
            // Varaible name: rAutoLVWeightL.i4X1
            // Max LV threshold to get min auto LV weight (i4Y1)
            // Unit: LV*10
            // Default value: 30
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight (i4Y1)
            //
            // Varaible name: rAutoLVWeightL.i4Y1
            // Min auto LV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoLVWeightL.i4X2
            // Min LV threshold to get max auto LV weight i4Y2
            // Unit: LV*10
            // Default value: 40
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight (i4Y2)
            //
            // Varaible name: rAutoLVWeightL.i4Y2
            // Max auto LV weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight of i4Y2              
            /**************************************************/
            {                   // rAutoLVWeightL
                 30,            // rAutoLVWeightL.i4X1
                768,            // rAutoLVWeightL.i4Y1
                 40,            // rAutoLVWeightL.i4X2
               1024             // rAutoLVWeightL.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightH
            // HDR auto LV weight lower bound, determined by high LV
            // If scene LV is high, it raise HDR AE weight
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightH
            //
            // Varaible name: rAutoLVWeightH.i4X1
            // Max LV threshold to get min auto LV weight lower bound (i4Y1)
            // Unit: LV*10
            // Default value: 100
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight lower bound (i4Y1)
            //
            // Varaible name: rAutoLVWeightH.i4Y1
            // Min auto LV weight lower bound according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight for auto LV lower bound
            //
            // Varaible name: rAutoLVWeightH.i4X2
            // Min LV threshold to get max auto LV weight lower bound i4Y2
            // Unit: LV*10
            // Default value: 130
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight lower bound (i4Y2)
            //
            // Varaible name: rAutoLVWeightH.i4Y2
            // Max auto LV weight lower bound according to i4X2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight lower bound (i4Y2)
            /**************************************************/
            {                   // rAutoLVWeightH
                100,            // rAutoLVWeightH.i4X1
                256,            // rAutoLVWeightH.i4Y1
                130,            // rAutoLVWeightH.i4X2
                512             // rAutoLVWeightH.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoNightWeight
            // HDR auto weight interpolation table, determined by night scene probability
            // Influence: HDR auto weight
            // rAutoNightWeight
            //
            // Varaible name: rAutoNightWeight.i4X1
            // Max night scene probability threshold to get max HDR auto weight (i4Y1)
            // Unit: LV*10
            // Default value: 1024
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger night scene probability zone will get max HDR auto weight (i4Y1)
            //
            // Varaible name: rAutoNightWeight.i4Y1
            // Max HDR auto weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y1, the higher HDR auto weight we will get
            //
            // Varaible name: rAutoNightWeight.i4X2
            // Min night scene probability threshold to get min HDR auto weight i4Y2
            // Unit: LV*10
            // Default value: 2048
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller night scene probability zone will get min HDR auto weight (i4Y2)
            //
            // Varaible name: rAutoNightWeight.i4Y2
            // Min HDR auto weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y2, the higher HDR auto weight we will get                
            /**************************************************/
            {                   // rAutoNightWeight
                1024,           // rAutoNightWeight.i4X1
                512,            // rAutoNightWeight.i4Y1
                2048,           // rAutoNightWeight.i4X2
                256             // rAutoNightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rHdrAoeTarget
            // AOE information for HDR AE calculation
            // HDR scene share the same property as over exposure
            //
            // Varaible name: i4AoePercent
            // The percent for AOE to calculate current AOE Y from bright
            // Unit: 10*%
            // Default value: 25
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The larger i4AoePercent, more percent of histogram from the bright side will be accumulated for AOE current Y
            //
            // Varaible name: i4AoeLV
            // The LV thresholds to deliver different AOE target
            // Unit: LV*10
            // Default value: {50, 100, 150}
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: Differentiate the AOE target by LV
            //
            // Varaible name: i4AoeTarget
            // Different AOE targets differentiated by LV
            // Unit: Y value in 8 bits
            // Default value: {200, 200, 200}
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: Different LV delivers different AOE target
            /**************************************************/
            {
                25, //20,       // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            /**************************************************/
            // Feature name: rHdrNightTarget
            // Night scene information for HDR AE calculation
            // HDR scene has almost the same property as night scene,
            // but if we use higher HDR ratio in night scene, 
            // the noise will be large because of the high ISO in night scene.
            // So, night scene should use lower HDR ratio
            // rHdrNightTarget
            /**************************************************/
            {                  
                /**************************************************/
                // Varaible name: i4NightPercent
                // The percent for night scene to calculate current night scene Y from bright
                // Default value: 150
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The larger i4NightPercent, more percent of histogram from the bright side will be accumulated for night scene current Y
                /**************************************************/
                150,             // i4NightPercent

                /**************************************************/
                // Feature name: rNightTarget
                // Night scene target interpolation table, determined by night scene probability
                // Influence: HDR night scene target Y, HDR night scene target gain, HDR target gain, HDR target Y
                // rNightTarget
                //
                // Varaible name: rNightTarget.i4X1
                // Max night scene probability threshold to get max night scene target (i4Y1)
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max night scene target (i4Y1)
                //
                // Varaible name: rNightTarget.i4Y1
                // Max night scene target according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 80
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher night scene target we will get
                //
                // Varaible name: rNightTarget.i4X2
                // Min night scene probability threshold to get min night scene target i4Y2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min night scene target (i4Y2)
                //
                // Varaible name: rNightTarget.i4Y2
                // Min night scene target according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 60
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher night scene target we will get                
                /**************************************************/
                {               // rNightTarget
                    0,          // i4X1
                    80,         // i4Y1
                    1024,       // i4X2
                    60          // i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitL
            // HDR AE target clamping for low LV
            // The LV lower than the low LV threshold use this table,
            // between low LV threshold and medium LV threshold use the interpolation result of medium/low LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitL
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitL.i4LVLimit
                // Low LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 60
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV value lower than rTargetLimitL.i4LVLimit should use rTargetMinLimit interpolation table
                /**************************************************/
                60,             // rTargetLimitL.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMinLimit
                // For low LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for low LV scene
                // rTargetLimitL.rTargetMinLimit
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR target threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMaxLimit
                // For low LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for low LV scene
                // rTargetLimitL.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold 
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitM
            // HDR AE target clamping for medium LV
            // The LV between low LV threshold and medium LV threshold use the interpolation result of low/medium LV
            // The LV between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for medium LV scene
            // rTargetLimitM
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitM.i4LVLimit
                // Medium LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The rTargetLimitL.i4LVLimit < LV < rTargetLimitM.i4LVLimit should use rTargetLimitM interpolation table      
                /**************************************************/
                80,             // rTargetLimitM.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMinLimit
                // For medium LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for medium LV scene
                // rTargetLimitM.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get
                /**************************************************/
                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMaxLimit
                // For medium LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for medium LV scene
                // rTargetLimitM.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get            
                /**************************************************/
                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitH
            // HDR AE target clamping for high LV
            // The LV higher than the high LV threshold use this table,
            // between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitH
            /**************************************************/
            {                  
                /**************************************************/
                // Varaible name: rTargetLimitH.i4LVLimit
                // High LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV larger than rTargetLimitH.i4LVLimit should use rTargetLimitH interpolation table               
                /**************************************************/
                120,            // rTargetLimitH.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMinLimit
                // For high LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for high LV scene
                // rTargetLimitH.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 25
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 40
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get            
                /**************************************************/
                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMaxLimit
                // For high LV, HDR AE target max threshold interpolation table
                // Influence: HDR AE target Y max clamping range for high LV scene
                // rTargetLimitH.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get                
                /**************************************************/
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
                /**************************************************/
                // Feature name: rFaceHdrProb
                // Face HDR probability interpolation table
                // rFaceHdrProb
                //
                // Varaible name: rFaceHdrProb.i4X1
                // Max face bright tone probability to get max night scene target (i4Y1)
                // Unit: weighting average pixel count
                // Default value: 2200
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger face bright tone probability zone will get max face HDR target (i4Y1)
                //
                // Varaible name: rFaceHdrProb.i4Y1
                // Max face HDR target according to i4X1
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher face HDR target we will get
                //
                // Varaible name: rFaceHdrProb.i4X2
                // Min face bright tone probability threshold to get min face HDR target i4Y2
                // Unit: weighting average pixel count
                // Default value: 3200
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller face bright tone probability zone will get min face HDR target (i4Y2)
                //
                // Varaible name: rFaceHdrProb.i4Y2
                // Min face HDR target according to i4X2
                // Unit: 1024*%/100
                // Default value: 256
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher face HDR target we will get                
                /**************************************************/
                {               // rFaceHdrProb
                    600,        // i4X1
                    400,        // i4Y1
                   1100,        // i4X2
                    400         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        /**************************************************/
        // Feature name: rHdrcHdr
        // Calculate cHDR detection information
        // rHdrcHdr      
        /**************************************************/
        {   
            /**************************************************/
            // Varaible name: i4LEThr
            // Current Y accumulated histogram from dark for LE information
            // Unit: Y value in 8 bits
            // Default value: 40
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4LEThr, more bins of histogram will be accumulated from dark side
            //
            // Varaible name: i4SERatio
            // Current Y accumulated histogram percentage from bright for SE information
            // Unit:10*%
            // Default value: 20
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SERatio, more % of histogram will be accumulated from bright side                   
            //  
            // Varaible name: i4SETarget
            // SE default target Y
            // Unit: Y value in 8 bits
            // Default value: 180
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SETarget, the higher SE target Y
            // 
            // Varaible name: i4BaseGain
            // The base gain for AE target
            // Unit: 1024*gain
            // Default value: 1024
            // Data range: Integer, i4BaseGain > 0
            // Constraints: N/A.
            // Effect: The higher i4BaseGain, the higher base for gain
            /**************************************************/
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
    {LIB3A_AE_SCENE_AUTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_NIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ACTION, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_BEACH, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_CANDLELIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_FIREWORKS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_LANDSCAPE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_NIGHT_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_PARTY, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_SNOW, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_SPORTS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_STEADYPHOTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_SUNSET, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_THEATRE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO_ANTI_SHAKE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO100, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO1600 , {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO3200,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15,  AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO6400,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15,  AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
    {LIB3A_AE_SCENE_ISO12800,{AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX16, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15,  AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15, AETABLE_SCENE_INDEX15}},
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
        TRUE,              // Perframe HDR AE smooth option

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
getAUTOHDRAEParam<ESensorDev_Sub>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        FALSE,                // bEnablePreIndex
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
        0,                    //bEnableFlareFastConverge
        0,                    //bEnableRAFastConverge
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
        1,                  //uCycleNumAESkipAfterExit;
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
        {35,    10},//u4Size_X[2];
        {1024, 256},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 256} //u4Loc_Y[2];
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)        
        70,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
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
          5,                  // face lost max count
         90,                  // face window scale ratio range 1~400% default 90%
         30,                //face NS lowbound
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
        {-2000, 0, 3000,5500},         // i4BV
        {150, 170, 180, 180}            // u4THD
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
       {-1000, -300},
       {1024, 	0},
    };
    static strHistStableCFG g_HistStableCFG =
    {
        FALSE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
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
        TRUE,                           //bEnableColorSuppressR
        TRUE,                           //bEnableColorSuppressG
        FALSE                         //bEnableColorSuppressB
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
            256                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        }
    };
    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,       80,   80},   //u4BTTHD[16]
    };
    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eSubHDR_AUTO,           // eHdrParamID

        /**************************************************/
        // Feature name: HDR detector parameter information
        // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
        // rHdrDetector
        /**************************************************/
        {                       
            /**************************************************/                   
            // Variable name: bHdrDetectorEnable
            // A switch to turn on/off of HDR detector
            // Default value: MTRUE.
            // Data range: MTRUE or MFALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR detector
            //
            // Variable name: i4HdrConfidenceThr
            // Threshold to turn HDR on/off
            // Unit: %
            // Default value: 80 or 220
            // Data range: Integer, i4HdrConfidenceThr > 0
            // Constraints: N/A.
            // Effect: Turn HDR on when result of HDR scene probability is larger than i4HdrConfidenceThr
            /**************************************************/
            MTRUE,              // bHdrDetectorEnable
            220, // 20,         // i4HdrConfidenceThr

            /**************************************************/
            // Feature name: rHdrHistInfo
            // HDR detector - histogram information calculation
            // Detect HDR scene by calculating full RGB histogram information
            // Find the proportion of pixel at bright/dark side of an image to check HDR characteristics in histogram
            /**************************************************/
            {                   
                /**************************************************/                
                // Variable name: bHistDetectEnable
                // A switch to turn on/off histogram analysis of HDR detector
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable histogram analysis of HDR detector
                /**************************************************/
                TRUE,           // bHistDetectEnable

                /**************************************************/
                // Feature name: HDR detector - dark weight
                // Dark side weighting table for HDR detector
                // Multiply full RGB histogram with dark side weight and add the result of bright side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the dark part of image
                // A pixel will be treated as a dark pixel, if its pixel value is lower than certain threshold
                // Here are the parameters to control the threshold of dark pixel and the corresponding weighting
                // rHdrDetectDarkWtWin
                //
                // Varaible name: i4HdrHistP0
                // Dark side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 6
                // Data range: Integer, 0 < i4HdrHistP0 < i4HdrHistP1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the more histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistP1
                // Dark side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 12
                // Data range: Integer, i4HdrHistP0 < i4HdrHistP1 <= 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the more histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistW0
                // Dark side higher pixel weight
                // Default value: 100
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins before i4HdrHistP0 will get higher fully weight
                //
                // Varaible name: i4HdrHistW1
                // Dark side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                /**************************************************/
                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },

                /**************************************************/
                // Feature name: HDR detector - bright weight
                // Bright side weighting table for HDR detector
                // Multiply full RGB histogram with bright side weight and add the result of dark side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the bright part of image
                // A pixel will be treated as a bright pixel, if its pixel value is higher than certain threshold
                // Influence: HDR probability, HDR on/off
                // rHdrDetectBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 180
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: HDR auto enhance bright weight
                // Weighting table, enhance histogram bright side
                // Multiply full RGB histogram with the bright side weight
                // It is used to calculate bright enhance probability
                // Influence: HDR auto weight, HDR ratio
                // rHdrEnhanceBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: rHdrFDBrightWtWin
                // Histogram from bright side weighting table for face detection
                // Multiply full RGB histogram with the face detection bright side weight
                // It is used to calculate face detection bright tone probability
                // Influence: HDR face probability
                // rHdrFDBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Face detection bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Face detection bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Face detection bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Face detection bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                /**************************************************/
                // Varaible name: i4HdrContrastPercentH
                // Bright side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 60
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrContrastPercentL
                // Dark side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightEVDiffPercentH
                // Bright side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 50 or 30
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher the i4HdrNightEVDiffPercentH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrNightEVDiffPercentL
                // Dark side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4HdrNightEVDiffPercentL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightDarkPercent
                // Night scene histogram Y %
                // It is used to calculate i4NightHistProb
                // Unit: 10*%
                // Default value: 500
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: It affects the calculation of histogram Y. The percentage depends on your purpose.
                /**************************************************/
                60,             // i4ContrastHistRatioH
                450,            // i4ContrastHistRatioL
                50,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                /**************************************************/
                // Feature name: HDR night scene EV difference probability
                // The probability table interpolating for night scene probability by EV difference
                // Influence: night EVD probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightEVDiffProb
                //
                // Varaible name: rHdrNightEVDiffProb.i4X1
                // Min EV difference threshold to get min night scene probability (i4Y1)
                // Unit: EV*1000
                // Default value: 4800
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X1, larger EV difference zone will get min night scene probability
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y1
                // Min night scene probability according to i4X1
                // Unit: 1024*%/100 
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 0, i4X1 would be the cutoff EV difference
                //         (EV difference < i4X1 will get zero probability)
                //         If i4Y1 > 0, we will get probability >= i4Y1
                //         (If i4Y1 > 0, we have certain faith of night scene for any EV difference input)
                //
                // Varaible name: rHdrNightEVDiffProb.i4X2
                // Max EV difference threshold to get max night scene probability (i4Y2)
                // Unit: EV*1000
                // Default value: 6500
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X2, the smaller zone of EV difference get max night scene probability (i4Y2)
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y2
                // Max night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 1024, the EV difference >= i4X2 will get night scene probability of i4Y2
                /**************************************************/
                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },
                /**************************************************/
                // Feature name: HDR night scene histogram probability
                // The probability table for night scene probability by histogram avg Y
                // Influence: night histogram probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightHistProb
                //
                // Varaible name: rHdrNightHistProb.i4X1
                // Max histogram avg Y threshold to get max night scene probability (i4Y1)
                // Unit: Y value in 8 bits
                // Default value: 2
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The avg Y <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightHistProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4X1 = 1024, the avg Y <= i4X1 will get 100% night scene probability
                //         If i4X1 < 1024, it means that we're not 100% sure for night scene, judging by avg Y
                //
                // Varaible name: rHdrNightHistProb.i4X2
                // Min histogram avg Y threshold to get min night scene probability (i4Y2)
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4X2, the higher avg Y we need to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightHistProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, then i4X2 is the cutoff avg Y to exclude the possibility of night scene
                /**************************************************/
                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                /**************************************************/
                // Feature name: HDR night scene LV probability
                // The probability table for night scene probability by scene LV
                // Influence: night LV probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                //  rHdrNightLVProb
                // Varaible name: rHdrNightLVProb.i4X1
                // Max scene LV threshold to get max night scene probability (i4Y1)
                // Unit: 10*LV
                // Default value: 50
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: Scene LV <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightLVProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 1024, scene LV <= i4X1 will get 100% night scene probability
                //         If i4Y1 < 1024, it means that we cannot 100% exclude the posibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4X2
                // Min scene LV threshold to get min night scene probability (i4Y2)
                // Unit: 10*LV
                // Default value: 100
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: The higher i4X2, higher scene LV is needed to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, i4X2 is the cutoff scene LV to exclude the possibility of night scene
                /**************************************************/
                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            /**************************************************/
            // Feature name: HDR geometry information (for HDR detector)
            // Calculating for HDR detector by geometry information
            // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
            // rHdrGeometryInfo
            /**************************************************/
            {               
                /**************************************************/
                // Feature name: geometry center over exposure probability
                // HDR scene share the same property as center over exposure
                // rGeometryCOEProp
                /**************************************************/
                {             
                    /**************************************************/             
                    // Variable name: bGeometryCOEEnable
                    // A switch for HDR detector by geometry COE
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry COE
                    //
                    // Variable name: i4COEWeight
                    // Weight to control the importance of COE in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If COE is important to detect the HDR scene, set i4COEWeight >= 1024.
                    /**************************************************/  
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight
                    /**************************************************/
                    // Feature name: HDR center over-exposure outer ratio
                    // The outer geometry average information of AE statistics for HDR detector
                    // Influence: COE outer probability, COE outer weight probability
                    // rHdrCOEOuterRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer average threshold to get max COE probability (i4Y1)
                    // Unit: Y value in 8 bits
                    // Default value: 23
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The outer average <= i4X1 will get max COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Max COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 1024, the outer average <= i4X1 will get 100% COE probability
                    //         If i4Y1 <= 1024, it means that we're not 100% sure for COE, judging by outer average
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer average threshold to get min COE probability (i4Y2)
                    // Unit: Y value in 8 bits
                    // Default value: 47
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The higher i4X2, higher outer average is needed to exclude the possibility of COE
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Min COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 0, i4X2 is the cutoff outer average to exclude the possibility of COE
                    /**************************************************/
                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center over-exposure difference ratio
                    // The outer/inner geometry EV difference of AE statistics for HDR detector
                    // Influence: COE difference probability, COE difference weight probability, HDR on/off
                    // rHdrCOEDiffRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer/inner difference threshold to get min COE probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 1900
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger outer/inner geometry EV difference zone will get min COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Min COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff outer/inner geometry EV difference
                    //         (EV difference < i4X1 will get zero probability)
                    //         If i4Y1 > 0, we will get probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of COE for any outer/inner geometry input)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer/inner difference threshold to get max COE probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 2400
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of outer/inner geometry EV difference get max COE probability (i4Y2)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Max COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the outer/inner EV difference >= i4X2 will get COE probability of i4Y2
                    /**************************************************/
                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                /**************************************************/
                // Feature name: HDR anti-back-light(ABL) probability
                // HDR scene share the same property as back-light
                // rGeometryABLProp
                /**************************************************/
                {             
                    /**************************************************/
                    // Variable name: bGeometryABLEnable
                    // A switch for HDR detector by geometry ABL
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry ABL
                    //
                    // Variable name: i4ABLWeight
                    // Weight to control the importance of ABL in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If ABL is important to detect the HDR scene, set i4ABLWeight >= 1024.
                    /**************************************************/
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    /**************************************************/
                    // Feature name: HDR center bright difference ratio
                    // The ABL target probability table when center is brighter than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterBrightDiffRatio
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center dark difference ratio
                    // The ABL target probability table when center is darker than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterDarkDiffRatio
                    // Varaible name: rCenterDarkDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100   
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100   
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
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
                /**************************************************/
                // Variable name: bLVDetectEnable
                // A switch to use LV to detect HDR scene
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable HDR LV analysis
                //
                // Varaible name: i4LVProbTbl
                // LV table for detecting HDR scene
                // Unit: %
                // Default value: { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
                // Data range: Integer, 0 ~ 100
                // Constraints: N/A.
                // Effect: Set the probability of HDR scene according to LV information
                /**************************************************/
                TRUE,           // bLVDetectEnable
               
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },
        
        /**************************************************/
        // Feature name: HDR auto-enhance
        // Calculating information for HDR ratio
        // rHdrAutoEnhance
        /**************************************************/
        {                       
            /**************************************************/
            // Variable name: bAutoEnhanceEnable
            // A switch to calculate HDR ratio information
            // Default value: TRUE.
            // Data range: TRUE or FALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR BV analysis
            //
            // Varaible name: i4RMGk
            // Control the height of turning point to LE/SE curve
            // Default value: 4095
            // Data range: Integer, 0 ~ 4095
            // Constraints: N/A.
            // Effect: Set i4RMGk higher(lower) to make LE longer(shorter)
            //
            // Varaible name: i4RMGtRatio
            // Set the percentage of current Y for calculating RMGt
            // Unit: 10*%
            // Default value: 40
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The higher i4RMGtRatio, more percentage of histogram will be accumulated in current Y
            /**************************************************/
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            40, //20,           // i4RMGtRatio

            /**************************************************/
            // Feature name: RMGt
            // Interpolation table for RMGt
            // Influence: the turning point between LE/SE curve
            // If current Y is large(small), use data of SE(LE) more
            // rRMGt
            //
            // Varaible name: rRMGt.i4X1
            // Max current Y threshold to get max RMGt i4Y1
            // Unit: Y value in 8 bits
            // Default value: 160
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X1, larger current Y zone will get max RMGt setting (i4Y1)
            //         The current Y lower than i4Y1 will get longer LE curve.
            //
            // Varaible name: rRMGt.i4Y1
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 512
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y1, the longer LE curve we will get
            //
            // Varaible name: rRMGt.i4X2
            // Min current Y to get min RMGt i4Y2
            // Unit: Y value in 8 bits
            // Default value: 220
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller current Y zone will get min RMGt setting (i4Y2)
            //         The current Y higher than i4Y1 will get shorter LE curve.
            //
            // Varaible name: rRMGt.i4Y2
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 384
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y2, the longer LE curve we will gets
            /**************************************************/
            {                       // rRMGt
                160,                // i4X1
                512, //768,         // i4Y1
                220,                // i4X2
                384 //256, //512    // i4Y2
            },
            //Not used
            /**************************************************/
            2,                  // i4SupportHdrNum
            1024,               // i4AutoEnhanceProb
            1024,               // i4HdrWeight
            /**************************************************/
            
            // rAutoBright
            {                   
                /**************************************************/
                // Varaible name: i4LVRatio50
                // Ratio interpolation table by scene LV, for 50 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                //
                // Varaible name: i4LVRatio60
                // Ratio interpolation table by scene LV, for 60 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                /**************************************************/
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio50
                {   100, 100, 100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200,  200,  200,  200, 200, 200, 200},
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio60
                {   100, 100, 100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200,  200,  200,  200, 200, 200, 200},

                /**************************************************/
                // Feature name: rContrastRatio
                // HDR ratio interpolation table by HDR contrast EV difference
                // Influence: auto contrast ratio, HDR ratio
                // rContrastRatio
                //
                // Varaible name: rContrastRatio.i4X1
                // Max contrast EVD threshold to get min HDR ratio i4Y1
                // Unit: EV*1000
                // Default value: 3500
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger contrast EVD zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rContrastRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 100
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rContrastRatio.i4X2
                // Min contrast EVD to get max HDR ratio i4Y2
                // Unit: EV*1000
                // Default value: 4500
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller contrast EVD zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rContrastRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {                 // rContrastRatio
                    3500, //4000, // i4X1
                    100,          // i4Y1
                    4500, //5000, // i4X2
                    200           // i4Y2
                },

                /**************************************************/
                // Feature name: rBrightRatio
                // HDR ratio interpolation table by HDR bright side histogram information
                // Influence: auto bright probability, HDR ratio
                //rBrightRatio
                //
                // Varaible name: rBrightRatio.i4X1
                // Max bright side histogram threshold to get HDR ratio i4Y1
                // Unit: weighting average pixel count
                // Default value: 600
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger bright side histogram zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rBrightRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rBrightRatio.i4X2
                // Min bright side histogram to get max HDR ratio i4Y2
                // Unit: weighting average pixel count
                // Default value: 1100
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller bright side histogram zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rBrightRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: rNightProbRatio
                // HDR ratio interpolation table by night scene probability
                // Influence: auto night probability, HDR ratio
                // rNightProbRatio
                // Varaible name: rNightProbRatio.i4X1
                // Max night scene probability threthold to get max HDR ratio i4Y1
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rNightProbRatio.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rNightProbRatio.i4X2
                // Min night scene probability to get max HDR ratio i4Y2
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rNightProbRatio.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rNightProbRatio
                    0,          // i4X1
                    200,        // i4Y1
                    512,        // i4X2
                    100         // i4Y2
                },

                /**************************************************/
                // Feature name: motion information
                // The motion corresponding information including gyro, accelerator,
                // Influence: HDR ratio, HDR on/off, on/off smoothness
                // rGGyroSensor
                /**************************************************/
                {
                    /**************************************************/
                    // Feature name: rAcceProb
                    // Acceleration probability interpolation table by accelerator information
                    // Influence: HDR ratio, motion on/off
                    // rAcceProb
                    //
                    // Varaible name: rAcceProb.i4X1
                    // Max acceleration threthold to get min acceleration probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger acceleration zone will get min acceleration probability (i4Y1)
                    //
                    // Varaible name: rAcceProb.i4Y1
                    // Min acceleration probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff acceleration to get min acceleration probability
                    //
                    // Varaible name: rAcceProb.i4X2
                    // Min acceleration to get max acceleration probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller acceleration zone will get max acceleration probability (i4Y2)
                    //
                    // Varaible name: rAcceProb.i4Y2
                    // Max acceleration probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the acceleration > i4X2 will get 100% acceleration probability
                    /**************************************************/
                    {               // rAcceProb
                        1100, //980,        // i4X1
                        0,          // i4Y1
                        1500,       // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rGyroProb
                    // Motion probability interpolation table by gyro sensor information
                    // Influence: HDR ratio, motion on/off
                    // rGyroProb
                    //
                    // Varaible name: rGyroProb.i4X1
                    // Max motion threthold to get min motion probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion zone will get min motion probability (i4Y1)
                    //
                    // Varaible name: rGyroProb.i4Y1
                    // Min motion probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff motion to get min motion probability
                    //
                    // Varaible name: rGyroProb.i4X2
                    // Min motion to get max motion probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion zone will get max motion probability (i4Y2)
                    //
                    // Varaible name: rGyroProb.i4Y2
                    // Max motion probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the motion > i4X2 will get 100% motion probability
                    /**************************************************/
                    {               // rGyroProb
                        50, //100,  // i4X1
                        0,          // i4Y1
                        200, //700, // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rMotionRatio
                    // HDR ratio interpolation table by motion (gyro + acce) probability information
                    // Influence: HDR ratio
                    // rMotionRatio
                    //
                    // Varaible name: rMotionRatio.i4X1
                    // Max motion probability threthold to get min HDR ratio i4Y1
                    // Unit: 2048*%/100
                    // Default value: 512
                    // Data range: Integer, 0 ~ 2048
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion probability zone will get max HDR ratio (i4Y1)
                    //
                    // Varaible name: rMotionRatio.i4Y1
                    // Max HDR ratio according to i4X1
                    // Unit: 100*LE/SE
                    // Default value: 400
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    //
                    // Varaible name: rMotionRatio.i4X2
                    // Min motion probability to get max HDR ratio i4Y2
                    // Unit: 2048*%/100
                    // Default value: 960
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion probability zone will get min HDR ratio (i4Y2)
                    //
                    // Varaible name: rMotionRatio.i4Y2
                    // Min HDR ratio according to i4X2
                    // Unit: 100*LE/SE
                    // Default value: 100
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    /**************************************************/
                    {               // rMotionRatio
                        512,        // i4X1
                        400,        // i4Y1
                        960,        // i4X2
                        100         // i4Y2
                    },

                    /**************************************************/
                    // Varaible name: i4MotionOnThr
                    // Threshold to determine motion on/off
                    // Unit: 1024*%/100
                    // Default value: 512 or 768
                    // Data range: Integer, i4MotionOnThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnThr, you need higher motion probability to set motion detector on
                    //
                    // Varaible name: i4MotionOnCntThr
                    // Counting threshold to update motion on/off information
                    // Unit: frame count
                    // Default value: 2 or 5
                    // Data range: Integer, i4MotionOnCntThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnCntThr, you need count more to update motion on/off
                    /**************************************************/
                    512, //768,            // i4MotionOnThr
                    2               //i4MotionOnCntThr
                }
            },

            /**************************************************/
            // Feature name: rAutoFlicker
            // Parameter setting to automatically avoid flicker
            // rAutoFlicker
            /**************************************************/
            {              
                /**************************************************/
                // Varaible name: i4Flkr1LVThr
                // LV threshold for deflicker enable
                // Unit: LV*10
                // Default value: {130,130}
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The higher i4Flkr1LVThr, the higher scene LV needed to enable deflicker
                //
                // Varaible name: i4MinRatioAntiFlk
                // The min acceptable ratio for anti-flicker
                // Unit: 100*LE/SE
                // Default value: 200 or 800
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The ratio lower i4MinRatioAntiFlk than could get flickered results
                /**************************************************/
                {130,130},        // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, // i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     // i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/

                /**************************************************/
                // Feature name: rIsoThr
                // HDR ratio interpolation table by ISO information
                // Influence: HDR ratio
                // rIsoThr
                //
                // Varaible name: rIsoThr.i4X1
                // Max ISO threshold to get max HDR ratio i4Y1
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger ISO zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rIsoThr.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rIsoThr.i4X2
                // Min ISO to get min HDR ratio i4Y2
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller ISO zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rIsoThr.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rIsoThr
                    12288,      //i4X1:1024 * 12
                    4,          //i4Y1
                    14336,      //i4X2:1024 * 10
                    1            //i4Y2
                },
            },

            /**************************************************/
            // Feature name: rAutoSmooth
            // HDR auto smooth (HDR on/off, motion on/off, HDR ratio smooth) information
            // rAutoSmooth
            /**************************************************/
            {                
                /**************************************************/
                // Variable name: bHdrWaitAEStable
                // A switch to determine wait for AE stable or not
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable wait
                //
                // Variable name: i4HdrInfoCounterThr
                // Threshold to update HDR on/off information
                // Unit: frame count
                // Default value: 6
                // Data range: Integer, i4HdrInfoCounterThr > 0
                // Constraints: N/A.
                // Effect: HDR on/off counter larger than i4HdrInfoCounterThr can update the on/off result
                /**************************************************/
                TRUE,           // bHdrWaitAEStable
                6,              // i4HdrInfoCounterThr

                //Not used
                /**************************************************/
                TRUE,           // bRatioSmoothEnable TRUE for platform / FALSE for simulation
                /**************************************************/

                /**************************************************/
                // Variable name: i4RatioSmoothThr
                // Threshold to update HDR ratio smooth information
                // Unit: frame count
                // Default value: none
                // Data range: Integer, i4RatioSmoothThr > 0
                // Constraints: N/A.
                // Effect: HDR ratio smooth counter larger than i4RatioSmoothThr can update the smoothed HDR ratio result
                //
                // Variable name: i4RoughRatioUnit
                // Threshold to check if difference of current ratio and current stable ratio is large enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4RoughRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current ratio and current stable ratio larger than i4RoughRatioUnit check for close results in incoming frames
                //
                // Variable name: i4FinerRatioUnit
                // Threshold to check if difference of current/previous ratio is small enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4FinerRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current/previous HDR ratio > i4FinerRatioUnit start to count
                /**************************************************/
                { 2,   2},      // i4RatioSmoothThr
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
            }
            /**************************************************/
        },

        /**************************************************/
        // Feature name: rHdrAETarget
        // HDR AE target parameter setting
        // rHdrAETarget
        /**************************************************/
        {   
            /**************************************************/
            // Variable name: i4DefTarget
            // Default AE target
            // Unit: Y value in 8 bits
            // Default value: 47
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: A reference of AE target, set higher to make AE target higher
            //
            // Variable name: i4DefWeight
            // Weight of default target, mixing with normal AE target
            // Default value: 47
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: Set i4DefWeight higher to make the proportion of default AE target higher
            /**************************************************/
            47,                 // i4DefTarget
            0, //256,           // i4DefWeight

            /**************************************************/
            // Feature name: rAutoEVWeight
            // HDR auto EV weight interpolation table
            // Influence: auto EV weight, HDR auto weight
            // rAutoEVWeight
            //
            // Varaible name: rAutoEVWeight.i4X1
            // Max contrast EVD threshold to get min auto EV weight (i4Y1)
            // Unit: EV*1000
            // Default value: 4000 or 3750
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger contrast EVD zone will get min auto EV weight (i4Y1)
            //
            // Varaible name: rAutoEVWeight.i4Y1
            // Min auto EV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoEVWeight.i4X2
            // Min contrast EVD threshold to get max auto EV weight i4Y2
            // Unit: EV*1000
            // Default value: 4500 or 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller contrast EVD zone will get max auto EV weight (i4Y2)
            //
            // Varaible name: rAutoEVWeight.i4Y2
            // Max auto EV weight according to i4X2
            // Unit: 1024*%/100
            // Default value:  512 or 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The contrast EVD larger than i4X2 will get auto EV weight of i4Y2
            /**************************************************/
            {                   // rAutoEVWeight
                4000,           // rAutoEVWeight.i4X1
                256,            // rAutoEVWeight.i4Y1
                5000,           // rAutoEVWeight.i4X2
                512             // rAutoEVWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoBrightWeight
            // HDR auto bright weight interpolation table
            // Influence: auto bright weight, auto night weight, HDR auto weight
            // rAutoBrightWeight
            //
            // Varaible name: rAutoBrightWeight.i4X1
            // Max bright enhance probability threshold to get min auto bright weight (i4Y1)
            // Unit: weighting average pixel count
            // Default value: 4000
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger bright enhance probability zone will get min auto bright weight (i4Y1)
            //
            // Varaible name: rAutoBrightWeight.i4Y1
            // Min auto bright weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoBrightWeight.i4X2
            // Min bright enhance probability threshold to get max auto bright weight i4Y2
            // Unit: weighting average pixel count
            // Default value: 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller bright enhance probability zone will get max auto bright weight (i4Y2)
            //
            // Varaible name: rAutoBrightWeight.i4Y2
            // Max auto bright weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The bright enhance probability larger than i4X2 will get max auto bright weight of i4Y2
            /**************************************************/
            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoABLWeight
            // HDR auto ABL weight interpolation table
            // Influence: auto ABL weight, auto night weight, HDR auto weight
            // rAutoABLWeight
            //
            // Varaible name: rAutoABLWeight.i4X1
            // Max ABL probability threshold to get min auto ABL weight (i4Y1)
            // Unit: 1024*%/100
            // Default value: 0
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X1, larger ABL probability zone will get min auto ABL weight (i4Y1)
            //
            // Varaible name: rAutoABLWeight.i4Y1
            // Min auto ABL weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoABLWeight.i4X2
            // Min ABL probability threshold to get max auto ABL weight i4Y2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller ABL probability zone will get max auto ABL weight (i4Y2)
            //
            // Varaible name: rAutoABLWeight.i4Y2
            // Max auto ABL weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The ABL probability larger than i4X2 will get max auto ABL weight of i4Y2
            /**************************************************/
            {                   // rAutoABLWeight
                0,              // rAutoABLWeight.i4X1
                1024,           // rAutoABLWeight.i4Y1
                512,            // rAutoABLWeight.i4X2
                1024            // rAutoABLWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightL
            // HDR auto LV weight interpolation table
            // It is for low LV specially, LV higher than 40 will get weight 1024
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightL
            //
            // Varaible name: rAutoLVWeightL.i4X1
            // Max LV threshold to get min auto LV weight (i4Y1)
            // Unit: LV*10
            // Default value: 30
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight (i4Y1)
            //
            // Varaible name: rAutoLVWeightL.i4Y1
            // Min auto LV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoLVWeightL.i4X2
            // Min LV threshold to get max auto LV weight i4Y2
            // Unit: LV*10
            // Default value: 40
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight (i4Y2)
            //
            // Varaible name: rAutoLVWeightL.i4Y2
            // Max auto LV weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight of i4Y2              
            /**************************************************/
            {                   // rAutoLVWeightL
                 30,            // rAutoLVWeightL.i4X1
                768,            // rAutoLVWeightL.i4Y1
                 40,            // rAutoLVWeightL.i4X2
               1024             // rAutoLVWeightL.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightH
            // HDR auto LV weight lower bound, determined by high LV
            // If scene LV is high, it raise HDR AE weight
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightH
            //
            // Varaible name: rAutoLVWeightH.i4X1
            // Max LV threshold to get min auto LV weight lower bound (i4Y1)
            // Unit: LV*10
            // Default value: 100
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight lower bound (i4Y1)
            //
            // Varaible name: rAutoLVWeightH.i4Y1
            // Min auto LV weight lower bound according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight for auto LV lower bound
            //
            // Varaible name: rAutoLVWeightH.i4X2
            // Min LV threshold to get max auto LV weight lower bound i4Y2
            // Unit: LV*10
            // Default value: 130
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight lower bound (i4Y2)
            //
            // Varaible name: rAutoLVWeightH.i4Y2
            // Max auto LV weight lower bound according to i4X2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight lower bound (i4Y2)
            /**************************************************/
            {                   // rAutoLVWeightH
                100,            // rAutoLVWeightH.i4X1
                256,            // rAutoLVWeightH.i4Y1
                130,            // rAutoLVWeightH.i4X2
                512             // rAutoLVWeightH.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoNightWeight
            // HDR auto weight interpolation table, determined by night scene probability
            // Influence: HDR auto weight
            // rAutoNightWeight
            //
            // Varaible name: rAutoNightWeight.i4X1
            // Max night scene probability threshold to get max HDR auto weight (i4Y1)
            // Unit: LV*10
            // Default value: 1024
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger night scene probability zone will get max HDR auto weight (i4Y1)
            //
            // Varaible name: rAutoNightWeight.i4Y1
            // Max HDR auto weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y1, the higher HDR auto weight we will get
            //
            // Varaible name: rAutoNightWeight.i4X2
            // Min night scene probability threshold to get min HDR auto weight i4Y2
            // Unit: LV*10
            // Default value: 2048
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller night scene probability zone will get min HDR auto weight (i4Y2)
            //
            // Varaible name: rAutoNightWeight.i4Y2
            // Min HDR auto weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y2, the higher HDR auto weight we will get                
            /**************************************************/
            {                   // rAutoNightWeight
                1024,           // rAutoNightWeight.i4X1
                512,            // rAutoNightWeight.i4Y1
                2048,           // rAutoNightWeight.i4X2
                256             // rAutoNightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rHdrAoeTarget
            // AOE information for HDR AE calculation
            // HDR scene share the same property as over exposure
            //
            // Varaible name: i4AoePercent
            // The percent for AOE to calculate current AOE Y from bright
            // Unit: 10*%
            // Default value: 25
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The larger i4AoePercent, more percent of histogram from the bright side will be accumulated for AOE current Y
            //
            // Varaible name: i4AoeLV
            // The LV thresholds to deliver different AOE target
            // Unit: LV*10
            // Default value: {50, 100, 150}
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: Differentiate the AOE target by LV
            //
            // Varaible name: i4AoeTarget
            // Different AOE targets differentiated by LV
            // Unit: Y value in 8 bits
            // Default value: {200, 200, 200}
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: Different LV delivers different AOE target
            /**************************************************/
            {
                25, //20,       // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            /**************************************************/
            // Feature name: rHdrNightTarget
            // Night scene information for HDR AE calculation
            // HDR scene has almost the same property as night scene,
            // but if we use higher HDR ratio in night scene, 
            // the noise will be large because of the high ISO in night scene.
            // So, night scene should use lower HDR ratio
            // rHdrNightTarget
            /**************************************************/
            {                   
                /**************************************************/
                // Varaible name: i4NightPercent
                // The percent for night scene to calculate current night scene Y from bright
                // Default value: 150
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The larger i4NightPercent, more percent of histogram from the bright side will be accumulated for night scene current Y
                /**************************************************/
                150,             // i4NightPercent

                /**************************************************/
                // Feature name: rNightTarget
                // Night scene target interpolation table, determined by night scene probability
                // Influence: HDR night scene target Y, HDR night scene target gain, HDR target gain, HDR target Y
                // rNightTarget
                //
                // Varaible name: rNightTarget.i4X1
                // Max night scene probability threshold to get max night scene target (i4Y1)
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max night scene target (i4Y1)
                //
                // Varaible name: rNightTarget.i4Y1
                // Max night scene target according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 80
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher night scene target we will get
                //
                // Varaible name: rNightTarget.i4X2
                // Min night scene probability threshold to get min night scene target i4Y2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min night scene target (i4Y2)
                //
                // Varaible name: rNightTarget.i4Y2
                // Min night scene target according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 60
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher night scene target we will get                
                /**************************************************/
                {               // rNightTarget
                    0,          // i4X1
                    80,         // i4Y1
                    1024,       // i4X2
                    60          // i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitL
            // HDR AE target clamping for low LV
            // The LV lower than the low LV threshold use this table,
            // between low LV threshold and medium LV threshold use the interpolation result of medium/low LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitL
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitL.i4LVLimit
                // Low LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 60
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV value lower than rTargetLimitL.i4LVLimit should use rTargetMinLimit interpolation table
                /**************************************************/
                60,             // rTargetLimitL.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMinLimit
                // For low LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for low LV scene
                // rTargetLimitL.rTargetMinLimit
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR target threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMaxLimit
                // For low LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for low LV scene
                // rTargetLimitL.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold 
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitM
            // HDR AE target clamping for medium LV
            // The LV between low LV threshold and medium LV threshold use the interpolation result of low/medium LV
            // The LV between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for medium LV scene
            // rTargetLimitM
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitM.i4LVLimit
                // Medium LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The rTargetLimitL.i4LVLimit < LV < rTargetLimitM.i4LVLimit should use rTargetLimitM interpolation table      
                /**************************************************/
                80,             // rTargetLimitM.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMinLimit
                // For medium LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for medium LV scene
                // rTargetLimitM.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get
                /**************************************************/
                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMaxLimit
                // For medium LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for medium LV scene
                // rTargetLimitM.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get            
                /**************************************************/
                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitH
            // HDR AE target clamping for high LV
            // The LV higher than the high LV threshold use this table,
            // between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitH
            /**************************************************/
            {           
                /**************************************************/
                // Varaible name: rTargetLimitH.i4LVLimit
                // High LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV larger than rTargetLimitH.i4LVLimit should use rTargetLimitH interpolation table               
                /**************************************************/
                120,            // rTargetLimitH.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMinLimit
                // For high LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for high LV scene
                // rTargetLimitH.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 25
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 40
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get            
                /**************************************************/
                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMaxLimit
                // For high LV, HDR AE target max threshold interpolation table
                // Influence: HDR AE target Y max clamping range for high LV scene
                // rTargetLimitH.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get                
                /**************************************************/
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
                /**************************************************/
                // Feature name: rFaceHdrProb
                // Face HDR probability interpolation table
                // rFaceHdrProb
                //
                // Varaible name: rFaceHdrProb.i4X1
                // Max face bright tone probability to get max night scene target (i4Y1)
                // Unit: weighting average pixel count
                // Default value: 2200
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger face bright tone probability zone will get max face HDR target (i4Y1)
                //
                // Varaible name: rFaceHdrProb.i4Y1
                // Max face HDR target according to i4X1
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher face HDR target we will get
                //
                // Varaible name: rFaceHdrProb.i4X2
                // Min face bright tone probability threshold to get min face HDR target i4Y2
                // Unit: weighting average pixel count
                // Default value: 3200
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller face bright tone probability zone will get min face HDR target (i4Y2)
                //
                // Varaible name: rFaceHdrProb.i4Y2
                // Min face HDR target according to i4X2
                // Unit: 1024*%/100
                // Default value: 256
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher face HDR target we will get                
                /**************************************************/
                {               // rFaceHdrProb
                    600,        // i4X1
                    400,        // i4Y1
                   1100,        // i4X2
                    400         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        /**************************************************/
        // Feature name: rHdrcHdr
        // Calculate cHDR detection information
        // rHdrcHdr      
        /**************************************************/
        {   
            /**************************************************/
            // Varaible name: i4LEThr
            // Current Y accumulated histogram from dark for LE information
            // Unit: Y value in 8 bits
            // Default value: 40
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4LEThr, more bins of histogram will be accumulated from dark side
            //
            // Varaible name: i4SERatio
            // Current Y accumulated histogram percentage from bright for SE information
            // Unit:10*%
            // Default value: 20
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SERatio, more % of histogram will be accumulated from bright side                   
            //  
            // Varaible name: i4SETarget
            // SE default target Y
            // Unit: Y value in 8 bits
            // Default value: 180
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SETarget, the higher SE target Y
            // 
            // Varaible name: i4BaseGain
            // The base gain for AE target
            // Unit: 1024*gain
            // Default value: 1024
            // Data range: Integer, i4BaseGain > 0
            // Constraints: N/A.
            // Effect: The higher i4BaseGain, the higher base for gain
            /**************************************************/
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
        TRUE,              // Perframe HDR AE smooth option

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
getVTAEParam<ESensorDev_Sub>()
{
    static strAEParamCFG g_AEParasetting =
    {
        FALSE,               // bEnableSaturationCheck
        FALSE,                // bEnablePreIndex
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
        0,                    //bEnableFlareFastConverge
        0,                    //bEnableRAFastConverge
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
        1,                  //uCycleNumAESkipAfterExit;
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
        {35,    10},//u4Size_X[2];
        {1024, 256},//u4Size_Y[2];
        {200,  600},//u4Loc_X[2];
        {1024, 256} //u4Loc_Y[2];
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
    };
    static strPSOConverge g_AEPsoConverge = {
        TRUE,
        100, //Speed control by PSO under wash-out (100 means 100%, fastest)        
        70,  //Speed control by PSO under non-linear area (100 means 100%, fastest)
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
          5,                  // face lost max count
         90,                  // face window scale ratio range 1~400% default 90%
         30,                //face NS lowbound
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
        {-2000, 0, 3000,5500},         // i4BV
        {150, 170, 180, 180}            // u4THD
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
       {-1000, -300},
       {1024,   0},
    };
    static strHistStableCFG g_HistStableCFG =
    {
        FALSE,                                            // bEnableStablebyHist
        250,                                                //u4HistEVDiff : 0.25EV
        242,                                                //u4OverexpoTHD  : 95%
        13,                                                 //u4UnderexpoTHD : 5%
        140,                                                //u4HistStableTHD : 15%(1024base)
    };
    static strOverExpoAOECFG g_strOverExpoAOECFG =
    {
        TRUE,                                            // bEnableOverExpoAOE
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
        TRUE,                           //bEnableColorSuppressR
        TRUE,                           //bEnableColorSuppressG
        FALSE                         //bEnableColorSuppressB
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
            256                          // DynWTRatioTbl.i4Y2 : add delta weight 1024base
        }
    };
    static strNSBVCFG g_NSBVCFG =
    {
        FALSE,   //bEnableNSBVCFG
        3,      //u4TblLength
        {-4500, -1000, 3000},       //i4BV[16]
        {55,       80,   80},   //u4BTTHD[16]
    };
    static HDR_AE_PARAM_T g_strHdrAECFG =
    {
        eSubHDR_VT,             // eHdrParamID

        /**************************************************/
        // Feature name: HDR detector parameter information
        // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
        // rHdrDetector
        /**************************************************/
        {                      
            /**************************************************/                   
            // Variable name: bHdrDetectorEnable
            // A switch to turn on/off of HDR detector
            // Default value: MTRUE.
            // Data range: MTRUE or MFALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR detector
            //
            // Variable name: i4HdrConfidenceThr
            // Threshold to turn HDR on/off
            // Unit: %
            // Default value: 80 or 220
            // Data range: Integer, i4HdrConfidenceThr > 0
            // Constraints: N/A.
            // Effect: Turn HDR on when result of HDR scene probability is larger than i4HdrConfidenceThr
            /**************************************************/
            MTRUE,              // bHdrDetectorEnable
            80, // 20,          // i4HdrConfidenceThr

            /**************************************************/
            // Feature name: rHdrHistInfo
            // HDR detector - histogram information calculation
            // Detect HDR scene by calculating full RGB histogram information
            // Find the proportion of pixel at bright/dark side of an image to check HDR characteristics in histogram
            /**************************************************/
            {                   
                /**************************************************/                
                // Variable name: bHistDetectEnable
                // A switch to turn on/off histogram analysis of HDR detector
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable histogram analysis of HDR detector
                /**************************************************/
                TRUE,           // bHistDetectEnable

                /**************************************************/
                // Feature name: HDR detector - dark weight
                // Dark side weighting table for HDR detector
                // Multiply full RGB histogram with dark side weight and add the result of bright side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the dark part of image
                // A pixel will be treated as a dark pixel, if its pixel value is lower than certain threshold
                // Here are the parameters to control the threshold of dark pixel and the corresponding weighting
                // rHdrDetectDarkWtWin
                //
                // Varaible name: i4HdrHistP0
                // Dark side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 6
                // Data range: Integer, 0 < i4HdrHistP0 < i4HdrHistP1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the more histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistP1
                // Dark side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 12
                // Data range: Integer, i4HdrHistP0 < i4HdrHistP1 <= 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the more histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistW0
                // Dark side higher pixel weight
                // Default value: 100
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins before i4HdrHistP0 will get higher fully weight
                //
                // Varaible name: i4HdrHistW1
                // Dark side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW1 < i4HdrHistW0
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                /**************************************************/
                {               // rHdrDetectDarkWtWin
                    6,          // i4HdrHistP0
                    12,         // i4HdrHistP1
                    100,        // i4HdrHistW0
                    0           // i4HdrHistW1
                },

                /**************************************************/
                // Feature name: HDR detector - bright weight
                // Bright side weighting table for HDR detector
                // Multiply full RGB histogram with bright side weight and add the result of dark side to find HDR histogram probability
                // Here are the parameters to control the HDR ON sensitivity to the bright part of image
                // A pixel will be treated as a bright pixel, if its pixel value is higher than certain threshold
                // Influence: HDR probability, HDR on/off
                // rHdrDetectBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 180
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrDetectBrightWtWin
                    180,        // i4HdrHistP0
                    220,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: HDR auto enhance bright weight
                // Weighting table, enhance histogram bright side
                // Multiply full RGB histogram with the bright side weight
                // It is used to calculate bright enhance probability
                // Influence: HDR auto weight, HDR ratio
                // rHdrEnhanceBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrEnhanceBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },
                /**************************************************/
                // Feature name: rHdrFDBrightWtWin
                // Histogram from bright side weighting table for face detection
                // Multiply full RGB histogram with the face detection bright side weight
                // It is used to calculate face detection bright tone probability
                // Influence: HDR face probability
                // rHdrFDBrightWtWin
                //
                // Varaible name: i4HdrHistP0
                // Face detection bright side histogram bin threshold to partial weight
                // Unit: Y value in 8 bits
                // Default value: 220
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP0, the fewer histogram bins are taken into partial consideration
                //
                // Varaible name: i4HdrHistP1
                // Face detection bright side histogram bin threshold to full weight
                // Unit: Y value in 8 bits
                // Default value: 245
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4HdrHistP1, the fewer histogram bins are taken into fully consideration
                //
                // Varaible name: i4HdrHistW0
                // Face detection bright side lower pixel weight
                // Default value: 0
                // Data range: Integer, 0 < i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW0, bins between (i4HdrHistP0, i4HdrHistP1) will get higher partial weight
                //
                // Varaible name: i4HdrHistW1
                // Face detection bright side higher pixel weight
                // Default value: 300
                // Data range: Integer, i4HdrHistW0 <i4HdrHistW1
                // Constraints: N/A.
                // Effect: The higher i4HdrHistW1, bins after i4HdrHistP1 will get higher fully weight
                /**************************************************/
                {               // rHdrFDBrightWtWin
                    220,        // i4HdrHistP0
                    245,        // i4HdrHistP1
                    0,          // i4HdrHistW0
                    300         // i4HdrHistW1
                },

                /**************************************************/
                // Varaible name: i4HdrContrastPercentH
                // Bright side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 60
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrContrastPercentL
                // Dark side accumulated percentage for calculating contrast EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4ContrastHistRatioL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightEVDiffPercentH
                // Bright side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 50 or 30
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher the i4HdrNightEVDiffPercentH, more % of histogram will be accumulated from bright side
                //
                // Varaible name: i4HdrNightEVDiffPercentL
                // Dark side accumulated percentage for calculating night scene EV difference
                // Unit: 10*%
                // Default value: 300
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The higher i4HdrNightEVDiffPercentL, more % of histogram will be accumulated from dark side
                //
                // Varaible name: i4HdrNightDarkPercent
                // Night scene histogram Y %
                // It is used to calculate i4NightHistProb
                // Unit: 10*%
                // Default value: 500
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: It affects the calculation of histogram Y. The percentage depends on your purpose.
                /**************************************************/
                30,             // i4ContrastHistRatioH
                300,            // i4ContrastHistRatioL
                30,             // i4HdrNightEVDiffPercentH
                300,            // i4HdrNightEVDiffPercentL
                500,            // i4HdrNightDarkPercent

                /**************************************************/
                // Feature name: HDR night scene EV difference probability
                // The probability table interpolating for night scene probability by EV difference
                // Influence: night EVD probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightEVDiffProb
                //
                // Varaible name: rHdrNightEVDiffProb.i4X1
                // Min EV difference threshold to get min night scene probability (i4Y1)
                // Unit: EV*1000
                // Default value: 4800
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X1, larger EV difference zone will get min night scene probability
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y1
                // Min night scene probability according to i4X1
                // Unit: 1024*%/100 
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 0, i4X1 would be the cutoff EV difference
                //         (EV difference < i4X1 will get zero probability)
                //         If i4Y1 > 0, we will get probability >= i4Y1
                //         (If i4Y1 > 0, we have certain faith of night scene for any EV difference input)
                //
                // Varaible name: rHdrNightEVDiffProb.i4X2
                // Max EV difference threshold to get max night scene probability (i4Y2)
                // Unit: EV*1000
                // Default value: 6500
                // Data range: Integer
                // Constraints: N/A.
                // Effect: The larger i4X2, the smaller zone of EV difference get max night scene probability (i4Y2)
                //
                // Varaible name: rHdrNightEVDiffProb.i4Y2
                // Max night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 1024, the EV difference >= i4X2 will get night scene probability of i4Y2
                /**************************************************/
                {               // rHdrNightEVDiffProb
                    4800,       // rHdrNightEVDiffProb.i4X1
                       0,       // rHdrNightEVDiffProb.i4Y1
                    6500,       // rHdrNightEVDiffProb.i4X2
                    1024        // rHdrNightEVDiffProb.i4Y2
                },
                /**************************************************/
                // Feature name: HDR night scene histogram probability
                // The probability table for night scene probability by histogram avg Y
                // Influence: night histogram probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                // rHdrNightHistProb
                //
                // Varaible name: rHdrNightHistProb.i4X1
                // Max histogram avg Y threshold to get max night scene probability (i4Y1)
                // Unit: Y value in 8 bits
                // Default value: 2
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The avg Y <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightHistProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4X1 = 1024, the avg Y <= i4X1 will get 100% night scene probability
                //         If i4X1 < 1024, it means that we're not 100% sure for night scene, judging by avg Y
                //
                // Varaible name: rHdrNightHistProb.i4X2
                // Min histogram avg Y threshold to get min night scene probability (i4Y2)
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4X2, the higher avg Y we need to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightHistProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, then i4X2 is the cutoff avg Y to exclude the possibility of night scene
                /**************************************************/
                {               // rHdrNightHistProb
                       2,       // rHdrNightHistProb.i4X1
                    1024,       // rHdrNightHistProb.i4Y1
                      10,       // rHdrNightHistProb.i4X2
                       0        // rHdrNightHistProb.i4Y2
                },

                /**************************************************/
                // Feature name: HDR night scene LV probability
                // The probability table for night scene probability by scene LV
                // Influence: night LV probability, night scene probability, HDR ratio, HDR auto weight, HDR AE target Y/gain
                //  rHdrNightLVProb
                // Varaible name: rHdrNightLVProb.i4X1
                // Max scene LV threshold to get max night scene probability (i4Y1)
                // Unit: 10*LV
                // Default value: 50
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: Scene LV <= i4X1 will get max night scene probability (i4Y1)
                //
                // Varaible name: rHdrNightLVProb.i4Y1
                // Max night scene probability according to i4X1
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y1 = 1024, scene LV <= i4X1 will get 100% night scene probability
                //         If i4Y1 < 1024, it means that we cannot 100% exclude the posibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4X2
                // Min scene LV threshold to get min night scene probability (i4Y2)
                // Unit: 10*LV
                // Default value: 100
                // Data range: Integer, 0 ~ 190
                // Constraints: N/A.
                // Effect: The higher i4X2, higher scene LV is needed to exclude the possibility of night scene
                //
                // Varaible name: rHdrNightLVProb.i4Y2
                // Min night scene probability according to i4X2
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: If i4Y2 = 0, i4X2 is the cutoff scene LV to exclude the possibility of night scene
                /**************************************************/
                {
                      50,       // rHdrNightLVProb.i4X1
                    1024,       // rHdrNightLVProb.i4Y1
                     100,       // rHdrNightLVProb.i4X2
                       0        // rHdrNightLVProb.i4Y2
                }
            },

            /**************************************************/
            // Feature name: HDR geometry information (for HDR detector)
            // Calculating for HDR detector by geometry information
            // HDR detector will turn HDR ON if the HDR probability results of histogram, geometry, LV exceed threshold
            // rHdrGeometryInfo
            /**************************************************/
            {                 
                /**************************************************/
                // Feature name: geometry center over exposure probability
                // HDR scene share the same property as center over exposure
                // rGeometryCOEProp
                /**************************************************/
                {              
                    /**************************************************/             
                    // Variable name: bGeometryCOEEnable
                    // A switch for HDR detector by geometry COE
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry COE
                    //
                    // Variable name: i4COEWeight
                    // Weight to control the importance of COE in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If COE is important to detect the HDR scene, set i4COEWeight >= 1024.
                    /**************************************************/  
                    TRUE,       // bGeometryCOEEnable
                    1024,       // i4COEWeight
                    /**************************************************/
                    // Feature name: HDR center over-exposure outer ratio
                    // The outer geometry average information of AE statistics for HDR detector
                    // Influence: COE outer probability, COE outer weight probability
                    // rHdrCOEOuterRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer average threshold to get max COE probability (i4Y1)
                    // Unit: Y value in 8 bits
                    // Default value: 23
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The outer average <= i4X1 will get max COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Max COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 1024, the outer average <= i4X1 will get 100% COE probability
                    //         If i4Y1 <= 1024, it means that we're not 100% sure for COE, judging by outer average
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer average threshold to get min COE probability (i4Y2)
                    // Unit: Y value in 8 bits
                    // Default value: 47
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The higher i4X2, higher outer average is needed to exclude the possibility of COE
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Min COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 0, i4X2 is the cutoff outer average to exclude the possibility of COE
                    /**************************************************/
                    {           // rHdrCOEOuterRatio
                        23,     // i4X1
                        1024,   // i4Y1
                        47,     // i4X2
                        0       // i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center over-exposure difference ratio
                    // The outer/inner geometry EV difference of AE statistics for HDR detector
                    // Influence: COE difference probability, COE difference weight probability, HDR on/off
                    // rHdrCOEDiffRatio
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X1
                    // Max outer/inner difference threshold to get min COE probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 1900
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger outer/inner geometry EV difference zone will get min COE probability (i4Y1)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y1
                    // Min COE probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff outer/inner geometry EV difference
                    //         (EV difference < i4X1 will get zero probability)
                    //         If i4Y1 > 0, we will get probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of COE for any outer/inner geometry input)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4X2
                    // Min outer/inner difference threshold to get max COE probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 2400
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of outer/inner geometry EV difference get max COE probability (i4Y2)
                    //
                    // Varaible name: rHdrCOEOuterRatio.i4Y2
                    // Max COE probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the outer/inner EV difference >= i4X2 will get COE probability of i4Y2
                    /**************************************************/
                    {           // rHdrCOEDiffRatio
                        1900,   // i4X1
                        0,      // i4Y1
                        2400,   // i4X2
                        1024    // i4Y2
                    }
                 },

                /**************************************************/
                // Feature name: HDR anti-back-light(ABL) probability
                // HDR scene share the same property as back-light
                // rGeometryABLProp
                /**************************************************/
                {           
                    /**************************************************/
                    // Variable name: bGeometryABLEnable
                    // A switch for HDR detector by geometry ABL
                    // Default value: TRUE.
                    // Data range: TRUE or FALSE.
                    // Constraints: N/A.
                    // Effect: Set TRUE to enable geometry ABL
                    //
                    // Variable name: i4ABLWeight
                    // Weight to control the importance of ABL in detecting HDR scene
                    // Default value: 1024
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: If ABL is important to detect the HDR scene, set i4ABLWeight >= 1024.
                    /**************************************************/
                    TRUE,       // bGeometryABLEnable
                    1024,       // i4ABLWeight

                    /**************************************************/
                    // Feature name: HDR center bright difference ratio
                    // The ABL target probability table when center is brighter than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterBrightDiffRatio
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterBrightDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
                    {           // rCenterBrightDiffRatio
                        3150,   //i4X1
                        0,      //i4Y1
                        5000,   //i4X2
                        1024    //i4Y2
                    },

                    /**************************************************/
                    // Feature name: HDR center dark difference ratio
                    // The ABL target probability table when center is darker than C/U type
                    // Influence: ABL difference probability, HDR on/off
                    // rCenterDarkDiffRatio
                    // Varaible name: rCenterDarkDiffRatio.i4X1
                    // Max ABL EV difference threshold to get min ABL probability (i4Y1)
                    // Unit: EV*1000
                    // Default value: 3150
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger ABL EV difference zone will get min ABL probability (i4Y1)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y1
                    // Min ABL probability according to i4X1
                    // Unit: 1024*%/100   
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 would be the cutoff ABL EV difference
                    //         (EV difference < i4X1 will get zero ABL probability)
                    //         If i4Y1 > 0, we will get ABL probability >= i4Y1
                    //         (If i4Y1 > 0, we have certain faith of ABL for any ABL EV difference input)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4X2
                    // Min ABL EV difference threshold to get max ABL probability (i4Y2)
                    // Unit: EV*1000
                    // Default value: 5000
                    // Data range: Integer
                    // Constraints: N/A.
                    // Effect: The larger i4X2, the smaller zone of ABL EV difference get max ABL probability (i4Y2)
                    //
                    // Varaible name: rCenterDarkDiffRatio.i4Y2
                    // Max ABL probability according to i4X2
                    // Unit: 1024*%/100   
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the the ABL EV difference >= i4X2 will get ABL probability of i4Y2
                    /**************************************************/
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
                /**************************************************/
                // Variable name: bLVDetectEnable
                // A switch to use LV to detect HDR scene
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable HDR LV analysis
                //
                // Varaible name: i4LVProbTbl
                // LV table for detecting HDR scene
                // Unit: %
                // Default value: { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
                // Data range: Integer, 0 ~ 100
                // Constraints: N/A.
                // Effect: Set the probability of HDR scene according to LV information
                /**************************************************/
                TRUE,           // bLVDetectEnable
               
            //LV  0  1   2   3   4   5   6    7    8    9   10   11   12   13   14   15   16   17   18    i4LVProbTbl
                { 0, 0, 25, 25, 25, 50, 75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
            }
        },
        
        /**************************************************/
        // Feature name: HDR auto-enhance
        // Calculating information for HDR ratio
        // rHdrAutoEnhance
        /**************************************************/
        {                      
            /**************************************************/
            // Variable name: bAutoEnhanceEnable
            // A switch to calculate HDR ratio information
            // Default value: TRUE.
            // Data range: TRUE or FALSE.
            // Constraints: N/A.
            // Effect: Set TRUE to enable HDR BV analysis
            //
            // Varaible name: i4RMGk
            // Control the height of turning point to LE/SE curve
            // Default value: 4095
            // Data range: Integer, 0 ~ 4095
            // Constraints: N/A.
            // Effect: Set i4RMGk higher(lower) to make LE longer(shorter)
            //
            // Varaible name: i4RMGtRatio
            // Set the percentage of current Y for calculating RMGt
            // Unit: 10*%
            // Default value: 40
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The higher i4RMGtRatio, more percentage of histogram will be accumulated in current Y
            /**************************************************/
            TRUE,               // bAutoEnhanceEnable
            4095,               // i4RMGk
            20,                 // i4RMGtRatio

            /**************************************************/
            // Feature name: RMGt
            // Interpolation table for RMGt
            // Influence: the turning point between LE/SE curve
            // If current Y is large(small), use data of SE(LE) more
            // rRMGt
            //
            // Varaible name: rRMGt.i4X1
            // Max current Y threshold to get max RMGt i4Y1
            // Unit: Y value in 8 bits
            // Default value: 160
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X1, larger current Y zone will get max RMGt setting (i4Y1)
            //         The current Y lower than i4Y1 will get longer LE curve.
            //
            // Varaible name: rRMGt.i4Y1
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 512
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y1, the longer LE curve we will get
            //
            // Varaible name: rRMGt.i4X2
            // Min current Y to get min RMGt i4Y2
            // Unit: Y value in 8 bits
            // Default value: 220
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller current Y zone will get min RMGt setting (i4Y2)
            //         The current Y higher than i4Y1 will get shorter LE curve.
            //
            // Varaible name: rRMGt.i4Y2
            // Max RMGt setting, controls the horizontal ratio of the result from RMGk.
            // Unit: Y value in 10 bits
            // Default value: 384
            // Data range: Integer, 0 ~ 1023
            // Constraints: N/A.
            // Effect: The larger i4Y2, the longer LE curve we will gets
            /**************************************************/
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
                /**************************************************/
                // Varaible name: i4LVRatio50
                // Ratio interpolation table by scene LV, for 50 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                //
                // Varaible name: i4LVRatio60
                // Ratio interpolation table by scene LV, for 60 Hz
                // Unit: 100*LE/SE
                // Default value: None
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: Set different HDR ratio for different scene LV
                /**************************************************/
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio50
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},
               //LV  0    1    2    3    4    5    6    7    8    9    10   11   12    13    14    15   16   17   18    i4LVRatio60
                {   200, 200, 200, 200, 200, 200, 300, 400, 400, 400, 400, 400, 400,  400,  400,  400, 400, 400, 400},

                /**************************************************/
                // Feature name: rContrastRatio
                // HDR ratio interpolation table by HDR contrast EV difference
                // Influence: auto contrast ratio, HDR ratio
                // rContrastRatio
                //
                // Varaible name: rContrastRatio.i4X1
                // Max contrast EVD threshold to get min HDR ratio i4Y1
                // Unit: EV*1000
                // Default value: 3500
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger contrast EVD zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rContrastRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 100
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rContrastRatio.i4X2
                // Min contrast EVD to get max HDR ratio i4Y2
                // Unit: EV*1000
                // Default value: 4500
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller contrast EVD zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rContrastRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rContrastRatio
                    3000,       // i4X1
                    200,        // i4Y1
                    4000,       // i4X2
                    400         // i4Y2
                },

                /**************************************************/
                // Feature name: rBrightRatio
                // HDR ratio interpolation table by HDR bright side histogram information
                // Influence: auto bright probability, HDR ratio
                //rBrightRatio
                //
                // Varaible name: rBrightRatio.i4X1
                // Max bright side histogram threshold to get HDR ratio i4Y1
                // Unit: weighting average pixel count
                // Default value: 600
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger bright side histogram zone will get min HDR ratio (i4Y1)
                //
                // Varaible name: rBrightRatio.i4Y1
                // Min HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rBrightRatio.i4X2
                // Min bright side histogram to get max HDR ratio i4Y2
                // Unit: weighting average pixel count
                // Default value: 1100
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller bright side histogram zone will get max HDR ratio (i4Y2)
                //
                // Varaible name: rBrightRatio.i4Y2
                // Max HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: 200 or 400
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rBrightProbRatio
                    600,        // i4X1
                    200,        // i4Y1
                   1100,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: rNightProbRatio
                // HDR ratio interpolation table by night scene probability
                // Influence: auto night probability, HDR ratio
                // rNightProbRatio
                // Varaible name: rNightProbRatio.i4X1
                // Max night scene probability threthold to get max HDR ratio i4Y1
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rNightProbRatio.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rNightProbRatio.i4X2
                // Min night scene probability to get max HDR ratio i4Y2
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rNightProbRatio.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rNightProbRatio
                    0,          // i4X1
                    400,        // i4Y1
                    512,        // i4X2
                    200         // i4Y2
                },

                /**************************************************/
                // Feature name: motion information
                // The motion corresponding information including gyro, accelerator,
                // Influence: HDR ratio, HDR on/off, on/off smoothness
                // rGGyroSensor
                /**************************************************/
                {            
                    /**************************************************/
                    // Feature name: rAcceProb
                    // Acceleration probability interpolation table by accelerator information
                    // Influence: HDR ratio, motion on/off
                    // rAcceProb
                    //
                    // Varaible name: rAcceProb.i4X1
                    // Max acceleration threthold to get min acceleration probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger acceleration zone will get min acceleration probability (i4Y1)
                    //
                    // Varaible name: rAcceProb.i4Y1
                    // Min acceleration probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff acceleration to get min acceleration probability
                    //
                    // Varaible name: rAcceProb.i4X2
                    // Min acceleration to get max acceleration probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller acceleration zone will get max acceleration probability (i4Y2)
                    //
                    // Varaible name: rAcceProb.i4Y2
                    // Max acceleration probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the acceleration > i4X2 will get 100% acceleration probability
                    /**************************************************/
                    {               // rAcceProb
                        1500,       // i4X1
                        0,          // i4Y1
                        2500,       // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rGyroProb
                    // Motion probability interpolation table by gyro sensor information
                    // Influence: HDR ratio, motion on/off
                    // rGyroProb
                    //
                    // Varaible name: rGyroProb.i4X1
                    // Max motion threthold to get min motion probability i4Y1
                    // Default value: none
                    // Data range: Integer, i4X1 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion zone will get min motion probability (i4Y1)
                    //
                    // Varaible name: rGyroProb.i4Y1
                    // Min motion probability according to i4X1
                    // Unit: 1024*%/100
                    // Default value: 0
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y1 = 0, i4X1 is the cutoff motion to get min motion probability
                    //
                    // Varaible name: rGyroProb.i4X2
                    // Min motion to get max motion probability i4Y2
                    // Default value: none
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion zone will get max motion probability (i4Y2)
                    //
                    // Varaible name: rGyroProb.i4Y2
                    // Max motion probability according to i4X2
                    // Unit: 1024*%/100
                    // Default value: 1024
                    // Data range: Integer, 0 ~ 1024
                    // Constraints: N/A.
                    // Effect: If i4Y2 = 1024, the motion > i4X2 will get 100% motion probability
                    /**************************************************/
                    {               // rGyroProb
                        100,        // i4X1
                        0,          // i4Y1
                        700,        // i4X2
                        1024        // i4Y2
                    },

                    /**************************************************/
                    // Feature name: rMotionRatio
                    // HDR ratio interpolation table by motion (gyro + acce) probability information
                    // Influence: HDR ratio
                    // rMotionRatio
                    //
                    // Varaible name: rMotionRatio.i4X1
                    // Max motion probability threthold to get min HDR ratio i4Y1
                    // Unit: 2048*%/100
                    // Default value: 512
                    // Data range: Integer, 0 ~ 2048
                    // Constraints: N/A.
                    // Effect: The larger i4X1, larger motion probability zone will get max HDR ratio (i4Y1)
                    //
                    // Varaible name: rMotionRatio.i4Y1
                    // Max HDR ratio according to i4X1
                    // Unit: 100*LE/SE
                    // Default value: 400
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    //
                    // Varaible name: rMotionRatio.i4X2
                    // Min motion probability to get max HDR ratio i4Y2
                    // Unit: 2048*%/100
                    // Default value: 960
                    // Data range: Integer, i4X2 > 0
                    // Constraints: N/A.
                    // Effect: The larger i4X2, smaller motion probability zone will get min HDR ratio (i4Y2)
                    //
                    // Varaible name: rMotionRatio.i4Y2
                    // Min HDR ratio according to i4X2
                    // Unit: 100*LE/SE
                    // Default value: 100
                    // Data range: Integer, 100 ~ 800
                    // Constraints: N/A.
                    // Effect: The larger i4Y1, the larger HDR ratio we will get
                    /**************************************************/
                    {               // rMotionRatio
                        512,        // i4X1
                        400,        // i4Y1
                        960,        // i4X2
                        100         // i4Y2
                    },

                    /**************************************************/
                    // Varaible name: i4MotionOnThr
                    // Threshold to determine motion on/off
                    // Unit: 1024*%/100
                    // Default value: 512 or 768
                    // Data range: Integer, i4MotionOnThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnThr, you need higher motion probability to set motion detector on
                    //
                    // Varaible name: i4MotionOnCntThr
                    // Counting threshold to update motion on/off information
                    // Unit: frame count
                    // Default value: 2 or 5
                    // Data range: Integer, i4MotionOnCntThr > 0
                    // Constraints: N/A.
                    // Effect: The larger i4MotionOnCntThr, you need count more to update motion on/off
                    /**************************************************/
                    768,            // i4MotionOnThr
                    5               // i4MotionOnCntThr
                }
            },

            /**************************************************/
            // Feature name: rAutoFlicker
            // Parameter setting to automatically avoid flicker
            // rAutoFlicker
            /**************************************************/
            {              
                /**************************************************/
                // Varaible name: i4Flkr1LVThr
                // LV threshold for deflicker enable
                // Unit: LV*10
                // Default value: {130,130}
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The higher i4Flkr1LVThr, the higher scene LV needed to enable deflicker
                //
                // Varaible name: i4MinRatioAntiFlk
                // The min acceptable ratio for anti-flicker
                // Unit: 100*LE/SE
                // Default value: 200 or 800
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The ratio lower i4MinRatioAntiFlk than could get flickered results
                /**************************************************/
                {130,130},        // i4Flkr1LVThr[0]: 50Hz   i4Flkr1LVThr[1]: 60Hz
                200,            // i4MinRatioAntiFlk: The min. acceptable ratio for anti-flicker. 200: 2x ratio

                //Not used
                /**************************************************/
                {70007, 66670}, // i4MaxExp[0]: 50Hz  i4MaxExp[1]: 60Hz
                {400, 400},     // i4MinExp[0]: 50Hz  i4MinExp[1]: 60Hz
                /**************************************************/

                /**************************************************/
                // Feature name: rIsoThr
                // HDR ratio interpolation table by ISO information
                // Influence: HDR ratio
                // rIsoThr
                //
                // Varaible name: rIsoThr.i4X1
                // Max ISO threshold to get max HDR ratio i4Y1
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger ISO zone will get max HDR ratio (i4Y1)
                //
                // Varaible name: rIsoThr.i4Y1
                // Max HDR ratio according to i4X1
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y1, the larger HDR ratio we will get
                //
                // Varaible name: rIsoThr.i4X2
                // Min ISO to get min HDR ratio i4Y2
                // Unit: 1024*gain
                // Default value: none
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller ISO zone will get min HDR ratio (i4Y2)
                //
                // Varaible name: rIsoThr.i4Y2
                // Min HDR ratio according to i4X2
                // Unit: LE/SE
                // Default value: none
                // Data range: Integer, 100 ~ 800
                // Constraints: N/A.
                // Effect: The larger i4Y2, the larger HDR ratio we will get
                /**************************************************/
                {               // rIsoThr
                    14336,      //i4X1:1024 * 12
                    1,          //i4Y1
                    12288,      //i4X2:1024 * 10
                    4           //i4Y2
                },
            },

            /**************************************************/
            // Feature name: rAutoSmooth
            // HDR auto smooth (HDR on/off, motion on/off, HDR ratio smooth) information
            // rAutoSmooth
            /**************************************************/
            {                
                /**************************************************/
                // Variable name: bHdrWaitAEStable
                // A switch to determine wait for AE stable or not
                // Default value: TRUE.
                // Data range: TRUE or FALSE.
                // Constraints: N/A.
                // Effect: Set TRUE to enable wait
                //
                // Variable name: i4HdrInfoCounterThr
                // Threshold to update HDR on/off information
                // Unit: frame count
                // Default value: 6
                // Data range: Integer, i4HdrInfoCounterThr > 0
                // Constraints: N/A.
                // Effect: HDR on/off counter larger than i4HdrInfoCounterThr can update the on/off result
                /**************************************************/
                TRUE,           // bHdrWaitAEStable
                6,              // i4HdrInfoCounterThr

                //Not used
                /**************************************************/
                FALSE,          // bRatioSmoothEnable TRUE for platform / FALSE for simulation
                /**************************************************/

                /**************************************************/
                // Variable name: i4RatioSmoothThr
                // Threshold to update HDR ratio smooth information
                // Unit: frame count
                // Default value: none
                // Data range: Integer, i4RatioSmoothThr > 0
                // Constraints: N/A.
                // Effect: HDR ratio smooth counter larger than i4RatioSmoothThr can update the smoothed HDR ratio result
                //
                // Variable name: i4RoughRatioUnit
                // Threshold to check if difference of current ratio and current stable ratio is large enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4RoughRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current ratio and current stable ratio larger than i4RoughRatioUnit check for close results in incoming frames
                //
                // Variable name: i4FinerRatioUnit
                // Threshold to check if difference of current/previous ratio is small enough
                // Unit: 100*LE/SE
                // Default value: none
                // Data range: Integer, i4FinerRatioUnit > 0
                // Constraints: N/A.
                // Effect: Difference of current/previous HDR ratio > i4FinerRatioUnit start to count
                /**************************************************/
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
            }
            /**************************************************/
        },

        /**************************************************/
        // Feature name: rHdrAETarget
        // HDR AE target parameter setting
        // rHdrAETarget
        /**************************************************/
        {   
            /**************************************************/
            // Variable name: i4DefTarget
            // Default AE target
            // Unit: Y value in 8 bits
            // Default value: 47
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: A reference of AE target, set higher to make AE target higher
            //
            // Variable name: i4DefWeight
            // Weight of default target, mixing with normal AE target
            // Default value: 47
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: Set i4DefWeight higher to make the proportion of default AE target higher
            /**************************************************/
            47,                 // i4DefTarget
            256,                // i4DefWeight

            /**************************************************/
            // Feature name: rAutoEVWeight
            // HDR auto EV weight interpolation table
            // Influence: auto EV weight, HDR auto weight
            // rAutoEVWeight
            //
            // Varaible name: rAutoEVWeight.i4X1
            // Max contrast EVD threshold to get min auto EV weight (i4Y1)
            // Unit: EV*1000
            // Default value: 4000 or 3750
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger contrast EVD zone will get min auto EV weight (i4Y1)
            //
            // Varaible name: rAutoEVWeight.i4Y1
            // Min auto EV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoEVWeight.i4X2
            // Min contrast EVD threshold to get max auto EV weight i4Y2
            // Unit: EV*1000
            // Default value: 4500 or 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller contrast EVD zone will get max auto EV weight (i4Y2)
            //
            // Varaible name: rAutoEVWeight.i4Y2
            // Max auto EV weight according to i4X2
            // Unit: 1024*%/100
            // Default value:  512 or 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The contrast EVD larger than i4X2 will get auto EV weight of i4Y2
            /**************************************************/
            {                   // rAutoEVWeight
                4000,           // rAutoEVWeight.i4X1
                256,            // rAutoEVWeight.i4Y1
                5000,           // rAutoEVWeight.i4X2
                512             // rAutoEVWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoBrightWeight
            // HDR auto bright weight interpolation table
            // Influence: auto bright weight, auto night weight, HDR auto weight
            // rAutoBrightWeight
            //
            // Varaible name: rAutoBrightWeight.i4X1
            // Max bright enhance probability threshold to get min auto bright weight (i4Y1)
            // Unit: weighting average pixel count
            // Default value: 4000
            // Data range: Integer, i4X1 > 0
            // Constraints: N/A.
            // Effect: The larger i4X1, larger bright enhance probability zone will get min auto bright weight (i4Y1)
            //
            // Varaible name: rAutoBrightWeight.i4Y1
            // Min auto bright weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoBrightWeight.i4X2
            // Min bright enhance probability threshold to get max auto bright weight i4Y2
            // Unit: weighting average pixel count
            // Default value: 5000
            // Data range: Integer, i4X2 > 0
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller bright enhance probability zone will get max auto bright weight (i4Y2)
            //
            // Varaible name: rAutoBrightWeight.i4Y2
            // Max auto bright weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The bright enhance probability larger than i4X2 will get max auto bright weight of i4Y2
            /**************************************************/
            {                   // rAutoBrightWeight
                4000,           // rAutoBrightWeight.i4X1
                256,            // rAutoBrightWeight.i4Y1
                5000,           // rAutoBrightWeight.i4X2
                768             // rAutoBrightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoABLWeight
            // HDR auto ABL weight interpolation table
            // Influence: auto ABL weight, auto night weight, HDR auto weight
            // rAutoABLWeight
            //
            // Varaible name: rAutoABLWeight.i4X1
            // Max ABL probability threshold to get min auto ABL weight (i4Y1)
            // Unit: 1024*%/100
            // Default value: 0
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X1, larger ABL probability zone will get min auto ABL weight (i4Y1)
            //
            // Varaible name: rAutoABLWeight.i4Y1
            // Min auto ABL weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoABLWeight.i4X2
            // Min ABL probability threshold to get max auto ABL weight i4Y2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller ABL probability zone will get max auto ABL weight (i4Y2)
            //
            // Varaible name: rAutoABLWeight.i4Y2
            // Max auto ABL weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The ABL probability larger than i4X2 will get max auto ABL weight of i4Y2
            /**************************************************/
            {                   // rAutoABLWeight
                0,              // rAutoABLWeight.i4X1
                1024,           // rAutoABLWeight.i4Y1
                512,            // rAutoABLWeight.i4X2
                1024            // rAutoABLWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightL
            // HDR auto LV weight interpolation table
            // It is for low LV specially, LV higher than 40 will get weight 1024
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightL
            //
            // Varaible name: rAutoLVWeightL.i4X1
            // Max LV threshold to get min auto LV weight (i4Y1)
            // Unit: LV*10
            // Default value: 30
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight (i4Y1)
            //
            // Varaible name: rAutoLVWeightL.i4Y1
            // Min auto LV weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 768
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight
            //
            // Varaible name: rAutoLVWeightL.i4X2
            // Min LV threshold to get max auto LV weight i4Y2
            // Unit: LV*10
            // Default value: 40
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight (i4Y2)
            //
            // Varaible name: rAutoLVWeightL.i4Y2
            // Max auto LV weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 1024
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight of i4Y2              
            /**************************************************/
            {                   // rAutoLVWeightL
                 30,            // rAutoLVWeightL.i4X1
                768,            // rAutoLVWeightL.i4Y1
                 40,            // rAutoLVWeightL.i4X2
               1024             // rAutoLVWeightL.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoLVWeightH
            // HDR auto LV weight lower bound, determined by high LV
            // If scene LV is high, it raise HDR AE weight
            // Influence: auto LV weight, auto night weight, HDR auto weight
            // rAutoLVWeightH
            //
            // Varaible name: rAutoLVWeightH.i4X1
            // Max LV threshold to get min auto LV weight lower bound (i4Y1)
            // Unit: LV*10
            // Default value: 100
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger LV zone will get min auto LV weight lower bound (i4Y1)
            //
            // Varaible name: rAutoLVWeightH.i4Y1
            // Min auto LV weight lower bound according to i4X1
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: We will at least get i4Y1 weight for auto LV lower bound
            //
            // Varaible name: rAutoLVWeightH.i4X2
            // Min LV threshold to get max auto LV weight lower bound i4Y2
            // Unit: LV*10
            // Default value: 130
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller LV zone will get max auto LV weight lower bound (i4Y2)
            //
            // Varaible name: rAutoLVWeightH.i4Y2
            // Max auto LV weight lower bound according to i4X2
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The LV larger than i4X2 will get max auto LV weight lower bound (i4Y2)
            /**************************************************/
            {                   // rAutoLVWeightH
                100,            // rAutoLVWeightH.i4X1
                256,            // rAutoLVWeightH.i4Y1
                130,            // rAutoLVWeightH.i4X2
                512             // rAutoLVWeightH.i4Y2
            },

            /**************************************************/
            // Feature name: rAutoNightWeight
            // HDR auto weight interpolation table, determined by night scene probability
            // Influence: HDR auto weight
            // rAutoNightWeight
            //
            // Varaible name: rAutoNightWeight.i4X1
            // Max night scene probability threshold to get max HDR auto weight (i4Y1)
            // Unit: LV*10
            // Default value: 1024
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X1, larger night scene probability zone will get max HDR auto weight (i4Y1)
            //
            // Varaible name: rAutoNightWeight.i4Y1
            // Max HDR auto weight according to i4X1
            // Unit: 1024*%/100
            // Default value: 512
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y1, the higher HDR auto weight we will get
            //
            // Varaible name: rAutoNightWeight.i4X2
            // Min night scene probability threshold to get min HDR auto weight i4Y2
            // Unit: LV*10
            // Default value: 2048
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: The larger i4X2, smaller night scene probability zone will get min HDR auto weight (i4Y2)
            //
            // Varaible name: rAutoNightWeight.i4Y2
            // Min HDR auto weight according to i4X2
            // Unit: 1024*%/100
            // Default value: 256
            // Data range: Integer, 0 ~ 1024
            // Constraints: N/A.
            // Effect: The higher i4Y2, the higher HDR auto weight we will get                
            /**************************************************/
            {                   // rAutoNightWeight
                1024,           // rAutoNightWeight.i4X1
                512,            // rAutoNightWeight.i4Y1
                2048,           // rAutoNightWeight.i4X2
                256             // rAutoNightWeight.i4Y2
            },

            /**************************************************/
            // Feature name: rHdrAoeTarget
            // AOE information for HDR AE calculation
            // HDR scene share the same property as over exposure
            //
            // Varaible name: i4AoePercent
            // The percent for AOE to calculate current AOE Y from bright
            // Unit: 10*%
            // Default value: 25
            // Data range: Integer, 0 ~ 1000
            // Constraints: N/A.
            // Effect: The larger i4AoePercent, more percent of histogram from the bright side will be accumulated for AOE current Y
            //
            // Varaible name: i4AoeLV
            // The LV thresholds to deliver different AOE target
            // Unit: LV*10
            // Default value: {50, 100, 150}
            // Data range: Integer, 0 ~ 180
            // Constraints: N/A.
            // Effect: Differentiate the AOE target by LV
            //
            // Varaible name: i4AoeTarget
            // Different AOE targets differentiated by LV
            // Unit: Y value in 8 bits
            // Default value: {200, 200, 200}
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: Different LV delivers different AOE target
            /**************************************************/
            {
                25, //20,       // i4AoePercent
                {50, 100, 150}, // i4AoeLV[3]
                {220, 220, 220} // i4AoeTarget[3]
            },

            /**************************************************/
            // Feature name: rHdrNightTarget
            // Night scene information for HDR AE calculation
            // HDR scene has almost the same property as night scene,
            // but if we use higher HDR ratio in night scene, 
            // the noise will be large because of the high ISO in night scene.
            // So, night scene should use lower HDR ratio
            // rHdrNightTarget
            /**************************************************/
            {                 
                /**************************************************/
                // Varaible name: i4NightPercent
                // The percent for night scene to calculate current night scene Y from bright
                // Default value: 150
                // Data range: Integer, 0 ~ 1000
                // Constraints: N/A.
                // Effect: The larger i4NightPercent, more percent of histogram from the bright side will be accumulated for night scene current Y
                /**************************************************/
                150,             // i4NightPercent

                /**************************************************/
                // Feature name: rNightTarget
                // Night scene target interpolation table, determined by night scene probability
                // Influence: HDR night scene target Y, HDR night scene target gain, HDR target gain, HDR target Y
                // rNightTarget
                //
                // Varaible name: rNightTarget.i4X1
                // Max night scene probability threshold to get max night scene target (i4Y1)
                // Unit: 1024*%/100
                // Default value: 0
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger night scene probability zone will get max night scene target (i4Y1)
                //
                // Varaible name: rNightTarget.i4Y1
                // Max night scene target according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 80
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher night scene target we will get
                //
                // Varaible name: rNightTarget.i4X2
                // Min night scene probability threshold to get min night scene target i4Y2
                // Unit: 1024*%/100
                // Default value: 1024
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller night scene probability zone will get min night scene target (i4Y2)
                //
                // Varaible name: rNightTarget.i4Y2
                // Min night scene target according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 60
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher night scene target we will get                
                /**************************************************/
                {               // rNightTarget
                    0,          // i4X1
                    80,         // i4Y1
                    1024,       // i4X2
                    60          // i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitL
            // HDR AE target clamping for low LV
            // The LV lower than the low LV threshold use this table,
            // between low LV threshold and medium LV threshold use the interpolation result of medium/low LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitL
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitL.i4LVLimit
                // Low LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 60
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV value lower than rTargetLimitL.i4LVLimit should use rTargetMinLimit interpolation table
                /**************************************************/
                60,             // rTargetLimitL.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMinLimit
                // For low LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for low LV scene
                // rTargetLimitL.rTargetMinLimit
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR target threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 10
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMinLimit
                    4000,       // rTargetLimitL.rTargetMinLimit.i4X1
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMinLimit.i4X2
                    10,         // rTargetLimitL.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitL.rTargetMaxLimit
                // For low LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for low LV scene
                // rTargetLimitL.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X1
                // For low LV, HDR contrast EVD interpolation lower bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target of i4X1
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4X2
                // For low LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold 
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitL.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get     
                /**************************************************/
                {               // rTargetLimitL.rTargetMaxLimit
                    4000,       // rTargetLimitL.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitL.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitL.rTargetMaxLimit.i4X2
                    50          // rTargetLimitL.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitM
            // HDR AE target clamping for medium LV
            // The LV between low LV threshold and medium LV threshold use the interpolation result of low/medium LV
            // The LV between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for medium LV scene
            // rTargetLimitM
            /**************************************************/
            {   
                /**************************************************/
                // Varaible name: rTargetLimitM.i4LVLimit
                // Medium LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The rTargetLimitL.i4LVLimit < LV < rTargetLimitM.i4LVLimit should use rTargetLimitM interpolation table      
                /**************************************************/
                80,             // rTargetLimitM.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMinLimit
                // For medium LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for medium LV scene
                // rTargetLimitM.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: none
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get
                /**************************************************/
                {               // rTargetLimitM.rTargetMinLimit
                    4000,       // rTargetLimitM.rTargetMinLimit.i4X1
                    15,         // rTargetLimitM.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMinLimit.i4X2
                    20,         // rTargetLimitM.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitM.rTargetMaxLimit
                // For medium LV, max HDR AE target threshold interpolation table
                // Influence: HDR AE target Y max clamping range for medium LV scene
                // rTargetLimitM.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X1
                // For medium LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4X2
                // For medium LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitM.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get            
                /**************************************************/
                {               // rTargetLimitM.rTargetMaxLimit
                    4000,       // rTargetLimitM.rTargetMaxLimit.i4X1
                    50,         // rTargetLimitM.rTargetMaxLimit.i4Y1
                    6000,       // rTargetLimitM.rTargetMaxLimit.i4X2
                    50          // rTargetLimitM.rTargetMaxLimit.i4Y2
                }
            },

            /**************************************************/
            // Feature name: rTargetLimitH
            // HDR AE target clamping for high LV
            // The LV higher than the high LV threshold use this table,
            // between medium LV threshold and high LV threshold use the interpolation result of medium/high LV
            // Influence: HDR AE target Y min/max clamping range for low LV scene
            // rTargetLimitH
            /**************************************************/
            {              
                /**************************************************/
                // Varaible name: rTargetLimitH.i4LVLimit
                // High LV threshold for determining HDR AE target max/min clamping interpolation table
                // Unit: LV*10
                // Default value: 80
                // Data range: Integer, 0 ~ 180
                // Constraints: N/A.
                // Effect: The LV larger than rTargetLimitH.i4LVLimit should use rTargetLimitH interpolation table               
                /**************************************************/
                120,            // rTargetLimitH.i4LVLimit

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMinLimit
                // For high LV, HDR AE target min threshold interpolation table
                // Influence: HDR AE target Y min clamping range for high LV scene
                // rTargetLimitH.rTargetMinLimit 
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target min threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y1
                // HDR target min threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 25
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target min threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target min threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target min threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMinLimit.i4Y2
                // HDR target min threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 40
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target min threshold we will get            
                /**************************************************/
                {               // rTargetLimitH.rTargetMinLimit
                    4000,       // rTargetLimitH.rTargetMinLimit.i4X1
                    25,         // rTargetLimitH.rTargetMinLimit.i4Y1
                    6000,       // rTargetLimitH.rTargetMinLimit.i4X2
                    40          // rTargetLimitH.rTargetMinLimit.i4Y2
                },

                /**************************************************/
                // Feature name: rTargetLimitH.rTargetMaxLimit
                // For high LV, HDR AE target max threshold interpolation table
                // Influence: HDR AE target Y max clamping range for high LV scene
                // rTargetLimitH.rTargetMaxLimit
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X1
                // For high LV, HDR contrast EVD interpolation lower bound for max HDR AE target threshold
                // Unit: EV*1000
                // Default value: 4000
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X1, larger HDR contrast EVD zone will get HDR AE target max threshold of i4X1
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y1
                // HDR target max threshold according to i4X1
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher HDR target max threshold we will get
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4X2
                // For high LV, HDR contrast EVD interpolation upper bound for HDR AE target max threshold
                // Unit: EV*1000
                // Default value: 6000
                // Data range: Integer, i4X2 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller HDR contrast EVD zone will get higher HDR AE target max threshold of i4Y2
                //
                // Varaible name: rTargetLimitH.rTargetMaxLimit.i4Y2
                // HDR target max threshold according to i4X2
                // Unit: Y value in 8 bits
                // Default value: 50
                // Data range: Integer, 0 ~ 255
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher HDR target max threshold we will get                
                /**************************************************/
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
                /**************************************************/
                // Feature name: rFaceHdrProb
                // Face HDR probability interpolation table
                // rFaceHdrProb
                //
                // Varaible name: rFaceHdrProb.i4X1
                // Max face bright tone probability to get max night scene target (i4Y1)
                // Unit: weighting average pixel count
                // Default value: 2200
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The larger i4X1, larger face bright tone probability zone will get max face HDR target (i4Y1)
                //
                // Varaible name: rFaceHdrProb.i4Y1
                // Max face HDR target according to i4X1
                // Unit: 1024*%/100
                // Default value: 512
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y1, the higher face HDR target we will get
                //
                // Varaible name: rFaceHdrProb.i4X2
                // Min face bright tone probability threshold to get min face HDR target i4Y2
                // Unit: weighting average pixel count
                // Default value: 3200
                // Data range: Integer, i4X1 > 0
                // Constraints: N/A.
                // Effect: The larger i4X2, smaller face bright tone probability zone will get min face HDR target (i4Y2)
                //
                // Varaible name: rFaceHdrProb.i4Y2
                // Min face HDR target according to i4X2
                // Unit: 1024*%/100
                // Default value: 256
                // Data range: Integer, 0 ~ 1024
                // Constraints: N/A.
                // Effect: The higher i4Y2, the higher face HDR target we will get                
                /**************************************************/
                {               // rFaceHdrProb
                    600,        // i4X1
                    400,        // i4Y1
                   1100,        // i4X2
                    400         // i4Y2
                }
            },
            {                   // rTouchAEProp
                // TBD
            }
        },

        /**************************************************/
        // Feature name: rHdrcHdr
        // Calculate cHDR detection information
        // rHdrcHdr      
        /**************************************************/
        {   
            /**************************************************/
            // Varaible name: i4LEThr
            // Current Y accumulated histogram from dark for LE information
            // Unit: Y value in 8 bits
            // Default value: 40
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4LEThr, more bins of histogram will be accumulated from dark side
            //
            // Varaible name: i4SERatio
            // Current Y accumulated histogram percentage from bright for SE information
            // Unit:10*%
            // Default value: 20
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SERatio, more % of histogram will be accumulated from bright side                   
            //  
            // Varaible name: i4SETarget
            // SE default target Y
            // Unit: Y value in 8 bits
            // Default value: 180
            // Data range: Integer, 0 ~ 255
            // Constraints: N/A.
            // Effect: The higher i4SETarget, the higher SE target Y
            // 
            // Varaible name: i4BaseGain
            // The base gain for AE target
            // Unit: 1024*gain
            // Default value: 1024
            // Data range: Integer, i4BaseGain > 0
            // Constraints: N/A.
            // Effect: The higher i4BaseGain, the higher base for gain
            /**************************************************/
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
isAEEnabled<ESensorDev_Sub>()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AE_CYCLE_NUM (3)

template <>
const MINT32*
getAEActiveCycle<ESensorDev_Sub>()
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
getAECycleNum<ESensorDev_Sub>()
{
    return AE_CYCLE_NUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Param Mapping Table by Scenario
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_PREVIEW>() {
    return getAEParam<ESensorDev_Sub>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_VIDEO>() {
    return getAEParam<ESensorDev_Sub>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CAPTURE>() {
    return getAEParam<ESensorDev_Sub>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM1>() {
    return getHDRAEParam<ESensorDev_Sub>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM2>() {
    return getAUTOHDRAEParam<ESensorDev_Sub>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM3>() {
    return getVTAEParam<ESensorDev_Sub>();
}
template <> AE_PARAM_T const& getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM4>() {
    return getAEParam<ESensorDev_Sub>();
}

