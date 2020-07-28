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
#define LOG_TAG "MtkCam/ISPStreamBuffers"
//
#include <log/log.h>
#include <utils/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>
//
#include "ISPStreamBuffers.h"

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_HAL_SERVER);
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
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
ISPImageStreamBuffer::
ISPStreamBufferHandle::
ISPStreamBufferHandle(buffer_handle_t handle)
{
    bufferHandle = handle;
}

/******************************************************************************
 *
 ******************************************************************************/
ISPImageStreamBuffer::
ISPStreamBufferHandle::
~ISPStreamBufferHandle()
{
    MY_LOGD("free isp buffer handle: %p", bufferHandle);
    if  ( bufferHandle != nullptr )
    {
        if  ( auto helper = IGrallocHelper::singleton() ) {
            helper->freeBuffer(bufferHandle);
        }
    }
    bufferHandle = nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
ISPImageStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo
)
    : mpStreamInfo(pStreamInfo)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ISPImageStreamBuffer::StreamBufferT*
ISPImageStreamBuffer::
Allocator::
operator()(
    IImageBufferHeap* pHeap,
    buffer_handle_t pBufferHandle,
    std::shared_ptr<ISPStreamBufferHandle> pISPBufferHandle,
    IStreamInfoT* pStreamInfo)
{
    if  ( pHeap == 0 ) {
        MY_LOGE(
            "NULL IImageBufferHeap: %s",
            mpStreamInfo->getStreamName()
        );
        return nullptr;
    }
    //
    return new StreamBufferT(
        pStreamInfo ? pStreamInfo : mpStreamInfo,
        pHeap,
        pBufferHandle,
        pISPBufferHandle
    );
}


/******************************************************************************
 *
 ******************************************************************************/
ISPImageStreamBuffer::
ISPImageStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    sp<IImageBufferHeap> pImageBufferHeap,
    buffer_handle_t pBufferHandle,
    std::shared_ptr<ISPStreamBufferHandle> pISPBufferHandle,
    IUsersManager* pUsersManager
)
    : TStreamBufferT(pStreamInfo, pImageBufferHeap.get(), pUsersManager)
    , mImageBufferHeap(pImageBufferHeap)
    //
{
    mpISPStreamBufferHandle = pISPBufferHandle;
}

/******************************************************************************
 *
 ******************************************************************************/
