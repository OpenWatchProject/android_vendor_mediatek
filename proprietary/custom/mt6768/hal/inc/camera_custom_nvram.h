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
#include "camera_custom_3a_nvram.h"

#include "camera_custom_af_nvram.h"
#include "camera_custom_tsf_tbl.h"


using namespace NSIspTuning;

#define NVRAM_CAMERA_SHADING_FILE_VERSION       1
#define NVRAM_CAMERA_PARA_FILE_VERSION          1
#define NVRAM_CAMERA_3A_FILE_VERSION            1
#define NVRAM_CAMERA_LENS_FILE_VERSION          1
#define NVRAM_CAMERA_STROBE_FILE_VERSION          2 // SC MODIFY
#define NVRAM_CAMERA_FEATURE_FILE_VERSION          1
#define NVRAM_CAMERA_GEOMETRY_FILE_VERSION          1
#define NVRAM_CAMERA_FOV_FILE_VERSION           1
#define NVRAM_CAMERA_PLINE_FILE_VERSION          1
#define NVRAM_CAMERA_AF_FILE_VERSION          1
#define NVRAM_CAMERA_FLASH_CALIBRATION_FILE_VERSION          1
#ifndef MTK_STEREO_KERNEL_NVRAM_LENGTH
//May defined in algo header
#define MTK_STEREO_KERNEL_NVRAM_LENGTH (8400) // refer to the same define in MtkStereoKernel.h
#endif

/*******************************************************************************
* ANR table
********************************************************************************/
#define ANR_SUPPORT_CT_NUM              (2)
#define ANR_SUPPORT_ISO_NUM             (6)
#define ANR_HW_TBL_NUM                  (256)

/*******************************************************************************
* shading
********************************************************************************/
#define SHADING_SUPPORT_CT_NUM          (4)
#define SHADING_SUPPORT_OP_NUM          (10)
#define SHADING_SUPPORT_CH_NUM          (4)
#define MAX_FRM_GRID_NUM                (16)
#define MAX_TIL_GRID_NUM                (32)
#define COEFF_BITS_PER_CH               (192)
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
    MUINT8     GainTable[2312]; //00DC   Capture Shading Table (17 X 17 X 2 X 4 bytes)       2312
    //MUINT8     UnitGainTable[2048]; //2048
} SHADING_GOLDEN_REF;

#define SHADING_DATA                                                                \
    struct {                                                                        \
        UINT32 Version;                                                             \
        UINT32 SensorId;                                                            \
        UINT8  GridXNum;                                                            \
        UINT8  GridYNum;                                                            \
        UINT16 Width;                                                               \
        UINT16 Height;                                                              \
        UINT32 CapTable[SHADING_SUPPORT_CT_NUM][MAX_SHADING_PvwFrm_SIZE];           \
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

//____3A NVRAM____
//typedef unsigned char  UINT8;
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
    UINT8 reserved[MAXIMUM_NVRAM_CAMERA_3A_FILE_SIZE-sizeof(UINT32)-sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM-sizeof(AWB_NVRAM_T)*CAM_SCENARIO_NUM-sizeof(FLASH_AWB_NVRAM_T)];
} NVRAM_CAMERA_3A_STRUCT, *PNVRAM_CAMERA_3A_STRUCT;

/*******************************************************************************
* ISP NVRAM parameter
********************************************************************************/
#define NVRAM_SUPPORT_SENSOR            (10) //10 sensor mode

#define NVRAM_SL2F_TBL_NUM              (NVRAM_SUPPORT_SENSOR)
#define NVRAM_DBS_TBL_NUM               (920)
#define NVRAM_OBC_TBL_NUM               (184)
#define NVRAM_BPC_TBL_NUM               (920)
#define NVRAM_NR1_TBL_NUM               (920)
#define NVRAM_PDC_TBL_NUM               (920)
#define NVRAM_RMM_TBL_NUM               (200)
#define NVRAM_HLR_TBL_NUM               (940)
#define NVRAM_RNR_TBL_NUM               (920)
#define NVRAM_SL2_TBL_NUM               (51)
#define NVRAM_UDM_TBL_NUM               (1040)
#define NVRAM_GGM_TBL_NUM               (5)
#define NVRAM_IHDR_GGM_TBL_NUM          (16)
#define NVRAM_ANR_TBL_NUM               (1280)
#define NVRAM_ANR2_TBL_NUM              (1280)
#define NVRAM_CCR_TBL_NUM               (360)
#define NVRAM_BOK_TBL_NUM               (60)
#define NVRAM_HFG_TBL_NUM               (1140)
#define NVRAM_EE_TBL_NUM                (1160)
#define NVRAM_NR3D_TBL_NUM              (640)
#define NVRAM_MFB_TBL_NUM               (120)
#define NVRAM_MIXER3_TBL_NUM            (120)
//#define NVRAM_CCM_CFC_TBL_NUM           (345)
#define NVRAM_ISP_REGS_ISO_SUPPORT_NUM  (20)
#define NVRAM_ABF_TBL_NUM               (920)
#define NVRAM_ISP_REGS_ZOOM_SUPPORT_NUM  (3)

// camera common parameters and sensor parameters
typedef struct
{
    UINT32 CommReg[64];
} ISP_NVRAM_COMMON_STRUCT, *PISP_NVRAM_COMMON_STRUCT;

