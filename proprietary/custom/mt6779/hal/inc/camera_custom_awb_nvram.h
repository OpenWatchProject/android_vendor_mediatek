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
#ifndef _CAMERA_CUSTOM_AWB_NVRAM_H_
#define _CAMERA_CUSTOM_AWB_NVRAM_H_

#include "MediaTypes.h"

#define NVRAM_CUSTOM_AWB_REVISION 	8284001

// AWB Calibration light source
typedef enum
{
    AWB_CAL_LIGHT_SRC_STROBE = 0,
    AWB_CAL_LIGHT_SRC_H,
    AWB_CAL_LIGHT_SRC_A,
    AWB_CAL_LIGHT_SRC_TL84,
    AWB_CAL_LIGHT_SRC_CWF,
    AWB_CAL_LIGHT_SRC_DNP,
    AWB_CAL_LIGHT_SRC_D65,
    AWB_CAL_LIGHT_SRC_DF,
    AWB_CAL_LIGHT_SRC_NUM
} AWB_CAL_LIGHT_SRC_T;

// Light source definition
typedef enum
{
    AWB_LIGHT_STROBE = 0,   // Strobe
    AWB_LIGHT_T,            // Tungsten
    AWB_LIGHT_WF,           // Warm fluorescent
    AWB_LIGHT_F,            // Fluorescent (TL84)
    AWB_LIGHT_CWF,          // CWF
    AWB_LIGHT_DAYLIGHT,     // Daylight
    AWB_LIGHT_SHADE,        // Shade
    AWB_LIGHT_DF,           // Daylight fluorescent
    AWB_LIGHT_NUM,          // Light source number
    AWB_LIGHT_NONE = AWB_LIGHT_NUM, // None: not neutral block
    //AWB_LIGHT_DONT_CARE = 0xFF    // Don't care: don't care the light source of block
} AWB_LIGHT_SRC_T;

typedef enum
{
    PWB_LIGHT_DAYLIGHT = 0,     // Daylight
    PWB_LIGHT_CLOUDY_D,         // Cloudy daylight
    PWB_LIGHT_SHADE,            // Shade
    PWB_LIGHT_TWILIGHT,         // Twilight
    PWB_LIGHT_F,                // Fluorescent
    PWB_LIGHT_WF,               // Warm fluorescent
    PWB_LIGHT_INCANDESCENT,     // Incandescent
    PWB_LIGHT_GRAY_WORLD,       // for CCT use
    PWB_LIGHT_NUM,
} PWB_LIGHT_SRC_T;


//____AWB NVRAM____
#define AWB_NVRAM_START
#define AWB_DAYLIGHT_LOCUS_LUT_SIZE        (21)
#define AWB_DAYLIGHT_LOCUS_LUT_MAX         (AWB_DAYLIGHT_LOCUS_LUT_SIZE - 1)
#define AWB_LIGHTSOURCE_NUM                 (8)

#define AWB_LV_INDEX_UNIT (10) // 1.0 LV
#define AWB_LV_INDEX_NEGATIVE_SHIFT (4)
#define AWB_LV_INDEX_MIN  (0)
#define AWB_LV_INDEX_MAX  (18) // LV 18
#define AWB_LV_INDEX_NUM ((AWB_LV_INDEX_MAX - AWB_LV_INDEX_MIN) + 1)
#define AWB_LV_N_INDEX_MAX  (18 + AWB_LV_INDEX_NEGATIVE_SHIFT) // support negative LV
#define AWB_LV_N_INDEX_NUM ((AWB_LV_N_INDEX_MAX - AWB_LV_INDEX_MIN) + 1) // support negative LV


#define AWB_GM_OFFSET_THR_INDEX_UNIT    (500)
#define AWB_GM_OFFSET_THR_INDEX_NUM     (21)
#define AWB_GM_OFFSET_THR_INDEX_MAX     (AWB_GM_OFFSET_THR_INDEX_NUM - 1)

#define AWB_OFFSET_RATIO_INDEX_UNIT     (500)
#define AWB_OFFSET_RATIO_INDEX_NUM      (21)
#define AWB_OFFSET_RATIO_INDEX_MAX      (AWB_OFFSET_RATIO_INDEX_NUM - 1)

#define AWB_LIGHT_WEIGHT_INDEX_UNIT_WF      (200)
#define AWB_LIGHT_WEIGHT_INDEX_UNIT_T       (100)
#define AWB_LIGHT_WEIGHT_INDEX_UNIT_SHADE   (100)
#define AWB_LIGHT_WEIGHT_INDEX_UNIT_FACE_AST    (25)
#define AWB_LIGHT_WEIGHT_INDEX_NUM          (11)
#define AWB_LIGHT_WEIGHT_INDEX_MAX          (AWB_LIGHT_WEIGHT_INDEX_NUM - 1)

#define AWB_LIGHT_GM_OFFSET_UNIT            (100)
#define AWB_LIGHT_GM_OFFSET_NUM             (11)
#define AWB_LIGHT_GM_OFFSET_MAX             (AWB_LIGHT_GM_OFFSET_NUM - 1)

//____None Bayer Sensor____
#define RWB_COEF_SIZE (9)

//____Auto White Balance Config____
#define AWB_SCALE_UNIT (512) // 1.0 = 512

