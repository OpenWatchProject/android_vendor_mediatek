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
#ifndef _CAMERA_CUSTOM_3A_NVRAM_H_
#define _CAMERA_CUSTOM_3A_NVRAM_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "camera_custom_AEPlinetable.h"
#include "CFG_Camera_File_Max_Size.h"


/*******************************************************************************
* 3A
********************************************************************************/
typedef struct
{
    UINT32 u4MinGain;
    UINT32 u4MaxGain;
    UINT32 u4MiniISOGain;
    UINT32 u4GainStepUnit;
    UINT32 u4PreExpUnit;
    UINT32 u4PreMaxFrameRate;
    UINT32 u4VideoExpUnit;
    UINT32 u4VideoMaxFrameRate;
    UINT32 u4Video2PreRatio;    // 1x = 1024
    UINT32 u4CapExpUnit;
    UINT32 u4CapMaxFrameRate;
    UINT32 u4Cap2PreRatio;     // 1x = 1024
    UINT32 u4Video1ExpUnit;
    UINT32 u4Video1MaxFrameRate;
    UINT32 u4Video12PreRatio;    // 1x = 1024
    UINT32 u4Video2ExpUnit;
    UINT32 u4Video2MaxFrameRate;
    UINT32 u4Video22PreRatio;    // 1x = 1024
    UINT32 u4Custom1ExpUnit;
    UINT32 u4Custom1MaxFrameRate;
    UINT32 u4Custom12PreRatio;    // 1x = 1024
    UINT32 u4Custom2ExpUnit;
    UINT32 u4Custom2MaxFrameRate;
    UINT32 u4Custom22PreRatio;    // 1x = 1024
    UINT32 u4Custom3ExpUnit;
    UINT32 u4Custom3MaxFrameRate;
    UINT32 u4Custom32PreRatio;    // 1x = 1024
    UINT32 u4Custom4ExpUnit;
    UINT32 u4Custom4MaxFrameRate;
    UINT32 u4Custom42PreRatio;    // 1x = 1024
    UINT32 u4Custom5ExpUnit;
    UINT32 u4Custom5MaxFrameRate;
    UINT32 u4Custom52PreRatio;    // 1x = 1024
    UINT32 u4LensFno;           // 10 Base
    UINT32 u4FocusLength_100x;           // 100 Base
} AE_DEVICES_INFO_T;

//histogram control information
#define AE_CCT_STRENGTH_NUM (5)
#define AE_AOE_STRENGTH_NUM (3)
// Camera Scenario
typedef enum
{
    CAM_SCENARIO_PREVIEW = 0,  // PREVIEW
    CAM_SCENARIO_VIDEO,        // VIDEO
    CAM_SCENARIO_CAPTURE,      // CAPTURE
    CAM_SCENARIO_CUSTOM1,      // HDR
    CAM_SCENARIO_CUSTOM2,      // AUTO HDR
    CAM_SCENARIO_CUSTOM3,      // VT
    CAM_SCENARIO_CUSTOM4,      // STEREO
    CAM_SCENARIO_NUM
} CAM_SCENARIO_T;

typedef struct {
    INT32 u4X1;
    INT32 u4Y1;
    INT32 u4X2;
    INT32 u4Y2;
} AE_TARGET_PROB_T;

typedef struct {
    BOOL   bEnableHistStretch;          // enable histogram stretch
    UINT32 u4HistStretchWeight;         // Histogram weighting value
    UINT32 u4Pcent;                     // 1%=10, 0~1000
    UINT32 u4Thd;                       // 0~255
    UINT32 u4FlatThd;                   // 0~255

    UINT32 u4FlatBrightPcent;
    UINT32 u4FlatDarkPcent;
    AE_TARGET_PROB_T    sFlatRatio;     //TARGET_HS_FLAT

    BOOL  bEnableGreyTextEnhance;
    UINT32 u4GreyTextFlatStart;
    AE_TARGET_PROB_T  sGreyTextRatio;   //TARGET_HS_COLOR
} AE_HS_SPEC_T;

typedef struct {
    BOOL   bEnableAntiOverExposure;     // enable anti over exposure
    UINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value
    UINT32 u4Pcent;                     // 1%=10, 0~1000
    UINT32 u4Thd;                       // 0~255

    BOOL bEnableCOEP;                   // enable COEP
    UINT32 u4COEPcent;                  // center over-exposure prevention
    UINT32 u4COEThd;                    // center y threshold
    UINT32 u4BVCompRatio;               // Compensate BV in nonlinear
    AE_TARGET_PROB_T    sCOEYRatio;     // the outer y ratio
    AE_TARGET_PROB_T    sCOEDiffRatio;  // inner/outer y difference ratio
} AE_AOE_SPEC_T;

typedef struct {
    BOOL   bEnableBlackLight;           // enable back light detector
    UINT32 u4BackLightWeight;           // Back light weighting value
    UINT32 u4Pcent;                     // 1%=10, 0~1000
    UINT32 u4Thd;                       // 0~255

    UINT32 u4CenterHighBnd;             // center luminance
    UINT32 u4TargetStrength;            // final target limitation
    AE_TARGET_PROB_T    sFgBgEVRatio;   //TARGET_ABL_DIFF
    AE_TARGET_PROB_T    sBVRatio;       //FT_ABL
} AE_ABL_SPEC_T;

