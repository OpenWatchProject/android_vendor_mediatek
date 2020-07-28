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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_HWNODE_STREAMID_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_HWNODE_STREAMID_H_
//
#include <mtkcam3/pipeline/stream/StreamId.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 * HW Pipeline Stream ID
 ******************************************************************************/
enum : int64_t
{
    //==========================================================================
    eSTREAMID_META_APP_CONTROL = eSTREAMID_END_OF_FWK,
     //==========================================================================
    eSTREAMID_BEGIN_OF_PIPE = (eSTREAMID_BEGIN_OF_INTERNAL),
    //==========================================================================

        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,

        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,

        eSTREAMID_IMAGE_PIPE_RAW_LCSO_00,

        eSTREAMID_IMAGE_PIPE_RAW_RSSO_00,

        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,

        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,

        eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,

        eSTREAMID_IMAGE_PIPE_RAW_RSSO_01,

        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_02,

        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_02,

        eSTREAMID_IMAGE_PIPE_RAW_LCSO_02,

        eSTREAMID_IMAGE_PIPE_RAW_RSSO_02,

        eSTREAMID_IMAGE_PIPE_YUV_JPEG_00,

        eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_00,

        eSTREAMID_IMAGE_PIPE_JPEG, // HAL Jpeg for debug & tuning on YUV capture

        eSTREAMID_IMAGE_FD,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_META_PIPE_CONTROL_00_SENSOR,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_META_PIPE_CONTROL,
        eSTREAMID_META_PIPE_CONTROL_MAIN2,
        eSTREAMID_META_PIPE_CONTROL_MAIN3,
        eSTREAMID_META_PIPE_DYNAMIC_01,
        eSTREAMID_META_PIPE_DYNAMIC_01_MAIN2,
        eSTREAMID_META_PIPE_DYNAMIC_01_MAIN3,
        eSTREAMID_META_PIPE_DYNAMIC_02,
        eSTREAMID_META_PIPE_DYNAMIC_02_CAP,
        eSTREAMID_META_PIPE_DYNAMIC_PDE,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // metadata for event callback
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        eSTREAMID_META_APP_DYNAMIC_CALLBACK,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_META_APP_DYNAMIC_01,
        eSTREAMID_META_APP_DYNAMIC_01_MAIN2,
        eSTREAMID_META_APP_DYNAMIC_01_MAIN3,
        eSTREAMID_META_APP_DYNAMIC_02,
        eSTREAMID_META_APP_DYNAMIC_02_CAP,
        eSTREAMID_META_APP_DYNAMIC_FD,
        eSTREAMID_META_APP_DYNAMIC_JPEG,
        eSTREAMID_META_APP_DYNAMIC_RAW16,
        eSTREAMID_META_APP_DYNAMIC_P1ISPPack,
        eSTREAMID_META_APP_DYNAMIC_P2ISPPack,
        eSTREAMID_META_APP_DYNAMIC_PHYSICAL_DUMMY_CAM0,
        eSTREAMID_META_APP_DYNAMIC_PHYSICAL_DUMMY_CAM1,
        eSTREAMID_META_APP_DYNAMIC_PHYSICAL_DUMMY_CAM2,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_IMAGE_HDR,
        eSTREAMID_META_PIPE_DYNAMIC_HDR,
        eSTREAMID_META_APP_DYNAMIC_HDR,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        eSTREAMID_IMAGE_NR3D,

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // VSDoF
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        eSTREAMID_IMAGE_PIPE_JPS_MAIN,
        eSTREAMID_IMAGE_PIPE_JPS_SUB,
        eSTREAMID_META_PIPE_DYNAMIC_03,
        eSTREAMID_META_PIPE_DYNAMIC_DEPTH,
        eSTREAMID_IMAGE_PIPE_YUV_JPS,
        eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL_JPS,

        eSTREAMID_IMAGE_PIPE_P1_SCALED_YUV_00,
        eSTREAMID_IMAGE_PIPE_P1_SCALED_YUV_01,
        eSTREAMID_IMAGE_PIPE_P1_SCALED_YUV_02,

        // [Jpeg pack]
        eSTREAMID_IMAGE_PIPE_YUV_JPEG_01,
        eSTREAMID_IMAGE_PIPE_DEPTH_MAP_YUV,

        // ISP HIDL
        eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RAW,
        eSTREAMID_IMAGE_ISP_DEVICE_INPUT_YUV,
        eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO,
        eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED,
        eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING,

        // Physical metadata
        eSTREAMID_META_APP_DYNAMIC_02_MAIN1         = eSTREAMID_BEGIN_OF_PHYSIC_ID,
        eSTREAMID_META_APP_DYNAMIC_02_MAIN2,
        eSTREAMID_META_APP_DYNAMIC_02_MAIN3,
        eSTREAMID_META_APP_DYNAMIC_02_CAP_MAIN1,
        eSTREAMID_META_APP_DYNAMIC_02_CAP_MAIN2,
        eSTREAMID_META_APP_DYNAMIC_02_CAP_MAIN3,
        eSTREAMID_META_APP_DYNAMIC_RAW16_MAIN1,
        eSTREAMID_META_APP_DYNAMIC_RAW16_MAIN2,
        eSTREAMID_META_APP_DYNAMIC_RAW16_MAIN3,
        eSTREAMID_META_APP_DYNAMIC_P1ISPPack_MAIN1,
        eSTREAMID_META_APP_DYNAMIC_P2ISPPack_MAIN1,
        eSTREAMID_META_APP_DYNAMIC_P1ISPPack_MAIN2,
        eSTREAMID_META_APP_DYNAMIC_P2ISPPack_MAIN2,
        eSTREAMID_META_APP_DYNAMIC_P1ISPPack_MAIN3,
        eSTREAMID_META_APP_DYNAMIC_P2ISPPack_MAIN3,

    //==========================================================================
    eSTREAMID_END_OF_PIPE = eSTREAMID_END_OF_INTERNAL,
    //==========================================================================
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_HWNODE_STREAMID_H_