#define AWB_PB_WEIGHT_LUT_INDEX_UINT  (16)
#define AWB_PB_WEIGHT_LUT_INDEX_MAX   (256 / AWB_PB_WEIGHT_LUT_INDEX_UINT)
#define AWB_PB_WEIGHT_LUT_INDEX_NUM   (AWB_PB_WEIGHT_LUT_INDEX_MAX + 1)

#define AWB_EXTRA_COLOR_NUM  (16)
#define AWB_EXCLUDE_COLOR_NUM  (8)

#define AWB_EXCLUDE_COLOR_MAX_WEIGHT (32)
#define AWB_SKY_GAIN_NUM (4)
#define AWB_SKY_GAIN_INDEX_UNIT (20)

// Parent block size
#ifdef _SUPPORT_PB_3X3_
#define AWB_PARENT_BLK_SIZE_X (3)
#define AWB_PARENT_BLK_SIZE_Y (3)
#else
#define AWB_PARENT_BLK_SIZE_X (5)
#define AWB_PARENT_BLK_SIZE_Y (5)
#endif
#define AWB_PARENT_BLK_SIZE (AWB_PARENT_BLK_SIZE_X * AWB_PARENT_BLK_SIZE_Y)

#define AWB_WINDOW_NUM_MAX_X (125)
#define AWB_WINDOW_NUM_MAX_Y (125)
#define AWB_PARENT_BLK_NUM_MAX_X (AWB_WINDOW_NUM_MAX_X/AWB_PARENT_BLK_SIZE_X)
#define AWB_PARENT_BLK_NUM_MAX_Y (AWB_WINDOW_NUM_MAX_Y/AWB_PARENT_BLK_SIZE_Y)
#define AWB_PARENT_BLK_NUM_MAX (AWB_PARENT_BLK_NUM_MAX_X * AWB_PARENT_BLK_NUM_MAX_Y)

#define AWB_FACE_STAT_WEIGHT_UNIT (100)
#define AWB_FACE_INDICATOR_UNIT (1000)

// Preference light source
typedef enum
{
    AWB_PREFER_LIGHT_T = 0,
    AWB_PREFER_LIGHT_WF,
    AWB_PREFER_LIGHT_S,
    AWB_PREFER_LIGHT_NUM
} AWB_PREFER_LIGHT_ENUM_T;

// Parent block weighting table index
typedef enum
{
    AWB_PB_WEIGHT_LUT_LOW = 0,
    AWB_PB_WEIGHT_LUT_MID,
    AWB_PB_WEIGHT_LUT_HIGH,
    AWB_PB_WEIGHT_LUT_NUM
} AWB_PB_WEIGHT_LUT_INDEX_ENUM_T;

// Parent block weighting method
typedef enum
{
    AWB_PB_WEIGHT_METHOD_DISABLE = 0,
    AWB_PB_WEIGHT_METHOD_LINEAR,
    AWB_PB_WEIGHT_METHOD_LUT_CT,
    AWB_PB_WEIGHT_METHOD_LUT_LV,
    AWB_PB_WEIGHT_METHOD_NUM
} AWB_PB_WEIGHT_METHOD_ENUM_T;

// Extra Color Mode
typedef enum
{
    AWB_EXTRA_COLOR_MODE_GAIN = 0,
    AWB_EXTRA_COLOR_MODE_P2,
    AWB_EXTRA_COLOR_MODE_DL_PROB,
    AWB_EXTRA_COLOR_MODE_NUM
} AWB_EXTRA_COLOR_MODE_ENUM_T;

// Face Preference
typedef enum
{
    AWB_FACE_PREF_REF_LOW = 0,
    AWB_FACE_PREF_REF_MID,
    AWB_FACE_PREF_REF_HIGH,
    AWB_FACE_PREF_REF_NUM
}AWB_FACE_PREF_REF_ENUM_T;

typedef enum
{
    AWB_FACE_PREF_CT_LOW = 0,
    AWB_FACE_PREF_CT_MID,
    AWB_FACE_PREF_CT_HIGH,
    AWB_FACE_PREF_CT_NUM
}AWB_FACE_PREF_CT_CLASS_ENUM_T;

// Face Assistance
typedef enum
{
    AWB_FACE_ASSIST_REF_LOW = 0,
    AWB_FACE_ASSIST_REF_MID,
    AWB_FACE_ASSIST_REF_HIGH,
    AWB_FACE_ASSIST_REF_NUM
}AWB_FACE_ASSIST_REF_ENUM_T;

// AWB gain
#ifndef AWBGAINT
#define AWBGAINT
typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
} AWB_GAIN_T;
#endif

// AWB gain
typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4B; // B gain
} AWB_GAIN_N_T;

// AWB bayer value
typedef struct
{
    MINT32 i4R;  // R  value
    MINT32 i4Gr; // Gr value
    MINT32 i4Gb; // Gb value
    MINT32 i4B;  // B  value
} AWB_BAYER_VALUE_T;

// XY coordinate
typedef struct
{
    MINT32 i4X; // X
    MINT32 i4Y; // Y
} XY_COORDINATE_T;

// Light area
typedef struct
{
    MINT32 i4RightBound; // Right bound
    MINT32 i4LeftBound;  // Left bound
    MINT32 i4UpperBound; // Upper bound
    MINT32 i4LowerBound; // Lower bound
} LIGHT_AREA_T;

