/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_MFLL_H_
#define _CAMERA_CUSTOM_MFLL_H_

#include "camera_custom_types.h"	// For MUINT*/MINT*/MVOID/MBOOL type definitions.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
// For MFLL Customer Parameters

// [Best Shot Selection (BSS)]
//     - When CUST_MFLL_ENABLE_BSS_FOR_MFLL==0,
//          Use the first image as base image for blending.
//          The advantage is shutter lag has been minimized.
//          The drawback is final image has higher motion blur (cause by handshack)possibility .
//     - When CUST_MFLL_ENABLE_BSS_FOR_MFLL==1 (recommended),
//          Use the image with the highest sharpness as base image.
//          The advantage is the motion blur (cause by handshack) has been minimized.
//          The drawback is a longer shutter lag (average ~+100ms)
#define CUST_MFLL_ENABLE_BSS_FOR_MFLL	1

//     - When CUST_MFLL_ENABLE_BSS_FOR_AIS==0,
//          Use the first image as base image for blending.
//          The advantage is shutter lag has been minimized.
//          The drawback is final image has higher motion blur (cause by handshack)possibility .
//     - When CUST_MFLL_ENABLE_BSS_FOR_AIS==1 (recommended),
//          Use the image with the highest sharpness as base image.
//          The advantage is the motion blur (cause by handshack) has been minimized.
//          The drawback is a longer shutter lag (average ~+100ms)
#define CUST_MFLL_ENABLE_BSS_FOR_AIS	1

//     - how many rows are skipped during processing 
//          recommand range: >=8
//          recommand value: 8
//          larger scale factor cause less accurate but faster execution time.
#define CUST_MFLL_BSS_SCALE_FACTOR      8

//     - minimum edge response
//          recommand range: 10~30
//          recommand value: 20
//          larger th0 cause better noise resistence but may miss real edges.
#define CUST_MFLL_BSS_CLIP_TH0          20

//     - maximum edge response
//          recommand range: 50~120
//          recommand value: 100
//          larger th1 will suppress less high contrast edges
#define CUST_MFLL_BSS_CLIP_TH1          100

//     - tri-pod/static scene detection
//          recommand range: 0~10
//          recommand value: 10
//          larger zero cause more scene will be considered as static
#define CUST_MFLL_BSS_ZERO              10

// [EIS]
//     - CUST_MFLL_EIS_TRUST_TH,
//          The threashold of the trust value of EIS statistic.
//          If the trust value of EIS statistic lower than this threshold, the GMV calculation process willn't include this block.
//          Use higher CUST_MFLL_EIS_TRUST_TH will reduce the possibility of artifact, but increase the possibility of single frame output too.
//          p.s. single frame outout - when the motion between frames are too large to be accepted, MFLL/AIS willn't take effect for avoiding artifact.
#define CUST_MFLL_EIS_TRUST_TH    100
//     - CUST_MFLL_EIS_OP_STEP_H,
//          reserved only, never modify this
#define CUST_MFLL_EIS_OP_STEP_H   1
//     - CUST_MFLL_EIS_OP_STEP_V,
//          reserved only, never modify this
#define CUST_MFLL_EIS_OP_STEP_V   1



#endif	// _CAMERA_CUSTOM_MFLL_H_