typedef struct {
    BOOL   bEnableNightScene;       // enable night scene
    UINT32 u4Pcent;                 // 1=0.1%, 0~1000
    UINT32 u4Thd;                   // 0~255
    UINT32 u4FlatThd;               // ev difference between darkest and brightest

    UINT32 u4BrightTonePcent;       // 1=0.1%bright tone percentage
    UINT32 u4BrightToneThd;         // < 255, bright tone THD

    UINT32 u4LowBndPcent;           // darkest percent, 1%=10, 0~1000
    UINT32 u4LowBndThd;             // <255, lower bound target
    UINT32 u4LowBndThdLimit;        // <255, upper bound of lower bound

    UINT32 u4FlatBrightPcent;       // the percentage of the brightest part used to cal flatness
    UINT32 u4FlatDarkPcent;         // the percentage of the darkest part used to cal flatness
    AE_TARGET_PROB_T    sFlatRatio; //TARGET_NS_FLAT
    AE_TARGET_PROB_T    sBVRatio;   //FT_NS

    BOOL bEnableNightSkySuppresion;
    AE_TARGET_PROB_T    sSkyBVRatio;   //FT_NS_SKY
} AE_NS_SPEC_T;

typedef struct {
    UINT32 uMeteringYLowBound;
    UINT32 uMeteringYHighBound;
    UINT32 uFaceYLowBound;
    UINT32 uFaceYHighBound;
    UINT32 uFaceCentralWeight;
    UINT32 u4MeteringStableMax;
    UINT32 u4MeteringStableMin;
} AE_TOUCH_FD_SPEC_T;

typedef struct {
    UINT32 uPrvFlareWeightArr[16];
    UINT32 uVideoFlareWeightArr[16];
    UINT32 u4FlareStdThrHigh;
    UINT32 u4FlareStdThrLow;
    UINT32 u4PrvCapFlareDiff;
    UINT32 u4FlareMaxStepGap_Fast;
    UINT32 u4FlareMaxStepGap_Slow;
    UINT32 u4FlarMaxStepGapLimitBV;
    UINT32 u4FlareAEStableCount;
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
    UINT32    u4BVCompRatio;               // Compensate BV in nonlinear
    strAEAOEAlgParam rAEAOEAlgParam[AE_AOE_STRENGTH_NUM];
} strAEAOEInputParm;

typedef struct
{
   //histogram info
    UINT32 u4HistHighThres;                         // central histogram high threshold
    UINT32 u4HistLowThres;                          // central histogram low threshold
    UINT32 u4MostBrightRatio;                       // full histogram high threshold
    UINT32 u4MostDarkRatio;                         // full histogram low threshold
    UINT32 u4CentralHighBound;                      // central block high boundary
    UINT32 u4CentralLowBound;                       // central block low bounary
    UINT32 u4OverExpThres[AE_CCT_STRENGTH_NUM];     // over exposure threshold
    UINT32 u4HistStretchThres[AE_CCT_STRENGTH_NUM]; // histogram stretch trheshold
    UINT32 u4BlackLightThres[AE_CCT_STRENGTH_NUM];  // backlight threshold
} AE_HIST_CFG_T;

