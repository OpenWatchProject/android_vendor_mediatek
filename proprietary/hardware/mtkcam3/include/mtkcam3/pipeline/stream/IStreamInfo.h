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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_STREAM_ISTREAMINFO_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_STREAM_ISTREAMINFO_H_
//
#include <any>
#include <vector>
//
#include <utils/String8.h>
#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <mtkcam/def/ImageBufferInfo.h>
#include <mtkcam3/plugin/streaminfo/types.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * Type of Camera Stream Id.
 */
typedef int64_t                     StreamId_T;


/**
 * Camera stream type declaration.
 */
enum EStreamType
{
    //  Image Streams
    eSTREAMTYPE_IMAGE_OUT           = 0u,   //Sync. to StreamType::OUTPUT
    eSTREAMTYPE_IMAGE_IN            = 1u,   //Sync. to StreamType::INPUT
    eSTREAMTYPE_IMAGE_INOUT         = 2u,
    eSTREAMTYPE_IMAGE_END_OF_TYPES,

    //  Metadata Streams
    eSTREAMTYPE_META_OUT,           //DYNAMIC
    eSTREAMTYPE_META_IN,            //CONTROL
    eSTREAMTYPE_META_INOUT,         //CONTROL/DYNAMIC

};


/**
 * An interface of stream info.
 */
class IStreamInfo : public virtual android::RefBase
{
public:     ////                    Definitions.
    using StreamId_T = NSCam::v3::StreamId_T;
    using PrivateData_t = NSCam::plugin::streaminfo::PrivateDataT;

public:     ////    Attributes.
    virtual auto    getMagicHandle() const -> std::any { return {}; }

public:     ////                    Attributes.

    /**
     * A stream name.
     *
     * @remark This should be fixed and unchangable.
     */
    virtual char const*             getStreamName() const                   = 0;

    /**
     * A unique stream ID.
     *
     * @remark This should be fixed and unchangable.
     */
    virtual StreamId_T              getStreamId() const                     = 0;

    /**
     * A stream type of eSTREAMTYPE_xxx.
     *
     * @remark This should be fixed and unchangable.
     */
    virtual MUINT32                 getStreamType() const                   = 0;

    /**
     * The maximum number of buffers which may be used at the same time.
     *
     * @remark This should be fixed and unchangable.
     */
    virtual size_t                  getMaxBufNum() const                    = 0;

    /**
     * Set the maximum number of buffers which may be used at the same time.
     *
     * @param[in] count: the max. number of buffers.
     */
    virtual MVOID                   setMaxBufNum(size_t count)              = 0;

    /**
     * The minimum number of buffers which is suggested to allocate initially.
     *
     * @remark This should be fixed and unchangable.
     */
    virtual size_t                  getMinInitBufNum() const                = 0;


    /**********************************************************************
     * Private
     */

    /**
     * Query the private data
     *
     * @param privData The private data.
     * Use std::any_cast<T>( privData ) to type-safe access to the contained object.
     *
     * @param privDataId The id of private data.
     */
    struct QueryPrivateData
    {
        PrivateData_t               privData;
        uint32_t                    privDataId{0}; //0: bad id
    };
    virtual auto    queryPrivateData() const -> QueryPrivateData { return {}; }

public:     ////                    Operations.

    /**
     * dump to a string for debug.
     */
    virtual android::String8        toString() const                        = 0;

    /**
     * physical camera id.
     * Logical stream return empty, physical stream return physical camera id string.
     * -1 means normal stream
     *
     * @remark This should be fixed and unchangable.
     */
    virtual MINT                    getPhysicalCameraId() const             = 0;
};


/**
 * An interface of metadata stream info.
 */
class IMetaStreamInfo : public virtual IStreamInfo
{
public:     ////                    Definitions.
public:     ////                    Attributes.
    /**
     * Query the maximum number of tags or sections.
     */
};


