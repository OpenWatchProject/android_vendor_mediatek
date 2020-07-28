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

#define LOG_TAG "MtkCam/CallbackBufferPoolImp"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/utils/std/Format.h>
//
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>

using namespace android;
using namespace android::MtkCamUtils;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::Utils;

using namespace NSCam::v3::Utils;
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

#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
#define BUFFERPOOL_NAME       ("Cam@v1BufferPool")
#define BUFFERPOOL_POLICY     (SCHED_OTHER)
#define BUFFERPOOL_PRIORITY   (0)


class CallbackBufferPool::CallbackBufferPoolImp
    : public virtual RefBase
    , public Thread
{
public:
                                        CallbackBufferPoolImp(
                                            sp<IImageStreamInfo> pStreamInfo,
                                            MBOOL isSecurity,
                                            SecType secType
                                        )
                                            : mpStreamInfo(pStreamInfo),
                                            mSecurity(isSecurity),
                                            mSecType(secType)
                                        {
                                            mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
                                            if ( mLogLevel == 0 ) {
                                                mLogLevel = ::property_get_int32("debug.camera.log.BPImp", 0);
                                            }
                                        }

                                        ~CallbackBufferPoolImp() {};
public:
    MERROR                              getBufferFromPool(
                                            char const*           szCallerName,
                                            sp<IImageBufferHeap>& rpBuffer,
                                            MUINT32&              rTransform
                                        );

    MERROR                              returnBufferToPool(
                                            char const* szCallerName,
                                            sp<IImageBufferHeap> rpBuffer
                                        );

    MERROR                              allocateBuffer(
                                            char const* szCallerName
                                        );

    MERROR                              addAllocateBuffer(
                                            size_t const addNumberOfBuffers
                                        );

    MERROR                              addBuffer(
                                             android::sp<IImageBuffer>& pBuffer
                                        );

    MERROR                              removeBufferFromPool(
                                             char const* szCallerName,
                                             android::sp<IImageBufferHeap>& pBuffer
                                        );

    MERROR                              do_construct(
                                            sp<IImageBufferHeap>& pImageBufferHeap
                                        );

    android::sp<IImageStreamInfo>       getStreamInfo();

    MERROR                              updateStreamInfo(
                                            const sp<IImageStreamInfo>& pStreamInfo
                                        );

    MVOID                               flush();

    MVOID                               setForceNoNeedReturnBuffer( MBOOL bFlag );

    MVOID                               setExtraParam( MBOOL contiguousPlanes);

    MBOOL                               isForceNoNeedReturn(){return mbForceNoNeedReturnBuffer;};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                onLastStrongRef(const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();

public:
    mutable Mutex                       mPoolLock;
    List< sp<IImageBufferHeap> >        mAvailableBuf;
    List< sp<IImageBufferHeap> >        mInUseBuf;
    mutable Condition                   mCondWaitBuf;

public:
    MINT32                              mLogLevel;
    MINT32                              mMaxBuffer;
    MINT32                              mMinBuffer;

public:
    sp<IImageStreamInfo>                mpStreamInfo;
    wp<Notifier>                        mpNotify;

private:
    MBOOL                               mbForceNoNeedReturnBuffer = MFALSE;

    IIonImageBufferHeap::AllocExtraParam
                                        mExtParam;
    MBOOL                               mSecurity;
    SecType                             mSecType;
};


/******************************************************************************
 *
 ******************************************************************************/
CallbackBufferPool::
~CallbackBufferPool()
{}

/******************************************************************************
 *
 ******************************************************************************/
CallbackBufferPool::
CallbackBufferPool(
    sp<IImageStreamInfo> pStreamInfo,
    MBOOL isSecurity,
    SecType secType
)
    : mImp( new CallbackBufferPool::CallbackBufferPoolImp(pStreamInfo, isSecurity, secType) )
    , mbNoNeedReturnBuffer( false )
{}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::
setNotifier( sp<Notifier> pNotify )
{
    mImp->mpNotify = pNotify;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
acquireFromPool(
    char const*           szCallerName,
    MINT32                rRequestNo,
    sp<IImageBufferHeap>& rpBuffer,
    MUINT32&              rTransform
)
{
    MY_LOGD_IF( 0 , "%s", szCallerName);

    if ( mImp->getBufferFromPool(
              szCallerName,
              rpBuffer,
              rTransform ) == OK )
    {
        sp<Notifier> pNotify = mImp->mpNotify.promote();
        if ( pNotify != 0 )
            pNotify->onBufferAcquired( rRequestNo, mImp->mpStreamInfo->getStreamId() );
        return OK;
    }

    return UNKNOWN_ERROR;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
getBufferFromPool(
    char const*           szCallerName,
    sp<IImageBufferHeap>& rpBuffer,
    MUINT32&              rTransform
)
{
    FUNC_START;

    Mutex::Autolock _l(mPoolLock);

    MY_LOGD_IF( mLogLevel >= 1, "[%s] mAvailableBuf:%d mInUseBuf:%d",
            szCallerName, mAvailableBuf.size(), mInUseBuf.size());

    if(mAvailableBuf.empty())
    {
        std::chrono::nanoseconds maxWaitTimeNS(std::chrono::seconds(1));
        MY_LOGD("No mAvailableBuf, start waiting %lld ns", maxWaitTimeNS.count());
        status_t status = mCondWaitBuf.waitRelative(mPoolLock, maxWaitTimeNS.count());
        if( status != OK )
        {
            MY_LOGW("waiting timeout");
        }
    }

    if( !mAvailableBuf.empty() )
    {
        typename List< sp<IImageBufferHeap> >::iterator iter = mAvailableBuf.begin();
        mInUseBuf.push_back(*iter);
        rpBuffer = *iter;
        rTransform = mpStreamInfo->getTransform();
        mAvailableBuf.erase(iter);
        //
        MY_LOGD_IF(
            mLogLevel >= 1,
            "[%s] mAvailableBuf:%d mInUseBuf:%d bufHeap:%p", szCallerName, mAvailableBuf.size(), mInUseBuf.size(), rpBuffer.get()
        );
        //
        FUNC_END;
        return OK;
    }
    MY_LOGW("mAvailableBuf is empty");

    FUNC_END;
    return TIMED_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
releaseToPool(
    char const*          szCallerName,
    MINT32               rRequestNo,
    sp<IImageBufferHeap> rpBuffer,
    MUINT64              /*rTimeStamp*/,
    bool                 rErrorResult
)
{
    MY_LOGD_IF( 0, "%s", szCallerName);

    FUNC_START;
    MBOOL ret = MFALSE;
    sp<Notifier> pNotify = mImp->mpNotify.promote();
    if ( pNotify != 0)
    {
        MY_LOGD_IF( 1 , "Ready to onBufferReturned, streamid(%#" PRIx64 "), bufHeap:%p", mImp->mpStreamInfo->getStreamId(), rpBuffer.get());
        pNotify->onBufferReturned(rRequestNo,
                                   mImp->mpStreamInfo->getStreamId(),
                                   rErrorResult,
                                   rpBuffer
                                   );
        //
        if( mbNoNeedReturnBuffer || mImp->isForceNoNeedReturn() )
        {
            ret = mImp->removeBufferFromPool(
                          szCallerName,
                          rpBuffer
                          );
        }
        else
        {
            ret = returnBufferToPool(
                          szCallerName,
                          rpBuffer
                          );
        }
        FUNC_END;
        return ret;
    }
    MY_LOGD_IF( 1 , "No pNotify Case");
    //
    ret = returnBufferToPool(
                  szCallerName,
                  rpBuffer
                  );
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
returnBufferToPool(
    char const* szCallerName,
    sp<IImageBufferHeap> rpBuffer
)
{
    return mImp->returnBufferToPool(
                  szCallerName,
                  rpBuffer
                  );
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
returnBufferToPool(
    char const* szCallerName,
    sp<IImageBufferHeap> rpBuffer
)
{
    FUNC_START;
    Mutex::Autolock _l(mPoolLock);

    typename List< sp<IImageBufferHeap> >::iterator iter = mInUseBuf.begin();
    while( iter != mInUseBuf.end() ) {
        if ( rpBuffer == (*iter) ) {
            mAvailableBuf.push_back(*iter);
            mInUseBuf.erase(iter);

            MY_LOGD_IF(
                mLogLevel >= 1,
                "[%s] mAvailableBuf:%d mInUseBuf:%d bufHeap:%p", szCallerName, mAvailableBuf.size(), mInUseBuf.size(), rpBuffer.get()
            );

            mCondWaitBuf.broadcast();

            FUNC_END;
            return OK;
        }
        iter++;
    }

    MY_LOGE("[%s] Cannot find buffer %p.", szCallerName, rpBuffer.get());

    mCondWaitBuf.signal();
    FUNC_END;

    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
CallbackBufferPool::
poolName() const
{
    return "CallbackBufferPool";
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::
dumpPool() const
{
#warning "TODO"
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
allocateBuffer(
    char const* szCallerName,
    size_t /*maxNumberOfBuffers*/,
    size_t /*minNumberOfInitialCommittedBuffers*/
)
{
    mbNoNeedReturnBuffer = false;
    return mImp->allocateBuffer(szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
allocateBuffer(
    char const* /*szCallerName*/
)
{
    FUNC_START;

    if ( mpStreamInfo == 0 ) {
        MY_LOGE("No ImageStreamInfo.");
        return UNKNOWN_ERROR;
    }

    mMaxBuffer = mpStreamInfo->getMaxBufNum();
    mMinBuffer = mpStreamInfo->getMinInitBufNum();

    if ( mMinBuffer > mMaxBuffer) {
        MY_LOGE("mMinBuffer:%d > mMaxBuffer:%d", mMinBuffer, mMaxBuffer);
        return UNKNOWN_ERROR;
    }

    for ( MINT32 i = 0; i < mMinBuffer; ++i ) {
        sp<IImageBufferHeap> pHeap;
        if( do_construct(pHeap) == NO_MEMORY ) {
            MY_LOGE("do_construct allocate buffer failed");
            continue;
        }
        {
            Mutex::Autolock _l(mPoolLock);
            MY_LOGD("add bufHeap: %p", pHeap.get());
            mAvailableBuf.push_back(pHeap);
        }
    }

    status_t status = run(BUFFERPOOL_NAME);
    if  ( OK != status ) {
        MY_LOGE("Fail to run the thread - status:%d(%s)", status, ::strerror(-status));
        return UNKNOWN_ERROR;
    }

    MY_LOGD("(min,max):(%d,%d)", mMinBuffer, mMaxBuffer);

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
addAllocateBuffer(
    size_t const addNumberOfBuffers
)
{
    return mImp->addAllocateBuffer(addNumberOfBuffers);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
addAllocateBuffer(
    size_t const addNumberOfBuffers
)
{
    FUNC_START;
    MERROR ret = OK;

    if ( mpStreamInfo == 0 ) {
        MY_LOGE("No ImageStreamInfo.");
        ret = UNKNOWN_ERROR;
    }
    else if ( addNumberOfBuffers <= 0 ) {
        MY_LOGE("Invaild add number of buffers: %zu", addNumberOfBuffers);
        ret = UNKNOWN_ERROR;
    }
    else {

        MY_LOGD("streamID %#" PRIx64 ", add %d buffer", mpStreamInfo->getStreamId(), addNumberOfBuffers);

        for ( MINT32 i = 0; i < addNumberOfBuffers; ++i ) {
            sp<IImageBufferHeap> pHeap;
            if( do_construct(pHeap) == NO_MEMORY ) {
                MY_LOGE("do_construct allocate buffer failed");
                continue;
            }
            {
                Mutex::Autolock _l(mPoolLock);
                MY_LOGD("streamID %#" PRIx64 ", add bufHeap: %p", mpStreamInfo->getStreamId(), pHeap.get());
                mAvailableBuf.push_back(pHeap);

                // update max buffer count
                mMaxBuffer = mMaxBuffer + addNumberOfBuffers;
                mpStreamInfo->setMaxBufNum(mMaxBuffer);
            }
        }
    }

    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
addBuffer(
    android::sp<IImageBuffer>& pBuffer
)
{
    mbNoNeedReturnBuffer = true;
    return mImp->addBuffer(pBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
addBuffer(
    android::sp<IImageBuffer>& pBuffer
)
{
    FUNC_START;
    IImageBufferHeap *pBuf = pBuffer->getImageBufferHeap();
    MY_LOGD("streamID %#" PRIx64", add bufHeap %p", mpStreamInfo->getStreamId(), pBuf);

    if(pBuffer.get() == NULL)
    {
        MY_LOGE("Input Buffer is NULL!");
        return INVALID_OPERATION;
    }

    Mutex::Autolock _l(mPoolLock);
    mAvailableBuf.push_back(pBuf);

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
android::sp<IImageStreamInfo>
CallbackBufferPool::
getStreamInfo()
{
    return mImp->getStreamInfo();
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IImageStreamInfo>
CallbackBufferPool::CallbackBufferPoolImp::
getStreamInfo()
{
    return mpStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::
updateStreamInfo(
    const sp<IImageStreamInfo>& pStreamInfo
)
{
    return mImp->updateStreamInfo(pStreamInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
updateStreamInfo(
    const sp<IImageStreamInfo>& pStreamInfo
)
{
    mpStreamInfo->updateStreamInfo(pStreamInfo);

    if (pStreamInfo->getImgFormat() != eImgFmt_JPEG &&
        pStreamInfo->getImgFormat() != eImgFmt_BLOB ){

        IImageStreamInfo::BufPlanes_t const& bufPlanes = mpStreamInfo->getBufPlanes();
        MY_LOGD("mAvailableBuf:%zu mInUseBuf:%zu, bufPlanes:%zu",
                mAvailableBuf.size(), mInUseBuf.size(), bufPlanes.size());

        size_t bufRowStrideInBytes[3] = {0};
        size_t bufSizeInBytes[3] = {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufRowStrideInBytes[i] = bufPlanes[i].rowStrideInBytes;
            bufSizeInBytes[i] = bufPlanes[i].sizeInBytes;
            MY_LOGD("stride:%zu, sizeInBytes:%zu", bufRowStrideInBytes[i], bufSizeInBytes[i]);
        }

        typename List< sp<IImageBufferHeap> >::iterator iter = mAvailableBuf.begin();
        while( iter != mAvailableBuf.end() ) {
            sp<IImageBufferHeap> pBuffer = *iter;
            pBuffer->updateImgInfo(
                    pStreamInfo->getImgSize(),
                    pStreamInfo->getImgFormat(),
                    bufSizeInBytes,
                    bufRowStrideInBytes,
                    bufPlanes.size());
            iter++;
        }
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::
setForceNoNeedReturnBuffer(MBOOL bFlag)
{
    mImp->setForceNoNeedReturnBuffer(bFlag);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::CallbackBufferPoolImp::
setForceNoNeedReturnBuffer(MBOOL bFlag)
{
    FUNC_START;

    Mutex::Autolock _l(mPoolLock);

    MY_LOGD_IF( mLogLevel >= 1, "mAvailableBuf:%d mInUseBuf:%d",
            mAvailableBuf.size(), mInUseBuf.size());

    while(!mInUseBuf.empty())
    {
        std::chrono::nanoseconds maxWaitTimeNS(std::chrono::seconds(5));
        MY_LOGD("still in-use buf, start waiting %lld ns", maxWaitTimeNS.count());
        status_t status = mCondWaitBuf.waitRelative(mPoolLock, maxWaitTimeNS.count());
        if( status != OK )
        {
            MY_LOGE("waiting timeout !");
            return;
        }
    }

    MY_LOGD_IF( mLogLevel >= 1, "noNeedReturn (%d)->(%d)",
        mbForceNoNeedReturnBuffer, bFlag);

    mbForceNoNeedReturnBuffer = bFlag;

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::
setExtraParam(MBOOL contiguousPlanes)
{
    mImp->setExtraParam(contiguousPlanes);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::CallbackBufferPoolImp::
setExtraParam(MBOOL contiguousPlanes)
{
    MY_LOGD("continue(%d)", contiguousPlanes);
    mExtParam.contiguousPlanes = contiguousPlanes;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::
flush()
{
    return mImp->flush();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackBufferPool::CallbackBufferPoolImp::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mPoolLock);
    mAvailableBuf.clear();
    mInUseBuf.clear();
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
removeBufferFromPool(
    char const* szCallerName,
    android::sp<IImageBufferHeap>& pBuffer
)
{
    FUNC_START;
    Mutex::Autolock _l(mPoolLock);

    typename List< sp<IImageBufferHeap> >::iterator iter = mInUseBuf.begin();
    while( iter != mInUseBuf.end() ) {
        if ( pBuffer == (*iter) ) {
            mInUseBuf.erase(iter);

            MY_LOGD_IF(
                mLogLevel >= 1,
                "mInUseBuf erase, [%s] mAvailableBuf:%d mInUseBuf:%d bufHeap:(%p)", szCallerName, mAvailableBuf.size(), mInUseBuf.size(), pBuffer.get()
            );
            FUNC_END;
            return OK;
        }
        iter++;
    }
    iter = mAvailableBuf.begin();
    while( iter != mAvailableBuf.end() ) {
        if ( pBuffer == (*iter) ) {
            mAvailableBuf.erase(iter);

            MY_LOGD_IF(
                mLogLevel >= 1,
                "mAvailableBuf erase, [%s] mAvailableBuf:%d mInUseBuf:%d bufHeap:(%p)", szCallerName, mAvailableBuf.size(), mInUseBuf.size(), pBuffer.get()
            );
            FUNC_END;
            return OK;
        }
        iter++;
    }

    MY_LOGE("[%s] Cannot find buffer %p.", szCallerName, pBuffer.get());

    FUNC_END;
    return UNKNOWN_ERROR;
}



/******************************************************************************
 *
 ******************************************************************************/
void
CallbackBufferPool::CallbackBufferPoolImp::
onLastStrongRef(const void* /*id*/)
{
    CAM_TRACE_NAME("CallbackBufferPool:onLastStrongRef");
    FUNC_START;
    requestExit();
    //
    Mutex::Autolock _l(mPoolLock);
    mAvailableBuf.clear();

    if ( mInUseBuf.size() > 0 ) {
        typename List< sp<IImageBufferHeap> >::iterator iter = mInUseBuf.begin();
        while( iter != mInUseBuf.end() ) {
            MY_LOGW("[%s] buffer %p not return to pool.", "AIAP"/*poolName()*/, (*iter).get());
            iter++;
        }
    }

    mInUseBuf.clear();
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CallbackBufferPool::CallbackBufferPoolImp::
requestExit()
{
    //let allocate thread back
    Thread::requestExitAndWait();
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackBufferPool::CallbackBufferPoolImp::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)BUFFERPOOL_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, BUFFERPOOL_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, BUFFERPOOL_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CallbackBufferPool::CallbackBufferPoolImp::
do_construct(
    sp<IImageBufferHeap>& pImageBufferHeap
)
{
    IImageStreamInfo::BufPlanes_t const& bufPlanes = mpStreamInfo->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    size_t bufCustomSizeInBytes[3] = {0};
    size_t bufReusableSizeInBytes[3] = {0};
    for (size_t i = 0; i < bufPlanes.size(); i++) {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        bufCustomSizeInBytes[i] = bufPlanes[i].sizeInBytes;
        bufReusableSizeInBytes[i] = bufPlanes[i].reusableSizeInBytes;
    }

    if ( eImgFmt_JPEG == mpStreamInfo->getImgFormat() ||
         eImgFmt_BLOB == mpStreamInfo->getImgFormat() )
    {
        IImageBufferAllocator::ImgParam imgParam(
                mpStreamInfo->getImgSize(),
                (*bufStridesInBytes),
                (*bufBoundaryInBytes));
        imgParam.imgFormat = eImgFmt_BLOB;
        MY_LOGD("eImgFmt_JPEG -> eImgFmt_BLOB");
        if (!mSecurity) {
            pImageBufferHeap = IIonImageBufferHeap::create(
                    mpStreamInfo->getStreamName(),
                    imgParam,
                    IIonImageBufferHeap::AllocExtraParam(),
                    MFALSE
                    );
        } else {
            pImageBufferHeap = ISecureImageBufferHeap::create(
                    mpStreamInfo->getStreamName(),
                    imgParam,
                    ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MFALSE, mSecType),
                    MFALSE
                    );

        }
    }
    else
    {
        IImageBufferAllocator::ImgParam imgParam(
            mpStreamInfo->getImgFormat(),
            mpStreamInfo->getImgSize(),
            bufStridesInBytes, bufBoundaryInBytes,
            bufCustomSizeInBytes, bufReusableSizeInBytes,
            bufPlanes.size()
            );
            MY_LOGD("format:%x, size:(%d,%d), stride:%zu, boundary:%zu, customized size: %zu, reuse size: %zu, planes:%zu, continue:%d",
            mpStreamInfo->getImgFormat(),
            mpStreamInfo->getImgSize().w, mpStreamInfo->getImgSize().h,
            bufStridesInBytes[0], bufBoundaryInBytes[0],
            bufCustomSizeInBytes[0], bufReusableSizeInBytes[0],
            bufPlanes.size(), mExtParam.contiguousPlanes
            );
        if (!mSecurity) {
            pImageBufferHeap = IIonImageBufferHeap::create(
                    mpStreamInfo->getStreamName(),
                    imgParam,
                    mExtParam,
                    MFALSE
                    );
        } else {
            if (bufPlanes.size() > 1) {
                // multi-plane
                MUINT32 allPlaneSize = 0;
                const auto format(mpStreamInfo->getImgFormat());
                const auto width(mpStreamInfo->getImgSize().w);
                const auto height(mpStreamInfo->getImgSize().h);
                for (size_t i = 0; i < bufPlanes.size(); i++) {
                    allPlaneSize +=
                        (Format::queryPlaneWidthInPixels(format, i, width) *
                         Format::queryPlaneBitsPerPixel(format, i) + 7) / 8 *
                         Format::queryPlaneHeightInPixels(format, i, height);
                }
                IImageBufferAllocator::ImgParam imgParam_yuv(allPlaneSize, 0);
                sp<ISecureImageBufferHeap> blobHeap =
                        ISecureImageBufferHeap::create(mpStreamInfo->getStreamName(),
                        imgParam_yuv,
                        ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MTRUE, mSecType));
                pImageBufferHeap = blobHeap->createImageBufferHeap_FromBlobHeap(
                        mpStreamInfo->getStreamName(), imgParam, MFALSE);
            } else {
                // single-plane
                // TODO use blob heap to create single-plane heap
                pImageBufferHeap = ISecureImageBufferHeap::create(
                        mpStreamInfo->getStreamName(),
                        imgParam,
                        ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MFALSE, mSecType),
                        MFALSE
                        );
            }
        }
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CallbackBufferPool::CallbackBufferPoolImp::
threadLoop()
{
    bool next = false;
    {
        Mutex::Autolock _l(mPoolLock);
        if ( (MINT32)( mAvailableBuf.size() + mInUseBuf.size() ) >= mMaxBuffer)
            return false;
    }
    sp<IImageBufferHeap> pHeap;
    if( do_construct(pHeap) == NO_MEMORY ) {
        MY_LOGE("do_construct allocate buffer failed");
        return true;
    }

    {
        Mutex::Autolock _l(mPoolLock);
        mAvailableBuf.push_back(pHeap);
        mCondWaitBuf.signal();

        next = (MINT32)( mAvailableBuf.size() + mInUseBuf.size() ) < mMaxBuffer;
    }
    return next;
}