//strAETable AE table Setting
typedef struct
{
    BOOL   bEnableBlackLight;           // enable back light detector
    BOOL   bEnableHistStretch;          // enable histogram stretch
    BOOL   bEnableAntiOverExposure;     // enable anti over exposure
    BOOL   bEnableTimeLPF;              // enable time domain LPF for smooth converge
    BOOL   bEnableCaptureThres;         // enable capture threshold or fix flare offset
    BOOL   bEnableVideoThres;             // enable video threshold or fix flare offset
    BOOL   bEnableVideo1Thres;       // enable video1 threshold or fix flare offset
    BOOL   bEnableVideo2Thres;       // enable video2 threshold or fix flare offset
    BOOL   bEnableCustom1Thres;    // enable custom1 threshold or fix flare offset
    BOOL   bEnableCustom2Thres;    // enable custom2 threshold or fix flare offset
    BOOL   bEnableCustom3Thres;    // enable custom3 threshold or fix flare offset
    BOOL   bEnableCustom4Thres;    // enable custom4 threshold or fix flare offset
    BOOL   bEnableCustom5Thres;    // enable custom5 threshold or fix flare offset
    BOOL   bEnableStrobeThres;           // enable strobe threshold or fix flare offset

    UINT32 u4AETarget;                  // central weighting target
    UINT32 u4StrobeAETarget;            // central weighting target
    UINT32 u4InitIndex;                 // AE initiail index

    UINT32 u4BackLightWeight;           // Back light weighting value
    UINT32 u4HistStretchWeight;         // Histogram weighting value
    UINT32 u4AntiOverExpWeight;         // Anti over exposure weighting value

    UINT32 u4BlackLightStrengthIndex;   // Black light threshold strength index
    UINT32 u4HistStretchStrengthIndex;  // Histogram stretch threshold strength index
    UINT32 u4AntiOverExpStrengthIndex;  // Anti over exposure threshold strength index
    UINT32 u4TimeLPFStrengthIndex;      // Smooth converge threshold strength index
    UINT32 u4LPFConvergeLevel[AE_CCT_STRENGTH_NUM];  //LPF converge support level

    UINT32 u4InDoorEV;                  // check the environment indoor/outdoor
    INT32   i4BVOffset;                  // Calibrate BV offset
    UINT32 u4PreviewFlareOffset;        // Fix preview flare offset
    UINT32 u4CaptureFlareOffset;        // Fix capture flare offset
    UINT32 u4CaptureFlareThres;         // Capture flare threshold
    UINT32 u4VideoFlareOffset;        // Fix video flare offset
    UINT32 u4VideoFlareThres;         // video flare threshold
    UINT32 u4CustomFlareOffset;        // Fix custom flare offset
    UINT32 u4CustomFlareThres;         // custom flare threshold
    UINT32 u4StrobeFlareOffset;        // Fix strobe flare offset
    UINT32 u4StrobeFlareThres;         // strobe flare threshold
    UINT32 u4PrvMaxFlareThres;        // for max preview flare thres used
    UINT32 u4PrvMinFlareThres;         // for min preview flare thres used
    UINT32 u4VideoMaxFlareThres;        // for video max flare thres used
    UINT32 u4VideoMinFlareThres;         // for video min flare thres used
    UINT32 u4FlatnessThres;              // 10 base for flatness condition.
    UINT32 u4FlatnessStrength;

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

typedef struct
{
    AE_DEVICES_INFO_T rDevicesInfo;
    AE_HIST_CFG_T rHistConfig;
    AE_CCT_CFG_T rCCTConfig;
    HDR_AE_CFG_T rHDRAEConfig;
} AE_NVRAM_T;


//____AWB NVRAM____
#define AWB_NVRAM_START
#define AWB_DAYLIGHT_LOCUS_LUT_SIZE        (21)
#define AWB_LIGHTSOURCE_NUM        (8)

#define AWB_LV_INDEX_UNIT (10) // 1.0 LV
#define AWB_LV_INDEX_MIN  (0)  // LV 0
#define AWB_LV_INDEX_MAX  (18) // LV 18
#define AWB_LV_INDEX_NUM ((AWB_LV_INDEX_MAX - AWB_LV_INDEX_MIN) + 1)

#define AWB_GM_OFFSET_THR_INDEX_UNIT    (500)
#define AWB_GM_OFFSET_THR_INDEX_NUM     (21)
#define AWB_GM_OFFSET_THR_INDEX_MAX     (AWB_GM_OFFSET_THR_INDEX_NUM - 1)

#define AWB_OFFSET_RATIO_INDEX_UNIT     (500)
#define AWB_OFFSET_RATIO_INDEX_NUM      (21)
#define AWB_OFFSET_RATIO_INDEX_MAX      (AWB_OFFSET_RATIO_INDEX_NUM - 1)

#define AWB_LIGHT_WEIGHT_INDEX_UNIT_WF      (200)
#define AWB_LIGHT_WEIGHT_INDEX_UNIT_T       (100)
#define AWB_LIGHT_WEIGHT_INDEX_UNIT_SHADE   (100)
#define AWB_LIGHT_WEIGHT_INDEX_NUM          (11)
#define AWB_LIGHT_WEIGHT_INDEX_MAX          (AWB_LIGHT_WEIGHT_INDEX_NUM - 1)

//____None Bayer Sensor____
#define RWB_COEF_SIZE (9)

// Preference light source
typedef enum
{
    AWB_PREFER_LIGHT_T = 0,
    AWB_PREFER_LIGHT_WF,
    AWB_PREFER_LIGHT_S,
    AWB_PREFER_LIGHT_NUM
} AWB_PREFER_LIGHT_ENUM_T;

// AWB gain
typedef struct
{
    INT32 i4R; // R gain
    INT32 i4G; // G gain
    INT32 i4B; // B gain
} AWB_GAIN_T;

// AWB bayer value
typedef struct
{
    INT32 i4R;  // R  value
    INT32 i4Gr; // Gr value
    INT32 i4Gb; // Gb value
    INT32 i4B;  // B  value
} AWB_BAYER_VALUE_T;

// XY coordinate
typedef struct
{
    INT32 i4X; // X
    INT32 i4Y; // Y
} XY_COORDINATE_T;

// Light area
typedef struct
{
    INT32 i4RightBound; // Right bound
    INT32 i4LeftBound;  // Left bound
    INT32 i4UpperBound; // Upper bound
    INT32 i4LowerBound; // Lower bound
} LIGHT_AREA_T;

// AWB calibration data
typedef struct
{
    AWB_GAIN_T rUnitGain;       // Unit gain: WB gain of DNP (individual camera)
    AWB_GAIN_T rGoldenGain;     // Golden sample gain: WB gain of DNP (golden sample)
    AWB_GAIN_T rUnitGainM;      // Unit gain: WB gain of TL84 (individual camera)
    AWB_GAIN_T rGoldenGainM;    // Golden sample gain: WB gain of TL84 (golden sample)
    AWB_GAIN_T rUnitGainL;      // Unit gain: WB gain of ALIGHT (individual camera)
    AWB_GAIN_T rGoldenGainL;    // Golden sample gain: WB gain of ALIGHT (golden sample)
    AWB_GAIN_T rTuningUnitGain; // Unit gain of tuning sample (for debug purpose)
    AWB_GAIN_T rD65Gain;        // WB gain of D65 (golden sample)
    AWB_BAYER_VALUE_T rUnitValue;     // Unit value: bayer RGB value of DNP (individual camera)
    AWB_BAYER_VALUE_T rGoldenValue;   // Golden sample value: bayer RGB value of DNP (golden sample)
    AWB_BAYER_VALUE_T rUnitValueM;    // Unit value: bayer RGB value of TL84 (individual camera)
    AWB_BAYER_VALUE_T rGoldenValueM;  // Golden sample value: bayer RGB value of TL84 (golden sample)
    AWB_BAYER_VALUE_T rUnitValueL;    // Unit value: bayer RGB value of ALIGHT (individual camera)
    AWB_BAYER_VALUE_T rGoldenValueL;  // Golden sample value: bayer RGB value of ALIGHT (golden sample)
} AWB_CALIBRATION_DATA_T;

// AWB light source XY coordinate
typedef struct
{
    XY_COORDINATE_T rStrobe;   // Strobe
    XY_COORDINATE_T rHorizon;  // Horizon
    XY_COORDINATE_T rA;        // A
    XY_COORDINATE_T rTL84;     // TL84
    XY_COORDINATE_T rCWF;      // CWF
    XY_COORDINATE_T rDNP;      // DNP
    XY_COORDINATE_T rD65;      // D65
    XY_COORDINATE_T rDF;       // Daylight fluorescent
} AWB_LIGHT_SOURCE_XY_COORDINATE_T;

// AWB light source AWB gain
typedef struct
{
    AWB_GAIN_T rStrobe;   // Strobe
    AWB_GAIN_T rHorizon;  // Horizon
    AWB_GAIN_T rA;        // A
    AWB_GAIN_T rTL84;     // TL84
    AWB_GAIN_T rCWF;      // CWF
    AWB_GAIN_T rDNP;      // DNP
    AWB_GAIN_T rD65;      // D65
    AWB_GAIN_T rDF;       // Daylight fluorescent
} AWB_LIGHT_SOURCE_AWB_GAIN_T;

// Rotation matrix parameter
typedef struct
{
    INT32 i4RotationAngle; // Rotation angle
    INT32 i4Cos;           // cos
    INT32 i4Sin;           // sin
} AWB_ROTATION_MATRIX_T;

// Daylight locus parameter
typedef struct
{
    INT32 i4SlopeNumerator;   // Slope numerator
    INT32 i4SlopeDenominator; // Slope denominator
} AWB_DAYLIGHT_LOCUS_T;

// Predictor parameter
typedef struct {
    AWB_GAIN_T rSpatial_L;
    AWB_GAIN_T rSpatial_H;
    AWB_GAIN_T rTemporal_General;
}AWB_PREDICTOR_GAIN_T;

// AWB light area
typedef struct
{
    LIGHT_AREA_T rStrobe; // Strobe
    LIGHT_AREA_T rTungsten;        // Tungsten
    LIGHT_AREA_T rWarmFluorescent; // Warm fluorescent
    LIGHT_AREA_T rFluorescent;     // Fluorescent
    LIGHT_AREA_T rCWF;             // CWF
    LIGHT_AREA_T rDaylight;        // Daylight
    LIGHT_AREA_T rShade;           // Shade
    LIGHT_AREA_T rDaylightFluorescent; // Daylight fluorescent
} AWB_LIGHT_AREA_T;

// PWB light area
typedef struct
{
    LIGHT_AREA_T rReferenceArea;   // Reference area
    LIGHT_AREA_T rDaylight;        // Daylight
    LIGHT_AREA_T rCloudyDaylight;  // Cloudy daylight
    LIGHT_AREA_T rShade;           // Shade
    LIGHT_AREA_T rTwilight;        // Twilight
    LIGHT_AREA_T rFluorescent;     // Fluorescent
    LIGHT_AREA_T rWarmFluorescent; // Warm fluorescent
    LIGHT_AREA_T rIncandescent;    // Incandescent
    LIGHT_AREA_T rGrayWorld; // for CCT use
} PWB_LIGHT_AREA_T;

// PWB default gain
typedef struct
{
    AWB_GAIN_T rDaylight;        // Daylight
    AWB_GAIN_T rCloudyDaylight;  // Cloudy daylight
    AWB_GAIN_T rShade;           // Shade
    AWB_GAIN_T rTwilight;        // Twilight
    AWB_GAIN_T rFluorescent;     // Fluorescent
    AWB_GAIN_T rWarmFluorescent; // Warm fluorescent
    AWB_GAIN_T rIncandescent;    // Incandescent
    AWB_GAIN_T rGrayWorld; // for CCT use
} PWB_DEFAULT_GAIN_T;

// AWB daylight locus target offset ratio LUT for T / WF
typedef struct
{
	MINT32 i4LUTIn[AWB_DAYLIGHT_LOCUS_LUT_SIZE]; // Look-up table
	MINT32 i4LUTOut[AWB_DAYLIGHT_LOCUS_LUT_SIZE]; // Look-up table
} AWB_OFFSET_LUT_T;

typedef struct {
	MINT32 i4Neutral_ParentBlk_Thr;
	MINT32 i4CWFDF_LUTThr[AWB_LV_INDEX_NUM];
} AWB_TEMPORAL_ENQUEUE_THR_T;

// Threshold
typedef struct
{
	MINT32 i4ThrL;
    MINT32 i4ThrH;
} AWB_THR_LOW_HIGH_T;

typedef struct {
	AWB_THR_LOW_HIGH_T rNonReliable;
	AWB_THR_LOW_HIGH_T rLightSrc[AWB_LIGHTSOURCE_NUM];
} AWB_PREDICTOR_LV_THR_T;

typedef struct
{
	MINT32 i4LUT[AWB_LV_INDEX_NUM];
} AWB_LV_LUT_T;

// AWB convergence parameter
typedef struct
{
    MINT32 i4Speed; // Convergence speed
    MINT32 i4StableThr; // Stable threshold
    MINT32 i4ToTagetEnable; //converge to target enable
} AWB_CONVERGENCE_PARAM_T;

// Neutral parent block number threshold
typedef struct
{
	MINT32 i4NonNeutral[AWB_LV_INDEX_NUM];	        // unit: %
    MINT32 i4NRThr[AWB_LIGHTSOURCE_NUM][AWB_LV_INDEX_NUM]; // unit: %
} AWB_NEUTRAL_PARENT_BLK_NUM_THR_T;

#define AWB_CCT_ESTIMATION_LIGHT_SOURCE_NUM (5)

// CCT estimation
typedef struct
{
    INT32 i4CCT[AWB_CCT_ESTIMATION_LIGHT_SOURCE_NUM];                // CCT
    INT32 i4RotatedXCoordinate[AWB_CCT_ESTIMATION_LIGHT_SOURCE_NUM]; // Rotated X coordinate
} AWB_CCT_ESTIMATION_T;

typedef struct
{
    MINT32 i4IsRWBEnable;
    MINT32 i4RWBCof[RWB_COEF_SIZE];
    MINT32 i4RWBCofInv[RWB_COEF_SIZE];
} AWB_NONE_BAYER_T;

typedef struct
{
    MINT32 i4ProjCode;
    MINT32 i4Model;
    MINT32 i4Date;
    MINT32 i4Res0;
    MINT32 i4Res1;
    MINT32 i4Res2;
    MINT32 i4Res3;
    MINT32 i4Res4;
} AWB_TUNING_INFO_T;

typedef struct
{
	MINT32 i4R;
	MINT32 i4G;
	MINT32 i4B;
} AWB_RGB_AVG_STAT_T;

typedef struct
{
	MINT32 i4RG;
	MINT32 i4BG;
} AWB_RG_BG_STAT_T;

typedef struct {
	MUINT32 u4LowBound;
	MUINT32 u4HiBound;
} AWB_FD_RGB_BOUNDARY_T;

typedef struct {
	AWB_RG_BG_STAT_T rHiCT;
	AWB_RG_BG_STAT_T rMidCT;
	AWB_RG_BG_STAT_T rLowCT;
} AWB_FACE_REF_TARGET_T;

typedef struct {
	MINT32 i4LV_Low;
	MINT32 i4LV_Hi;
	MINT32 i4FaceRB_Low;
	MINT32 i4FaceRB_Hi;
	MINT32 i4TempoWeight;
} AWB_FACE_DYNAMIC_TARGET_T;

typedef struct {
	MINT32 i4LV_Low;
	MINT32 i4LV_Hi;
	MINT32 i4Prob_Low;
	MINT32 i4Prob_Hi;
} AWB_FACE_OUTDOOR_PROB_LV_T;

typedef struct {
	MINT32 i4P0_Low;
	MINT32 i4P0_Hi;
	MINT32 i4Prob_Low;
	MINT32 i4Prob_Hi;
} AWB_FACE_OUTDOOR_PROB_DAYLIGHT_T;

typedef struct {
	MINT32 i4P0_Low;
	MINT32 i4P0_Hi;
	MINT32 i4Prob_Low;
	MINT32 i4Prob_Hi;
	MINT32 i4NewOffset_Hi;
} AWB_FACE_OUTDOOR_PROB_SHADE_T;

typedef struct {
	AWB_FACE_OUTDOOR_PROB_LV_T rLV;
	AWB_FACE_OUTDOOR_PROB_DAYLIGHT_T rDaylight;
	AWB_FACE_OUTDOOR_PROB_SHADE_T rShade;
} AWB_FACE_OUTDOOR_PROB_T;

typedef struct {
	MINT32 i4FaceSizeRatio_Low;
	MINT32 i4FaceSizeRatio_Hi;
	MINT32 i4Prob0_Min;
	MINT32 i4Prob0_Max;
} AWB_FACE_PROB0_T;

typedef struct {
	MINT32 i4FaceAreaRG_LowTh;
	MINT32 i4FaceAreaRG_HiTh;
	MINT32 i4FaceAreaRG_Tolerance;
	MINT32 i4Prob2_Min;
	MINT32 i4Prob2_Max;
} AWB_FACE_PROB2_T;

typedef struct {
	MINT32 i4FaceAreaBG_LowTh;
	MINT32 i4FaceAreaBG_HiTh;
	MINT32 i4FaceAreaBG_Tolerance;
	MINT32 i4Prob3_Min;
	MINT32 i4Prob3_Max;
} AWB_FACE_PROB3_T;

typedef struct {
	MINT32 i4FaceSizeRatio_Hi;
	MINT32 i4FaceSizeRatio_Low;
	MINT32 i4Prob0_Min;
	MINT32 i4Prob0_Max;
} AWB_FACE_SPATIAL_PROB0_T;

typedef struct {
	MINT32 i4NeutralParentBlkNum_Hi;
	MINT32 i4NeutralParentBlkNum_Low;
	MINT32 i4Prob1_Min;
	MINT32 i4Prob1_Max;
} AWB_FACE_SPATIAL_PROB1_T;

typedef struct {
	AWB_FACE_SPATIAL_PROB0_T rSpatial_P0;
	AWB_FACE_SPATIAL_PROB1_T rSpatial_P1;
	MINT32 i4LV_Low;
	MINT32 i4LV_Hi;
} AWB_FACE_SPATIAL_T;

typedef struct {
	MINT32 i4LVChange_Th;
	MINT32 i4AWBGainChange_Th;
} AWB_FACE_SCENE_CHANGE_T;

typedef struct {
	MINT32 i4Speed;
	MINT32 i4MinStep;
	MINT32 i4ProbReduceStep;
} AWB_FACE_TEMPO_SMOOTH_T;

typedef struct {
	MINT32 i4Enable;
	MINT32 i4StatAvoidFaceArea;
	MINT32 i4FaceWinRatio;
	MUINT32 u4FaceCentralWeight;
	AWB_FD_RGB_BOUNDARY_T rFD_RGB_Bound;
	AWB_FACE_REF_TARGET_T rRefTarget;
	AWB_FACE_DYNAMIC_TARGET_T rDynamicTarget;
	MINT32 i4SameHueKeepOriGain_En;
	AWB_FACE_OUTDOOR_PROB_T rOutdoorProb;
	MINT32 i4GainRatioRestrictLUT[AWB_LV_INDEX_NUM];
	AWB_GAIN_T rPrefGain[AWB_LV_INDEX_NUM];
	AWB_FACE_PROB0_T rFaceAWBProb0;
	MINT32 i4FaceAWBProb1LUT[AWB_LV_INDEX_NUM];
	AWB_FACE_PROB2_T rFaceAWBProb2;
	AWB_FACE_PROB3_T rFaceAWBProb3;
	AWB_FACE_SPATIAL_T rSpatial;
	AWB_FACE_TEMPO_SMOOTH_T rTempoSmooth;
	AWB_FACE_SCENE_CHANGE_T rSceneChange;
} AWB_FACE_COMP_T;

typedef struct {
	MINT32 i4Enable;
	MINT32 i4StatWinShrinkOffset[AWB_LIGHTSOURCE_NUM];
	MINT32 Reserved1;
	MINT32 Reserved2;
	MINT32 Reserved3;
	MINT32 Reserved4;
	MINT32 Reserved5;
} AWB_SMOOTH_STATISTIC_T;

typedef struct {
	MINT32 i4Enable;
	MINT32 i4WeightingLUT[AWB_LV_INDEX_NUM];
	AWB_GAIN_T rPreferenceGainLUT[AWB_LIGHTSOURCE_NUM][AWB_LV_INDEX_NUM];
	MINT32 Reserved1;
	MINT32 Reserved2;
	MINT32 Reserved3;
	MINT32 Reserved4;
	MINT32 Reserved5;
} AWB_LIGHT_SENSOR_ASSIST_T;

// AWB daylight locus target offset ratio LUT for tungsten and fluorescent0
typedef struct
{
	MINT32 i4Lut[AWB_OFFSET_RATIO_INDEX_NUM]; // Look-up table
} AWB_OFFSET_RATIO_LUT_T;

// AWB Green/Magenta offset threshold LUT
typedef struct
{
    MINT32 i4OffsetMode;
	MINT32 i4LutM[AWB_GM_OFFSET_THR_INDEX_NUM]; // Look-up table
	MINT32 i4LutG[AWB_GM_OFFSET_THR_INDEX_NUM]; // Look-up table
} AWB_GM_OFFSET_THR_LUT_T;

// AWB light weight LUT for P2
typedef struct
{
	MINT32 i4LutM[AWB_LIGHT_WEIGHT_INDEX_NUM]; // Look-up table
	MINT32 i4LutG[AWB_LIGHT_WEIGHT_INDEX_NUM]; // Look-up table
} AWB_LIGHT_WEIGHT_LUT_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4LimitY[AWB_LIGHTSOURCE_NUM];
    MINT32 i4WeightReduce[AWB_LIGHTSOURCE_NUM];
} AWB_STATISTIC_LIMIT_T;

