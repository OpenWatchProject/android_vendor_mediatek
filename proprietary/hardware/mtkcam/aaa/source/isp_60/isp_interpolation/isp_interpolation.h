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

#ifndef _ISP_INTERPOLATION_H
#define _ISP_INTERPOLATION_H

#include <ispfeature.h>

MVOID FaceYNR(const RAWIspCamInfo& rIspCamInfo,
                 MUINT32 FD_GGM_END_VAR,
                 ISP_NVRAM_GGM_T* GGM_D1, MUINT32 GGM_D1_END_VAR, //input
                 ISP_NVRAM_YNR_FD_T& rSmoothFD_YNR, //SW param input
                 ISP_NVRAM_YNR_SKIN_T& rYNR_SKIN, //output
                 ISP_NVRAM_YNR_LUT_T& rSmoothANR_TBL, //input & output
                 NSCam::IImageBuffer* AlphaMap_Out); //output


MVOID FD_GGM_Calculation(
                      ISP_NVRAM_GGM_T* FD_GGM,
                      MUINT32 FD_GGM_END_VAR,
                      ISP_NVRAM_GGM_T* P2_GGM,
                      MUINT32 P2_GGM_END_VAR,
                      const CAMERA_TUNING_FD_INFO_T& rFdInfoOut);

MVOID SmoothYNR_LCE_SW(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_YNR_LCE_LINK_T const& rUpperYNR_LCE_SW,   // ABF settings for upper ISO
                ISP_NVRAM_YNR_LCE_LINK_T const& rLowerYNR_LCE_SW,   // ABF settings for lower ISO
                ISP_NVRAM_YNR_LCE_LINK_T& rSmoothYNR_LCE_SW);   // Output

MVOID Calculate_YNR_LCE(MBOOL GCE_EN,  //input
                      MBOOL LCE_EN,  //input
                      ISP_NVRAM_GGM_T& FD_GGM,  //input D3
                      MUINT32 GGM_END_VAR,  //input
                      ISP_NVRAM_LCE_T const& LCE_Param, //input
                      ISP_NVRAM_YNR_LUT_T& YNR_TBL, //input & output
                      ISP_NVRAM_YNR_LCE_LINK_T const & SW_Param,  //input
                      MUINT16 rVer,  //input
                      ISP_NVRAM_YNR_LCE_OUT_T& rOutYNR_LCE);  //output

MVOID Smooth_YNR_FD(MUINT32 u4RealISO,	// Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_YNR_FD_T const& rUpperYnrFd,   // OBC settings for upper ISO
                ISP_NVRAM_YNR_FD_T const& rLowerYnrFd,   // OBC settings for lower ISO
                ISP_NVRAM_YNR_FD_T& rYnrFd);


MVOID FD_YNR_TBL(CAMERA_TUNING_FD_INFO_T const& rFdInfo,
               ISP_NVRAM_YNR_LUT_T& rSmoothANR_TBL,
               ISP_NVRAM_YNR_FD_T& rFD_YNR);

MVOID YNR_ALPHA_MAP(
               CAMERA_TUNING_FD_INFO_T const& rFdInfo,
               const RAWIspCamInfo& rIspCamInfo,
               ISP_NVRAM_YNR_FD_T& rFD_YNR,
               NSCam::IImageBuffer* alphamap, //in & out
               ISP_NVRAM_YNR_SKIN_T& rynr_skin);//out

MVOID SmoothAINR_SW(MUINT32 u4RealISO,  // Real ISO
                    MUINT32 u4UpperISO, // Upper ISO
                    MUINT32 u4LowerISO, // Lower ISO
                    FEATURE_NVRAM_AINR_T const& rUpperAINR_SW,   // AINR SW reg settings for upper ISO
                    FEATURE_NVRAM_AINR_T const& rLowerAINR_SW,   // AINR SW reg settings for lower ISO
                    FEATURE_NVRAM_AINR_T & rSmoothAINR_SW);

