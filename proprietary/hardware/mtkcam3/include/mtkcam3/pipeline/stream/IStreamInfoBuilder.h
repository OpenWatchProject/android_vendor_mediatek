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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_STREAM_ISTREAMINFOBUILDER_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_STREAM_ISTREAMINFOBUILDER_H_
//
#include <memory>

#include <utils/StrongPointer.h>

#include "IStreamInfo.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3 {


/**
 * An interface of stream info builder.
 */
class IImageStreamInfoBuilder
{
public:
    using StreamId_T = NSCam::v3::StreamId_T;
    using BufPlane   = NSCam::BufPlane;
    using BufPlanesT = NSCam::BufPlanes;

public:
    virtual         ~IImageStreamInfoBuilder() = default;

public:     ////    Operations.
    virtual auto    build() const -> android::sp<IImageStreamInfo>          = 0;

    /**********************************************************************
     * Common info.
     */

    /**
     * Set the stream name.
     *
     * @param name The stream name.
     */
    virtual void    setStreamName(std::string&& name)                       = 0;

    /**
     * Set the stream id.
     *
     * @param streamId The stream id.
     */
    virtual void    setStreamId(StreamId_T streamId)                        = 0;

    /**
     * Set the stream type.
     *
     * @param streamType The stream type.
     */
    virtual void    setStreamType(uint32_t streamType)                      = 0;

    /**
     * Set the secure info.
     *
     * @param secureInfo The secure info.
     */
    virtual void    setSecureInfo(SecureInfo const& secureInfo)             = 0;


    /**********************************************************************
     * Allocation info.
     */

    /**
     * Set the maximum number of buffers which may be used at the same time.
     *
     * @param maxBufNum The max. number of buffers.
     */
    virtual void    setMaxBufNum(size_t maxBufNum)                          = 0;

    /**
     * Set the minimum number of buffers which is suggested to be allocated
     * initially.
     *
     * @param minInitBufNum The min. number of buffers suggested to be allocated
     *  initially..
     */
    virtual void    setMinInitBufNum(size_t minInitBufNum)                  = 0;

    /**
     * Set the usage for the allocator.
     *
     * @param usage The usage for the allocator.
     */
    virtual void    setUsageForAllocator(uint_t usage)                      = 0;

    /**
     * Set the allocation format.
     *
     * @param format The allocation format to set.
     *  It should be eImgFmt_BLOB by default.
     */
    virtual void    setAllocImgFormat(int format)                           = 0;

    /**
     * Set the allocation buffer planes.
     *
     * @param bufPlanes The allocation buffer planes to set.
     */
    virtual void    setAllocBufPlanes(BufPlanesT&& bufPlanes)               = 0;


    /**********************************************************************
     * (Default) Request info.
     */

    /**
     * Set the image buffer information.
     *
     * @param info The image buffer information to set.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setImageBufferInfo(ImageBufferInfo&& info)              = 0;

    /**
     * Set the image resolution, in pixels.
     *
     * @param imgSize The image size to set.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setImgSize(MSize const& imgSize)                        = 0;

    /**
     * Set the format (for default requests).
     *
     * @param foramt The format to set.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setImgFormat(int format)                                = 0;

    /**
     * Set the buffer planes (for default requests).
     *
     * @param bufPlanes The buffer planes to set.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setBufPlanes(BufPlanesT&& bufPlanes)                    = 0;

    /**
     * Set the number of buffers.
     *
     * @param count The number of buffers.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setBufCount(size_t count)                               = 0;

    /**
     * Set the buffer step, in bytes.
     *
     * @param step The buffer step, in bytes.
     *  This field is the distance in bytes from one buffer to the next.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setBufStep(size_t step)                                 = 0;

    /**
     * Set the start offset, in bytes, of the first buffer.
     *
     * @param offset The start offset, in bytes, of the first buffer.
     *
     * @remark This API is related to ImageBufferInfo.
     */
    virtual void    setBufStartOffset(size_t offset)                        = 0;

    /**
     * Set the data space.
     *
     * @param dataspace The data space.
     */
    virtual void    setDataSpace(uint32_t dataspace)                        = 0;

    /**
     * Set the transform.
     *
     * @param transform The transform, defined in
     *  mtkcam/include/mtkcam/def/ImageFormat.h.
     */
    virtual void    setTransform(uint32_t transform)                        = 0;


};


/**
 * An interface of a factory to make stream info builder(s).
 */
class IStreamInfoBuilderFactory
{
public:

    virtual         ~IStreamInfoBuilderFactory() = default;

    /**
     * Create an IImageStreamInfoBuilder instance which is initialized in a
     * default way.
     *
     * @return A newly-created IImageStreamInfoBuilder instance.
     */
    virtual auto    makeImageStreamInfoBuilder() const
                        -> std::shared_ptr<IImageStreamInfoBuilder>         = 0;

    /**
     * Create an IImageStreamInfoBuilder instance which is initialized with an
     * existing IImageStreamInfo instance.
     *
     * @param info: An existing IImageStreamInfo instance.
     *
     * @return A newly-created IImageStreamInfoBuilder instance.
     */
    virtual auto    makeImageStreamInfoBuilder(IImageStreamInfo const* info) const
                        -> std::shared_ptr<IImageStreamInfoBuilder>         = 0;


};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_STREAM_ISTREAMINFOBUILDER_H_