typedef struct
{
    MINT32 i4GeneralMode;        // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
    MINT32 i4NonReliableMode;    // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
    MINT32 i4TempInitMode;       // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
    MINT32 i4ThrTemp;
    MINT32 i4TempLv[AWB_LIGHTSOURCE_NUM];
} AWB_SPATIAL_MODE_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4PbRatio;
    MINT32 i4Range;
    MINT32 i4Clip;
} AWB_REFINEMENT_P0_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4PbRatio;
    MINT32 i4LvThr;
    MINT32 i4P0Thr1;
    MINT32 i4P0Thr2;
} AWB_REFINEMENT_P2_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4Thr[AWB_PREFER_LIGHT_NUM];
} AWB_OFFSET_SMOOTH_T;

typedef struct
{
    MINT32 i4CbMinThr;
    AWB_SPATIAL_MODE_T rSpatialMode;
    AWB_REFINEMENT_P0_T rRefineP0;
    AWB_REFINEMENT_P2_T rRefineP2;
    AWB_OFFSET_SMOOTH_T rOffsetSmooth;
} AWB_REFINEMENT_T;

typedef struct {
    MINT32 i4Enable;
	MINT32 i4LVThr_L;
    MINT32 i4LVThr_H;
	MINT32 i4SunsetCountThr;
	MINT32 i4SunsetCountRatio_L;
	MINT32 i4SunsetCountRatio_H;
    XY_COORDINATE_T rArea;
} AWB_SUNSET_PROP_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4Method;    // 0: Method 0, 1: Method 1
	MINT32 i4LVThr_L;
    MINT32 i4LVThr_H;
	MINT32 i4DaylightProb;
    XY_COORDINATE_T rArea;
    XY_COORDINATE_T rVertex;
} AWB_SUBWIN_PROP_T;

