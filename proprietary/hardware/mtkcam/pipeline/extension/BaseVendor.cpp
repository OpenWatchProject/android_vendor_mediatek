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

#define LOG_TAG "MtkCam/vmgr_base"
//
#include "BaseVendor.h"
#include "utils/RequestFrame.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

// AOSP
#include <cutils/compiler.h> // CC_LIKELY, CC_UNLIKELY

// STL
#include <thread>

using namespace android;
using namespace NSCam::plugin;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
BaseVendor::
BaseVendor( MINT32  openId, MINT64  mode, String8 name )
    : mOpenId(openId)
    , mMode(mode)
    , mName(name)
{}

status_t
BaseVendor::
acquireWorkingBuffer(
    MINT32             openId,
    const MetaItem&    /*setting*/,
    /*output*/
    BufferItem&        buf
)
{
    CAM_TRACE_CALL();
    Mutex::Autolock _l(mLock);
    MINT32 direction = (buf.bufferStatus&BUFFER_IN)  ? eDIRECTION_IN :
                       (buf.bufferStatus&BUFFER_OUT) ? eDIRECTION_OUT: eDIRECTION_MAX_NUM;
    //
    typename List< OneTimeHeap >::iterator iter = mHeapList.begin();
    while( iter != mHeapList.end() )
    {
        if(((*iter).direction == direction)&&(((*iter).openId==openId)||((*iter).openId==OPENID_DONT_CARE))) {
            buf.bufferStatus = BUFFER_EMPTY;
            buf.heap         = (*iter).heap;
            buf.streamInfo   = (*iter).info;
            //
            mHeapList.erase(iter);
            MY_LOGD_IF(0 ,"acquireWorkingBuffer %p state:%d %zu", buf.heap.get(), buf.bufferStatus, mHeapList.size() );
            return OK;
        }
        iter++;
    }
    MY_LOGE_IF(mHeapList.size()>10, "buffer list too large.");
    //
    sp<WorkPool> pPool;
    if ( buf.bufferStatus&BUFFER_IN )  pPool = mpInPool;
    if ( buf.bufferStatus&BUFFER_OUT ) pPool = mpOutPool;
    if ( pPool.get() ) {
        buf.bufferStatus |= BUFFER_EMPTY;
        buf.heap         = pPool->acquireHeap();
        buf.streamInfo   = pPool->getStreamInfo();
        //
        MY_LOGD_IF(0 ,"acquireWorkingBuffer %p state:%d", buf.heap.get(), buf.bufferStatus );
        return OK;
    } else {
        MY_LOGW("pool not set.");
        return -ENODEV;
    }
}

status_t
BaseVendor::
releaseWorkingBuffer(
    MINT32             /*openId*/,
    const MetaItem&    /*setting*/,
    BufferItem&        buf
)
{
    MY_LOGD_IF(1 ,"releaseWorkingBuffer %p", buf.heap.get() );
    Mutex::Autolock _l(mLock);
    sp<WorkPool> pPool;
    if ( buf.bufferStatus&BUFFER_IN )  pPool = mpInPool;
    if ( buf.bufferStatus&BUFFER_OUT ) pPool = mpOutPool;
    if ( pPool.get() ) {
        return pPool->releaseHeap(buf.heap);
    }

    return OK;
}

status_t
BaseVendor::
prepareWorkingBuffer(MINT32 rDirection, sp<IImageStreamInfo> pInfo, MBOOL continuous)
{
    Mutex::Autolock _l(mLock);
    sp<WorkPool> pPool/* = (rDirection == eDIRECTION_IN) ? mpInPool : mpOutPool*/;
    if( pPool.get() ) pPool->clear();
    else pPool = new WorkPool();
    if(rDirection == eDIRECTION_IN) mpInPool = pPool;
    else mpOutPool = pPool;
    if ( eDIRECTION_IN_OUT == rDirection ) mpInPool = pPool;
    //
    if ( continuous ) return pPool->allocateFromBlob(pInfo);
    else return pPool->allocate(pInfo);

}

