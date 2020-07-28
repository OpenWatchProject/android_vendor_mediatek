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
#ifndef _CAMERA_CUSTOM_AE_NVRAM_H_
#define _CAMERA_CUSTOM_AE_NVRAM_H_

// #include <stddef.h>
#include "MediaTypes.h"
#include "camera_custom_AEPlinetable.h"
// #include "CFG_Camera_File_Max_Size.h"
#include "aaa/ae_feature.h"
#define NVRAM_CUSTOM_AE_REVISION   7482001

#ifndef MBOOL
typedef int MBOOL;
#endif
#ifndef MINT8
typedef signed char         MINT8;
#endif
#ifndef MINT16
typedef signed short         MINT16;
#endif
#ifndef MINT32
typedef signed int         MINT32;
#endif
#ifndef MTRUE
#define MTRUE  1
#endif
#ifndef MFALSE
#define MFALSE 0
#endif

#define MOVING_TABLE_SIZE 24
/*******************************************************************************
* AE
********************************************************************************/
typedef struct
{
    MUINT32 u4MinGain;
    MUINT32 u4MaxGain;
    MUINT32 u4MiniISOGain;
    MUINT32 u4GainStepUnit;
    MUINT32 u4PreExpUnit;
    MUINT32 u4PreMaxFrameRate;
    MUINT32 u4VideoExpUnit;
    MUINT32 u4VideoMaxFrameRate;
    MUINT32 u4Video2PreRatio;    // 1x = 1024
    MUINT32 u4CapExpUnit;
    MUINT32 u4CapMaxFrameRate;
    MUINT32 u4Cap2PreRatio;     // 1x = 1024
    MUINT32 u4Video1ExpUnit;
    MUINT32 u4Video1MaxFrameRate;
    MUINT32 u4Video12PreRatio;    // 1x = 1024
    MUINT32 u4Video2ExpUnit;
    MUINT32 u4Video2MaxFrameRate;
    MUINT32 u4Video22PreRatio;    // 1x = 1024
    MUINT32 u4Custom1ExpUnit;
    MUINT32 u4Custom1MaxFrameRate;
    MUINT32 u4Custom12PreRatio;    // 1x = 1024
    MUINT32 u4Custom2ExpUnit;
    MUINT32 u4Custom2MaxFrameRate;
    MUINT32 u4Custom22PreRatio;    // 1x = 1024
    MUINT32 u4Custom3ExpUnit;
    MUINT32 u4Custom3MaxFrameRate;
    MUINT32 u4Custom32PreRatio;    // 1x = 1024
    MUINT32 u4Custom4ExpUnit;
    MUINT32 u4Custom4MaxFrameRate;
    MUINT32 u4Custom42PreRatio;    // 1x = 1024
    MUINT32 u4Custom5ExpUnit;
    MUINT32 u4Custom5MaxFrameRate;
    MUINT32 u4Custom52PreRatio;    // 1x = 1024
    MUINT32 u4LensFno;           // 10 Base
    MUINT32 u4FocusLength_100x;           // 100 Base
} AE_DEVICES_INFO_T;

//histogram control information
#define AE_CCT_STRENGTH_NUM (5)
#define AE_AOE_STRENGTH_NUM (3)


typedef struct {
    MINT32 u4X1;
    MINT32 u4Y1;
    MINT32 u4X2;
    MINT32 u4Y2;
} AE_TARGET_PROB_T;

typedef struct {
    MBOOL   bEnableHistStretch;          // enable histogram stretch
    MUINT32 u4HistStretchWeight;         // Histogram weighting value
    MUINT32 u4Pcent;                     // 1%=10, 0~1000
    MUINT32 u4Thd;                       // 0~255
    MUINT32 u4FlatThd;                   // 0~255

    MUINT32 u4FlatBrightPcent;
    MUINT32 u4FlatDarkPcent;
    AE_TARGET_PROB_T    sFlatRatio;     //TARGET_HS_FLAT

    MBOOL  bEnableGreyTextEnhance;
    MUINT32 u4GreyTextFlatStart;
    AE_TARGET_PROB_T  sGreyTextRatio;   //TARGET_HS_COLOR
} AE_HS_SPEC_T;

typedef struct {
    MBOOL   bEnableAntiOverExposure;     // enable anti over exposure
    MUINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value
    MUINT32 u4Pcent;                     // 1%=10, 0~1000
    MUINT32 u4Thd;                       // 0~255

    MBOOL bEnableCOEP;                   // enable COEP
    MUINT32 u4COEPcent;                  // center over-exposure prevention
    MUINT32 u4COEThd;                    // center y threshold
    MUINT32 u4BVCompRatio;               // Compensate BV in nonlinear
    AE_TARGET_PROB_T    sCOEYRatio;     // the outer y ratio
    AE_TARGET_PROB_T    sCOEDiffRatio;  // inner/outer y difference ratio
} AE_AOE_SPEC_T;

typedef struct {
    MBOOL   bEnableBlackLight;           // enable back light detector
    MUINT32 u4BackLightWeight;           // Back light weighting value
    MUINT32 u4Pcent;                     // 1%=10, 0~1000
    MUINT32 u4Thd;                       // 0~255

    MUINT32 u4CenterHighBnd;             // center luminance
    MUINT32 u4TargetStrength;            // final target limitation
    AE_TARGET_PROB_T    sFgBgEVRatio;   //TARGET_ABL_DIFF
    AE_TARGET_PROB_T    sBVRatio;       //FT_ABL
} AE_ABL_SPEC_T;

typedef struct {
    MBOOL   bEnableNightScene;       // enable night scene
    MUINT32 u4Pcent;                 // 1=0.1%, 0~1000
    MUINT32 u4Thd;                   // 0~255
    MUINT32 u4FlatThd;               // ev difference between darkest and brightest

    MUINT32 u4BrightTonePcent;       // 1=0.1%bright tone percentage
    MUINT32 u4BrightToneThd;         // < 255, bright tone THD

    MUINT32 u4LowBndPcent;           // darkest percent, 1%=10, 0~1000
    MUINT32 u4LowBndThd;             // <255, lower bound target
    MUINT32 u4LowBndThdLimit;        // <255, upper bound of lower bound

    MUINT32 u4FlatBrightPcent;       // the percentage of the brightest part used to cal flatness
    MUINT32 u4FlatDarkPcent;         // the percentage of the darkest part used to cal flatness
    AE_TARGET_PROB_T    sFlatRatio; //TARGET_NS_FLAT
    AE_TARGET_PROB_T    sBVRatio;   //FT_NS

    MBOOL bEnableNightSkySuppresion;
    AE_TARGET_PROB_T    sSkyBVRatio;   //FT_NS_SKY
} AE_NS_SPEC_T;

typedef struct {
    MUINT32 uMeteringYLowBound;
    MUINT32 uMeteringYHighBound;
    MUINT32 uFaceYLowBound;
    MUINT32 uFaceYHighBound;
    MUINT32 uFaceCentralWeight;
    MUINT32 u4MeteringStableMax;
    MUINT32 u4MeteringStableMin;
} AE_TOUCH_FD_SPEC_T;

typedef struct {
    MUINT32 uPrvFlareWeightArr[16];
    MUINT32 uVideoFlareWeightArr[16];
    MUINT32 u4FlareStdThrHigh;
    MUINT32 u4FlareStdThrLow;
    MUINT32 u4PrvCapFlareDiff;
    MUINT32 u4FlareMaxStepGap_Fast;
    MUINT32 u4FlareMaxStepGap_Slow;
    MUINT32 u4FlarMaxStepGapLimitBV;
    MUINT32 u4FlareAEStableCount;
} AE_FLARE_T;

typedef struct {
    AE_HS_SPEC_T        rHS_Spec;
    AE_AOE_SPEC_T       rAOE_Spec;
    AE_ABL_SPEC_T       rABL_Spec;
    AE_NS_SPEC_T        rNS_Spec;
    AE_TOUCH_FD_SPEC_T  rTOUCHFD_Spec;
} AE_METER_SPEC_T;


typedef struct
{
        MUINT32 u4SpeedUpRatio;
        MUINT32 u4GlobalRatio;
        MUINT32 u4Bright2TargetEnd;
        MUINT32 u4Dark2TargetStart;
        MUINT32 u4B2TEnd;
        MUINT32 u4B2TStart;
        MUINT32 u4D2TEnd;
        MUINT32 u4D2TStart;
} strAEMovingRatio;

typedef struct {
    MUINT32   u4Y_Target;                     // for AOE target , LE target , SE target -> 47
    MUINT32   u4AOE_OE_percent;         // high light percentage  x / 1000 -> 1%
    MUINT32   u4AOE_OEBound;             // for Over expsosure boud -> 184
    MUINT32   u4AOE_DarkBound;          // for Min luminance bound -> 20
    MUINT32   u4AOE_LowlightPrecent;  // for Lowlight bound percentage / 1000 ,95%
    MUINT32   u4AOE_LowlightBound;     // for Lowlight bound , 95% -> 10
    MUINT32   u4AOESceneLV_L;             // low LV start to reduce AOE -> 100
    MUINT32   u4AOESceneLV_H;             // High LV start to reduce AOE -> 150
    MUINT32   u4AOE_SWHdrLE_Bound;  // LE Condition for SW HDR -> 40
} strAEAOEAlgParam;

