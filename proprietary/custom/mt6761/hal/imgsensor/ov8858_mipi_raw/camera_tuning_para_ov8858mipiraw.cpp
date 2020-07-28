/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 *      *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <cutils/log.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8858mipiraw.h"
#include "camera_info_ov8858mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"

#define NVRAM_TUNING_PARAM_NUM  5341001
extern "C" fptrDefault getDefaultDataFunc(MUINT32 sensorType)
{
    if(sensorType==0)
        return NSFeature::RAWSensorInfo<SENSOR_ID>::getDefaultData;
    else
        return NSFeature::YUVSensorInfo<SENSOR_ID>::getDefaultData;
}

const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    .Version= NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    .SensorId= SENSOR_ID,
    .ISPComm={
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      }
    },
    .ISPPca= {
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    .ISPRegs={
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    .ISPMfbMixer={{
            0x01FF0001, // MIX3_CTRL_0
            0x00FF0000, // MIX3_CTRL_1
            0xFFFF0000  // MIX3_SPARE
    }},
    .ISPMulitCCM={
            .Poly22={
            82075,      // i4R_AVG
            17131,      // i4R_STD
            94925,      // i4B_AVG
            18952,      // i4B_STD
            0,      // i4R_MAX
            0,      // i4R_MIN
            0,      // i4G_MAX
            0,      // i4G_MIN
            0,      // i4B_MAX
            0,      // i4B_MIN
        { // i4P00[9]
                10620000, -4900000, -595000, -2022500, 10182500, -3027500, -952500, -3972500, 10047500
        },
        { // i4P10[9]
                1724927, -2339168, 601852, -209403, -458911, 641635, 717575, -1358872, 613329
        },
        { // i4P01[9]
                1226296, -2294734, 1055336, -1149497, -61494, 1183324, 26338, -2280502, 2220302
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
            .AWBGain={
                // Strobe
               {
                    512,  // i4R
                    512,  // i4G
                    512 // i4B
               },
                // A
                {
                612,    // i4R
                    512,  // i4G
                1133    // i4B
               },
                // TL84
               {
                794,    // i4R
                    512,  // i4G
                968    // i4B
               },
                // CWF
                {
                849,    // i4R
                    512,  // i4G
                1011    // i4B
               },
                // D65
               {
                1028,    // i4R
                    512,  // i4G
                685    // i4B
               },
                // Reserved 1
               {
                    512,  // i4R
                    512,  // i4G
                    512 // i4B
               },
                // Reserved 2
               {
                    512,  // i4R
                    512,  // i4G
                    512 // i4B
               },
                // Reserved 3
               {
                    512,  // i4R
                    512,  // i4G
                    512 // i4B
               }
            },
            .Weight={
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
          MDPMulitCCM:{
                .Poly22={
                  125225, // i4R_AVG
                   32141, // i4R_STD
                  165675, // i4B_AVG
                   30249, // i4B_STD
                     646, // i4R_MAX
                     527, // i4R_MIN
                     789, // i4G_MAX
                     670, // i4G_MIN
                     856, // i4B_MAX
                     721, // i4B_MIN
                  {  // i4P00[9]
                     5995000,   -505000,   -370000,  -1387500,   7165000,   -657500,   -532500,  -2045000,   7697500
                  },
                  {  // i4P10[9]
                      725905,  -1081879,    355974,    627096,   -389018,   -238079,    626246,    -86040,   -540207
                  },
                  {  // i4P01[9]
                      257857,   -576218,    318361,    -96902,   -138571,    235473,     24959,    193148,   -218107
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
                .AWBGain={
                  // Strobe
                  {
                    1016,    // i4R
                    512,    // i4G
                    1101    // i4B
                  },
                  // A
                  {
                    878,    // i4R
                      512,    // i4G
                    1971    // i4B
                  },
                  // TL84
                  {
                    1192,    // i4R
                      512,    // i4G
                    1628    // i4B
                  },
                  // CWF
                  {
                    1280,    // i4R
                      512,    // i4G
                    1773    // i4B
                  },
                  // D65
                  {
                    1659,    // i4R
                      512,    // i4G
                    1255    // i4B
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
                .Weight={
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
    .isp_ccm_ratio= 1.0,
    //bInvokeSmoothCCM
    .bInvokeSmoothCCM= MTRUE,
    .DngMetadata=
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
    },
    .ANR_TBL=
    {0}
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
            10512,    // u4MaxGain, 16x
            100,    // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            25915,    // u4PreExpUnit
            30,    // u4PreMaxFrameRate
            13039,    // u4VideoExpUnit
            30,    // u4VideoMaxFrameRate
            1024,    // u4Video2PreRatio, 1024 base = 1x
            13039,    // u4CapExpUnit
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            15256,     // u4Video1ExpUnit
            120,     // u4Video1MaxFrameRate
            1024,   // u4Video12PreRatio, 1024 base = 1x
            25915,     // u4Video2ExpUnit
            30,     // u4Video2MaxFrameRate
            1024,   // u4Video22PreRatio, 1024 base = 1x
            19770,     // u4Custom1ExpUnit
            30,     // u4Custom1MaxFrameRate
            1024,   // u4Custom12PreRatio, 1024 base = 1x
            19770,     // u4Custom2ExpUnit
            30,     // u4Custom2MaxFrameRate
            1024,   // u4Custom22PreRatio, 1024 base = 1x
            19770,     // u4Custom3ExpUnit
            30,     // u4Custom3MaxFrameRate
            1024,   // u4Custom32PreRatio, 1024 base = 1x
            19770,     // u4Custom4ExpUnit
            30,     // u4Custom4MaxFrameRate
            1024,   // u4Custom42PreRatio, 1024 base = 1x
            19770,     // u4Custom5ExpUnit
            30,     // u4Custom5MaxFrameRate
            1024,   // u4Custom52PreRatio, 1024 base = 1x
            20,    // u4LensFno, Fno = 2.8
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
            50,//52,//62,//65,//47,                // u4AETarget
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
            -4,               // i4BVOffset delta BV = -2.3
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
                                      200,// 220, // 160,//u4Thd                        // 0~255
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
                                        TRUE,//bEnableBlackLight
                                        1024,//u4BackLightWeight
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
                                        200, //  174,  //u4Thd
                                        56, //72,   //u4FlatThd
                                        200,  //u4BrightTonePcent
                                        80, //110, //90,   //u4BrightToneThdMul;      // bright tone THD 1x=1024, m_u4AETarget*u4BrightToneThdMul/1024
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
                                                1024,  //i4Y1
                                                -2000,  //i4X2
                                                0     //i4Y2
                                        }
                            },
                            // rTOUCHFD_Spec
                            {
                    40, //uMeteringYLowBound;
                    50, //uMeteringYHighBound;
                   45,// 55, //uFaceYLowBound;
                   55,// 65, //uFaceYHighBound;
                    3, //uFaceCentralWeight;
                    120, //u4MeteringStableMax;
                    80, //u4MeteringStableMin;
                            }
                        }, //End rMeteringSpec
                        // rFlareSpec
                        {
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                96,                                               //u4FlareStdThrHigh;
                48,                                               //u4FlareStdThrLow;
                0,                                               //u4PrvCapFlareDiff;
                4,                                               //u4FlareMaxStepGap_Fast;
                0,                                               //u4FlareMaxStepGap_Slow;
                1800,                                               //u4FlarMaxStepGapLimitBV;
                0,                                               //u4FlareAEStableCount;
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
                {//rAEAOEAlgParam
                    {//rAEAOEAlgParam[0]
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
                    {//rAEAOEAlgParam[1]
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
                    {//rAEAOEAlgParam[2]
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
    AWB_NVRAM_START
    {
   {
   {
  // AWB calibration data
  {
    // rUnitGain (unit gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
    // rGoldenGain (golden sample gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
                            // rUnitGain TL84 (unit gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                            // rGoldenGain TL84 (golden sample gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                             // rUnitGain Alight (unit gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                            // rGoldenGain Alight (golden sample gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
    // rD65Gain (D65 WB gain: 1.0 = 512)
    {
                            989,    // D65Gain_R
                            512,    // D65Gain_G
                            668    // D65Gain_B
    }
  },
  // Original XY coordinate of AWB light source
  {
    // Strobe
    {
      0,  // i4X
      0 // i4Y
    },
    // Horizon
    {
                        -364,    // OriX_Hor
                        -325    // OriY_Hor
    },
    // A
    {
                        -253,    // OriX_A
                        -351    // OriY_A
    },
    // TL84
    {
                        -84,    // OriX_TL84
                        -383    // OriY_TL84
    },
    // CWF
    {
                        -70,    // OriX_CWF
                        -430    // OriY_CWF
    },
    // DNP
    {
                        47,    // OriX_DNP
                        -430    // OriY_DNP
    },
    // D65
    {
                        145,    // OriX_D65
                        -342    // OriY_D65
    },
    // DF
    {
                        105,    // OriX_DF
                        -429    // OriY_DF
    }
  },
  // Rotated XY coordinate of AWB light source
  {
    // Strobe
    {
      0,     // i4X
      0     // i4Y
    },
    // Horizon
    {
                        -369,    // RotX_Hor
                        -319    // RotY_Hor
    },
    // A
    {
                        -258,    // RotX_A
                        -347    // RotY_A
    },
    // TL84
    {
                        -90,    // RotX_TL84
                        -382    // RotY_TL84
    },
    // CWF
    {
                        -77,    // RotX_CWF
                        -429    // RotY_CWF
    },
    // DNP
    {
                        40,    // RotX_DNP
                        -431    // RotY_DNP
    },
    // D65
    {
                        140,    // RotX_D65
                        -344    // RotY_D65
    },
    // DF
    {
                        98,    // RotX_DF
                        -431    // RotY_DF
    }
  },
  // AWB gain of AWB light source
  {
    // Strobe
    {
      512,  // i4R
      512,  // i4G
      512 // i4B
    },
    // Horizon
    {
                    512,    // AWBGAIN_HOR_R
                    539,    // AWBGAIN_HOR_G
                    1370    // AWBGAIN_HOR_B
    },
    // A
    {
                    584,    // AWBGAIN_A_R
                    512,    // AWBGAIN_A_G
                    1160    // AWBGAIN_A_B
    },
    // TL84
    {
                    767,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    963    // AWBGAIN_TL84_B
    },
    // CWF
    {
                    834,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1008    // AWBGAIN_CWF_B
    },
    // DNP
    {
                    977,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    860    // AWBGAIN_DNP_B
    },
    // D65
    {
                    989,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    668    // AWBGAIN_D65_B
    },
    // DF
    {
                    1055,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    794    // AWBGAIN_DF_B
    }
  },
  // Rotation matrix parameter
  {
    1,  // i4RotationAngle
    256,  // i4Cos
    4 // i4Sin
  },
  // Daylight locus parameter
  {
    -130, // i4SlopeNumerator
    128 // i4SlopeDenominator
  },
 // Predictor gain
 {
                    101, // i4PrefRatio100
                        // DaylightLocus_L
                        {
                        959,    // i4R
                        530,    // i4G
                        688    // i4B
                        },
                        // DaylightLocus_H
                        {
                        850,    // i4R
                            512,    // i4G
                        782    // i4B
                        },
                        // Temporal General
                        {
                        989,    // i4R
                            512,    // i4G
                        668    // i4B
                        },
                        // AWB LSC Gain
                        {
                        850,        // i4R
                            512,    // i4G
                        782        // i4B
                        }
 },
  // AWB light area
  {
    // Strobe
    {
      0,  // StrobeRightBound
      0,  // StrobeLeftBound
      0,  // StrobeUpperBound
      0 // StrobeLowerBound
    },
    // Tungsten
    {
                        -157,    // TungRightBound
                        -769,    // TungLeftBound
                        -264,    // TungUpperBound
                        -365    // TungLowerBound
    },
    // Warm fluorescent
    {
                        -157,    // WFluoRightBound
                        -769,    // WFluoLeftBound
                        -365,    // WFluoUpperBound
                        -519    // WFluoLowerBound
    },
    // Fluorescent
    {
                        -2,    // FluoRightBound
                        -157,    // FluoLeftBound
                        -284,    // FluoUpperBound
                        -406    // FluoLowerBound
    },
    // CWF
    {
                        0,    // CWFRightBound
                        -157,    // CWFLeftBound
                        -406,    // CWFUpperBound
                        -484    // CWFLowerBound
    },
    // Daylight
    {
                        170,    // DayRightBound
                        -2,    // DayLeftBound
                        -284,    // DayUpperBound
                        -406    // DayLowerBound
    },
    // Shade
    {
                        500,    // ShadeRightBound
                        170,    // ShadeLeftBound
                        -284,    // ShadeUpperBound
                        -378    // ShadeLowerBound
    },
    // Daylight Fluorescent
    {
                        170,    // DFRightBound
                        0,    // DFLeftBound
                        -406,    // DFUpperBound
                        -484    // DFLowerBound
    }
  },
  // PWB light area
  {
    // Reference area
    {
                        500,    // PRefRightBound
                        -769,    // PRefLeftBound
                        -239,    // PRefUpperBound
                        -519    // PRefLowerBound
    },
    // Daylight
    {
                        195,    // PDayRightBound
                        -2,    // PDayLeftBound
                        -284,    // PDayUpperBound
                        -406    // PDayLowerBound
    },
    // Cloudy daylight
    {
                        295,    // PCloudyRightBound
                        120,    // PCloudyLeftBound
                        -284,    // PCloudyUpperBound
                        -406    // PCloudyLowerBound
    },
    // Shade
    {
                        395,    // PShadeRightBound
                        120,    // PShadeLeftBound
                        -284,    // PShadeUpperBound
                        -406    // PShadeLowerBound
    },
    // Twilight
    {
                        -2,    // PTwiRightBound
                        -162,    // PTwiLeftBound
                        -284,    // PTwiUpperBound
                        -406    // PTwiLowerBound
    },
    // Fluorescent
    {
                        190,    // PFluoRightBound
                        -190,    // PFluoLeftBound
                        -294,    // PFluoUpperBound
                        -479    // PFluoLowerBound
    },
    // Warm fluorescent
    {
                        -158,    // PWFluoRightBound
                        -358,    // PWFluoLeftBound
                        -294,    // PWFluoUpperBound
                        -479    // PWFluoLowerBound
    },
    // Incandescent
    {
                        -158,    // PIncaRightBound
                        -358,    // PIncaLeftBound
                        -284,    // PIncaUpperBound
                        -406    // PIncaLowerBound
    },
    // Gray World
    {
      5000, // PGWRightBound
      -5000,  // PGWLeftBound
      5000, // PGWUpperBound
      -5000 // PGWLowerBound
    }
  },
  // PWB default gain
  {
    // Daylight
    {
                    936,    // PWB_Day_R
      512,  // PWB_Day_G
                    710    // PWB_Day_B
    },
    // Cloudy daylight
    {
                    1085,    // PWB_Cloudy_R
      512,  // PWB_Cloudy_G
                    610    // PWB_Cloudy_B
    },
    // Shade
    {
                    1159,    // PWB_Shade_R
      512,  // PWB_Shade_G
                    569    // PWB_Shade_B
    },
    // Twilight
    {
                    738,    // PWB_Twi_R
      512,  // PWB_Twi_G
                    908    // PWB_Twi_B
    },
    // Fluorescent
    {
                    871,    // PWB_Fluo_R
      512,  // PWB_Fluo_G
                    857    // PWB_Fluo_B
    },
    // Warm fluorescent
    {
                    618,    // PWB_WFluo_R
      512,  // PWB_WFluo_G
                    1222    // PWB_WFluo_B
    },
    // Incandescent
    {
                    583,    // PWB_Inca_R
      512,  // PWB_Inca_G
                    1156    // PWB_Inca_B
    },
    // Gray World
    {
      512,  // PWB_GW_R
      512,  // PWB_GW_G
      512 // PWB_GW_B
    }
  },
  // AWB preference color
  {
    // Tungsten
    {
                    50,    // TUNG_SLIDER
                    4046    // TUNG_OFFS
    },
    // Warm fluorescent
    {
      50, // WFluo_SLIDER
                    4046    // WFluo_OFFS
    },
    // Shade
    {
      50, // Shade_SLIDER
      909 // Shade_OFFS
    },
            // Sunset Area
            {
                    72,   // i4Sunset_BoundXr_Thr
                    -431     // i4Sunset_BoundYr_Thr
                           },
            // Shade F Area
            {
                    -157,   // i4BoundXrThr
                    -386    // i4BoundYrThr
                            },
            // Shade F Vertex
           {
                    -80,   // i4BoundXrThr
                    -396    // i4BoundYrThr
                            },
            // Shade CWF Area
            {
                    -157,   // i4BoundXrThr
                    -433    // i4BoundYrThr
                            },
            // Shade CWF Vertex
            {
                    -79,   // i4BoundXrThr
                    -459    // i4BoundYrThr
                            }
  },
  // CCT estimation
  {
    // CCT
    {
      2300, // CCT0
      2850, // CCT1
      3750, // CCT2
      5100, // CCT3
      6500     // CCT4
    },
    // Rotated X coordinate
    {
                        -509,    // i4RotatedXCoordinate[0]
                        -398,    // i4RotatedXCoordinate[1]
                        -230,    // i4RotatedXCoordinate[2]
                        -100,    // i4RotatedXCoordinate[3]
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
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
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
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // TUNGSTEN
                  {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
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
                    { 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Flurescent
                {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // CWF
                {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Daylight
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2},  // (%)
                    // DF
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,      // i4Enable
                            120,        // i4LVThr_L
                          130,        // i4LVThr_H
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,      // i4Enable
                        50,        // i4LVThr_L
                        70,        // i4LVThr_H
                            128         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,        // i4Enable
                        50,        // i4LVThr_L
                        70,        // i4LVThr_H
                        192         // i4DaylightProb
                        }
                },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {   0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {
            //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  25,  0,  0,  0,  0}, // Strobe
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}, // Warm fluorescent
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  40,  30,  20}, // Daylight
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0,   0}, // Shade
                {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0} // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6735,       // project code
                    5588,       // model
                    20150624,   // date
                    0,          // reserved 0
                    1,          // reserved 1
                    2,          // reserved 2
                    3,          // reserved 3
                    4,          // reserved 4
                }
            }

},
                {
   {
  // AWB calibration data
  {
    // rUnitGain (unit gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
    // rGoldenGain (golden sample gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
                            // rUnitGain TL84 (unit gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                            // rGoldenGain TL84 (golden sample gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                             // rUnitGain Alight (unit gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                            // rGoldenGain Alight (golden sample gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
    // rD65Gain (D65 WB gain: 1.0 = 512)
    {
                    872,    // D65Gain_R
                    512,    // D65Gain_G
                    579    // D65Gain_B
    }
  },
  // Original XY coordinate of AWB light source
  {
    // Strobe
    {
      0,  // i4X
      0 // i4Y
    },
    // Horizon
    {
                    -345,    // OriX_Hor
                    -231    // OriY_Hor
    },
    // A
    {
                    -243,    // OriX_A
                    -247    // OriY_A
    },
    // TL84
    {
                    -100,    // OriX_TL84
                    -279    // OriY_TL84
    },
    // CWF
    {
                    -79,    // OriX_CWF
                    -377    // OriY_CWF
    },
    // DNP
    {
                    8,    // OriX_DNP
                    -294    // OriY_DNP
    },
    // D65
    {
                    151,    // OriX_D65
                    -242    // OriY_D65
    },
    // DF
    {
                    113,    // OriX_DF
                    -32    // OriY_DF
    }
  },
  // Rotated XY coordinate of AWB light source
  {
    // Strobe
    {
      0,     // i4X
      0     // i4Y
    },
    // Horizon
    {
                    -349,    // RotX_Hor
                    -226    // RotY_Hor
    },
    // A
    {
                    -247,    // RotX_A
                    -243    // RotY_A
    },
    // TL84
    {
                    -104,    // RotX_TL84
                    -277    // RotY_TL84
    },
    // CWF
    {
                    -85,    // RotX_CWF
                    -376    // RotY_CWF
    },
    // DNP
    {
                    3,    // RotX_DNP
                    -294    // RotY_DNP
    },
    // D65
    {
                    147,    // RotX_D65
                    -244    // RotY_D65
    },
    // DF
    {
                    108,    // RotX_DF
                    -334    // RotY_DF
    }
  },
  // AWB gain of AWB light source
  {
    // Strobe
    {
      512,  // i4R
      512,  // i4G
      512 // i4B
    },
    // Horizon
    {
                    512,    // AWBGAIN_HOR_R
                    597,    // AWBGAIN_HOR_G
                    1302    // AWBGAIN_HOR_B
    },
    // A
    {
                    514,    // AWBGAIN_A_R
                    512,    // AWBGAIN_A_G
                    994    // AWBGAIN_A_B
    },
    // TL84
    {
                    653,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    855    // AWBGAIN_TL84_B
    },
    // CWF
    {
                    766,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    949    // AWBGAIN_CWF_B
    },
    // DNP
    {
                    770,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    755    // AWBGAIN_DNP_B
    },
    // D65
    {
                    872,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    579    // AWBGAIN_D65_B
    },
    // DF
    {
                    935,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    688    // AWBGAIN_DF_B
    }
  },
  // Rotation matrix parameter
  {
                1,    // RotationAngle
                256,    // Cos
                4    // Sin
  },
  // Daylight locus parameter
  {
    -130, // i4SlopeNumerator
    128 // i4SlopeDenominator
  },
 // Predictor gain
 {
                151, // i4PrefRatio100
                        // DaylightLocus_L
                        {
                    872,    // i4R
                            512,    // i4G
                    579    // i4B
                        },
                        // DaylightLocus_H
                        {
                    620,    // i4R
                            512,    // i4G
                    822    // i4B
                        },
                        // Temporal General
                        {
                    872,    // i4R
                            512,    // i4G
                    579    // i4B
                        },
                        // AWB LSC Gain
                        {
                    620,        // i4R
                            512,    // i4G
                    822        // i4B
                        }
 },
  // AWB light area
  {
    // Strobe
    {
      0,  // StrobeRightBound
      0,  // StrobeLeftBound
      0,  // StrobeUpperBound
      0 // StrobeLowerBound
    },
    // Tungsten
    {
                    -161,    // TungRightBound
                    -749,    // TungLeftBound
                    -191,    // TungUpperBound
                    -260    // TungLowerBound
    },
    // Warm fluorescent
    {
                    -161,    // WFluoRightBound
                    -749,    // WFluoLeftBound
                    -260,    // WFluoUpperBound
                    -406    // WFluoLowerBound
    },
    // Fluorescent
    {
                    -24,    // FluoRightBound
                    -161,    // FluoLeftBound
                    -204,    // FluoUpperBound
                    -339    // FluoLowerBound
    },
    // CWF
    {
                -8,    // CWFRightBound
                -161,    // CWFLeftBound
                -339,    // CWFUpperBound
                -421    // CWFLowerBound
    },
    // Daylight
    {
                    177,    // DayRightBound
                    -24,    // DayLeftBound
                    -204,    // DayUpperBound
                    -339    // DayLowerBound
    },
    // Shade
    {
                    507,    // ShadeRightBound
                    177,    // ShadeLeftBound
                    -204,    // ShadeUpperBound
                    -297    // ShadeLowerBound
    },
    // Daylight Fluorescent
    {
                    177,    // DFRightBound
                    -8,    // DFLeftBound
                    -339,    // DFUpperBound
                    -421    // DFLowerBound
    }
  },
  // PWB light area
  {
    // Reference area
    {
                    507,    // PRefRightBound
                    -749,    // PRefLeftBound
                    -166,    // PRefUpperBound
                    -421    // PRefLowerBound
    },
    // Daylight
    {
                    202,    // PDayRightBound
                    -24,    // PDayLeftBound
                    -204,    // PDayUpperBound
                    -339    // PDayLowerBound
    },
    // Cloudy daylight
    {
                    302,    // PCloudyRightBound
                    127,    // PCloudyLeftBound
                    -204,    // PCloudyUpperBound
                    -339    // PCloudyLowerBound
    },
    // Shade
    {
                    402,    // PShadeRightBound
                    127,    // PShadeLeftBound
                    -204,    // PShadeUpperBound
                    -339    // PShadeLowerBound
    },
    // Twilight
    {
                    -24,    // PTwiRightBound
                    -184,    // PTwiLeftBound
                    -204,    // PTwiUpperBound
                    -339    // PTwiLowerBound
    },
    // Fluorescent
    {
                    197,    // PFluoRightBound
                    -204,    // PFluoLeftBound
                    -194,    // PFluoUpperBound
                    -426    // PFluoLowerBound
    },
    // Warm fluorescent
    {
                    -147,    // PWFluoRightBound
                    -347,    // PWFluoLeftBound
                    -194,    // PWFluoUpperBound
                    -426    // PWFluoLowerBound
    },
    // Incandescent
    {
                    -147,    // PIncaRightBound
                    -347,    // PIncaLeftBound
                    -204,    // PIncaUpperBound
                    -339    // PIncaLowerBound
    },
    // Gray World
    {
      5000, // PGWRightBound
      -5000,  // PGWLeftBound
      5000, // PGWUpperBound
      -5000 // PGWLowerBound
    }
  },
  // PWB default gain
  {
    // Daylight
    {
                    837,    // PWB_Day_R
      512,  // PWB_Day_G
                    650    // PWB_Day_B
    },
    // Cloudy daylight
    {
                    990,    // PWB_Cloudy_R
      512,  // PWB_Cloudy_G
                    547    // PWB_Cloudy_B
    },
    // Shade
    {
                    1058,    // PWB_Shade_R
      512,  // PWB_Shade_G
                    511    // PWB_Shade_B
    },
    // Twilight
    {
                    647,    // PWB_Twi_R
      512,  // PWB_Twi_G
                    848    // PWB_Twi_B
    },
    // Fluorescent
    {
                    780,    // PWB_Fluo_R
      512,  // PWB_Fluo_G
                    778    // PWB_Fluo_B
    },
    // Warm fluorescent
    {
                    564,    // PWB_WFluo_R
      512,  // PWB_WFluo_G
                    1087    // PWB_WFluo_B
    },
    // Incandescent
    {
                    535,    // PWB_Inca_R
      512,  // PWB_Inca_G
                    1032    // PWB_Inca_B
    },
    // Gray World
    {
      512,  // PWB_GW_R
      512,  // PWB_GW_G
      512 // PWB_GW_B
    }
  },
  // AWB preference color
  {
    // Tungsten
    {
      60, // TUNG_SLIDER
                    3996    // TUNG_OFFS
    },
    // Warm fluorescent
    {
      50, // WFluo_SLIDER
                    4196    // WFluo_OFFS
    },
    // Shade
    {
      50, // Shade_SLIDER
      909 // Shade_OFFS
    },
            // Sunset Area
            {
                                        30,   // i4Sunset_BoundXr_Thr
                                        -294     // i4Sunset_BoundYr_Thr
                           },
            // Shade F Area
            {
                    -77,   // i4BoundXrThr
                    -244    // i4BoundYrThr
                            },
            // Shade F Vertex
           {
                    -77,   // i4BoundXrThr
                    -244    // i4BoundYrThr
                            },
            // Shade CWF Area
            {
                    -85,   // i4BoundXrThr
                    -376    // i4BoundYrThr
                            },
            // Shade CWF Vertex
            {
                                        -85,   // i4BoundXrThr
                                        -376    // i4BoundYrThr
                            }
  },
  // CCT estimation
  {
    // CCT
    {
      2300, // CCT0
      2850, // CCT1
      3750, // CCT2
      5100, // CCT3
      6500     // CCT4
    },
    // Rotated X coordinate
    {
                -496,    // i4RotatedXCoordinate[0]
                -394,    // i4RotatedXCoordinate[1]
                -251,    // i4RotatedXCoordinate[2]
                -144,    // i4RotatedXCoordinate[3]
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
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
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
                    { 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Flurescent
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // CWF
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Daylight
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2},  // (%)
                    // DF
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,      // i4Enable
                            120,        // i4LVThr_L
                          130,        // i4LVThr_H
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,      // i4Enable
                                    0,        // i4LVThr_L
                                  1,        // i4LVThr_H
                        96         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,        // i4Enable
                                    0,        // i4LVThr_L
                                    1,        // i4LVThr_H
                                    208         // i4DaylightProb
                        }
                },

                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  50,  30,  20,  10,   0,   0,   0,   0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {
            //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  25,  0,  0,  0,  0}, // Strobe
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}, // Tungsten
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  95,  70,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  40,  30,  20}, // Daylight
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0,   0}, // Shade
                    {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0} // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6735,       // project code
                    5588,       // model
                    20150624,   // date
                    0,          // reserved 0
                    1,          // reserved 1
                    2,          // reserved 2
                    3,          // reserved 3
                    4,          // reserved 4
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
        0,
                                             sizeof(NVRAM_CAMERA_FEATURE_STRUCT)
                                            };

    if (CameraDataType > CAMERA_NVRAM_DATA_FEATURE || NULL == pDataBuf || (size != dataSize[CameraDataType]))
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
