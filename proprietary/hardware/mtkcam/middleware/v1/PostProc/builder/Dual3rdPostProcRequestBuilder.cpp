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

#define LOG_TAG "Dual3rdPostProcRequestBuilder"


// Standard C header file
#include <algorithm>
#include <mutex>
// Android system/core header file

// mtkcam custom header file
#include <mtkcam/utils/std/Log.h>
// mtkcam global header file
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
// Module header file
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/middleware/v1/IParamsManager.h> // size related: to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h> // size related
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/TableDefine.h>
// Local header file
#include "Dual3rdPostProcRequestBuilder.h"

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
namespace {

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

template <typename T>
inline MBOOL
trySetMetadata(
        IMetadata& metadata,
        MUINT32 const tag,
        T const& val)
{
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    if (OK == metadata.update(entry.tag(), entry)) {
        return MTRUE;
    }
    //
    return MFALSE;
}

MBOOL
tryGetDepthMapSize(
        android::sp<PostProcRequestSetting> setting,
        const MSize& finialImageSize,
        MSize& depthMapSize)
{
    // get depth map size
    ENUM_STEREO_RATIO stereo_ratio;

    // todo: wait to implement
    // compute capture ratio
    double ratio = 0;
    if(finialImageSize.w > finialImageSize.h)
    {
        ratio = (double)finialImageSize.w / (double) finialImageSize.h;
    }
    else
    {
        ratio = (double)finialImageSize.h / (double) finialImageSize.w;
    }

    double ratio_4_3 = 4.0/3.0;
    if(ratio == ratio_4_3)
    {
        MY_LOGD("set ratio: 4:3");
        stereo_ratio = ENUM_STEREO_RATIO::eRatio_4_3;
    }
    else
    {
        MY_LOGD("set ratio: 16:9");
        stereo_ratio = ENUM_STEREO_RATIO::eRatio_16_9;
    }

    MINT32 sensorProfile = 0;
    if(setting->mvInputData[0].mHalMetadataQueue.size() > 0)
    {
        if(!tryGetMetadata<MINT32>(&setting->mvInputData[0].mHalMetadataQueue[0], MTK_STEREO_FEATURE_SENSOR_PROFILE, sensorProfile))
        {
            MY_LOGE("failed to get sensor profile");
            return MFALSE;
        }
    }
    else
    {
        MY_LOGE("input hal metadata is empty");
        return MFALSE;
    }
    depthMapSize = StereoSizeProvider::getInstance()->thirdPartyDepthmapSize((ENUM_STEREO_SENSOR_PROFILE)sensorProfile, stereo_ratio);

    return MTRUE;
}

template<typename T>
inline MVOID
setMetadata(IMetadata& metaData, MINT32 tag, T value)
{

}

template<>
inline MVOID
setMetadata(IMetadata& halMetadata, MINT32 tag, MSize value)
{
    IMetadata::IEntry entry(tag);
    entry.push_back(value.w, Type2Type<MINT32>());
    entry.push_back(value.h, Type2Type<MINT32>());
    halMetadata.update(entry.tag(), entry);
}


sp<IImageBuffer>
getEmptyImgBuffer()
{
    static std::mutex locker;
    static sp<IImageBuffer> imgBufferPtr = nullptr;
    if(imgBufferPtr == nullptr)
    {
        std::lock_guard<std::mutex> _l(locker);
        if(imgBufferPtr == nullptr)
        {
            MSize imgSize = {2, 2};
            MUINT32 strides[3] = {2, 1, 1};
            MUINT32 fullRawBoundary[3] = {0, 0, 0};
            IImageBufferAllocator::ImgParam imgParam = {eImgFmt_YV12, imgSize, strides, fullRawBoundary, 3};

            IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
            imgBufferPtr = allocator->alloc(LOG_TAG, imgParam);

            imgBufferPtr->lockBuf(LOG_TAG);
            {
                MY_LOGD("create empty buffer and push into container: addr: %p (PA:%p, VA:%p), heap: %p",
                    imgBufferPtr.get(), imgBufferPtr->getBufPA(0), imgBufferPtr->getBufVA(0), imgBufferPtr->getImageBufferHeap());
            }
            imgBufferPtr->unlockBuf(LOG_TAG);
        }
    }
    return imgBufferPtr;
}

} // anonymouse namespace
/******************************************************************************
 *
 ******************************************************************************/