// AWB calibration data
typedef struct
{
    AWB_GAIN_T rUnitGain;       // Unit gain: WB gain of DNP (individual camera)
    AWB_GAIN_T rGoldenGain;     // Golden sample gain: WB gain of DNP (golden sample)
    AWB_GAIN_T rUnitGainM;      // Unit gain: WB gain of TL84 at 3 illuminants calibration, or ALIGHT at 2 illuminants calibration (individual camera)
    AWB_GAIN_T rGoldenGainM;    // Golden sample gain: WB gain of TL84 at 3 illuminants calibration, or ALIGHT at 2 illuminants calibration (golden sample)
    AWB_GAIN_T rUnitGainL;      // Unit gain: WB gain of ALIGHT at 3 illuminants calibration (individual camera)
    AWB_GAIN_T rGoldenGainL;    // Golden sample gain: WB gain of ALIGHT at 3 illuminants calibration (golden sample)
    AWB_GAIN_T rTuningUnitGain; // Unit gain of tuning sample (for debug purpose)
    AWB_GAIN_T rD65Gain;        // WB gain of D65 (golden sample)
    AWB_BAYER_VALUE_T rUnitValue;     // Unit value: bayer RGB value of DNP (individual camera)
    AWB_BAYER_VALUE_T rGoldenValue;   // Golden sample value: bayer RGB value of DNP (golden sample)
    AWB_BAYER_VALUE_T rUnitValueM;    // Unit value: bayer RGB value of TL84 at 3 illuminants calibration, or ALIGHT at 2 illuminants calibration (individual camera)
    AWB_BAYER_VALUE_T rGoldenValueM;  // Golden sample value: bayer RGB value of TL84 at 3 illuminants calibration, or ALIGHT at 2 illuminants calibration (golden sample)
    AWB_BAYER_VALUE_T rUnitValueL;    // Unit value: bayer RGB value of ALIGHT at 3  illuminants calibration (individual camera)
    AWB_BAYER_VALUE_T rGoldenValueL;  // Golden sample value: bayer RGB value of ALIGHT at 3 illuminants calibration (golden sample)
} AWB_CALIBRATION_DATA_T;


// Rotation matrix parameter
typedef struct
{
    MINT32 i4RotationAngle; // Rotation angle
    MINT32 i4Cos;           // cos
    MINT32 i4Sin;           // sin
} AWB_ROTATION_MATRIX_T;

// Daylight locus parameter
typedef struct
{
    MINT32 i4SlopeNumerator;   // Slope numerator
    MINT32 i4SlopeDenominator; // Slope denominator
} AWB_DAYLIGHT_LOCUS_T;

// Predictor parameter
typedef struct {
    AWB_GAIN_T rSpatial_L;
    AWB_GAIN_T rSpatial_H;
    AWB_GAIN_T rTemporal_General;
    AWB_GAIN_T rSpatial_CWFDF_L;
    AWB_GAIN_T rSpatial_CWFDF_H;
}AWB_PREDICTOR_GAIN_T;

// AWB daylight locus target offset ratio LUT for T / WF
typedef struct
{
    MINT32 i4LUTIn[AWB_DAYLIGHT_LOCUS_LUT_SIZE]; // Look-up table
    MINT32 i4LUTOut[AWB_DAYLIGHT_LOCUS_LUT_SIZE]; // Look-up table
} AWB_OFFSET_LUT_T;

typedef struct
{
    AWB_GAIN_N_T rDlOffsetGain[AWB_DAYLIGHT_LOCUS_LUT_SIZE];
    AWB_GAIN_N_T rMOffsetGain[AWB_LIGHT_GM_OFFSET_NUM];
    AWB_GAIN_N_T rGOffsetGain[AWB_LIGHT_GM_OFFSET_NUM];
} AWB_PREFER_GAIN_LUT_T;

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

typedef struct
{
    MINT32 i4Low;
    MINT32 i4Mid;
    MINT32 i4High;
} AWB_3_TURN_POINT_T;

typedef struct {
    AWB_THR_LOW_HIGH_T rNonReliable;
    AWB_THR_LOW_HIGH_T rLightSrc[AWB_LIGHT_NUM];
} AWB_PREDICTOR_LV_THR_T;

typedef struct
{
    MINT32 i4LUT[AWB_LV_INDEX_NUM];
} AWB_LV_LUT_T;

typedef struct
{
    MINT32 i4LUT[AWB_LV_N_INDEX_NUM];
} AWB_LV_N_LUT_T; //support negative LV

// AWB convergence parameter
typedef struct
{
    MINT32 i4Speed; // Convergence speed
    MINT32 i4StableThr; // Stable threshold
    MINT32 i4ToTagetEnable; //converge to target enable
} AWB_CONVERGENCE_PARAM_T;

//Nonneutral parent block statistics information
typedef struct
{
    MINT32 i4Mode; //0:use non-neutral block statistics, 1: use total CB average
}AWB_NONNEUTRAL_PARENT_BLK_PARAM_T;

// Neutral parent block number threshold
typedef struct
{
    MINT32 i4NonNeutral[AWB_LV_INDEX_NUM];	        // unit: %
    MINT32 i4NRThr[AWB_LIGHT_NUM][AWB_LV_INDEX_NUM]; // unit: %
} AWB_NEUTRAL_PARENT_BLK_NUM_THR_T;

#define AWB_CCT_ESTIMATION_LIGHT_SOURCE_NUM (5)

