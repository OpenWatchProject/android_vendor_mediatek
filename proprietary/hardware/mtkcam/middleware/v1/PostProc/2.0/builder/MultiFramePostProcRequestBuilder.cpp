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

#define LOG_TAG "MulitFramePostProcRequestBuilder"

// Standard C header file
#include <algorithm>
// Android system/core header file

// mtkcam custom header file
#include <mtkcam/utils/std/Log.h>
// mtkcam global header file
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
// Module header file
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
#include <mtkcam/middleware/v1/IParamsManager.h> // size related: to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h> // size related
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/TableDefine.h>
// Local header file
#include "MulitFramePostProcRequestBuilder.h"

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
// use to check current image frame is last or not.
// If this processor needs callback more picture, please modify this value.
#define IMAGE_COUNT 1
// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif // SCOPE_TIMER

#define UNUSED(expr) do { (void)(expr); } while (0)
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android::NSPostProc;
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
MulitFramePostProcRequestBuilder::
MulitFramePostProcRequestBuilder(
    std::string const& name,
    android::sp<ILegacyPipeline> pipeline,
    android::sp<ImageStreamManager> imageStreamManager,
    android::sp<PostProcRequestSetting> setting
) : PostProcRequestBuilderBase(name, pipeline, imageStreamManager, setting)
{
    // CallbackImageBufferManager will allocate buffer and provider will be set
    // to ImageStreamManager.
    mpCallbackImageBufferManager  = new CallbackImageBufferManager(
                                            (MUINT32)setting->mProcessType,
                                            mImageStreamManager);
    mpCallbackBufferHandler       = new BufferCallbackHandler((MUINT32)setting->mProcessType);
    mpCallbackBufferHandler->setImageCallback(this);

    // TODO: refactor into base
    mSupportedInputStreamId.push_back(eSTREAMID_IMAGE_PIPE_YUV_00);

    for(auto item : mSupportedInputStreamId)
    {
        MY_LOGD("support streamId: %d", item);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MulitFramePostProcRequestBuilder::
~MulitFramePostProcRequestBuilder()
{
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
MulitFramePostProcRequestBuilder::
allocOutputBuffer()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(t1, "alloc buffer");
    status_t ret = UNKNOWN_ERROR;
    //
    sp<IImageStreamInfo> pResultImageStreamInfo = nullptr;
    sp<IImageStreamInfo> pThumbImageStreamInfo = nullptr;
    // jpeg - bokeh
    sp<IImageStreamInfo> pJpegStreamInfo = nullptr;
    //
    MSize finalPictureSize = MSize(mSetting->mShotParam.mi4PictureWidth, mSetting->mShotParam.mi4PictureHeight);
    MSize finalThumbnailSize = MSize(mSetting->mJpegParam.mi4JpegThumbWidth, mSetting->mJpegParam.mi4JpegThumbHeight);
    if(mSetting->mShotParam.mu4Transform & eTransform_ROT_90)
    {
        finalPictureSize   = MSize(mSetting->mShotParam.mi4PictureHeight, mSetting->mShotParam.mi4PictureWidth);
        finalThumbnailSize = MSize(mSetting->mJpegParam.mi4JpegThumbHeight, mSetting->mJpegParam.mi4JpegThumbWidth);
    }
    MY_LOGD("image buffer size: %ld final capture size:%dx%d",
                mSetting->mvInputData[0].mImageQueue.size(),
                finalPictureSize.w, finalPictureSize.h);

    // pResultImageStreamInfo
    {
        pResultImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_YUV_JPEG")+std::to_string(mSetting->mRequestNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_NV21,
                                        finalPictureSize,
                                        mSetting->mShotParam.mu4Transform);
    }
    // pThumbImageStreamInfo
    {
        pThumbImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_YUV_THUMB")+std::to_string(mSetting->mRequestNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_YV12,
                                        finalThumbnailSize,
                                        mSetting->mShotParam.mu4Transform);
    }
    // jpgClean
    {
        pJpegStreamInfo =
                        createImageStreamInfo((std::string("App:Image:JpegEnc_TP")+std::to_string(mSetting->mRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_JPEG,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // update dst stream
    auto addDstStreams = [](BufferList& dstStream, MINT32 streamId, MBOOL bNeedProvider)
    {
        dstStream.push_back(
            BufferSet{
                .streamId       = streamId,
                .criticalBuffer = bNeedProvider,
            }
        );
    };
    // update empty stream
    addDstStreams(mvPrecedingDstStreams, eSTREAMID_META_APP_DYNAMIC_TP, MFALSE);
    addDstStreams(mvPrecedingDstStreams, eSTREAMID_META_HAL_DYNAMIC_TP, MFALSE);
    // update final stream
    // metadata
    addDstStreams(mvWorkingDstStreams, eSTREAMID_META_APP_DYNAMIC_TP, MFALSE);
    addDstStreams(mvWorkingDstStreams, eSTREAMID_META_HAL_DYNAMIC_TP, MFALSE);
    // image
    addDstStreams(mvWorkingDstStreams, eSTREAMID_IMAGE_PIPE_YUV_JPEG, MTRUE);
    addDstStreams(mvWorkingDstStreams, eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, MTRUE);
    addDstStreams(mvWorkingDstStreams, eSTREAMID_IMAGE_JPEG, MTRUE);

    // allocate buffer, update ImageStreamManager with new Pool & Provider
    MBOOL bAllocatedBufferRet = MTRUE;
    bAllocatedBufferRet = mpCallbackImageBufferManager->allocBuffer(pJpegStreamInfo, mpCallbackBufferHandler);
    bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pResultImageStreamInfo);
    bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pThumbImageStreamInfo);

    if(!bAllocatedBufferRet)
    {
        MY_LOGD("set time stamp for output buffer: req#: %d, timeStamp: %lld", mSetting->mRequestNo, mSetting->miTimeStamp);
        mpCallbackImageBufferManager->setTimestamp(mSetting->mRequestNo, mSetting->miTimeStamp);
        ret = OK;
    }
    else
    {
        MY_LOGE("failed to allocate buffer");
    }

lbExit:
    MY_LOGD("-");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
MulitFramePostProcRequestBuilder::
setInputBuffer(
    MUINT32 inputDataSeq,
    MUINT32 index,
    const std::map<MINT32, IPostProcRequestBuilder::CallbackBufferPoolItem>& bufferPoolTable
)
{
    FUNCTION_SCOPE;
    // for default behavior, it will set full yuv and resize yuv to input pool.
    // default eSTREAMID_IMAGE_PIPE_YUV_00 is full yuv.
    // default eSTREAMID_IMAGE_PIPE_YUV_01 is resize yuv.
#define SET_INPUT_BUFFER(streamId, bufferPool, inputSEQ, FRAME_INDEX) \
    do\
    {\
        if(FRAME_INDEX >= mSetting->mvInputData[inputSEQ].mImageQueue.size())\
        {\
            MY_LOGE("Index(" #FRAME_INDEX ") is bigger than queue size(%d)", mSetting->mvInputData[inputSEQ].mImageQueue.size());\
            return UNKNOWN_ERROR;\
        }\
        ssize_t imgQueIndex = mSetting->mvInputData[inputSEQ].mImageQueue[FRAME_INDEX].indexOfKey(streamId);\
        if(imgQueIndex >= 0)\
        {\
            if(bufferPool != nullptr)\
            {\
                sp<IImageBuffer> pBuffer = mSetting->mvInputData[inputSEQ].mImageQueue[FRAME_INDEX].valueAt(imgQueIndex);\
                MY_LOGD("[%d][%d] streamid(%x) pBuffer(%x) heap(%x)", inputSEQ, FRAME_INDEX, streamId, pBuffer.get(), pBuffer->getImageBufferHeap());\
                bufferPool->addBuffer(pBuffer);\
                sp<NSCam::v1::StreamBufferProvider> sbProvider = mImageStreamManager->getBufProviderByStreamId(streamId);\
                if(sbProvider != nullptr)\
                {\
                    MY_LOGD("doTimestampCallback, req#: %d, timeStamp:%lld", mSetting->mRequestNo, mSetting->miTimeStamp);\
                    sbProvider->doTimestampCallback(mSetting->mRequestNo, MFALSE, mSetting->miTimeStamp);\
                }\
            }\
            else\
            {\
                MY_LOGE("[" #FRAME_INDEX "]" #bufferPool " is null");\
            }\
        }\
        else\
        {\
            MY_LOGD("[" #FRAME_INDEX "] miss -" #streamId);\
        }\
    }while(0);

    for(auto item : bufferPoolTable)
    {
        const MINT32 streamId = item.first;
        if ( std::find(mSupportedInputStreamId.begin(), mSupportedInputStreamId.end(), streamId) != mSupportedInputStreamId.end())
        {
            // set notifier
            sp<NSCam::v1::CallbackBufferPool> pBufferPool = std::get<1>(item.second);
            pBufferPool->setNotifier(this);
            SET_INPUT_BUFFER(streamId, pBufferPool, inputDataSeq, index);
        }
    }

#undef SET_INPUT_BUFFER
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
MulitFramePostProcRequestBuilder::
onGetPrecedingData(
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata,
    ThirdPartyType& type
)
{
    UNUSED(index);
    UNUSED(appMetadata);
    UNUSED(halMetadata);
    type = ThirdPartyType::SINGLE_3RD_F_WO_JPEG;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
MulitFramePostProcRequestBuilder::
onGetWorkingData(
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata,
    ThirdPartyType& type
)
{
    UNUSED(index);
    UNUSED(appMetadata);
    UNUSED(halMetadata);
    type = ThirdPartyType::SINGLE_3RD_F_W_JPEG;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
MulitFramePostProcRequestBuilder::
onResultReceived(
    MUINT32    const requestNo,
    StreamId_T const streamId,
    MBOOL      const errorBuffer,
    android::sp<IImageBuffer>& pBuffer
)
{
    FUNCTION_SCOPE;
    if(errorBuffer)
    {
        MY_LOGW("receive error image buffer");
    }
    // todo : dump buffer for debug
    bool bFinalFrame = (++mImageCount) == IMAGE_COUNT;
    if(mSetting->mpImageCallback != nullptr)
    {
        pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
        MUINT32 u4Size = pBuffer->getBitstreamSize();
        uint8_t const* puBuf = (uint8_t const*)pBuffer->getBufVA(0);
        mSetting->mpImageCallback->onCB_CompressedImage_packed(
                            mSetting->mProcessType,
                            mSetting->miTimeStamp,
                            u4Size,
                            puBuf,
                            MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                            bFinalFrame
                            );
        pBuffer->unlockBuf(LOG_TAG);
        if(bFinalFrame)
        {
            sp<IPostProcCB> cb = mEventCb.promote();
            if(cb != nullptr)
            {
                cb->onEvent(
                        requestNo,
                        PostProcCB::PROCESS_DONE,
                        streamId,
                        errorBuffer);
            }
        }
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
MulitFramePostProcRequestBuilder::
onBufferAcquired(
    MINT32           rRequestNo,
    StreamId_T       rStreamId
)
{
    UNUSED(rRequestNo);
    UNUSED(rStreamId);
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
MulitFramePostProcRequestBuilder::
onBufferReturned(
    MINT32                         rRequestNo,
    StreamId_T                     rStreamId,
    bool                           bErrorBuffer,
    android::sp<IImageBufferHeap>& rpBuffer
)
{
    MY_LOGD("req(%d) streamid(%x)", rRequestNo, rStreamId);
    return true;
}
