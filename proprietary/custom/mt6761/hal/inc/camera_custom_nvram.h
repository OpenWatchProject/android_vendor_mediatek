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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _CAMERA_CUSTOM_NVRAM_H_
#define _CAMERA_CUSTOM_NVRAM_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "ispif.h"
#include "CFG_Camera_File_Max_Size.h"
#include "camera_custom_AEPlinetable.h"

#include "camera_custom_af_nvram.h"
#include "camera_custom_tsf_tbl.h"

#define ZION_OLD_NVRAM 0
#define NVRAM_CUSTOM_REVISION 	5341001

using namespace NSIspTuning;

#define NVRAM_CAMERA_SHADING_FILE_VERSION       1
#define NVRAM_CAMERA_PARA_FILE_VERSION          1
#define NVRAM_CAMERA_3A_FILE_VERSION            1
#define NVRAM_CAMERA_LENS_FILE_VERSION          1
#define NVRAM_CAMERA_STROBE_FILE_VERSION          2 // SC MODIFY
#define NVRAM_CAMERA_FEATURE_FILE_VERSION          1
#define NVRAM_CAMERA_GEOMETRY_FILE_VERSION          1
#define NVRAM_CAMERA_PLINE_FILE_VERSION          1
#define NVRAM_CAMERA_AF_FILE_VERSION          1
#define NVRAM_CAMERA_FLASH_CALIBRATION_FILE_VERSION 1

#define MTK_STEREO_KERNEL_NVRAM_LENGTH (5800) // refer to the same define in MtkStereoKernel.h

/*******************************************************************************
* ANR table
********************************************************************************/
#define ANR_SUPPORT_CT_NUM              (4)
#define ANR_SUPPORT_ISO_NUM             (3)
#define ANR_HW_TBL_NUM                  (256)

/*******************************************************************************
* shading
********************************************************************************/
#define SHADING_SUPPORT_CT_NUM          (4)
#define SHADING_SUPPORT_OP_NUM          (10)
#define SHADING_SUPPORT_CH_NUM          (4)
#define MAX_FRM_GRID_NUM                (16)
#define MAX_TIL_GRID_NUM                (32)
#define COEFF_BITS_PER_CH               (128)
#define COEFF_PER_CH_U32                (COEFF_BITS_PER_CH>>5)
#define COEFF_PER_CH_U8                 (COEFF_BITS_PER_CH>>3)
#define MAX_SHADING_SIZE                (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1024) //INT32
#define MAX_SHADING_CapFrm_SIZE         (MAX_FRM_GRID_NUM*MAX_FRM_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(4096) //INT32
#define MAX_SHADING_CapTil_SIZE         (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(4096) //INT32
#define MAX_SHADING_PvwFrm_SIZE         (MAX_FRM_GRID_NUM*MAX_FRM_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1600) //INT32
#define MAX_SHADING_PvwTil_SIZE         (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1600) //INT32
#define MAX_SHADING_VdoFrm_SIZE         (MAX_FRM_GRID_NUM*MAX_FRM_GRID_NUM*SHADING_SUPPORT_CH_NUM*COEFF_PER_CH_U32)//(1600) //INT32
#define MAX_SVD_SHADING_SIZE            (MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM*SHADING_SUPPORT_CH_NUM*sizeof(UINT32))//(1024) //Byte
#define MAX_SENSOR_CAL_SIZE             (2048)//(1024) //Byte

#define member_size(type, member) sizeof(((type *)0)->member)
#define struct_size(type, start, end) \
    ((offsetof(type, end) - offsetof(type, start) + member_size(type, end)))

#define SIZEOF  sizeof

typedef struct {
    MUINT8     PixId; //0,1,2,3: B,Gb,Gr,R
    MUINT32    SlimLscType; //00A0  FF 00 02 01 (4 bytes)       4
    MUINT16    Width; //00A8    Capture Width (2 bytes) Capture Height (2 bytes)    2
    MUINT16    Height; //00A8    Capture Width (2 bytes) Capture Height (2 bytes)    2
    MUINT16    OffsetX; //00AA    Capture Offset X (2 bytes)  Capture Offfset Y (2 bytes) 2
    MUINT16    OffsetY; //00AA    Capture Offset X (2 bytes)  Capture Offfset Y (2 bytes) 2
    MUINT32    TblSize; //00B0   Capture Shading Table Size (4 bytes)        4
    MUINT32    IspLSCReg[5]; //00C8 Capture Shading Register Setting (5x4 bytes)        20
    MUINT8     GainTable[2048]; //00DC   Capture Shading Table (16 X 16 X 2 X 4 bytes)       2048
    MUINT8     UnitGainTable[2048]; //2048
} SHADING_GOLDEN_REF;

/* according to sensor mode
typedef enum
{
    ESensorMode_Preview    = 0,
    ESensorMode_Capture,
    ESensorMode_Video,
    ESensorMode_SlimVideo1,
    ESensorMode_SlimVideo2,
    ESensorMode_NUM
} ESensorMode_T;
*/