// CCT estimation
typedef struct
{
    MINT32 i4CCT[AWB_CCT_ESTIMATION_LIGHT_SOURCE_NUM];                // CCT
    MINT32 i4RotatedXCoordinate[AWB_CCT_ESTIMATION_LIGHT_SOURCE_NUM]; // Rotated X coordinate
} AWB_CCT_ESTIMATION_T;

typedef struct
{
    MINT32 i4IsRWBEnable;
    MINT32 i4RWBCof[RWB_COEF_SIZE];
    MINT32 i4RWBCofInv[RWB_COEF_SIZE];
} AWB_NONE_BAYER_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4ErrCntThr;
}AWB_LINEAR_AAO_PARAM_T;

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

typedef struct
{
    MINT32 i4Hue;
    MINT32 i4Sat;
} AWB_HS_STAT_T;

typedef struct
{
    MINT32 i4Factor;  //0: disable luma-NR, others: factor value
} AWB_FACE_LUMA_NR_T;

typedef struct
{
    MINT32 i4DiffThr;

    AWB_THR_LOW_HIGH_T rLandmarkCVThr;
    AWB_THR_LOW_HIGH_T rLandmarkProb;

    AWB_3_TURN_POINT_T rDistVarThr;
    AWB_3_TURN_POINT_T rFeatureCVProb;
} AWB_FACE_CHROMA_NR_T;

typedef struct
{
    MINT32 i4Mode;
    
    AWB_FACE_LUMA_NR_T rLuma;
    AWB_FACE_CHROMA_NR_T rChroma;    
    
} AWB_FD_STAT_NR_T;

typedef struct
{
    MUINT32 u4LowBound;
    MUINT32 u4HiBound;
} AWB_FD_RGB_BOUNDARY_T;

typedef struct
{
    MINT32 i4Low;
    MINT32 i4Hi;
    MINT32 i4Range;
} AWB_FACEPREF_BPF_T; //Band Pass Filter

typedef struct
{
    MINT32 i4RefHS_GThr[AWB_FACE_PREF_REF_NUM];
    AWB_HS_STAT_T rRefHS[AWB_LIGHT_NUM][AWB_FACE_PREF_REF_NUM];
    MINT32 i4TempoWeight;
} AWB_FACEPREF_REF_TARGET_T;

typedef struct
{
    MINT32 i4LVLow;
    MINT32 i4LVHi;
    MINT32 i4FaceRawXrR_HM;
    MINT32 i4FaceRawXrL_HM;
    MINT32 i4FaceRawXrR_ML;
    MINT32 i4FaceRawXrL_ML;
    MINT32 i4FaceRawYrU;
    MINT32 i4FaceRawYrD;
} AWB_FACEPREF_SCENE_JUDGE_T;

typedef struct
{
    MINT32 i4TargetConvergeCtrlEn;
    AWB_HS_STAT_T rTOL;
	AWB_HS_STAT_T rRestrictRatio_TOL;
} AWB_FACEPREF_CONVERGE_CTRL_T;

typedef struct
{
    MINT32 i4LVLow;
    MINT32 i4LVHi;
    MINT32 i4OverSatProb_Low;
    MINT32 i4OverSatProb_Hi;
} AWB_FACEPREF_OVERSAT_LV_T;

typedef struct
{
    MINT32 i4CT_P_Low;
    MINT32 i4CT_P_Hi;
    MINT32 i4OverSatProb_Low;
    MINT32 i4OverSatProb_Hi;
} AWB_FACEPREF_OVERSAT_CT_T;

typedef struct
{
    AWB_FACEPREF_OVERSAT_LV_T rLV;
    AWB_FACEPREF_OVERSAT_CT_T rHiCT;
    AWB_FACEPREF_OVERSAT_CT_T rMidCT;
    AWB_FACEPREF_OVERSAT_CT_T rLowCT;
} AWB_FACEPREF_OVERSAT_T;

typedef struct
{
    MINT32 i4SatHi;
    MINT32 i4SatLow;
    MINT32 i4NewRestrict;
    AWB_FACEPREF_BPF_T rBPF_LV;
} AWB_FACEPREF_UNDERSAT_T;

typedef struct
{
    MINT32 i4FaceSizeRatioLow;
    MINT32 i4FaceSizeRatioHi;
    MINT32 i4Prob0_Min;
    MINT32 i4Prob0_Max;
} AWB_FACEPREF_PROB0_T;

typedef struct
{
    AWB_LV_LUT_T rLightSrc[AWB_LIGHT_NUM];
} AWB_FACEPREF_PROB1_T;

typedef struct
{
    AWB_FACEPREF_BPF_T rBPF_FaceRG;
    MINT32 i4Prob2_Min;
    MINT32 i4Prob2_Max;
} AWB_FACEPREF_PROB2_T;

typedef struct
{
    AWB_FACEPREF_BPF_T rBPF_FaceBG;
    MINT32 i4Prob3_Min;
    MINT32 i4Prob3_Max;
} AWB_FACEPREF_PROB3_T;

typedef struct
{
    AWB_FACEPREF_BPF_T rBPF_FaceG;
    MINT32 i4Prob4_Min;
    MINT32 i4Prob4_Max;
} AWB_FACEPREF_PROB4_T;

typedef struct
{
    MINT32 i4FaceSizeRatioHi;
    MINT32 i4FaceSizeRatioLow;
    MINT32 i4Prob0_Min;
    MINT32 i4Prob0_Max;
} AWB_FACEPREF_SPATIAL_PROB0_T;