typedef struct ISP_NVRAM_ISO_INTERVAL_STRUCT
{
    MUINT16 u2Length;
    INT16 IDX_Partition[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
} ISP_NVRAM_ISO_INTERVAL_T, *PISP_NVRAM_ISO_INTERVAL_T;

typedef struct ISP_NVRAM_ZOOM_INTERVAL_STRUCT
{
    MUINT16 u2Length;
    INT16 IDX_Partition[NVRAM_ISP_REGS_ZOOM_SUPPORT_NUM];
} ISP_NVRAM_ZOOM_INTERVAL_T, *PISP_NVRAM_ZOOM_INTERVAL_T;

typedef struct ISP_NVRAM_REG_INDEX_STRUCT
{
    UINT16  DBS;
    UINT16  OBC;
    UINT16  BNR_BPC;
    UINT16  BNR_NR1;
    UINT16  BNR_PDC;
    UINT16  RMM;
    //UINT16  HLR;
    UINT16  RNR;
    UINT16  UDM;
    UINT16  ANR;
    UINT16  ANR2;
    //UINT16  ABF;
    UINT16  CCR;
    UINT16  BOK;
    UINT16  HFG;
    UINT16  EE;
    UINT16  NR3D;
    UINT16  MFB;
    UINT16  MIXER3;

    UINT16  SL2F;
    UINT16  SL2J;
    UINT16  SL2;
    UINT16  CCM;
    UINT16  GGM;
    UINT16  IHDR_GGM;
    UINT16  COLOR;

    //UINT16 PCA;
    //UINT16 ANR_TBL;
} ISP_NVRAM_REG_INDEX_T, *PISP_NVRAM_REG_INDEX_T;

typedef struct
{
    ISP_NVRAM_ISO_INTERVAL_T    Iso_Env;
    ISP_NVRAM_ZOOM_INTERVAL_T   Zoom_Env;
    ISP_NVRAM_REG_INDEX_T       Idx;
    ISP_NVRAM_SL2_T             SL2F[NVRAM_SL2F_TBL_NUM];
    ISP_NVRAM_DBS_T             DBS[NVRAM_DBS_TBL_NUM];
    ISP_NVRAM_OBC_T             OBC[NVRAM_OBC_TBL_NUM];
    ISP_NVRAM_BNR_BPC_T         BNR_BPC[NVRAM_BPC_TBL_NUM];
    ISP_NVRAM_BNR_NR1_T         BNR_NR1[NVRAM_NR1_TBL_NUM];
    ISP_NVRAM_BNR_PDC_T         BNR_PDC[NVRAM_PDC_TBL_NUM];
    ISP_NVRAM_RMM_T             RMM[NVRAM_RMM_TBL_NUM];
    ISP_NVRAM_RNR_T             RNR[NVRAM_RNR_TBL_NUM];
    ISP_NVRAM_SL2_T             SL2[NVRAM_SL2_TBL_NUM];
    ISP_NVRAM_UDM_T             UDM[NVRAM_UDM_TBL_NUM];
    ISP_NVRAM_ANR_T             ANR[NVRAM_ANR_TBL_NUM];
    ISP_NVRAM_ANR2_T            ANR2[NVRAM_ANR2_TBL_NUM];
    ISP_NVRAM_CCR_T             CCR[NVRAM_CCR_TBL_NUM];
    ISP_NVRAM_BOK_T             BOK[NVRAM_BOK_TBL_NUM];
    ISP_NVRAM_HFG_T             HFG[NVRAM_HFG_TBL_NUM];
    ISP_NVRAM_EE_T              EE[NVRAM_EE_TBL_NUM];
    ISP_NVRAM_NR3D_T            NR3D[NVRAM_NR3D_TBL_NUM];
    ISP_NVRAM_MFB_T             MFB[NVRAM_MFB_TBL_NUM];
    ISP_NVRAM_MIXER3_T          MIXER3[NVRAM_MIXER3_TBL_NUM];
    //ISP_NVRAM_HLR_T             HLR[NVRAM_HLR_TBL_NUM];
    //ISP_NVRAM_ABF_T             ABF[NVRAM_ABF_TBL_NUM];
} ISP_NVRAM_REGISTER_STRUCT, *PISP_NVRAM_REGISTER_STRUCT;

/*
*   GMA NVRAM Param
*/

#define ISP_NVRAM_GMA_NUM                               (12)     // Normal/HDRon/AutoHDR preview/capture/video + 3 reserved
#define ISP_NVRAM_GMA_CONTRAST_WEIGHTING_TBL_NUM        (11)
#define ISP_NVRAM_GMA_LV_WEIGHTING_TBL_NUM              (20)

typedef enum {
    eISP_FIXED_GMA_MODE = 0,
    eISP_DYNAMIC_GMA_MODE
} eISP_GMA_MODE_T;


typedef struct {
    MINT32 i4HdrContrastWtTbl[ISP_NVRAM_GMA_CONTRAST_WEIGHTING_TBL_NUM];
    MINT32 i4HdrLVWtTbl[ISP_NVRAM_GMA_LV_WEIGHTING_TBL_NUM];
    MINT32 i4NightContrastWtTbl[ISP_NVRAM_GMA_CONTRAST_WEIGHTING_TBL_NUM];
    MINT32 i4NightLVWtTbl[ISP_NVRAM_GMA_LV_WEIGHTING_TBL_NUM];
} ISP_NVRAM_GMA_TUNING_LUT_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4CenterPt;
    MINT32 i4LowPercent;
    MINT32 i4LowCurve100;
    MINT32 i4HighCurve100;
    MINT32 i4HighPercent;
    MINT32 i4SlopeH100;
    MINT32 i4SlopeL100;
} ISP_NVRAM_GMA_TUNING_CURVE_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4WaitAEStable;
    MINT32 i4Speed;     // 0 ~ 10
} ISP_NVRAM_GMA_TUNING_SMOOTH_T;

typedef struct {
    MINT32 i4Enable;
} ISP_NVRAM_GMA_TUNING_FLARE_T;


typedef struct {
    eISP_GMA_MODE_T i4GMAMode;  // 0: Fixed Gamma  1: Dynamic Gamma 2: Adaptive Gamma
    MINT32 i4LowContrastThr;
    MINT32 i4LowContrastRatio;
    MINT32 i4LowContrastSeg;
    ISP_NVRAM_GMA_TUNING_LUT_T rGMALUTs;
    ISP_NVRAM_GMA_TUNING_SMOOTH_T rGMASmooth;
    ISP_NVRAM_GMA_TUNING_CURVE_T rGMACurve;
    ISP_NVRAM_GMA_TUNING_FLARE_T rGMAFlare;
} ISP_NVRAM_GMA_TUNING_PARAM_T;

