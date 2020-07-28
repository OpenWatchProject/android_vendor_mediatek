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

#ifndef _MTK_CAMERA_THIRDPARTY_IPOSTPROC_REQUEST_BUILDER_H_
#define _MTK_CAMERA_THIRDPARTY_IPOSTPROC_REQUEST_BUILDER_H_

// Standard C header file
#include <string>
#include <vector>
#include <tuple>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/middleware/v1/PostProc/IPostProcCB.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/TableDefine.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/table_data_set.h>
// Local header file
#include "../../common/CbImgBufMgr/CallbackImageBufferManager.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace android {
namespace NSPostProc {
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class IPostProcRequestBuilder
 * @brief interface of post processing request builder
 */
class IPostProcRequestBuilder :
        public virtual RefBase
{
public:
    using CallbackBufferPoolItem = std::tuple<const char*, sp<NSCam::v1::CallbackBufferPool>>;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IPostProcRequestBuilder() = delete;
    IPostProcRequestBuilder(
                        std::string const& name,
                        android::sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> pipeline,
                        android::sp<ImageStreamManager> imageStreamManager,
                        android::sp<PostProcRequestSetting> setting);
    virtual ~IPostProcRequestBuilder();
    /**
     * @brief set PostProcRequestSettingt
     */
    virtual MVOID setRequestSetting(
                        android::sp<PostProcRequestSetting> setting) = 0;
    /**
     * @brief allocate output buffer
     * @return The allocate result
     */
    virtual android::status_t allocOutputBuffer() = 0;
    /**
     * @brief set input buffer
     * @return The set result
     */
    virtual android::status_t setInputBuffer(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        const std::map<MINT32, IPostProcRequestBuilder::CallbackBufferPoolItem>& bufferPoolTable) = 0;
    /**
     * @brief get preceding data.
     * @return The set result
     */
    virtual android::status_t getPrecedingData(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        NSCam::v1::NSLegacyPipeline::BufferList& mvDstStream,
                        android::NSPostProc::ThirdPartyType& type) = 0;
    /**
     * @brief get preceding data.
     * @return The set result
     */
    virtual android::status_t getWorkingData(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        NSCam::v1::NSLegacyPipeline::BufferList& mvDstStream,
                        android::NSPostProc::ThirdPartyType& type) = 0;
    /**
     * @brief set event callback
     * @return void
     */
    virtual void setEventCallback(
                        wp<IPostProcCB> cb)
                        { mEventCb = cb; }
protected:
    std::string mName;
    android::sp<PostProcRequestSetting> mSetting = nullptr;
    android::sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> mPipeline = nullptr;
    android::sp<ImageStreamManager> mImageStreamManager = nullptr;
    sp<NSCam::v1::NSLegacyPipeline::CallbackImageBufferManager> mpCallbackImageBufferManager = nullptr;
    sp<NSCam::v1::BufferCallbackHandler> mpCallbackBufferHandler = nullptr;
    //
    std::vector<MINT32> mSupportedInputStreamId;
    //
    sp<IImageCallback> mpImageCallback = nullptr;
    //
    MUINT8  mImageCount = 0;
    //
    MINT32 mFirstRequestNum;
    //
    wp<IPostProcCB> mEventCb = nullptr;
};
/*******************************************************************************
* Class Define
********************************************************************************/
class PostProcRequestBuilderBase : public IPostProcRequestBuilder
{
public:
    PostProcRequestBuilderBase() = delete;
    PostProcRequestBuilderBase(
                        std::string const& name,
                        android::sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> pipeline,
                        android::sp<ImageStreamManager> imageStreamManager,
                        android::sp<PostProcRequestSetting> setting);
    virtual ~PostProcRequestBuilderBase();
public:
    /**
     * @brief set PostProcRequestSettingt
     */
    MVOID setRequestSetting(
                        android::sp<PostProcRequestSetting> setting) override final;
    /**
     * @brief get preceding data.
     * @return The set result
     */
    android::status_t getPrecedingData(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        NSCam::v1::NSLegacyPipeline::BufferList& mvDstStream,
                        android::NSPostProc::ThirdPartyType& type) override final;
    /**
     * @brief provide interface let user to update metadata, before submit request
     * @return The set result
     */
    virtual android::status_t onGetPrecedingData(
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        android::NSPostProc::ThirdPartyType& type);
    /**
     * @brief get preceding data.
     * @return The set result
     */
    android::status_t getWorkingData(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        NSCam::v1::NSLegacyPipeline::BufferList& mvDstStream,
                        android::NSPostProc::ThirdPartyType& type) override final;
    /**
     * @brief provide interface let user to update metadata, before submit request
     * @return The set result
     */
    virtual android::status_t onGetWorkingData(
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        android::NSPostProc::ThirdPartyType& type);
    /**
     * @brief create image stream info
     * @return return IImageStreamInfo
     */
    virtual android::sp<ImageStreamInfo> createImageStreamInfo(
                                                    char const* streamName,
                                                    StreamId_T streamId,
                                                    MUINT32 streamType,
                                                    size_t maxBufNum,
                                                    size_t minInitBufNum,
                                                    MUINT usageForAllocator,
                                                    MINT imgFormat,
                                                    MSize const& imgSize,
                                                    MUINT32 transform) final;
protected:
    /**
     * @brief dump Image Buffer
     */
    virtual void dumpImageBuffer(sp<IImageBuffer> pBuffer, std::string filename);
private:
    /**
     * @brief get metadata from PostProcRequestSetting.
     * @return The set result
     */
    android::status_t setDefaultMetadata(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata);
protected:
    NSCam::v1::NSLegacyPipeline::BufferList          mvPrecedingDstStreams;
    NSCam::v1::NSLegacyPipeline::BufferList          mvWorkingDstStreams;
    MBOOL                                            mbEnableDump = MFALSE;
    std::string                                      msFilename = std::string("");
};
/*******************************************************************************
* Class Define
********************************************************************************/
class PostProcRequestBuilderFactory
{
public:
    static android::sp<IPostProcRequestBuilder> createInstance(
                                    android::sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> pipeline,
                                    android::sp<ImageStreamManager> imageStreamManager,
                                    android::sp<PostProcRequestSetting> setting);
};
};
};
#endif  //  _MTK_CAMERA_THIRDPARTY_IPOSTPROC_REQUEST_BUILDER_H_
