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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ICAPTURESTREAMUPDATERPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ICAPTURESTREAMUPDATERPOLICY_H_
//
#include "types.h"

#include <functional>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace capturestreamupdater {


/**
 * A structure definition for output parameters
 */
struct RequestOutputParams
{
    /**
     * The Jpeg orientation is passed to HAL at the request stage.
     */
    android::sp<IImageStreamInfo>*              pHalImage_Jpeg_YUV = nullptr;

    /**
     * The thumbnail size is passed to HAL at the request stage.
     */
    android::sp<IImageStreamInfo>*              pHalImage_Thumbnail_YUV = nullptr;

    /**
     * The Sub Jpeg orientation is passed to HAL at the request stage.
     */
    android::sp<IImageStreamInfo>*              pHalImage_Jpeg_Sub_YUV = nullptr;

    /**
     * The depth orientation is passed to HAL at the request stage.
     */
    android::sp<IImageStreamInfo>*              pHalImage_Depth_YUV = nullptr;

};


/**
 * A structure definition for input parameters
 */
struct RequestInputParams
{
    /**************************************************************************
     * Request parameters
     *
     * The parameters related to this capture request is shown as below.
     *
     **************************************************************************/

    /**
     * Request App metadata control, sent at the request stage.
     *
     * pRequest_ParsedAppMetaControl is a partial parsed result from pRequest_AppControl,
     * just for the purpose of a quick reference.
     */
    IMetadata const*                            pRequest_AppControl = nullptr;
    ParsedMetaControl const*                    pRequest_ParsedAppMetaControl = nullptr;


    /*************************************************************************
     * Configuration info.
     *
     * The final configuration information of the pipeline decided at the
     * configuration stage are as below.
     *
     **************************************************************************/
    android::sp<IImageStreamInfo> const*        pConfiguration_HalImage_Jpeg_YUV = nullptr;
    android::sp<IImageStreamInfo> const*        pConfiguration_HalImage_Thumbnail_YUV = nullptr;
    // [Jpeg pack]
    android::sp<IImageStreamInfo> const*        pConfiguration_HalImage_Jpeg_Sub_YUV = nullptr;
    android::sp<IImageStreamInfo> const*        pConfiguration_HalImage_Depth_YUV = nullptr;


    /*************************************************************************
     * Static info.
     *
     **************************************************************************/

    /**
     * true indicates Jpeg capture with rotation is supported.
     */
    bool                                        isJpegRotationSupported = true;

    /**
    * current sensor id
    */
    MUINT32 sensorID;

    /**
    * current sensor id
    */
    bool                                        isSupportJpegPack = false;

};


};  //namespace capturestreamupdater


////////////////////////////////////////////////////////////////////////////////


/**
 * The function type definition.
 * It is used to decide whether or not to update the capture streams.
 *
 * @param[out] out: input parameters
 *
 * @param[in] in: input parameters
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
using FunctionType_CaptureStreamUpdaterPolicy
    = std::function<int(
        capturestreamupdater::RequestOutputParams& /*out*/,
        capturestreamupdater::RequestInputParams const& /*in*/
    )>;


//==============================================================================


/**
 * Policy instance makers
 *
 */

// default version
FunctionType_CaptureStreamUpdaterPolicy makePolicy_CaptureStreamUpdater_Default();


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ICAPTURESTREAMUPDATERPOLICY_H_

