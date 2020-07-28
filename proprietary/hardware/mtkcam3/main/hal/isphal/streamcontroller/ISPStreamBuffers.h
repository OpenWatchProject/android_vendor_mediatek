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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_ISPSTREAMBUFFERS_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_ISPSTREAMBUFFERS_H_
//
#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of isp image stream buffer.
 */
class ISPImageStreamBuffer
    : public Utils::TStreamBuffer<ISPImageStreamBuffer, IImageStreamBuffer>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.

    struct  ISPStreamBufferHandle
    {
        buffer_handle_t bufferHandle = nullptr;
        //
        ISPStreamBufferHandle(buffer_handle_t);
        ~ISPStreamBufferHandle();
    };

    struct  Allocator
    {
    public:     ////                            Data Members.
        android::sp<IStreamInfoT>               mpStreamInfo;

    public:     ////                            Operations.
                                                Allocator(
                                                    IStreamInfoT* pStreamInfo
                                                );
        virtual                                 ~Allocator() {}

    public:     ////                            Operator.
        virtual StreamBufferT*                  operator()(
                                                    IImageBufferHeap* pHeap,
                                                    buffer_handle_t pBufferHandle,
                                                    std::shared_ptr<ISPStreamBufferHandle> pISPBufferHandle,
                                                    IStreamInfoT* pStreamInfo = NULL
                                                );
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    android::sp<IImageBufferHeap>        mImageBufferHeap;

public:     ////                                Operations.
    std::shared_ptr<ISPStreamBufferHandle>      mpISPStreamBufferHandle;

                                                ISPImageStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    android::sp<IImageBufferHeap> pImageBufferHeap,
                                                    buffer_handle_t pBufferHandle,
                                                    std::shared_ptr<ISPStreamBufferHandle> pISPBufferHandle,
                                                    IUsersManager* pUsersManager = 0
                                                );

                                                ~ISPImageStreamBuffer();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Accessors.
    android::sp<IImageBufferHeap>               getImageBufferHeap() const;
    virtual MINT                                getAcquireFence()   const;
    virtual MVOID                               setAcquireFence(MINT fence);
    virtual MINT                                getReleaseFence()   const;
    virtual MVOID                               setReleaseFence(MINT fence);
public:     ////                                Attributes.
    virtual android::String8                    toString() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IUsersManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Operations.
    virtual ssize_t                             enqueUserGraph(
                                                    android::sp<IUserGraph> pUserGraph
                                                );

};


/**
 * An implementation of ISP image stream buffer for error return.
 */
class ISPErrorImageStreamBuffer
    : public ISPImageStreamBuffer
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.

    struct  Allocator
    {
    public:     ////                            Operator.
        virtual                                 ~Allocator() {}
        virtual StreamBufferT*                  operator()(
                                                    IStreamInfoT* pStreamInfo,
                                                    MINT dupReleaseFence = -1
                                                );
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    android::sp<NSCam::Utils::Sync::IFence>     mReleaseFence;

public:     ////                                Operations.
                                                ISPErrorImageStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    MINT dupReleaseFence
                                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Accessors.
    virtual MINT                                getAcquireFence()   const;
    virtual MVOID                               setAcquireFence(MINT fence);
    virtual MINT                                getReleaseFence()   const;
    virtual MVOID                               setReleaseFence(MINT fence);
};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of ISP metadata stream buffer.
 */
class ISPMetaStreamBuffer
    : public Utils::TStreamBuffer<ISPMetaStreamBuffer, IMetaStreamBuffer>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.

    struct  Allocator
    {

    public:     ////                            Data Members.
        android::sp<IStreamInfoT>               mpStreamInfo;

    public:     ////                            Operations.
                                                Allocator(
                                                    IStreamInfoT* pStreamInfo
                                                );
        virtual                                 ~Allocator() {}

    public:     ////                            Operator.
        virtual StreamBufferT*                  operator()();
        virtual StreamBufferT*                  operator()(NSCam::IMetadata const& metadata);
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    NSCam::IMetadata                            mMetadata;  // IBufferT-derived object.
    MBOOL                                       mRepeating; // Non-zero indicates repeating meta settings.

public:     ////                                Operations.
                                                ISPMetaStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    IUsersManager* pUsersManager = 0
                                                );
                                                ISPMetaStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    NSCam::IMetadata const& metadata,
                                                    IUsersManager* pUsersManager = 0
                                                );

public:     ////                                Attributes.
    virtual MVOID                               setRepeating(MBOOL const repeating);
    virtual MBOOL                               isRepeating() const;
    virtual android::String8                    toString() const;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_ISPSTREAMBUFFERS_H_