typedef struct
{
    MINT32    i4AOEStrengthIdx;              // AOE strength index: 0 / 1 / 2
    MUINT32    u4BVCompRatio;               // Compensate BV in nonlinear
    strAEAOEAlgParam rAEAOEAlgParam[AE_AOE_STRENGTH_NUM];
} strAEAOEInputParm;

typedef struct
{
   //histogram info
    MUINT32 u4HistHighThres;                         // central histogram high threshold
    MUINT32 u4HistLowThres;                          // central histogram low threshold
    MUINT32 u4MostBrightRatio;                       // full histogram high threshold
    MUINT32 u4MostDarkRatio;                         // full histogram low threshold
    MUINT32 u4CentralHighBound;                      // central block high boundary
    MUINT32 u4CentralLowBound;                       // central block low bounary
    MUINT32 u4OverExpThres[AE_CCT_STRENGTH_NUM];     // over exposure threshold
    MUINT32 u4HistStretchThres[AE_CCT_STRENGTH_NUM]; // histogram stretch trheshold
    MUINT32 u4BlackLightThres[AE_CCT_STRENGTH_NUM];  // backlight threshold
} AE_HIST_CFG_T;

//strAETable AE table Setting
typedef struct
{
    MBOOL   bEnableBlackLight;           // enable back light detector
    MBOOL   bEnableHistStretch;          // enable histogram stretch
    MBOOL   bEnableAntiOverExposure;     // enable anti over exposure
    MBOOL   bEnableTimeLPF;              // enable time domain LPF for smooth converge
    MBOOL   bEnableCaptureThres;         // enable capture threshold or fix flare offset
    MBOOL   bEnableVideoThres;             // enable video threshold or fix flare offset
    MBOOL   bEnableVideo1Thres;       // enable video1 threshold or fix flare offset
    MBOOL   bEnableVideo2Thres;       // enable video2 threshold or fix flare offset
    MBOOL   bEnableCustom1Thres;    // enable custom1 threshold or fix flare offset
    MBOOL   bEnableCustom2Thres;    // enable custom2 threshold or fix flare offset
    MBOOL   bEnableCustom3Thres;    // enable custom3 threshold or fix flare offset
    MBOOL   bEnableCustom4Thres;    // enable custom4 threshold or fix flare offset
    MBOOL   bEnableCustom5Thres;    // enable custom5 threshold or fix flare offset
    MBOOL   bEnableStrobeThres;           // enable strobe threshold or fix flare offset

    MUINT32 u4AETarget;                  // central weighting target
    MUINT32 u4StrobeAETarget;            // central weighting target
    MUINT32 u4InitIndex;                 // AE initiail index

    MUINT32 u4BackLightWeight;           // Back light weighting value
    MUINT32 u4HistStretchWeight;         // Histogram weighting value
    MUINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value

    MUINT32 u4BlackLightStrengthIndex;   // Black light threshold strength index
    MUINT32 u4HistStretchStrengthIndex;  // Histogram stretch threshold strength index
    MUINT32 u4AntiOverExpStrengthIndex;  // Anti over exposure threshold strength index
    MUINT32 u4TimeLPFStrengthIndex;      // Smooth converge threshold strength index
    MUINT32 u4LPFConvergeLevel[AE_CCT_STRENGTH_NUM];  //LPF converge support level

    MUINT32 u4InDoorEV;                  // check the environment indoor/outdoor
    MINT32   i4BVOffset;                  // Calibrate BV offset
    MUINT32 u4PreviewFlareOffset;        // Fix preview flare offset
    MUINT32 u4CaptureFlareOffset;        // Fix capture flare offset
    MUINT32 u4CaptureFlareThres;         // Capture flare threshold
    MUINT32 u4VideoFlareOffset;        // Fix video flare offset
    MUINT32 u4VideoFlareThres;         // video flare threshold
    MUINT32 u4CustomFlareOffset;        // Fix custom flare offset
    MUINT32 u4CustomFlareThres;         // custom flare threshold
    MUINT32 u4StrobeFlareOffset;        // Fix strobe flare offset
    MUINT32 u4StrobeFlareThres;         // strobe flare threshold
    MUINT32 u4PrvMaxFlareThres;        // for max preview flare thres used
    MUINT32 u4PrvMinFlareThres;         // for min preview flare thres used
    MUINT32 u4VideoMaxFlareThres;        // for video max flare thres used
    MUINT32 u4VideoMinFlareThres;         // for video min flare thres used
    MUINT32 u4FlatnessThres;              // 10 base for flatness condition.
    MUINT32 u4FlatnessStrength;

    // v2.0
    AE_METER_SPEC_T rMeteringSpec;
    AE_FLARE_T      rFlareSpec;
    strAEMovingRatio  rAEMovingRatio;         // Preview
    strAEMovingRatio  rAEVideoMovingRatio;    // Video
    strAEMovingRatio  rAEVideo1MovingRatio; // Video1 Tracking
    strAEMovingRatio  rAEVideo2MovingRatio; // Video2 Tracking
    strAEMovingRatio  rAECustom1MovingRatio; // Custom1 Tracking
    strAEMovingRatio  rAECustom2MovingRatio; // Custom2 Tracking
    strAEMovingRatio  rAECustom3MovingRatio; // Custom3 Tracking
    strAEMovingRatio  rAECustom4MovingRatio; // Custom4 Tracking
    strAEMovingRatio  rAECustom5MovingRatio; // Custom5 Tracking
    strAEMovingRatio  rAEFaceMovingRatio;     // Face AE
    strAEMovingRatio  rAETrackingMovingRatio; // Object Tracking
    strAEAOEInputParm rAEAOENVRAMParam;
} AE_CCT_CFG_T;                            // histogram control information

#define AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM        (11)

typedef struct {
    MINT32 i4RMGSeg;
    MINT32 i4RMGTurn;
    MINT32 i4HDRTarget_L;
    MINT32 i4HDRTarget_H;
    MINT32 i4HDRTargetLV_L;
    MINT32 i4HDRTargetLV_H;
    MINT32 i4OverExpoRatio;
    MINT32 i4OverExpoTarget;
    MINT32 i4OverExpoLV_L;
    MINT32 i4OverExpoLV_H;
    MINT32 i4UnderExpoContrastThr;
    MINT32 i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
    MINT32 i4UnderExpoRatio;
    MINT32 i4AvgExpoRatio;
    MINT32 i4AvgExpoTarget;
    MINT32 i4HDRAESpeed;
    MINT32 i4HDRConvergeThr;
    /*
    *   SW HDR
    */
    MINT32 i4SWHdrLEThr;
    MINT32 i4SWHdrSERatio;
    MINT32 i4SWHdrSETarget;
    MINT32 i4SWHdrBaseGain;
} HDR_AE_CFG_T;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//From ae_param.h
#define MAX_AE_METER_AREAS  9
#define AE_RATIOTBL_MAXSIZE 8
#define AE_V4P0_BLOCK_NO  15
#define MAX_WEIGHT_TABLE 4
#define AE_BLOCK_NO  5
#define AE_TG_BLOCK_NO_X 12
#define AE_TG_BLOCK_NO_Y 9
#define AE_TG_BLOCK_TOTAL_SIZE (AE_TG_BLOCK_NO_X*AE_TG_BLOCK_NO_Y)

