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

MVOID SmoothABF(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_ABF_T const& rUpperABF,   // ABF settings for upper ISO
                ISP_NVRAM_ABF_T const& rLowerABF,   // ABF settings for lower ISO
                ISP_NVRAM_ABF_T& rSmoothABF);   // Output

MVOID SmoothADBS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_ADBS_T const& rUpperADBS,   // ADBS settings for upper ISO
                ISP_NVRAM_ADBS_T const& rLowerADBS,   // ADBS settings for lower ISO
                ISP_NVRAM_ADBS_T& rSmoothADBS);   // Output


MVOID SmoothDBS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_DBS_T const& rUpperDBS, // DBS settings for upper ISO
                ISP_NVRAM_DBS_T const& rLowerDBS,   // DBS settings for lower ISO
                ISP_NVRAM_DBS_T& rSmoothDBS);  // Output

MVOID SmoothOBC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_OBC_T const& rUpperOBC, // OBV settings for upper ISO
                ISP_NVRAM_OBC_T const& rLowerOBC,   // OBV settings for lower ISO
                ISP_NVRAM_OBC_T& rSmoothOBC);   // Output

MVOID SmoothBPC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_BPC_T const& rUpperBPC, // BPC settings for upper ISO
                ISP_NVRAM_BNR_BPC_T const& rLowerBPC,   // BPC settings for lower ISO
                ISP_NVRAM_BNR_BPC_T& rSmoothBPC);   // Output

MVOID SmoothNR1(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_NR1_T const& rUpperNR1, // NR1 settings for upper ISO
                ISP_NVRAM_BNR_NR1_T const& rLowerNR1,   // NR1 settings for lower ISO
                ISP_NVRAM_BNR_NR1_T& rSmoothNR1);   // Output

MVOID SmoothPDC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_PDC_T const& rUpperPDC, // PDC settings for upper ISO
                ISP_NVRAM_BNR_PDC_T const& rLowerPDC,   // PDC settings for lower ISO
                ISP_NVRAM_BNR_PDC_T& rSmoothPDC);   // Output

MVOID SmoothRMM(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_RMM_T const& rUpperRMM, // RMM settings for upper ISO
                ISP_NVRAM_RMM_T const& rLowerRMM,   // RMM settings for lower ISO
                ISP_NVRAM_RMM_T& rSmoothRMM);   // Output

MVOID SmoothRNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_RNR_T const& rUpperRNR, // RNR settings for upper ISO
                ISP_NVRAM_RNR_T const& rLowerRNR,   // RNR settings for lower ISO
                ISP_NVRAM_RNR_T& rSmoothRNR);   // Output

MVOID SmoothUDM(ISP_NVRAM_UDM_INT_T const& rParam,   // UDM settings
                ISP_NVRAM_UDM_T& rSmoothUDM);  // Output

MVOID SmoothLCE(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_LCE_T const& rUpperLCE, // LCE settings for upper ISO
                ISP_NVRAM_LCE_T const& rLowerLCE,   // LCE settings for lower ISO
                ISP_NVRAM_LCE_T& rSmoothLCE);  // Output

MVOID SmoothEE(ISP_NVRAM_EE_INT_T const& rParam,   // EE settings
               ISP_NVRAM_EE_T& rSmoothEE);  // Output

MVOID SmoothANR(ISP_NVRAM_ANR_INT_T const& rParam,   // ANR settings
               ISP_NVRAM_ANR_T& rSmoothANR);  // Output

MVOID SmoothANR2(ISP_NVRAM_ANR2_INT_T const& rParam,   // ANR2 settings
                ISP_NVRAM_ANR2_T& rSmoothANR2);  // Output

#if 0
MVOID SmoothANR_TBL(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_ANR_LUT_T const& rUpperANR_TBL, // ANR1 TBL for upper ISO
               ISP_NVRAM_ANR_LUT_T const& rLowerANR_TBL,   // ANR1 TBL for lower ISO
               ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL);  // Output
#endif

MVOID SmoothANR_TBL(MINT32 u4RealLV,  // Real LV
               MINT32 u4UpperLV, // Upper LV
               MINT32 u4LowerLV, // Lower LV
               ISP_NVRAM_FD_ANR_T const (& rUpperFD_ANR),   // NR1 settings for upper LV
               ISP_NVRAM_FD_ANR_T const (& rLowerFD_ANR),   // NR1 settings for lower LV
               CAMERA_TUNING_FD_INFO_T const& rFdInfo,
               ISP_NVRAM_ANR_LUT_T const& rUpperANR_TBL, // ANR1 TBL for upper LV
               ISP_NVRAM_ANR_LUT_T const& rLowerANR_TBL,   // ANR1 TBL for lower LV
               ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL);  // Output

MVOID SmoothCCR(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_CCR_T const& rUpperCCR, // CCR settings for upper ISO
               ISP_NVRAM_CCR_T const& rLowerCCR,   // CCR settings for lower ISO
               ISP_NVRAM_CCR_T& rSmoothCCR);  // Output

MVOID SmoothHFG(ISP_NVRAM_HFG_INT_T const& rParam,   // HFG settings
                ISP_NVRAM_HFG_T& rSmoothHFG);   // Output

MVOID SmoothMIXER3(ISP_NVRAM_MIXER3_INT_T const& rParam,   //MIXER3 settings
                ISP_NVRAM_MIXER3_T& rSmoothMIXER3);   // Output

MVOID SmoothMFB(ISP_NVRAM_MFB_INT_T const& rParam,   //MFB settings
                ISP_NVRAM_MFB_T& rSmoothMFB);   // Output

MVOID SmoothNR3D(ISP_NVRAM_NR3D_INT_T const& rParam,   // EE settings
               ISP_NVRAM_NR3D_T& rSmoothNR3D,
               ISP_NR3D_SMOOTH_INFO_T const& rNR3D_SW_Params);  // Output

MVOID SmoothFSWNR(MUINT32 u4RealISO, // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rUpper, // settings for upper ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rLower,   // settings for lower ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT& rSmooth);  // Output

MVOID SmoothNBC_LTM_SW(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NBC_LTM_SW_PARAM_T const& rUpperNBC_LTM_SW,   // ABF settings for upper ISO
                ISP_NBC_LTM_SW_PARAM_T const& rLowerNBC_LTM_SW,   // ABF settings for lower ISO
                ISP_NBC_LTM_SW_PARAM_T& rSmoothNBC_LTM_SW);   // Output



MVOID Calculate_NBC_LTM( MBOOL GCE_EN, MBOOL LCE_EN,
                      ISP_NVRAM_GGM_LUT_T const& GGM,
                      MUINT32 GGM_END_VAR,
                      ISP_NVRAM_LCE_T const& LCE_Param,
                      ISP_NVRAM_ANR_LUT_T& ANR_TBL,
                      ISP_NBC_LTM_SW_PARAM_T const & SW_Param,
                      MUINT16& rVer,
                      ISP_NVRAM_ANR_LTM_T& rOutANR_LTM);

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