/**
 * An interface of image stream info.
 */
class IImageStreamInfo : public virtual IStreamInfo
{
public:     ////                    Definitions.
    using BufPlane    = NSCam::BufPlane;
    using BufPlanes_t = NSCam::BufPlanes;
public:     ////                    Attributes.

    /**********************************************************************
     * Allocation Stage
     */

    /**
     * Usage for buffer consumer.
     *
     * @remark Both usages for allocator and consumer may have no intersection.
     */
    virtual MUINT64                 getUsageForConsumer() const             = 0;

    /**
     * Usage for buffer allocator.
     *
     * @remark Both usages for allocator and consumer may have no intersection.
     */
    virtual MUINT64                 getUsageForAllocator() const            = 0;

    /**
     * Image format at allocation stage
     */
    virtual MINT                    getAllocImgFormat() const               = 0;

    /**
     * A vector of buffer planes at allocation stage
     */
    virtual BufPlanes_t const&      getAllocBufPlanes() const               = 0;

    /**
     * Check if the stream is secure or not
     */
    virtual MBOOL                   isSecure() const                        = 0;

    /**
     * Get Image seucre/normal memory type
     */
    virtual SecureInfo const&       getSecureInfo() const                   = 0;


    /**********************************************************************
     * (Default) Request Stage
     */

    /**
     * Image format.
     */
    virtual MINT                    getImgFormat() const                    = 0;

    /**
     * Image format.
     */
    virtual MINT                    getOriImgFormat() const                 = 0;

    /**
     * Image resolution, in pixels, without cropping.
     *
     * @remark This should be fixed and unchangable.
     */
    virtual MSize                   getImgSize() const                      = 0;

    /**
     * A vector of buffer planes.
     */
    virtual BufPlanes_t const&      getBufPlanes() const                    = 0;

    /**
     * Get Image Buffer Info (for IImageBufferHeap::createImageBuffers_FromBlobHeap)
     */

    virtual ImageBufferInfo const&  getImageBufferInfo() const              = 0;

    /**
     * Get Data Space.
     */
    virtual MUINT32                 getDataSpace() const                    = 0;

    /**
     * Get Image transform type.
     */
    virtual MUINT32                 getTransform() const                    = 0;

    /**
     * Set Image transform type.
     */
    virtual MERROR                  setTransform(MUINT32 transform)         = 0;

};



/**
 * An interface of stream info set.
 */
class IStreamInfoSet
    : public virtual android::RefBase
{
public:     ////                            Definitions.
    template <class _IStreamInfoT_>
    struct  IMap
        : public virtual android::RefBase
    {
    public:     ////                        Operations.
        typedef _IStreamInfoT_              IStreamInfoT;

        virtual size_t                      size() const                        = 0;
        virtual ssize_t                     indexOfKey(StreamId_T id) const     = 0;
        virtual android::sp<IStreamInfoT>   valueFor(StreamId_T id) const       = 0;
        virtual android::sp<IStreamInfoT>   valueAt(size_t index) const         = 0;
    };

public:     ////                            Operations.
    virtual android::sp<IMap<IMetaStreamInfo> >
                                            getMetaInfoMap()    const           = 0;
    virtual size_t                          getMetaInfoNum()    const           = 0;
    virtual android::sp<IMetaStreamInfo>    getMetaInfoFor(StreamId_T id) const = 0;
    virtual android::sp<IMetaStreamInfo>    getMetaInfoAt(size_t index) const   = 0;

    virtual android::sp<IMap<IImageStreamInfo> >
                                            getImageInfoMap()   const           = 0;
    virtual size_t                          getImageInfoNum()   const           = 0;
    virtual android::sp<IImageStreamInfo>   getImageInfoFor(StreamId_T id) const= 0;
    virtual android::sp<IImageStreamInfo>   getImageInfoAt(size_t index) const  = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_STREAM_ISTREAMINFO_H_

