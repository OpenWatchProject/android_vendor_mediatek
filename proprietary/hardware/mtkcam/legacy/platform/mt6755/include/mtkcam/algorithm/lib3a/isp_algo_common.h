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

#ifndef _ISP_ALGO_COMMON_H
#define _ISP_ALGO_COMMON_H



/*
pGamma:
    function output,
    Gamma table for GPU use,
    memory is allocated outside before calling,
    memory size is MUINT8 * 4096 (point) * 3(channel),
    order is 4096R->4096G->4096B
pDeGamma:
    function output,
    Inverse Gamma for GPU use,
    memory is allocated outside before calling,
    memory size is MINT32 * 256 (point) * 3(channel),
    order is 256R->256G->256B
*/
void getGammaInfo(ISP_NVRAM_GGM_T const & ggm, MUINT8* pGamma, MUINT32* pDeGamma);




/*
pInCCM:
    function input, ISP_CCM (100%, smooth out or dynamic out),
    memory allocated outside, size: MINT32 * (3*3)
ratio:
    function input, floating value, ratio of HW_ISP_CCM
M:
    integer precision, CCM format is Q1.M.N
N:
    decimal precision, CCM format is Q1.3.N
pOutCCM1:
    function output, for GPU CCM use, (1-ratio),
    memory allocated outside, size: MINT32 * (3*3)
pOutCCM2:
    function output, for ISP CCM use, (ratio)
    memory allocated outside, size: MINT32 * (3*3)
it will satisfy the equation: OutCCM1*OutCCM2 = InCCM //multiply ISP first, then multiply GPU
*/
MVOID separate_CCM3x3_by_ratio(MINT32* pInCCM, float ratio, MINT32 M, MINT32 N, MINT32* pOutCCM1, MINT32* pOutCCM2);

#endif
