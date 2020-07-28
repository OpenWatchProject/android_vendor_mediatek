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

#ifndef _ISPFEATURE_H
#define _ISPFEATURE_H

// CCM
typedef struct
{
    MINT32 M11[8][8];
    MINT32 M12[8][8];
    MINT32 M13[8][8];
    MINT32 M21[8][8];
    MINT32 M22[8][8];
    MINT32 M23[8][8];
    MINT32 M31[8][8];
    MINT32 M32[8][8];
    MINT32 M33[8][8];
} ISP_CCM_T_2D;

typedef struct
{
    MINT32 M11;
    MINT32 M12;
    MINT32 M13;
    MINT32 M21;
    MINT32 M22;
    MINT32 M23;
    MINT32 M31;
    MINT32 M32;
    MINT32 M33;
} ISP_CCM_T;

typedef struct
{
    MINT32 i4CT0;   // Strobe
    MINT32 i4CT1;        // A
    MINT32 i4CT2;     // TL84
    MINT32 i4CT3;      // CWF
    MINT32 i4CT4;      // D65
    MINT32 i4CT5;     // RSV1
    MINT32 i4CT6;     // RSV2
    MINT32 i4CT7;     // RSV3
} ISP_CCM_WEIGHT_T;

MVOID SmoothCCM(MBOOL useFullGain,
               MINT32 M,  // Merlin: integer bitwidth
               MINT32 N,  // Merlin: decimal bitwidth
               ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM,
               ISP_NVRAM_CCM_T& rCCMOutput,
               MUINT16& rVer,
               RAWIspCamInfo const& rCamInfo,
               ISP_NVRAM_CCM_T* pSmoothCCM_Record,
               MUINT32& rSmoothCCM_Counter,
               MUINT32* pSmoothCCM_Coef);


//************************************************************
// Smooth COLOR/Smooth PCA
//************************************************************
MVOID SmoothPCA_TBL(MBOOL IsCapture,
                          MUINT32 u4UpperCT, // Upper CT
                          MUINT32 u4LowerCT, // Lower CT
                          EIndex_PCA_LUT_CT_T eUpperIdx,
                          EIndex_PCA_LUT_CT_T eLowerIdx,
                          ISP_NVRAM_PCA_LUT_T const(& rNVRAM_PCA_LUTS)[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM],
                          ISP_NVRAM_PCA_PARAM_T const(& Para_PCA)[ISP_NVRAM_COLOR_SCENE_NUM],
                          RAWIspCamInfo const& rCamInfo,
                          ISP_NVRAM_PCA_LUT_T& rSmoothPCA_TBL,
                          MUINT16& rVer);

MVOID SmoothCOLOR(MBOOL IsCapture,
                          MUINT32 u4UpperCT, // Upper CT
                          MUINT32 u4LowerCT, // Lower CT
                          EIndex_PCA_LUT_CT_T eUpperIdx,
                          EIndex_PCA_LUT_CT_T eLowerIdx,
                          ISP_NVRAM_COLOR_T const(& rNVRAM_COLORS)[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM],
                          ISP_NVRAM_PCA_PARAM_T const(& Para_PCA)[ISP_NVRAM_COLOR_SCENE_NUM],
                          RAWIspCamInfo const& rCamInfo,
                          ISP_NVRAM_COLOR_T& rSmoothCOLOR_TBL,
                          MUINT16& rVer,
                          MBOOL& rFirstTimeBoot,
                          ISP_NVRAM_COLOR_T& rSmoothColor_Prv);

#endif
