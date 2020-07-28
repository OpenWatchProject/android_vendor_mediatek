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

#define LOG_TAG "MtkCam/ZsdProcessor"

#include "ZsdProcessor.h"
#include <list>
//

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::Zsd;

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
#define FUNC_START     MY_LOGD_IF(1, "+")
#define FUNC_END       MY_LOGD_IF(1, "-")
#else
#define FUNC_START     MY_LOGD_IF(mLogLevel>=2, "+")
#define FUNC_END       MY_LOGD_IF(mLogLevel>=2, "-")
#endif

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( err != OK ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of external image stream buffer.
 */
class ZsdProcessorImp
    : public ZsdProcessor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                        ZsdProcessorImp();
                        ~ZsdProcessorImp();

     virtual auto       onLastStrongRef(const void* id) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ZsdProcessor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // config needed stream Ids for zsd usage.
    virtual auto        configStreamIds(
                            InfoSetT& rStreamConfigSet
                        ) -> int;

    // check buffers of specific request is ready or not.
    virtual auto        isZsdBuffersReady(
                            int64_t serialNo
                        ) -> bool;

    // get external buffers for zsd usage in specific request, most called after isExternalBuffersReadyLocked()
    virtual auto        getZsdBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo,
                            int64_t newSerialNo
                        ) -> int;

    // register StreamBuffer already allocated (from AppStreamMgr).
    // keep its lifecycle in ZsdProcessor by holding strong poniter.
    virtual auto        registerAllocatedBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo
                        ) -> int;

    // deque external buffers from ZsdProcessor.
    // need to implement external pool in image and meta.
    virtual auto        dequeBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo
                        ) -> int;

    virtual auto        notifyZsdDone(
                            int64_t serialNo
                        ) -> int;

    // enque USED external buffers to ZsdProcessor for future use.
    virtual auto        enqueBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo
                        ) -> int;

    // enque USED external images to ZsdProcessor for future use.
    virtual auto        enqueExternalImages(
                            ImageSet& rImageSet,
                            int64_t serialNo
                        ) -> int;

    // enque USED external metas to ZsdProcessor for future use.
    virtual auto        enqueExternalMetas(
                            MetaSet& rMetaSet,
                            int64_t serialNo
                        ) -> int;

    // clear inflight external buffers.
    virtual auto        clear() -> int;

protected:
    virtual auto        allocatePool(sp<IImageStreamInfo> pStreamInfo) -> sp<ExtImageStreamBufferPoolT>;
    virtual auto        allocateMaxBufferFromPool(sp<ExtImageStreamBufferPoolT> pPool) -> int;
    virtual auto        allocateBufferList() -> int;
    virtual auto        releaseBufferList() -> int;
    virtual auto        reallocateBufferSet(BufferSetT& bufferSet) -> int;
    virtual auto        isExistInBufferListLocked(BufferSetT& bufferSet) -> bool;
    virtual auto        isExternalBuffersReadyLocked(int64_t serialNo) -> bool;

protected:
    virtual auto        dump() -> void;
    virtual auto        dumpCircularBuffer() -> void;
    virtual auto        dumpImageSet(ImageSet& imageSet) -> void;
    virtual auto        dumpMetaSet(MetaSet& metaSet) -> void;
    virtual auto        dumpBufferList() -> void;

protected:
    typedef std::list<BufferSetT>    BufferList;
    struct CircularBuffer
        : DefaultKeyedVector<int64_t, FrameSetT>
    {
        typedef DefaultKeyedVector<int64_t, FrameSetT>
                        ParentT;

        int32_t         mMaxSize;   // max circular buffer size, set by users.
                                    // todo: how to handle different max count setting from user?
        size_t          mIndex;     // latest FrameSet index

        // overwrite some operations.
        // avoid exceeding max circular size,
        size_t          size() const { return (ParentT::size() <= mMaxSize)? ParentT::size() : mMaxSize ; }
        ssize_t         add(const int64_t& key, const FrameSetT& item)
                        {
                            if ( ParentT::size() < mMaxSize ) {
                                mIndex++;
                                return ParentT::add(key, item);
                            } else {
                                mIndex = (mIndex+1)%mMaxSize;
                                ParentT::removeItemsAt(mIndex);
                                return ParentT::add(key, item);
                            }

                        }
        ssize_t         setCapacity(size_t size) { return 0; }
        ssize_t         setMaxSize(size_t size) { return mMaxSize = size; }
        ssize_t         getMaxSize() const { return mMaxSize; }

        //
        void            releaseOneLocked()
                        {
                            for( size_t i=0; i<size(); i++) {
                                if ( valueAt(i).status == eStatus_ENQUED ||
                                     valueAt(i).status == eStatus_RELEASED  ) {
                                    removeItemsAt(i);
                                    break;
                                }
                            }
                        }
    };

