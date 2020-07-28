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

#define LOG_TAG "mtkcam-streaminfo"
//
#include <inttypes.h>
//
#include <type_traits>
//
#include <cutils/compiler.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_UTILITY);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace
{
template <std::size_t I>
using const_data_t = typename std::tuple_element_t<I, ImageStreamInfo::DataTupleT>::element_type;
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
template <class T>
constexpr size_t type2index()
{
    using DataTupleT = ImageStreamInfoBuilder::DataTupleT;
    if      constexpr (  std::is_same_v<T, std::tuple_element_t<0, DataTupleT>>  ) { return 0; }
    else if constexpr (  std::is_same_v<T, std::tuple_element_t<1, DataTupleT>>  ) { return 1; }
    else { static_assert(std::is_same_v<T, T*>, "Bad type"); }
}
}


/******************************************************************************
 *
 ******************************************************************************/
template <size_t I>
auto
ImageStreamInfoBuilder::
acquireClonedData() -> typename std::tuple_element_t<I, DataTupleT>
{
    auto& rptr = std::get<I>(mDataTuple);
    if ( mUseSharedData[I] ) {
        mUseSharedData[I] = false; // replace shared with clone
        auto temp = std::make_shared<const_data_t<I>>();
        if (temp == nullptr) {
            return nullptr;
        }
        *temp = *rptr;
        rptr = temp;
    }
    return rptr;
}


/******************************************************************************
 *
 ******************************************************************************/
