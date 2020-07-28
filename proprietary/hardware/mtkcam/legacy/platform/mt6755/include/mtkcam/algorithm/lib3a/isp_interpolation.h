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



MVOID SmoothNR1(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_NR1_T const& rUpperNR1, // NR1 settings for upper ISO
                ISP_NVRAM_NR1_T const& rLowerNR1,   // NR1 settings for lower ISO
                ISP_NVRAM_NR1_T& rSmoothNR1);   // Output


MVOID SmoothBPC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BPC_T const& rUpperBPC, // NR1 settings for upper ISO
                ISP_NVRAM_BPC_T const& rLowerBPC,   // NR1 settings for lower ISO
                ISP_NVRAM_BPC_T& rSmoothBPC);   // Output


MVOID SmoothDBS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_DBS_T const& rUpperDBS, // DBS settings for upper ISO
                ISP_NVRAM_DBS_T const& rLowerDBS,   // DBS settings for lower ISO
                ISP_NVRAM_DBS_T& rSmoothDBS);  // Output

MVOID SmoothUDM(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_UDM_T const& rUpperUDM, // DBS settings for upper ISO
                ISP_NVRAM_UDM_T const& rLowerUDM,   // DBS settings for lower ISO
                ISP_NVRAM_UDM_T& rSmoothUDM);  // Output

MVOID SmoothEE(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_EE_T const& rUpperEE, // EE settings for upper ISO
               ISP_NVRAM_EE_T const& rLowerEE,   // EE settings for lower ISO
               ISP_NVRAM_EE_T& rSmoothEE);  // Output

MVOID SmoothANR(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_ANR_T const& rUpperANR, // ANR settings for upper ISO
               ISP_NVRAM_ANR_T const& rLowerANR,   // ANR settings for lower ISO
               ISP_NVRAM_ANR_T& rSmoothANR);  // Output

MVOID SmoothANR_TBL(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_ANR_LUT_T const& rUpperANR_TBL, // ANR1 TBL for upper ISO
               ISP_NVRAM_ANR_LUT_T const& rLowerANR_TBL,   // ANR1 TBL for lower ISO
               ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL);  // Output

MVOID SmoothANR2(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_ANR2_T const& rUpperANR, // ANR settings for upper ISO
                ISP_NVRAM_ANR2_T const& rLowerANR,   // ANR settings for lower ISO
                ISP_NVRAM_ANR2_T& rSmoothANR);  // Output

MVOID SmoothCCR(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_CCR_T const& rUpperCCR, // ANR settings for upper ISO
               ISP_NVRAM_CCR_T const& rLowerCCR,   // ANR settings for lower ISO
               ISP_NVRAM_CCR_T& rSmoothCCR);  // Output

MVOID SmoothPCA_TBL(MUINT32 u4RealCT,  // Real CT
               MUINT32 u4UpperCT, // Upper CT
               MUINT32 u4LowerCT, // Lower CT
               ISP_NVRAM_PCA_LUT_T const& rUpperPCA_TBL,
               ISP_NVRAM_PCA_LUT_T const& rLowerPCA_TBL,
               ISP_NVRAM_PCA_LUT_T& rSmoothPCA_TBL,
               MUINT32 u4RealLV,
               ISP_CUSTOM_PARA_PCA_T* Para_PCA);

MVOID SmoothSWNR(MUINT32 u4RealISO, // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rUpperSWNR, // SWNR settings for upper ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rLowerSWNR,   // SWNR settings for lower ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT& rSmoothSWNR);  // Output
#endif