typedef struct
{
    MBOOL   bEnableSaturationCheck;        //if toward high saturation scene , then reduce AE target
    MBOOL   bEnablePreIndex;                    // decide the re-initial index after come back to camera
    MBOOL   bEnableRotateWeighting;        // AE rotate the weighting automatically or not
    MBOOL   bEV0TriggerStrobe;
    MBOOL   bLockCamPreMeteringWin;
    MBOOL   bLockVideoPreMeteringWin;
    MBOOL   bLockVideoRecMeteringWin;
    MBOOL   bSkipAEinBirghtRange;            // To skip the AE in some brightness range for meter AE
    MBOOL   bPreAFLockAE;                        // Decide the do AE in the pre-AF or post-AF
    MBOOL   bStrobeFlarebyCapture;          // to Decide the strobe flare by capture image or precapture image
    MBOOL   bEnableFaceAE;                        // Enable the Face AE or not
    MBOOL   bEnableMeterAE;                      // Enable the Meter AE or not
    MBOOL   bFlarMaxStepGapLimitEnable;   //enable max step gap for low light
    MBOOL   bEnableAESmoothGain;             // Enable AE smooth gain
    MBOOL   bEnableLongCaptureCtrl;          // enable the log capture control sequence
    MBOOL   bEnableTouchSmoothRatio;         // enable Touch mode smooth converge
    MBOOL   bEnableTouchSmooth;              // enable Touch mode smooth with perframe
    MBOOL   bEnablePixelBaseHist;            // enable pixel based histogram
    MBOOL   bEnableHDRSTATconfig;            // enable HDR Y LSB config
    MBOOL   bEnableAEOVERCNTconfig;          // enable AE over-exposure count config
    MBOOL   bEnableTSFSTATconfig;            // enable TSF RGB STAT config
    MUINT8   bEnableHDRLSB;                     //HW HDR enable flag 0:8bit(default) 1:12bit 2:14bit
    MBOOL   bEnableFlareFastConverge;        // enable fast flare converge
    MBOOL   bEnableRAFastConverge;           // enable fast RA converge
    MBOOL   bEnableFaceFastConverge;         // enable fast face converge
    MUINT32 u4BackLightStrength;              // strength of backlight condtion
    MUINT32 u4OverExpStrength;               // strength of anti over exposure
    MUINT32 u4HistStretchStrength;           //strength of  histogram stretch
    MUINT32 u4SmoothLevel;                      // time LPF smooth level , internal use
    MUINT32 u4TimeLPFLevel;                     //time LOW pass filter level
    MUINT32 u4AEWinodwNumX;                   // AE statistic winodw number X
    MUINT32 u4AEWinodwNumY;                   // AE statistic winodw number Y
    MUINT8 uBlockNumX;                         //AE X block number
    MUINT8 uBlockNumY;                         //AE Yblock number
    MUINT8 uTgBlockNumX;                         //AE X block number
    MUINT8 uTgBlockNumY;                         //AE Yblock number
    MUINT8 uSatBlockCheckLow;             //saturation block check , low thres
    MUINT8 uSatBlockCheckHigh;            //sturation  block check , hight thres
    MUINT8 uSatBlockAdjustFactor;        // adjust factore , to adjust central weighting target value
    MUINT8 uMeteringYLowSkipRatio;     // metering area min Y value to skip AE
    MUINT8 uMeteringYHighSkipRatio;    // metering area max Y value to skip AE
    MUINT32 u4MinYLowBound;                 // metering boundary min Y value
    MUINT32 u4MaxYHighBound;                // metering boundary max Y value
    MUINT32 u4MeterWeight;              // metering weight betwenn CWR and MeterTarget
    MUINT32 u4MinCWRecommend;           // mini target value
    MUINT32 u4MaxCWRecommend;          // max target value
    MINT8   iMiniBVValue;                          // mini BV value.
    MINT8   uAEShutterDelayCycle;         // for AE smooth used.
    MINT8   uAESensorGainDelayCycleWShutter;
    MINT8   uAESensorGainDelayCycleWOShutter;
    MINT8   uAEIspGainDelayCycle;

    // custom Y coefficients
    MUINT32   u4AEYCoefR;        // for AE_YCOEF_R
    MUINT32   u4AEYCoefG;        // for AE_YCOEF_G
    MUINT32   u4AEYCoefB;        // for AE_YCOEF_B

    MUINT32   u4LongCaptureThres;          // enable the log capture control sequence
    MUINT32   u4CenterFaceExtraWeighting;  // give center face an extra weighting
    MUINT16 u2AEStatThrs;
    MUINT8  uCycleNumAESkipAfterExit;
    MUINT8  uOverExpoTHD;                  //overexposure cnt threshold
    MUINT32 u4HSSmoothTHD;
    MUINT32 u4FinerEvIndexBase;            // 1:0.1EV 2:0.05EV 3:0.033EV  10:0.01
    MBOOL bNewConstraintForMeteringEnable; // Temp solution, it will be removed @ ISP5
}strAEParamCFG;

typedef enum
{
    AE_WEIGHTING_CENTRALWEIGHT=0,
    AE_WEIGHTING_SPOT,
    AE_WEIGHTING_AVERAGE
}eWeightingID;

typedef struct
{
   eWeightingID eID;  //weighting table ID
   MUINT32 W[5][5];    //AE weighting table
}strWeightTable;

typedef struct
{
   MBOOL bAFPlineEnable;
   MINT16 i2FrameRate[5][2];
}strAFPlineInfo;

typedef struct
{
    MINT32 Diff_EV;     //  delta EVx10 ,different between Yavg and Ytarget     Diff_EV=    log(  Yarg/Ytarget,2)
    MINT32  Ration;        //  Yarg/Ytarget  *100
    MINT32  move_index;   // move index
}strAEMOVE;


typedef struct
{
    strAEMOVE table[MOVING_TABLE_SIZE];
} strAEMoveTable;

typedef struct
{
    MINT8 iLEVEL1_GAIN;
    MINT8 iLEVEL2_GAIN;
    MINT8 iLEVEL3_GAIN;
    MINT8 iLEVEL4_GAIN;
    MINT8 iLEVEL5_GAIN;
    MINT8 iLEVEL6_GAIN;
    MINT8 iLEVEL1_TARGET_DIFFERENCE;
    MINT8 iLEVEL2_TARGET_DIFFERENCE;
    MINT8 iLEVEL3_TARGET_DIFFERENCE;
    MINT8 iLEVEL4_TARGET_DIFFERENCE;
    MINT8 iLEVEL5_TARGET_DIFFERENCE;
    MINT8 iLEVEL6_TARGET_DIFFERENCE;
    MINT8 iLEVEL1_GAINH;
    MINT8 iLEVEL1_GAINL;
    MINT8 iLEVEL2_GAINH;
    MINT8 iLEVEL2_GAINL;
    MINT8 iLEVEL3_GAINH;
    MINT8 iLEVEL3_GAINL;
    MINT8 iLEVEL4_GAINH;
    MINT8 iLEVEL4_GAINL;
    MINT8 iLEVEL5_GAINH;
    MINT8 iLEVEL5_GAINL;
    MINT8 iLEVEL6_GAINH;
    MINT8 iLEVEL6_GAINL;
    MINT8 iGAIN_DIFFERENCE_LIMITER;
}strAELimiterTable;

/*******************************************************************************
* Dynamic Frame Rate for Video
******************************************************************************/
typedef struct VdoDynamicFrameRate_S
{
    MBOOL   isEnableDFps;
    MUINT32 EVThresNormal;
    MUINT32 EVThresNight;
} VdoDynamicFrameRate_T;

typedef struct
{
    MUINT8 uHist0StartBlockXRatio;       //Histogram 0 window config start block X ratio (0~100)
    MUINT8 uHist0EndBlockXRatio;         //Histogram 0 window config end block X ratio (0~100)
    MUINT8 uHist0StartBlockYRatio;       //Histogram 0 window config start block Y ratio (0~100)
    MUINT8 uHist0EndBlockYRatio;         //Histogram 0 window config end block Y ratio (0~100)
    MUINT8 uHist0OutputMode;               //Histogram 0 output source mode
    MUINT8 uHist0BinMode;                    //Histogram 0 bin mode range
    MUINT8 uHist1StartBlockXRatio;       //Histogram 1 window config start block X ratio (0~100)
    MUINT8 uHist1EndBlockXRatio;         //Histogram 1 window config end block X ratio (0~100)
    MUINT8 uHist1StartBlockYRatio;       //Histogram 1 window config start block Y ratio (0~100)
    MUINT8 uHist1EndBlockYRatio;         //Histogram 1 window config end block Y ratio (0~100)
    MUINT8 uHist1OutputMode;               //Histogram 1 output source mode
    MUINT8 uHist1BinMode;                    //Histogram 1 bin mode range
    MUINT8 uHist2StartBlockXRatio;       //Histogram 2 window config start block X ratio (0~100)
    MUINT8 uHist2EndBlockXRatio;         //Histogram 2 window config end block X ratio (0~100)
    MUINT8 uHist2StartBlockYRatio;       //Histogram 2 window config start block Y ratio (0~100)
    MUINT8 uHist2EndBlockYRatio;         //Histogram 2 window config end block Y ratio (0~100)
    MUINT8 uHist2OutputMode;               //Histogram 2 output source mode
    MUINT8 uHist2BinMode;                    //Histogram 2 bin mode range
    MUINT8 uHist3StartBlockXRatio;       //Histogram 3 window config start block X ratio (0~100)
    MUINT8 uHist3EndBlockXRatio;         //Histogram 3 window config end block X ratio (0~100)
    MUINT8 uHist3StartBlockYRatio;       //Histogram 3 window config start block Y ratio (0~100)
    MUINT8 uHist3EndBlockYRatio;         //Histogram 3 window config end block Y ratio (0~100)
    MUINT8 uHist3OutputMode;               //Histogram 3 output source mode
    MUINT8 uHist3BinMode;                      //Histogram 3 bin mode range
    MUINT8 uHist4StartBlockXRatio;       //Histogram 4 window config start block X ratio (0~100)
    MUINT8 uHist4EndBlockXRatio;         //Histogram 4 window config end block X ratio (0~100)
    MUINT8 uHist4StartBlockYRatio;       //Histogram 4 window config start block Y ratio (0~100)
    MUINT8 uHist4EndBlockYRatio;         //Histogram 4 window config end block Y ratio (0~100)
    MUINT8 uHist4OutputMode;               //Histogram 4 output source mode
    MUINT8 uHist4BinMode;                      //Histogram 4 bin mode range
    MUINT8 uHist5StartBlockXRatio;       //Histogram 5 window config start block X ratio (0~100)
    MUINT8 uHist5EndBlockXRatio;         //Histogram 5 window config end block X ratio (0~100)
    MUINT8 uHist5StartBlockYRatio;       //Histogram 5 window config start block Y ratio (0~100)
    MUINT8 uHist5EndBlockYRatio;         //Histogram 5 window config end block Y ratio (0~100)
    MUINT8 uHist5OutputMode;               //Histogram 5 output source mode
    MUINT8 uHist5BinMode;                      //Histogram 5 bin mode range
} AE_HIST_WIN_CFG_T;