typedef struct
{
    MINT32 i4Enable;
    AWB_GAIN_T rGain;
    LIGHT_AREA_T rArea;
    AWB_THR_LOW_HIGH_T rLv;
    AWB_THR_LOW_HIGH_T rCount;
    AWB_THR_LOW_HIGH_T rWeighting;
} AWB_EXTRACOLOR_PROP_T;

typedef struct {
	AWB_SUNSET_PROP_T rSunsetProp;
	AWB_SUBWIN_PROP_T rSubWinPropF;
	AWB_SUBWIN_PROP_T rSubWinPropCWF;
    AWB_EXTRACOLOR_PROP_T  rExtraColorProp;
} AWB_FEATURE_RROP_T;

// Preference color
typedef struct
{
    MINT32 i4OffsetThr[AWB_PREFER_LIGHT_NUM];   // Offset threshold
    AWB_THR_LOW_HIGH_T rOffsetRatioLv[AWB_PREFER_LIGHT_NUM];
    AWB_OFFSET_RATIO_LUT_T rOffsetRatio[AWB_PREFER_LIGHT_NUM];
	AWB_OFFSET_LUT_T rOffsetLUTs[AWB_PREFER_LIGHT_NUM];
    AWB_GM_OFFSET_THR_LUT_T rGMOffsetThr[AWB_PREFER_LIGHT_NUM];
    AWB_LIGHT_WEIGHT_LUT_T rLightWeight[AWB_PREFER_LIGHT_NUM];
} AWB_PREFERENCE_COLOR_T;

