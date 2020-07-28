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

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_FBM_BUFFERHEAPPOOL_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_FBM_BUFFERHEAPPOOL_H_

#include <functional>
#include <future>
#include <vector>
#include <list>
//
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
//
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonDevice.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>

namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class BufferHeapPool
    : public android::RefBase
{
public:
    using AllocFuncT = std::function<IImageBufferHeap*(const std::shared_ptr<IIonDevice>&)>;

                    BufferHeapPool(
                        android::sp<IImageStreamInfo> pStreamInfo,
                        AllocFuncT& allocateFunc
                    );

                    ~BufferHeapPool() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:   //// operations.

    /**
     * Try to acquire a buffer from the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[out] rpBuffer: a reference to a newly acquired buffer.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual auto    acquireFromPool(
                        char const*                     szCallerName,
                        android::sp<IImageBufferHeap>&  rpBuffer
                    ) -> android::status_t;

    /**
     * Release a buffer to the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[in] pBuffer: a buffer to release.
     *
     * @return
     *      0 indicates success; non-zero indicates an error code.
     */
    virtual auto    releaseToPool(
                        char const*                     szCallerName,
                        android::sp<IImageBufferHeap>   pBuffer
                    ) -> android::status_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    //// debug
    /**
     * Pool name.
     */
    virtual auto    poolName() const -> char const*;

    /**
     * Dump information for debug.
     */
    virtual auto    dumpPool() const -> void;

public:    //// set buffer source

    /**
     * allocate buffer for pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[in] maxNumberOfBuffers: maximum number of buffers which can be
     *  allocated from this pool.
     *
     * @param[in] minNumberOfInitialCommittedBuffers: minimum number of buffers
     *  which are initially committed.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual auto    allocateBuffers(
                        char const* szCallerName,
                        size_t maxNumberOfBuffers,
                        size_t minNumberOfInitialCommittedBuffers
                    ) -> android::status_t;

    virtual auto    updateBufferCount(
                        int32_t rMaxBufferNumber
                    ) -> android::status_t;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   //// acquire buffer
    auto            getBufferFromPool_l(
                        char const* szCallerName,
                        android::sp<IImageBufferHeap>& rpBuffer
                    ) -> android::status_t;

protected:
    auto            returnBufferToPool_l(
                        char const* szCallerName,
                        android::sp<IImageBufferHeap> rpBuffer
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
    const int32_t                       mLogLevel;

protected:
    android::sp<IImageStreamInfo>       mpStreamInfo;
    AllocFuncT                          mAllocateFunc;

protected:
    mutable android::Mutex              mPoolOpsLock;
    mutable android::Mutex              mAvailLock;
    mutable android::Mutex              mInUseLock;
    std::list< android::sp<IImageBufferHeap> >
                                        mAvailableBuf;
    std::list< android::sp<IImageBufferHeap> >
                                        mInUseBuf;

protected:
    bool                                mbInited;
    int32_t                             mMaxBuffer;
    int32_t                             mMinBuffer;
    std::shared_ptr<IIonDevice>         mIonDevice = nullptr;
    std::vector< std::future< android::status_t > >
                                        mvFutures;
    android::Condition                  mAvailableCond;
    bool                                mExit;
    //
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_FBM_BUFFERHEAPPOOL_H_