ISPImageStreamBuffer::
~ISPImageStreamBuffer()
{
    mpISPStreamBufferHandle = nullptr;
    mImageBufferHeap = nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBufferHeap>
ISPImageStreamBuffer::
getImageBufferHeap() const
{
    Mutex::Autolock _l(mBufMutex);
    //
    return mImageBufferHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ISPImageStreamBuffer::
getAcquireFence()   const
{
    Mutex::Autolock _l(mBufMutex);
    //
    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ISPImageStreamBuffer::
setAcquireFence(MINT fence)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ISPImageStreamBuffer::
getReleaseFence()   const
{
    Mutex::Autolock _l(mBufMutex);
    //
    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ISPImageStreamBuffer::
setReleaseFence(MINT fence)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
ISPImageStreamBuffer::
enqueUserGraph(
    android::sp<IUserGraph> pUserGraph
)
{
    ssize_t const groupIndex = mUsersManager->enqueUserGraph(pUserGraph);
    if  ( 0 != groupIndex ) {
        return groupIndex;
    }
    //
    MBOOL found = MFALSE;
    sp<IUserGraph> const& pMyUserGraph = pUserGraph;
    for (size_t i = 0; i < pMyUserGraph->size(); i++) {
        if ( IUsersManager::Category::NONE == pMyUserGraph->getCategory(i) )
            continue;
        MY_LOGD_IF(0, "Node:%zu is a (%d: producer or consumer)!", i, pMyUserGraph->getCategory(i));
        //
        MERROR err = OK;
        if  ( ! found ) {
            found = MTRUE;
            MY_LOGD_IF(0, "0-indegree user:%zu streamid(%#" PRIx64 ") set acquire fence:%d", i, this->getStreamInfo()->getStreamId(), getAcquireFence());
            err = pMyUserGraph->setAcquireFence(i, getAcquireFence());
        }
        else {
            MINT fence = ::dup(getAcquireFence());
            MY_LOGW("another 0-indegree user:%zu; need dup acquire fence:%d->%d", i, getAcquireFence(), fence);
            err = pMyUserGraph->setAcquireFence(i, fence);
        }
        //
        if  ( OK != err ) {
            MY_LOGE("Fail to setAcquireFence(%zu, %d)", i, getAcquireFence());
        }
    }
    return groupIndex;
}


/******************************************************************************
 *
 ******************************************************************************/
android::String8
ISPImageStreamBuffer::
toString() const
{
    android::String8 os;

    os += android::String8::format("%#" PRIx64 "(%s)", getStreamId(), getName());

    if  (auto s = getStatus()) {
        os += android::String8::format(" status:%#x", s);
    }

    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
ISPErrorImageStreamBuffer::StreamBufferT*
ISPErrorImageStreamBuffer::
Allocator::
operator()(IStreamInfoT* pStreamInfo, MINT dupReleaseFence)
{
    return new ISPErrorImageStreamBuffer(
        pStreamInfo,
        dupReleaseFence
    );
}


/******************************************************************************
 *
 ******************************************************************************/
ISPErrorImageStreamBuffer::
ISPErrorImageStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    MINT dupReleaseFence
)
    : ISPImageStreamBuffer(pStreamInfo, nullptr, nullptr, nullptr)
    , mReleaseFence(NSCam::Utils::Sync::IFence::create(dupReleaseFence))
{
    markStatus(STREAM_BUFFER_STATUS::ERROR);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ISPErrorImageStreamBuffer::
getAcquireFence()   const
{
    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ISPErrorImageStreamBuffer::
setAcquireFence(MINT fence)
{
    MY_LOGE("ISPErrorImageStreamBuffer::setAcquireFence - not supported, fence:%d", fence);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ISPErrorImageStreamBuffer::
getReleaseFence()   const
{
    auto const dupReleaseFence = mReleaseFence->dup();
    MY_LOGD_IF(0 && -1!=dupReleaseFence, "ISPErrorImageStreamBuffer::getReleaseFence - fd:%d", dupReleaseFence);
    return dupReleaseFence;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ISPErrorImageStreamBuffer::
setReleaseFence(MINT fence)
{
    MY_LOGE("ISPErrorImageStreamBuffer::setReleaseFence - not supported, fence:%d", fence);
}


/******************************************************************************
 *
 ******************************************************************************/
ISPMetaStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo
)
    : mpStreamInfo(pStreamInfo)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ISPMetaStreamBuffer::StreamBufferT*
ISPMetaStreamBuffer::
Allocator::
operator()()
{
    return new StreamBufferT(
        mpStreamInfo
    );
}


/******************************************************************************
 *
 ******************************************************************************/
ISPMetaStreamBuffer::StreamBufferT*
ISPMetaStreamBuffer::
Allocator::
operator()(NSCam::IMetadata const& metadata)
{
    return new StreamBufferT(
        mpStreamInfo, metadata
    );
}


/******************************************************************************
 *
 ******************************************************************************/
ISPMetaStreamBuffer::
ISPMetaStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    IUsersManager* pUsersManager
)
    : TStreamBufferT(pStreamInfo, mMetadata, pUsersManager)
    , mMetadata()
    , mRepeating(MFALSE)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
ISPMetaStreamBuffer::
ISPMetaStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    NSCam::IMetadata const& metadata,
    IUsersManager* pUsersManager
)
    : TStreamBufferT(pStreamInfo, mMetadata, pUsersManager)
    , mMetadata(metadata)
    , mRepeating(MFALSE)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ISPMetaStreamBuffer::
setRepeating(MBOOL const repeating)
{
    android::Mutex::Autolock _l(mBufMutex);
    mRepeating = repeating;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ISPMetaStreamBuffer::
isRepeating() const
{
    android::Mutex::Autolock _l(mBufMutex);
    return mRepeating;
}


/******************************************************************************
 *
 ******************************************************************************/
android::String8
ISPMetaStreamBuffer::
toString() const
{
    android::String8 os;

    os += android::String8::format("%#" PRIx64 "(%s)", getStreamId(), getName());

    if (mBufMutex.timedLock(10000000 /* 10ms */) == OK) {
        os += android::String8::format(" #tags:%u", mMetadata.count());
        mBufMutex.unlock();
    }

    if  (isRepeating()) {
        os += " REPEAT";
    }

    if  (auto s = getStatus()) {
        os += android::String8::format(" status:%#x", s);
    }

    return os;
}

