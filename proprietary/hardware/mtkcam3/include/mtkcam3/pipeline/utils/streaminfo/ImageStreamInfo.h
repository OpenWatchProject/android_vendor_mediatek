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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_UTILS_STREAMINFO_IMAGESTREAMINFO_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_UTILS_STREAMINFO_IMAGESTREAMINFO_H_
//
#include <memory>
#include <string>
#include <tuple>
#include <utility>
//
#include <mtkcam3/pipeline/stream/IStreamInfo.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {

class ImageStreamInfoBuilder;


/**
 * image stream info.
 */
class ImageStreamInfo : public IImageStreamInfo
{
    friend ImageStreamInfoBuilder;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    /**
     * Data definitions.
     *
     * These data are allowed to be shared between the stream infos with the same stream id if not change.
     */
    struct SharedData0
    {
        std::string         mStreamName{"unknown"};
        StreamId_T          mStreamId{-1L};
        MUINT32             mStreamType{0};
        MINT                mPhysicalCameraId{-1};
    };

    struct SharedData1
    {
        MUINT               mUsageForAllocator{0};  /**< usage for buffer allocator. */
        MINT                mAllocImgFormat{0};
        BufPlanes_t         mvAllocBufPlanes;
        SecureInfo          mSecureInfo{SecureInfo()};
    };

    using DataTupleT =
            std::tuple<
                std::shared_ptr<SharedData0>,
                std::shared_ptr<SharedData1>
            >;
    DataTupleT      mDataTuple;


    /**
     * Data definitions.
     *
     * These data have a standalone copy of it for each stream info.
     */
    struct NonSharedData
    {
        PrivateData_t       mPrivData;
        uint32_t            mPrivDataId{0};

        size_t              mMaxBufNum{0};
        size_t              mMinInitBufNum{0};

        MUINT32             mTransform{0};
        MUINT32             mDataSpace{0};

        // Request Stage Info
        ImageBufferInfo     mImageBufferInfo;

    };
    NonSharedData   mData;

private:
    static auto     cloneData(IImageStreamInfo const* pInfo, SharedData0& data) -> void
                    {
                        if ( pInfo != nullptr ) {
                            data.mStreamName        = pInfo->getStreamName();
                            data.mStreamId          = pInfo->getStreamId();
                            data.mStreamType        = pInfo->getStreamType();
                            data.mPhysicalCameraId  = pInfo->getPhysicalCameraId();
                        }
                    }

    static auto     cloneData(IImageStreamInfo const* pInfo, SharedData1& data) -> void
                    {
                        if ( pInfo != nullptr ) {
                            data.mUsageForAllocator = pInfo->getUsageForAllocator();
                            data.mAllocImgFormat    = pInfo->getAllocImgFormat();
                            data.mvAllocBufPlanes   = pInfo->getAllocBufPlanes();
                            data.mSecureInfo        = pInfo->getSecureInfo();
                        }
                    }

    static auto     cloneNonSharedData(IImageStreamInfo const* pInfo, NonSharedData& data) -> void
                    {
                        if ( pInfo != nullptr ) {
                            data.mMaxBufNum     = pInfo->getMaxBufNum();
                            data.mMinInitBufNum = pInfo->getMinInitBufNum();
                            data.mTransform = pInfo->getTransform();
                            data.mDataSpace = pInfo->getDataSpace();
                            data.mImageBufferInfo = pInfo->getImageBufferInfo();
                            {
                                auto const& priv= pInfo->queryPrivateData();
                                data.mPrivData  = priv.privData;
                                data.mPrivDataId= priv.privDataId;
                            }
                        }
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ImageStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
    static  auto    castFrom(IStreamInfo const* pInfo) -> ImageStreamInfo const*;

public:
                    ImageStreamInfo() = default;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes.
    virtual auto    getMagicHandle() const -> std::any { return this; }

public:     ////                    Attributes.

    virtual char const*             getStreamName() const;

    virtual StreamId_T              getStreamId() const;

    virtual MUINT32                 getStreamType() const;

    virtual size_t                  getMaxBufNum() const;

    virtual MVOID                   setMaxBufNum(size_t count);

    virtual size_t                  getMinInitBufNum() const;

    virtual auto    queryPrivateData() const -> QueryPrivateData;

    virtual android::String8        toString() const override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.

    virtual MUINT64                 getUsageForConsumer() const;

    virtual MUINT64                 getUsageForAllocator() const;

    virtual MINT                    getImgFormat() const;

    virtual MINT                    getOriImgFormat() const;

    virtual MSize                   getImgSize() const;

    virtual BufPlanes_t const&      getBufPlanes() const;

    virtual MUINT32                 getTransform() const;

    virtual MERROR                  setTransform(MUINT32 transform);

    virtual MUINT32                 getDataSpace() const;

    virtual MBOOL                   isSecure() const override;

    virtual SecureInfo const&       getSecureInfo() const override;

    virtual MINT                    getPhysicalCameraId() const override;

    virtual BufPlanes_t const&      getAllocBufPlanes() const;

    virtual MINT                    getAllocImgFormat() const;

    virtual ImageBufferInfo const&  getImageBufferInfo() const;

};


/**
 * image stream info builder.
 */
class ImageStreamInfoBuilder
{
public:     ////    Definitions.
    using PrivateData_t = IStreamInfo::PrivateData_t;
    using DataTupleT = ImageStreamInfo::DataTupleT;

protected:  ////    Definitions.
    static constexpr size_t DataTupleNum = std::tuple_size<DataTupleT>::value;
    using NonSharedData = ImageStreamInfo::NonSharedData;

protected:  ////    Data Members.
    ImageStreamInfo::NonSharedData
                    mData;
    DataTupleT      mDataTuple;
    bool            mUseSharedData[DataTupleNum] = {0};

private:    ////    Operations.
    template <size_t I>
    auto            acquireClonedData() -> typename std::tuple_element_t<I, DataTupleT>;

public:     ////    Operations.
    virtual         ~ImageStreamInfoBuilder() = default;
                    ImageStreamInfoBuilder(IImageStreamInfo const* pInfo = nullptr);
    virtual auto    build() const -> android::sp<IImageStreamInfo>;

public:

