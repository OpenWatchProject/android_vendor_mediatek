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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_APP_CAM3IMAGESTREAMINFO_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_APP_CAM3IMAGESTREAMINFO_H_
//
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <HidlCameraDevice.h>
//
#include <vector>
#include <string>
//
#include <utils/Printer.h>
#include <utils/String8.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * camera3 image stream info.
 */
class Camera3ImageStreamInfo : public IImageStreamInfo
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    struct  CreationInfo
    {
        android::String8            mStreamName;
        BufPlanes_t                 mvbufPlanes;

        /**
         *  image format in reality.
         *
         *  If HalStream::overrideFormat is NOT set by HAL,
         *  then the request format == HalStream::overrideFormat;
         *  if not, the real format == HalStream::overrideFormat.
         */
        MINT                        mImgFormat = 0;

        /**
         *  original image format.
         *
         *  Always keep original image format of each stream id
         *  to reduce duplicate setting in static metadata.
         */
        MINT                        mOriImgFormat = 0;

        /** android/hardware/camera/device/3.x/types.h */
        V3_4::Stream                mStream;
        V3_4::HalStream             mHalStream;
    };

protected:  ////                    Data Members.

    CreationInfo                    mInfo;
    StreamId_T                      mStreamId;              /** Stream::id, HalStream::id */
    MSize                           mImgSize;               /** image size, in pixels.
                                                             *  Stream::width, Stream::height
                                                             */
    MUINT32                         mTransform = 0;         /** like Stream::rotation */

    /**
     * hardware/interfaces/camera/device/3.4/types.hal
     *
     *
     * Stream::usage
     *
     * The gralloc usage flags for this stream, as needed by the consumer of
     * the stream.
     *
     * The usage flags from the producer and the consumer must be combined
     * together and then passed to the platform gralloc HAL module for
     * allocating the gralloc buffers for each stream.
     *
     * The HAL may use these consumer flags to decide stream configuration. For
     * streamType INPUT, the value of this field is always 0. For all streams
     * passed via configureStreams(), the HAL must set its own
     * additional usage flags in its output HalStreamConfiguration.
     *
     * The usage flag for an output stream may be bitwise combination of usage
     * flags for multiple consumers, for the purpose of sharing one camera
     * stream between those consumers. The HAL must fail configureStreams call
     * with ILLEGAL_ARGUMENT if the combined flags cannot be supported due to
     * imcompatible buffer format, dataSpace, or other hardware limitations.
     *
     *
     * HalStream::producerUsage / HalStream::consumerUsage
     *
     * The gralloc usage flags for this stream, as needed by the HAL.
     *
     * For output streams, these are the HAL's producer usage flags. For input
     * streams, these are the HAL's consumer usage flags. The usage flags from
     * the producer and the consumer must be combined together and then passed
     * to the platform graphics allocator HAL for allocating the gralloc buffers
     * for each stream.
     *
     * If the stream's type is INPUT, then producerUsage must be 0, and
     * consumerUsage must be set. For other types, producerUsage must be set,
     * and consumerUsage must be 0.
     */
    MUINT64                         mHalUsage = 0;
    MUINT64                         mConsumerUsage = 0; // shouldn't be defined

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    Camera3ImageStreamInfo(CreationInfo const& info);

public:     ////                    Attributes.

    virtual V3_4::Stream const&     getStream() const;
    virtual MSize                   getLandscapeSize() const;
    virtual void                    dumpState(android::Printer& printer, uint32_t indent = 0) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.

    /**
     * Usage for buffer consumer.
     */
    virtual MUINT64                 getUsageForConsumer() const;

    /**
     * Usage for buffer allocator.
     *
     * @remark It must equal to the gralloc usage passed to the platform
     *  graphics allocator HAL for allocating the gralloc buffers.
     */
    virtual MUINT64                 getUsageForAllocator() const;

    virtual MINT                    getImgFormat() const;
    virtual MINT                    getOriImgFormat() const;
    virtual MSize                   getImgSize() const;
    virtual BufPlanes_t const&      getBufPlanes() const;
    virtual MUINT32                 getTransform() const;
    virtual MERROR                  setTransform(MUINT32 transform);
    virtual MINT32                  getDataSpace() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual char const*             getStreamName() const;
    virtual StreamId_T              getStreamId() const;
    virtual MUINT32                 getStreamType() const;
    virtual size_t                  getMaxBufNum() const;
    virtual MVOID                   setMaxBufNum(size_t count);
    virtual size_t                  getMinInitBufNum() const;
    virtual MBOOL                   getSecureInfo() const override;
    virtual MERROR                  updateStreamInfo(
                                        const android::sp<IImageStreamInfo>& pStreamInfo
                                    ){ return android::UNKNOWN_ERROR; }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_APP_CAM3IMAGESTREAMINFO_H_