#define SHADING_DATA                                                                \
    struct {                                                                        \
        UINT32 Version;                                                             \
        UINT32 SensorId;                                                            \
        UINT16 LSCSize[SHADING_SUPPORT_OP_NUM];                                     \
        UINT32 PrvTable[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 CapTable[SHADING_SUPPORT_CT_NUM][MAX_SHADING_CapTil_SIZE];           \
        UINT32 VdoTable[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Sv1Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Sv2Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Cs1Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Cs2Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Cs3Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Cs4Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        UINT32 Cs5Table[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
        SHADING_GOLDEN_REF SensorGoldenCalTable;                                    \
        }


struct _ISP_SHADING_STRUCT
{
    SHADING_DATA;
};

typedef struct
{
    SHADING_DATA;
    UINT8 CameraData[MAXIMUM_CAMERA_SHADING_SIZE-
                     sizeof(struct _ISP_SHADING_STRUCT)];
} ISP_SHADING_STRUCT, *PISP_SHADING_STRUCT;

typedef struct
{
   	ISP_SHADING_STRUCT  Shading;
} NVRAM_CAMERA_SHADING_STRUCT, *PNVRAM_CAMERA_SHADING_STRUCT;

/*******************************************************************************
* 3A
********************************************************************************/

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

// AE NVRAM Mapping Table by Scenario
static MUINT32 AENVRAMMapping[CAM_SCENARIO_NUM] = {
    static_cast<MUINT32>(CAM_SCENARIO_PREVIEW),   // PREVIEW
    static_cast<MUINT32>(CAM_SCENARIO_VIDEO),     // VIDEO
    static_cast<MUINT32>(CAM_SCENARIO_CAPTURE),   // CAPTURE
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM1),   // HDR
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM2),   // AUTO HDR
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM3),   // VT
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM4)    // STEREO
};
// AWB NVRAM Mapping Table by Scenario
   static MUINT32 AWBNVRAMMapping[CAM_SCENARIO_NUM] = {
   static_cast<MUINT32>(CAM_SCENARIO_PREVIEW),   // PREVIEW
   static_cast<MUINT32>(CAM_SCENARIO_VIDEO),     // VIDEO
   static_cast<MUINT32>(CAM_SCENARIO_CAPTURE),   // CAPTURE
   static_cast<MUINT32>(CAM_SCENARIO_CUSTOM1),   // HDR
   static_cast<MUINT32>(CAM_SCENARIO_CUSTOM2),   // AUTO HDR
   static_cast<MUINT32>(CAM_SCENARIO_CUSTOM3),   // VT
   static_cast<MUINT32>(CAM_SCENARIO_CUSTOM4)    // STEREO
};
//____AE NVRAM____

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
    UINT32 u4Cap2PreRatio;	    // 1x = 1024
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

	BOOL	bEnableNightSkySuppresion;
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
    MUINT32   u4AOE_OEBound;             // for Over expsosure boud	-> 184
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
#define AWB_LUT_SIZE		(19)
#define AWB_DAYLIGHT_LOCUS_LUT_SIZE		(21)
#define AWB_LIGHTSOURCE_NUM	(8)

// AWB gain
typedef struct
{
    INT32 i4R; // R gain
    INT32 i4G; // G gain
    INT32 i4B; // B gain
} AWB_GAIN_T;

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

// Preference color
typedef struct
{
    INT32 i4SliderValue; // Slider value
    INT32 i4OffsetThr;   // Offset threshold
} PREFERENCE_COLOR_T;

// AWB calibration data
typedef struct
{
    AWB_GAIN_T rUnitGain;      // Unit gain: WB gain of DNP (individual camera)
    AWB_GAIN_T rGoldenGain;    // Golden sample gain: WB gain of DNP (golden sample)
    AWB_GAIN_T rUnitGainM;      // Unit gain: WB gain of TL84 (individual camera)
    AWB_GAIN_T rGoldenGainM;    // Golden sample gain: WB gain of TL84 (golden sample)
    AWB_GAIN_T rUnitGainL;      // Unit gain: WB gain of ALIGHT (individual camera)
    AWB_GAIN_T rGoldenGainL;    // Golden sample gain: WB gain of ALIGHT (golden sample)
    AWB_GAIN_T rTuningUnitGain; // Unit gain of tuning sample (for debug purpose)
    AWB_GAIN_T rD65Gain;    // WB gain of D65 (golden sample)
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
	MINT32 i4PrefRatio100;
	AWB_GAIN_T rSpatial_DaylightLocus_L;
	AWB_GAIN_T rSpatial_DaylightLocus_H;
	AWB_GAIN_T rTemporal_General;
    AWB_GAIN_T rAWBGain_LSC;
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
	MINT32 i4LUTSize; // LUT dimension
	MINT32 i4LUTIn[AWB_DAYLIGHT_LOCUS_LUT_SIZE]; // Look-up table
	MINT32 i4LUTOut[AWB_DAYLIGHT_LOCUS_LUT_SIZE]; // Look-up table
} AWB_DAYLIGHT_LOCUS_TARGET_OFFSET_LUT_T;

typedef struct
{
	MINT32 i4BoundXrThr;
	MINT32 i4BoundYrThr;
} AWB_FEATURE_AREA_T;

// AWB preference color
typedef struct
{
    PREFERENCE_COLOR_T rTungsten;        // Tungsten
    PREFERENCE_COLOR_T rWarmFluorescent; // Warm fluorescent
    PREFERENCE_COLOR_T rShade;           // Shade
    AWB_FEATURE_AREA_T rSunsetArea;
    AWB_FEATURE_AREA_T rShadeFArea;
    AWB_FEATURE_AREA_T rShadeFVerTex;
    AWB_FEATURE_AREA_T rShadeCWFArea;
    AWB_FEATURE_AREA_T rShadeCWFVerTex;
} AWB_PREFERENCE_COLOR_T;


typedef struct {
	MINT32 i4Neutral_ParentBlk_Thr;
	MINT32 i4CWFDF_LUTThr[AWB_LUT_SIZE];
} AWB_TEMPORAL_ENQUEUE_THR_T;

typedef struct {
	MINT32 i4InitLVThr_L;
	MINT32 i4InitLVThr_H;
	MINT32 i4EnqueueLVThr;
} AWB_PREDICTOR_LV_THR_T;

typedef struct {
    MINT32 i4Enable;
	MINT32 i4LVThr_L;
    MINT32 i4LVThr_H;
	MINT32 i4SunsetCountThr;
	MINT32 i4SunsetCountRatio_L;
	MINT32 i4SunsetCountRatio_H;
} AWB_SUNSET_PROP_T;

typedef struct {
    MINT32 i4Enable;
	MINT32 i4LVThr_L;
    MINT32 i4LVThr_H;
	MINT32 i4DaylightProb;
} AWB_SHADE_PROP_T;

typedef struct {
	AWB_SUNSET_PROP_T rSunsetProp;
	AWB_SHADE_PROP_T rShadeFProp;
	AWB_SHADE_PROP_T rShadeCWFProp;
} AWB_FEATURE_RROP_T;

typedef struct
{
	MINT32 i4LUT[AWB_LUT_SIZE];	// look-up table for temporal and spatial weighting
}AWB_PROBABILITY_LUT_T;

// Parent block weight parameter used in light source statistics
typedef struct
{
	BOOL bEnable; // Enable parent block weight
	MINT32 i4ScalingFactor; // 6: 1~12, 7: 1~6, 8: 1~3, 9: 1~2, >=10: 1
} AWB_PARENT_BLOCK_WEIGHT_T;

// Neutral parent block number threshold

