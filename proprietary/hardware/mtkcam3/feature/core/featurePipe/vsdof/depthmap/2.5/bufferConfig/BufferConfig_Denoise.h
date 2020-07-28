/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/**
 * @file BufferConfig.h
 * @brief buffer config header
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_VSDOF_BUFFERCONFIG_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_VSDOF_BUFFERCONFIG_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "BaseBufferConfig.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

/*******************************************************************************
* Config Define
********************************************************************************/

/**
 * @brief Define the input/output type of each bufferID inside the EffectRequest
          for VSDOF scearios
 */
const NodeBufferSetting preview_buffer_config[] =
{
    // each setting ends by this
    {BID_INVALID, eBUFFER_IOTYPE_INPUT}
};

const NodeBufferSetting record_buffer_config[] =
{
    // each setting ends by this
    {BID_INVALID, eBUFFER_IOTYPE_INPUT}
};

const NodeBufferSetting capture_buffer_config[] =
{
    // input
    {BID_P2A_IN_FSRAW1, eBUFFER_IOTYPE_INPUT},
    {BID_P2A_IN_FSRAW2, eBUFFER_IOTYPE_INPUT},
    {BID_P2A_IN_RSRAW1, eBUFFER_IOTYPE_INPUT},
    {BID_P2A_IN_RSRAW2, eBUFFER_IOTYPE_INPUT},
    // output
    {BID_N3D_OUT_WARPING_MATRIX, eBUFFER_IOTYPE_OUTPUT},
    {BID_DPE_OUT_DMP_L, eBUFFER_IOTYPE_OUTPUT},
    {BID_DPE_OUT_DMP_R, eBUFFER_IOTYPE_OUTPUT},
    {BID_FD_OUT_EXTRADATA, eBUFFER_IOTYPE_OUTPUT},
    {BID_N3D_OUT_SCENE_INFO, eBUFFER_IOTYPE_OUTPUT},
    // each setting ends by this
    {BID_INVALID, eBUFFER_IOTYPE_INPUT}
};

const NodeBufferSetting metadata_config[] =
{
    {BID_META_IN_APP, eBUFFER_IOTYPE_INPUT},
    {BID_META_IN_HAL_MAIN1, eBUFFER_IOTYPE_INPUT},
    {BID_META_IN_HAL_MAIN2, eBUFFER_IOTYPE_INPUT},
    {BID_META_OUT_APP, eBUFFER_IOTYPE_OUTPUT},
    {BID_META_OUT_HAL, eBUFFER_IOTYPE_OUTPUT},

    // each setting ends by this
    {BID_INVALID, eBUFFER_IOTYPE_INPUT}
};


}; // namespace NSFeaturePipe_DepthMap
}; // namespace NSCamFeature
}; // namespace NSCam

#endif






