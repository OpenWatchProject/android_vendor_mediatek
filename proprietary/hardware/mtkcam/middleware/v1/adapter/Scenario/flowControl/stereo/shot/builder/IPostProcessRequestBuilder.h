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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
 * @file IPostProcessRequestBuilder.h
 * @brief The interface of capture and related params.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_IPOSTPROC_REQUEST_BUILDER_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_IPOSTPROC_REQUEST_BUILDER_H_

// Standard C header file

// Android system/core header file
#include <mtkcam/def/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoCommon.h>
// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace v1 {
    class BufferCallbackHandler;
    class IImageCallback;
//    class StereoBufferPool;
    class CallbackBufferPool;
namespace NSLegacyPipeline {
    class StereoSelector;
    class CallbackImageBufferManager;
    class BokehPostProcessor;
    class ILegacyPipeline;
};
};
};
using namespace std;
using namespace android;
using namespace android::NSShot;
using namespace NSCam::v1;
using namespace NSCam::v3;
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
/*******************************************************************************
* Class Define
********************************************************************************/
struct PostProcRequestInitSetting
{
    MINT32 iTargetPipelineId = -1;
    MINT32 iRequestNo;
    MINT64 timestamp;
    ShotParam shotParam;
    JpegParam jpegParam;

    MSize depthMapWrapper;
    MUINT32 extraDataSize;
    sp<ILegacyPipeline> pILegacyPipeline = nullptr;
    IMetadata appMetadata;
    IMetadata halMetadata;
    sp<IImageCallback> pImageCallback = nullptr;
    sp<ImageStreamManager> pImageStreamManager = nullptr;
};
/**
 * @class PostProcRequestBuilder
 * @brief Used to build PostProc Pipeline Frame and do PostProc
 */
class IPostProcessRequestBuilder
    : public virtual RefBase
{
protected:
    class BufferData
    {
    public:
        MINT32  openId;
        StreamId_T imgStreamId;
        IMetadata mAppMetadata;
        IMetadata mHalMetadata;
        sp<IImageBufferHeap> mBufferHeap = nullptr;
    };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    IPostProcessRequestBuilder() = default;

protected:
// Destructor
    virtual ~IPostProcessRequestBuilder() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPostProcessRequestBuilder Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init PostProc request builder
     * @param [in] setting using initial setting to initialize PostProc request builder.
     * @return return initial status
     */
    virtual status_t init(PostProcRequestInitSetting &setting) = 0;
    /**
     * @brief get PostProc frame
     * @param [in] setting PostProc pipeline setting
     * @param [in] frame pieline frame
     * @return return create pipeline frame status, if return status not equal OK, don't use.
     */
    virtual status_t doPostProc() = 0;
    /**
     * @brief destroy PostProc request builder.
     * @return return destroy status.
     */
    virtual status_t destroy() = 0;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif
