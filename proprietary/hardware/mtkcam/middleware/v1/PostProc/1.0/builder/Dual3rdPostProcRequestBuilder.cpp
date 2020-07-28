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
// mtkcam
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
// size related
#include <mtkcam/middleware/v1/IParamsManager.h> // to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
// metadata tag
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
//
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/TableDefine.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#include "Dual3rdPostProcRequestBuilder.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android::NSPostProc;
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
/******************************************************************************
 *
 ******************************************************************************/
#define UNUSED(expr) do { (void)(expr); } while (0)
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
template <class T>
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
/******************************************************************************
 *
 ******************************************************************************/
Dual3rdPostProcRequestBuilder::
Dual3rdPostProcRequestBuilder(
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
    if(mImageStreamManager != nullptr)
    {
        // check if need jpeg00
        mbSupportJpeg00 = true;
        // check if need depth out
        mbSupportDepthOut = true;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
Dual3rdPostProcRequestBuilder::
~Dual3rdPostProcRequestBuilder()
{
    MY_LOGD("dctor %p", this);
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
    //
    MSize finalPictureSize = MSize(mSetting->mShotParam.mi4PictureWidth, mSetting->mShotParam.mi4PictureHeight);
    MSize finalThumbnailSize = MSize(mSetting->mJpegParam.mi4JpegThumbWidth, mSetting->mJpegParam.mi4JpegThumbHeight);
    if(mSetting->mShotParam.mu4Transform & eTransform_ROT_90)
    {
        MY_LOGD("eTransform_ROT_90");
        finalPictureSize   = MSize(mSetting->mShotParam.mi4PictureHeight, mSetting->mShotParam.mi4PictureWidth);
        //finalThumbnailSize = MSize(mSetting->mJpegParam.mi4JpegThumbHeight, mSetting->mJpegParam.mi4JpegThumbWidth);
    }
    //
    MSize depthMapSize = MSize(1, 1);
    // get depth map size
    ENUM_STEREO_RATIO stereo_ratio;
    {
        // todo: wait to implement
        // compute capture ratio
        double ratio = 0;
        if(finalPictureSize.w > finalPictureSize.h)
        {
            ratio = (double)finalPictureSize.w / (double) finalPictureSize.h;
        }
        else
        {
            ratio = (double)finalPictureSize.h / (double) finalPictureSize.w;
        }
        double ratio_4_3 = 4.0/3.0;
        if(ratio == ratio_4_3)
        {
            MY_LOGD("4:3");
            stereo_ratio = ENUM_STEREO_RATIO::eRatio_4_3;
        }
        else
        {
            MY_LOGD("16:9");
            stereo_ratio = ENUM_STEREO_RATIO::eRatio_16_9;
        }
        MINT32 sensorProfile = 0;
        if(mSetting->mvInputData[0].mHalMetadataQueue.size() > 0)
        {
            if(!tryGetMetadata<MINT32>(&mSetting->mvInputData[0].mHalMetadataQueue[0], MTK_STEREO_FEATURE_SENSOR_PROFILE, sensorProfile))
            {
                MY_LOGE("Get sensor profile fail.");
                return ret;
            }
        }
        else
        {
            MY_LOGE("input hal metadata is empty");
            return ret;
        }
        depthMapSize = StereoSizeProvider::getInstance()->thirdPartyDepthmapSize((ENUM_STEREO_SENSOR_PROFILE)sensorProfile, stereo_ratio);
    }
    MUINT32 shiftIndex = mSetting->mvInputData.size() * 2 - 1;
    MY_LOGD("image buffer size: %d final capture size: %dx%d thumbsize: %dx%d depth: %dx%d shift: %d",
                    mSetting->mvInputData[0].mImageQueue.size(),
                    finalPictureSize.w, finalPictureSize.h,
                    finalThumbnailSize.w, finalThumbnailSize.h,
                    depthMapSize.w, depthMapSize.h,
                    shiftIndex);
    // pResultImageStreamInfo
    {
        pResultImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_YUV_JPEG")+std::to_string(mSetting->mRequestNo + shiftIndex)).c_str(),
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
                                        (std::string("Hal:Image:TP_YUV_THUMB")+std::to_string(mSetting->mRequestNo + shiftIndex)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_YV12,
                                        finalThumbnailSize,
                                        mSetting->mShotParam.mu4Transform);
    }
    // pCleanImageStreamInfo
    if(mbSupportJpeg00)
    {
        pCleanImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_YUV_JPEG0")+std::to_string(mSetting->mRequestNo + shiftIndex)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_TP_OUT00,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_NV21,
                                        finalPictureSize,
                                        0);
    }
    // pDepthMapSize
    if(mbSupportDepthOut)
    {
        pDepthMapImageStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:TP_Y_Depth")+std::to_string(mSetting->mRequestNo + shiftIndex)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_Y8,
                                        depthMapSize,
                                        0);
    }
    // jpgClean
    {
        pJpegStreamInfo =
                        createImageStreamInfo((std::string("App:Image:JpegEnc_TP")+std::to_string(mSetting->mRequestNo + shiftIndex)).c_str(),
                                              eSTREAMID_IMAGE_JPEG,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // jpgClean
    if(mbSupportJpeg00)
    {
        pCleanJpegStreamInfo =
                        createImageStreamInfo((std::string("App:Image:JpegEnc_TP0")+std::to_string(mSetting->mRequestNo + shiftIndex)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_TP_OUT00,
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
    ret = mpCallbackImageBufferManager->allocBuffer(pJpegStreamInfo, mpCallbackBufferHandler);
    mTotalImgCount++;
    ret |= mpCallbackImageBufferManager->allocBuffer(pResultImageStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(pThumbImageStreamInfo);
    if(mbSupportDepthOut)
    {
        ret |= mpCallbackImageBufferManager->allocBuffer(pDepthMapImageStreamInfo, mpCallbackBufferHandler);
        mTotalImgCount++;
    }
    if(mbSupportJpeg00)
    {
        ret |= mpCallbackImageBufferManager->allocBuffer(pCleanImageStreamInfo);
        ret |= mpCallbackImageBufferManager->allocBuffer(pCleanJpegStreamInfo, mpCallbackBufferHandler);
        mTotalImgCount++;
    }

    mpCallbackImageBufferManager->setTimestamp(mSetting->mRequestNo + shiftIndex, mSetting->miTimeStamp);

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
    MUINT32 inputDataSeq,
    MUINT32 index,
    sp<CallbackBufferPool> pPool_YUV00,
    sp<CallbackBufferPool> pPool_YUV01
)
{
    FUNCTION_SCOPE;
    // for default behavior, it will set full yuv and resize yuv to input pool.
    // default eSTREAMID_IMAGE_PIPE_YUV_00 is full yuv.
    // default eSTREAMID_IMAGE_PIPE_YUV_01 is resize yuv.
    // set notifier
    if(pPool_YUV00 != nullptr) pPool_YUV00->setNotifier(this);
    if(pPool_YUV01 != nullptr) pPool_YUV01->setNotifier(this);
#define SET_INPUT_BUFFER(streamId, bufferPool, name, inputSEQ, FRAME_INDEX) \
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
                MY_LOGD("[%d][%d] streamid(%#" PRIx64 ") pBuffer(%p) heap(%p)", inputSEQ, FRAME_INDEX, streamId, pBuffer.get(), pBuffer->getImageBufferHeap());\
                bufferPool->addBuffer(pBuffer);\
                sp<NSCam::v1::StreamBufferProvider> sbProvider = mImageStreamManager->getBufProviderByStreamId(streamId);\
                if(sbProvider != nullptr)\
                {\
                    sbProvider->doTimestampCallback(mSetting->mRequestNo + index, MFALSE, mSetting->miTimeStamp);\
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
    SET_INPUT_BUFFER(eSTREAMID_IMAGE_PIPE_YUV_00, pPool_YUV00, "PP_YUV_00", inputDataSeq, index);
    SET_INPUT_BUFFER(eSTREAMID_IMAGE_PIPE_YUV_01, pPool_YUV01, "PP_YUV_01", inputDataSeq, index);
#undef SET_INPUT_BUFFER
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
Dual3rdPostProcRequestBuilder::
setInputBuffer(
    android::DefaultKeyedVector<
                                            StreamId_T,
                                            sp<NSCam::v1::CallbackBufferPool> >&cbPool
)
{
    MY_LOGW("no implement");
    return UNKNOWN_ERROR;
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
    MY_LOGD("recive (%d) total(%d)", mImageCount, mTotalImgCount);
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
    MY_LOGD("req(%d) streamid(%x)", rRequestNo, rStreamId);
    return true;
}