typedef struct
{
    MINT32 i4NeutralParentBlkNum_Hi;
    MINT32 i4NeutralParentBlkNum_Low;
    MINT32 i4Prob1_Min;
    MINT32 i4Prob1_Max;
} AWB_FACEPREF_SPATIAL_PROB1_T;

typedef struct
{
    AWB_FACEPREF_SPATIAL_PROB0_T rP0;
    AWB_FACEPREF_SPATIAL_PROB1_T rP1;
    MINT32 i4LVLow;
    MINT32 i4LVHi;
} AWB_FACEPREF_SPATIAL_T;

typedef struct
{
    MINT32 i4LVChangeTh;
    MINT32 i4AWBGainChangeTh;
} AWB_FACE_SCENE_CHANGE_T;

typedef struct
{
    MINT32 i4Speed;
    MINT32 i4MinStep;
    MINT32 i4ProbReduceStep;
} AWB_FACE_TEMPO_SMOOTH_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4NoPortraitTh;
    MINT32 i4IsPortraitTh;
    MINT32 i4FrameDelay;
} AWB_FACEPREF_PORTRAIT_DELAY_T;

typedef struct
{
    MINT32 i4Enable;
    AWB_HS_STAT_T rTargetOFS;
    MINT32 i4OversatProbRatio;
} AWB_FACEPREF_GENDER_TARGET_T;

typedef struct
{
    MINT32 i4StatAvoidFaceArea;
    MINT32 i4FaceWinRatio;
    MUINT32 u4CentralWeight;
    MUINT32 u4LandmarkWeight;
    AWB_FD_STAT_NR_T rStatisticNR;
    AWB_FD_RGB_BOUNDARY_T rFD_RGB_Bound;
    AWB_FACE_TEMPO_SMOOTH_T rTempoSmooth;
    AWB_FACE_SCENE_CHANGE_T rSceneChange;
}AWB_FACE_CMN_SETTING_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4SatTargetEn;
    AWB_FACEPREF_REF_TARGET_T rRefTarget;
    AWB_FACEPREF_SCENE_JUDGE_T rSceneJudge;
    AWB_FACEPREF_CONVERGE_CTRL_T rConvergeCtrl;
    AWB_FACEPREF_OVERSAT_T rOversat;
    AWB_FACEPREF_UNDERSAT_T rUndersat;
    MINT32 i4GainRatioRestrictLUT[AWB_LV_N_INDEX_NUM];
    AWB_GAIN_T rPrefGain[AWB_LV_INDEX_NUM];
    AWB_FACEPREF_PROB0_T rProb0;
    AWB_FACEPREF_PROB1_T rProb1;
    AWB_FACEPREF_PROB2_T rProb2;
    AWB_FACEPREF_PROB3_T rProb3;
    AWB_FACEPREF_PROB4_T rProb4;
    AWB_FACEPREF_SPATIAL_T rSpatial;
    AWB_FACEPREF_PORTRAIT_DELAY_T rPortrait;
    AWB_FACEPREF_GENDER_TARGET_T rGender;
} AWB_FACE_PREFERENCE_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4StatWinShrinkOffset[AWB_LIGHT_NUM];
    MINT32 Reserved1;
    MINT32 Reserved2;
    MINT32 Reserved3;
    MINT32 Reserved4;
    MINT32 Reserved5;
} AWB_SMOOTH_STATISTIC_T;

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
    MINT32 i4Mode; // 0 : disable statistic limit, 1 : average statistic projection, 2 : PB statistic projection
    MINT32 i4GlevelRange; //used to do Glevel smooth on project weight, only work in mode 2
    AWB_THR_LOW_HIGH_T rLv;
    MINT32 i4LimitY[AWB_LIGHT_NUM];
    MINT32 i4WeightReduce[AWB_LIGHT_NUM];
    MINT32 i4ProjWeight[AWB_LIGHT_NUM];
    AWB_THR_LOW_HIGH_T rGlevel[AWB_LIGHT_NUM]; //only work in mode 2
} AWB_STATISTIC_LIMIT_T;

typedef struct
{
    MINT32 i4GeneralMode;        // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
    MINT32 i4NonReliableMode;    // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
    MINT32 i4TempInitMode;       // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
    MINT32 i4ThrTemp;
    MINT32 i4TempLv[AWB_LIGHT_NUM];
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
    MINT32 i4ModeWeight[AWB_EXTRA_COLOR_MODE_NUM];
    MINT32 i4ConfThr;
    MINT32 i4SelLightSrc[AWB_LIGHT_NUM];
    MINT32 i4LvRange;
    MINT32 i4GainCompWeight;
    AWB_GAIN_T rGain;
    LIGHT_AREA_T rArea;
    AWB_THR_LOW_HIGH_T rGlevel;
    AWB_THR_LOW_HIGH_T rLv;
    AWB_THR_LOW_HIGH_T rCount;
    AWB_THR_LOW_HIGH_T rWeighting;
} AWB_EXTRACOLOR_PROP_T;

typedef struct
{
    MINT32 i4Weight; 
    MINT32 i4GlevelRange;
    MINT32 i4LvRange;
    LIGHT_AREA_T rArea;
    AWB_THR_LOW_HIGH_T rGlevel;
    AWB_THR_LOW_HIGH_T rLv;
} AWB_EXCLUDE_COLOR_T;