typedef struct {
	AWB_REFINEMENT_T 			rRefinement;
    AWB_STATISTIC_LIMIT_T       rStatLimit;
	AWB_FACE_COMP_T				rFaceComp;
	AWB_SMOOTH_STATISTIC_T		rSmoothStatistic;
	AWB_LIGHT_SENSOR_ASSIST_T	rLightSensorAssist;
} AWB_ADV_FUNC_T;

typedef struct {
    AWB_CALIBRATION_DATA_T rCalData; // AWB calibration data
    AWB_LIGHT_SOURCE_XY_COORDINATE_T rOriginalXY; // Original XY coordinate of AWB light source
    AWB_LIGHT_SOURCE_XY_COORDINATE_T rRotatedXY; // Rotated XY coordinate of AWB light source
	AWB_LIGHT_SOURCE_AWB_GAIN_T rLightAWBGain; // AWB gain of AWB light source
    AWB_ROTATION_MATRIX_T rRotationMatrix; // Rotation matrix parameter
    AWB_DAYLIGHT_LOCUS_T rDaylightLocus; // Daylight locus parameter
    AWB_PREDICTOR_GAIN_T rPredictorGain;	// Spatial / Temporal predictor gain
    AWB_LIGHT_AREA_T rAWBLightArea; // AWB light area
    PWB_LIGHT_AREA_T rPWBLightArea; // PWB light area
    PWB_DEFAULT_GAIN_T rPWBDefaultGain; // PWB default gain
    AWB_CCT_ESTIMATION_T rCCTEstimation; // CCT estimation
    AWB_NONE_BAYER_T rNoneBayer;
}AWB_ALGO_CAL_T;