/*
*   LCE NVRAM Param
*/
#define ISP_NVRAM_LCE_NUM                               (12)     // Normal/HDRon/AutoHDR x Preview/Capture/Video + 3 reversed
#define ISP_NVRAM_LCE_LV_TBL_NUM                        (19)
#define ISP_NVRAM_LCE_CONTRAST_TBL_NUM                  (11)
#define ISP_NVRAM_LCE_DETAIL_TBL_NUM                    (11)
#define ISP_NVRAM_LCE_DIFF_RANGE_BIN_NUM                (12)


typedef struct {
    MINT32 i4vTbl1[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl2[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl3[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl4[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4DetailWtTbl[ISP_NVRAM_LCE_DETAIL_TBL_NUM];
    MINT32 i4CenSlopeEnhance[ISP_NVRAM_LCE_DIFF_RANGE_BIN_NUM];
} ISP_NVRAM_LCE_TUNING_LUT_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4WaitAEStable;
    MINT32 i4Speed;     // 0 ~ 10
} ISP_NVRAM_LCE_TUNING_SMOOTH_T;

typedef struct {
    MINT32 i4Enable;
} ISP_NVRAM_LCE_TUNING_FLARE_T;

typedef struct {
    MINT32 i4HlrRatio;
    MINT32 i4HlrKeepRatio;
    MINT32 i4HlrSMin;
    MINT32 i4HlrScarRatio;
    MINT32 i4HlrFixEn;
} ISP_NVRAM_LCE_HLR_TUNING_T;


typedef struct {
    MINT32 i4HistCCLB;
    MINT32 i4HistCCUB;

    MINT32 i4LCESeg;
    MINT32 i4LCEContrastRatio;
    MINT32 i4LCEHistYThr;
    MINT32 i4SlopeLow;
    MINT32 i4SlopeHigh;
    MINT32 i4SlopeTop;
    MINT32 i4DetailRangeRatio;
    MINT32 i4CenSlopeMin;
    MINT32 i4vBlacKeep;
    MINT32 i4vFixedPt1;
    MINT32 i4vFixedPt2;
    MINT32 i4vFixedPt3;
    MINT32 i4vFixedPt4;
    MINT32 i4TCPLB;
    MINT32 i4TCPUB;

    MINT32 i4Reserved0;
    MINT32 i4Reserved1;
    MINT32 i4Reserved2;
    MINT32 i4Reserved3;
    MINT32 i4Reserved4;

    ISP_NVRAM_LCE_TUNING_LUT_T rLCELUTs;
    ISP_NVRAM_LCE_TUNING_SMOOTH_T rLCESmooth;
    ISP_NVRAM_LCE_TUNING_FLARE_T rLCEFlare;
    ISP_NVRAM_LCE_HLR_TUNING_T rLCEHlrTbl;
}ISP_AUTO_LCE_TUNING_T;

typedef struct {
    MINT32 i4AutoHDREnable;
    ISP_AUTO_LCE_TUNING_T rAutoLCEParam[ISP_NVRAM_LCE_NUM];
} ISP_NVRAM_LCE_TUNING_PARAM_T;

typedef struct{
    ISP_NVRAM_GGM_T                 GGM[NVRAM_GGM_TBL_NUM];
    ISP_NVRAM_GGM_T                 IHDR_GGM[NVRAM_IHDR_GGM_TBL_NUM];
    ISP_NVRAM_GMA_TUNING_PARAM_T    rGmaParam[ISP_NVRAM_GMA_NUM];
    ISP_NVRAM_LCE_TUNING_PARAM_T    rLceParam;
}ISP_NVRAM_TONEMAP_PARAM_T;

/*
*   CCM NVRAM Param
*/
#define ISP_NVRAM_CCM_CT_NUM                            (8)
#define ISP_NVRAM_CCM_LV_NUM                            (6)
#define ISP_NVRAM_CCM_FLASH_NUM                         (2)
#define ISP_NVRAM_DYNAMIC_CCM_NUM                       (4)

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
    MUINT32                       CCM_Method;
    ISP_NVRAM_CCM_T               dynamic_CCM[ISP_NVRAM_DYNAMIC_CCM_NUM];
    AWB_GAIN_T                    dynamic_CCM_AWBGain[ISP_NVRAM_DYNAMIC_CCM_NUM];
    MUINT32                       PQ_Type;
    MUINT32                       CCM_LV_valid_NUM;
    MUINT32                       CCM_FLASH_valid_NUM;
    MUINT32                       CCM_CT_valid_NUM;
    ISP_NVRAM_CCM_T               CCM[(ISP_NVRAM_CCM_LV_NUM + ISP_NVRAM_CCM_FLASH_NUM)][ISP_NVRAM_CCM_CT_NUM];
    AWB_GAIN_T                    AWBGain[(ISP_NVRAM_CCM_LV_NUM + ISP_NVRAM_CCM_FLASH_NUM)][ISP_NVRAM_CCM_CT_NUM];
    MINT32                        Weight[(ISP_NVRAM_CCM_LV_NUM + ISP_NVRAM_CCM_FLASH_NUM)][ISP_NVRAM_CCM_CT_NUM];
    ISP_NVRAM_CCM_POLY22_STRUCT   Poly22[(ISP_NVRAM_CCM_LV_NUM + ISP_NVRAM_CCM_FLASH_NUM)];
} ISP_NVRAM_MULTI_CCM_STRUCT, *PISP_NVRAM_MULTI_CCM_STRUCT;


#define ISP_NVRAM_COLOR_SCENE_NUM                         (4)
#define ISP_NVRAM_COLOR_CT_NUM                            (5)

typedef struct{
    MUINT32 u4Ratio_Max; //(0~64, 1x=16)
	MUINT32 u4Ratio_TH; //(0~64, 1x=16)
    MUINT32 u4Ratio_Min; //(0~64, 1x=16)
    MUINT32 u4LV_THA; //(0~255)
    MUINT32 u4LV_THB; //(0~255)
    MUINT32 u4LV_THC; //(0~255)
    MUINT32 u4LV_THD; //(0~255)
    MUINT32 u4DC_Offset; //(0~255, 0x~2x)
    MUINT32 u4Lum_Speed; // 0~32
    MUINT32 u4Hue_Speed; // 0~32
    MUINT32 u4Sat_Speed; // 0~32
    MUINT32 u4LSP_LV_TH; //(0~255)
    MUINT32 u4Outdoor_EN; // 1: on, 0: off
    MUINT32 u4Outdoor_THA; //(0~255)
    MUINT32 u4Outdoor_THB; //(0~255)
    MUINT32 u4Outdoor_Speed; // 0~32
    MUINT32 u4Outdoor_Ratio; // default is 16
} ISP_NVRAM_PCA_PARAM_T;


typedef struct{
    MUINT32 u4FD_CM_Enable;
    MUINT32 u4Priority;
    MUINT32 u4Range_W1_Y;
    MUINT32 u4Range_W1_S;
    MUINT32 u4Range_W1_H;
    MUINT32 u4Offset_W1_Y;
    MUINT32 u4Offset_W1_S;
    MUINT32 u4Offset_W1_H;
    MUINT32 u4Range_W2_Y;
    MUINT32 u4Range_W2_S;
    MUINT32 u4Range_W2_H;
    MUINT32 u4Offset_W2_Y;
    MUINT32 u4Offset_W2_S;
    MUINT32 u4Offset_W2_H;
    MUINT32 u4Range_W3_Y;
    MUINT32 u4Range_W3_S;
    MUINT32 u4Range_W3_H;
    MUINT32 u4Offset_W3_Y;
    MUINT32 u4Offset_W3_S;
    MUINT32 u4Offset_W3_H;
    MUINT32 u4Backup_0;
    MUINT32 u4Backup_1;
    MUINT32 u4Backup_2;
    MUINT32 u4Backup_3;
    MUINT32 u4Backup_4;
    MUINT32 u4Backup_5;
    MUINT32 u4Backup_6;
    MUINT32 u4Backup_7;
    MUINT32 u4Backup_8;
    MUINT32 u4Backup_9;
    MUINT32 u4Backup_10;
    MUINT32 u4Backup_11;
    MUINT32 u4Backup_12;
    MUINT32 u4Backup_13;
    MUINT32 u4Backup_14;
    MUINT32 u4Backup_15;
    MUINT32 u4Backup_16;
    MUINT32 u4Backup_17;
    MUINT32 u4Backup_18;
    MUINT32 u4Backup_19;
    MUINT32 u4Backup_20;
    MUINT32 u4Backup_21;
    MUINT32 u4Backup_22;
    MUINT32 u4Backup_23;
} ISP_NVRAM_FD_COLOR_PARAM_T;

typedef struct{
    MUINT32 u4FD_ANR_LUT_Enable;
    MUINT32 u4Range_W1_Y;
    MUINT32 u4Range_W1_U;
    MUINT32 u4Range_W1_V;
    MUINT32 u4W1_enable;
    MUINT32 u4Gain_W1_Y;
    MUINT32 u4Gain_W1_U;
    MUINT32 u4Gain_W1_V;
    MUINT32 u4Range_W2_Y;
    MUINT32 u4Range_W2_U;
    MUINT32 u4Range_W2_V;
    MUINT32 u4W2_enable;
    MUINT32 u4Gain_W2_Y;
    MUINT32 u4Gain_W2_U;
    MUINT32 u4Gain_W2_V;
    MUINT32 u4Range_W3_Y;
    MUINT32 u4Range_W3_U;
    MUINT32 u4Range_W3_V;
    MUINT32 u4W3_enable;
    MUINT32 u4Gain_W3_Y;
    MUINT32 u4Gain_W3_U;
    MUINT32 u4Gain_W3_V;
    MUINT32 u4Range_W4_Y;
    MUINT32 u4Range_W4_U;
    MUINT32 u4Range_W4_V;
    MUINT32 u4W4_enable;
    MUINT32 u4Gain_W4_Y;
    MUINT32 u4Gain_W4_U;
    MUINT32 u4Gain_W4_V;
    MUINT32 u4Range_W5_Y;
    MUINT32 u4Range_W5_U;
    MUINT32 u4Range_W5_V;
    MUINT32 u4W5_enable;
    MUINT32 u4Gain_W5_Y;
    MUINT32 u4Gain_W5_U;
    MUINT32 u4Gain_W5_V;
    MUINT32 u4Backup_0;
    MUINT32 u4Backup_1;
    MUINT32 u4Backup_2;
    MUINT32 u4Backup_3;
    MUINT32 u4Backup_4;
    MUINT32 u4Backup_5;
    MUINT32 u4Backup_6;
    MUINT32 u4Backup_7;
    MUINT32 u4Backup_8;
    MUINT32 u4Backup_9;
    MUINT32 u4Backup_10;
    MUINT32 u4Backup_11;
    MUINT32 u4Backup_12;
    MUINT32 u4Backup_13;
    MUINT32 u4Backup_14;
    MUINT32 u4Backup_15;
    MUINT32 u4Backup_16;
    MUINT32 u4Backup_17;
    MUINT32 u4Backup_18;
    MUINT32 u4Backup_19;
    MUINT32 u4Backup_20;
    MUINT32 u4Backup_21;
    MUINT32 u4Backup_22;
    MUINT32 u4Backup_23;
} ISP_NVRAM_FD_ANR_PARAM_T;

typedef struct
{
    MUINT32                       COLOR_Method;
    ISP_NVRAM_PCA_T               Config;
    INT16                         COLOR_CT_Env[ISP_NVRAM_COLOR_CT_NUM];
    ISP_NVRAM_PCA_PARAM_T         rPcaParam[ISP_NVRAM_COLOR_SCENE_NUM];
    ISP_NVRAM_PCA_LUT_T           PCA_LUT[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM];
    ISP_NVRAM_COLOR_T             COLOR[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM];
} ISP_NVRAM_COLOR_STRUCT, *PISP_NVRAM_COLOR_STRUCT;

#define ISP_NVRAM_ANR_TBL_CT_NUM        (4)
#define ISP_NVRAM_ANR_TBL_LV_NUM        (8)

typedef struct
{
    ISP_NVRAM_MULTI_CCM_STRUCT    ISPMulitCCM;
    ISP_NVRAM_ANR_LUT_T           ANR_TBL[ISP_NVRAM_ANR_TBL_CT_NUM][ISP_NVRAM_ANR_TBL_LV_NUM];
    ISP_NVRAM_COLOR_STRUCT        ISPColor;
    ISP_NVRAM_NR3D_PARAS_T        NR3D_PARA;
    ISP_NVRAM_FD_COLOR_PARAM_T    FD_COLOR[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM];
    ISP_NVRAM_FD_ANR_PARAM_T      FD_ANR[ISP_NVRAM_ANR_TBL_LV_NUM];
} ISP_NVRAM_COLOR_TABLE_STRUCT, *PISP_NVRAM_COLOR_TABLE_STRUCT;


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
        MUINT32                         Version;
        MUINT32                         SensorId;    // ID of sensor module
        ISP_NVRAM_COMMON_STRUCT         ISPComm;
        ISP_NVRAM_REGISTER_STRUCT       ISPRegs;
        ISP_NVRAM_COLOR_TABLE_STRUCT    ISPColorTbl;
        ISP_NVRAM_TONEMAP_PARAM_T       ISPToneMap;
        ISP_NVRAM_DNG_METADATA_T        DngMetadata;
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
        NUM_SL2F        =   NVRAM_SL2F_TBL_NUM,
        NUM_SL2J        =   NVRAM_SL2_TBL_NUM,
        NUM_DBS         =   NVRAM_DBS_TBL_NUM,
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_BNR_BPC     =   NVRAM_BPC_TBL_NUM,
        NUM_BNR_NR1     =   NVRAM_NR1_TBL_NUM,
        NUM_BNR_PDC     =   NVRAM_PDC_TBL_NUM,
        NUM_RMM         =   NVRAM_RMM_TBL_NUM,
        NUM_RNR         =   NVRAM_RNR_TBL_NUM,
        NUM_SL2         =   NVRAM_SL2_TBL_NUM,
        NUM_UDM         =   NVRAM_UDM_TBL_NUM,
        NUM_CCM         =   ISP_NVRAM_CCM_CT_NUM,
        NUM_GGM         =   NVRAM_GGM_TBL_NUM,
        NUM_IHDR_GGM    =   NVRAM_IHDR_GGM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_ANR2        =   NVRAM_ANR2_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
        NUM_BOK         =   NVRAM_BOK_TBL_NUM,
        NUM_HFG         =   NVRAM_HFG_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM,
        NUM_NR3D        =   NVRAM_NR3D_TBL_NUM,
        NUM_MFB         =   NVRAM_MFB_TBL_NUM,
        NUM_MIXER3      =   NVRAM_MIXER3_TBL_NUM,
        NUM_HLR         =   NVRAM_HLR_TBL_NUM,
        NUM_COLOR       =   (ISP_NVRAM_COLOR_SCENE_NUM * ISP_NVRAM_COLOR_CT_NUM),
        NUM_ABF         =   NVRAM_ABF_TBL_NUM,
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
    inline bool setIdx_SL2F(UINT16 const idx)        { return setIdx(m_rIdx.SL2F, idx, NUM_SL2F); }
    inline bool setIdx_SL2J(UINT16 const idx)        { return setIdx(m_rIdx.SL2J, idx, NUM_SL2J); }
    inline bool setIdx_DBS(UINT16 const idx)        { return setIdx(m_rIdx.DBS, idx, NUM_DBS); }
    inline bool setIdx_OBC(UINT16 const idx)        { return setIdx(m_rIdx.OBC, idx, NUM_OBC); }
    inline bool setIdx_BPC(UINT16 const idx)        { return setIdx(m_rIdx.BNR_BPC, idx, NUM_BNR_BPC); }
    inline bool setIdx_NR1(UINT16 const idx)        { return setIdx(m_rIdx.BNR_NR1, idx, NUM_BNR_NR1); }
    inline bool setIdx_PDC(UINT16 const idx)        { return setIdx(m_rIdx.BNR_PDC, idx, NUM_BNR_PDC); }
    inline bool setIdx_RMM(UINT16 const idx)        { return setIdx(m_rIdx.RMM, idx, NUM_RMM); }
    inline bool setIdx_RNR(UINT16 const idx)        { return setIdx(m_rIdx.RNR, idx, NUM_RNR); }
    inline bool setIdx_SL2(UINT16 const idx)        { return setIdx(m_rIdx.SL2, idx, NUM_SL2); }
    inline bool setIdx_UDM(UINT16 const idx)        { return setIdx(m_rIdx.UDM, idx, NUM_UDM); }
    inline bool setIdx_CCM(UINT16 const idx)        { return setIdx(m_rIdx.CCM, idx, NUM_CCM); }
    //inline bool setIdx_CCM_CFC(UINT16 const idx)        { return setIdx(m_rIdx.CCM_CFC, idx, NUM_CCM_CFC); }
    inline bool setIdx_GGM(UINT16 const idx)        { return setIdx(m_rIdx.GGM, idx, NUM_GGM); }
    inline bool setIdx_IHDR_GGM(UINT16 const idx)   { return setIdx(m_rIdx.IHDR_GGM, idx, NUM_IHDR_GGM); }
    inline bool setIdx_ANR(UINT16 const idx)        { return setIdx(m_rIdx.ANR, idx, NUM_ANR); }
    inline bool setIdx_ANR2(UINT16 const idx)        { return setIdx(m_rIdx.ANR2, idx, NUM_ANR2); }
    inline bool setIdx_CCR(UINT16 const idx)        { return setIdx(m_rIdx.CCR, idx, NUM_CCR); }
    inline bool setIdx_BOK(UINT16 const idx)        { return setIdx(m_rIdx.BOK, idx, NUM_BOK); }
    inline bool setIdx_HFG(UINT16 const idx)         { return setIdx(m_rIdx.HFG, idx, NUM_HFG); }
    inline bool setIdx_EE(UINT16 const idx)         { return setIdx(m_rIdx.EE, idx, NUM_EE); }
    inline bool setIdx_NR3D(UINT16 const idx)       { return setIdx(m_rIdx.NR3D, idx, NUM_NR3D); }
    inline bool setIdx_MFB(UINT16 const idx)        { return setIdx(m_rIdx.MFB, idx, NUM_MFB); }
    inline bool setIdx_MIX3(UINT16 const idx)        { return setIdx(m_rIdx.MIXER3, idx, NUM_MIXER3); }
    //inline bool setIdx_HLR(UINT16 const idx)        { return setIdx(m_rIdx.HLR, idx, NUM_HLR); }
    inline bool setIdx_COLOR(UINT16 const idx)        { return setIdx(m_rIdx.COLOR, idx, NUM_COLOR); }
    //inline bool setIdx_ABF(UINT16 const idx)        { return setIdx(m_rIdx.ABF, idx, NUM_ABF); }


public:     ////    Get Index

    inline ISP_NVRAM_REG_INDEX_STRUCT     getIdx() const { return m_rIdx; }

    inline UINT16 getIdx_SL2F()       const { return m_rIdx.SL2F; }
    inline UINT16 getIdx_SL2J()       const { return m_rIdx.SL2J; }
    inline UINT16 getIdx_DBS()       const { return m_rIdx.DBS; }
    inline UINT16 getIdx_OBC()       const { return m_rIdx.OBC; }
    inline UINT16 getIdx_BPC()       const { return m_rIdx.BNR_BPC; }
    inline UINT16 getIdx_NR1()       const { return m_rIdx.BNR_NR1; }
    inline UINT16 getIdx_PDC()       const { return m_rIdx.BNR_PDC; }
    inline UINT16 getIdx_RMM()       const { return m_rIdx.RMM; }
    inline UINT16 getIdx_RNR()       const { return m_rIdx.RNR; }
    inline UINT16 getIdx_SL2()       const { return m_rIdx.SL2; }
    inline UINT16 getIdx_UDM()       const { return m_rIdx.UDM; }
    inline UINT16 getIdx_CCM()       const { return m_rIdx.CCM; }
    //inline UINT16 getIdx_CCM_CFC()       const { return m_rIdx.CCM_CFC; }
    inline UINT16 getIdx_GGM()       const { return m_rIdx.GGM; }
    inline UINT16 getIdx_IHDR_GGM()  const { return m_rIdx.IHDR_GGM; }
    inline UINT16 getIdx_ANR()       const { return m_rIdx.ANR; }
    inline UINT16 getIdx_ANR2()       const { return m_rIdx.ANR2; }
    inline UINT16 getIdx_CCR()       const { return m_rIdx.CCR; }
    inline UINT16 getIdx_BOK()       const { return m_rIdx.BOK; }
    inline UINT16 getIdx_HFG()        const { return m_rIdx.HFG; }
    inline UINT16 getIdx_EE()        const { return m_rIdx.EE; }
    inline UINT16 getIdx_NR3D()      const { return m_rIdx.NR3D; }
    inline UINT16 getIdx_MFB()       const { return m_rIdx.MFB; }
    inline UINT16 getIdx_MIX3()       const { return m_rIdx.MIXER3; }
    inline UINT16 getIdx_COLOR()        const { return m_rIdx.COLOR; }
//    inline UINT16 getIdx_HLR()        const { return m_rIdx.HLR; }
//    inline UINT16 getIdx_ABF()        const { return m_rIdx.ABF; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    inline ISP_NVRAM_SL2_T&         getSL2F(UINT16 const idx) { return m_rRegs.SL2F[idx]; }
    inline ISP_NVRAM_DBS_T&         getDBS(UINT16 const idx) { return m_rRegs.DBS[idx]; }
    inline ISP_NVRAM_OBC_T&         getOBC(UINT16 const idx) { return m_rRegs.OBC[idx]; }
    inline ISP_NVRAM_BNR_BPC_T&         getBPC(UINT16 const idx) { return m_rRegs.BNR_BPC[idx]; }
    inline ISP_NVRAM_BNR_NR1_T&         getNR1(UINT16 const idx) { return m_rRegs.BNR_NR1[idx]; }
    inline ISP_NVRAM_BNR_PDC_T&         getPDC(UINT16 const idx) { return m_rRegs.BNR_PDC[idx]; }
    inline ISP_NVRAM_RMM_T&         getRMM(UINT16 const idx)  { return m_rRegs.RMM[idx]; }
    //inline ISP_NVRAM_HLR_T&         getHLR(UINT16 const idx)  { return m_rRegs.HLR[idx]; }
    inline ISP_NVRAM_RNR_T&         getRNR(UINT16 const idx)  { return m_rRegs.RNR[idx]; }
    inline ISP_NVRAM_SL2_T&         getSL2(UINT16 const idx) { return m_rRegs.SL2[idx]; }
    inline ISP_NVRAM_UDM_T&         getUDM(UINT16 const idx) { return m_rRegs.UDM[idx]; }
    //inline ISP_NVRAM_CCM_T&         getCCM(UINT16 const idx) { return m_rRegs.CCM[idx]; }
    //inline ISP_NVRAM_CCM_CTL_T&     getCCM_CFC(UINT16 const idx)  { return m_rRegs.CCM_CFC[idx]; }
    //inline ISP_NVRAM_GGM_T&         getGGM(UINT16 const idx) { return m_rRegs.GGM[idx]; }
    //inline ISP_NVRAM_GGM_T&         getIHDRGGM(UINT16 const idx) { return m_rRegs.IHDR_GGM[idx]; }
    inline ISP_NVRAM_ANR_T&         getANR(UINT16 const idx) { return m_rRegs.ANR[idx]; }
    inline ISP_NVRAM_ANR2_T&        getANR2(UINT16 const idx) { return m_rRegs.ANR2[idx]; }
    inline ISP_NVRAM_CCR_T&         getCCR(UINT16 const idx) { return m_rRegs.CCR[idx]; }
    inline ISP_NVRAM_BOK_T&         getBOK(UINT16 const idx) { return m_rRegs.BOK[idx]; }
    inline ISP_NVRAM_HFG_T&          getHFG(UINT16 const idx)  { return m_rRegs.HFG[idx]; }
    inline ISP_NVRAM_EE_T&          getEE(UINT16 const idx)  { return m_rRegs.EE[idx]; }
    inline ISP_NVRAM_NR3D_T&        getNR3D(UINT16 const idx) { return m_rRegs.NR3D[idx]; }
    inline ISP_NVRAM_MFB_T&         getMFB(UINT16 const idx)  { return m_rRegs.MFB[idx]; }
    inline ISP_NVRAM_MIXER3_T&      getMIX3(UINT16 const idx)  { return m_rRegs.MIXER3[idx]; }
 //   inline ISP_NVRAM_ABF_T&         getABF(UINT16 const idx)  { return m_rRegs.ABF[idx]; }

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
    AWB_GAIN_T                      rCalGain;
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
    CAMERA_NVRAM_DATA_FOV,
    CAMERA_NVRAM_DATA_FEATURE,
    CAMERA_NVRAM_DATA_AF,
    CAMERA_NVRAM_DATA_AF_CALIBRATION = CAMERA_NVRAM_DATA_AF,
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
            short ispVer[SENSOR_IDX_MAX]; //main, sub, main2
            short aaaVer[SENSOR_IDX_MAX];
            short shadingVer[SENSOR_IDX_MAX];
            short lensVer[SENSOR_IDX_MAX];
            short aePlineVer[SENSOR_IDX_MAX];
            short strobeVer[SENSOR_IDX_MAX];
            short tsfVer[SENSOR_IDX_MAX];
            short pdcVer[SENSOR_IDX_MAX];
            short geometryVer[SENSOR_IDX_MAX];
            short fovVer[SENSOR_IDX_MAX];
            short featureVer[SENSOR_IDX_MAX];
            short afVer[SENSOR_IDX_MAX];
            short flashCalibrationVer[SENSOR_IDX_MAX];
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_VERSION_FILE_SIZE];
}NVRAM_CAMERA_VERSION_STRUCT;


