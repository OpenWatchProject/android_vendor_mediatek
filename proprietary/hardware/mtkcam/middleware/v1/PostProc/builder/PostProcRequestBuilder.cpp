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

#define LOG_TAG "PostProcRequestBuilder"

// Standard C header file
#include <sys/prctl.h>
#include <sys/resource.h>
#include <chrono>
// Android system/core header file
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
// Module header file
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Local header file
#include "../inc/IPostProcRequestBuilder.h"
#include "MulitFramePostProcRequestBuilder.h"

#if defined (MTKCAM_STEREO_SUPPORT)
#include "Dual3rdPostProcRequestBuilder.h"
#endif
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");
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

using namespace android;
using namespace android::NSPostProc;
using namespace NSCam;
using namespace NSCam::v1::NSLegacyPipeline;
#define WRITE_PERMISSION 0660
/******************************************************************************
 *
 ******************************************************************************/
#define UNUSED(expr) do { (void)(expr); } while (0)
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
IPostProcRequestBuilder::
IPostProcRequestBuilder(
    std::string const& name,
    android::sp<ILegacyPipeline> pipeline,
    android::sp<ImageStreamManager> imageStreamManager,
    sp<PostProcRequestSetting> setting
) : mName(name),
    mSetting(setting),
    mPipeline(pipeline),
    mImageStreamManager(imageStreamManager),
    mFirstRequestNum(setting->mRequestNo)
{
    FUNCTION_SCOPE;
    MY_LOGD("ctr (%s:%d)", mName.c_str(), mSetting->mProcessType);
}
/******************************************************************************
 *
 ******************************************************************************/
IPostProcRequestBuilder::
~IPostProcRequestBuilder(
)
{
    FUNCTION_SCOPE;
    MY_LOGD("dct (%s:%d)", mName.c_str(), mSetting->mProcessType);
}
/******************************************************************************
 *
 ******************************************************************************/
