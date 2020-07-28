/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

#ifndef _PD_PARAM_H
#define _PD_PARAM_H


#define PD_PARAM_REVISION   8111001
#include <aaa_types.h>
#define MAX_PD_PAIR_NUM 16  // max pd pair in one block

typedef enum
{
    EPDSensorDevId_0   = 0x0,  //ESensorDev_None
    EPDSensorDevId_1   = 0x1,  //ESensorDev_Main
    EPDSensorDevId_2   = 0x2,  //ESensorDev_Sub
    EPDSensorDevId_3   = 0x3,
    EPDSensorDevId_4   = 0x4,  //ESensorDev_MainSecond
    EPDSensorDevId_5   = 0x5,  //ESensorDev_Main3D
    EPDSensorDevId_6   = 0x6,
    EPDSensorDevId_7   = 0x7,
    EPDSensorDevId_8   = 0x8,  //ESensorDev_SubSecond
    EPDSensorDevId_9   = 0x9,
    EPDSensorDevId_10   = 0xA,
    EPDSensorDevId_11   = 0xB,
    EPDSensorDevId_12   = 0xC,
    EPDSensorDevId_13   = 0xD,
    EPDSensorDevId_14   = 0xE,
    EPDSensorDevId_15   = 0xF,
    EPDSensorDevId_16   = 0x10, //ESensorDev_MainThird
    EPDSensorDevId_17   = 0x11,
    EPDSensorDevId_18   = 0x12,
    EPDSensorDevId_19   = 0x13,
    EPDSensorDevId_20   = 0x14,
    EPDSensorDevId_Max  = 0x21
} EPDSensorDevId_T;


// This declaration should be replaced (removed) in ISP 6.0.
typedef enum
{
    eAF_ROI_TYPE_NONE = 0,
    eAF_ROI_TYPE_AP,
    eAF_ROI_TYPE_OT,
    eAF_ROI_TYPE_FD,
    eAF_ROI_TYPE_CENTER,
    eAF_ROI_TYPE_CUS1,
    eAF_ROI_TYPE_CUS2,
    eAF_ROI_TYPE_CUS3,
    eAF_ROI_TYPE_CUS4,
    eAF_ROI_TYPE_DEFAULT,
} eAF_ROI_TYPE_T;

typedef struct
{
    MINT32 i4X;
    MINT32 i4Y;
    MINT32 i4W;
    MINT32 i4H;
    eAF_ROI_TYPE_T sAFType;
} PD_AREA_T;

typedef struct
{
    MINT32 i4OffsetX;   // start offset of first PD block
    MINT32 i4OffsetY;
    MINT32 i4PitchX;    // PD block pitch
    MINT32 i4PitchY;
    MINT32 i4BlockNumX;    // total PD block number in x direction
    MINT32 i4BlockNumY;     // total PD block number in y direction
    MINT32 i4PosL[MAX_PD_PAIR_NUM][2];  // left pd pixel position in one block
    MINT32 i4PosR[MAX_PD_PAIR_NUM][2];  // right pd pixel position in one block
    MINT32 i4PairNum;   // PD pair num in one block
    MINT32 i4SubBlkW;   // sub block width (one pd pair in one sub block)
    MINT32 i4SubBlkH;   // sub block height

} PD_BLOCK_INFO_T;

typedef struct
{
     MINT32 i4BufStride;       // PD buffer size
     MINT32 i4BufHeight;
     MINT32 i4Bits;
     MINT32 i4BinningX;     // L/R pixel binning mode
     MINT32 i4BinningY;
     MINT32 i4BufFmt;     // L/R arrangement in buffer
} DUAL_PD_FORMAT_T;

typedef struct
{
    MINT32 i4RawWidth;
    MINT32 i4RawHeight;
    MINT32 i4RawStride;
    MINT32 i4Bits;
    MINT32 i4IsPacked;
    PD_BLOCK_INFO_T sPdBlockInfo;
    MINT32 i4FullRawWidth; // sensor full size
    MINT32 i4FullRawHeight;
    MINT32 i4FullRawXOffset; // X crop
    MINT32 i4FullRawYOffset; // Y crop
    DUAL_PD_FORMAT_T sDPdFormat;
    MINT32 i4DRVBlockInfoUseEn;  // 0 (default): use calibration data, 1: use driver setting
    MINT32 i4SensorType; // 0: default; 4: four cells, binning
} PD_CONFIG_T;

typedef struct
{
    PD_NVRAM_T rPDNVRAM;         // PD NVRAM param
} PD_INIT_T;

typedef struct
{
    MVOID *pRawBuf;             // raw data buffer
    MVOID *pPDBuf;              // for virtual channel
    PD_AREA_T sFocusWin;           // focus window refer to raw image coordinate

} PD_EXTRACT_INPUT_T;

typedef struct
{
    PD_AREA_T sPdWin;
    MUINT16 *pPDLData;
    MUINT16 *pPDRData;
    MUINT16 *pPDLPos;
    MUINT16 *pPDRPos;
    PD_AREA_T sPdBlk;   // refined focus PD window (refer to PD image coordinate)

} PD_EXTRACT_DATA_T;

typedef struct
{
    PD_EXTRACT_DATA_T sPDExtractData; // extracted PD data from IPdAlgo::extractPD()
    MINT32 i4CurLensPos;   // current lens position
    MINT32 i4MinLensPos;   // minimum lens position (for dynamic search range)
    MINT32 i4MaxLensPos;   // maximum lens position (for dynamic search range )
    MBOOL   bIsFace;        // is FD window
    MINT32 i4SensorAGain;  // sensor analog gain of input image (1024 Base)
    MINT32 i4ISO;  // ISO speed
} PD_INPUT_T;

typedef struct
{
    MINT32 i4FocusLensPos;
    MINT32 i4ConfidenceLevel;
    MFLOAT fPdValue;
    MINT32 i4SatPercent;
    MINT32 i4ConfidenceExt;
    MINT32 i4AlgoType;
} PD_OUTPUT_T;

typedef enum
{
    ePDWIN_M0F0 = 0,
    ePDWIN_M0F1 = 2,
    ePDWIN_M1F0 = 1,
    ePDWIN_M1F1 = 3

} ePDWIN_ORIENTATION_T;
/* 0:IMAGE_NORMAL,1:IMAGE_H_MIRROR,2:IMAGE_V_MIRROR,3:IMAGE_HV_MIRROR*/

typedef struct
{
    MUINT32 u4MajorVersion;
    MUINT32 u4MinorVersion;
} PD_LIB_VERSION_T;

typedef enum
{
    ePDSENS_NORMAL = 0,
    ePDSENS_1BY2_2BY1 = 1,
    ePDSENS_2BY2 = 2,
    ePDSENS_4CELL_FULL = 3,
    ePDSENS_4CELL_BIN = 4,
    ePDSENS_NA = 99,
} ePDSENS_TYPE_T;

typedef enum
{
    ePDALGO_CVPD_CPU = 0,
    ePDALGO_CVPD_VPU = 1,
    ePDALGO_AIPD_MDLA = 2,
    ePDALGO_AIPD_VPU = 3,
    ePDALGO_AIPD_TFLITE = 4,
    ePDALGO_NA = 99,
} ePDALGO_TYPE_T;

#endif