typedef struct // maximum 2048 bytes
{
    // MFLL/AIS (12 bytes, total: 12 bytes)
    MUINT8  capture_frame_number;       //default=4, range=3~4, step=1
    MUINT8  blend_frame_number;         //default=4, range=3~4, step=1
    MUINT8  full_size_mc;               //default=0, 0: for using quarter size mc, otherwiase: full size
    MUINT32 memc_bad_mv_range;          //default=255
    MUINT32 memc_bad_mv_rate_th;        //default=12707

    // MFLL (4 bytes, total: 16 bytes)
    MUINT16 mfll_iso_th;                //default=800, range=0~65535, step=1
    MUINT16 pline_iso_th;               //default=800, range=0~65535, step=1

    // Adaptive frame number (24 bytes, total: 40 bytes)
    MUINT16 iso_level1;
    MUINT16 iso_level2;
    MUINT16 iso_level3;
    MUINT16 iso_level4;
    MUINT16 iso_level5;
    MUINT16 iso_level6;                 //reserved for extension
    MUINT16 iso_level7;                 //reserved for extension
    MUINT8  frame_num1;
    MUINT8  frame_num2;
    MUINT8  frame_num3;
    MUINT8  frame_num4;
    MUINT8  frame_num5;
    MUINT8  frame_num6;
    MUINT8  frame_num7;                 // reserved for extension
    MUINT8  frame_num8;                 // reserved for extension

    // MEMC level map (44 bytes, total 84 bytes) , since MEMC v1.1
    MUINT32 memc_iso_th0;
    MUINT32 memc_iso_th1;
    MUINT32 memc_iso_th2;
    MUINT32 memc_iso_th3;
    MUINT32 memc_iso_th4;
    MUINT32 memc_lv_val0;
    MUINT32 memc_lv_val1;
    MUINT32 memc_lv_val2;
    MUINT32 memc_lv_val3;
    MUINT32 memc_lv_val4;
    MUINT32 memc_lv_val5;

    // AIS w/ gyro info (36 bytes, total 120 bytes) , since Strategy v1.3
    float   fRsc;                       // needs calibration
    float   fBiasX;                     // needs calibration
    float   fBiasY;                     // needs calibration
    float   fBiasZ;                     // needs calibration
    float   fFocal;                     // needs calibration
    float   fTofst;                     // needs calibration
    MUINT8  uMotionBase;                // default=120,             step=1
    MUINT8  uMVThLow;                   // default=1,               step=1
    MUINT8  uMVThHigh;                  // default=3,               step=1
    MUINT32 u4MinExpUs;                 // default=16000us (16ms),  step=1
    MUINT32 u4MaxIso;                   // default=3200,            step=1
    MUINT32 u4ParamWidth;               // default=????,            step=1
    MUINT32 u4ParamHeight;              // default=????,            step=1

    // reserved space
    MUINT32 reserved[480];

} NVRAM_CAMERA_FEATURE_MFLL_STRUCT, *PNVRAM_CAMERA_FEATURE_MFLL_STRUCT;