PostProcRequestBuilderBase::
PostProcRequestBuilderBase(
    std::string const& name,
    android::sp<ILegacyPipeline> pipeline,
    android::sp<ImageStreamManager> imageStreamManager,
    sp<PostProcRequestSetting> setting
) : IPostProcRequestBuilder(name, pipeline, imageStreamManager, setting)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.di.stereo.dumpcapturedata", cLogLevel, "0");
    MINT32 value = ::atoi(cLogLevel);
    if(value > 0)
    {
        mbEnableDump = MTRUE;
        // regenerate filename if need.
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        msFilename = std::string("/sdcard/camera_bgs/")+std::to_string(millis);
        MY_LOGD("filename(%s)", msFilename.c_str());
        if ( !NSCam::Utils::makePath(msFilename.c_str(), WRITE_PERMISSION)) {
            MY_LOGW("makePath returns error");
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
PostProcRequestBuilderBase::
setRequestSetting( android::sp<PostProcRequestSetting> setting)
{
    MY_LOGD("setRequestSettings: old#:%d, new:%d", mSetting->mRequestNo, setting->mRequestNo);
    mSetting = setting;
}
/******************************************************************************
 *
 ******************************************************************************/
PostProcRequestBuilderBase::
~PostProcRequestBuilderBase()
{

}
/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PostProcRequestBuilderBase::
getPrecedingData(
    MUINT32 inputDataSeq,
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata,
    BufferList& mvDstStream,
    ThirdPartyType& type
)
{
    setDefaultMetadata(inputDataSeq, index, appMetadata, halMetadata);
    mvDstStream = mvPrecedingDstStreams;
    return onGetPrecedingData(index, appMetadata, halMetadata, type);
}
/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PostProcRequestBuilderBase::
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
    UNUSED(type);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PostProcRequestBuilderBase::
getWorkingData(
    MUINT32 inputDataSeq,
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata,
    BufferList& mvDstStream,
    ThirdPartyType& type
)
{
    setDefaultMetadata(inputDataSeq, index, appMetadata, halMetadata);
    mvDstStream = mvWorkingDstStreams;
    return onGetWorkingData(index, appMetadata, halMetadata, type);
}
/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PostProcRequestBuilderBase::
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
    UNUSED(type);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
android::status_t
PostProcRequestBuilderBase::
setDefaultMetadata(
    MUINT32 inputDataSeq,
    MUINT32 index,
    IMetadata& appMetadata,
    IMetadata& halMetadata
)
{
    if(inputDataSeq >= mSetting->mvInputData.size())
    {
        MY_LOGE("invaild operation. inputdata count(%d)", inputDataSeq);
        return UNKNOWN_ERROR;
    }
    if(index >= mSetting->mvInputData[inputDataSeq].mAppMetadataQueue.size())
    {
        MY_LOGE("invaild operation. data(%d), index(%d) app listcount(%ld)", inputDataSeq, index, mSetting->mvInputData[inputDataSeq].mAppMetadataQueue.size());
        return UNKNOWN_ERROR;
    }
    if(index >= mSetting->mvInputData[inputDataSeq].mHalMetadataQueue.size())
    {
        MY_LOGE("invaild operation. data(%d), index(%d) hal listcount(%ld)", inputDataSeq, index, mSetting->mvInputData[inputDataSeq].mHalMetadataQueue.size());
        return UNKNOWN_ERROR;
    }
    appMetadata = mSetting->mvInputData[inputDataSeq].mAppMetadataQueue[index];
    halMetadata = mSetting->mvInputData[inputDataSeq].mHalMetadataQueue[index];
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
PostProcRequestBuilderBase::
createImageStreamInfo(
    char const* streamName,
    StreamId_T streamId,
    MUINT32 streamType,
    size_t maxBufNum,
    size_t minInitBufNum,
    MUINT usageForAllocator,
    MINT imgFormat,
    MSize const& imgSize,
    MUINT32 transform
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
    #define addBufPlane(planes, height, stride)                                      \
            do{                                                                      \
                size_t _height = (size_t)(height);                                   \
                size_t _stride = (size_t)(stride);                                   \
                IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
                planes.push_back(bufPlane);                                          \
            }while(0)
        switch( imgFormat ) {
            case eImgFmt_YV12:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                break;
            case eImgFmt_NV21:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
                break;
            case eImgFmt_RAW16:
            case eImgFmt_YUY2:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
                break;
            case eImgFmt_Y8:
            case eImgFmt_STA_BYTE:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                break;
            case eImgFmt_RGBA8888:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w<<2);
                break;
            case eImgFmt_BLOB:
                /*
                add 328448 for image size
                standard exif: 1280 bytes
                4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
                max thumbnail size: 64K bytes
                */
                addBufPlane(bufPlanes , 1              , (imgSize.w * imgSize.h * 12 / 10) + 328448); //328448 = 64K+1280+65408*4
                break;
            default:
                MY_LOGE("format not support yet %d", imgFormat);
                break;
        }
    #undef  addBufPlane

        sp<ImageStreamInfo>
            pStreamInfo = new ImageStreamInfo(
                    streamName,
                    streamId,
                    streamType,
                    maxBufNum,
                    minInitBufNum,
                    usageForAllocator,
                    imgFormat,
                    imgSize,
                    bufPlanes,
                    transform);

        if( pStreamInfo == NULL ) {
            MY_LOGE("create ImageStream failed, %s, %#" PRIxPTR ,
                    streamName, streamId);
        }

        return pStreamInfo;
}
/******************************************************************************
 *
 ******************************************************************************/
void
PostProcRequestBuilderBase::
dumpImageBuffer(sp<IImageBuffer> pBuffer, std::string filename)
{
    if(mbEnableDump && pBuffer != nullptr)
    {
        pBuffer->saveToFile((msFilename+std::string("/")+filename).c_str());
    }
}
/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPostProcRequestBuilder>
PostProcRequestBuilderFactory::
createInstance(
    android::sp<ILegacyPipeline> pipeline,
    android::sp<ImageStreamManager> imageStreamManager,
    sp<PostProcRequestSetting> setting
)
{
    switch(setting->mProcessType)
    {
    case PostProcessorType::THIRDPARTY_MFNR:
        return new MulitFramePostProcRequestBuilder("MFNR", pipeline, imageStreamManager, setting);
    case PostProcessorType::THIRDPARTY_HDR:
        return new MulitFramePostProcRequestBuilder("HDR", pipeline, imageStreamManager, setting);
#if defined (MTKCAM_STEREO_SUPPORT)
    case PostProcessorType::THIRDPARTY_BOKEH:
        return new Dual3rdPostProcRequestBuilder("TP_BOKEH", pipeline, imageStreamManager, setting);
    case PostProcessorType::THIRDPARTY_DCMF:
        return new Dual3rdPostProcRequestBuilder("TP_DCMF", pipeline, imageStreamManager, setting);
#endif
    default:
        MY_LOGE("not supported type(%d)", (MUINT32)setting->mProcessType);
        return nullptr;
    }
}
