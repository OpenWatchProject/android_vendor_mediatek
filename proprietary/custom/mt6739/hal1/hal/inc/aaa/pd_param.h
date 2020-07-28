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

#include <aaa_types.h>

#define PD_PARAM_REVISION    7245001

#define MAX_PD_PAIR_NUM 40  // max pd pair in one block

typedef struct
{
    MINT32 i4X;
    MINT32 i4Y;
    MINT32 i4W;
    MINT32 i4H;

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
    MBOOL   bIsFace;        // is FD window
    MINT32 i4SensorAGain;  // sensor analog gain of input image (1024 Base)

} PD_INPUT_T;

typedef struct
{
    MINT32 i4FocusLensPos;
    MINT32 i4ConfidenceLevel;
    MFLOAT fPdValue;

} PD_OUTPUT_T;

typedef enum
{
    ePDWIN_M0F0 = 0,
    ePDWIN_M0F1,
    ePDWIN_M1F0,
    ePDWIN_M1F1

} ePDWIN_ORIENTATION_T;



#endif