typedef struct
{
    MUINT8 uHistStartBlockXRatio;       //Histogram 3 window config start block X ratio (0~100)
    MUINT8 uHistEndBlockXRatio;         //Histogram 3 window config end block X ratio (0~100)
    MUINT8 uHistStartBlockYRatio;       //Histogram 3 window config start block Y ratio (0~100)
    MUINT8 uHistEndBlockYRatio;         //Histogram 3 window config end block Y ratio (0~100)
} PS_HIST_WIN_CFG_T;

typedef struct {
    MBOOL   bFaceLocSizeCheck;
    MUINT32 u4Size_X[2];
    MUINT32 u4Size_Y[2];
    MUINT32 u4Loc_X[2];
    MUINT32 u4Loc_Y[2];

    MBOOL   bFaceOECheck;
    MUINT32 u4OE_X[2];
    MUINT32 u4OE_Y[2];
    MINT32  i4OE_BV_X[2];
    MUINT32 u4OE_BV_Y[2];
    MUINT32 u4FCY_DX[2];
    MUINT32 u4FCY_INDOOR_DX[2];
    MUINT32 u4FCY_BX[2];
    MUINT32 u4FCY_Y[2];

    MBOOL   bLandmarkSize;
} strFaceLocSize;

typedef struct {
    MINT32  i4DeltaEVDiff;
    MUINT32 u4WoAccRatio;// 65
    MUINT32 u4BoAccRatio;// 80
    MUINT32 u4PredY_H;// 240
    MUINT32 u4PredY_L;// 4
    MUINT32 u4AccY_Max;//254
    MUINT32 u4AccCWV_Max;// 510
    MUINT32 u4AccY_Min;// 1
    MBOOL   bRecycleEnable;
} strPerframeCFG;

/*
* TG AE speedup tuning parameters
*/
typedef struct
{

//B2T
    MUINT32 OverExpoThr; //245
    MUINT32 OverExpoRatio; //70%
    MUINT32 OverExpoResetAccEVTh; //200

    MUINT32 OverExpoSTD_Th[3];
    MUINT32 OverExpoSTD_Ratio[4];

    MUINT32 OverExpoAccDeltaEV_Th[3];
    MINT32 OverExpoAccDeltaEV_DeltaEV[4];

    MUINT32 OverExpoCountSTD_Th;

    MUINT32 OverExpoLowBound;
    MUINT32 OverExpLightAcc1;
    MUINT32 OverExpLightAcc2;
    MUINT32 OverExpLightAcc3;
    MUINT32 OverExpLightAcc4;

//D2T
    MUINT32 UnderExpoThr; //10
    MUINT32 UnderExpoRatio; //95%
    MUINT32 UnderExpoResetAccEVTh; //20

    MUINT32 UnderExpoSTD_Th[3];
    MUINT32 UnderExpoSTD_Ratio[4];

    MUINT32 UnderExpoAccDeltaEV_Th[3];
    MINT32 UnderExpoAccDeltaEV_DeltaEV[4];

    MUINT32 UnderExpoCountSTD_Th;

} strAETgTuningPara;

typedef struct {
    MUINT32 u4InStableThd;  // 0.08EV
    MUINT32 u4OutStableThd; // 0.08EV

    MBOOL   bEnableAEModeStableTHD;         // enable video ae stable threshold setting
    MUINT32 u4VideoInStableThd;               // video mode each index 0.08EV
    MUINT32 u4VideoOutStableThd;              // video mode each index 0.08EV
    MUINT32 u4FaceInStableThd;               // Face mode each index 0.08EV
    MUINT32 u4FaceOutStableThd;              // Face mode each index 0.08EV
    MUINT32 u4FaceOutB2TStableThd;              // Face mode each index 0.08EV
    MUINT32 u4FaceOutD2TStableThd;              // Face mode each index 0.08EV
    MUINT32 u4TouchInStableThd;               // Touch mode each index 0.08EV
    MUINT32 u4TouchOutStableThd;              // Touch mode each index 0.08EV
    MBOOL   bEnableFaceAeLock;               // Face Ae lock option
    MBOOL   bEnableZeroStableThd;            // enable zero stable thd
} strAEStableThd;

typedef struct {
    MBOOL u4BVAccEnable;
    MINT32 i4DeltaBVRatio;
    MUINT32 u4B2T_Target;
    strAEMovingRatio  pAEBVAccRatio;           // Preview ACC ratio
} strBVAccRatio;

typedef struct {
    MBOOL u4PsoAccEnable;
    MINT32 i4PsoEscRatioWO;  //PSO speedup ratio discount, 100 base (Wash-Out case)
    MINT32 i4PsoEscRatioNonLinear; //PSO speedup ratio discount, 100 base (Non-Linear Case)
    MINT32 ConvergeDeltaPosBound;
    MINT32 ConvergeDeltaNegBound;
}strPSOConverge;

typedef struct {
    MBOOL   bNonCWRAccEnable;
    MUINT32 u4HS_FHY_HBound;
    MUINT32 u4CWV_HBound;
    MUINT32 u4HS_TargetHBound;
    MUINT32 u4HS_ACCTarget;
} strNonCWRAcc;

typedef struct {
    MUINT32 u4MinFDYLowBound;               // face boundary min Y value
    MUINT32 u4MaxFDYHighBound;              // face boundary max Y value
    MINT32  i4FaceBVLowBound;               // face boundary min BV value
    MINT32  i4FaceBVHighBound;              // face boundary max BV value
    MUINT32 u4TmpFDY_HBound;                // tempFDY HighBound
    MUINT32 u4TmpFDY_LBound;                // tempFDY LowBound
    MUINT32 u4FD_Lost_MaxCnt;               // face lost max count
    MUINT32 u4FD_Scale_Ratio;               // face window scale ratio
    MUINT32 u4FaceNSLowBound;       //NS face target Y
    MUINT32 u4FD_Lock_MaxCnt;
    MUINT32 u4FD_TemporalSmooth;
////        HTLU:FACECROP_START       ////
    MUINT32 u4FD_FaceTopCropRat;
    MUINT32 u4FD_FaceBottomCropRat;
    MUINT32 u4FD_FaceLeftCropRat;
    MUINT32 u4FD_FaceRightCropRat;
////        HTLU:FACECROP_END       ////
    MUINT32 u4FD_InToOutThdMaxCnt;
    MUINT32 u4FD_OTLockUpperBnd;
    MUINT32 u4FD_OTLockLowerBnd;
    MUINT32 u4FD_ContinueTrustCnt;
    MUINT32 u4FD_PerframeAntiOverFlag;
    MUINT32 u4FD_SideFaceLock;
    MUINT32 u4FD_LimitStableThdLowBnd;
    MUINT32 u4FD_LimitStableThdLowBndNum;
    MUINT32 u4FD_ReConvergeWhenFaceChange;
    MUINT32 u4FD_FaceMotionLockRat;
    MUINT32 u4FD_ImpulseLockCnt;
    MUINT32 u4FD_ImpulseUpperThd;
    MUINT32 u4FD_ImpulseLowerThd;
    MUINT32 u4FD_ROPTrustCnt;
    MUINT32 u4FD_ReCovergeWhenSizeChangeRat;
    MUINT32 u4FD_ReCovergeWhenPosChangeDist;
////        HTLU:BVDIFF_START       ////
    MUINT32 u4FD_ReCovergeWhenRealBVx1000DiffLarge;
////        HTLU:BVDIFF_END       ////
    MUINT32 u4FD_ReCovergeWhenAFDone;
    MUINT32 u4FD_OTFaceLock;
////        HTLU:PORTRAIT_START       ////
    MBOOL   bCnnFaceEnable;
    MBOOL   bReConvergeWhenCNNFaceBack;
    MINT32  i4PortraitTolerance;
    MINT32  i4FD_CNN_Lock_MaxCnt;
////        HTLU:PORTRAIT_END       ////
////        HTLU:TOUCH_START       ////
    MBOOL   bEVCompExceptionEnable;
    MUINT32 u4EVCompTouchMeteringStableMax;
    MUINT32 u4EVCompTouchMeteringStableMin;
    MUINT32 u4EVCompTouchOutStableThd;
    MUINT32 u4EVCompTouchInStableThd;
////        HTLU:TOUCH_END       ////
////        HTLU:ACCGYRO_START      ////
    MBOOL   bFaceAccLockEnable;
    MUINT32 u4FaceAccLockThd;
    MUINT32 u4FaceAccLock_MaxCnt;
    MBOOL   bFaceGyroDiffLockEnable;
    MUINT32 u4FaceGyroDiffLockThd;
    MUINT32 u4FaceGyroDiffLock_MaxCnt;
////        HTLU:ACCGYRO_END       ////
}strFaceSmooth;

typedef enum
{
    FACE_PRIOR_TIME,
    FACE_PRIOR_SIZE,
    FACE_PRIOR_LIGHT,
    FACE_PRIOR_DARK,
    FACE_PRIOR_MAX
}FACE_PRIOR_ENUM;

typedef struct {
    MBOOL bEnableMultiFaceWeight;
    FACE_PRIOR_ENUM Prior;
    MUINT32 u4FaceWeight[MAX_AE_METER_AREAS];

}strFaceWeight;