status_t
BaseVendor::
prepareOneWithoutPool(MINT32 rDirection, sp<IImageStreamInfo> pInfo, MBOOL continuous, MINT32 openId)
{
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mLock);
    sp<WorkPool> pPool = new WorkPool();

    if ( rDirection == eDIRECTION_IN_OUT ) {
        MY_LOGE("error direction.");
        return -ENODEV;
    }
    //
    OneTimeHeap heap;
    if ( continuous ) {
        heap.heap = pPool->allocateFromBlobOne(pInfo);
    }
    else {
        heap.heap = pPool->allocateOne(pInfo);
    }

    if( !heap.heap.get() ) {
        MY_LOGE("allocate fail.");
        return -ENODEV;
    }

    heap.direction = rDirection;
    heap.info      = pInfo;
    heap.openId    = openId;
    mHeapList.push_back(heap);
    MY_LOGD("add buffer into mHeapList (openId=%d)",openId);

    return OK;

}

status_t
BaseVendor::
setPool(MINT32 rDirection, sp<WorkPool> pPool)
{
    Mutex::Autolock _l(mLock);
    if ( eDIRECTION_IN_OUT == rDirection ) {
        mpInPool  = pPool;
        mpOutPool = pPool;
    } else if ( eDIRECTION_IN == rDirection ) {
        mpInPool  = pPool;
    } else if ( eDIRECTION_OUT == rDirection ) {
        mpOutPool = pPool;
    } else {
        MY_LOGE("Unknown direction.");
    }

    return OK;
}


void BaseVendor::drop(
        MINT32  const               openId,
        wp<IVendor::IDataCallback>  cb,
        const BufferParam&          bufParam,
        const MetaParam&            metaParam,
        bool                        bAsync /* = true */
        )
{
    if (bAsync) {
        const char* pszName = getName();
        std::thread([openId, cb, bufParam, metaParam, pszName]() {
                static const RequestFrame::StreamIdMap_Img  emptyMapImg;
                static const RequestFrame::StreamIdMap_Meta emptyMapMeta;
                std::shared_ptr<RequestFrame> f(new RequestFrame(
                    openId,
                    pszName,
                    bufParam,
                    metaParam,
                    cb,
                    emptyMapImg,
                    emptyMapMeta));
                // tricky: add a time delay to reduce ratio of threads race condition.
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                f = nullptr;
        }).detach();
    }
}


// ----------------------------------------------------------------------------
// Thread Slots
// ----------------------------------------------------------------------------
bool BaseVendor::setThreadShotsSize(size_t maxSize)
{
    return mFutureSlot.init(maxSize);
}


size_t BaseVendor::getThreadShotSize() const
{
    std::lock_guard<FutureSlot::OP_MUTEX_T> __l(mFutureSlot._mx);
    return mFutureSlot._size;
}


void BaseVendor::syncAllThread()
{
    mFutureSlot.clear();
}


void BaseVendor::asyncRun(std::function<void()>&& task)
{
    mFutureSlot.async_run(std::move(task));
}


BaseVendor::FutureSlot::FutureSlot()
    : _size(0)
    , _index(0)
{
}


bool BaseVendor::FutureSlot::init(size_t size)
{
    std::lock_guard<FutureSlot::OP_MUTEX_T> __l(_mx);

    if (CC_UNLIKELY(_size > 0)) {
        return false;
    }

    _size = size;
    _future_slots.resize(size);
    _future_slots_mx.resize(size);
    return true;
}


void BaseVendor::FutureSlot::async_run(std::function<void()>&& func)
{
    {
        std::lock_guard<FutureSlot::OP_MUTEX_T> __l(_mx);
        if (CC_UNLIKELY(_size <= 0)) {
            MY_LOGE("Run task w/ Thread Slots w/o initialization, default" \
                    "set the size of Thread Slots to 1");
            _size = 1;
        }
    }

    size_t idx = _index++ % _size;
    {
        std::lock_guard<std::mutex> __l(_future_slots_mx[idx]);

        auto& slot = _future_slots[idx];

        // join the primitive thread first
        if (slot.valid())
            slot.get();
        slot = std::async(std::launch::async, func);
    }
}


void BaseVendor::FutureSlot::clear()
{
    {
        std::lock_guard<FutureSlot::OP_MUTEX_T> __l(_mx);
        if (CC_UNLIKELY(_size <= 0))
            return;
    }

    for (size_t i = 0; i < _size; i++) {
        std::lock_guard<std::mutex> __l(_future_slots_mx[i]);
        auto& slot = _future_slots[i];

        if (slot.valid())
            slot.get();
        slot = std::future<void>();
    }
}