typedef struct
{
    MINT32 i4XrIndex;
    AWB_GAIN_N_T rGain;
}AWB_SKYCOLOR_GAIN_T;


typedef struct
{
    MINT32 i4Enable;
    MINT32 i4ConfThr;
    MINT32 i4LvRange;
    AWB_SKYCOLOR_GAIN_T rSkyGain[AWB_SKY_GAIN_NUM];
    LIGHT_AREA_T rArea;
    AWB_THR_LOW_HIGH_T rGlevel;
    AWB_THR_LOW_HIGH_T rLv;
    AWB_THR_LOW_HIGH_T rCount;
    AWB_THR_LOW_HIGH_T rWeighting;
} AWB_SKYCOLOR_PROP_T;

typedef struct {
    AWB_SUNSET_PROP_T rSunsetProp;
    AWB_SUBWIN_PROP_T rSubWinPropF;
    AWB_SUBWIN_PROP_T rSubWinPropCWF;
    AWB_EXTRACOLOR_PROP_T rExtraColorProp[AWB_EXTRA_COLOR_NUM];
    AWB_EXCLUDE_COLOR_T rExcludeColor[AWB_EXCLUDE_COLOR_NUM];
    AWB_SKYCOLOR_PROP_T rSkyColorProp;    
} AWB_FEATURE_RROP_T;

typedef struct {
    MINT32 i4Slope;
    MINT32 i4XrOffsetThr;
    AWB_LV_LUT_T rProb;
}AWB_NONNEUTRAL_PROB_T;

// Preference color
typedef struct
{
    MINT32 i4OffsetThr[AWB_PREFER_LIGHT_NUM];   // Offset threshold
    AWB_THR_LOW_HIGH_T rOffsetRatioLv[AWB_PREFER_LIGHT_NUM];
    AWB_OFFSET_RATIO_LUT_T rOffsetRatio[AWB_PREFER_LIGHT_NUM];
    AWB_OFFSET_LUT_T rOffsetLUTs[AWB_PREFER_LIGHT_NUM];
    AWB_GM_OFFSET_THR_LUT_T rGMOffsetThr[AWB_PREFER_LIGHT_NUM];
    AWB_LIGHT_WEIGHT_LUT_T rLightWeight[AWB_PREFER_LIGHT_NUM];
    AWB_PREFER_GAIN_LUT_T rPreferGain[AWB_PREFER_LIGHT_NUM];
} AWB_PREFERENCE_COLOR_T;

// Face Assisted AWB
typedef struct
{
    MINT32 i4Mode;
    MINT32 i4WeightCoef_a;
    MINT32 i4WeightCoef_b;
    MINT32 i4RefTargetThr[AWB_FACE_ASSIST_REF_NUM];
    XY_COORDINATE_T rRefFaceXY[AWB_CAL_LIGHT_SRC_NUM][AWB_FACE_ASSIST_REF_NUM];
}AWB_FACEAST_REF_TARGET_T;

typedef struct
{
    MINT32 i4FaceSizeRatioLow;
    MINT32 i4FaceSizeRatioHi;
    MINT32 i4ProbMin;
    MINT32 i4ProbMax;
}AWB_FACEAST_PROB0_T;

typedef struct
{
    AWB_LV_LUT_T rNonNeutralProb1;
    AWB_LV_LUT_T rLightProb1[AWB_LIGHT_NUM];

}AWB_FACEAST_PROB1_T;

typedef struct
{
    AWB_LIGHT_WEIGHT_LUT_T rHiCT;
    AWB_LIGHT_WEIGHT_LUT_T rMidCT;
    AWB_LIGHT_WEIGHT_LUT_T rLowCT;
}AWB_FACEAST_PROB2_T;

typedef struct
{
    MINT32 i4StableConfLow;
    MINT32 i4StableConfHi;
    MINT32 i4ProbMin;
    MINT32 i4ProbMax;
}AWB_FACEAST_PROB3_T;

typedef struct
{
    MINT32 i4FaceColorRatioLow;
    MINT32 i4FaceColorRatioHi;
    MINT32 i4ProbMin;
    MINT32 i4ProbMax;
}AWB_FACEAST_PROB4_T;

typedef struct
{
    AWB_THR_LOW_HIGH_T  rConfThr;
    AWB_3_TURN_POINT_T  rDistanceThr;
    AWB_3_TURN_POINT_T  rCompRatio;
}AWB_FACEAST_COMP_T;

typedef struct
{
    MINT32 i4TempoWeight;
    MINT32 i4DelayFrm;
}AWB_FACEAST_STBLE_T;

typedef struct
{
    MINT32 i4DistType;
    MINT32 i4Resv02;
    MINT32 i4Resv03;
    MINT32 i4Resv04;
    MINT32 i4Resv05;
    MINT32 i4Resv06;
}AWB_FACEAST_ADVANCE_T;

typedef struct
{
    MINT32 i4Enable;
    MINT32 i4TOL[AWB_CAL_LIGHT_SRC_NUM];
    AWB_FACEAST_REF_TARGET_T rRefTarget;
    AWB_GAIN_N_T rPrefGain[AWB_CAL_LIGHT_SRC_NUM];
    AWB_FACEAST_PROB0_T rProb0;
    AWB_FACEAST_PROB1_T rProb1;
    AWB_FACEAST_PROB2_T rProb2;
    AWB_FACEAST_PROB3_T rProb3;
    AWB_FACEAST_PROB4_T rProb4;
    AWB_FACEAST_COMP_T rCompSetting;
    AWB_FACEAST_STBLE_T rStableSetting;

    AWB_FACEAST_ADVANCE_T rAdvSetting;    //rReserved
} AWB_FACE_ASSIST_T;

