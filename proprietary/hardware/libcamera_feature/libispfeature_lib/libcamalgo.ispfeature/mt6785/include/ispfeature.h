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
    MINT32 M11[2][10];
    MINT32 M12[2][10];
    MINT32 M13[2][10];
    MINT32 M21[2][10];
    MINT32 M22[2][10];
    MINT32 M23[2][10];
    MINT32 M31[2][10];
    MINT32 M32[2][10];
    MINT32 M33[2][10];
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
    MINT16 RealLV;//i4RealLightValue_x10
    MINT16 AWB_NoPrefGain[3];//a_rAWBOutput.rAWBSyncInput_N3D.rAwbGainNoPref
    MINT16 NoPrefCCT;//a_rAWBOutput.rAWBInfo.i4CCT
} ColorInfo;

MVOID SmoothCCM(MINT32 M,  // integer bitwidth
                MINT32 N,  // decimal bitwidth
                ISP_NVRAM_CCM_T& rCCMOutput,
                MUINT16& rVer,
			    MINT16 UpperLV,
			    MINT16 LowerLV,
			    ISP_NVRAM_MULTI_CCM_STRUCT& rCCMUpper,
			    ISP_NVRAM_MULTI_CCM_STRUCT& rCCMLower,
                ColorInfo const& rColorInfo,
                ISP_NVRAM_CCM_T* pSmoothCCM_Record,
                MUINT32& rSmoothCCM_Counter);


			   
#endif
