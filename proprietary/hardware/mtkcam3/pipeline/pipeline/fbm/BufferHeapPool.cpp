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

#define LOG_TAG "mtkcam-fbm_pool"

#include "BufferHeapPool.h"
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
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


/******************************************************************************
 *
 ******************************************************************************/
BufferHeapPool::
BufferHeapPool(
    sp<IImageStreamInfo> pStreamInfo,
    AllocFuncT& allocateFunc
)
    : mLogLevel(::property_get_int32("vendor.debug.camera.log.pipeline.fbm.pool", 0))
    , mpStreamInfo(pStreamInfo)
    , mAllocateFunc(allocateFunc)
    , mPoolOpsLock()
    , mAvailLock()
    , mInUseLock()
    , mAvailableBuf()
    , mInUseBuf()
    //
    , mbInited(false)
    , mMaxBuffer(0)
    , mMinBuffer(0)
    , mvFutures()
    , mAvailableCond()
    , mExit(false)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
allocateBuffers(
    char const* szCallerName,
    size_t maxNumberOfBuffers,
    size_t minNumberOfInitialCommittedBuffers
) -> status_t
{
    FUNC_START;

    if (mpStreamInfo == nullptr) {
        MY_LOGE("[%s] No ImageStreamInfo.", szCallerName);
        return UNKNOWN_ERROR;
    }

    if (minNumberOfInitialCommittedBuffers > maxNumberOfBuffers) {
        MY_LOGE("[%s] min(%zu) > max(%zu)", szCallerName, minNumberOfInitialCommittedBuffers, maxNumberOfBuffers);
        return UNKNOWN_ERROR;
    }

    if (!mbInited) {
        mMaxBuffer = minNumberOfInitialCommittedBuffers;
        mMinBuffer = minNumberOfInitialCommittedBuffers;

        mIonDevice = IIonDeviceProvider::get()->makeIonDevice(poolName(), 0);
        if  (CC_UNLIKELY( mIonDevice == nullptr )) {
            MY_LOGE("fail to makeIonDevice in Pool");
        }

        for (int32_t i = 0; i < mMinBuffer; ++i) {
            sp<IImageBufferHeap> pHeap = mAllocateFunc(mIonDevice);
            if (pHeap == nullptr) {
                MY_LOGE("[%s] do_construct allocate buffer failed, mIonDevice(%p)", szCallerName, mIonDevice.get());
                continue;
            }
            {
                Mutex::Autolock _l(mAvailLock);
                mAvailableBuf.push_back(pHeap);
            }
        }
        mbInited = true;
    }

    MY_LOGD("[%s] max(%zu) min(%zu) mIonDevice(%p)",
        szCallerName, maxNumberOfBuffers, minNumberOfInitialCommittedBuffers, mIonDevice.get());

    return updateBufferCount(maxNumberOfBuffers);
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
onLastStrongRef(const void* /*id*/) -> void
{
    mExit = true;
    mAvailableCond.signal();
    //
    status_t result = OK;
    for (auto& fut : mvFutures) {
        result = fut.get();
    }
    mvFutures.clear();
    //
    {
        Mutex::Autolock _l(mAvailLock);
        mAvailableBuf.clear();
    }

    {
        Mutex::Autolock _ll(mInUseLock);
        if (mInUseBuf.size() > 0) {
            auto iter = mInUseBuf.begin();
            while (iter != mInUseBuf.end()) {
                MY_LOGW("[%s] buffer %p not return to pool.", poolName(), (*iter).get());
                iter++;
            }
        }

        mInUseBuf.clear();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
acquireFromPool(
    char const* szCallerName,
    sp<IImageBufferHeap>& rpBuffer
) -> status_t
{
    MY_LOGD1("[%s]", szCallerName);

    Mutex::Autolock _l(mPoolOpsLock);
    return  getBufferFromPool_l(
                szCallerName,
                rpBuffer
            );
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
getBufferFromPool_l(
    char const* szCallerName,
    sp<IImageBufferHeap>& rpBuffer
) -> status_t
{
    FUNC_START;

    Mutex::Autolock _l(mAvailLock);
    Mutex::Autolock _ll(mInUseLock);

    MY_LOGD1("[%s] mAvailableBuf:%zu mInUseBuf:%zu",
             szCallerName, mAvailableBuf.size(), mInUseBuf.size());
    if (!mAvailableBuf.empty()) {
        auto iter = mAvailableBuf.begin();
        mInUseBuf.push_back(*iter);
        rpBuffer = *iter;
        mAvailableBuf.erase(iter);
        //
        FUNC_END;
        return OK;
    }

    FUNC_END;
    return TIMED_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
releaseToPool(
    char const* szCallerName,
    sp<IImageBufferHeap> rpBuffer
) -> status_t
{
    MY_LOGD1("[%s]", szCallerName);

    Mutex::Autolock _l(mPoolOpsLock);
    return  returnBufferToPool_l(
                szCallerName,
                rpBuffer
            );
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
returnBufferToPool_l(
    char const* szCallerName,
    sp<IImageBufferHeap> rpBuffer
) -> status_t
{
    FUNC_START;

    Mutex::Autolock _l(mAvailLock);
    Mutex::Autolock _ll(mInUseLock);

    if (mInUseBuf.size() == 0) {
        MY_LOGE("mInUseBuf is empty! [%s] mAvailableBuf:%zu", szCallerName, mAvailableBuf.size());
        FUNC_END;
        return UNKNOWN_ERROR;
    }
    auto iter = mInUseBuf.begin();
    while (iter != mInUseBuf.end()) {
        if (rpBuffer == (*iter)) {
            mAvailableBuf.push_back(*iter);
            mInUseBuf.erase(iter);

            mAvailableCond.signal();

            MY_LOGD1("[%s] mAvailableBuf:%zu mInUseBuf:%zu return buf:%p",
                     szCallerName, mAvailableBuf.size(), mInUseBuf.size(), rpBuffer.get());
            FUNC_END;
            return OK;
        }
        iter++;
    }

    MY_LOGE("[%s] Cannot find buffer %p.", szCallerName, rpBuffer.get());

    FUNC_END;

    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
poolName() const -> char const*
{
    return (mpStreamInfo == 0) ? "Pool" : mpStreamInfo->getStreamName();
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
dumpPool() const -> void
{
    MY_LOGI("[%#" PRIx64 " %s] stream:%dx%d format:%#x transform:%d",
            mpStreamInfo->getStreamId(), mpStreamInfo->getStreamName(),
            mpStreamInfo->getImgSize().w, mpStreamInfo->getImgSize().h,
            mpStreamInfo->getImgFormat(), mpStreamInfo->getTransform());

    Mutex::Autolock _l(mAvailLock);
    Mutex::Autolock _ll(mInUseLock);

    MY_LOGI("logLevel:%d pIonDevice:%p Max:%d Min:%d future:%zu inUse:%zu available:%zu",
            mLogLevel, mIonDevice.get(), mMaxBuffer, mMinBuffer, mvFutures.size(),
            mInUseBuf.size(), mAvailableBuf.size());
    //
    {
        String8 str = String8::format("Available Buffer: ");
        auto iter = mAvailableBuf.cbegin();
        while (iter != mAvailableBuf.end()) {
            str += String8::format("%p; ", (*iter).get());
            iter++;
        }
        MY_LOGI("%s", str.string());
    }
    //
    {
        String8 str = String8::format("Inuse Buffer: ");
        auto iter = mInUseBuf.cbegin();
        while (iter != mInUseBuf.end()) {
            str += String8::format("%p; ", (*iter).get());
            iter++;
        }
        MY_LOGI("%s", str.string());
    }
}

/******************************************************************************
 *
 ******************************************************************************/
auto
BufferHeapPool::
updateBufferCount(
    int32_t rMaxBufferNumber
) -> status_t
{
    status_t result = OK;
    for( auto &fut : mvFutures ) {
        result = fut.get();
    }
    //
    mvFutures.clear();
    //
    mvFutures.push_back(
        std::async(std::launch::async,
            [ this ](int32_t rMaxBufferNumber) {
                MY_LOGD2("updateBufferCount: thread for allocate buffer of pool +");
                ::prctl(PR_SET_NAME, (unsigned long)"updateBufferCount", 0, 0, 0);
                //
                status_t err = OK;
                while (mMaxBuffer != rMaxBufferNumber && !mExit) {
                    MY_LOGD1("updateBufferCount: current max(%d) target(%d)", mMaxBuffer, rMaxBufferNumber);
                    // increase buffer
                    if (mMaxBuffer < rMaxBufferNumber) {
                        sp<IImageBufferHeap> pHeap = mAllocateFunc(mIonDevice);
                        if (pHeap == nullptr) {
                            MY_LOGE("do_construct allocate buffer failed");
                            continue;
                        }
                        {
                            Mutex::Autolock _l(mAvailLock);
                            mAvailableBuf.push_back(pHeap);
                            mMaxBuffer++;
                        }
                    }
                    // reduce buffer
                    else if (mMaxBuffer > rMaxBufferNumber) {
                        Mutex::Autolock _l(mAvailLock);
                        if (mAvailableBuf.empty()) {
                            mAvailableCond.wait(mAvailLock);
                        }
                        //
                        if (!mAvailableBuf.empty()) {
                            mAvailableBuf.erase(mAvailableBuf.begin());
                            mMaxBuffer--;
                        }
                    }
                }
                MY_LOGD2("updateBufferCount: thread for allocate buffer of pool -");
                MY_LOGD_IF((mMaxBuffer == rMaxBufferNumber),
                            "updateBufferCount: [%s] done (%d)",
                            mpStreamInfo->getStreamName(), rMaxBufferNumber);
                //
                return err;
            }, rMaxBufferNumber
        )
    );

    return OK;
}