ImageStreamInfoBuilder::
ImageStreamInfoBuilder(IImageStreamInfo const* pInfo)
{
    auto initializeData = [&, this](auto& dummy){
            using T = typename std::remove_reference<decltype(dummy)>::type;
            constexpr size_t I = type2index<T>();

            if ( pInfo != nullptr ) {
                if (auto pSharedStreamInfo = ImageStreamInfo::castFrom(pInfo)) {
                    // share with pInfo
                    mUseSharedData[I] = true;
                    std::get<I>(mDataTuple) = std::get<I>(pSharedStreamInfo->mDataTuple);
                }
                else {
                    // clone from pInfo
                    mUseSharedData[I] = false;
                    auto temp = std::get<I>(mDataTuple) = std::make_shared<typename T::element_type>();
                    MY_LOGF_IF(temp==nullptr, "Fail on make_shared");
                    ImageStreamInfo::cloneData(pInfo, *temp);
                }
            }
            else {
                    // clone with default values
                    mUseSharedData[I] = false;
                    auto temp = std::get<I>(mDataTuple) = std::make_shared<typename T::element_type>();
                    MY_LOGF_IF(temp==nullptr, "Fail on make_shared");
            }
        };

    initializeData(std::get<0>(mDataTuple));
    initializeData(std::get<1>(mDataTuple));
    if ( pInfo != nullptr ) {
        ImageStreamInfo::cloneNonSharedData(pInfo, mData);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ImageStreamInfoBuilder::
build() const -> android::sp<IImageStreamInfo>
{
    android::sp<ImageStreamInfo> pStreamInfo = new ImageStreamInfo();
    if (CC_UNLIKELY(pStreamInfo==nullptr)) {
        MY_LOGE("Fail on build a new ImageStreamInfo(%s)", std::get<0>(mDataTuple)->mStreamName.c_str());
        return nullptr;
    }

    pStreamInfo->mDataTuple = mDataTuple;
    pStreamInfo->mData = mData;
    return pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ImageStreamInfo::
castFrom(IStreamInfo const* pInfo) -> ImageStreamInfo const*
{
    if ( pInfo != nullptr ) {
        auto anyHandle = pInfo->getMagicHandle();
        auto ppStreamInfo = std::any_cast<ImageStreamInfo const*>(&anyHandle);
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
char const*
ImageStreamInfo::
getStreamName() const
{
    return  std::get<0>(mDataTuple)->mStreamName.c_str();
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamInfo::StreamId_T
ImageStreamInfo::
getStreamId() const
{
    return  std::get<0>(mDataTuple)->mStreamId;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
ImageStreamInfo::
getStreamType() const
{
    return  std::get<0>(mDataTuple)->mStreamType;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
ImageStreamInfo::
getMaxBufNum() const
{
    return  mData.mMaxBufNum;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ImageStreamInfo::
setMaxBufNum(size_t count)
{
    mData.mMaxBufNum = count;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
ImageStreamInfo::
getMinInitBufNum() const
{
    return  mData.mMinInitBufNum;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT64
ImageStreamInfo::
getUsageForConsumer() const
{
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT64
ImageStreamInfo::
getUsageForAllocator() const
{
    return std::get<1>(mDataTuple)->mUsageForAllocator;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ImageStreamInfo::
getImgFormat() const
{
    return mData.mImageBufferInfo.imgFormat;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT
ImageStreamInfo::
getOriImgFormat() const
{
    MY_LOGW("ImageStreamInfo::getOriImgFormat() not implement!");
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
MSize
ImageStreamInfo::
getImgSize() const
{
    return MSize(mData.mImageBufferInfo.imgWidth, mData.mImageBufferInfo.imgHeight);
}


/******************************************************************************
 *
 ******************************************************************************/
IImageStreamInfo::BufPlanes_t const&
ImageStreamInfo::
getBufPlanes() const
{
    return mData.mImageBufferInfo.bufPlanes;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
ImageStreamInfo::
getTransform() const
{
    return mData.mTransform;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ImageStreamInfo::
setTransform(MUINT32 transform)
{
    mData.mTransform = transform;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
ImageStreamInfo::
getDataSpace() const
{
    return mData.mDataSpace;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageStreamInfo::
isSecure() const
{
    return std::get<1>(mDataTuple)->mSecureInfo.type != SecType::mem_normal;
}


/******************************************************************************
 *
 ******************************************************************************/
SecureInfo const&
ImageStreamInfo::
getSecureInfo() const
{
    return std::get<1>(mDataTuple)->mSecureInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
android::String8
ImageStreamInfo::
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

    android::String8 os;
    os += android::String8::format("%#" PRIx64 "", getStreamId());

    // default/request settings
    os += android::String8::format(" %4dx%-4d", getImgSize().w, getImgSize().h);

    os += android::String8::format(" ImgFormat:%#x(%s)",
        getImgFormat(),
        NSCam::Utils::Format::queryImageFormatName(getImgFormat()).c_str());

    os += " BufPlanes(strides/sizeInBytes):";
    os += toString_BufPlanes(getBufPlanes());

    os += android::String8::format(" startOffset:%zu", getImageBufferInfo().startOffset);
    os += android::String8::format(" bufStep:%zu", getImageBufferInfo().bufStep);

    os += android::String8::format(" t:%d", getTransform());

    // alloc settings
    os += android::String8::format(" maxBufNum:%zu minInitBufNum:%zu",
        getMaxBufNum(), getMinInitBufNum());

    os += android::String8::format(" %s", getStreamName());

    os += android::String8::format(" AllocImgFormat:%#x(%s)",
        getAllocImgFormat(),
        NSCam::Utils::Format::queryImageFormatName(getAllocImgFormat()).c_str());

    os += " AllocBufPlanes(strides/sizeInBytes):";
    os += toString_BufPlanes(getAllocBufPlanes());

    os += android::String8(" Private{exist?") + std::to_string(NSCam::plugin::streaminfo::has_data(mData.mPrivData)).c_str()
        + " id:" + std::to_string(mData.mPrivDataId).c_str() + "}";

    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ImageStreamInfo::
getPhysicalCameraId() const
{
    // Hal Image: N/A
    return std::get<0>(mDataTuple)->mPhysicalCameraId;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ImageStreamInfo::
getAllocImgFormat() const
{
    return std::get<1>(mDataTuple)->mAllocImgFormat;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageStreamInfo::BufPlanes_t const&
ImageStreamInfo::
getAllocBufPlanes() const
{
    return std::get<1>(mDataTuple)->mvAllocBufPlanes;
}


/******************************************************************************
 *
 ******************************************************************************/
ImageBufferInfo const&
ImageStreamInfo::
getImageBufferInfo() const
{
    return mData.mImageBufferInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ImageStreamInfo::
queryPrivateData() const -> QueryPrivateData
{
    return {
            .privData   = mData.mPrivData,
            .privDataId = mData.mPrivDataId,
        };
}