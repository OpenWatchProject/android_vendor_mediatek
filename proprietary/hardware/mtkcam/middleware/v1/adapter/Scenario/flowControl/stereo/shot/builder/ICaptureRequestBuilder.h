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
 * @file ICaptureRequestBuilder.h
 * @brief The interface of capture and related params.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_ICAPTURE_REQUEST_BUILDER_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_ICAPTURE_REQUEST_BUILDER_H_

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
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
// Local header file
#include "../ICaptureRequestCB.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/

namespace NSCam {
namespace v1 {
    class BufferCallbackHandler;
    class StereoBufferPool;
namespace NSLegacyPipeline {
    class StereoSelector;
    class CallbackImageBufferManager;
    class IMetaShotCallback;
    class IImageShotCallback;
    class ShotCallbackProcessor;
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
/**
 * @class CallbackImageBufferManager
 * @brief Used to maintain all callback image buffer.
 */
struct CaptureRequestInitSetting
{
    MINT32 iTargetPipelineId = -1;
    MINT32 iCaptureNo;
    StereoShotParam shotParam;
    sp<IParamsManagerV3> pParamMgr = nullptr;
    sp<ILegacyPipeline> pILegacyPipeline = nullptr;
    IMetadata appMetadata;
    IMetadata halMetadata;
    MVOID* userData = nullptr;
    // capture table
    ContextBuilderContent content;
    sp<ICaptureRequestCB> mCB;
    sp<android::NSPostProc::IPostProcRequestCB> mPostProcCB;
    sp<IShotCallback> mShotCallback;
    sp<ImageStreamManager> pImageStreamManager = NULL;
    android::NSPostProc::PostProcessorType mPostProcType = android::NSPostProc::PostProcessorType::NONE;
};
class BufferPoolInfo
{
public:
    MINT32 openId = -1;
    StreamId_T poolStreamId;
    sp<StereoBufferPool> mpPool;
    String8 name;
};
struct CaptureSetting
{
    std::vector< std::shared_ptr<BufferPoolInfo> > mvBufferPool;
};
/**
 * @class CallbackImageBufferManager
 * @brief Used to maintain all callback image buffer.
 */
class ICaptureRequestBuilder
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
    ICaptureRequestBuilder() = default;

protected:
// Destructor
    virtual ~ICaptureRequestBuilder() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  sendCallback Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init capture request builder
     * @param [in] setting using initial setting to initialize capture request builder.
     * @return return initial status
     */
    virtual status_t init(
                                CaptureRequestInitSetting &setting) = 0;
    /**
     * @brief get capture frame
     * @param [in] setting capture pipeline setting
     * @param [in] frame pieline frame
     * @return return create pipeline frame status, if return status not equal OK, don't use.
     */
    virtual status_t doCapture(
                                CaptureSetting &setting) = 0;
    /**
     * @brief destroy capture request builder.
     * @return return destroy status.
     */
    virtual status_t destroy() = 0;
protected:
    /**
     * @brief Build node config manager. Node config manager is used to decide IO map.
     * @param [in] content contain ImageTbl, MetaTbl and node connect table.
     * @return return build status.
     */
    virtual status_t buildNodeConfigManager(
                                            ContextBuilderContent content) = 0;
    /**
     * @brief get selector from IResourceContain by specific open id and stream id.
     * @param [in] openId open id to get ResourceContainer.
     * @param [in] streamId stream id to get buffer provider from ResourceContainer.
     * @return return selector
     */
    virtual sp<ISelector> getSelector(
                                            MUINT32 openId,
                                            StreamId_T streamId) = 0;
    /**
     * @brief get buffer data from selector.
     * @param [in] selector get metadata and image buffer from selector.
     * @return return buffer data.
     */
    virtual std::shared_ptr<BufferData>
                        getBufferDataFromSelect(
                                            sp<ISelector> selector,
                                            MINT32 const iAppMetadataStreamId,
                                            MINT32 const iHalMetadataStreamId) = 0;
    /**
     * @brief prepare metadata
     * @param [in] vBufferDataList buffer data list.
     * @return return buffer data.
     */
    virtual status_t prepareMetadata(
                                            IMetadata* main1AppMeta,
                                            IMetadata* main1HalMeta,
                                            IMetadata* main2AppMeta,
                                            IMetadata* main2HalMeta) = 0;
    /**
     * @brief prepare image provider for capture frame.
     * @return return allocate status
     */
    virtual status_t prepareImageProvider() = 0;
    /**
     * @brief get timestamp from metadata
     * @param [in] metadata hal meta data.
     * @return return process result.
     */
    virtual status_t getTimestamp(
                                    IMetadata *metadata, MINT64 &timestamp) = 0;
    /**
     * @brief send time stamp to provider
     * @param [in] timestamp time stamp
     * @return return process result.
     */
    virtual status_t sendTimestampToProvider(
                                    MUINT32 reqNo,
                                    MINT64 timestamp) = 0;
    /**
     * @brief set image callback
     * @return return process result.
     */
    virtual status_t setImageCallback() = 0;
    /**
     * @brief set metadata callback
     * @return return process result.
     */
    virtual status_t setMetadataCallback() = 0;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};
};


#endif