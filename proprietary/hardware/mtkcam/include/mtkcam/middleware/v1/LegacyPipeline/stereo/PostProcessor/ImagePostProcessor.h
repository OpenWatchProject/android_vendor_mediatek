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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_CAMERA_STEREO_FLOW_CONTROL_IMAGE_POST_PROCESSOR_H_
#define _MTK_CAMERA_STEREO_FLOW_CONTROL_IMAGE_POST_PROCESSOR_H_

#include <list>
#include <utils/KeyedVector.h>
// Module header file
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/middleware/v1/IShot.h>
// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace android::NSShot;

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

class ICaptureRequestCB;

/*******************************************************************************
* Class Define
********************************************************************************/
enum class PostImgProcessorType
{
    NONE = 0,
    BOKEH = 22,
    THIRDPARTY,
};
enum class CameraMode
{
    OpenCamera,
    CloseCamera,
};
class ImagePostProcessorData : virtual public android::RefBase
{
    public:
        PostImgProcessorType                                            mProcessType = PostImgProcessorType::NONE;
        android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer> > mImageQueue;
        android::DefaultKeyedVector<MINT32, IMetadata>                  mMetadataQueue;
        MINT64                                                          miTimeStamp = -1;
        ShotParam                                                       mShotParam;
        JpegParam                                                       mJpegParam;
        android::wp<ICaptureRequestCB>                                  mpCb;
        MINT32                                                          mCaptureNo;
};
/**
 * @class ImagePostProcessor
 * @brief Processing post image
 */
class ImagePostProcessor:
        virtual public android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static android::sp<ImagePostProcessor> createInstance(PostImgProcessorType type);
    /**
     * @brief enque to post processor.
     * @return The process result
     */
    virtual android::status_t                  enque(android::sp<ImagePostProcessorData> data) = 0;
    /**
     * @brief flush post processor queue.
     * @return The process result
     */
    virtual android::status_t                  flush() = 0;
    /**
     * @brief wait all processing done.
     * @return The process result
     */
    virtual android::status_t                  waitUntilDrain() = 0;
    /**
     * @brief destroy all member field.
     * @return The process result
     */
    virtual android::status_t                  destroy() = 0;
    /**
     * @brief get queue size
     * @return return size
     */
    virtual MINT32                             size() = 0;
    /**
     * @brief set camera mode
     * @return the process result
     */
    virtual android::status_t                  setCameraStatus(CameraMode mode) = 0;
    /**
     * @brief set callback
     * @return the process result
     */
    virtual android::status_t                  setCallback(android::sp<ICaptureRequestCB> pCb) = 0;


};
};
};
};
#endif  //  _MTK_CAMERA_STEREO_FLOW_CONTROL_IMAGE_POST_PROCESSOR_FACTORY_H_