typedef struct
{
    MINT32 i4PBRatioH;
    MINT32 i4PBRatioL;
    MINT32 i4ErrCntRatioL;
    MINT32 i4ErrCntRatioH;
    MINT32 i4RefineP1L;
    MINT32 i4RefineP1H;
} AWB_PANEL_P0_T;

typedef struct
{
    MINT32 i4YDiffTh;
    MINT32 i4RgainDiffRatioTh;
} AWB_PANEL_SECNE_TRIGGER_T;

typedef struct
{
    MINT32 i4LV;
    AWB_PANEL_SECNE_TRIGGER_T rTrigger;
    MINT32 i4TrackingYDiffTh;
} AWB_PANEL_SECNE_T;

typedef struct
{
    MINT32 i4Max1;
    MINT32 i4Max2;
} AWB_PANEL_GAIN_EFFECT_T;

typedef struct
{
    MINT32 i4Max1;
    MINT32 i4Max2;
} AWB_PANEL_SPEED_EFFECT_T;

typedef struct
{
    MINT32 i4XYDiffLow;
    MINT32 i4XYDiffHi;
    AWB_PANEL_GAIN_EFFECT_T rGain;
    AWB_PANEL_SPEED_EFFECT_T rSpeed;
} AWB_PANEL_STRENGTH_T;

typedef struct
{
    MINT32 i4Ratio1;
    MINT32 i4Ratio2;
    MINT32 i4ProbLow;
    MINT32 i4ProbHi;
} AWB_PANEL_SAT_RATIO_CTRL_T;

typedef struct
{
    MINT32 i4Resv01;
    MINT32 i4Resv02;
    MINT32 i4Resv03;
    MINT32 i4Resv04;
    MINT32 i4Resv05;
    MINT32 i4Resv06;
    MINT32 i4Resv07;
}AWB_PANEL_RESERVED;

typedef struct
{
    MINT32 i4Enable;
    AWB_PANEL_P0_T rP0;
    AWB_PANEL_SECNE_T rScene;
    AWB_PANEL_STRENGTH_T rStr;
    AWB_PANEL_SAT_RATIO_CTRL_T rSatRatioCtrl;
    AWB_PANEL_RESERVED rRsv;
} AWB_PANEL_T;

typedef struct
{
    AWB_REFINEMENT_T            rRefinement;
    AWB_STATISTIC_LIMIT_T       rStatLimit;
    AWB_FACE_CMN_SETTING_T      rFaceCommon;
    AWB_FACE_PREFERENCE_T       rFacePref;
    AWB_FACE_ASSIST_T           rFaceAssist;
    AWB_SMOOTH_STATISTIC_T      rSmoothStatistic;
    AWB_PANEL_T                 rPanel;
} AWB_ADV_FUNC_T;

// Chip dependent parameter
typedef struct
{
    MINT32 i4AWBGainOutputScaleUnit; // AWB gain output scale unit
    MINT32 i4AWBGainOutputUpperLimit; // AWB gain output upper limit
    MINT32 i4RotationMatrixUnit; // Rotation matrix unit
} AWB_CHIP_PARAM_T;

// Parent block weight parameter used in light source statistics
typedef struct
{
    MINT32 i4Mode; // Parent block weight mode selection : 0:Disable 1:Linear Weighting 2:Weighting LUT
    MINT32 i4ScalingFactor; // 6: 1~12, 7: 1~6, 8: 1~3, 9: 1~2, >=10: 1
    MINT32 i4LvThld[AWB_PB_WEIGHT_LUT_NUM];
    MINT32 i4GammaLut[AWB_PB_WEIGHT_LUT_INDEX_NUM];
    MINT32 i4WeightLut[AWB_PB_WEIGHT_LUT_NUM][AWB_PB_WEIGHT_LUT_INDEX_NUM];
    MINT32 i4NonneutralPBMode; //0:use non-neutral PB sum, 1:use all PB average    
} AWB_PARENT_BLOCK_WEIGHT_T;

// AWB one-shot parameter
typedef struct
{
    MBOOL bSmoothEnable; // Enable smooth one-shot AWB for dark environment: take weighted average of one-shot AWB gain and preview AWB gain based on scene LV
    MINT32 i4LVThrL; // Low LV threshold: take preview AWB gain only when scene LV <= i4LVThrL
    MINT32 i4LVThrH; // High LV threshold: take one-shot AWB gain only when scene LV >= i4LVThrH
                     // Perform interpolation when i4LVThrH > LV > i4LVThrL
} AWB_ONE_SHOT_T;

//MWB window size parameter
typedef struct
{
    MINT32 i4WindowSizeX;
    MINT32 i4WindowSizeY;
}AWB_MWB_PARAM_T;

// Misc parameter
typedef struct
{
    MINT32 i4CctGainSel;
    MINT32 i4StrobePbReduceX;
    MINT32 i4StrobePbReduceY;
} AWB_MISC_PARAM_T;