typedef struct {
    MBOOL   bEnableAEHsTS;                   // enable AE HS Target temp smooth
    MBOOL   bEnableAEAblTS;                  // enable AE ABL Target temp smooth
    MBOOL   bEnableAEAoeTS;                  // enable AE AOE Target temp smooth
    MBOOL   bEnableAENsTS;                   // enable AE NS Target temp smooth
    MBOOL   bEnableAEFaceTS;                 // enable AE Face Target temp smooth
    MBOOL   bEnableAEVideoTS;                // enable AE Video Target temp smooth
    MBOOL   bEnableAEMainTargetTS;                // enable AE Main Target temp smooth
    MBOOL   bEnableAEHsV4p0TargetTS;                // enable AE HSV4p0 Target temp smooth
} strAECWRTempSmooth;

typedef struct {
    MBOOL   bLandmarkCtrlFlag;              // face landmark ctrol flag
    MUINT32 u4LandmarkCV_Thd;               // face landmark info cv thd
    MUINT32 u4LandmarkWeight;               // face landmark weight
    MINT32  i4LandmarkExtRatW;
    MINT32  i4LandmarkExtRatH;
    MINT32  i4LandmarkTrustRopDegree;
    MINT32  i4RoundXYPercent;
    MINT32  i4RoundSZPercent;
    MINT32  i4THOverlap;
    MINT32  i4BUFLEN; //must not exceed #LANDMARK_SMOOTH_MAX_LENGTH
    MINT32  i4SMOOTH_LEVEL;
    MINT32  i4MOMENTUM;
    MINT32  u4LandmarkWeightPercent;
}strFaceLandMarkCtrl;

typedef struct
{
    MBOOL   bEnable;                        //enable adaptive AOE THD
    MUINT32 u4TblLength;
    MINT32 i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4THD[AE_RATIOTBL_MAXSIZE];
} strAOEBVRef;

typedef struct
{
    MBOOL   bEnable;            //enable NS CDF
    MUINT32  u4Pcent;            // 1=0.1%, 0~1000
    MINT32 i4NS_CDFRatio_X[2];
    MUINT32 u4NS_CDFRatio_Y[2];
} strNS_CDF;

typedef struct {
    MBOOL  bEnableFlatSkyEnhance;                                // enable HS adaptive THD by realBV
    MUINT8  uSkyDetInThd;
    MUINT8  uSkyDetOutThd;
    MUINT32 u4FlatSkyTHD;
    MUINT32 u4FlatSkyEVD;
    MINT32 u4BVRatio_X[2];
    MINT32 u4BVRatio_Y[2];
} strHSFlatSkyCFG;

typedef struct {
    MBOOL bEnableABLabsDiff;                                // enable enhance ABL(absDiff)
    MINT32  u4EVDiffRatio_X[2];
    MINT32  u4EVDiffRatio_Y[2];
    MINT32  u4BVRatio_X[2];
    MINT32  u4BVRatio_Y[2];
} strABL_absDiff;

typedef struct
{
    MBOOL   bEnableMultiStepHS;         //Enable HS enhance method : MultiStep HS(Conflict with ori HS)
    MUINT32 u4TblLength;                                //Max size : 16
    MUINT32 u4MS_EVDiff[AE_RATIOTBL_MAXSIZE];            //size must large than u4TblLength
    MUINT32 u4MS_OutdoorTHD[AE_RATIOTBL_MAXSIZE];        //size must large than u4TblLength
    MUINT32 u4MS_IndoorTHD[AE_RATIOTBL_MAXSIZE];         //size must large than u4TblLength
    MINT32    i4MS_BVRatio[2];                      //MS_HS In/Out door BV threshold
} strHSMultiStep;

typedef struct {
    MBOOL bEnableStablebyHist;
    MUINT32 u4HistEVDiff;
    MUINT32 u4OverexpoTHD;
    MUINT32 u4UnderexpoTHD;
    MUINT32 u4HistStableTHD;
} strHistStableCFG;

typedef struct {
    MBOOL bEnableOverExpoAOE;
    MBOOL bEnableHistOverExpoCnt;
    MUINT32 u4OElevel;
    MUINT32 u4OERatio_LowBnd;
    MUINT32 u4OERatio_HighBnd;
    MINT32 i4OE_BVRatio_X[2];
    MUINT32 u4OE_BVRatio_Y[2];
    MUINT8 u4OEWeight[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];
} strOverExpoAOECFG;

typedef struct {
    MBOOL bEnableAEMainTarget;
    MUINT32 u4MainTargetWeight;     //weight 1024 base
    MUINT8 u4MainTargetWeightTbl[AE_V4P0_BLOCK_NO][AE_V4P0_BLOCK_NO];  //15x15 Gau weight table
    AE_TARGET_PROB_T TargetBVRatioTbl;
    MBOOL bEnableColorWTRatio;
    AE_TARGET_PROB_T ColorWTRatioTbl;
    MUINT8 uColorSupRatioR;
    MUINT8 uColorSupRatioG;
    MUINT8 uColorSupRatioB;
    AE_TARGET_PROB_T ColorSupBVRatioTbl;
} strMainTargetCFG;

typedef struct {
    MBOOL bEnableHSv4p0;
    MUINT32 u4HSv4p0Weight;     //weight 1024 base
    MUINT32 u4BVSize;      //max size 16
    MUINT32 u4EVDSize;   //max size 16
    MINT32 i4HS_BVRatio[AE_RATIOTBL_MAXSIZE];            //Ratio[uBVSize]
    MUINT32 u4HS_PcntRatio[AE_RATIOTBL_MAXSIZE];     //Ratio[uBVSize]
    MUINT32 u4HS_EVDRatio[AE_RATIOTBL_MAXSIZE];     //Ratio[u4EVDSize]
    MUINT32 u4HSTHDRatioTbl[AE_RATIOTBL_MAXSIZE][AE_RATIOTBL_MAXSIZE]; //RatioTbl[uBVSize][uEVDSize]
    MBOOL bEnableDynWTRatio;
    AE_TARGET_PROB_T DynWTRatioTbl;


    MBOOL bEnableHsGreenSupress;    // Enable flag  for HS green supress
    MUINT32 u4HsGreenGRRatio;       //  HS green G/R Ratio
    MUINT32 u4HsGreenGBRatio;       //  HS green G/B Ratio
    MUINT32 u4HsSupRatio;           // HS green surpess stregth ratio
    MUINT32 u4HS_GCountRatio_X[2];  // HS green count weighting TBL X
    MUINT32 u4HS_GCountRatio_Y[2];  // HS green count weighting TBL Y

} strHSv4p0CFG;