protected:
    mutable Mutex       mLock;
    Condition           mCond;

    int32_t             mLogLevel;

    BufferList          mBufferList;
    CircularBuffer      mCircularBufferMap;
    DefaultKeyedVector<int64_t, sp<ExtImageStreamBufferPoolT> >
                        mPoolMap;
    InfoSetT            mStreamConfigSet;

};

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessor::
createInstance() -> android::sp<ZsdProcessor>
{
    auto pImp = new ZsdProcessorImp();
    return pImp;
}

/******************************************************************************
 *
 ******************************************************************************/
ZsdProcessorImp::
ZsdProcessorImp()
    : mLock()
    , mCond()
    , mLogLevel()
    , mCircularBufferMap()
    , mStreamConfigSet()
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.hwpipeline.zsd", 0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
ZsdProcessorImp::
~ZsdProcessorImp()
{
    FUNC_START;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
onLastStrongRef(const void* /*id*/) -> void
{
    FUNC_START;
    clear();
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
configStreamIds(
    InfoSetT& rStreamConfigSet
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;
    mStreamConfigSet.clear();
    for(size_t i=0; i<rStreamConfigSet.vImageConfig.size(); i++ ) {
        StreamId_T streamId = rStreamConfigSet.vImageConfig.keyAt(i);
        auto imageCfg = rStreamConfigSet.vImageConfig.editValueAt(i);
        mStreamConfigSet.vImageConfig.add(streamId, imageCfg);
        size_t maxSize = imageCfg.pInfo->getMaxBufNum();
        MY_LOGI("%#" PRIx64 ":%p(%s) w/ size(%zu)", streamId, imageCfg.pInfo.get(), imageCfg.pInfo->getStreamName(), maxSize );
        if ( mCircularBufferMap.size() < maxSize ) {
            mCircularBufferMap.setMaxSize( maxSize );
        }
    }
    for(size_t i=0; i<rStreamConfigSet.vMetaConfig.size(); i++) {
        StreamId_T streamId = rStreamConfigSet.vMetaConfig.keyAt(i);
        auto metaCfg = rStreamConfigSet.vMetaConfig.editValueAt(i);
        MY_LOGI("%#" PRIx64 ":%p(%s)", streamId, metaCfg.pInfo.get(), metaCfg.pInfo->getStreamName() );
        mStreamConfigSet.vMetaConfig.add(streamId, metaCfg);
    }
    if ( mCircularBufferMap.getMaxSize()==0 )   // means there no image buffer
        mCircularBufferMap.setMaxSize(10);

    MY_LOGI("maxSize: %zu", mCircularBufferMap.getMaxSize());
// allocate buffer pool with another thread for ExtImageStreamBuffer.
// ExtMetaStreamBuffer should use run-time allocation.
#if 1
    for(size_t i=0; i<rStreamConfigSet.vImageConfig.size(); i++) {
        if( rStreamConfigSet.vImageConfig.editValueAt(i).bExtPool ) {
            sp<ExtImageStreamBufferPoolT> pPool = allocatePool(rStreamConfigSet.vImageConfig.editValueAt(i).pInfo);
            mPoolMap.add(rStreamConfigSet.vImageConfig.keyAt(i), pPool);
            {
                // new thread to process allocation in background thread
                CHECK_ERROR( allocateMaxBufferFromPool(pPool) );
            }
        }
    }
    CHECK_ERROR( allocateBufferList() );
#endif
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
allocateBufferList() -> int
{
    CAM_TRACE_CALL();
    FUNC_START;

    auto allocateImageBuffer = [&](auto& imageSet) {
        ImageConfigSet& imageConfig = mStreamConfigSet.vImageConfig;
        for(size_t i=0; i<imageConfig.size(); i++) {
            StreamId_T streamId = imageConfig[i].pInfo->getStreamId();
            if( imageConfig[i].bExtPool ) {
                sp<ExtImageStreamBuffer> pStreamBuffer;
                sp<ExtImageStreamBufferPoolT> pPool = mPoolMap.editValueFor(streamId);

                MERROR err = ( !pPool.get() )? NAME_NOT_FOUND :
                     pPool->acquireFromPool(__FUNCTION__, pStreamBuffer, ::s2ns(10));

                if ( err || !pStreamBuffer.get() ) {
                    MY_LOGE("[acquireFromPool] err:%d(%s) pStreamBuffer:%p stream:%#" PRIx64 "(%s)",
                            err, ::strerror(-err), pStreamBuffer.get(),
                            streamId, imageConfig[i].pInfo->getStreamName() );
                    return NAME_NOT_FOUND;
                }
                MY_LOGD_IF(mLogLevel>=0, "allocate image(%#" PRIx64 ") buffer(%p) from pool", streamId, pStreamBuffer.get() );
                imageSet.add(streamId, new ImageItem(pStreamBuffer, false) );
            } else {
                MY_LOGE("not implement yet");
            }
        }
        return OK;
    };

    auto allocateMetaBuffer = [&](auto& metaSet) {
        MetaConfigSet& metaConfig = mStreamConfigSet.vMetaConfig;
        for(size_t i=0; i<metaConfig.size(); i++) {
            // check already exists or not
            StreamId_T streamId = metaConfig[i].pInfo->getStreamId();
            if( metaConfig[i].bExtPool ) {
                MY_LOGE("not implement yet");
            } else {
                sp<ExtMetaStreamBuffer> pStreamBuffer =
                ExtMetaStreamBuffer::Allocator(metaConfig[i].pInfo.get())();
                MY_LOGD_IF(mLogLevel>=0, "register meta(%#" PRIx64 ") buffer(%p)", streamId, pStreamBuffer.get() );
                metaSet.add(streamId, new MetaItem(pStreamBuffer, false) );
            }
        }
        return OK;
    };

    Mutex::Autolock _l(mLock);
    for( size_t i=0; i<mCircularBufferMap.getMaxSize(); i++ ) {
        BufferSetT bufferSet;
        CHECK_ERROR( allocateImageBuffer(bufferSet.vImageSet) );
        CHECK_ERROR( allocateMetaBuffer(bufferSet.vMetaSet) );
        // dumpImageSet(bufferSet.vImageSet);
        // dumpMetaSet(bufferSet.vMetaSet);
        mBufferList.push_back(bufferSet);
        MY_LOGD("buffer list size %zu", mBufferList.size() );
    }
    dumpBufferList();

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
releaseBufferList() -> int
{
    CAM_TRACE_CALL();
    FUNC_START;

    dumpBufferList();
    auto releaseImageBuffer = [&](auto& imageSet) {
        ImageConfigSet& imageConfig = mStreamConfigSet.vImageConfig;
        for(size_t i=0; i<imageConfig.size(); i++) {
            StreamId_T streamId = imageConfig[i].pInfo->getStreamId();
            if( imageConfig[i].bExtPool ) {
                for( size_t i=0; i<imageSet.size(); i++) {
                    if ( streamId == imageSet[i]->pStreamBuffer->getStreamInfo()->getStreamId() ) {
                        MY_LOGI("release to pool");
                        imageSet[i]->pStreamBuffer->releaseBuffer();
                    }
                }
            }
        }
        return OK;
    };

    Mutex::Autolock _l(mLock);
    dumpBufferList();
    for( size_t i=0; i<mCircularBufferMap.getMaxSize(); i++ ) {
        BufferSetT bufferSet = mBufferList.front();
        CHECK_ERROR( releaseImageBuffer(bufferSet.vImageSet) );
        // CHECK_ERROR( allocateMetaBuffer(bufferSet.vMetaSet) );
        // dumpImageSet(bufferSet.vImageSet);
        // dumpMetaSet(bufferSet.vMetaSet);
        mBufferList.pop_front();
        MY_LOGD("buffer list size %zu", mBufferList.size() );
    }
    dumpBufferList();

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
allocateMaxBufferFromPool(
    sp<ExtImageStreamBufferPoolT> pPool
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;
    if  ( pPool == 0 ) {
        MY_LOGW("null image pool");
        return NAME_NOT_FOUND;
    }

    MERROR err = pPool->commitPool(pPool->poolName());
    if  ( OK != err ) {
        MY_LOGE("%s: commitPool err:%d(%s)", pPool->poolName(), err, ::strerror(-err));
        return NO_MEMORY;
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
allocatePool(sp<IImageStreamInfo> pStreamInfo) -> sp<ExtImageStreamBufferPoolT>
{
    CAM_TRACE_CALL();
    IImageStreamInfo::BufPlanes_t const& bufPlanes = pStreamInfo->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    for (size_t i = 0; i < bufPlanes.size(); i++) {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
    }

    sp<ExtImageStreamBufferPoolT>         pPool;

    IImageBufferAllocator::ImgParam const imgParam(
        pStreamInfo->getImgFormat(),
        pStreamInfo->getImgSize(),
        bufStridesInBytes, bufBoundaryInBytes,
        bufPlanes.size()
    );

    pPool = new ExtImageStreamBufferPoolT(
        pStreamInfo->getStreamName(),
        ExtImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)
    );

    if  ( pPool == 0 ) {
        MY_LOGE("Fail to new a image pool:%s", pStreamInfo->getStreamName());
        return NULL;
    }
    //
    MERROR err = pPool->initPool(pStreamInfo->getStreamName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
    if  ( OK != err ) {
        MY_LOGE("%s: initPool err:%d(%s)", pStreamInfo->getStreamName(), err, ::strerror(-err));
        return NULL;
    }
    // if  ( OK != pPool->commitPool(pStreamInfo->getStreamName()) ) {
    //     MY_LOGE("%s: commitPool err:%d(%s)", pStreamInfo->getStreamName(), err, ::strerror(-err));
    //     return NULL;
    // }
    //
    return pPool;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
isZsdBuffersReady(
    int64_t serialNo
) -> bool
{
    CAM_TRACE_CALL();
    FUNC_START;

    Mutex::Autolock _l(mLock);
    if( mCircularBufferMap.indexOfKey(serialNo) < 0 ) {
        MY_LOGE("there exists no circular buffer in %" PRId64, serialNo);
        dump();
        dumpBufferList();
        return false;
    }

    FUNC_END;
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
registerAllocatedBuffers(
    BufferSetT& rBufferSet,
    int64_t serialNo
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;

    auto waitAndGetBuffer = [this](nsecs_t timeout, BufferSetT& rBuffer) {
        MY_LOGD_IF( mLogLevel>=3, "waitAndGetBuffer +");
        nsecs_t const startTime = ::systemTime();
        while ( mBufferList.empty() && (::systemTime()-startTime<timeout) ) {
            int err = mCond.waitRelative(mLock, timeout);
            if( err != OK ) {
                //
            }
        }
        if ( mBufferList.empty() ) {
            MY_LOGE("timeout for wait buffer");
            return NAME_NOT_FOUND;
        }
        rBuffer = mBufferList.front();
        mBufferList.pop_front();
        if ( mLogLevel>=2 ) dumpBufferList();
        MY_LOGD_IF( mLogLevel>=2, "buffer list size: %zu", mBufferList.size() );
        return OK;
    };

    auto addBufferFromUser = [this](auto& frameSet, auto& bufferSet) {
        for (size_t i=0; i<bufferSet.vImageSet.size(); i++) {
            auto streamId = bufferSet.vImageSet.keyAt(i);
            auto streamBuffer = bufferSet.vImageSet.valueAt(i)->pStreamBuffer;
            MY_LOGD_IF( mLogLevel>=2, "register image buffer from user (%#" PRIx64 ":%p)",
                        streamId, streamBuffer.get());
            if( mStreamConfigSet.vImageConfig.indexOfKey(streamId) < 0 ) {
                continue;
            }
            frameSet.bufferSet.vImageSet.replaceValueFor(
                    streamId, new ImageItem(streamBuffer, false) );
        }
        for(size_t i=0; i<bufferSet.vMetaSet.size(); i++) {
            auto streamId = bufferSet.vMetaSet.keyAt(i);
            auto streamBuffer = bufferSet.vMetaSet.valueAt(i)->pStreamBuffer;
            MY_LOGD_IF( mLogLevel>=2,"register meta buffer from user (%#" PRIx64 ":%p)",
                        streamId, streamBuffer.get());
            if( mStreamConfigSet.vMetaConfig.indexOfKey(streamId) < 0 ) {
                continue;
            }
            frameSet.bufferSet.vMetaSet.replaceValueFor(
                    streamId, new MetaItem(streamBuffer, false) );
        }
        return OK;
    };

    Mutex::Autolock _l(mLock);
    ssize_t index = mCircularBufferMap.indexOfKey(serialNo);
    if ( index >= 0 ) {
        MY_LOGW("should not exists in circular buffer of %" PRId64, serialNo);
    }

    // TODO: get next buffer for circular bufferset
    // int status = getNextCircularBuffer(frameSet);
    BufferSetT bufferSet;
    CHECK_ERROR( waitAndGetBuffer(3000000000, bufferSet) );
    mCircularBufferMap.releaseOneLocked();
    mCircularBufferMap.add(serialNo,
                           FrameSetT{.bufferSet=bufferSet,
                                     .requestNo=serialNo} );
    MY_LOGD_IF( mLogLevel>=2, "CB(%zu)/BL(%zu)", mCircularBufferMap.size(), mBufferList.size() );
    reallocateBufferSet(mCircularBufferMap.editValueFor(serialNo).bufferSet);

    CHECK_ERROR( addBufferFromUser(mCircularBufferMap.editValueFor(serialNo), rBufferSet) );
    mCircularBufferMap.editValueFor(serialNo).updateStatus(eStatus_REGISTERED);
    // MY_LOGD("frameItem is at %zu -> %#" PRId64 , mCircularBufferMap.indexOfKey(serialNo), serialNo );
    if( mLogLevel>=2 ) dump();

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
reallocateBufferSet(BufferSetT& bufferSet) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;
    for( size_t i=0; i<bufferSet.vImageSet.size(); i++) {
        StreamId_T streamId = bufferSet.vImageSet.keyAt(i);
        auto pStreamBuffer  = bufferSet.vImageSet.editValueAt(i)->pStreamBuffer;
        pStreamBuffer->releaseBuffer();

        // sp<ExtImageStreamBuffer> pStreamBuffer;
        sp<ExtImageStreamBufferPoolT> pPool = mPoolMap.editValueFor(streamId);

        MERROR err = ( !pPool.get() )? NAME_NOT_FOUND :
             pPool->acquireFromPool(__FUNCTION__, pStreamBuffer, ::s2ns(10));

        if ( err || !pStreamBuffer.get() ) {
            MY_LOGE("[acquireFromPool] err:%d(%s) pStreamBuffer:%p stream:%#" PRIx64 "(%s)",
                    err, ::strerror(-err), pStreamBuffer.get(),
                    streamId, pStreamBuffer->getStreamInfo()->getStreamName() );
            return NAME_NOT_FOUND;
        }
        MY_LOGD_IF(mLogLevel>=2, "allocate image(%#" PRIx64 ") buffer(%p) from pool", streamId, pStreamBuffer.get() );
        bufferSet.vImageSet.replaceValueFor(streamId, new ImageItem(pStreamBuffer, false) );

    }
    for( size_t i=0; i<bufferSet.vMetaSet.size(); i++) {
        StreamId_T streamId = bufferSet.vMetaSet.keyAt(i);
        auto pStreamBuffer  = bufferSet.vMetaSet.editValueAt(i)->pStreamBuffer;
        pStreamBuffer       = ExtMetaStreamBuffer::Allocator(mStreamConfigSet.vMetaConfig.valueFor(streamId).pInfo.get())();
        MY_LOGD_IF(mLogLevel>=2, "register meta(%#" PRIx64 ") buffer(%p)", streamId, pStreamBuffer.get() );
        bufferSet.vMetaSet.replaceValueFor(streamId, new MetaItem(pStreamBuffer, false) );

    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
dequeBuffers(
    BufferSetT& rBufferSet,
    int64_t serialNo
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;
    Mutex::Autolock _l(mLock);

    if( mCircularBufferMap.indexOfKey(serialNo) < 0 ) {
        MY_LOGE("there exists no circular buffer in %" PRId64, serialNo);
        return NAME_NOT_FOUND;
    }

    FrameSetT& frameSet = mCircularBufferMap.editValueFor(serialNo);
    if( frameSet.status != eStatus_REGISTERED ) {
        MY_LOGE("frameSet(%" PRId64 ") is not registered for deque, status(%d)", serialNo, frameSet.status);
        return INVALID_OPERATION;
    }

    rBufferSet = frameSet.bufferSet;
    frameSet.updateStatus(eStatus_DEQUED);

    MY_LOGD_IF( mLogLevel>=1, "frameSet(%" PRId64 ") is dequed", serialNo );
    if( mLogLevel>=2) dumpImageSet(rBufferSet.vImageSet);
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
enqueBuffers(
    BufferSetT& rBufferSet,
    int64_t serialNo
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;
    Mutex::Autolock _l(mLock);

    if( mCircularBufferMap.indexOfKey(serialNo) < 0 ) {
        MY_LOGE("there exists no circular buffer in %" PRId64, serialNo);
        return NAME_NOT_FOUND;
    }

    FrameSetT& frameSet = mCircularBufferMap.editValueFor(serialNo);
    if ( frameSet.status != eStatus_DEQUED ) {
        MY_LOGE("frameSet(%" PRId64 ") is not dequed, status(%d)", serialNo, frameSet.status);
        return INVALID_OPERATION;
    }
    // check image buffer because it cannot be passed by updateResult() in pipeline frame mechanism
    for ( size_t i=0; i<frameSet.bufferSet.vImageSet.size(); i++ ) {
        auto streamId = frameSet.bufferSet.vImageSet.keyAt(i);
        if ( frameSet.bufferSet.vImageSet[i]->pStreamBuffer->haveAllUsersReleased() == OK &&
             frameSet.bufferSet.vImageSet[i]->valid == false ) {
            // MY_LOGD("image(%#" PRIx64 ") release! ", streamId );
            frameSet.bufferSet.vImageSet.editValueFor(streamId)->valid = true;
            frameSet.imageDone++;
        }
    }
    for ( size_t i=0; i<frameSet.bufferSet.vMetaSet.size(); i++) {
        auto streamId = frameSet.bufferSet.vMetaSet.keyAt(i);
        if ( frameSet.bufferSet.vMetaSet[i]->pStreamBuffer->haveAllUsersReleased() == OK &&
             frameSet.bufferSet.vMetaSet[i]->valid == false ) {
            // MY_LOGD("meta(%#" PRIx64 ") release! ", streamId );
            frameSet.bufferSet.vMetaSet.editValueFor(streamId)->valid = true;
            frameSet.metaDone++;
        }
    }

    auto Log = [this](auto& frameSet) {
        ImageSet& imageSet = frameSet.bufferSet.vImageSet;
        MetaSet&  metaSet  = frameSet.bufferSet.vMetaSet;
        String8 str = String8::format("Image - ");
        for(size_t i=0; i<imageSet.size(); i++) {
            str.appendFormat("%#" PRIx64 "(%p):%d; ", imageSet.keyAt(i),
                             imageSet.valueAt(i)->pStreamBuffer.get(), imageSet.valueAt(i)->valid);
        }
        MY_LOGD("%s", str.string());
        str.clear();
        str = String8::format("Meta - ");
        for(size_t i=0; i<metaSet.size(); i++) {
            str.appendFormat("%#" PRIx64 "(%p):%d; ", metaSet.keyAt(i),
                             metaSet.valueAt(i)->pStreamBuffer.get(), metaSet.valueAt(i)->valid);

        }
        MY_LOGD("%s", str.string());
    };
    // Log(frameSet);
    if ( frameSet.metaDone  == frameSet.bufferSet.vMetaSet.size() &&
         frameSet.imageDone == frameSet.bufferSet.vImageSet.size() ) {
        frameSet.updateStatus(eStatus_ENQUED);
        MY_LOGD_IF( mLogLevel>=1, "completed request %" PRId64 " metaDone(%zu) imageDone(%zu)", serialNo, frameSet.metaDone, frameSet.imageDone);

        mBufferList.push_back(frameSet.bufferSet);
        if ( mLogLevel>=2 ) dumpBufferList();
        mCond.broadcast();
    }
    //
    MY_LOGD_IF( mLogLevel>=2, "CB(%zu)/BL(%zu)", mCircularBufferMap.size(), mBufferList.size() );
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
enqueExternalImages(
    ImageSet& rImageSet,
    int64_t serialNo
) -> int
{
    FUNC_START;
    MY_LOGE("Not implemented yet");
    FUNC_END;
    return NAME_NOT_FOUND;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
enqueExternalMetas(
    MetaSet& rMetaSet,
    int64_t serialNo
) -> int
{
    FUNC_START;
    MY_LOGE("Not implemented yet");
    FUNC_END;
    return NAME_NOT_FOUND;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
isExternalBuffersReadyLocked(
    int64_t serialNo
) -> bool
{
    CAM_TRACE_CALL();
    FUNC_START;

    auto waitResult = [=](nsecs_t timeout) {
        CAM_TRACE_NAME("waitResult");
        MY_LOGD_IF( mLogLevel>=3, "waitResult +");
        if ( eStatus_ENQUED == mCircularBufferMap.editValueFor(serialNo).status )
            return OK;
        nsecs_t const startTime = ::systemTime();
        while ( eStatus_ENQUED != mCircularBufferMap.editValueFor(serialNo).status &&
                (::systemTime()-startTime<timeout) ) {
            int err = mCond.waitRelative(mLock, timeout);
            if( err != OK ) {
                //
            }
        }
        MY_LOGD_IF( mLogLevel>=3, "waitResult -");
        return OK;
    };

    if( mCircularBufferMap.indexOfKey(serialNo) < 0 ) {
        MY_LOGE("there exists no circular buffer in %" PRId64, serialNo);
        dump();
        dumpBufferList();
        return false;
    }

    if ( waitResult(3000000000) != OK ) {
        MY_LOGE("frameSet(%" PRId64 ") is not enqued in 3 secs", serialNo);
        return false;
    }

    FUNC_END;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
getZsdBuffers(
    BufferSetT& rBufferSet,
    int64_t serialNo,
    int64_t newSerialNo
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;

    auto removeFromBufferListIfExist = [&](auto pStreamBuffer) {
        auto it = mBufferList.begin();
        while( it != mBufferList.end() ) {
            // should check it's pool type or not.
            for ( size_t i=0; i<rBufferSet.vImageSet.size(); i++ ) {
                if ( pStreamBuffer == (*it).vImageSet[0]->pStreamBuffer ) {
                    MY_LOGD("remove %p from bufferlist, becasue it's occupied by zsd-processing(%p)",
                            pStreamBuffer.get(), (*it).vImageSet[0]->pStreamBuffer.get() );
                    mBufferList.erase(it);
                    return OK;
                }
            }
            ++it;
        }
        return NAME_NOT_FOUND;
    };

    Mutex::Autolock _l(mLock);
    if( ! isExternalBuffersReadyLocked(serialNo) ) {
        // prepare new buffer.
        return NAME_NOT_FOUND;
    }

    // FrameSetT frameSet = mCircularBufferMap.editValueFor(serialNo);
    rBufferSet = mCircularBufferMap.editValueFor(serialNo).bufferSet;
    for( size_t i=0; i<rBufferSet.vImageSet.size(); i++) {
        auto pStreamBuffer = rBufferSet.vImageSet.editValueAt(i)->pStreamBuffer;
        auto pStreamInfo   = pStreamBuffer->getStreamInfo();
        rBufferSet.vImageSet.editValueAt(i)->valid = false;
        pStreamBuffer->clearStatus();
        sp<IUsersManager> pUsersManager = new UsersManager(pStreamInfo->getStreamId(), pStreamInfo->getStreamName() );
        pStreamBuffer->setUsersManager(pUsersManager);
        MY_LOGD("reset usersmanager of streamId(0x%" PRIx64 ":%s)->UsersManager(%p)",
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pUsersManager.get() );

        // erase this buffer from mBufferList
        CHECK_ERROR( removeFromBufferListIfExist(pStreamBuffer) );
    }
    for( size_t i=0; i<rBufferSet.vMetaSet.size(); i++) {
        auto pStreamBuffer = rBufferSet.vMetaSet.editValueAt(i)->pStreamBuffer;
        auto pStreamInfo   = pStreamBuffer->getStreamInfo();
        rBufferSet.vMetaSet.editValueAt(i)->valid = false;
        pStreamBuffer->clearStatus();
        sp<IUsersManager> pUsersManager = new UsersManager(pStreamInfo->getStreamId(), pStreamInfo->getStreamName() );
        pStreamBuffer->setUsersManager(pUsersManager);
        MY_LOGD("reset usersmanager of streamId(0x%" PRIx64 ":%s)->UsersManager(%p)",
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pUsersManager.get() );
    }

    if( mLogLevel>=1 ) {
        dump();
        dumpBufferList();
    }

    MY_LOGI("update serialNo: %" PRId64 " -> %" PRId64, serialNo, newSerialNo);
    mCircularBufferMap.editValueFor(serialNo).requestNo = newSerialNo;
    mCircularBufferMap.editValueFor(serialNo).imageDone = 0;
    mCircularBufferMap.editValueFor(serialNo).metaDone  = 0;
    mCircularBufferMap.editValueFor(serialNo).updateStatus(eStatus_ZSD);
    if ( mLogLevel>=1 ) {
        dumpImageSet(rBufferSet.vImageSet);
        dumpMetaSet(rBufferSet.vMetaSet);
        dumpCircularBuffer();
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
notifyZsdDone(
    int64_t serialNo
) -> int
{
    CAM_TRACE_CALL();
    FUNC_START;
    Mutex::Autolock _l(mLock);

    bool bNewRequestExist = false;
    size_t index = 0;
    for ( size_t i=0; i<mCircularBufferMap.size(); i++) {
        if ( mCircularBufferMap.valueAt(i).requestNo == serialNo ) {
            index = i;
            bNewRequestExist = true;
            break;
        }
    }
    if( ! bNewRequestExist ) {
        MY_LOGE("there exists no circular buffer in %" PRId64, serialNo);
        return NAME_NOT_FOUND;
    }

    FrameSetT& frameSet = mCircularBufferMap.editValueAt(index);
    if ( frameSet.status != eStatus_ZSD ) {
        MY_LOGE("frameSet(%" PRId64 ") is not in zsd processing, status(%d)", serialNo, frameSet.status);
        return INVALID_OPERATION;
    }
    mBufferList.push_back( frameSet.bufferSet );
    if ( mLogLevel>=3 ) dumpBufferList();
    mCond.broadcast();
    frameSet.updateStatus(eStatus_RELEASED);

    MY_LOGD_IF( mLogLevel>=2, "CB(%zu)/BL(%zu)", mCircularBufferMap.size(), mBufferList.size() );
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
isExistInBufferListLocked(BufferSetT& bufferSet) -> bool
{
    auto it = mBufferList.begin();
    for ( size_t i=0; i<bufferSet.vImageSet.size(); i++ ) {
        while ( it != mBufferList.end() ) {
            for ( size_t j=0; j<(*it).vImageSet.size(); j++) {
                if ( bufferSet.vImageSet[i]->pStreamBuffer == (*it).vImageSet[j]->pStreamBuffer )
                    return true;
            }
            ++it;
        }
    }
    return false;
};


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
clear() -> int
{
    CAM_TRACE_CALL();
    MY_LOGD("+");
    {
        Mutex::Autolock _l(mLock);
        MY_LOGD_IF( mLogLevel>=3, "waitCircularBufferEmpty +");
        while ( mCircularBufferMap.size()>0 ) {
            MY_LOGD_IF( mLogLevel>=3, "waitCircularBufferEmpty wait condition +");
            int err = mCond.waitRelative(mLock, 1000000000);
            MY_LOGD_IF( mLogLevel>=3, "waitCircularBufferEmpty wait condition -");
            if( err != OK ) {
                //
            }
            //
            for ( ssize_t i=(ssize_t)mCircularBufferMap.size()-1; i>=0; i--) {
                auto frameSet = mCircularBufferMap.editValueAt(i);
                if( frameSet.status == eStatus_UNINITED ||
                    frameSet.status == eStatus_ENQUED ||
                    frameSet.status == eStatus_RELEASED ) {
                    if ( ! isExistInBufferListLocked(frameSet.bufferSet) )
                        mBufferList.push_back(frameSet.bufferSet);
                    mCircularBufferMap.removeItemsAt(i);
                }
                MY_LOGD_IF( mLogLevel>=1, "%zu: rest: %zu", i, mCircularBufferMap.size() );
            }

            if ( mCircularBufferMap.size() ) {
                MY_LOGD("rest: %zu", mCircularBufferMap.size() );
                dump();
            }
        }
        MY_LOGD_IF( mLogLevel>=3, "waitCircularBufferEmpty -");
        dump();
        dumpBufferList();
    }
    MY_LOGD("-");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
dump() -> void
{

    String8 str = String8::format("current Map size(%zu): ", mCircularBufferMap.size() );
    for(size_t i=0; i<mCircularBufferMap.size(); i++) {
        str.appendFormat( "%" PRId64 "(%d) ", mCircularBufferMap[i].requestNo, mCircularBufferMap[i].status );
    }
    MY_LOGD("%s", str.string() );
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
dumpCircularBuffer() -> void
{
    for(size_t i=0; i<mCircularBufferMap.size(); i++) {
        String8 str;
        auto frameItem = mCircularBufferMap.valueAt(i);
        MY_LOGD("Map(%zu/%zu):(%p)-key(%" PRId64 ")/req(%" PRId64 ") shutter(%" PRIu64 ") status(%d) imageDone(%zu) metaDone(%zu)",
                i, mCircularBufferMap.size(), &frameItem, mCircularBufferMap.keyAt(i), frameItem.requestNo,
                frameItem.timestampShutter, frameItem.status, frameItem.imageDone, frameItem.metaDone);
        // dumpImageSet(mCircularBufferMap.editValueAt(i).bufferSet.vImageSet);
        // dumpMetaSet(mCircularBufferMap.editValueAt(i).bufferSet.vMetaSet);
    }
}



/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
dumpImageSet(ImageSet& imageSet) -> void
{
    String8 str = String8::format("ImageSet(%zu):", imageSet.size());
    for(size_t i=0; i<imageSet.size(); i++) {
        str += String8::format("(%zu):(%#" PRIx64 ":%p);",
            i, imageSet.keyAt(i), imageSet.editValueAt(i)->pStreamBuffer.get());
    }
    MY_LOGD("%s", str.string());
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
dumpMetaSet(MetaSet& metaSet) -> void
{
    String8 str = String8::format("MetaSet(%zu):", metaSet.size());
    for(size_t i=0; i<metaSet.size(); i++) {
        str += String8::format("(%zu):(%#" PRIx64 ":%p);",
            i, metaSet.keyAt(i), metaSet.editValueAt(i)->pStreamBuffer.get());
    }
    MY_LOGD("%s", str.string());
}



/******************************************************************************
 *
 ******************************************************************************/
auto
ZsdProcessorImp::
dumpBufferList() -> void
{
    int i=0;
    auto it = mBufferList.begin();
    while( it != mBufferList.end() ) {
        MY_LOGD("(%d:%p)", i, &(*it) );
        dumpImageSet((*it).vImageSet);
        dumpMetaSet((*it).vMetaSet);
        ++i;
        ++it;
    }
}