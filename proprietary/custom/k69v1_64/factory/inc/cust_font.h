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

#ifndef FTM_CUST_FONT_H
#define FTM_CUST_FONT_H

#include "cust.h"

#if defined(FEATURE_FTM_FONT_36x64)
#define CHAR_WIDTH      36
#define CHAR_HEIGHT     64
#elif defined(FEATURE_FTM_FONT_32x60)
#define CHAR_WIDTH      32
#define CHAR_HEIGHT     60
#elif defined(FEATURE_FTM_FONT_24x44)
#define CHAR_WIDTH      24
#define CHAR_HEIGHT     44
#elif defined(FEATURE_FTM_FONT_16x30)
#define CHAR_WIDTH      16
#define CHAR_HEIGHT     30
#elif defined(FEATURE_FTM_FONT_16x28)
#define CHAR_WIDTH      16
#define CHAR_HEIGHT     28
#elif defined(FEATURE_FTM_FONT_12x22)
#define CHAR_WIDTH      12
#define CHAR_HEIGHT     22
#elif defined(FEATURE_FTM_FONT_10x18)
#define CHAR_WIDTH      10
#define CHAR_HEIGHT     18
#elif defined(FEATURE_FTM_FONT_8x14)
#define CHAR_WIDTH      8
#define CHAR_HEIGHT     14
#elif defined(FEATURE_FTM_FONT_6x10)
#define CHAR_WIDTH      6
#define CHAR_HEIGHT     10

#elif defined(FEATURE_FTM_FONT_72x72)
#define CHAR_WIDTH      72
#define CHAR_HEIGHT     72
#elif defined(FEATURE_FTM_FONT_64x64)
#define CHAR_WIDTH      64
#define CHAR_HEIGHT     64
#elif defined(FEATURE_FTM_FONT_48x48)
#define CHAR_WIDTH      48
#define CHAR_HEIGHT     48
#elif defined(FEATURE_FTM_FONT_32x32)
#define CHAR_WIDTH      32
#define CHAR_HEIGHT     32
#elif defined(FEATURE_FTM_FONT_28x28)
#define CHAR_WIDTH      28
#define CHAR_HEIGHT     28
#elif defined(FEATURE_FTM_FONT_26x26)
#define CHAR_WIDTH      26
#define CHAR_HEIGHT     26
#elif defined(FEATURE_FTM_FONT_24x24)
#define CHAR_WIDTH      24
#define CHAR_HEIGHT     24
#elif defined(FEATURE_FTM_FONT_20x20)
#define CHAR_WIDTH      20
#define CHAR_HEIGHT     20
#elif defined(FEATURE_FTM_FONT_18x18)
#define CHAR_WIDTH      18
#define CHAR_HEIGHT     18
#elif defined(FEATURE_FTM_FONT_16x16)
#define CHAR_WIDTH      16
#define CHAR_HEIGHT     16
#elif defined(FEATURE_FTM_FONT_12x12)
#define CHAR_WIDTH      12
#define CHAR_HEIGHT     12

#else
//#error "font size is not defined"
#if defined(SUPPORT_GB2312)
#define CHAR_WIDTH      26
#define CHAR_HEIGHT     26
#warning "font size is not defined, use default (26x26)"
#else
#define CHAR_WIDTH      12
#define CHAR_HEIGHT     22
#warning "font size is not defined, use default (12x22)"
#endif


#endif

#endif /* FTM_CUST_H */

