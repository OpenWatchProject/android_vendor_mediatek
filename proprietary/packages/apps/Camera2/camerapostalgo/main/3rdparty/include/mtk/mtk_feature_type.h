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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_MTK_MTK_FEATURETYPE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_MTK_MTK_FEATURETYPE_H_

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSPipelinePlugin {

enum eFeatureIndexMtk {
    NO_FEATURE_NORMAL       = 0ULL,
    // MTK (bit 0-31)
    MTK_FEATURE_MFNR        = 1ULL << 0,
    MTK_FEATURE_HDR         = 1ULL << 1,
    MTK_FEATURE_REMOSAIC    = 1ULL << 2,
    MTK_FEATURE_ABF         = 1ULL << 3,
    MTK_FEATURE_NR          = 1ULL << 4,
    MTK_FEATURE_FB          = 1ULL << 5,
    MTK_FEATURE_CZ          = 1ULL << 6,
    MTK_FEATURE_DRE         = 1ULL << 7,
    MTK_FEATURE_DEPTH       = 1ULL << 8,
    MTK_FEATURE_BOKEH       = 1ULL << 9,
    MTK_FEATURE_VSDOF       = (MTK_FEATURE_DEPTH|MTK_FEATURE_BOKEH),
    MTK_FEATURE_FSC         = 1ULL << 10,
    MTK_FEATURE_3DNR        = 1ULL << 11,
    MTK_FEATURE_EIS         = 1ULL << 12,
    MTK_FEATURE_AINR        = 1ULL << 13,
    MTK_FEATURE_DUAL_YUV    = 1ULL << 14,
    MTK_FEATURE_DUAL_HWDEPTH  = 1ULL << 15,
    MTK_FEATURE_AIS         = 1ULL << 16,
    MTK_FEATURE_HFG         = 1ULL << 17,
    MTK_FEATURE_DCE         = 1ULL << 18,
    MTK_FEATURE_AINR_YUV    = 1ULL << 19,
    MTK_FEATURE_ASYNC_ASD   = 1ULL << 20,
    MTK_FEATURE_VFB_PREVIEW        = 1ULL << 21,
    MTK_FEATURE_VFB_CAPTURE        = 1ULL << 22,
    MTK_FEATURE_AINR_FOR_HDR   = 1ULL << 23,
    MTK_FEATURE_YHDR_FOR_AINR  = 1ULL << 24,
    MTK_FEATURE_AINR_YHDR      = (MTK_FEATURE_AINR_FOR_HDR | MTK_FEATURE_YHDR_FOR_AINR),
    MTK_FEATURE_FILTER_PREVIEW = 1ULL << 25,
    MTK_FEATURE_FILTER_CAPTURE = 1ULL << 26,
    MTK_FEATURE_FILTER_MATRIX  = 1ULL << 27,
    MTK_FEATURE_AUTORAMA       = 1ULL << 28,
    MTK_FEATURE_PORTRAIT       = 1ULL << 29,

    // TODO: reserve only for MTK proprietary feature index (bit 0-31)
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPipelinePlugin
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_MTK_MTK_FEATURETYPE_H_