    /**********************************************************************
     * Allocation Stage
     */

    virtual auto    setStreamName(std::string&& name) -> ImageStreamInfoBuilder& {
                        acquireClonedData<0>()->mStreamName = std::move(name);
                        return *this;
                    }

    virtual auto    setStreamId(StreamId_T streamId) -> ImageStreamInfoBuilder& {
                        acquireClonedData<0>()->mStreamId = streamId;
                        return *this;
                    }

    virtual auto    setStreamType(MUINT32 streamType) -> ImageStreamInfoBuilder& {
                        acquireClonedData<0>()->mStreamType = streamType;
                        return *this;
                    }

    virtual auto    setMaxBufNum(size_t maxBufNum) -> ImageStreamInfoBuilder& {
                        mData.mMaxBufNum = maxBufNum;
                        return *this;
                    }

    virtual auto    setMinInitBufNum(size_t minInitBufNum) -> ImageStreamInfoBuilder& {
                        mData.mMinInitBufNum = minInitBufNum;
                        return *this;
                    }

    virtual auto    setUsageForAllocator(MUINT usage) -> ImageStreamInfoBuilder& {
                        acquireClonedData<1>()->mUsageForAllocator = usage;
                        return *this;
                    }

    virtual auto    setAllocImgFormat(MINT format) -> ImageStreamInfoBuilder& {
                        acquireClonedData<1>()->mAllocImgFormat = format;
                        return *this;
                    }

    virtual auto    setAllocBufPlanes(IImageStreamInfo::BufPlanes_t const& bufPlanes) -> ImageStreamInfoBuilder& {
                        acquireClonedData<1>()->mvAllocBufPlanes = bufPlanes;
                        return *this;
                    }

    virtual auto    setAllocBufPlanes(IImageStreamInfo::BufPlanes_t&& bufPlanes) -> ImageStreamInfoBuilder& {
                        acquireClonedData<1>()->mvAllocBufPlanes = bufPlanes;
                        return *this;
                    }

    virtual auto    setSecureInfo(SecureInfo const& secureInfo) -> ImageStreamInfoBuilder& {
                        acquireClonedData<1>()->mSecureInfo = secureInfo;
                        return *this;
                    }

    /**********************************************************************
     * (Default) Request Stage
     */

    virtual auto    setImgFormat(MINT format) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.imgFormat = format;
                        return *this;
                    }
    virtual auto    getImgFormat() const -> MINT { return mData.mImageBufferInfo.imgFormat; }

    virtual auto    setImgSize(MSize const& imgSize) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.imgWidth  = imgSize.w;
                        mData.mImageBufferInfo.imgHeight = imgSize.h;
                        return *this;
                    }
    virtual auto    getImgSize() const -> MSize { return MSize(mData.mImageBufferInfo.imgWidth, mData.mImageBufferInfo.imgHeight); }

    virtual auto    setBufPlanes(IImageStreamInfo::BufPlanes_t const& bufPlanes) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.bufPlanes = bufPlanes;
                        return *this;
                    }

    virtual auto    setBufPlanes(IImageStreamInfo::BufPlanes_t&& bufPlanes) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.bufPlanes = bufPlanes;
                        return *this;
                    }

    virtual auto    setBufCount(size_t const count) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.count = count;
                        return *this;
                    }

    virtual auto    setStartOffset(size_t const startOffset) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.startOffset = startOffset;
                        return *this;
                    }

    virtual auto    setBufStep(size_t const bufStep) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo.bufStep = bufStep;
                        return *this;
                    }


    virtual auto    setImageBufferInfo(ImageBufferInfo const& imageBufferInfo) -> ImageStreamInfoBuilder& {
                        mData.mImageBufferInfo = imageBufferInfo;
                        return *this;
                    }

    virtual auto    setDataSpace(MUINT32 dataSpace) -> ImageStreamInfoBuilder& {
                        mData.mDataSpace = dataSpace;
                        return *this;
                    }

    virtual auto    setTransform(MUINT32 transform) -> ImageStreamInfoBuilder& {
                        mData.mTransform = transform;
                        return *this;
                    }


    /**********************************************************************
     * Private
     */

    struct SetPrivateData
    {
        PrivateData_t   privData;
        uint32_t        privDataId{0};
    };
    virtual auto    setPrivateData(SetPrivateData const& arg) -> ImageStreamInfoBuilder& {
                        mData.mPrivData   = arg.privData;
                        mData.mPrivDataId = arg.privDataId;
                        return *this;
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_UTILS_STREAMINFO_IMAGESTREAMINFO_H_

