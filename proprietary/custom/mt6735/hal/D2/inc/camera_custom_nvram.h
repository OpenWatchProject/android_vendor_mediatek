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
#ifndef _CAMERA_CUSTOM_NVRAM_H_
#define _CAMERA_CUSTOM_NVRAM_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "ispif.h"
#include "CFG_Camera_File_Max_Size.h"
#include "camera_custom_AEPlinetable.h"

#include "camera_custom_tsf_tbl.h"

using namespace NSIspTuning;

#define NVRAM_CAMERA_SHADING_FILE_VERSION       1
#define NVRAM_CAMERA_PARA_FILE_VERSION          1
#define NVRAM_CAMERA_3A_FILE_VERSION            1
#define NVRAM_CAMERA_LENS_FILE_VERSION          1
#define NVRAM_CAMERA_STROBE_FILE_VERSION          2 // SC MODIFY
#define NVRAM_CAMERA_FEATURE_FILE_VERSION          1
#define NVRAM_CAMERA_GEOMETRY_FILE_VERSION          1
#define NVRAM_CAMERA_PLINE_FILE_VERSION          1

#define OEM_CAMERA_PARTS_LIST_FILE_VERSION          1

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
        UINT32 CapTable[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
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

typedef struct
{
    AE_DEVICES_INFO_T rDevicesInfo;
    AE_HIST_CFG_T rHistConfig;
    AE_CCT_CFG_T rCCTConfig;
} AE_NVRAM_T;

//____AF NVRAM____

#define AF_TABLE_NUM (30)
#define ISO_MAX_NUM 	(8)
#define GMEAN_MAX_NUM 	(6)

#define JUMP_NUM		(5)
#define STEPSIZE_NUM	(15)
#define PRERUN_NUM		(2)
#define FPS_THRES_NUM	(2)


typedef struct
{
    MINT32 i4InfPos;
    MINT32 i4MacroPos;

} FOCUS_RANGE_T;

typedef struct
{
	MINT32 i4Enable;

	MINT32 i4ChgType;			// 1 is &&, 0 is ||
	MINT32 i4ChgOffset;			// value -> 0 more sensitive
	MINT32 i4ChgThr[3];			// percentage -> 0 more sensitive; level 1~3 is stable~sensitive
	MINT32 i4ChgCnt[3];			// level 1~3 is stable~sensitive

	MINT32 i4StbType;			// 1 is &&, 0 is ||
	MINT32 i4StbOffset;        	// value -> 0 more stable
	MINT32 i4StbThr[3];			// percentage -> 0 more stable; level 1~3 is stable~sensitive
	MINT32 i4StbCnt[3];			// level 1~3 is stable~sensitive
} SCENE_MONITOR_T;


#define PD_CALI_DATA_SIZE   (2048)
#define PD_CONF_IDX_SIZE    (5)

typedef struct
{
    MINT32  i4FocusPDSizeX;
    MINT32  i4FocusPDSizeY;
    MINT32  i4ConfIdx1[PD_CONF_IDX_SIZE];
    MINT32  i4ConfIdx2[PD_CONF_IDX_SIZE];
    MINT32  i4ConfTbl[PD_CONF_IDX_SIZE+1][PD_CONF_IDX_SIZE+1];
    MINT32  i4SaturateLevel;
    MINT32  i4SaturateThr;
    MINT32  i4ConfThr;
    MINT32  i4Reserved[20];
} PD_ALGO_TUNING_T;

typedef struct
{
    MUINT8  uData[PD_CALI_DATA_SIZE];
    MINT32  i4Size;
} PD_CALIBRATION_DATA_T;

typedef struct
{
    PD_CALIBRATION_DATA_T rCaliData;
    PD_ALGO_TUNING_T rTuningData;
} PD_NVRAM_T;

typedef struct
{
    MINT32  i4Offset;
    MINT32  i4NormalNum;
    MINT32  i4MacroNum;
    MINT32  i4InfIdxOffset;
    MINT32  i4MacroIdxOffset;
    MINT32  i4Pos[AF_TABLE_NUM];

} NVRAM_AF_TABLE_T;

typedef struct
{
    MINT32 i4ISONum;
    MINT32 i4ISO[ISO_MAX_NUM];

    MINT32 i4GMR[7][ISO_MAX_NUM];

    MINT32 i4FV_DC[ISO_MAX_NUM];
    MINT32 i4MIN_TH[ISO_MAX_NUM];
    MINT32 i4HW_TH[ISO_MAX_NUM];

    MINT32 i4FV_DC2[ISO_MAX_NUM];
    MINT32 i4MIN_TH2[ISO_MAX_NUM];
    MINT32 i4HW_TH2[ISO_MAX_NUM];

} NVRAM_AF_THRES_T;

typedef struct
{
    NVRAM_AF_TABLE_T sTABLE;
    MINT32 i4THRES_MAIN;
    MINT32 i4THRES_SUB;
    MINT32 i4AFC_FAIL_CNT;
    MINT32 i4FAIL_POS;

    MINT32 i4INIT_WAIT;
    MINT32 i4FRAME_WAIT[JUMP_NUM];
    MINT32 i4DONE_WAIT;
} NVRAM_AF_COEF;

typedef struct
{
    // --- AF SW coef ---
    NVRAM_AF_COEF sAF_Coef;			// for normal
    //NVRAM_AF_COEF sZSD_AF_Coef;		// for slow preview fps
    NVRAM_AF_COEF sVAFC_Coef;		// for smooth

    // --- AF HW thres ---
    NVRAM_AF_THRES_T sAF_TH;		// sensor preview mode
    NVRAM_AF_THRES_T sZSD_AF_TH;	// sensor capture mode
    NVRAM_AF_THRES_T sVID_AF_TH;	// sensor video mode
    NVRAM_AF_THRES_T sVID1_AF_TH;	// sensor video mode reserve 1
    NVRAM_AF_THRES_T sVID2_AF_TH;	// sensor video mode reserve 2
    NVRAM_AF_THRES_T sIHDR_AF_TH;       // sensor ihdr mode
    NVRAM_AF_THRES_T sREV1_AF_TH;	// sensor mode reserve 1
    NVRAM_AF_THRES_T sREV2_AF_TH;	// sensor mode reserve 2
    NVRAM_AF_THRES_T sREV3_AF_TH;	// sensor mode reserve 3
    NVRAM_AF_THRES_T sREV4_AF_TH;	// sensor mode reserve 4
    NVRAM_AF_THRES_T sREV5_AF_TH;	// sensor mode reserve 5

    // --- Common use ---
    MINT32 i4ReadOTP; 				// 0 : disable, 1:enable
	MINT32 i4StatGain;
    MINT32 i4LV_THRES;
    MINT32 i4InfPos;
    MINT32 i4FRAME_TIME;
    MINT32 i4BackJump[JUMP_NUM];
    MINT32 i4BackJumpPos;
	// AFC & VAFC
    MINT32 i4AFC_STEP_SIZE;
	MINT32 i4SPOT_PERCENT_W;		// AFC window location
    MINT32 i4SPOT_PERCENT_H;		// AFC window location
	MINT32 i4CHANGE_CNT_DELTA;

	// AFS
	MINT32 i4AFS_STEP_MIN_ENABLE;
	MINT32 i4AFS_STEP_MIN_NORMAL;
	MINT32 i4AFS_STEP_MIN_MACRO;

    // FV Monitor
    MINT32 i4FIRST_FV_WAIT;
    MINT32 i4FV_1ST_STABLE_THRES;
    MINT32 i4FV_1ST_STABLE_OFFSET;
    MINT32 i4FV_1ST_STABLE_NUM;
    MINT32 i4FV_1ST_STABLE_CNT;
	MINT32 i4FV_SHOCK_THRES;
	MINT32 i4FV_SHOCK_OFFSET;
	MINT32 i4FV_VALID_CNT;
	MINT32 i4FV_SHOCK_FRM_CNT;
	MINT32 i4FV_SHOCK_CNT;

    // --- FaceAF ---
    MINT32 i4FDWinPercent;
    MINT32 i4FDSizeDiff;
    MINT32 i4FD_DETECT_CNT;
	MINT32 i4FD_NONE_CNT;

	// --- AFv1.1/AFv1.2 ---
	MINT32 i4LeftSearchEnable;		//[0] enable left peak search if i4Coef[0] != 0
	MINT32 i4LeftJumpStep;			//[1] disable left peak search, left step= 3 + i4Coef[1]
	MINT32 i4Curve5ptFit;			//[2] enable 5 point curve fitting if i4Coef[2] != 0
	MINT32 i4AfDoneDelay;			//[3] AF done happen delay count
	MINT32 i4VdoAfDoneDelay;		//[3] AF done happen delay count
	MINT32 i4ZoomInWinChg;			//[4] enable AF window change with Zoom-in
	MINT32 i4SensorEnable;			//[5] AF use sensor lister => 0:disable, 1:enable
	MINT32 i4PostureComp;			//[6] post comp max offset => 0:disable, others:enable
	MINT32 i4SceneMonitorLevel;		//[7] scenechange enhancement level => 0:original, 1~3:;level stable to sensitive, 9:use coef
	MINT32 i4VdoSceneMonitorLevel;	//[7] scenechange enhancement level => 0:original, 1~3:;level stable to sensitive, 9:use coef
	// Scene Monitor
	SCENE_MONITOR_T sFV;
	SCENE_MONITOR_T sGS;
	SCENE_MONITOR_T sAEB;
	SCENE_MONITOR_T sGYRO;
	SCENE_MONITOR_T sACCE;
	SCENE_MONITOR_T sVdoFV;
	SCENE_MONITOR_T sVdoGS;
	SCENE_MONITOR_T sVdoAEB;
	SCENE_MONITOR_T sVdoGYRO;
	SCENE_MONITOR_T sVdoACCE;

	// --- AFv2.0 ---
	// Black faceAF
	MINT32 i4FvExtractEnable;							// 0 is disable, > 0 is enable
	MINT32 i4FvExtractThr;								// percentage, fix
	// Damping compensation
	MINT32 i4DampingCompEnable;							// 0 is disable, 1 is enable
	MINT32 i4DampingStep[STEPSIZE_NUM];					//{ step1, step2, ... }
	MINT32 i4DampingRdirComp[PRERUN_NUM][STEPSIZE_NUM];	//{ prerun0:{rComp1, rComp2, ...}, prerun1:{rComp1, rComp2, ...} }
	MINT32 i4DampingLdirComp[PRERUN_NUM][STEPSIZE_NUM];	//{ prerun0:{lComp1, lComp2, ...}, prerun1:{lComp1, lComp2, ...} }
	// Tunable lens search direction
	MINT32 i4DirSelectEnable;							// 0 is disable, 1 is enable
	MINT32 i4InfDir;									// 1: right search
	MINT32 i4MidDir;									// 2: reverse search ; 0: keep search
	MINT32 i4MacDir;     							// -1: left search
	MINT32 i4RatioInf;      							// below % of path belongs to INF
	MINT32 i4RatioMac;      							// above % of path belongs to MAC
	// Tunable lens search startPos
	MINT32 i4StartBoundEnable;							// 0 is disable, 1 is enable
	MINT32 i4StartCamCAF;								// -1: force start from idx 0 (INF)
	MINT32 i4StartCamTAF;								// X: if close to boundary in X steps, start from boundary
	MINT32 i4StartVdoCAF;								// 0: keep current idx
	MINT32 i4StartVdoTAF;
	// FPS-adjusted AF table
	MINT32 i4FpsRemapTblEnable;							// 0 is disable, 1 is enable
	MINT32 i4FpsThres[FPS_THRES_NUM];					// fps = {12, 15}
	MINT32 i4TableClipPt[FPS_THRES_NUM];				// = {2, 1};
	MINT32 i4TableRemapPt[FPS_THRES_NUM];				// = {7, 8};

	// --- Easytuning ---
	MINT32 i4EasyTuning[100];							// AF easy tuning paremeters

		// --- DepthAF ---
	MINT32 i4DepthAF[500];



    // --- reserved ---
    MINT32 i4Coefs[100];
    // [0] inclinePeak rejection; 0: is disable, 1: is enable
    // [1] fdOffMaxCnt, fd turnOff maxCnt for exiting faceAF; 0: use default value 10
    // [2~10]
    // [11~20] for DAF
    // [21~30] for PDAF
    // [31]~[40]
    // [41]~[50] for PL detect
} AF_NVRAM_T;

//____AWB NVRAM____

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
    AWB_FEATURE_AREA_T rSunsetVerTex;
    AWB_FEATURE_AREA_T rShadeFArea;
    AWB_FEATURE_AREA_T rShadeFVerTex;
    AWB_FEATURE_AREA_T rShadeCWFArea;
    AWB_FEATURE_AREA_T rShadeCWFVerTex;
    AWB_FEATURE_AREA_T rLowCCTArea;
    AWB_FEATURE_AREA_T rLowCCTVerTex;
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
    MINT32 i4Enable;
    MINT32 i4SpeedRatio;
} AWB_LOWCCT_PROP_T;