typedef struct {
    AWB_GAIN_T rLightPrefGain[AWB_LIGHTSOURCE_NUM][AWB_LV_INDEX_NUM];
	AWB_PREDICTOR_LV_THR_T rPredictorLVThr;
	AWB_TEMPORAL_ENQUEUE_THR_T rTemporalEnqueueThr;
	AWB_NEUTRAL_PARENT_BLK_NUM_THR_T rNeutralBlkThr;
	AWB_FEATURE_RROP_T rFeatureProp;
	AWB_LV_LUT_T rNonNeutralProb;
	AWB_LV_LUT_T rDaylightLocusProb[AWB_LIGHTSOURCE_NUM];
    AWB_LV_LUT_T rLightProb1[AWB_LIGHTSOURCE_NUM];
    AWB_CONVERGENCE_PARAM_T rConvergence;
	AWB_PREFERENCE_COLOR_T rPreferColor;
	AWB_ADV_FUNC_T rAdvFunction;
    AWB_TUNING_INFO_T rTuningInfo;
}AWB_ALGO_TUNING_T;


// AWB NVRAM structure
typedef struct
{
    AWB_ALGO_CAL_T      rAlgoCalParam;      // AWB param by calibration
    AWB_ALGO_TUNING_T	rAlgoTuningParam;	// AWB param by tuning
} AWB_NVRAM_T;

// Flash AWB tuning parameter
typedef struct
{
//=== Foreground and Background definition ===
    MUINT32 ForeGroundPercentage;  //>50   default: 9
    MUINT32 BackGroundPercentage;  //<50   default: 95

//=== Table to decide foreground weight (m_FG_Weight) ===
//Th1 < Th2 < Th3 < Th4
//FgPercentage_Thx_Val < 2000
    MUINT32 FgPercentage_Th1;  //default: 2
    MUINT32 FgPercentage_Th2;  //default: 5
    MUINT32 FgPercentage_Th3; //default: 10
    MUINT32 FgPercentage_Th4; //default: 15
    MUINT32 FgPercentage_Th1_Val; //default: 200
    MUINT32 FgPercentage_Th2_Val; //default: 250
    MUINT32 FgPercentage_Th3_Val; //default: 300
    MUINT32 FgPercentage_Th4_Val; //default: 350

//=== Location weighting map ===//
//Th1 < Th2 < Th3 < Th4
//location_map_val1 <= location_map_val2 <= location_map_val3 <= location_map_val4 < 500
    MUINT32 location_map_th1; //default: 10
    MUINT32 location_map_th2; //default: 20
    MUINT32 location_map_th3; //default: 40
    MUINT32 location_map_th4; //default: 50
    MUINT32 location_map_val1; //default: 100
    MUINT32 location_map_val2; //default: 110
    MUINT32 location_map_val3; //default: 130
    MUINT32 location_map_val4; //default: 150

//=== Decide foreground Weighting ===//
// FgBgTbl_Y0 <= 2000
    MUINT32 SelfTuningFbBgWeightTbl;  //default: 0
    MUINT32 FgBgTbl_Y0;
    MUINT32 FgBgTbl_Y1;
    MUINT32 FgBgTbl_Y2;
    MUINT32 FgBgTbl_Y3;
    MUINT32 FgBgTbl_Y4;
    MUINT32 FgBgTbl_Y5;


//=== Decide luminance weight === //
//YPrimeWeightTh[i] <= 256
//YPrimeWeight[i] <= 10
    MUINT32 YPrimeWeightTh[5];     // default: {5,9,11,13,15}
    MUINT32 YPrimeWeight[4];     // default: {0, 0.1, 0.3, 0.5, 0.7}

    AWB_GAIN_T FlashPreferenceGain;

}FLASH_AWB_TUNING_PARAM_T;

#define FLASH_DUTY_NUM (1600)

typedef struct
{
    AWB_GAIN_T flashWBGain[FLASH_DUTY_NUM]; // Flash AWB calibration data
} FLASH_AWB_CALIBRATION_DATA_STRUCT, *PFLASH_AWB_CALIBRATION_DATA_STRUCT;

// Flash AWB NVRAM structure
typedef struct
{
    FLASH_AWB_TUNING_PARAM_T rTuningParam; // Flash AWB tuning parameter
    FLASH_AWB_CALIBRATION_DATA_STRUCT rCalibrationData; // Flash AWB calibration data
} FLASH_AWB_NVRAM_T;

