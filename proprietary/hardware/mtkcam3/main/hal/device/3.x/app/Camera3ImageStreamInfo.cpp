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

#include "Camera3ImageStreamInfo.h"
#include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_DEVICE);

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
static MUINT32 mapRotationToTransform(StreamRotation rotation)
{
    switch (rotation)
    {
    case StreamRotation::ROTATION_270:
        return HAL_TRANSFORM_ROT_90;

    case StreamRotation::ROTATION_180:
        return HAL_TRANSFORM_ROT_180;

    case StreamRotation::ROTATION_90:
        return HAL_TRANSFORM_ROT_270;

    case StreamRotation::ROTATION_0:
    default:
        return 0;
    }
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
Camera3ImageStreamInfo::
Camera3ImageStreamInfo(CreationInfo const& info)
    : IImageStreamInfo()
    , mInfo(info)
    , mSensorId(info.mSensorId)
    , mStreamId(info.mStream.v3_2.id)
    , mTransform(mapRotationToTransform(info.mStream.v3_2.rotation))
    , mHalUsage(info.mStream.v3_2.streamType==StreamType::OUTPUT ? info.mHalStream.v3_3.v3_2.producerUsage : info.mHalStream.v3_3.v3_2.consumerUsage)
    , mConsumerUsage(info.mStream.v3_2.streamType==StreamType::OUTPUT ? info.mStream.v3_2.usage : mHalUsage)
{
}


/******************************************************************************
 *
 ******************************************************************************/
Camera3ImageStreamInfo::
Camera3ImageStreamInfo(Camera3ImageStreamInfo const& other, int imgFormat, size_t stride)
{
    mInfo.mStreamName       = other.mInfo.mStreamName;
    mInfo.mvbufPlanes       = other.mInfo.mvbufPlanes;
    mInfo.mImgFormat        = other.mInfo.mImgFormat;
    mInfo.mOriImgFormat     = other.mInfo.mOriImgFormat;
    mInfo.mRealAllocFormat  = other.mInfo.mRealAllocFormat;
    mInfo.mStream           = other.mInfo.mStream;
    mInfo.mHalStream        = other.mInfo.mHalStream;
    mInfo.mSensorId         = other.mInfo.mSensorId;
    mInfo.mImageBufferInfo  = other.mInfo.mImageBufferInfo;
    mInfo.mSecureInfo.type  = other.mInfo.mSecureInfo.type;
    mSensorId           = other.mSensorId;
    mStreamId           = other.mStreamId;
    mTransform          = other.mTransform;
    mHalUsage           = other.mHalUsage;
    mConsumerUsage      = other.mConsumerUsage;

    if ((EImageFormat)imgFormat != eImgFmt_UNKNOWN) {
        mInfo.mImageBufferInfo.imgFormat = imgFormat;
    }
    if (stride > 0) {
        for (size_t i = 0 ; i < mInfo.mvbufPlanes.count ; i++) {
            mInfo.mImageBufferInfo.bufPlanes.planes[i].rowStrideInBytes = stride;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
V3_4::Stream const&
Camera3ImageStreamInfo::
getStream() const
{
    return mInfo.mStream;
}


/******************************************************************************
 *
 ******************************************************************************/
MSize
Camera3ImageStreamInfo::
getLandscapeSize() const
{
    if ( !(getImgSize().w >= getImgSize().h) ){
        MSize ret;
        ret.w = getImgSize().h;
        ret.h = getImgSize().w;
        return ret;
    }
    return getImgSize();
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT64
Camera3ImageStreamInfo::
getUsageForConsumer() const
{
    return mConsumerUsage;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT64
Camera3ImageStreamInfo::
getUsageForAllocator() const
{
    // bitwise combination of Hal usage and Hal's client usage.
    return (mHalUsage | mInfo.mStream.v3_2.usage);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
Camera3ImageStreamInfo::
getImgFormat() const
{
    return mInfo.mImageBufferInfo.imgFormat;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
Camera3ImageStreamInfo::
getOriImgFormat() const
{
    return mInfo.mOriImgFormat;
}


/******************************************************************************
 *
 ******************************************************************************/
MSize
Camera3ImageStreamInfo::
getImgSize() const
{
    return MSize(mInfo.mImageBufferInfo.imgWidth, mInfo.mImageBufferInfo.imgHeight);
}


/******************************************************************************
 *
 ******************************************************************************/
Camera3ImageStreamInfo::BufPlanes_t const&
Camera3ImageStreamInfo::
getBufPlanes() const
{
    return mInfo.mImageBufferInfo.bufPlanes;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
Camera3ImageStreamInfo::
getTransform() const
{
    return mTransform;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
Camera3ImageStreamInfo::
setTransform(MUINT32 transform)
{
    mTransform = transform;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
Camera3ImageStreamInfo::
getDataSpace() const
{
    return mInfo.mStream.v3_2.dataSpace;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
Camera3ImageStreamInfo::
isSecure() const
{
    return mInfo.mSecureInfo.type != SecType::mem_normal;
}

/******************************************************************************
 *
 ******************************************************************************/
SecureInfo const&
Camera3ImageStreamInfo::
getSecureInfo() const
{
    return mInfo.mSecureInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
Camera3ImageStreamInfo::
getStreamName() const
{
    return mInfo.mStreamName.string();
}


/******************************************************************************
 *
 ******************************************************************************/
StreamId_T
Camera3ImageStreamInfo::
getStreamId() const
{
    return mStreamId;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
Camera3ImageStreamInfo::
getStreamType() const
{
    return static_cast<MUINT32>(mInfo.mStream.v3_2.streamType);
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
Camera3ImageStreamInfo::
getMaxBufNum() const
{
    return mInfo.mHalStream.v3_3.v3_2.maxBuffers;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Camera3ImageStreamInfo::
setMaxBufNum(size_t count)
{
    mInfo.mHalStream.v3_3.v3_2.maxBuffers = count;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
Camera3ImageStreamInfo::
getMinInitBufNum() const
{
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
android::String8
Camera3ImageStreamInfo::
toString() const
{
    auto toString_BufPlanes = [](const BufPlanes_t& o) {
        android::String8 os;
        os += "[";
        for (size_t i = 0; i < o.count; i++) {
            auto const& plane = o.planes[i];
            os += String8::format(" %zu/%zu", plane.rowStrideInBytes, plane.sizeInBytes);
        }
        os += " ]";
        return os;
    };

    String8 s8RealFormat, s8RequestFormat, s8OverrideFormat;
    String8 s8HalUsage, s8HalClientUsage;
    String8 s8Dataspace;

    if ( auto pGrallocHelper = IGrallocHelper::singleton() ) {
        s8RealFormat    = pGrallocHelper->queryPixelFormatName(mInfo.mRealAllocFormat);
        s8RequestFormat = pGrallocHelper->queryPixelFormatName((int)mInfo.mStream.v3_2.format);
        s8OverrideFormat= pGrallocHelper->queryPixelFormatName((int)mInfo.mHalStream.v3_3.v3_2.overrideFormat);
        s8HalUsage      = pGrallocHelper->queryGrallocUsageName(mHalUsage);
        s8HalClientUsage= pGrallocHelper->queryGrallocUsageName(mInfo.mStream.v3_2.usage);
        s8Dataspace     = pGrallocHelper->queryDataspaceName(mInfo.mStream.v3_2.dataSpace);
    }

    android::String8 os;
    os += android::String8::format("%#" PRIx64 "", getStreamId());

    os += android::String8::format(" %4dx%-4d %s",
          mInfo.mImageBufferInfo.imgWidth, mInfo.mImageBufferInfo.imgHeight,
          (StreamType::OUTPUT==mInfo.mStream.v3_2.streamType ? "OUT" : "IN "));

    os += android::String8::format(" ImgFormat:%#x(%s)",
                                    getImgFormat(), NSCam::Utils::Format::queryImageFormatName(getImgFormat()).c_str());
    os += " BufPlanes(strides/sizeInBytes):";
    os += toString_BufPlanes(getBufPlanes());

    os += android::String8::format(" startOffset:%zu", getImageBufferInfo().startOffset);
    os += android::String8::format(" bufStep:%zu", getImageBufferInfo().bufStep);

    os += android::String8::format(" t:%d/r:%d maxBuffers:%d d/s:0x%08x(%s)",
                                    mTransform, mInfo.mStream.v3_2.rotation, mInfo.mHalStream.v3_3.v3_2.maxBuffers,
                                    mInfo.mStream.v3_2.dataSpace, s8Dataspace.c_str());

    os += android::String8::format(" %s", mInfo.mStreamName.string());


    os += android::String8::format(" AllocImgFormat:%#x(%s)",
                                    getAllocImgFormat(), NSCam::Utils::Format::queryImageFormatName(getAllocImgFormat()).c_str());
    os += " AllocBufPlanes(strides/sizeInBytes):";
    os += toString_BufPlanes(getAllocBufPlanes());

    os += android::String8::format(" Real:%#x(%s) Request:%#x(%s) Override:%#x(%s)",
                                    mInfo.mRealAllocFormat, s8RealFormat.c_str(),                              //real format
                                    mInfo.mStream.v3_2.format, s8RequestFormat.c_str(),                                 //request format
                                    mInfo.mHalStream.v3_3.v3_2.overrideFormat, s8OverrideFormat.c_str()                 //override format
                                  );

    os += android::String8::format(" Hal-Client-usage:%#" PRIx64 "(%s)",
                                    mInfo.mStream.v3_2.usage, s8HalClientUsage.c_str());                                //Hal-Client usage

    os += android::String8::format(" Hal-usage:%#" PRIx64 "(%s) HalStream::(consumer/producer)Usage:%#" PRIx64 "/%#" PRIx64 "",
                                    mHalUsage, s8HalUsage.c_str(),                                                      //Hal-appended usage
                                    mInfo.mHalStream.v3_3.v3_2.consumerUsage, mInfo.mHalStream.v3_3.v3_2.producerUsage);//consumer/producer usages

    os += android::String8::format(" %p phy:%d", this, mSensorId);

    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Camera3ImageStreamInfo::
dumpState(android::Printer& printer, uint32_t indent) const
{
    printer.printFormatLine("%-*c%s", indent, ' ', toString().c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
Camera3ImageStreamInfo::
getPhysicalCameraId() const
{
   return mSensorId;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT
Camera3ImageStreamInfo::
getAllocImgFormat() const
{
    return mInfo.mImgFormat;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageStreamInfo::BufPlanes_t const&
Camera3ImageStreamInfo::
getAllocBufPlanes() const
{
    return mInfo.mvbufPlanes;
}


/******************************************************************************
 *
 ******************************************************************************/
ImageBufferInfo const&
Camera3ImageStreamInfo::
getImageBufferInfo() const
{
    return mInfo.mImageBufferInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfo::
castFrom(IStreamInfo const* pInfo) -> Camera3ImageStreamInfo const*
{
    if ( pInfo != nullptr ) {
        auto anyHandle = pInfo->getMagicHandle();
        auto ppStreamInfo = std::any_cast<Camera3ImageStreamInfo const*>(&anyHandle);
        if ( ppStreamInfo && *ppStreamInfo ) {
            return *ppStreamInfo;
        }
        MY_LOGW("castFrom(%s) - ppStreamInfo:%p", pInfo->getStreamName(), ppStreamInfo);
    }
    return nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
Camera3ImageStreamInfoBuilder::
Camera3ImageStreamInfoBuilder(IImageStreamInfo const* info)
{
    auto pInfo = Camera3ImageStreamInfo::castFrom(info);
    if (pInfo) {
        mStreamInfo = new Camera3ImageStreamInfo(*pInfo);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
build() const -> android::sp<IImageStreamInfo>
{
    return mStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setStreamId(StreamId_T streamId) -> void
{
    mStreamInfo->mStreamId = streamId;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setAllocImgFormat(int format) -> void
{
    mStreamInfo->mInfo.mImgFormat = format;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setAllocBufPlanes(BufPlanesT&& bufPlanes) -> void
{
    mStreamInfo->mInfo.mvbufPlanes = std::move(bufPlanes);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setImageBufferInfo(ImageBufferInfo&& info) -> void
{
    mStreamInfo->mInfo.mImageBufferInfo = std::move(info);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setImgSize(MSize const& imgSize) -> void
{
    mStreamInfo->mInfo.mImageBufferInfo.imgWidth  = imgSize.w;
    mStreamInfo->mInfo.mImageBufferInfo.imgHeight = imgSize.h;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setImgFormat(int format) -> void
{
    mStreamInfo->mInfo.mImageBufferInfo.imgFormat = format;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
Camera3ImageStreamInfoBuilder::
setBufPlanes(BufPlanesT&& bufPlanes) -> void
{
    mStreamInfo->mInfo.mImageBufferInfo.bufPlanes = std::move(bufPlanes);
}
