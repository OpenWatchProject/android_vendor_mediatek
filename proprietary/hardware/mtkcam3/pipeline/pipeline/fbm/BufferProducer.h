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

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_FBM_BUFFERPRODUCER_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_FBM_BUFFERPRODUCER_H_

#include <mtkcam3/pipeline/stream/IStreamBuffer.h>
#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>
//
#include "BufferHeapPool.h"


namespace NSCam::v3::pipeline::NSPipelineContext {

/******************************************************************************
 *
 ******************************************************************************/
class BufferProducer
    : public Utils::IStreamBufferPool<IImageStreamBuffer>
{
public:
    using AllocFuncT = BufferHeapPool::AllocFuncT;
    using CallbackFuncT = std::function<void(StreamId_T, bool)>;

                    BufferProducer(
                        android::sp<IImageStreamInfo> pStreamInfo,
                        CallbackFuncT const& callbackFunc,
                        android::sp<BufferHeapPool> pPool = nullptr
                    );

                    ~BufferProducer() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:   //// operations.
    auto            initProducer(
                        char const* szCallerName,
                        size_t maxNumberOfBuffers,
                        size_t minNumberOfInitialCommittedBuffers,
                        AllocFuncT&& allocateFunc
                    ) -> android::status_t;

    virtual auto    setMaxBufferCount(
                        size_t maxNumberOfBuffers
                    ) -> android::status_t;

    virtual auto    setBufferHeapPool(
                        android::sp<BufferHeapPool> pPool
                    ) -> android::status_t;

    virtual auto    hasAcquiredMaxBuffer() -> bool;

    virtual auto    isInAcquire() -> bool;

    virtual auto    clear() -> void;

    virtual auto    dump() -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    auto            allocatePoolBuffersLocked(
                        AllocFuncT& allocateFunc
                    ) -> android::status_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    auto            onLastStrongRef(const void* /*id*/) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    const int32_t                   mLogLevel;
    android::Mutex                  mLock;
    android::Condition              mCond;

    android::sp<IImageStreamInfo>   mpStreamInfo;
    android::sp<BufferHeapPool>     mpBufferPool;
    bool                            mbInited;
    size_t                          mMaxNumberOfBuffers;

protected:
    android::Mutex                  mBufMapLock;
    // map of StreamBuffer and ImageBufferHeap.
    // This module use BufferPool to acquire ImageBufferHeap and encapsulate into StreamBuffer.
    android::KeyedVector<IImageStreamBuffer*, android::sp<IImageBufferHeap> >
                                    mBufferMap;

    std::atomic_int                 mInAcquireCnt{0};
    struct AutoCounter {
        std::atomic_int& counter;
        AutoCounter(std::atomic_int& c);
        ~AutoCounter();
    };

    CallbackFuncT                   mCallbackFunc;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBufferPool Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Operations.

    auto            poolName() const -> char const* { return mpBufferPool->poolName(); }

    auto            dumpPool() const -> void { mpBufferPool->dumpPool(); }

    auto            initPool(  // [No need] use initProducer()
                        char const* szCallerName  __unused,
                        size_t maxNumberOfBuffers  __unused,
                        size_t minNumberOfInitialCommittedBuffers  __unused
                    ) -> android::status_t { return android::INVALID_OPERATION; }

    auto            uninitPool(  // [No need] use onLastStrongRef()
                        char const* szCallerName __unused
                    ) -> void { }

    auto            commitPool(  // [No need] use allocateBufferFromPoolLocked()
                        char const* szCallerName __unused
                    ) -> android::status_t { return android::INVALID_OPERATION; }

    auto            acquireFromPool(
                        char const* szCallerName,
                        android::sp<IImageStreamBuffer>& rpBuffer,
                        nsecs_t nsTimeout = 1000000000  // 1s
                    ) -> android::status_t;

    auto            releaseToPool(
                        char const* szCallerName,
                        android::sp<IImageStreamBuffer> pBuffer
                    ) -> android::status_t;

};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_FBM_BUFFERPRODUCER_H_