static_assert( sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT) <= 2048,
        "NVRAM_CAMERA_FEATURE_MFLL_STRUCT is greater 2048 bytes, please make sure " \
        "it's smaller 2048 bytes");

#if 0 // debug NVRAM_CAMERA_FEATURE_MFLL_STRUCT structure size usage
template<int s> struct NVRAM_CAMERA_FEATURE_MFLL_STRUCT_SIZE_IS;
NVRAM_CAMERA_FEATURE_MFLL_STRUCT_SIZE_IS
<sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT)> n_______n;
#endif

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


struct NR_PARAM
{
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
};

struct NR2_PARAM
{
    MINT32 NR_K;
    MINT32 NR_S;
    MINT32 NR_SD;
    MINT32 NR_BLD_W;
    MINT32 NR_BLD_TH;
    MINT32 NR_SMTH;
    MINT32 NR_NTRL_TH_1_Y;
    MINT32 NR_NTRL_TH_2_Y;
    MINT32 NR_NTRL_TH_1_UV;
    MINT32 NR_NTRL_TH_2_UV;
};

struct HFG_PARAM
{
    MINT32 HFG_ENABLE;
    MINT32 HFG_GSD;
    MINT32 HFG_SD0;
    MINT32 HFG_SD1;
    MINT32 HFG_SD2;
    MINT32 HFG_TX_S;
    MINT32 HFG_LCE_LINK_EN;
    MINT32 HFG_LUMA_CPX1;
    MINT32 HFG_LUMA_CPX2;
    MINT32 HFG_LUMA_CPX3;
    MINT32 HFG_LUMA_CPY0;
    MINT32 HFG_LUMA_CPY1;
    MINT32 HFG_LUMA_CPY2;
    MINT32 HFG_LUMA_CPY3;
    MINT32 HFG_LUMA_SP0;
    MINT32 HFG_LUMA_SP1;
    MINT32 HFG_LUMA_SP2;
    MINT32 HFG_LUMA_SP3;
};