MVOID Smooth_OBC(MUINT32 u4RealISO,	// Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_OBC_T const& rUpperOBC,   // OBC settings for upper ISO
                ISP_NVRAM_OBC_T const& rLowerOBC,   // OBC settings for lower ISO
                ISP_NVRAM_OBC_T& rSmoothOBC);

MVOID Smooth_BPC_BPC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BPC_BPC_T const& rUpperBPC_BPC,   // BPC_BPC settings for upper ISO
                ISP_NVRAM_BPC_BPC_T const& rLowerBPC_BPC,   // BPC_BPC settings for lower ISO
                ISP_NVRAM_BPC_BPC_T& rSmoothBPC_BPC);


MVOID Smooth_BPC_CT(MUINT32 u4RealISO,	// Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BPC_CT_T const& rUpperBPC_CT,   // BPC_CT settings for upper ISO
                ISP_NVRAM_BPC_CT_T const& rLowerBPC_CT,   // BPC_CT settings for lower ISO
                ISP_NVRAM_BPC_CT_T& rSmoothBPC_CT);

MVOID Smooth_BPC_PDC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BPC_PDC_T const& rUpperBPC_PDC,	// BPC_PDC settings for upper ISO
                ISP_NVRAM_BPC_PDC_T const& rLowerBPC_PDC,	// BPC_PDC settings for lower ISO
                ISP_NVRAM_BPC_PDC_T& rSmoothBPC_PDC);

MVOID Smooth_LDNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_LDNR_T const& rUpperLDNR,   // LDNR settings for upper ISO
                ISP_NVRAM_LDNR_T const& rLowerLDNR,   // LDNR settings for lower ISO
                ISP_NVRAM_LDNR_T& rSmoothLDNR);

MVOID Smooth_DM(MUINT32 u4RealISO,	// Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_DM_T const& rUpperDM,   // DM settings for upper ISO
                ISP_NVRAM_DM_T const& rLowerDM,   // DM settings for lower ISO
                ISP_NVRAM_DM_T& rSmoothDM);

MVOID Smooth_YNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_YNR_T const& rUpperYNR,   // YNR settings for upper ISO
                ISP_NVRAM_YNR_T const& rLowerYNR,   // YNR settings for lower ISO
                ISP_NVRAM_YNR_T& rSmoothYNR);

MVOID Smooth_CNR_ABF(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_CNR_ABF_T const& rUpperCNR_ABF,	// CNR_ABF settings for upper ISO
                ISP_NVRAM_CNR_ABF_T const& rLowerCNR_ABF,	// CNR_ABF settings for lower ISO
                ISP_NVRAM_CNR_ABF_T& rSmoothCNR_ABF);

MVOID Smooth_CNR_CCR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_CNR_CCR_T const& rUpperCNR_CCR,   // CNR_CCR settings for upper ISO
                ISP_NVRAM_CNR_CCR_T const& rLowerCNR_CCR,   // CNR_CCR settings for lower ISO
                ISP_NVRAM_CNR_CCR_T& rSmoothCNR_CCR);

MVOID Smooth_CNR_CNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_CNR_CNR_T const& rUpperCNR_CNR,	// CNR_CNR settings for upper ISO
                ISP_NVRAM_CNR_CNR_T const& rLowerCNR_CNR,	// CNR_CNR settings for lower ISO
                ISP_NVRAM_CNR_CNR_T& rSmoothCNR_CNR);

MVOID Smooth_EE(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_EE_T const& rUpperEE,   // EE settings for upper ISO
                ISP_NVRAM_EE_T const& rLowerEE,   // EE settings for lower ISO
                ISP_NVRAM_EE_T& rSmoothEE);

MVOID Smooth_NR3D(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_NR3D_T const& rUpperNR3D,   // NR3D settings for upper ISO
                ISP_NVRAM_NR3D_T const& rLowerNR3D,   // NR3D settings for lower ISO
                ISP_NVRAM_NR3D_T& rSmoothNR3D);