typedef struct
{
	MINT32 m_i4NonNeutral[AWB_LUT_SIZE];	// unit: %
    MINT32 m_i4F[AWB_LUT_SIZE]; 		 	// unit: %
	MINT32 m_i4CWF[AWB_LUT_SIZE];			// unit: %
	MINT32 m_i4Daylight[AWB_LUT_SIZE];		// unit: %
	MINT32 m_i4DF[AWB_LUT_SIZE];			// unit: %
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
    MINT32 i4ProjCode;
    MINT32 i4Model;
    MINT32 i4Date;
    MINT32 i4Res0;
    MINT32 i4Res1;
    MINT32 i4Res2;
    MINT32 i4Res3;
    MINT32 i4Res4;
} AWB_TUNING_INFO_T;


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
    AWB_PREFERENCE_COLOR_T rPreferenceColor; // AWB preference color
    AWB_CCT_ESTIMATION_T rCCTEstimation; // CCT estimation
}AWB_ALGO_CAL_T;

typedef struct {
	BOOL bAWBBackupEnable;
    AWB_DAYLIGHT_LOCUS_TARGET_OFFSET_LUT_T rDaylightLocusOffsetLUTs_Tungsten;
	AWB_DAYLIGHT_LOCUS_TARGET_OFFSET_LUT_T rDaylightLocusOffsetLUTs_WF;
	AWB_DAYLIGHT_LOCUS_TARGET_OFFSET_LUT_T rDaylightLocusOffsetLUTs_Shade;
    AWB_GAIN_T rLightPrefGain[AWB_LIGHTSOURCE_NUM][AWB_LUT_SIZE];
	AWB_PARENT_BLOCK_WEIGHT_T rParentBlkWeightParam;
	AWB_PREDICTOR_LV_THR_T rPredictorLVThr;
	AWB_TEMPORAL_ENQUEUE_THR_T rTemporalEnqueueThr;
	AWB_NEUTRAL_PARENT_BLK_NUM_THR_T rNeutralBlkThr;
	AWB_FEATURE_RROP_T rFeaturePropThr;
	AWB_PROBABILITY_LUT_T rNonNeutralProb;
	AWB_PROBABILITY_LUT_T rDaylightLocusProb[AWB_LIGHTSOURCE_NUM];
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
	MUINT32	location_map_th2; //default: 20
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

#define FlASH_AE_NUM_2 (4)
#define FlASH_AWB_NUM_2 (4)
#define FlASH_CALIBRATION_NUM_2 (4)

#define FLASH_CUSTOM_MAX_DUTY_NUM_HT (40) // Note, related to NVRAM spec
#define FLASH_CUSTOM_MAX_DUTY_NUM_LT (40) // Note, related to NVRAM spec
#define FLASH_CUSTOM_MAX_DUTY_NUM (FLASH_CUSTOM_MAX_DUTY_NUM_HT * FLASH_CUSTOM_MAX_DUTY_NUM_LT)

#define FLASH_DUTY_NUM FLASH_CUSTOM_MAX_DUTY_NUM

#define FLASH_LV_INDEX_UNIT (10) // 1.0 LV
#define FLASH_LV_INDEX_MIN  (0)  // LV 0
#define FLASH_LV_INDEX_MAX  (18) // LV 18
#define FLASH_LV_INDEX_NUM ((FLASH_LV_INDEX_MAX - FLASH_LV_INDEX_MIN) + 1)


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


//____3A NVRAM____

//typedef unsigned char  UINT8;

// AWB NVRAM index
typedef enum
{
	AWB_NVRAM_IDX_NORMAL = 0,
    AWB_NVRAM_IDX_VHDR,
	AWB_NVRAM_IDX_NUM
} AWB_NVRAM_IDX_T;

typedef struct
{
    //data structure version, update once structure been modified.
    UINT32 u4Version;

    // ID of sensor module
    UINT32 SensorId;

    //data content
    AE_NVRAM_T rAENVRAM[CAM_SCENARIO_NUM];
    AWB_NVRAM_T rAWBNVRAM[CAM_SCENARIO_NUM];
    FLASH_AWB_NVRAM_T rFlashAWBNVRAM;
    //SSS(reserved unused spaces(bytes)) = total-used;,
    //ex. SSS = 4096-sizeof(UINT32)--sizeof(NVRAM_AAA_T)-sizeof(NVRAM_bbb_T);
    //    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)-sizeof(AF_NVRAM_T)-sizeof(AWB_NVRAM_T)];
    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)-sizeof(AWB_NVRAM_T)*AWB_NVRAM_IDX_NUM-sizeof(FLASH_AWB_NVRAM_T)];
} NVRAM_CAMERA_3A_STRUCT, *PNVRAM_CAMERA_3A_STRUCT;

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

} NVRAM_FLASH_TUNING_PARA;

typedef struct CAMERA_PDTBL_STRUCT_t
{
    MUINT32 bpci_xsize;
    MUINT32 bpci_ysize;
    MUINT32 pdo_xsize;
    MUINT32 pdo_ysize;
    MUINT8* bpci_array;

} CAMERA_PD_TBL_STRUCT, *PCAMERA_PD_TBL_STRUCT;

typedef struct CAMERA_BPCI_STRUCT_t
{
    CAMERA_PD_TBL_STRUCT PDC_FULL_TBL;
    CAMERA_PD_TBL_STRUCT PDC_RSV_TBL;

} CAMERA_BPCI_STRUCT, *PCAMERA_BPCI_STRUCT;