//==============================
// flash nvram
//==============================

enum
{
    e_NVRAM_AE_SCENE_DEFAULT=-2,
};

typedef struct
{
    int yTarget;  // 188 (10bit)
    int fgWIncreaseLevelbySize; // 10
    int fgWIncreaseLevelbyRef;  // 0
    int ambientRefAccuracyRatio;  // 5  5/256=2%
    int flashRefAccuracyRatio;  // 1   1/256=0.4%
    int backlightAccuracyRatio; // 18 18/256=7%
    int backlightUnderY;  //  40 (10-bit)
    int backlightWeakRefRatio;  // 32  32/256=12.5%
    int safetyExp; // 33322
    int maxUsableISO;  // 680
    int yTargetWeight;  // 0 base:256
    int lowReflectanceThreshold;  // 13  13/256=5%
    int flashReflectanceWeight;  // 0 base:256
    int bgSuppressMaxDecreaseEV;  // 2EV
    int bgSuppressMaxOverExpRatio; // 6  6/256=2%
    int fgEnhanceMaxIncreaseEV; // 5EV
    int fgEnhanceMaxOverExpRatio; // 6  10/256=2%
    int isFollowCapPline;  // 0 for auto mode, 1 for others
    int histStretchMaxFgYTarget; // 266 (10bit)
    int histStretchBrightestYTarget; // 328 (10bit)
    int fgSizeShiftRatio; // 0 0/256=0%
    int backlitPreflashTriggerLV; // 90 (unit:0.1EV)
    int backlitMinYTarget; // 100 (10bit)
    int minstameanpass; // 80 (10bit)
} NVRAM_FLASH_TUNING_PARA;

typedef struct
{
    int exp;
    int afe_gain;
    int isp_gain;
    int distance;
    short yTab[40*40];  //x128


}NVRAM_FLASH_CCT_ENG_TABLE;


typedef struct
{
    //torch, video
    int torchDuty;
    int torchDutyEx[20];
    //AF
    int afDuty;
    //pf, mf
    //normal bat setting
    int pfDuty;
    int mfDutyMax;
    int mfDutyMin;
    //low bat setting
    int IChangeByVBatEn;
    int vBatL;  //mv
    int pfDutyL;
    int mfDutyMaxL;
    int mfDutyMinL;
    //burst setting
    int IChangeByBurstEn;
    int pfDutyB;
    int mfDutyMaxB;
    int mfDutyMinB;
    //high current setting, set the duty at about 1A. when I is larget, notify system to reduce modem power, cpu ...etc
    int decSysIAtHighEn;
    int dutyH;
}
NVRAM_FLASH_ENG_LEVEL;


typedef struct
{
    //torch, video
    int torchDuty;
    int torchDutyEx[20];


    //AF
    int afDuty;

    //pf, mf
    //normal bat setting
    int pfDuty;
    int mfDutyMax;
    int mfDutyMin;
    //low bat setting
    int pfDutyL;
    int mfDutyMaxL;
    int mfDutyMinL;
    //burst setting
    int pfDutyB;
    int mfDutyMaxB;
    int mfDutyMinB;
}
NVRAM_FLASH_ENG_LEVEL_LT; //low color temperature

typedef enum
{
  FLASH_CHOOSE_WARM,
  FLASH_CHOOSE_COLD,
}EFLASH_CHOOSE_TYPE;

typedef struct
{
    int toleranceEV_pos;
    int toleranceEV_neg;

    int XYWeighting;

    bool  useAwbPreferenceGain;

    int envOffsetIndex[4];
    int envXrOffsetValue[4];
    int envYrOffsetValue[4];

    MINT32 VarianceTolerance;
    EFLASH_CHOOSE_TYPE ChooseColdOrWarm;

}NVRAM_DUAL_FLASH_TUNING_PARA;


typedef struct CAMERA_PD_TBL_STRUCT_t
{
    MUINT32 bpci_xsize;
    MUINT32 bpci_ysize;
    MUINT32 pdo_xsize;
    MUINT32 pdo_ysize;
    MUINT8* bpci_array;
} CAMERA_PD_TBL_STRUCT, *PCAMERA_PD_TBL_STRUCT;

typedef struct CAMERA_BPCI_STRUCT_t
{
    CAMERA_PD_TBL_STRUCT PDC_TBL_1;
    CAMERA_PD_TBL_STRUCT PDC_TBL_2;
    CAMERA_PD_TBL_STRUCT PDC_TBL_3;
    CAMERA_PD_TBL_STRUCT PDC_TBL_4;

} CAMERA_BPCI_STRUCT, *PCAMERA_BPCI_STRUCT;

typedef union
{
    struct
    {
        UINT32 u4Version;
        NVRAM_FLASH_CCT_ENG_TABLE engTab;
        NVRAM_FLASH_TUNING_PARA tuningPara[8];;
        UINT32 paraIdxForceOn[19];
        UINT32 paraIdxAuto[19];
        NVRAM_FLASH_ENG_LEVEL engLevel;
        NVRAM_FLASH_ENG_LEVEL_LT engLevelLT;
        NVRAM_DUAL_FLASH_TUNING_PARA dualTuningPara;
    };
    UINT8 temp[MAXIMUM_NVRAM_CAMERA_DEFECT_FILE_SIZE];

} NVRAM_CAMERA_STROBE_STRUCT, *PNVRAM_CAMERA_STROBE_STRUCT;


typedef union
{
    struct
    {
        short yTab[1600];
        AWB_GAIN_T flashWBGain[1600]; // Flash AWB calibration data
    };
    UINT8 temp[MAXIMUM_NVRAM_CAMERA_FLASH_CALIBRATION_FILE_SIZE];

} NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT, *PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT;

#endif // _CAMERA_CUSTOM_3A_NVRAM_H_