typedef struct{
    MBOOL   bEnableNSBVCFG;
    MUINT32 u4TblLength;
    MINT32  i4BV[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4BTTHD[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4NSTHD[AE_RATIOTBL_MAXSIZE];
} strNSBVCFG;

/*
*   HDR Application
*/
#define SUPPORTED_MAX_HDR_EXPNUM            (4)
#define HDR_AE_LV_TBL_NUM                   (19)


// HDR Application - Information

typedef struct
{
    unsigned long long  u8TotalHistCount;
    unsigned long long  u8EqvHdrProb;
    unsigned long long  u8DarkHdrProb;
    unsigned long long  u8BrightHdrProb;
    MINT32              i4BrightEnhanceProb;
    MINT32              i4FDBrightToneProb;
    MINT32              i4HdrContrastL;
    MINT32              i4HdrContrastH;
    MINT32              i4HdrContrastEVDiff;
    MINT32              i4NightEVDark;
    MINT32              i4NightEVBright;
    MINT32              i4NightEVDiff;
    MINT32              i4NightEVDiffProb;
    MINT32              i4NightHistY;
    MINT32              i4NightHistProb;
    MINT32              i4NightLVProb;
    MINT32              i4NightProb;
    MINT32              i4COEDiffProb;
    MINT32              i4COEDiffWeightProb;
    MINT32              i4COEOuterProb;
    MINT32              i4COEOuterWeightProb;
    MINT32              i4ABLDiffProb;
    MINT32              i4LVProb;
    MINT32              i4TotalProb;
    MINT32              i4FinalProb;
    MINT32              i4HdrOnOff;
    MINT32              i4MotionProb;
    MINT32              i4MotionOnOff;
    MINT32              i4RMGtCurrY;
    MINT32              i4cHdrLECurrY;
    MINT32              i4cHdrSECurrY;
} HDR_DETECTOR_INFO_T;

typedef struct
{
    MINT32                      i4Acce[3];
    MINT32                      i4Gyro[3];
} HDR_GGYRO_INFO_T;


typedef struct
{
    MINT32  i4AutoRMGk;
    MINT32  i4PvRMGt;
    MINT32  i4CurrRMGt;
    MINT32  i4MeanRMGt;
    MINT32  i4AutoRMGt;
    MINT32  i4AutoLVRatio;
    MINT32  i4AutoContrastRatio;
    MINT32  i4PvBrightRatio;
    MINT32  i4CurrBrightRatio;
    MINT32  i4MeanBrightRatio;
    MINT32  i4AutoBrightRatio;
    MINT32  i4AutoNSRatio;
    MINT32  i4AutoISORatio;
    MINT32  i4AutoGyroRatio;
    MINT32  i4AutoAlgRatio;
    MINT32  i4AutoPvAlgoRatio;
    HDR_GGYRO_INFO_T rGGyroInfo;
} HDR_AUTO_ENHANCE_INFO_T;

typedef struct {
	MINT32 GTM_X0; 			// 12b
	MINT32 GTM_Y0;			// 16b
	MINT32 GTM_S0;			// 16.4b
	MINT32 GTM_S1;			// 16.4b
} HDR_DCPN_INFO_T;

typedef struct {
	MINT32 GTM_X[7];		// 12b
	MINT32 GTM_Y[7];	    // 12b
	MINT32 GTM_S[8];		// 4.8b
} HDR_CPN_INFO_T;


typedef struct {
	HDR_DCPN_INFO_T 			rHdrDCPNProp;
	HDR_CPN_INFO_T				rHdrCPNProp;
} HDR_TONE_INFO_T;

typedef struct {
    MINT32 i4NormalAETarget;
    MINT32 i4HdrNormalAETarget;
    MINT32 i4AutoEVWeight;
    MINT32 i4PvBrightWeight;
    MINT32 i4CurrBrightWeight;
    MINT32 i4AutoBrightWeight;
    MINT32 i4AutoABLWeight;
    MINT32 i4AutoLVWeightL;
    MINT32 i4AutoLVWeightH;
    MINT32 i4AutoNightWeight;
    MINT32 i4AutoWeight;
    MINT32 i4CurrAoeY;
    MINT32 i4TargetAoeY;
    MINT32 i4TargetAoeGain;
    MINT32 i4CurrNSY;
    MINT32 i4TargetNSY;
    MINT32 i4TargetNSGain;
    MINT32 i4TargetGain;
    MINT32 i4HdrTargetY;
    MINT32 i4MinTargetY;
    MINT32 i4MaxTargetY;
    MUINT32 u4AutoTarget;
    MINT32  i4AEBlendingTarget;
    MINT32  i4FDTarget;
    MINT32  i4FDBlendingTarget;
    MINT32  i4HdrFinalTarget;
} HDR_AE_TARGET_INFO_T;

typedef struct {
    MINT32  i4GainBase;     /*  Video HDR 0 EV info.  */
    MINT32  i4SEInfo;       /*  Delta EV compared with i4GaniBase  */
    MINT32  i4LEInfo;       /*  Low averege data  */
} HDR_CHDR_INFO_T;

typedef struct {
    /*
    *   HDR detector smooth info
    */
    MINT32  i4PvHdrOnOff;
    MINT32  i4PvHdrStableOnOff;
    MINT32  i4HdrOnOffCounter;
    MINT32  i4HdrStableOnOff;
    /*
    *   HDR weighting smooth info
    */
    MINT32  i4AutoWeightStableCount;
    /*
    *   Motion smooth info
    */
    MINT32  i4MotionOnCount;
    MINT32  i4PvMotionOnOff;
    MINT32  i4PvMotionStableOnOff;
    MINT32  i4MotionStableOnOff;

    /*
    *   RMG smooth info
    */
    MINT32 i4RMGtStableCount;

    /*
    *   Ratio smooth info
    */
    MINT32  i4SmoothTrigger;
    MINT32  i4RatioStableCount;
    MINT32  i4PvHdrRatio;
    MINT32  i4PvHdrStableRatio;
    MINT32  i4HdrRatioCount;
    MINT32  i4MeanHdrRatio;
    MINT32  i4HdrStableRatio;
    MINT32  i4HdrRatioStep;
    MINT32  i4PvHdrSmoothRatio;
    MINT32  i4HdrSmoothRatio;
    MINT32  i4MediumHdrRatio;
} HDR_SMOOTH_INFO_T;

typedef struct {
    MINT32  i4LEExpo;
    MINT32  i4LEAfeGain;
    MINT32  i4LEIspGain;
    MINT32  i4MEExpo;
    MINT32  i4MEAfeGain;
    MINT32  i4MEIspGain;
    MINT32  i4SEExpo;
    MINT32  i4SEAfeGain;
    MINT32  i4SEIspGain;
    MINT32  i4VSEExpo;
    MINT32  i4VSEAfeGain;
    MINT32  i4VSEIspGain;
    MINT32  i4HdrRatio;
} HDR_EV_INFO_T;

typedef enum
{
    MVHDR_STATE_MOVING = 0,
    MVHDR_STATE_STABLE,
}eMVHDR_STATE;

typedef struct {
    MUINT32 u4targetHDRRatio;
    MUINT32 u4curHDRRatio;
    MUINT32 u4preHDRRatio;
    MUINT32 u4realStatHDRRatio;
    MUINT32 u4mVHDRTarget;
    MUINT32 u4TargetWeight;
    eMVHDR_STATE eHDRState;
} HDR_3EXPO_INFO_T;
typedef struct
{
    MINT32                      i4AEMode;
    MINT32                      i4LV;
    HDR_DETECTOR_INFO_T         rDetectorInfo;
    HDR_AUTO_ENHANCE_INFO_T     rAutoEnhanceInfo;
    HDR_AE_TARGET_INFO_T        rHdrAETargetInfo;
    HDR_CHDR_INFO_T             rcHdrInfo;
    HDR_SMOOTH_INFO_T           rHdrSmoothInfo;
    HDR_EV_INFO_T               rHdrEvInfo;
    HDR_TONE_INFO_T             rHdrToneInfo;
    HDR_3EXPO_INFO_T            rHdr3ExpoInfo;
} HDR_AE_INFO_T;

// HDR Application - Parameters

typedef struct {
    MINT32              i4HdrHistP0, i4HdrHistP1;
    MINT32              i4HdrHistW0, i4HdrHistW1;
} HDR_WEIGHTING_WIN_T;

typedef struct
{
    MBOOL               bHistDetectEnable;
    HDR_WEIGHTING_WIN_T rHdrDetectDarkWtWin;
    HDR_WEIGHTING_WIN_T rHdrDetectBrightWtWin;
    HDR_WEIGHTING_WIN_T rHdrEnhanceBrightWtWin;
    HDR_WEIGHTING_WIN_T rHdrFDBrightWtWin;
    MINT32              i4HdrContrastPercentH;
    MINT32              i4HdrContrastPercentL;
    MINT32              i4HdrNightEVDiffPercentH;
    MINT32              i4HdrNightEVDiffPercentL;
    MINT32              i4HdrNightDarkPercent;
    AE_TARGET_PROB_T    rHdrNightEVDiffProb;
    AE_TARGET_PROB_T    rHdrNightHistProb;
    AE_TARGET_PROB_T    rHdrNightLVProb;
} HDR_DETECT_HIST_T;

typedef struct
{
    MBOOL               bGeometryCOEEnable;
    MINT32              i4COEWeight;                // x1 = 1024
    AE_TARGET_PROB_T    rHdrCOEOuterRatio;          // the outer y ratio
    AE_TARGET_PROB_T    rHdrCOEDiffRatio;           // inner/outer y difference ratio
} HDR_GEOMETRY_COE_T;

typedef struct
{
    MBOOL                bGeometryABLEnable;
    MINT32              i4ABLWeight;                // x1 = 1024
    AE_TARGET_PROB_T    rCenterBrightDiffRatio;     // C-type / U-type y difference ratio
    AE_TARGET_PROB_T    rCenterDarkDiffRatio;       // C-type / U-type y difference ratio
} HDR_GEOMETRY_ABL_T;

typedef struct
{
    HDR_GEOMETRY_COE_T rGeometryCOEProp;
    HDR_GEOMETRY_ABL_T rGeometryABLProp;
} HDR_DETECT_GEOMETRY_T;

typedef struct
{
    MBOOL    bLVDetectEnable;
    MINT32  i4LVProbTbl[HDR_AE_LV_TBL_NUM];
} HDR_DETECT_LV_T;

typedef struct
{
    MBOOL                    bHdrDetectorEnable;
    MINT32                  i4HdrConfidenceThr;
    HDR_DETECT_HIST_T       rHdrHistInfo;
    HDR_DETECT_GEOMETRY_T   rHdrGeometryInfo;
    HDR_DETECT_LV_T         rHdrLVInfo;
} HDR_DETECTOR_T;

typedef struct
{
    MINT32                  i4NightPercent;
    AE_TARGET_PROB_T        rNightTarget;
} HDR_NIGHT_TARGET_T;

typedef struct
{
    MINT32                  i4AoePercent;
    MINT32                  i4AoeLV[3];
    MINT32                  i4AoeTarget[3];
} HDR_AOE_TARGET_T;

typedef struct {
    MINT32                  i4LVLimit;
    AE_TARGET_PROB_T        rTargetMinLimit;
    AE_TARGET_PROB_T        rTargetMaxLimit;
} HDR_TARGET_LIMIT_T;

typedef struct
{
    MINT32                  i4DefTarget;
    MINT32                  i4DefWeight;
    AE_TARGET_PROB_T        rAutoEVWeight;
    AE_TARGET_PROB_T        rAutoBrightWeight;
    AE_TARGET_PROB_T        rAutoABLWeight;
    AE_TARGET_PROB_T        rAutoLVWeightL;
    AE_TARGET_PROB_T        rAutoLVWeightH;
    AE_TARGET_PROB_T        rAutoNightWeight;
    HDR_AOE_TARGET_T        rHdrAoeTarget;
    HDR_NIGHT_TARGET_T      rHdrNightTarget;
    HDR_TARGET_LIMIT_T      rTargetLimitL;
    HDR_TARGET_LIMIT_T      rTargetLimitM;
    HDR_TARGET_LIMIT_T      rTargetLimitH;
} HDR_AE_TARGET_T;

typedef struct {
    AE_TARGET_PROB_T        rFaceEVProb;
    AE_TARGET_PROB_T        rFaceHdrProb;
} HDR_FACE_T;

typedef struct {
    HDR_FACE_T              rFaceAEProp;
} HDR_MISC_PROP_T;

typedef struct
{
    AE_TARGET_PROB_T        rAcceProb;
    AE_TARGET_PROB_T        rGyroProb;
    AE_TARGET_PROB_T        rMotionRatio;
    MINT32                  i4MotionOnThr;
    MINT32                  i4MotionOnCntThr;

} HDR_G_GYRO_SENSOR_T;


typedef struct
{
    MINT32              i4LVRatio50[HDR_AE_LV_TBL_NUM];
    MINT32              i4LVRatio60[HDR_AE_LV_TBL_NUM];
    AE_TARGET_PROB_T    rContrastRatio;
    AE_TARGET_PROB_T    rBrightRatio;
    AE_TARGET_PROB_T    rNightProbRatio;
    HDR_G_GYRO_SENSOR_T rGGyroSensor;
} AUTO_BRIGHT_ENHANCE_T;

typedef struct
{
    MINT32  i4RmmExpoDiffThr;           //  %
    MINT32  i4LscRatio;                 //
} AUTO_TUNING_T;


typedef struct
{
    MBOOL    bHdrWaitAEStable;
    MINT32  i4HdrInfoCounterThr;
    MBOOL    bRatioSmoothEnable;
    MINT32  i4AutoWeightMode;           // 0: Max, 1:Min, 2:Mean
    MINT32  i4AutoWeightSmooth;
    MINT32  i4AutoWeightStableThr;
    MINT32  i4RMGtStableMode;           // 0: Max, 1:Min, 2:Mean
    MINT32  i4RMGtStableThr;
    MINT32  i4HdrRatioStableMode;       // 0: Max, 1:Min, 2:Mean
    MINT32  i4HdrRatioStableThr;
    MINT32  i4RatioSmoothThr[2];        //  [0]:normal, [1]:motion
    MINT32  i4RoughRatioUnit[2];        //  [0]:normal, [1]:motion
    MINT32  i4FinerRatioUnit[2];        //  [0]:normal, [1]:motion
    MINT32  i4RMGtUnit[2];              //  [0]:normal, [1]:motion
} AUTO_SMOOTH_T;

typedef struct
{
    MINT32 i4Flkr1LVThr[2];         // [0]: 50 Hz,  [1]: 60 Hz
    MINT32 i4MinRatioAntiFlk;
    MINT32 i4MaxExp[2];
    MINT32 i4MinExp[2];
    AE_TARGET_PROB_T rIsoThr;
} AUTO_FLICKER_RATIO_T;

typedef struct
{
    MBOOL                    bAutoEnhanceEnable;
    MINT32                  i4RMGk;
    MINT32                  i4RMGtRatio;
    AE_TARGET_PROB_T        rRMGt;
    MINT32                  i4SupportHdrNum;
    MINT32                  i4AutoEnhanceProb;
    MINT32                  i4HdrWeight;
    AUTO_BRIGHT_ENHANCE_T   rAutoBright;
    AUTO_FLICKER_RATIO_T    rAutoFlicker;
    AUTO_SMOOTH_T           rAutoSmooth;
    AUTO_TUNING_T           rAutoTuning;
    MINT32                  i4FlashRatio;
} HDR_AUTO_ENHANCE_T;

typedef struct
{
    MINT32                  i4LEThr;
    MINT32                  i4SERatio;
    MINT32                  i4SETarget;
    MINT32                  i4BaseGain;
} HDR_CHDR_T;

typedef enum {
    eMainHDR_OFF = 0,
    eMainHDR_AUTO,
    eMainHDR_ON,
    eMainHDR_VT,
    eMain2HDR_OFF,
    eMain2HDR_AUTO,
    eMain2HDR_ON,
    eMain2HDR_VT,
    eSubHDR_OFF,
    eSubHDR_AUTO,
    eSubHDR_ON,
    eSubHDR_VT,
    eSub2HDR_OFF,
    eSub2HDR_AUTO,
    eSub2HDR_ON,
    eSub2HDR_VT
}eHDRPARAM_ID;

typedef struct
{
    eHDRPARAM_ID            eHdrParamID;
    HDR_DETECTOR_T          rHdrDetector;
    HDR_AUTO_ENHANCE_T      rHdrAutoEnhance;
    HDR_AE_TARGET_T         rHdrAETarget;
    HDR_MISC_PROP_T         rHdrMiscProp;
    HDR_CHDR_T              rHdrcHdr;
} HDR_AE_PARAM_T;

typedef struct {
    MUINT32 u4FlareSmallDeltaIdxStep;
    MUINT32 u4CWVChangeStep;
    MUINT32 u4AllStableStep;
    MBOOL  bOutdoorFlareEnable;
    MUINT32 u4OutdoorMaxFlareThres;
    MINT32  i4BVRatio[2];
} strDynamicFlare;

typedef enum
{
    SYNC_AE_DUAL_CAM_DENOISE_BMDN = 0,
    SYNC_AE_DUAL_CAM_DENOISE_MFNR,
    SYNC_AE_DUAL_CAM_DENOISE_MAX
} SYNC_AE_DUAL_CAM_DENOISE_ENUM;

typedef struct {
    MINT32 i4RMGt;
    MINT32 i4RMGk;
    HDR_EV_INFO_T rHdrEVInfo;
    MINT32 i4HdrOnOff;
    MINT32 i4HdrConfidence;
    MINT32 i4BrightConfidence;
    MINT32 i4DarkConfidence;
    MINT32 i4cHdrGainBase;                                  /*  Video HDR 0 EV info.  */
    MINT32 i4cHdrSEInfo;                                    /*  Delta EV compared with i4GaniBase  */
    MINT32 i4cHdrLEInfo;                                    /*  Low averege data  */
    MINT32 i4SwitchHLR;
    //3ExpoHDRratio
    MUINT32 u4mVHdrRealRatio;
    MUINT32 u4veLTCRate;
    MUINT32 u4veLimitGain;
} strEvHdrSetting;

/* 3EXP HDR Start
*/
typedef struct {
    // Conditions for HDR Maximum Ratio Control
    MINT32  i4MaxHDRRatio;        // 6400 stands for 64x [LE/SE]
    MINT32  i4MaxHDRRatio_Face;   // 6400 stands for 64x [LE/SE]
    MUINT32 u4TblLength;         // max is 8
    MINT32  i4BV4MaxHDRRatio[AE_RATIOTBL_MAXSIZE]; // | -1000 | 1000 | 3500 | 8000 |
    MUINT32 u4MaxHDRRatio_BV[AE_RATIOTBL_MAXSIZE]; // |  100x | 200x | 400x | 800x |
} strMaxHdrRatioCFG;

typedef struct {
    // for HDR Ratio Converge
    MINT32 i4HDRRatioInTolerance;  // 0.15*100
    MINT32 i4HDRRatioOutTolerance;  // 0.30*100
    MINT32 i4HDRStableCountTHD;
} strHdrRatioConvergeCFG;

typedef struct {
    // Method1 : HL Avg Control
    BOOL bEnableHLAvgRatioControl;    // Enable HLAvg Control
    MUINT32 u4HDRHLRatio;             // 20 means 20%
    MUINT32 u4HDRHLTarget;            // 100 for 256

    // Method2 : OE Hist Control
    BOOL bEnableHistOERatioControl;              // Enable HistOE Control
    MUINT32 u4TblLength;                         // max is 8
    MINT32  i4OEPct[AE_RATIOTBL_MAXSIZE];        // |    0  |    5  |   10  |   20  |   30  |
    MINT32  i4RatioMovOE1[AE_RATIOTBL_MAXSIZE];  // |    0  |    0  |    0  |  100  |  150  |
    MINT32  i4RatioMovOE2[AE_RATIOTBL_MAXSIZE];  // | -100  |    0  |    0  |    0  |    0  |
    MUINT32 u4OEBin[2];                          // Last 2Bin Position[1 | 129]

} strHdrRatioTargetCFG;

typedef struct {
    // Conditions for Ratio Decrease : BV & OE Ratio
    MBOOL bEnableBVDecrease;                               // Enable BV Decrease
    MUINT32 u4BVTblLength;                                // max is 8
    MINT32  i4BV4HDRRatioDecrease[AE_RATIOTBL_MAXSIZE];   // | -1000 | 1000 | 3500 | 8000 |
    MUINT32 u4RatioDecreaseRate4BV[AE_RATIOTBL_MAXSIZE];  // |  50%  | 80%  | 100% | 100% |

    // Calculate (OE_10bins/ALL_256bins) Ratio
    MBOOL bEnableOEDecrease;                               // Enable OE Decrease
    MUINT32 u4OEBinN;                                     // OE : 255~(255-OEBinN)
    MUINT32 u4OETblLength;                                // max is 8
    MUINT32 u4OERate4HDRDecrease[AE_RATIOTBL_MAXSIZE];    // | 10% | 20% | 30% | 40% |
    MUINT32 u4RatioDecreaseRate4OE[AE_RATIOTBL_MAXSIZE];  // | 80% | 80% | 100%| 100%|
} strHdrRatioDecCFG;

typedef struct {
    strMaxHdrRatioCFG HdrMaxRatioInfo;

    // for Ratio Moving Start
    strHdrRatioConvergeCFG HdrRatioConvergeInfo;

    // HDR Ratio Control
    strHdrRatioTargetCFG HdrRatioTargetInfo;
    strHdrRatioDecCFG HdrRatioDecreaseInfo;
} strHdrRatioCFG;

typedef struct {
    // BV vs Target Table
    MUINT32 u4TblLength;                         // max is 8
    MINT32  i4BV4HDRTarget[AE_RATIOTBL_MAXSIZE]; // | -1000 | 1000 | 3500 | 8000 |
    MUINT32 u4HDRTarget[AE_RATIOTBL_MAXSIZE];    // |  100x | 200x | 400x | 800x |
} strMvhdrTargetCFG;

typedef struct {
    // HDRRatio vs FusionRatio Table
    MUINT32 u4TblLength;                                   // max is 8
    MUINT32 i4HDRRatio4TargetFusion[AE_RATIOTBL_MAXSIZE];  // | 150 | 200 |
    MUINT32 u4HDRTargetFusionRate[AE_RATIOTBL_MAXSIZE];    // |  0% | 100%| [Sensor Portion]
} strMvhdrTargetFusionCFG;

typedef struct {
    /* Other Target Decision Used Parameters.
    */
    strMvhdrTargetCFG HdrTargetInfo;
    strMvhdrTargetFusionCFG FusionInfo;
} strHdrTargetControlCFG;

typedef struct{
    MBOOL   bEnable;
    MUINT32 u4TblLength;
    MUINT32 u4RatioPcent_X[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4RatioPcent_Y[AE_RATIOTBL_MAXSIZE];
} str3ExpomVHdrRatioSmoothTbl;

typedef struct{
    MBOOL   bEnable;
    MUINT32 u4TblLength;
    MUINT32 u4RatioLimit_X[AE_RATIOTBL_MAXSIZE];
    MUINT32 u4RatioLimit_Y[AE_RATIOTBL_MAXSIZE];
} str3ExpomVHdrRatioLimitTbl;


typedef struct{
    str3ExpomVHdrRatioSmoothTbl  SmoothRatioTbl;
    str3ExpomVHdrRatioLimitTbl LimitRatioTbl;
} str3ExpomVHdrCFG;

typedef struct{
    MUINT32 u4B2DLinearThd;
    MINT32 i4B2DBVACCExtreamBound;
    MINT32 i4B2DBVACCNonlinearBound;
    MINT32 i4B2DNonlinearEVDBound;
    MINT32 i4B2DLinearEVDBound;
    
    MUINT32 u4D2BLinearThd;
    MINT32 i4D2BNonlinearEVDBound;
    MINT32 i4D2BLinearEVDBound;
} strHdrSmoothCFG;

/* 3EXP HDR end
*/

typedef struct
{
    MBOOL           bAEStable;      // Only used in Preview/Movie
    MBOOL           bGammaEnable;   // gamma enable
    strEvSetting    EvSetting;
    strEvHdrSetting HdrEvSetting;
    MUINT32        u4CWValue;
    MUINT32        u4AvgY;
    MUINT32        u4FracGain;    // stepless gain, lower than 0.1EV
    MINT32         Bv;
    MINT32         AoeCompBv;
    MINT32         i4EV;
    MUINT32        u4AECondition;
    MINT32         i4DeltaBV;
    MINT32         i4PreDeltaBV;
    MUINT32        u4DeltaBVRatio[SYNC_AE_DUAL_CAM_DENOISE_MAX];
    MUINT32        u4ISO;          //correspoing ISO , only use in capture
    MUINT16        u2FrameRate;     // Calculate the frame
    MINT16        i2FlareOffset;
    MINT16        i2FlareGain;   // in 512 domain
    MINT16        i2FaceDiffIndex;
    MINT32        i4AEidxCurrent;  // current AE idx
    MINT32        i4AEidxCurrentF;  // current AE idx
    MINT32        i4AEidxNext;   // next AE idx
    MINT32        i4AEidxNextF;   // next AE idx
    MINT32        i4gammaidx;   // next gamma idx
    MINT32        i4LESE_Ratio;    // LE/SE ratio
    MINT32        i4cHdrSEDeltaEV;      //for sw HDR SE ,  -(x*100) EV , compare with converge AE
} strAEOutput;


typedef struct {
    MBOOL bEnableWorkaround;    // MTRUE : enable, MFALSE : disable
    MUINT32 u4SEExpTimeInUS;     // unit: us short exposure
    MUINT32 u4SESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4SEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LEExpTimeInUS;     // unit: us long exposure
    MUINT32 u4LESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4LEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LE_SERatio_x100;   // 100x
    strEvHdrSetting  HdrEvSetting;
} rVHDRExpSettingInfo_T;


typedef struct
{
    MUINT32 EVValue[81];
} strEVC;

struct AE_PARAMETER
{
    strAEParamCFG strAEParasetting;
    strWeightTable Weighting[MAX_WEIGHT_TABLE];   //AE WEIGHTING TABLE
    strAFPlineInfo strAFPLine;
    strAFPlineInfo strAFZSDPLine;
    strAFPlineInfo strStrobePLine;
    strAFPlineInfo strStrobeZSDPLine;
    strEVC EVValueArray;
    strAEMoveTable AEMovingTable;
    strAEMoveTable AEVideoMovingTable;
    strAEMoveTable AEFaceMovingTable;
    strAEMoveTable AETrackingMovingTable;

    strAELimiterTable strAELimiterData;
    VdoDynamicFrameRate_T strVdoDFps;

    AE_HIST_WIN_CFG_T AEBlockWINCFG;
    AE_HIST_WIN_CFG_T AEPixelWINCFG;
    PS_HIST_WIN_CFG_T PSPixelWINCFG;
    // v1.2
    MBOOL   bOldAESmooth;                          // Select the new or old AE smooth control
    MBOOL   bEnableSubPreIndex;                // decide the sub camera re-initial index after come back to camera
    MUINT32 u4VideoLPFWeight; // 0~24

    strFaceLocSize FaceLocSizecheck;
    strPerframeCFG PerframeCFG;
    MBOOL   bPerFrameAESmooth;                    // Perframe AE smooth option
    MBOOL   bPerFrameHDRAESmooth;                    // Perframe AE smooth option

    // TG interrupt
    MBOOL   bTgIntAEEn;                    // TG interrupt option
    MBOOL   bSpeedupEscOverExp;
    MBOOL   bSpeedupEscUnderExp;
    strAETgTuningPara AETgTuningParam;
    strEVC TgEVCompJumpRatioOverExp;
    strEVC TgEVCompJumpRatioUnderExp;
    MUINT32 pTgWeightTbl[9][12];

    //Open AE
    strAEStableThd AEStableThd;
    strBVAccRatio   BVAccRatio;              // Preview ACC ratio
    strPSOConverge PSOConverge;
    strNonCWRAcc    NonCWRAcc;
    strFaceSmooth  FaceSmooth;
    strFaceWeight  FaceWeight;
    strAECWRTempSmooth  AETempSmooth;
    strFaceLandMarkCtrl FaceLandmark;
    strAOEBVRef     AOERefBV;
    strNS_CDF       NsCdfRatio;

    //v2.5
    strAEMovingRatio AETouchMovingRatio;   //(temp) Touch smooth ratio
    strHSFlatSkyCFG     HSFlatSkyEnhance; //HS adaptive Prob Threshold by real BV
    strABL_absDiff      ABLabsDiff;
    //v3.0
    strHSMultiStep    HSMultiStep;
    strHistStableCFG  HistStableCFG;
    strOverExpoAOECFG OverExpoAOECFG;
    //v4.0
    MBOOL bAEv4p0MeterEnable;
    strMainTargetCFG MainTargetCFG;
    strHSv4p0CFG HSv4p0CFG;
    strNSBVCFG NSBVCFG;

    //v4.1
    strAEMovingRatio AEACCMovingRatio;   //(temp) acc smooth ratio
    // HDR
    HDR_AE_PARAM_T    HdrAECFG;

    // Stereo Pline mapping
    strAESceneMapping strStereoPlineMapping;
    // HDR Pline mapping
    strAESceneMapping strHDRPlineMapping;

    //Dynamic Flare
    strDynamicFlare DynamicFlareCFG;

};

typedef struct AE_PARAMETER AE_PARAM_T;

typedef struct
{
    strHdrTargetControlCFG *pHdrTargetControl;
    strHdrRatioCFG *pHdrRatioControl;
    str3ExpomVHdrCFG *p3ExpomVHdrCFG;
    strHdrSmoothCFG *pHdrSmoothCFG;
}AE_PARAM_TEMP_T;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
    MUINT32 u4TopProtect;
    AE_DEVICES_INFO_T rDevicesInfo;
    AE_HIST_CFG_T rHistConfig;
    AE_CCT_CFG_T rCCTConfig;
    HDR_AE_CFG_T rHDRAEConfig;
    AE_PARAM_T rAeParam;
    MUINT32 u4BottomProtect;
} AE_NVRAM_T;

#endif