struct CCR_PARAM
{
    MINT32 CCR_ENABLE;
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
};

struct SWNR_PARAM
{
    NR_PARAM  NR;  //50
    HFG_PARAM HFG; //18
    CCR_PARAM CCR; //30
};

struct FSWNR_PARAM
{
    NR2_PARAM NR;  //10
    HFG_PARAM HFG; //18
    CCR_PARAM CCR; //30
};

typedef struct
{
    SWNR_PARAM   swnr;
    FSWNR_PARAM  fswnr;
} NVRAM_CAMERA_FEATURE_SWNR_STRUCT;

#define NVRAM_SWNR_TBL_NUM (NVRAM_ISP_REGS_ISO_SUPPORT_NUM) // 20 by iso

typedef struct _NVRAM_CAMERA_FEATURE_ABF_STRUCT
{
    // tuning parameters
    MINT32 ABF_STHRE_R;
    MINT32 ABF_STHRE_G;
    MINT32 ABF_STHRE_B;
    MINT32 ABF_NSR_R1;
    MINT32 ABF_NSR_R2;
    MINT32 ABF_R1;
    MINT32 ABF_R2;
    MINT32 ABF_Y0;
    MINT32 ABF_Y1;
    MINT32 ABF_Y2;
    MINT32 ABF_Y3;
    MINT32 ABF_CX0;
    MINT32 ABF_CX1;
    MINT32 ABF_CX2;
    MINT32 ABF_CX3;
    MINT32 ABF_CY0;
    MINT32 ABF_CY1;
    MINT32 ABF_CY2;
    MINT32 ABF_CY3;
    MINT32 ABF_CX_SP0;
    MINT32 ABF_CX_SP1;
    MINT32 ABF_CY_SP0;
    MINT32 ABF_CY_SP1;
    MINT32 ABF_Y_SP0;
    MINT32 ABF_Y_SP1;
    MINT32 ABF_CX_IDX;
    MINT32 ABF_CY_IDX;
    MINT32 ABF_BF_WIDTH;
    MINT32 ABF_BF_MAX_DIS;
    MINT32 ABF_BF_U_OFST;
    MINT32 ABF_NR_LV;
    MINT32 ABF_THRE_LO;
    MINT32 ABF_THRE_HI;
    MINT32 ABF_CHECK;
    MINT32 ABF_ON;
} NVRAM_CAMERA_FEATURE_SWABF_STRUCT;