typedef struct {
	AWB_SUNSET_PROP_T rSunsetProp;
	AWB_SHADE_PROP_T rShadeFProp;
	AWB_SHADE_PROP_T rShadeCWFProp;
	AWB_LOWCCT_PROP_T rLowCCTProp;
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


//____3A NVRAM____

//typedef unsigned char  UINT8;

// AWB NVRAM index
typedef enum
{
	AWB_NVRAM_IDX_NORMAL = 0,
    AWB_NVRAM_IDX_VHDR,
	AWB_NVRAM_IDX_NUM
} AWB_NVRAM_IDX_T;

typedef union
{
	struct
	{
    //data structure version, update once structure been modified.
    UINT32 u4Version;

    // ID of sensor module
    UINT32 SensorId;

    //data content
    AE_NVRAM_T rAENVRAM;
    AWB_NVRAM_T rAWBNVRAM[AWB_NVRAM_IDX_NUM];
    FLASH_AWB_NVRAM_T rFlashAWBNVRAM;
    //SSS(reserved unused spaces(bytes)) = total-used;,
    //ex. SSS = 4096-sizeof(UINT32)--sizeof(NVRAM_AAA_T)-sizeof(NVRAM_bbb_T);
    //    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)-sizeof(AF_NVRAM_T)-sizeof(AWB_NVRAM_T)];
    char tmp[1000];
	};
	char data[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE];
    //UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)-sizeof(AWB_NVRAM_T)];
} NVRAM_CAMERA_3A_STRUCT, *PNVRAM_CAMERA_3A_STRUCT;

///////////////////////////////////////////////////////////////////




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
    int vBatL;    //mv
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


typedef struct
{
    int toleranceEV_pos;
    int toleranceEV_neg;

    int XYWeighting;

    bool  useAwbPreferenceGain;

    int envOffsetIndex[4];
    int envXrOffsetValue[4];
    int envYrOffsetValue[4];

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


/*******************************************************************************
* ISP NVRAM parameter
********************************************************************************/
#define NVRAM_OBC_TBL_NUM               (70)//TEMP(12)
#define NVRAM_BPC_TBL_NUM               (105)//TEMP(9)
#define NVRAM_NR1_TBL_NUM               (99)//TEMP(9)
#define NVRAM_LSC_TBL_NUM               (SHADING_SUPPORT_OP_NUM)
#define NVRAM_SL2_TBL_NUM               (NVRAM_LSC_TBL_NUM*SHADING_SUPPORT_CT_NUM)
#define NVRAM_CFA_TBL_NUM               (351)//TEMP(43) // +1: default for disable
#define NVRAM_CCM_TBL_NUM               (4)
#define NVRAM_GGM_TBL_NUM               (5)
#define NVRAM_ANR_TBL_NUM               (410)//TEMP(49)
#define NVRAM_CCR_TBL_NUM               (70)//TEMP(7)
#define NVRAM_EE_TBL_NUM                (370)//TEMP(49)

#define NVRAM_CFA_DISABLE_IDX           (NVRAM_CFA_TBL_NUM-1)

// camera common parameters and sensor parameters
typedef struct
{
    UINT32 CommReg[64];
} ISP_NVRAM_COMMON_STRUCT, *PISP_NVRAM_COMMON_STRUCT;

typedef struct ISP_NVRAM_REG_INDEX_STRUCT
{
    UINT16 OBC;
    UINT16 BPC;
    UINT16 NR1;
    UINT16 LSC;
    UINT16 SL2;
    UINT16 CFA;
    UINT16 CCM;
    UINT16 GGM;
    UINT16 ANR;
    UINT16 CCR;
    UINT16 EE;
}    ISP_NVRAM_REG_INDEX_T, *PISP_NVRAM_REG_INDEX_T;

typedef struct
{
    ISP_NVRAM_REG_INDEX_T       Idx;
    ISP_NVRAM_OBC_T             OBC[NVRAM_OBC_TBL_NUM];
    ISP_NVRAM_BPC_T             BPC[NVRAM_BPC_TBL_NUM];
    ISP_NVRAM_NR1_T             NR1[NVRAM_NR1_TBL_NUM];
    ISP_NVRAM_LSC_T             LSC[NVRAM_LSC_TBL_NUM];
    ISP_NVRAM_SL2_T             SL2[NVRAM_SL2_TBL_NUM];
    ISP_NVRAM_CFA_T             CFA[NVRAM_CFA_TBL_NUM];
    ISP_NVRAM_CCM_T             CCM[NVRAM_CCM_TBL_NUM];
    ISP_NVRAM_GGM_T             GGM[NVRAM_GGM_TBL_NUM];
    ISP_NVRAM_ANR_T             ANR[NVRAM_ANR_TBL_NUM];
    ISP_NVRAM_CCR_T             CCR[NVRAM_CCR_TBL_NUM];
    ISP_NVRAM_EE_T              EE[NVRAM_EE_TBL_NUM];
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

// MFB SW mixer parameter
typedef struct
{
    MINT32 i4M0;
    MINT32 i4M1;
} ISP_NVRAM_MFB_MIXER_PARAM_T;

typedef struct
{
    ISP_NVRAM_MFB_MIXER_PARAM_T param[7];
} ISP_NVRAM_MFB_MIXER_STRUCT, *PISP_NVRAM_MFB_MIXER_STRUCT;

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
        ISP_NVRAM_MFB_MIXER_STRUCT  ISPMfbMixer;
        ISP_NVRAM_MULTI_CCM_STRUCT  ISPMulitCCM;
        MINT32                      bInvokeSmoothCCM;
        ISP_NVRAM_DNG_METADATA_T    DngMetadata;
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
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_BPC         =   NVRAM_BPC_TBL_NUM,
        NUM_NR1         =   NVRAM_NR1_TBL_NUM,
        NUM_LSC         =   NVRAM_LSC_TBL_NUM,
        NUM_SL2         =   NVRAM_SL2_TBL_NUM,
        NUM_CFA         =   NVRAM_CFA_TBL_NUM,
        NUM_CCM         =   NVRAM_CCM_TBL_NUM,
        NUM_GGM         =   NVRAM_GGM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM,
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
    inline bool setIdx_OBC(UINT16 const idx)        { return setIdx(m_rIdx.OBC, idx, NUM_OBC); }
    inline bool setIdx_BPC(UINT16 const idx)        { return setIdx(m_rIdx.BPC, idx, NUM_BPC); }
    inline bool setIdx_NR1(UINT16 const idx)        { return setIdx(m_rIdx.NR1, idx, NUM_NR1); }
    inline bool setIdx_LSC(UINT16 const idx)        { return setIdx(m_rIdx.LSC, idx, NUM_LSC); }
    inline bool setIdx_SL2(UINT16 const idx)        { return setIdx(m_rIdx.SL2, idx, NUM_SL2); }
    inline bool setIdx_CFA(UINT16 const idx)        { return setIdx(m_rIdx.CFA, idx, NUM_CFA); }
    inline bool setIdx_CCM(UINT16 const idx)        { return setIdx(m_rIdx.CCM, idx, NUM_CCM); }
    inline bool setIdx_GGM(UINT16 const idx)        { return setIdx(m_rIdx.GGM, idx, NUM_GGM); }
    inline bool setIdx_ANR(UINT16 const idx)        { return setIdx(m_rIdx.ANR, idx, NUM_ANR); }
    inline bool setIdx_CCR(UINT16 const idx)        { return setIdx(m_rIdx.CCR, idx, NUM_CCR); }
    inline bool setIdx_EE(UINT16 const idx)         { return setIdx(m_rIdx.EE, idx, NUM_EE); }

public:     ////    Get Index
    inline UINT16 getIdx_OBC()       const { return m_rIdx.OBC; }
    inline UINT16 getIdx_BPC()       const { return m_rIdx.BPC; }
    inline UINT16 getIdx_NR1()       const { return m_rIdx.NR1; }
    inline UINT16 getIdx_LSC()       const { return m_rIdx.LSC; }
    inline UINT16 getIdx_SL2()       const { return m_rIdx.SL2; }
    inline UINT16 getIdx_CFA()       const { return m_rIdx.CFA; }
    inline UINT16 getIdx_CCM()       const { return m_rIdx.CCM; }
    inline UINT16 getIdx_GGM()       const { return m_rIdx.GGM; }
    inline UINT16 getIdx_ANR()       const { return m_rIdx.ANR; }
    inline UINT16 getIdx_CCR()       const { return m_rIdx.CCR; }
    inline UINT16 getIdx_EE()        const { return m_rIdx.EE; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    inline ISP_NVRAM_OBC_T&         getOBC() { return m_rRegs.OBC[getIdx_OBC()]; }
    inline ISP_NVRAM_BPC_T&         getBPC() { return m_rRegs.BPC[getIdx_BPC()]; }
    inline ISP_NVRAM_NR1_T&         getNR1() { return m_rRegs.NR1[getIdx_NR1()]; }
    inline ISP_NVRAM_LSC_T&         getLSC() { return m_rRegs.LSC[getIdx_LSC()]; }
    inline ISP_NVRAM_SL2_T&         getSL2() { return m_rRegs.SL2[getIdx_SL2()]; }
    inline ISP_NVRAM_CFA_T&         getCFA() { return m_rRegs.CFA[getIdx_CFA()]; }
    inline ISP_NVRAM_CCM_T&         getCCM() { return m_rRegs.CCM[getIdx_CCM()]; }
    inline ISP_NVRAM_GGM_T&         getGGM() { return m_rRegs.GGM[getIdx_GGM()]; }
    inline ISP_NVRAM_ANR_T&         getANR() { return m_rRegs.ANR[getIdx_ANR()]; }
    inline ISP_NVRAM_CCR_T&         getCCR() { return m_rRegs.CCR[getIdx_CCR()]; }
    inline ISP_NVRAM_EE_T&          getEE()  { return m_rRegs.EE[getIdx_EE()]; }

    inline ISP_NVRAM_OBC_T&         getOBC(UINT16 const idx) { return m_rRegs.OBC[idx]; }
    inline ISP_NVRAM_BPC_T&         getBPC(UINT16 const idx) { return m_rRegs.BPC[idx]; }
    inline ISP_NVRAM_NR1_T&         getNR1(UINT16 const idx) { return m_rRegs.NR1[idx]; }
    inline ISP_NVRAM_LSC_T&         getLSC(UINT16 const idx) { return m_rRegs.LSC[idx]; }
    inline ISP_NVRAM_SL2_T&         getSL2(UINT16 const idx) { return m_rRegs.SL2[idx]; }
    inline ISP_NVRAM_CFA_T&         getCFA(UINT16 const idx) { return m_rRegs.CFA[idx]; }
    inline ISP_NVRAM_CCM_T&         getCCM(UINT16 const idx) { return m_rRegs.CCM[idx]; }
    inline ISP_NVRAM_GGM_T&         getGGM(UINT16 const idx) { return m_rRegs.GGM[idx]; }
    inline ISP_NVRAM_ANR_T&         getANR(UINT16 const idx) { return m_rRegs.ANR[idx]; }
    inline ISP_NVRAM_CCR_T&         getCCR(UINT16 const idx) { return m_rRegs.CCR[idx]; }
    inline ISP_NVRAM_EE_T&          getEE(UINT16 const idx)  { return m_rRegs.EE[idx]; }


private:    ////    Data Members.
    ISP_NVRAM_REGISTER_STRUCT&      m_rRegs;
    ISP_NVRAM_REG_INDEX_STRUCT&     m_rIdx;
};

/*******************************************************************************
*
********************************************************************************/

typedef struct
{
    UINT32 Version;
    FOCUS_RANGE_T rFocusRange;
    AF_NVRAM_T    rAFNVRAM;
    PD_NVRAM_T rPDNVRAM;
    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(FOCUS_RANGE_T)-sizeof(AF_NVRAM_T)-sizeof(PD_NVRAM_T)];
} NVRAM_LENS_PARA_STRUCT, *PNVRAM_LENS_PARA_STRUCT;


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
    AWB_GAIN_T                         rCalGain;
} GET_SENSOR_CALIBRATION_DATA_STRUCT, *PGET_SENSOR_CALIBRATION_DATA_STRUCT;

/*******************************************************************************
*
********************************************************************************/
typedef enum
{
    CAMERA_DATA_TYPE_START = 0,
    CAMERA_NVRAM_DATA_ISP = CAMERA_DATA_TYPE_START,
    CAMERA_NVRAM_DATA_3A,
    CAMERA_NVRAM_DATA_SHADING,
    CAMERA_NVRAM_DATA_LENS,
    CAMERA_DATA_AE_PLINETABLE,
    CAMERA_NVRAM_DATA_STROBE,
    CAMERA_DATA_TSF_TABLE,
 CAMERA_NVRAM_DATA_GEOMETRY,
    CAMERA_NVRAM_DATA_FEATURE,
 CAMERA_NVRAM_VERSION,
 CAMERA_OEM_COMMON,
 CAMERA_OEM_SENSOR_LAYOUT,
 CAMERA_OEM_PARTS_LIST,
 CAMERA_OEM_AE,
 CAMERA_OEM_AWB,
 CAMERA_OEM_AF,
 //CAMERA_OEM_FLASH,
 CAMERA_OEM_FEATURE,
    CAMERA_DATA_TYPE_NUM
} CAMERA_DATA_TYPE_ENUM;

typedef enum
{
    GET_CAMERA_DATA_NVRAM,
    GET_CAMERA_DATA_DEFAULT,
    SET_CAMERA_DATA_NVRAM,
} MSDK_CAMERA_NVRAM_DATA_CTRL_CODE_ENUM;

typedef enum
{
 OEM_PARTS_SENSOR_ID = 0,
 OEM_PARTS_LENS_ID = 1,
 OEM_PARTS_STROBE_ID =2,
}MSDK_CAMERA_NVRAM_OEM_PARTS_ENUM;

typedef union
{
    struct
    {
   short ispVer[3]; //main, sub, main2
   short aaaVer[3];
   short shadingVer[3];
   short lensVer[3];
   short aePlineVer[3];
   short strobeVer[3];
   short tsfVer[3];
   short geometryVer[3];
   short featureVer[3];
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_VERSION_FILE_SIZE];
}NVRAM_CAMERA_VERSION_STRUCT;

typedef union
{
    struct
    {
   MINT32 temp;
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_FEATURE_FILE_SIZE];
} NVRAM_CAMERA_FEATURE_STRUCT, *PNVRAM_CAMERA_FEATURE_STRUCT;
typedef union
{
    struct
    {
   MINT32 temp;
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE];
}NVRAM_CAMERA_GEOMETRY_STRUCT;


typedef union
{
    struct
    {
        int ver;
        MINT32 OemMainSensorId[10];
        MINT32 OemSubSensorId[10];

        char strMake[32];
        char strModel[32];
        char strSoftware[32];


        short ispVer[3]; //main, sub, main2
  short aaaVer[3];
  short shadingVer[3];
  short lensVer[3];
  short aePlineVer[3];
  short strobeVer[3];
  short tsfVer[3];
  short geometryVer[3];
        short featureVer[3];
        short partsListVer[3];
        short oemAeVer[3];
        short oemAwbVer[3];
        short oemAfVer[3];
        short oemLayoutVer[3];
    };
    UINT8   Data[100];

}OEM_CAMERA_COMMON;



typedef union
{
    struct
    {
        int SensorOrientation;
        int SensorFakeOrientation;
        int SensorHorFOV;
        int SensorVerFOV;
        int IsSensorHMirror;
        int IsSensorVFlip;
    };
    UINT8   data[100];

}OEM_CAMERA_SENSOR_LAYOUT;


typedef union
{
    struct
    {
        int temp;
    };
    UINT8   data[100];

}OEM_CAMERA_AE;

typedef union
{
    struct
    {
        int temp;
    };
    UINT8   data[100];

}OEM_CAMERA_AWB;

typedef union
{
    struct
    {
     bool isDataExist;
  int  setNum;
        int  mapping[20][3];
    };
    UINT8   data[200];

}OEM_CAMERA_PARTS_LIST;

typedef union
{
    struct
    {
        int temp;
    };
    UINT8   data[100];

}OEM_CAMERA_AF;

/*
typedef union
{
    struct
    {
        int temp;
    };
    UINT8   data[100];

}OEM_CAMERA_FLASH;
*/

struct OEM_CAMERA_FEATURE_HDR {
    //
    MUINT32 CapturePolicy;
    MUINT32 CoreNumber;
    MUINT32 CaptureAlgorithm;
    MUINT32 NeOverexpPercent;
    MUINT32 BRatio;
    MUINT32 Gain[11];
    MUINT32 BottomFRatio;
    MUINT32 TopFRatio;
    MUINT32 BottomFBound;
    MUINT32 TopFBound;
    MUINT32 ThHigh;
    MUINT32 ThLow;
    MUINT32 TargetLevelSub;
    //
    MUINT32 MaxHDRExpTimeInUS;
    MUINT32 MaxSafeHDRExpTimeInUs;
    MUINT32 MaxHDRSensorGain;
    //
    double  TargetTopEV;
    double  SafeTargetTopEV;
    double  TargetBottomEV;
};
typedef union
{
    struct
    {
        OEM_CAMERA_FEATURE_HDR hdr;
    };
    UINT8   data[100];

}OEM_CAMERA_FEATURE;


enum
{
    e_CamParaTypeNvram,
    e_CamParaTypeOem,

};

int cust_getCamParaType();

//int fillDefaultOemComm(OEM_CAMERA_COMMON* oem);
int fillDefaultOemComm(OEM_CAMERA_COMMON* oem);
int fillDefaultOemCamLayout(int sensortype, OEM_CAMERA_SENSOR_LAYOUT* oem);
int fillDefaultOemCameraPartsList(OEM_CAMERA_PARTS_LIST* oem);
int fillDefaultOemFeature(int sensortype, OEM_CAMERA_FEATURE* oem);


#endif // _CAMERA_CUSTOM_NVRAM_H_