Dual3rdPostProcRequestBuilder::
Dual3rdPostProcRequestBuilder(
    std::string const& name,
    android::sp<ILegacyPipeline> pipeline,
    android::sp<ImageStreamManager> imageStreamManager,
    android::sp<PostProcRequestSetting> setting
)
: PostProcRequestBuilderBase(name, pipeline, imageStreamManager, setting)
{
    MY_LOGD("ctor, firstRequestNum:%d, addr:%p", mFirstRequestNum, this);

    // CallbackImageBufferManager will allocate buffer and provider will be set
    // to ImageStreamManager.
    mpCallbackImageBufferManager  = new CallbackImageBufferManager(
                                            (MUINT32)setting->mProcessType,
                                            mImageStreamManager);
    mpCallbackBufferHandler       = new BufferCallbackHandler((MUINT32)setting->mProcessType);
    mpCallbackBufferHandler->setImageCallback(this);
    if(mImageStreamManager != nullptr)
    {
        // check if need jpeg00
        mbSupportJpeg00 = true;
        // check if need depth out
        mbSupportDepthOut = true;
    }

    // get outputImageSize
    const MBOOL isRolation = setting->mShotParam.mu4Transform & eTransform_ROT_90;
    if(isRolation)
    {
        mFinalPictureSize = MSize(mSetting->mShotParam.mi4PictureHeight, mSetting->mShotParam.mi4PictureWidth);;
    }
    else
    {
        mFinalPictureSize = MSize(setting->mShotParam.mi4PictureWidth, setting->mShotParam.mi4PictureHeight);
    }

    // no need to transform thumbnail, it is done by JpegNode
    mFinalThumbnailSize = MSize(setting->mJpegParam.mi4JpegThumbWidth, setting->mJpegParam.mi4JpegThumbHeight);

    mDepthMapSize = MSize(1, 1);
    if(!tryGetDepthMapSize(setting, mFinalPictureSize, mDepthMapSize))
    {
        MY_LOGE("faile to get depth map size");
    }

    MY_LOGD("output imgage buffer size, isRolation:%d,  finalPictureSize:%dx%d, finalThumbnailSize:%dx%d, depthMapSize:%dx%d",
        isRolation, mFinalPictureSize.w, mFinalPictureSize.h, mFinalThumbnailSize.w, mFinalThumbnailSize.h, mDepthMapSize.w, mDepthMapSize.h);

    // set support stream
    mSupportedInputStreamId.push_back(eSTREAMID_IMAGE_PIPE_YUV_00);
    mSupportedInputStreamId.push_back(eSTREAMID_IMAGE_PIPE_YUV_01);

    // TODO: refactor into base
    for(auto item : mSupportedInputStreamId)
    {
        MY_LOGD("support streamId: %d", item);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
Dual3rdPostProcRequestBuilder::
~Dual3rdPostProcRequestBuilder()
{
    MY_LOGD("dctor, firstRequestNum:%d, addr:%p", mFirstRequestNum, this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
Dual3rdPostProcRequestBuilder::
allocOutputBuffer()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(t1, "alloc buffer");
    status_t ret = UNKNOWN_ERROR;
    //
    sp<IImageStreamInfo> pResultImageStreamInfo = nullptr;
    sp<IImageStreamInfo> pThumbImageStreamInfo = nullptr;
    sp<IImageStreamInfo> pCleanImageStreamInfo = nullptr;
    sp<IImageStreamInfo> pDepthMapImageStreamInfo = nullptr;
    // jpeg - bokeh
    sp<IImageStreamInfo> pJpegStreamInfo = nullptr;
    sp<IImageStreamInfo> pCleanJpegStreamInfo = nullptr;

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
                                        mFinalPictureSize,
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
                                        mFinalThumbnailSize,
                                        mSetting->mShotParam.mu4Transform);
    }
    // pCleanImageStreamInfo
    if(mbSupportJpeg00)
    {
        pCleanImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_YUV_JPEG0")+std::to_string(mSetting->mRequestNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_TP_OUT00,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_NV21,
                                        mFinalPictureSize,
                                        0);
    }
    // pDepthMapSize
    if(mbSupportDepthOut)
    {
        pDepthMapImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_Y_Depth")+std::to_string(mSetting->mRequestNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_Y8,
                                        mDepthMapSize,
                                        0);
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
                                              mFinalPictureSize,
                                              0);
    }
    // jpgClean
    if(mbSupportJpeg00)
    {
        pCleanJpegStreamInfo =
                        createImageStreamInfo((std::string("App:Image:JpegEnc_TP0")+std::to_string(mSetting->mRequestNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              mFinalPictureSize,
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
    if(mbSupportDepthOut)
    {
        addDstStreams(mvWorkingDstStreams, eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV, MTRUE);
    }
    if(mbSupportJpeg00)
    {
        addDstStreams(mvWorkingDstStreams, eSTREAMID_IMAGE_PIPE_TP_OUT00, MTRUE);
        addDstStreams(mvWorkingDstStreams, eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00, MTRUE);
    }

    // allocate buffer, update ImageStreamManager with new Pool & Provider
    MBOOL bAllocatedBufferRet = MTRUE;
    bAllocatedBufferRet = mpCallbackImageBufferManager->allocBuffer(pJpegStreamInfo, mpCallbackBufferHandler);
    mTotalImgCount++;

    bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pResultImageStreamInfo);
    bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pThumbImageStreamInfo);
    if(mbSupportDepthOut)
    {
        bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pDepthMapImageStreamInfo, mpCallbackBufferHandler);
        mTotalImgCount++;
    }
    if(mbSupportJpeg00)
    {
        bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pCleanImageStreamInfo);
        bAllocatedBufferRet |= mpCallbackImageBufferManager->allocBuffer(pCleanJpegStreamInfo, mpCallbackBufferHandler);
        mTotalImgCount++;
    }

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
    MY_LOGD("total image count (%d) -", mTotalImgCount);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