#define NVRAM_SWABF_TBL_NUM (NVRAM_ISP_REGS_ISO_SUPPORT_NUM)


typedef struct NVRAM_CAMERA_FEATURE_STRUCT_t
{
    NVRAM_CAMERA_FEATURE_GIS_STRUCT gis;
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT mfll;
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT nr_normal[NVRAM_SWNR_TBL_NUM];
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT nr_mfll[NVRAM_SWNR_TBL_NUM];
    NVRAM_CAMERA_FEATURE_SWABF_STRUCT abf[NVRAM_SWABF_TBL_NUM];
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT nr_dual[NVRAM_SWNR_TBL_NUM];
} NVRAM_CAMERA_FEATURE_STRUCT, *PNVRAM_CAMERA_FEATURE_STRUCT;

static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_STRUCT) <= MAXIMUM_NVRAM_CAMERA_FEATURE_FILE_SIZE,
        "nvram feature size is not enough"
        );

typedef union
{
    struct
    {
        MUINT32 StereoData[MTK_STEREO_KERNEL_NVRAM_LENGTH];
        float DepthAfData[ (MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE - MTK_STEREO_KERNEL_NVRAM_LENGTH * sizeof(MUINT32) ) / sizeof(float) ];
    } StereoNvramData;

    UINT8   Data[MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE];
}NVRAM_CAMERA_GEOMETRY_STRUCT;

typedef struct NVRAM_CAMERA_FOV_STRUCT_t
{
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_FOV_FILE_SIZE];
} NVRAM_CAMERA_FOV_STRUCT, *PNVRAM_CAMERA_FOV_STRUCT;

static_assert(
        sizeof(NVRAM_CAMERA_FOV_STRUCT) <= MAXIMUM_NVRAM_CAMERA_FOV_FILE_SIZE,
        "nvram fov size is not enough"
        );

#endif // _CAMERA_CUSTOM_NVRAM_H_