MVOID Smooth_LPCNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_LPCNR_T const& rUpperLPCNR,   // LPCNR settings for upper ISO
                ISP_NVRAM_LPCNR_T const& rLowerLPCNR,   // LPCNR settings for lower ISO
                ISP_NVRAM_LPCNR_T& rSmoothLPCNR);

MVOID Smooth_MIX(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_MIX_T const& rUpperMIX,	// MIX settings for upper ISO
                ISP_NVRAM_MIX_T const& rLowerMIX,	// MIX settings for lower ISO
                ISP_NVRAM_MIX_T& rSmoothMIX);

MVOID Smooth_MFB(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_MFB_T const& rUpperMFB,   // MFB settings for upper ISO
                ISP_NVRAM_MFB_T const& rLowerMFB,   // MFB settings for lower ISO
                ISP_NVRAM_MFB_T& rSmoothMFB);

MVOID Smooth_FUS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_FUS_T const& rUpperFUS,	// FUS settings for upper ISO
                ISP_NVRAM_FUS_T const& rLowerFUS,	// FUS settings for lower ISO
                ISP_NVRAM_FUS_T& rSmoothFUS);

MVOID Smooth_ZFUS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_ZFUS_T const& rUpperZFUS,   // ZFUS settings for upper ISO
                ISP_NVRAM_ZFUS_T const& rLowerZFUS,   // ZFUS settings for lower ISO
                ISP_NVRAM_ZFUS_T& rSmoothZFUS);

MVOID Smooth_YNRS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_YNRS_T const& rUpperYNRS,   // YNRS settings for upper ISO
                ISP_NVRAM_YNRS_T const& rLowerYNRS,   // YNRS settings for lower ISO
                ISP_NVRAM_YNRS_T& rSmoothYNRS);

MVOID Smooth_YNR_TBL(MINT32 u4ReaISO,  // Rea ISO
               MINT32 u4UpperISO, // Upper ISO
               MINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_YNR_LUT_T const& rUpperYNR_TBL, // YNR TBL for upper ISO
               ISP_NVRAM_YNR_LUT_T const& rLowerYNR_TBL,   // YNR TBL for lower ISO
               ISP_NVRAM_YNR_LUT_T& rSmoothYNR_TBL);

MVOID SmoothFSWNR(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rUpper, // settings for upper ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rLower,	// settings for lower ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT& rSmooth);

MVOID SmoothCOLOR(  MBOOL IsCapture,
                    MUINT32 u4UpperCT, // Upper CT
                    MUINT32 u4LowerCT, // Lower CT
                    MINT32 i4UpperLV,
                    MINT32 i4LowerLV,
                    ColorInfo& rColorInfo, // New structure, including CCT, RealLV & AWB gain
                    CAMERA_TUNING_FD_INFO_T const& rFdInfo,
                    ISP_NVRAM_SCOLOR_PARAM_T const (& rNVRAM_COLORS_LV_Lo_CT_Lo),// NVRAM Color Table
                    ISP_NVRAM_SCOLOR_PARAM_T const (& rNVRAM_COLORS_LV_Lo_CT_Up),// NVRAM Color Table
                    ISP_NVRAM_SCOLOR_PARAM_T const (& rNVRAM_COLORS_LV_Up_CT_Lo),// NVRAM Color Table
                    ISP_NVRAM_SCOLOR_PARAM_T const (& rNVRAM_COLORS_LV_Up_CT_Up),// NVRAM Color Table
                    ISP_NVRAM_COLOR_PARAM_T const& Para_Color,// Color parameters
                    ISP_NVRAM_COLOR_T& rSmoothCOLOR_TBL,// SmoothCOLOR output
                    MUINT16& rVer,
                    MBOOL& rFirstTimeBoot,
                    ISP_NVRAM_COLOR_T& rSmoothColor_Prv
                    );


#endif