Dual3rdPostProcRequestBuilder::
setInputBuffer(
    MUINT32 frameIndex,
    MUINT32 subFrameIndex,
    const std::map<MINT32, IPostProcRequestBuilder::CallbackBufferPoolItem>& bufferPoolTable
)
{
    auto streamId2Name = [] (MINT32 id) -> const char*
    {
        #define MAKE_NAME_CASE(name)  case name: return #name
        {
            switch(id)
            {
                MAKE_NAME_CASE(eSTREAMID_IMAGE_PIPE_YUV_00);
                MAKE_NAME_CASE(eSTREAMID_IMAGE_PIPE_YUV_01);
            }
            return "unknown";
        }
        #undef MAKE_NAME_CASE
    };

    FUNCTION_SCOPE;

    const MINT32 framSize = mSetting->mvInputData.size();
    if(frameIndex >= framSize)
    {
        MY_LOGE("invalidated frameIndex: %u(%zu)", frameIndex, mSetting->mvInputData.size());
        return UNKNOWN_ERROR;
    }
    InputData& frame = mSetting->mvInputData[frameIndex];

    const MINT32 subFrameSize = frame.mImageQueue.size();
    if(subFrameIndex >= frame.mImageQueue.size())
    {
        MY_LOGE("invalidated subFrameIndex: %u(%zu)", subFrameIndex, frame.mImageQueue.size());
        return UNKNOWN_ERROR;
    }
    auto& subFrame = frame.mImageQueue[subFrameIndex];


    const MINT32 subFrameImgCount = subFrame.size();
    MY_LOGD("req#: %d, timeStamp: %lld, frameSize: %d, subFramSize: %d, subFrameImgCount: %d",
        mSetting->mRequestNo, mSetting->miTimeStamp, framSize, subFrameSize, subFrameImgCount);

    for(auto item : bufferPoolTable)
    {
        const MINT32 streamId = item.first;
        if (std::find(mSupportedInputStreamId.begin(), mSupportedInputStreamId.end(), streamId) != mSupportedInputStreamId.end())
        {
            MY_LOGD("find support streamId: %d(%s)", streamId, streamId2Name(streamId));

            const ssize_t streamIdInSubFrame = subFrame.indexOfKey(streamId);
            sp<IImageBuffer> imgBufferPtr = nullptr;
            if(streamIdInSubFrame >= 0)
            {
                imgBufferPtr = subFrame.valueAt(streamIdInSubFrame);
            }
            else
            {
                imgBufferPtr = ::getEmptyImgBuffer();
                MY_LOGD("miss streamIdInSubFrame and getEmptyImgBuffer");
            }

            sp<NSCam::v1::CallbackBufferPool>& bufferPoolPtr = std::get<1>(item.second);
            sp<NSCam::v1::StreamBufferProvider> sbProviderPtr = mImageStreamManager->getBufProviderByStreamId(streamId);

            if((imgBufferPtr == nullptr) || (bufferPoolPtr == nullptr) || (sbProviderPtr == nullptr))
            {
                MY_LOGE("invalidated imgBufferPtr(%p), or bufferPoolPtr(%p), or sbProviderPtr(%p)",
                    imgBufferPtr.get(), bufferPoolPtr.get(), sbProviderPtr.get());
                return UNKNOWN_ERROR;
            }

            MY_LOGD("setNotifier and addBuffer to pool, imgBuffer-addr: %p heap: %p, pool-addr: %p",
                imgBufferPtr.get(), imgBufferPtr->getImageBufferHeap(), bufferPoolPtr.get());
            bufferPoolPtr->setNotifier(this);
            bufferPoolPtr->addBuffer(imgBufferPtr);

            MY_LOGD("doTimestampCallback, req#: %d, timeStamp: %lld, provider: %p",
                mSetting->mRequestNo, mSetting->miTimeStamp, sbProviderPtr.get());
            sbProviderPtr->doTimestampCallback(mSetting->mRequestNo, MFALSE, mSetting->miTimeStamp);
        }
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
Dual3rdPostProcRequestBuilder::
onGetPrecedingData(
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata,
    ThirdPartyType& type
)
{
    UNUSED(index);
    UNUSED(appMetadata);
    // set plugin mode for dual cam 3rd party
    switch(mSetting->mProcessType)
    {
        case PostProcessorType::THIRDPARTY_DCMF:
            MY_LOGD("set plugin mode as DCMF");
            if(!trySetMetadata<MINT32>(halMetadata, MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_DCMF_3RD_PARTY))
            {
                MY_LOGE("set plugin mode fail.");
            }
            break;

        case PostProcessorType::THIRDPARTY_BOKEH:
        default:
            MY_LOGD("set plugin mode as BOKEH");
            if(!trySetMetadata<MINT32>(halMetadata, MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_BOKEH_3RD_PARTY))
            {
                MY_LOGE("set plugin mode fail.");
            }
            break;
    }

    MRect cropSize;
    if(tryGetMetadata<MRect>(&appMetadata, MTK_SCALER_CROP_REGION, cropSize))
    {
        MY_LOGD("crop size (%d, %d, %d, %d)", cropSize.p.x, cropSize.p.y, cropSize.s.w, cropSize.s.h);
    }
    else
    {
        MY_LOGE("can not get crop size");
    }
    MINT32 openId;
    if(tryGetMetadata<MINT32>(&halMetadata, MTK_STEREO_FEATURE_OPEN_ID, openId))
    {
        MY_LOGD("openId(%d)", openId);
    }
    else
    {
        MY_LOGE("can not get open id");
    }
    // set value to meta
    {
        ::setMetadata<MSize>(halMetadata, MTK_STEREO_DCMF_DEPTHMAP_SIZE, mDepthMapSize);
    }

    type = ThirdPartyType::DUAL_3RD_FR_WO_JPEG;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
Dual3rdPostProcRequestBuilder::
onGetWorkingData(
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata,
    ThirdPartyType& type
)
{
    FUNCTION_SCOPE;

    UNUSED(index);
    UNUSED(appMetadata);
    // set plugin mode for dual cam 3rd party
    switch(mSetting->mProcessType)
    {
        case PostProcessorType::THIRDPARTY_DCMF:
            MY_LOGD("set plugin mode as DCMF");
            if(!trySetMetadata<MINT32>(halMetadata, MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_DCMF_3RD_PARTY))
            {
                MY_LOGE("set plugin mode fail.");
            }
            break;

        case PostProcessorType::THIRDPARTY_BOKEH:
        default:
            MY_LOGD("set plugin mode as BOKEH");
            if(!trySetMetadata<MINT32>(halMetadata, MTK_PLUGIN_MODE, MTK_PLUGIN_MODE_BOKEH_3RD_PARTY))
            {
                MY_LOGE("set plugin mode fail.");
            }
            break;
    }

    MRect cropSize;
    if(tryGetMetadata<MRect>(&appMetadata, MTK_SCALER_CROP_REGION, cropSize))
    {
        MY_LOGD("crop size (%d, %d, %d, %d)", cropSize.p.x, cropSize.p.y, cropSize.s.w, cropSize.s.h);
    }
    else
    {
        MY_LOGE("can not get crop size");
    }
    MINT32 openId;
    if(tryGetMetadata<MINT32>(&halMetadata, MTK_STEREO_FEATURE_OPEN_ID, openId))
    {
        MY_LOGD("openId(%d)", openId);
    }
    else
    {
        MY_LOGE("can not get open id");
    }
    if(::supportMain2FullRaw())
    {
        MY_LOGD("DUAL_3RD_FR_W_JPEG");
        type = ThirdPartyType::DUAL_3RD_FR_W_JPEG;
    }
    else
    {
        MY_LOGD("DUAL_3RD_R_W_JPEG");
        type = ThirdPartyType::DUAL_3RD_R_W_JPEG;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPostProcRequestBuilder::
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
    bool bFinalFrame = (++mImageCount) == mTotalImgCount;
    MY_LOGD("recive:%d/%d, requestNo: %u, streamId: %d, errorBuffer: %d",
        mImageCount, mTotalImgCount, requestNo, streamId, errorBuffer);

    if(mSetting->mpImageCallback != nullptr)
    {
        pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);

        // jpeg size need get frome bitstreamsize
        MUINT32 u4Size = 0;
        if(streamId == eSTREAMID_IMAGE_JPEG || streamId == eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00)
        {
            u4Size = pBuffer->getBitstreamSize();
        }
        else
        {
            u4Size = pBuffer->getBufSizeInBytes(0);
        }
        //
        auto getFileName = [&streamId]()
        {
            if(streamId == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV)
            {
                return std::string("depth_map.yuv");
            }
            else if(streamId == eSTREAMID_IMAGE_JPEG)
            {
                return std::string("result_jpeg0.jpg");
            }
            else if(streamId == eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00)
            {
                return std::string("result_jpeg00.jpg");
            }
            else
            {
                return std::string("no_name.data");
            }
        };
        dumpImageBuffer(pBuffer, getFileName());

        MUINT32 msgType = 0;
        if(streamId == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV)
        {
            msgType = MTK_CAMERA_MSG_EXT_DATA_DEPTHMAP;
        }
        else if(streamId == eSTREAMID_IMAGE_JPEG)
        {
            msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE;
        }
        else if(streamId == eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00)
        {
            msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE;
        }
        else
        {
            msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE;
        }

        uint8_t const* puBuf = (uint8_t const*)pBuffer->getBufVA(0);
        mSetting->mpImageCallback->onCB_CompressedImage_packed(
                            mSetting->mProcessType,
                            mSetting->miTimeStamp,
                            u4Size,
                            puBuf,
                            msgType,
                            bFinalFrame,
                            msgType
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
Dual3rdPostProcRequestBuilder::
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
Dual3rdPostProcRequestBuilder::
onBufferReturned(
    MINT32                         rRequestNo,
    StreamId_T                     rStreamId,
    bool                           bErrorBuffer,
    android::sp<IImageBufferHeap>& rpBuffer
)
{
    MY_LOGD("req(%d) streamid(%x) bErrorBuffer(%d) bufferHeap(%p)", rRequestNo, rStreamId, bErrorBuffer, rpBuffer.get());
    return true;
}
