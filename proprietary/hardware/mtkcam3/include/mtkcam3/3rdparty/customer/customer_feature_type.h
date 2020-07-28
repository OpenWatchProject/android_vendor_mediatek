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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_CUSTOMER_CUSTOMER_FEATURETYPE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_CUSTOMER_CUSTOMER_FEATURETYPE_H_

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSPipelinePlugin {

enum eFeatureIndexCustomer {
    // ThirdParty (bit 32-63)
    TP_FEATURE_HDR          = 1ULL << 32,
    TP_FEATURE_MFNR         = 1ULL << 33,
    TP_FEATURE_EIS          = 1ULL << 34,
    TP_FEATURE_FB           = 1ULL << 35,
    TP_FEATURE_FILTER       = 1ULL << 36,
    TP_FEATURE_DEPTH        = 1ULL << 37,
    TP_FEATURE_BOKEH        = 1ULL << 38,
    TP_FEATURE_VSDOF        = (TP_FEATURE_DEPTH|TP_FEATURE_BOKEH),
    TP_FEATURE_FUSION       = 1ULL << 39,
    TP_FEATURE_HDR_DC       = 1ULL << 40,   // used by DualCam
    TP_FEATURE_DUAL_YUV     = 1ULL << 41,
    TP_FEATURE_DUAL_HWDEPTH = 1ULL << 42,
    TP_FEATURE_PUREBOKEH    = 1ULL << 43,
    TP_FEATURE_RAW_HDR      = 1ULL << 44,
    TP_FEATURE_RELIGHTING   = 1ULL << 45,
    TP_FEATURE_ASYNC_ASD    = 1ULL << 46,
    // TODO: reserve for customer feature index (bit 32-63)
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPipelinePlugin
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_CUSTOMER_CUSTOMER_FEATURETYPE_H_