typedef struct
{
	int exp;
	int afe_gain;
	int isp_gain;
	int distance;
#ifdef FLASHLIGHT_OFFSET_CALI
	short yTab[40*39];  //x128
    short dutyOffsetMA[40];
#else
	short yTab[40*40];  //x128
#endif

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
	int vBatL;	//mv
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

#define MAXIMUM_NVRAM_CAMERA_FLASH_CALIBRATION_FILE_SIZE   (12800)
typedef union
{
    struct
    {
        short yTab[FLASH_DUTY_NUM];
        AWB_GAIN_T flashWBGain[FLASH_DUTY_NUM]; // Flash AWB calibration data
    };
    UINT8 temp[MAXIMUM_NVRAM_CAMERA_FLASH_CALIBRATION_FILE_SIZE];

} NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT, *PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT;

/*******************************************************************************
* ISP NVRAM parameter
********************************************************************************/
#define NVRAM_DBS_TBL_NUM               (350)     //714forbuild
#define NVRAM_OBC_TBL_NUM               (70)
#define NVRAM_BNR_BPC_TBL_NUM           (350)
#define NVRAM_BNR_NR1_TBL_NUM           (330)
#define NVRAM_BNR_PDC_TBL_NUM           (330)               
#define NVRAM_LSC_TBL_NUM               (SHADING_SUPPORT_OP_NUM)
#define NVRAM_NSL2_TBL_NUM               (NVRAM_LSC_TBL_NUM*SHADING_SUPPORT_CT_NUM)
#define NVRAM_UDM_TBL_NUM               (351) // +1: default for disable
#define NVRAM_CCM_TBL_NUM               (4)
#define NVRAM_GGM_TBL_NUM               (5)
#define NVRAM_IHDR_GGM_TBL_NUM          (16)
#define NVRAM_ANR_TBL_NUM               (410)
#define NVRAM_ANR2_TBL_NUM              (410)
#define NVRAM_CCR_TBL_NUM               (70)
#define NVRAM_BOK_TBL_NUM               (50)
#define NVRAM_EE_TBL_NUM                (370)
#define NVRAM_NR3D_TBL_NUM              (240)
#define NVRAM_MFB_TBL_NUM               (20)
#define NVRAM_LCE_TBL_NUM               (10)
#define NVRAM_UDM_DISABLE_IDX           (NVRAM_UDM_TBL_NUM-1)

// camera common parameters and sensor parameters
typedef struct
{
    UINT32 CommReg[64];
} ISP_NVRAM_COMMON_STRUCT, *PISP_NVRAM_COMMON_STRUCT;

typedef struct ISP_NVRAM_REG_INDEX_STRUCT
{
    UINT16 DBS;
    UINT16 OBC;
    UINT16 BNR_BPC;
    UINT16 BNR_NR1;
    UINT16 BNR_PDC;    
    UINT16 LSC;
    UINT16 NSL2;
    UINT16 UDM;
    UINT16 CCM;
    UINT16 MDP_CCM;
    UINT16 GGM;
    UINT16 IHDR_GGM;
    UINT16 ANR;
    UINT16 ANR2;
    UINT16 CCR;
    UINT16 BOK;
    UINT16 EE;
    UINT16 NR3D;
    UINT16 MFB;
    UINT16 LCE;
}	ISP_NVRAM_REG_INDEX_T, *PISP_NVRAM_REG_INDEX_T;

typedef struct
{
    ISP_NVRAM_REG_INDEX_T       Idx;
    ISP_NVRAM_DBS_T             DBS[NVRAM_DBS_TBL_NUM];
    ISP_NVRAM_OBC_T             OBC[NVRAM_OBC_TBL_NUM];
    ISP_NVRAM_BNR_BPC_T         BNR_BPC[NVRAM_BNR_BPC_TBL_NUM];
    ISP_NVRAM_BNR_NR1_T         BNR_NR1[NVRAM_BNR_NR1_TBL_NUM];
    ISP_NVRAM_BNR_PDC_T         BNR_PDC[NVRAM_BNR_PDC_TBL_NUM];
    ISP_NVRAM_LSC_T             LSC[NVRAM_LSC_TBL_NUM];
    ISP_NVRAM_NSL2_T            NSL2[NVRAM_NSL2_TBL_NUM];
    ISP_NVRAM_UDM_T             UDM[NVRAM_UDM_TBL_NUM];
    ISP_NVRAM_CCM_T             CCM[NVRAM_CCM_TBL_NUM];
    ISP_NVRAM_CCM_T             MDP_CCM[NVRAM_CCM_TBL_NUM];
    ISP_NVRAM_GGM_T             GGM[NVRAM_GGM_TBL_NUM];
    ISP_NVRAM_GGM_T             IHDR_GGM[NVRAM_IHDR_GGM_TBL_NUM];
    ISP_NVRAM_ANR_T             ANR[NVRAM_ANR_TBL_NUM];
    ISP_NVRAM_ANR2_T            ANR2[NVRAM_ANR2_TBL_NUM];
    ISP_NVRAM_CCR_T             CCR[NVRAM_CCR_TBL_NUM];
    ISP_NVRAM_BOK_T             BOK[NVRAM_BOK_TBL_NUM];
    ISP_NVRAM_EE_T              EE[NVRAM_EE_TBL_NUM];
    ISP_NVRAM_NR3D_T            NR3D[NVRAM_NR3D_TBL_NUM];
    ISP_NVRAM_MFB_T             MFB[NVRAM_MFB_TBL_NUM];
    ISP_NVRAM_LCE_T             LCE[NVRAM_LCE_TBL_NUM];
} ISP_NVRAM_REGISTER_STRUCT, *PISP_NVRAM_REGISTER_STRUCT;

typedef struct
{
    MINT32 value[6];
} ISP_NVRAM_PCA_SLIDER_STRUCT, *PISP_NVRAM_PCA_SLIDER_STRUCT;

typedef struct
{
    ISP_NVRAM_PCA_SLIDER_STRUCT Slider;
    ISP_NVRAM_PCA_T        Config;
    ISP_NVRAM_PCA_LUTS_T   PCA_LUTS;
} ISP_NVRAM_PCA_STRUCT, *PISP_NVRAM_PCA_STRUCT;

typedef struct
{
    MINT32 i4R_AVG;
    MINT32 i4R_STD;
    MINT32 i4B_AVG;
    MINT32 i4B_STD;
    MINT32 i4R_MAX; // default = (1<<(M+N)) - 1; M = 3, N = 9
    MINT32 i4R_MIN; // default = 1<<N; M = 3, N = 9
    MINT32 i4G_MAX; // default = (1<<(M+N)) - 1; M = 3, N = 9
    MINT32 i4G_MIN; // default = 1<<N; M = 3, N = 9
    MINT32 i4B_MAX; // default = (1<<(M+N)) - 1; M = 3, N = 9
    MINT32 i4B_MIN; // default = 1<<N; M = 3, N = 9
    MINT32 i4P00[9];
    MINT32 i4P10[9];
    MINT32 i4P01[9];
    MINT32 i4P20[9];
    MINT32 i4P11[9];
    MINT32 i4P02[9];
} ISP_NVRAM_CCM_POLY22_STRUCT;

typedef struct
{
	AWB_GAIN_T rStrobe;   // Strobe
    AWB_GAIN_T rA;        // A
    AWB_GAIN_T rTL84;     // TL84
    AWB_GAIN_T rCWF;      // CWF
    AWB_GAIN_T rD65;      // D65
    AWB_GAIN_T rRSV1;     // Reserved 1
    AWB_GAIN_T rRSV2;     // Reserved 2
    AWB_GAIN_T rRSV3;     // Reserved 3
} ISP_NVRAM_CCM_AWB_GAIN_STRUCT;

typedef struct
{
	MINT32 i4Strobe;   // Strobe
    MINT32 i4A;        // A
    MINT32 i4TL84;     // TL84
    MINT32 i4CWF;      // CWF
    MINT32 i4D65;      // D65
    MINT32 i4RSV1;     // Reserved 1
    MINT32 i4RSV2;     // Reserved 2
    MINT32 i4RSV3;     // Reserved 3
} ISP_NVRAM_CCM_WEIGHT_STRUCT;

typedef struct
{
    ISP_NVRAM_CCM_POLY22_STRUCT   Poly22;
    ISP_NVRAM_CCM_AWB_GAIN_STRUCT AWBGain;
    ISP_NVRAM_CCM_WEIGHT_STRUCT   Weight;
} ISP_NVRAM_MULTI_CCM_STRUCT, *PISP_NVRAM_MULTI_CCM_STRUCT;

typedef struct
{
    // y = ax + b
    double a;
    double b;
} CoefLinear_T;

typedef struct
{
    CoefLinear_T S;
    CoefLinear_T O;
} DngNoiseProile_T;

typedef struct
{
    MINT32 i4RefereceIlluminant1;
    MINT32 i4RefereceIlluminant2;
    DngNoiseProile_T rNoiseProfile[4];
} ISP_NVRAM_DNG_METADATA_T, *PISP_NVRAM_DNG_METADATA_T;

typedef union
{
    struct  {
        MUINT32                     Version;
        MUINT32                     SensorId;    // ID of sensor module
        ISP_NVRAM_COMMON_STRUCT     ISPComm;
        ISP_NVRAM_PCA_STRUCT        ISPPca;
        ISP_NVRAM_REGISTER_STRUCT   ISPRegs;
        ISP_NVRAM_MIXER3_T          ISPMfbMixer;
        ISP_NVRAM_MULTI_CCM_STRUCT  ISPMulitCCM;
        ISP_NVRAM_MULTI_CCM_STRUCT  MDPMulitCCM;
        float                       isp_ccm_ratio;
        MINT32                      bInvokeSmoothCCM;
        ISP_NVRAM_DNG_METADATA_T    DngMetadata;
        ISP_NVRAM_ANR_TBL_T         ANR_TBL[ANR_SUPPORT_CT_NUM][ANR_SUPPORT_ISO_NUM][ANR_HW_TBL_NUM];
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_ISP_FILE_SIZE];
} NVRAM_CAMERA_ISP_PARAM_STRUCT, *PNVRAM_CAMERA_ISP_PARAM_STRUCT;


class IspNvramRegMgr
{
public:
    IspNvramRegMgr(ISP_NVRAM_REGISTER_STRUCT*const pIspNvramRegs)
        : m_rRegs(*pIspNvramRegs)
        , m_rIdx(pIspNvramRegs->Idx)
    {}
    virtual ~IspNvramRegMgr() {}

public:
    enum EIndexNum
    {
        NUM_DBS         =   NVRAM_DBS_TBL_NUM,
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_BPC         =   NVRAM_BNR_BPC_TBL_NUM,
        NUM_NR1         =   NVRAM_BNR_NR1_TBL_NUM,
        NUM_PDC     =   NVRAM_BNR_PDC_TBL_NUM,
        NUM_LSC         =   NVRAM_LSC_TBL_NUM,
        NUM_NSL2        =   NVRAM_NSL2_TBL_NUM,
        NUM_UDM         =   NVRAM_UDM_TBL_NUM,
        NUM_CCM         =   NVRAM_CCM_TBL_NUM,
        NUM_GGM         =   NVRAM_GGM_TBL_NUM,
        NUM_IHDR_GGM    =   NVRAM_IHDR_GGM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_ANR2        =   NVRAM_ANR2_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
        NUM_BOK         =   NVRAM_BOK_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM,
        NUM_NR3D        =   NVRAM_NR3D_TBL_NUM,
        NUM_MFB         =   NVRAM_MFB_TBL_NUM,
        NUM_LCE         =   NVRAM_LCE_TBL_NUM,
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Set Index
    inline bool setIdx(UINT16 &rIdxTgt, UINT16 const IdxSrc, EIndexNum const Num)
    {
        if  (IdxSrc < Num)
        {
            rIdxTgt = IdxSrc;
            return  true;
        }
        return  false;
    }
public:     ////    Set Index
    inline bool setIdx_DBS(UINT16 const idx)        { return setIdx(m_rIdx.DBS, idx, NUM_DBS); }
    inline bool setIdx_OBC(UINT16 const idx)        { return setIdx(m_rIdx.OBC, idx, NUM_OBC); }
    inline bool setIdx_BPC(UINT16 const idx)        { return setIdx(m_rIdx.BNR_BPC, idx, NUM_BPC); }
    inline bool setIdx_NR1(UINT16 const idx)        { return setIdx(m_rIdx.BNR_NR1, idx, NUM_NR1); }
    inline bool setIdx_PDC(UINT16 const idx)        { return setIdx(m_rIdx.BNR_PDC, idx, NUM_PDC); }
    inline bool setIdx_LSC(UINT16 const idx)        { return setIdx(m_rIdx.LSC, idx, NUM_LSC); }
    inline bool setIdx_NSL2(UINT16 const idx)        { return setIdx(m_rIdx.NSL2, idx, NUM_NSL2); }
    inline bool setIdx_UDM(UINT16 const idx)        { return setIdx(m_rIdx.UDM, idx, NUM_UDM); }
    inline bool setIdx_CCM(UINT16 const idx)        { return setIdx(m_rIdx.CCM, idx, NUM_CCM); }
    inline bool setIdx_GGM(UINT16 const idx)        { return setIdx(m_rIdx.GGM, idx, NUM_GGM); }
    inline bool setIdx_IHDR_GGM(UINT16 const idx)   { return setIdx(m_rIdx.IHDR_GGM, idx, NUM_IHDR_GGM); }
    inline bool setIdx_ANR(UINT16 const idx)        { return setIdx(m_rIdx.ANR, idx, NUM_ANR); }
    inline bool setIdx_ANR2(UINT16 const idx)        { return setIdx(m_rIdx.ANR2, idx, NUM_ANR2); }
    inline bool setIdx_CCR(UINT16 const idx)        { return setIdx(m_rIdx.CCR, idx, NUM_CCR); }
    inline bool setIdx_BOK(UINT16 const idx)        { return setIdx(m_rIdx.BOK, idx, NUM_BOK); }
    inline bool setIdx_EE(UINT16 const idx)         { return setIdx(m_rIdx.EE, idx, NUM_EE); }
    inline bool setIdx_NR3D(UINT16 const idx)       { return setIdx(m_rIdx.NR3D, idx, NUM_NR3D); }
    inline bool setIdx_MFB(UINT16 const idx)        { return setIdx(m_rIdx.MFB, idx, NUM_MFB); }
    inline bool setIdx_LCE(UINT16 const idx)        { return setIdx(m_rIdx.LCE, idx, NUM_LCE); }

public:     ////    Get Index
    inline ISP_NVRAM_REG_INDEX_STRUCT     getIdx() const { return m_rIdx; }
    inline UINT16 getIdx_DBS()       const { return m_rIdx.DBS; }
    inline UINT16 getIdx_OBC()       const { return m_rIdx.OBC; }
    inline UINT16 getIdx_BPC()       const { return m_rIdx.BNR_BPC; }
    inline UINT16 getIdx_NR1()       const { return m_rIdx.BNR_NR1; }
    inline UINT16 getIdx_PDC()       const { return m_rIdx.BNR_PDC; }
    inline UINT16 getIdx_LSC()       const { return m_rIdx.LSC; }
    inline UINT16 getIdx_NSL2()       const { return m_rIdx.NSL2; }
    inline UINT16 getIdx_UDM()       const { return m_rIdx.UDM; }
    inline UINT16 getIdx_CCM()       const { return m_rIdx.CCM; }
    inline UINT16 getIdx_GGM()       const { return m_rIdx.GGM; }
    inline UINT16 getIdx_IHDR_GGM()  const { return m_rIdx.IHDR_GGM; }
    inline UINT16 getIdx_ANR()       const { return m_rIdx.ANR; }
    inline UINT16 getIdx_ANR2()       const { return m_rIdx.ANR2; }
    inline UINT16 getIdx_CCR()       const { return m_rIdx.CCR; }
    inline UINT16 getIdx_BOK()       const { return m_rIdx.BOK; }
    inline UINT16 getIdx_EE()        const { return m_rIdx.EE; }
    inline UINT16 getIdx_NR3D()      const { return m_rIdx.NR3D; }
    inline UINT16 getIdx_MFB()       const { return m_rIdx.MFB; }
    inline UINT16 getIdx_LCE()       const { return m_rIdx.LCE; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    inline ISP_NVRAM_DBS_T&         getDBS() { return m_rRegs.DBS[getIdx_DBS()]; }
    inline ISP_NVRAM_OBC_T&         getOBC() { return m_rRegs.OBC[getIdx_OBC()]; }
    inline ISP_NVRAM_BNR_BPC_T&     getBPC() { return m_rRegs.BNR_BPC[getIdx_BPC()]; }
    inline ISP_NVRAM_BNR_NR1_T&     getNR1() { return m_rRegs.BNR_NR1[getIdx_NR1()]; }
    inline ISP_NVRAM_BNR_PDC_T&     getPDC() { return m_rRegs.BNR_PDC[getIdx_PDC()]; }
    inline ISP_NVRAM_LSC_T&         getLSC() { return m_rRegs.LSC[getIdx_LSC()]; }
    inline ISP_NVRAM_NSL2_T&        getNSL2() { return m_rRegs.NSL2[getIdx_NSL2()]; }
    inline ISP_NVRAM_UDM_T&         getUDM() { return m_rRegs.UDM[getIdx_UDM()]; }
    inline ISP_NVRAM_CCM_T&         getCCM() { return m_rRegs.CCM[getIdx_CCM()]; }
    inline ISP_NVRAM_GGM_T&         getGGM() { return m_rRegs.GGM[getIdx_GGM()]; }
    inline ISP_NVRAM_GGM_T&         getIHDRGGM() { return m_rRegs.IHDR_GGM[getIdx_IHDR_GGM()]; }
    inline ISP_NVRAM_ANR_T&         getANR() { return m_rRegs.ANR[getIdx_ANR()]; }
    inline ISP_NVRAM_ANR2_T&        getANR2() { return m_rRegs.ANR2[getIdx_ANR2()]; }
    inline ISP_NVRAM_CCR_T&         getCCR() { return m_rRegs.CCR[getIdx_CCR()]; }
    inline ISP_NVRAM_BOK_T&         getBOK() { return m_rRegs.BOK[getIdx_BOK()]; }
    inline ISP_NVRAM_EE_T&          getEE()  { return m_rRegs.EE[getIdx_EE()]; }
    inline ISP_NVRAM_NR3D_T&        getNR3D() { return m_rRegs.NR3D[getIdx_NR3D()]; }
    inline ISP_NVRAM_MFB_T&         getMFB()  { return m_rRegs.MFB[getIdx_MFB()]; }
    inline ISP_NVRAM_LCE_T&         getLCE()  { return m_rRegs.LCE[getIdx_LCE()]; }

    inline ISP_NVRAM_DBS_T&         getDBS(UINT16 const idx) { return m_rRegs.DBS[idx]; }
    inline ISP_NVRAM_OBC_T&         getOBC(UINT16 const idx) { return m_rRegs.OBC[idx]; }
    inline ISP_NVRAM_BNR_BPC_T&     getBPC(UINT16 const idx) { return m_rRegs.BNR_BPC[idx]; }
    inline ISP_NVRAM_BNR_NR1_T&     getNR1(UINT16 const idx) { return m_rRegs.BNR_NR1[idx]; }
    inline ISP_NVRAM_BNR_PDC_T&     getPDC(UINT16 const idx) { return m_rRegs.BNR_PDC[idx]; }
    inline ISP_NVRAM_LSC_T&         getLSC(UINT16 const idx) { return m_rRegs.LSC[idx]; }
    inline ISP_NVRAM_NSL2_T&        getNSL2(UINT16 const idx) { return m_rRegs.NSL2[idx]; }
    inline ISP_NVRAM_UDM_T&         getUDM(UINT16 const idx) { return m_rRegs.UDM[idx]; }
    inline ISP_NVRAM_CCM_T&         getCCM(UINT16 const idx) { return m_rRegs.CCM[idx]; }
    inline ISP_NVRAM_GGM_T&         getGGM(UINT16 const idx) { return m_rRegs.GGM[idx]; }
    inline ISP_NVRAM_GGM_T&         getIHDRGGM(UINT16 const idx) { return m_rRegs.IHDR_GGM[idx]; }
    inline ISP_NVRAM_ANR_T&         getANR(UINT16 const idx) { return m_rRegs.ANR[idx]; }
    inline ISP_NVRAM_ANR2_T&        getANR2(UINT16 const idx) { return m_rRegs.ANR2[idx]; }
    inline ISP_NVRAM_CCR_T&         getCCR(UINT16 const idx) { return m_rRegs.CCR[idx]; }
    inline ISP_NVRAM_BOK_T&         getBOK(UINT16 const idx) { return m_rRegs.BOK[idx]; }
    inline ISP_NVRAM_EE_T&          getEE(UINT16 const idx)  { return m_rRegs.EE[idx]; }
    inline ISP_NVRAM_NR3D_T&        getNR3D(UINT16 const idx) { return m_rRegs.NR3D[idx]; }
    inline ISP_NVRAM_MFB_T&         getMFB(UINT16 const idx)  { return m_rRegs.MFB[idx]; }
    inline ISP_NVRAM_LCE_T&         getLCE(UINT16 const idx)  { return m_rRegs.LCE[idx]; }

private:    ////    Data Members.
    ISP_NVRAM_REGISTER_STRUCT&      m_rRegs;
    ISP_NVRAM_REG_INDEX_STRUCT&     m_rIdx;
};

/*******************************************************************************
*
********************************************************************************/

//____AF NVRAM____
#define AF_CAM_SCENARIO_NUM CAM_SCENARIO_NUM

// AF NVRAM Mapping Table by Scenario
static MUINT32 AFNVRAMMapping[AF_CAM_SCENARIO_NUM] = {
    static_cast<MUINT32>(CAM_SCENARIO_PREVIEW),   // PREVIEW (binning-size preview)
    static_cast<MUINT32>(CAM_SCENARIO_VIDEO),     // VIDEO   (full-size vdo record)
    static_cast<MUINT32>(CAM_SCENARIO_CAPTURE),   // CAPTURE (full-size preview)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM1),   // HDR     (N/A)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM2),   // AUTO HDR(N/A)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM3),   // VT      (binning-size vdo record)
    static_cast<MUINT32>(CAM_SCENARIO_CUSTOM4)    // STEREO  (N/A)
};

typedef struct
{
    MUINT8          rFilePath[256];
    AF_NVRAM_T      rAFNVRAM;
    PD_NVRAM_T      rPDNVRAM;
    DUALCAM_NVRAM_T rDualCamNVRAM;
} NVRAM_LENS_DATA_PARA_STRUCT, *PNVRAM_LENS_DATA_PARA_STRUCT;

typedef struct
{
    UINT32 Version;
    FOCUS_RANGE_T   rFocusRange;
    NVRAM_LENS_DATA_PARA_STRUCT rLENSNVRAM[AF_CAM_SCENARIO_NUM];
    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(NVRAM_LENS_DATA_PARA_STRUCT)*AF_CAM_SCENARIO_NUM];
} NVRAM_LENS_PARA_STRUCT, *PNVRAM_LENS_PARA_STRUCT;

typedef union
{
    struct
    {
        MUINT8    LaserOffsetCalEn;
        MUINT16   LaserOffsetCalVal;
        MUINT8    LaserXTalkCalEn;
        MUINT16   LaserXTalkCalVal;

        PD_CALIBRATION_DATA_T rPdCaliData;
    };
    UINT8 Data[MAXIMUM_NVRAM_CAMERA_AF_FILE_SIZE];

} NVRAM_AF_PARA_STRUCT, *PNVRAM_AF_PARA_STRUCT;



/*******************************************************************************
*
********************************************************************************/
#define CAL_INFO_IN_COMM_LOAD   34

#define CAL_GET_DEFECT_FLAG     0x01
#define CAL_GET_3ANVRAM_FLAG    0x02
#define CAL_GET_SHADING_FLAG    0x04
#define CAL_GET_PARA_FLAG       0x08
#define CAL_DATA_LOAD           0x6C6F6164//"load"
#define CAL_DATA_UNLOAD         0x00000000
#define CAL_SHADING_TYPE_SENSOR 0x216D746B//"!mtk"
#define CAL_SHADING_TYPE_ISP    0x3D6D746B//"=mtk"

typedef struct
{
//    PNVRAM_CAMERA_DEFECT_STRUCT     pCameraDefect;
    PNVRAM_CAMERA_SHADING_STRUCT    pCameraShading;
    PNVRAM_CAMERA_ISP_PARAM_STRUCT  pCameraPara;
    AWB_GAIN_T 						rCalGain;
} GET_SENSOR_CALIBRATION_DATA_STRUCT, *PGET_SENSOR_CALIBRATION_DATA_STRUCT;

/*******************************************************************************
*
********************************************************************************/
typedef enum
{
    CAMERA_DATA_TYPE_START=0,
    CAMERA_NVRAM_DATA_ISP = CAMERA_DATA_TYPE_START,
    CAMERA_NVRAM_DATA_3A,
    CAMERA_NVRAM_DATA_SHADING,
    CAMERA_NVRAM_DATA_LENS,
    CAMERA_DATA_AE_PLINETABLE,
    CAMERA_NVRAM_DATA_STROBE,
    CAMERA_DATA_TSF_TABLE,
    CAMERA_DATA_PDC_TABLE,
    CAMERA_NVRAM_DATA_GEOMETRY,
    CAMERA_NVRAM_DATA_FEATURE,
    CAMERA_NVRAM_DATA_AF,
    CAMERA_NVRAM_DATA_FLASH_CALIBRATION,
    CAMERA_NVRAM_VERSION,
    CAMERA_DATA_TYPE_NUM
} CAMERA_DATA_TYPE_ENUM;




typedef enum
{
    GET_CAMERA_DATA_NVRAM,
    GET_CAMERA_DATA_DEFAULT,
    SET_CAMERA_DATA_NVRAM,
} MSDK_CAMERA_NVRAM_DATA_CTRL_CODE_ENUM;

typedef union
{
    struct
    {
        short ispVer[SENSOR_IDX_MAX]; //main, sub, sub2, main2, main3
        short aaaVer[SENSOR_IDX_MAX];
        short shadingVer[SENSOR_IDX_MAX];
        short lensVer[SENSOR_IDX_MAX];
        short aePlineVer[SENSOR_IDX_MAX];
        short strobeVer[SENSOR_IDX_MAX];
        short tsfVer[SENSOR_IDX_MAX];
        short pdcVer[SENSOR_IDX_MAX];
        short geometryVer[SENSOR_IDX_MAX];
        short featureVer[SENSOR_IDX_MAX];
        short afVer[SENSOR_IDX_MAX];
        short flashCalibrationVer[SENSOR_IDX_MAX];
    };
    struct
    {
        UINT8  Data[MAXIMUM_NVRAM_CAMERA_VERSION_FILE_SIZE-sizeof(UINT32)];
        UINT32 isCct;
    };
}NVRAM_CAMERA_VERSION_STRUCT;


typedef struct
{
    // MFLL/AIS (4)
    MUINT32 max_frame_number;           //default=4, range=3~4, step=1
    MUINT32 bss_clip_th;                //default=8, range=0~255, step=1
    MUINT32 memc_bad_mv_range;          //default=48,range=0~255, step=1
    MUINT32 memc_bad_mv_rate_th;        //default=90,range=0~100, step=1    //typo of memc_bad_mb_rate_th

    // MFLL (1)
    MUINT32 mfll_iso_th;                //default=800 iso

    // AIS (4)
    MUINT32 ais_exp_th;                 //default=33000 us
    MUINT32 ais_advanced_tuning_en;     //default=1
    MUINT32 ais_advanced_max_iso;       //default=3200
    MUINT32 ais_advanced_max_exposure;  //default=66000 us

    //
    MUINT32 reserved[7]; //(64/4)-9
} NVRAM_CAMERA_FEATURE_MFLL_STRUCT, *PNVRAM_CAMERA_FEATURE_MFLL_STRUCT;

#define GIS_MAXSUPPORTED_SMODE (20)

typedef struct NVRAM_CAMERA_FEATURE_GIS_STRUCT_t
{
    MUINT32 gis_defWidth;
    MUINT32 gis_defHeight;
    MUINT32 gis_defCrop;
    double   gis_defParameter1[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_defParameter2[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_defParameter3[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_deftRS[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
    double   gis_deftRerserved1[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
    double   gis_deftRerserved2[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
} NVRAM_CAMERA_FEATURE_GIS_STRUCT, *PNVRAM_CAMERA_FEATURE_GIS_STRUCT;


typedef struct
{
    // quality parameters
    MINT32 ANR_Y_LUMA_SCALE_RANGE;
    MINT32 ANR_C_CHROMA_SCALE;
    MINT32 ANR_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3;
    MINT32 ANR_Y_SCALE_CPY4;
    MINT32 ANR_Y_CPX1;
    MINT32 ANR_Y_CPX2;
    MINT32 ANR_Y_CPX3;
    MINT32 ANR_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH;
    MINT32 ANR_PTY_GAIN_TH;
    MINT32 ANR_KSIZE_LO_TH;
    MINT32 ANR_KSIZE_HI_TH;
    MINT32 ANR_KSIZE_LO_TH_C;
    MINT32 ANR_KSIZE_HI_TH_C;
    MINT32 ITUNE_ANR_PTY_STD;
    MINT32 ITUNE_ANR_PTU_STD;
    MINT32 ITUNE_ANR_PTV_STD;
    MINT32 ANR_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_BLD_TH_Y;
    MINT32 ANR_ACT_SLANT_Y;
    MINT32 ANR_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_BLD_TH_C;
    MINT32 ANR_ACT_SLANT_C;
    MINT32 RADIUS_H;
    MINT32 RADIUS_V;
    MINT32 RADIUS_H_C;
    MINT32 RADIUS_V_C;
    MINT32 ANR_PTC_HGAIN;
    MINT32 ANR_PTY_HGAIN;
    MINT32 ANR_LPF_HALFKERNEL;
    MINT32 ANR_LPF_HALFKERNEL_C;
    MINT32 ANR_ACT_MODE;
    MINT32 ANR_LCE_SCALE_GAIN;
    MINT32 ANR_LCE_C_GAIN;
    MINT32 ANR_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3;
    MINT32 ANR_MEDIAN_LOCATION;
    MINT32 ANR_CEN_X;
    MINT32 ANR_CEN_Y;
    MINT32 ANR_R1;
    MINT32 ANR_R2;
    MINT32 ANR_R3;
    MINT32 LUMA_ON_OFF;
    MINT32 CCR_EN;
    MINT32 CCR_CEN_U;
    MINT32 CCR_CEN_V;
    MINT32 CCR_Y_CPX1;
    MINT32 CCR_Y_CPX2;
    MINT32 CCR_Y_CPY1;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_X1;
    MINT32 CCR_UV_X2;
    MINT32 CCR_UV_X3;
    MINT32 CCR_UV_GAIN1;
    MINT32 CCR_UV_GAIN2;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_UV_GAIN_SP2;
    MINT32 CCR_Y_CPX3;
    MINT32 CCR_Y_CPY0;
    MINT32 CCR_Y_CPY2;
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP2;
    MINT32 CCR_UV_GAIN_MODE;
    MINT32 CCR_MODE;
    MINT32 CCR_OR_MODE;
    MINT32 CCR_HUE_X1;
    MINT32 CCR_HUE_X2;
    MINT32 CCR_HUE_X3;
    MINT32 CCR_HUE_X4;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;
    MINT32 CCR_HUE_GAIN1;
    MINT32 CCR_HUE_GAIN2;
    // total 320 btyes
} NVRAM_CAMERA_FEATURE_SWNR_STRUCT;

#define NVRAM_SWNR_TBL_NUM (24) // 10 by iso, 2 preserved * (mfll/single)

typedef struct NVRAM_CAMERA_FEATURE_STRUCT_t
{
    NVRAM_CAMERA_FEATURE_GIS_STRUCT gis;
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT mfll;
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT swnr[NVRAM_SWNR_TBL_NUM];
} NVRAM_CAMERA_FEATURE_STRUCT, *PNVRAM_CAMERA_FEATURE_STRUCT;



typedef union
{
    struct
    {
        MUINT32 StereoData[MTK_STEREO_KERNEL_NVRAM_LENGTH];
        float DepthAfData[ (MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE - MTK_STEREO_KERNEL_NVRAM_LENGTH * sizeof(MUINT32) ) / sizeof(float) ];
    } StereoNvramData;

    UINT8   Data[MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE];
}NVRAM_CAMERA_GEOMETRY_STRUCT;

#endif // _CAMERA_CUSTOM_NVRAM_H_

