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

#ifndef _MTK_HARDWARE_MTKCAM_MIDDLEWARE_V3_PIPELINE_UTILS_EXTSTREAMBUFFERS_H_
#define _MTK_HARDWARE_MTKCAM_MIDDLEWARE_V3_PIPELINE_UTILS_EXTSTREAMBUFFERS_H_
//
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
#include <mtkcam/v3/utils/streambuf/IStreamBufferPool.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {

/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of external image stream buffer.
 */
class ExtImageStreamBuffer
    : public TStreamBuffer<ExtImageStreamBuffer, IImageStreamBuffer>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.
    typedef IStreamBufferPool<IImageStreamBuffer>
                                                IStreamBufferPoolT;

    struct  Allocator
    {
    public:     ////                            Definitions.
        typedef StreamBufferPool<
            IStreamBufferT,
            StreamBufferT,
            Allocator
        >                                       StreamBufferPoolT;

    public:     ////                            Data Members.
        android::sp<IStreamInfoT>               mpStreamInfo;
        IIonImageBufferHeap::AllocImgParam_t    mAllocImgParam;

    public:     ////                            Operations.
                                                Allocator(
                                                    IStreamInfoT* pStreamInfo,
                                                    IIonImageBufferHeap::AllocImgParam_t const& rAllocImgParam
                                                );

    public:     ////                            Operator.
        StreamBufferT*                          operator()(
                                                    IStreamBufferPoolT* pPool = NULL
                                                );
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    mutable android::Mutex                      mBufPoolMutex;
    android::wp<IStreamBufferPoolT> const       mBufPool;
    android::sp<IImageBufferHeap>               mImageBufferHeap;

public:     ////                                Operations.
                                                ExtImageStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    android::wp<IStreamBufferPoolT> pStreamBufPool,
                                                    android::sp<IImageBufferHeap>pImageBufferHeap
                                                );
                                                ~ExtImageStreamBuffer();
public:     ////                                Operations.
    virtual MVOID                               resetBuffer();
    virtual MVOID                               releaseBuffer();

    virtual MVOID                               setUsersManager(
                                                    android::sp<IUsersManager> value
                                                );


protected:  ////                                Operations.
    virtual android::sp<IStreamBufferPoolT>     tryGetBufferPool() const;

};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of external metadata stream buffer.
 */
class ExtMetaStreamBuffer
    : public Utils::TStreamBuffer<ExtMetaStreamBuffer, IMetaStreamBuffer>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.

    struct  Allocator : public virtual android::RefBase
    {
    public:     ////                            Data Members.
        android::sp<IStreamInfoT>               mpStreamInfo;

    public:     ////                            Operations.
                                                Allocator(
                                                    IStreamInfoT* pStreamInfo
                                                );

    public:     ////                            Operator.
        virtual StreamBufferT*                  operator()();
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    NSCam::IMetadata                            mMetadata;  // IBufferT-derived object.

public:     ////                                Operations.
                                                ExtMetaStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo
                                                );

public:     ////                                Operations.
    virtual MVOID                               resetBuffer();
    virtual MVOID                               releaseBuffer();

    virtual MVOID                               setUsersManager(
                                                    android::sp<IUsersManager> value
                                                );

public:     ////                                Attributes.
    // virtual MVOID                               setRepeating(MBOOL const repeating);
    virtual MBOOL                               isRepeating() const;

};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MIDDLEWARE_V3_PIPELINE_UTILS_EXTSTREAMBUFFERS_H_

