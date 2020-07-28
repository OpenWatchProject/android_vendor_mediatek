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

#define LOG_TAG "mtkcam-fbm_producer"

#include "BufferProducer.h"
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::pipeline::NSPipelineContext;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#if 0
#define FUNC_START     MY_LOGD("%p:+", this)
#define FUNC_END       MY_LOGD("%p:-", this)
#else
#define FUNC_START
#define FUNC_END
#endif

BufferProducer::AutoCounter::
AutoCounter(std::atomic_int& c) : counter(c)
{
    counter++;
    //MY_LOGD("++ %d", counter.load());
}

BufferProducer::AutoCounter::
~AutoCounter()
{
    counter--;
    //MY_LOGD("-- %d", counter.load());
}
/******************************************************************************
 *
 ******************************************************************************/
BufferProducer::
BufferProducer(
    sp<IImageStreamInfo> pStreamInfo,
    CallbackFuncT const& callbackFunc,
    sp<BufferHeapPool> pPool
)
    : mLogLevel(::property_get_int32("vendor.debug.camera.log.pipeline.fbm.producer", 0))
    , mLock()
    , mCond()
    //
    , mpStreamInfo(pStreamInfo)
    , mpBufferPool(pPool)
    , mbInited(false)
    , mMaxNumberOfBuffers(pStreamInfo->getMaxBufNum())
    //
    , mBufMapLock()
    , mBufferMap()
    , mCallbackFunc(callbackFunc)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::onLastStrongRef(const void* /*id*/) -> void
{
    FUNC_START;
    clear();
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
allocatePoolBuffersLocked(
    AllocFuncT& allocateFunc
) -> android::status_t
{
    FUNC_START;
    //
    if (mpBufferPool == nullptr) {
        if (!allocateFunc) {
            MY_LOGE("Producer(%p) cannot get allocator of stream(%#" PRIx64 ":%s)",
                    this, mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName());
            return NO_MEMORY;
        }
        sp<BufferHeapPool> pPool = new BufferHeapPool(mpStreamInfo, allocateFunc);
        if (pPool == nullptr) {
            MY_LOGE("Producer(%p) cannot create BufferHeapPool of stream(%#" PRIx64 ":%s)",
                    this, mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName());
            return NO_MEMORY;
        }
        mpBufferPool = pPool;
    }
    //
    status_t err = OK;
    err = mpBufferPool->allocateBuffers(mpStreamInfo->getStreamName(),
                                       mMaxNumberOfBuffers,
                                       mpStreamInfo->getMinInitBufNum());

    if (err != OK) {
        MY_LOGE("Producer(%p) cannot allocate buffer from pool(%p)",
                this, mpBufferPool.get());
        return err;
    }
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
#define CHECK_POOL(pPool)                                           \
    do {                                                            \
        if (pPool == nullptr) {                                     \
            MY_LOGE("no BufferHeapPool of stream(%#" PRIx64 ":%s)", \
                    mpStreamInfo->getStreamId(),                    \
                    mpStreamInfo->getStreamName());                 \
            return UNKNOWN_ERROR;                                   \
        }                                                           \
    } while (0)


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
initProducer(
    char const* szCallerName,
    size_t maxNumberOfBuffers,
    size_t minNumberOfInitialCommittedBuffers,
    AllocFuncT&& allocateFunc
) -> status_t
{
    FUNC_START;
    MY_LOGD1("%s", szCallerName);
    setMaxBufferCount(maxNumberOfBuffers);
    if (minNumberOfInitialCommittedBuffers != mpStreamInfo->getMinInitBufNum())
        MY_LOGW("minNumberOfInitialCommittedBuffers(%zu) is differnt from mMinInitBufNum(%zu), use latter instead",
                minNumberOfInitialCommittedBuffers, mpStreamInfo->getMinInitBufNum());
    //
    status_t err = OK;
    {
        Mutex::Autolock _l(mLock);
        MY_LOGD1("inited(%s)", (mbInited == true) ? "TRUE" : "FALSE");
        if (!mbInited) {
            err = allocatePoolBuffersLocked(allocateFunc);
            mbInited = true;
        }
    }
    FUNC_END;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
setMaxBufferCount(
    size_t maxNumberOfBuffers
) -> status_t
{
    FUNC_START;
    {
        Mutex::Autolock _l(mBufMapLock);
        mMaxNumberOfBuffers = maxNumberOfBuffers;
        if (mpBufferPool.get())
            mpBufferPool->updateBufferCount(maxNumberOfBuffers);
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
setBufferHeapPool(
    sp<BufferHeapPool> pPool
) -> status_t
{
    FUNC_START;
    {
        Mutex::Autolock _l(mBufMapLock);
        if (!mBufferMap.isEmpty()) {
            MY_LOGE("stream(%#" PRIx64 ":%s) are in-use",
                    mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName());
            return UNKNOWN_ERROR;
        }
    }
    //
    {
        Mutex::Autolock _l(mLock);
        mpBufferPool = pPool;
        mbInited = false;
    }
    //
    FUNC_END;
    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
hasAcquiredMaxBuffer() -> bool
{
    FUNC_START;
    //
    Mutex::Autolock _l(mBufMapLock);
    MY_LOGD2("mapSize(%zu) - maxSize(%zu)", mBufferMap.size(), mMaxNumberOfBuffers);
    FUNC_END;
    return mBufferMap.size() == mMaxNumberOfBuffers;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
isInAcquire() -> bool
{
    FUNC_START;
    int cnt = mInAcquireCnt.load();
    MY_LOGD2("mInAcquireCnt(%d)", cnt);
    FUNC_END;
    return (cnt > 0);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
clear() -> void
{
    FUNC_START;
    MY_LOGD("clear Producer(%p) of stream(%#" PRIx64 ":%s) pool(%p)->size(%zu)",
            this, mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName(),
            mpBufferPool.get(), mBufferMap.size());
    //
    sp<IImageBufferHeap> rpHeap;
    {
        Mutex::Autolock _l(mBufMapLock);
        for (size_t i = 0; i < mBufferMap.size(); ++i) {
            rpHeap = mBufferMap.editValueAt(i);
            if (rpHeap != nullptr) {
                Mutex::Autolock _l(mLock);
                MY_LOGD1("release (%zu/%zu) - pHeap:%p", i, mBufferMap.size(), rpHeap.get());
                mpBufferPool->releaseToPool(mpStreamInfo->getStreamName(), rpHeap);
            }
        }
        mBufferMap.clear();
    }
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
dump() -> void
{
    FUNC_START;
    Mutex::Autolock _l(mBufMapLock);
    MY_LOGI("dump Producer(%p) stream(%#" PRIx64 ":%s) pool(%p)->size(%zu)",
            this, mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName(),
            mpBufferPool.get(), mBufferMap.size());
    String8 str = String8::format("Acquired Buffer: ");
    for (size_t i = 0; i < mBufferMap.size(); ++i) {
        str += String8::format("[%zu/%zu](%p:%p); ", i, mBufferMap.size(),
                               mBufferMap.keyAt(i), mBufferMap.valueAt(i).get());
    }
    MY_LOGI("%s", str.c_str());
    //
    mpBufferPool->dumpPool();
    FUNC_END;
}


/******************************************************************************
 *  IStreamBufferPool
 ******************************************************************************/
auto
BufferProducer::
acquireFromPool(
    char const* szCallerName __unused,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    nsecs_t nsTimeout
) -> status_t
{
    FUNC_START;
    auto pPool = [this](){ Mutex::Autolock _l(mLock); return mpBufferPool; }();
    sp<IImageBufferHeap> pHeap;
    {
        CHECK_POOL(pPool);
        auto startTime = std::chrono::system_clock::now();
        int retry_times = 1;
        AutoCounter _lac(mInAcquireCnt);
        while (OK != pPool->acquireFromPool(mpStreamInfo->getStreamName(), pHeap))
        {
            if (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - startTime) >
                std::chrono::nanoseconds(nsTimeout)) {  // timeout
                MY_LOGW("stream(%#" PRIx64 ":%s) cannot acquire from pool after timeout(%" PRId64 " ns).",
                        mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName(), nsTimeout);
                return NO_MEMORY;
            }

            if (hasAcquiredMaxBuffer()) {
                mCallbackFunc(mpStreamInfo->getStreamId(), (retry_times == 1));
                if (OK == pPool->acquireFromPool(mpStreamInfo->getStreamName(), pHeap)) {
                    MY_LOGD1("stream(%#" PRIx64 ":%s) acquire buffer after notify", mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName());
                    break;
                }
            }

            if (Mutex::Autolock _l(mLock); OK != mCond.waitRelative(mLock, 10000000)) {  // wait signal or 10ms to retry
                MY_LOGD1("stream(%#" PRIx64 ":%s) retry #%d", mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName(), retry_times);
                retry_times++;
            }
        }
    }

    rpStreamBuffer = new HalImageStreamBuffer(mpStreamInfo, this, pHeap);
    if (rpStreamBuffer == nullptr) {
        MY_LOGE("cannot new HalImageStreamBuffer. Something wrong...");
        return NO_MEMORY;
    }

    {
        Mutex::Autolock _l(mBufMapLock);
        mBufferMap.add(rpStreamBuffer.get(), pHeap);
        MY_LOGD2("acquire - buffer:%p pHeap:%p", rpStreamBuffer.get(), pHeap.get());
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferProducer::
releaseToPool(
    char const* szCallerName __unused,
    sp<IImageStreamBuffer> pStreamBuffer
) -> status_t
{
    FUNC_START;
    //
    sp<IImageBufferHeap> rpHeap;
    {
        Mutex::Autolock _l(mBufMapLock);
        rpHeap = mBufferMap.editValueFor(pStreamBuffer.get());
        mBufferMap.removeItem(pStreamBuffer.get());
    }
    //
    {
        Mutex::Autolock _l(mLock);
        CHECK_POOL(mpBufferPool);
        auto bBufStatus = static_cast<bool>(pStreamBuffer->getStatus() & STREAM_BUFFER_STATUS::ERROR);
        MY_LOGD2("release - bBufStatus:%d pHeap:%p", bBufStatus, rpHeap.get());
        mpBufferPool->releaseToPool(mpStreamInfo->getStreamName(), rpHeap);
        mCond.signal();
    }
    //
    FUNC_END;
    return OK;
}