typedef struct {
    AWB_CALIBRATION_DATA_T rCalData; // AWB calibration data
    XY_COORDINATE_T rOriginalXY[AWB_CAL_LIGHT_SRC_NUM]; // Original XY coordinate of AWB light source
    XY_COORDINATE_T rRotatedXY[AWB_CAL_LIGHT_SRC_NUM]; // Rotated XY coordinate of AWB light source
    AWB_GAIN_T rLightAWBGain[AWB_CAL_LIGHT_SRC_NUM]; // AWB gain of AWB light source
    AWB_ROTATION_MATRIX_T rRotationMatrix; // Rotation matrix parameter
    AWB_DAYLIGHT_LOCUS_T rDaylightLocus; // Daylight locus parameter
    AWB_PREDICTOR_GAIN_T rPredictorGain;	// Spatial / Temporal predictor gain
    LIGHT_AREA_T rAWBLightArea[AWB_LIGHT_NUM]; // AWB light area
    LIGHT_AREA_T rPWBReferenceArea; // PWB light Reference area
    LIGHT_AREA_T rPWBLightArea[PWB_LIGHT_NUM]; // PWB light area
    AWB_GAIN_T rPWBDefaultGain[PWB_LIGHT_NUM]; // PWB default gain
    AWB_CCT_ESTIMATION_T rCCTEstimation; // CCT estimation
    AWB_NONE_BAYER_T rNoneBayer;
    AWB_LINEAR_AAO_PARAM_T rLinearAaoParam;
}AWB_ALGO_CAL_T;

typedef struct {
    AWB_GAIN_T rLightPrefGain[AWB_LIGHT_NUM][AWB_LV_N_INDEX_NUM];
    AWB_PREDICTOR_LV_THR_T rPredictorLVThr;
    AWB_TEMPORAL_ENQUEUE_THR_T rTemporalEnqueueThr;
    AWB_NEUTRAL_PARENT_BLK_NUM_THR_T rNeutralBlkThr;
    AWB_FEATURE_RROP_T rFeatureProp;
    AWB_NONNEUTRAL_PROB_T rNonNeutralProb;
    AWB_LV_N_LUT_T rDaylightLocusProb[AWB_LIGHT_NUM];
    AWB_LV_N_LUT_T rLightProb1[AWB_LIGHT_NUM];
    AWB_CONVERGENCE_PARAM_T rConvergence;
    AWB_PREFERENCE_COLOR_T rPreferColor;
    AWB_ADV_FUNC_T rAdvFunction;
    AWB_CHIP_PARAM_T rChipParam;
    AWB_PARENT_BLOCK_WEIGHT_T rParentBlkWeightParam;
    AWB_ONE_SHOT_T rOneShotParam;
    AWB_MWB_PARAM_T rMwbParam;
    AWB_MISC_PARAM_T rMiscParam;
    AWB_TUNING_INFO_T rTuningInfo;
}AWB_ALGO_TUNING_T;

// AWB statistics parameter
typedef struct
{
    // Number of AWB windows
    MINT32 i4WindowNumX; // Number of horizontal AWB windows
    MINT32 i4WindowNumY; // Number of vertical AWB windows

    // Thresholds
    MINT32 i4LowThresholdR;  // Low threshold of R
    MINT32 i4LowThresholdG;  // Low threshold of G
    MINT32 i4LowThresholdB;  // Low threshold of B
    MINT32 i4HighThresholdR; // High threshold of R
    MINT32 i4HighThresholdG; // High threshold of G
    MINT32 i4HighThresholdB; // High threshold of B

    MINT32 i4LightSrcLowThresholdR;  // Low threshold of R for light source estimation
    MINT32 i4LightSrcLowThresholdG;  // Low threshold of G for light source estimation
    MINT32 i4LightSrcLowThresholdB;  // Low threshold of B for light source estimation
    MINT32 i4LightSrcHighThresholdR; // High threshold of R for light source estimation
    MINT32 i4LightSrcHighThresholdG; // High threshold of G for light source estimation
    MINT32 i4LightSrcHighThresholdB; // High threshold of B for light source estimation

    // Pre-gain maximum limit clipping
    MINT32 i4PreGainLimitR; // Maximum limit clipping for R color
    MINT32 i4PreGainLimitG; // Maximum limit clipping for G color
    MINT32 i4PreGainLimitB; // Maximum limit clipping for B color

    // AWB error threshold
    MINT32 i4ErrorThreshold; // Programmable threshold for the allowed total
                             // over-exposured and under-exposered pixels in one main stat window

    // AWB error count shift bits
    MINT32 i4ErrorShiftBits; // Programmable error count shift bits: 0 ~ 7
                             // Note: AWB statistics provide 4-bits error count output only

    // AWB error pixel ratio
    MINT32 i4ErrorRatio; // Programmable error pixel count by AWB window size (base : 256)

    // AWB motion error pixel ratio
    MINT32 i4MoErrorRatio; // Programmable motion error pixel count by AWB window size (base : 256)

    // AWB output mode select flag
    MINT32 i4LinearOutputEn; // 1: Linear mode, 0: Non-linear mode

} AWB_STAT_PARAM_T;

// AWB NVRAM structure
typedef struct
{
    AWB_ALGO_CAL_T      rAlgoCalParam;      // AWB param by calibration
    AWB_ALGO_TUNING_T	rAlgoTuningParam;	// AWB param by tuning
    AWB_STAT_PARAM_T    rStatParam;         // AWB statistics parameter
} AWB_NVRAM_T;

#endif // _CAMERA_CUSTOM_AWB_NVRAM_H_
