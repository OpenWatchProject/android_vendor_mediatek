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

#define LOG_TAG "MtkCam/IspTuningDataPackNode"
//
#include <stdlib.h>
#include <stdio.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Sync.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/IFDContainer.h>
#include <mtkcam3/pipeline/hwnode/IspTuningDataPackNode.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
#include <mtkcam3/pipeline/stream/IStreamBuffer.h>
#include "BaseNode.h"
#include "MyUtils.h"
#include <mtkcam3/pipeline/utils/packutils/PackUtils.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
//

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

/******************************************************************************
 *
 ******************************************************************************/
#define ISPPACKTHREAD_NAME       ("Cam@IspPack")
#define ISPPACKTHREAD_POLICY     (SCHED_OTHER)
#define ISPPACKTHREAD_PRIORITY   (0)

/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_PACK_NODE);
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

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

#define INIT_PACK_BUFFER_SIZE_FOR_RAW (6*1024*1024)
#define INIT_PACK_BUFFER_SIZE_FOR_YUV (2*1024*1024)
#define MIN_INCREASE_PACK_BUFFER_SIZE (2*1024*1024)

/******************************************************************************
 *
 ******************************************************************************/
class IspTuningDataPackNodeImp
    : public BaseNode
    , public IspTuningDataPackNode
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Implementations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:     ////Definitions.
        typedef android::sp<IPipelineFrame> QueNode_T;
        typedef android::List<QueNode_T> Que_T;

    protected:

        class PackThread
            : public Thread
        {
            public:
                PackThread(IspTuningDataPackNodeImp* pNodeImp)
                    : mpNodeImp(pNodeImp)
                {}
                ~PackThread()
                {}
            public:
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //  Thread Interface.
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            public:
                // Ask this object's thread to exit. This function is asynchronous, when the
                // function returns the thread might still be running. Of course, this
                // function can be called from a different thread.
                virtual void requestExit();
                // Good place to do one-time initializations
                virtual status_t readyToRun();
            private:
                // Derived class must implement threadLoop(). The thread starts its life
                // here. There are two ways of using the Thread object:
                // 1) loop: if threadLoop() returns true, it will be called again if
                //          requestExit() wasn't called.
                // 2) once: if threadLoop() returns false, the thread will exit upon return.
                virtual bool threadLoop();
            private:
                IspTuningDataPackNodeImp* mpNodeImp;
        };
        //
    public:
        IspTuningDataPackNodeImp();

        ~IspTuningDataPackNodeImp();

        virtual MERROR config(ConfigParams const& rParams);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  IPipelineNode Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        virtual MERROR init(InitParams const& rParams);

        virtual MERROR uninit();

        virtual MERROR flush();

        virtual MERROR flush(
                android::sp<IPipelineFrame> const &pFrame
                );

        virtual MERROR queue(
                android::sp<IPipelineFrame> pFrame
                );
    protected:
        MERROR allocateInternalBuffer(size_t allocateSize);
        MVOID releaseInternalBuffer();

    protected:
        MERROR onDequeRequest(
                android::sp<IPipelineFrame>& rpFrame
                );

        MVOID onProcessFrame(
                android::sp<IPipelineFrame> const& pFrame
                );

        MERROR unlockImgStream(android::sp<IPipelineFrame> const& pFrame,
                sp<IImageStreamBuffer>& pStreamBuffer,
                sp<IImageBuffer>& pImageBuffer,
                MBOOL const isOut = MFALSE,
                MBOOL const packSuccess = MFALSE);

        MVOID waitForRequestDrained();

        MERROR getImageBufferAndLock(
                android::sp<IPipelineFrame> const& pFrame,
                StreamId_T const streamId,
                sp<IImageStreamInfo> const streamInfo,
                sp<IImageStreamBuffer>& rpStreamBuffer,
                sp<IImageBuffer>& rpImageBuffer,
                MBOOL const isOutStream
                );

        MERROR errorHandle(
                android::sp<IPipelineFrame> const& pFrame
                );

        MVOID unlockImage(
                sp<IImageStreamBuffer>& rpStreamBuffer,
                sp<IImageBuffer>& rpImageBuffer
                );

        MERROR getMetadataAndLock(
                android::sp<IPipelineFrame> const& pFrame,
                StreamId_T const streamId,
                MBOOL isOut,
                sp<IMetaStreamBuffer>& rpStreamBuffer,
                IMetadata*& rpMetadata
                );

        MVOID returnMetadataAndUnlock(
                android::sp<IPipelineFrame> const& pFrame,
                StreamId_T const streamId,
                sp<IMetaStreamBuffer>  rpStreamBuffer,
                IMetadata* rpMetadata,
                MBOOL isOut = MFALSE,
                MBOOL success = MTRUE
                );

    protected:
        MERROR threadSetting();

    protected:
        MERROR processFrameWithIOMap(
                android::sp<IPipelineFrame> const& pFrame,
                MINT32 id,
                IPipelineFrame::ImageInfoIOMap const& imageIOMap,
                IPipelineFrame::MetaInfoIOMap const& imageMetaMap
                );
        MBOOL  processOnePackIOMapInfo(
                IPipelineFrame::MetaInfoIOMap&  metaIOMap,
                IPipelineFrame::ImageInfoIOMap& imageIOMap,
                MINT32& id,
                IPipelineFrame::ImageInfoIOMap& packedImageIOMap,
                IPipelineFrame::MetaInfoIOMap& packedMetaIOMap
                );
        MERROR releaseAllBuffers(
                android::sp<IPipelineFrame> const& pFrame,
                IPipelineFrame::InfoIOMapSet const& IOMapSet
                );

    protected:  //// Data Members. (Request Queue)
        mutable Mutex mRequestQueueLock;
        Condition mRequestQueueCond;
        Que_T mRequestQueue;
        MBOOL mbRequestDrained;
        Condition mbRequestDrainedCond;
        MBOOL mbRequestExit;

    private:

        std::unordered_map<MINT32, struct ConfigParams::PackStreamInfo> mpvStreamInfoMap;
        sp<PackThread> mpPackThread;
        sp<IImageBuffer> mpInternalPackBuffer;
        size_t mInternalPackBufferSize;
        IImageBufferAllocator* mAllocator = NULL;
        MINT32 mLogLevel;
        SrcType mSourceType;
        MINT32 mForcePackTuning = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IspTuningDataPackNode>
IspTuningDataPackNode::
createInstance()
{
    MY_LOGD("createInstance");
    return new IspTuningDataPackNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
IspTuningDataPackNodeImp::
    IspTuningDataPackNodeImp()
    : BaseNode()
    , IspTuningDataPackNode()
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    , mpPackThread(NULL)
    , mpInternalPackBuffer(NULL)
    , mInternalPackBufferSize(0)
    , mAllocator(NULL)
    , mLogLevel(0)
    , mSourceType(YuvFromP2)
{
    mNodeName = "IspTuningDataPackNode";//default name
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.IspTuningDataPackNode", 0);
    }
    mForcePackTuning = ::property_get_int32("vendor.debug.camera.packtuning.cshot", 0);
}


/******************************************************************************
 *
 ******************************************************************************/
IspTuningDataPackNodeImp::
~IspTuningDataPackNodeImp()
{
}

/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}

/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(std::vector<sp<IImageStreamInfo>> pvStreamInfo, StreamId_T streamId )
{
    std::vector<sp<IImageStreamInfo>>::iterator it = pvStreamInfo.begin();
    while ( it != pvStreamInfo.end() ) {
        if((*it)->getStreamId() == streamId)
            return MTRUE;
        it++;
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(std::vector<sp<IMetaStreamInfo>> pvStreamInfo, StreamId_T streamId )
{
    std::vector<sp<IMetaStreamInfo>>::iterator it = pvStreamInfo.begin();
    while ( it != pvStreamInfo.end() ) {
        if((*it)->getStreamId() == streamId)
            return MTRUE;
        it++;
    }
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
allocateInternalBuffer(size_t allocateSize)
{
    mAllocator = IImageBufferAllocator::getInstance();
    IImageBufferAllocator::ImgParam imgParam(allocateSize,0);
    mpInternalPackBuffer = mAllocator->alloc("mpInternalPackBuffer", imgParam);
    if ( mpInternalPackBuffer.get() == 0 )
    {
        MY_LOGE("NULL Buffer");
        return UNKNOWN_ERROR;
    }
    if ( !mpInternalPackBuffer->lockBuf( "mpInternalPackBuffer", (eBUFFER_USAGE_SW_MASK)) )
    {
        MY_LOGE("lock Buffer failed");
        return UNKNOWN_ERROR;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
IspTuningDataPackNodeImp::
releaseInternalBuffer()
{
    if(mpInternalPackBuffer != NULL && mAllocator != NULL)
    {
        mpInternalPackBuffer->unlockBuf("mpInternalPackBuffer");
        mAllocator->free(mpInternalPackBuffer.get());
        mpInternalPackBuffer = NULL;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpPackThread = new PackThread(this);
    if( mpPackThread->run(ISPPACKTHREAD_NAME) != OK ) {
        return UNKNOWN_ERROR;
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    // map for pysical stream
    mpvStreamInfoMap = rParams.pvStreamInfoMap;
    mSourceType = rParams.sourceType;

    // create internal buffer anyway
    {
        MY_LOGI("config output image info is NULL, will put tuning data to APP out(mpOutAppMetaInfo)");
        if (mpInternalPackBuffer == NULL)
        {
            if(mSourceType == YuvFromP2)
            {
                mInternalPackBufferSize = INIT_PACK_BUFFER_SIZE_FOR_YUV;
            }
            else
            {
                mInternalPackBufferSize = INIT_PACK_BUFFER_SIZE_FOR_RAW;
            }
            // for debug
            {
                ssize_t debugBufferSize = ::property_get_int32("vendor.debug.camera.packtuning.bufSize", -1);
                if(debugBufferSize > 0)
                {
                    MY_LOGI("Forced to set mInternalPackBufferSize(%zd)", debugBufferSize);
                    mInternalPackBufferSize = (size_t)debugBufferSize;
                }
            }
            //
            MY_LOGI("Allocate internal pack buffer, mInternalPackBufferSize = %d (mSourceType:%d)", mInternalPackBufferSize, mSourceType);
            MERROR ret = OK;
            ret = allocateInternalBuffer(mInternalPackBufferSize);
            if(ret != OK)
            {
                MY_LOGE("allocateInternalBuffer fail! mInternalPackBufferSize(%zu)",mInternalPackBufferSize);
                return ret;
            }
        }
        else
        {
            MY_LOGW("Already have mpInternalPackBuffer, is it correct flow?");
        }
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() ) {
        MY_LOGE("flush failed");
        return BAD_VALUE;
    }
    //
    // exit threads
    mpPackThread->requestExit();
    // join
    mpPackThread->join();
    //
    mpPackThread = NULL;
    //
    releaseInternalBuffer();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    return BaseNode::flush(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
flush()
{
    FUNC_START;
    //
    // 1. clear requests
    {
        Mutex::Autolock _l(mRequestQueueLock);
        //
        Que_T::iterator it = mRequestQueue.begin();
        while ( it != mRequestQueue.end() ) {
            BaseNode::flush(*it);
            it = mRequestQueue.erase(it);
        }
    }
    //
    // 2. wait enque thread
    waitForRequestDrained();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mRequestQueueLock);

    //  Make sure the request with a smaller frame number has a higher priority.
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin(); ) {
        --it;
        if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
            ++it;   //insert(): insert before the current node
            break;
        }
    }
    mRequestQueue.insert(it, pFrame);

    mRequestQueueCond.signal();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
onDequeRequest(
        android::sp<IPipelineFrame>& rpFrame
        )
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Wait until the queue is not empty or not going exit
    while ( mRequestQueue.empty() && ! mbRequestExit )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();
        //
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                    "wait status:%d:%s, mRequestQueue.size:%zu",
                    status, ::strerror(-status), mRequestQueue.size()
                   );
        }
    }
    //
    if  ( mbRequestExit ) {
        MY_LOGW_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }
    //
    //  Here the queue is not empty, take the first request from the queue.
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IspTuningDataPackNodeImp::
processOnePackIOMapInfo(
        IPipelineFrame::MetaInfoIOMap&  metaIOMap,
        IPipelineFrame::ImageInfoIOMap& imageIOMap,
        MINT32& id,
        IPipelineFrame::ImageInfoIOMap& packedImageIOMap,
        IPipelineFrame::MetaInfoIOMap& packedMetaIOMap
        )
{
    MBOOL found = false;
    for (auto const& it : mpvStreamInfoMap)
    {
        android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamInfo> > vMetaOut = metaIOMap.vOut;
        for ( size_t i = 0; i < vMetaOut.size(); i++ )
        {
            StreamId_T const streamId = vMetaOut.keyAt(i);
            if(isStream(it.second.pOutAppMetaInfo, streamId))
            {
                found = true;
                id = it.first;
                packedMetaIOMap.vOut.add(streamId, it.second.pOutAppMetaInfo);
                metaIOMap.vOut.removeItem(streamId);
                MY_LOGD("found app out meta(%#" PRIx64 ")", streamId);
                break;
            }
        }
        if (found)
        {
            break;
        }

    }
    if (!found)
    {
        return found;
    }
    //
    android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamInfo> > vIn = imageIOMap.vIn;
    android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamInfo> > vOut = imageIOMap.vOut;
    for ( size_t i = 0; i < vIn.size(); i++ )
    {
        StreamId_T const streamId = vIn.keyAt(i);
        if(isStream(mpvStreamInfoMap[id].pvInImgInfo_statistics, streamId))
        {
            packedImageIOMap.vIn.add(streamId, vIn.valueAt(i));
            MY_LOGD("found statistics(%#" PRIx64 ")", streamId);
        }
        if(isStream(mpvStreamInfoMap[id].pvInImgInfo_rrzo, streamId))
        {
            packedImageIOMap.vIn.add(streamId, vIn.valueAt(i));
            MY_LOGD("found rrzo(%#" PRIx64 ")", streamId);
        }
    }
    for ( size_t i = 0; i < vOut.size(); i++ )
    {
        StreamId_T const streamId = vOut.keyAt(i);
        if(isStream(mpvStreamInfoMap[id].pOutImgInfo, streamId))
        {
            packedImageIOMap.vOut.add(streamId, vOut.valueAt(i));
            MY_LOGD("found outImg(%#" PRIx64 ")", streamId);
        }
    }
    //
    android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamInfo> > vMetaIn = metaIOMap.vIn;
    android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamInfo> > vMetaOut = metaIOMap.vOut;
    for ( size_t i = 0; i < vMetaIn.size(); i++ )
    {
        StreamId_T const streamId = vMetaIn.keyAt(i);
        if(isStream(mpvStreamInfoMap[id].pvInAppCtlMetaInfo, streamId))
        {
            packedMetaIOMap.vIn.add(streamId, vMetaIn.valueAt(i));
            MY_LOGD("found ctrl app meta(%#" PRIx64 ")", streamId);
        }
        if(isStream(mpvStreamInfoMap[id].pvInP1AppMetaInfo, streamId))
        {
            packedMetaIOMap.vIn.add(streamId, vMetaIn.valueAt(i));
            MY_LOGD("found p1 app result meta(%#" PRIx64 ")", streamId);
        }
        if(isStream(mpvStreamInfoMap[id].pvInP2AppMetaInfo, streamId))
        {
            packedMetaIOMap.vIn.add(streamId, vMetaIn.valueAt(i));
            MY_LOGD("found p2 app result meta(%#" PRIx64 ")", streamId);
        }
        if(isStream(mpvStreamInfoMap[id].pvInHalMetaInfo, streamId))
        {
            packedMetaIOMap.vIn.add(streamId, vMetaIn.valueAt(i));
            MY_LOGD("found hal result meta(%#" PRIx64 ")", streamId);
        }
    }
    return found;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
processFrameWithIOMap(
        android::sp<IPipelineFrame> const& pFrame,
        MINT32 id,
        IPipelineFrame::ImageInfoIOMap const& imageIOMap,
        IPipelineFrame::MetaInfoIOMap const& metaIOMap
        )
{
    FUNC_START;

    MBOOL bPackSuccess = MFALSE;
    MERROR err = OK;

    sp<IImageStreamInfo> streamOutInfo = NULL;
    sp<IImageStreamInfo> streamInRRzInfo = NULL;
    sp<IImageStreamInfo> streamInSttInfo = NULL;
    struct ConfigParams::PackStreamInfo streamSet = mpvStreamInfoMap[id];
    for( size_t i = 0; i < imageIOMap.vIn.size(); i++ ) {
        StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
        if (isStream(streamSet.pvInImgInfo_rrzo, streamId)) {
            streamInRRzInfo = imageIOMap.vIn.valueAt(i);
        } else if (isStream(streamSet.pvInImgInfo_statistics, streamId)) {
            streamInSttInfo = imageIOMap.vIn.valueAt(i);
        }
    }

    for (size_t i = 0; i < imageIOMap.vOut.size(); i++) {
        StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
        if (isStream(streamSet.pOutImgInfo, streamId)) {
            streamOutInfo = imageIOMap.vOut.valueAt(i);
        }
    }
    if (streamOutInfo == NULL) {
        MY_LOGD("No output image stream in request");
    }
    //
    sp<IMetaStreamInfo> streamOutMetaInfo = NULL;
    sp<IMetaStreamInfo> streamInCtrlAppMetaInfo = NULL;
    sp<IMetaStreamInfo> streamInP1AppMetaInfo = NULL;
    sp<IMetaStreamInfo> streamInP2AppMetaInfo = NULL;
    sp<IMetaStreamInfo> streamInHalMetaInfo = NULL;
    for( size_t i = 0; i < metaIOMap.vIn.size(); i++ ) {
        StreamId_T const streamId = metaIOMap.vIn.keyAt(i);
        if (isStream(streamSet.pvInAppCtlMetaInfo, streamId)) {
            streamInCtrlAppMetaInfo = metaIOMap.vIn.valueAt(i);
        } else if (isStream(streamSet.pvInP1AppMetaInfo, streamId)) {
            streamInP1AppMetaInfo = metaIOMap.vIn.valueAt(i);
        } else if (isStream(streamSet.pvInP2AppMetaInfo, streamId)) {
            streamInP2AppMetaInfo = metaIOMap.vIn.valueAt(i);
        } else if (isStream(streamSet.pvInHalMetaInfo, streamId)) {
            streamInHalMetaInfo = metaIOMap.vIn.valueAt(i);
        }
    }

    for (size_t i = 0; i < metaIOMap.vOut.size(); i++) {
        StreamId_T const streamId = metaIOMap.vOut.keyAt(i);
        if (isStream(streamSet.pOutAppMetaInfo, streamId)) {
            streamOutMetaInfo = metaIOMap.vOut.valueAt(i);
        }
    }
    if (streamOutMetaInfo == NULL) {
        MY_LOGD("No output meta stream in request");
    }
    //
    {
        IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
        sp<IMetaStreamBuffer> pInMetaStream_P1AppResult = NULL;
        sp<IMetaStreamBuffer> pInMetaStream_P2AppResult = NULL;
        sp<IMetaStreamBuffer> pInMetaStream_HalResult = NULL;
        sp<IMetaStreamBuffer> pInMetaStream_AppControl = NULL;
        sp<IMetaStreamBuffer> pOutMetaStream_AppPackRet = NULL;

        IMetadata* pInMeta_P1AppResult = NULL;
        IMetadata* pInMeta_P2AppResult = NULL;
        IMetadata* pInMeta_HalResult = NULL;
        IMetadata* pInMeta_AppControl = NULL;
        IMetadata* pOutMeta_AppResult = NULL;

        //handle input stream
        sp<IImageStreamBuffer> pInRRzStreamBuffer = NULL;
        sp<IImageBuffer> pInRRzBuffer = NULL;
        sp<IImageStreamBuffer> pInSttStreamBuffer = NULL;
        sp<IImageBuffer> pInSttBuffer = NULL;
        //handle output stream
        sp<IImageStreamBuffer> pOutImageStreamBuffer = NULL;
        sp<IImageBuffer> pOutImageBuffer = NULL;
        {
            //p1 app result meta
            if(streamInP1AppMetaInfo.get())
            {
                err = getMetadataAndLock(
                        pFrame,
                        streamInP1AppMetaInfo->getStreamId(),
                        MFALSE,
                        pInMetaStream_P1AppResult,
                        pInMeta_P1AppResult
                        );
                if (err != OK) {
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamInP1AppMetaInfo is NULL, skip getMetadataAndLock()");
            }
            //p2 app result meta
            if(streamInP2AppMetaInfo.get())
            {
                err = getMetadataAndLock(
                        pFrame,
                        streamInP2AppMetaInfo->getStreamId(),
                        MFALSE,
                        pInMetaStream_P2AppResult,
                        pInMeta_P2AppResult
                        );
                if (err != OK) {
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamInP2AppMetaInfo is NULL, skip getMetadataAndLock()");
            }
            //hal result meta
            if(streamInHalMetaInfo.get())
            {
                err = getMetadataAndLock(
                        pFrame,
                        streamInHalMetaInfo->getStreamId(),
                        MFALSE,
                        pInMetaStream_HalResult,
                        pInMeta_HalResult
                        );

                if( mLogLevel >= 0x10 ){
                    MY_LOGD("HalResult dump +");
                    pInMeta_HalResult->dump();
                    MY_LOGD("HalResult dump -");
                }

                if (err != OK) {
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamInHalMetaInfo is NULL, skip getMetadataAndLock()");
            }
            //ctrl app meta
            if(streamInCtrlAppMetaInfo.get())
            {
                err = getMetadataAndLock(
                        pFrame,
                        streamInCtrlAppMetaInfo->getStreamId(),
                        MFALSE,
                        pInMetaStream_AppControl,
                        pInMeta_AppControl
                        );
                if (err != OK) {
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamInCtrlAppMetaInfo is NULL, skip getMetadataAndLock()");
            }
            //
            if(streamOutMetaInfo.get())
            {
                err = getMetadataAndLock(
                        pFrame,
                        streamOutMetaInfo->getStreamId(),
                        MTRUE,
                        pOutMetaStream_AppPackRet,
                        pOutMeta_AppResult
                        );
                if (err != OK) {
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamOutMetaInfo is NULL, skip getMetadataAndLock()");
            }
        }
        {// out
            if(streamOutInfo != NULL)
            {
                err = getImageBufferAndLock(
                        pFrame,
                        streamSet.pOutImgInfo->getStreamId(),
                        streamOutInfo,
                        pOutImageStreamBuffer,
                        pOutImageBuffer,
                        MTRUE
                        );

                if( err != OK ) {
                    MY_LOGE("getImageBufferAndLock err = %d", err);
                    unlockImage(pOutImageStreamBuffer, pOutImageBuffer);
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("mpOutImgInfo is NULL, skip getImageBufferAndLock()");
            }
        }
        { // rrzo
            if (streamInRRzInfo != NULL) {
                err = getImageBufferAndLock(
                        pFrame,
                        streamInRRzInfo->getStreamId(),
                        streamInRRzInfo,
                        pInRRzStreamBuffer,
                        pInRRzBuffer,
                        MFALSE
                        );
                if(err != OK) {
                    MY_LOGE("getImageBufferAndLock err = %d", err);
                    unlockImage(pInRRzStreamBuffer, pInRRzBuffer);
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamInRRzInfo is null!");
            }
        }
        { // Stto
            if (streamInSttInfo != NULL) {
                err = getImageBufferAndLock(
                        pFrame,
                        streamInSttInfo->getStreamId(),
                        streamInSttInfo,
                        pInSttStreamBuffer,
                        pInSttBuffer,
                        MFALSE
                        );
                if(err != OK) {
                    MY_LOGE("getImageBufferAndLock err = %d", err);
                    unlockImage(pInSttStreamBuffer, pInSttBuffer);
                    errorHandle(pFrame);
                    return err;
                }
            }
            else
            {
                MY_LOGD("streamInSttInfo is null!");
            }
        }
        if (pOutImageBuffer == NULL && pOutMeta_AppResult == NULL) {
            MY_LOGE("No output image stream in request and no output app metadata");
            errorHandle(pFrame);
            err = UNKNOWN_ERROR;
            return err;
        }
        // Pack ISP Tunning Data
        IMetadata::IEntry HalTime = pInMeta_HalResult->entryFor(MTK_P1NODE_FRAME_START_TIMESTAMP);
        MINT64 HalTimeStamp = 0;
        if( !HalTime.isEmpty() )
        {
            HalTimeStamp = HalTime.itemAt(0, Type2Type<MINT64>());
        }
        else
        {
            HalTimeStamp = pFrame->getSensorTimestamp();
            MY_LOGW("cannot get MTK_P1NODE_FRAME_START_TIMESTAMP, use pFrame->getSensorTimestamp() : %lld", HalTimeStamp);
        }
        IMetadata::IEntry sensorEntry = pInMeta_HalResult->entryFor(MTK_HAL_REQUEST_SENSOR_ID);
        MINT32 sensorId = -1;
        if( !sensorEntry.isEmpty() )
        {
            sensorId = sensorEntry.itemAt(0, Type2Type<MINT32>());
        }
        else
        {
            MY_LOGW("cannot get MTK_HAL_REQUEST_SENSOR_ID");
        }
        auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
        auto fdData = fdReader->queryLock((HalTimeStamp - 500000000), HalTimeStamp);
        MUINT8 *pfdDataBuffer = NULL;
        MUINT32 fdDataSize = 0;
        MERROR pack_ret = OK;
        MY_LOGD("get FD data : %d", fdData.size());
        for (auto idx = fdData.size(); idx != 0; idx--)
        {
            auto fdChunk = fdData[idx - 1];
            if (CC_LIKELY( fdChunk != NULL ) && (sensorId == -1 || sensorId == fdChunk->sensorId))
            {
                MY_LOGD("Number_of_faces: %d",fdChunk->facedata.number_of_faces);
                pfdDataBuffer = (MUINT8 *)&(fdChunk->facedata);
                fdDataSize = sizeof(fdChunk->facedata);
                MY_LOGD("fd data size: %d", fdDataSize);
                break;
            }
        }
        IspTuningDataPackUtil PackUtils;
        IspTuningDataPackUtil::PackInputParam packParam;
        android::sp<IImageBufferHeap> pOutImgBufHeap;
        MBOOL needAPPRet = false;
        MINT32 totalPackSize = 0;
        MINT32 isCShot = 0;
        IMetadata::Tag_t updateTag = -1;
        packParam.requestNo = pFrame->getRequestNo();
        packParam.frameNo = pFrame->getFrameNo();
        packParam.timestamp = pFrame->getSensorTimestamp();
        packParam.haltimestamp = HalTimeStamp;
        packParam.appControlMeta = *pInMeta_AppControl;
        packParam.halResultMeta = *pInMeta_HalResult;
        packParam.pSttoImageBufferHeap = pInSttBuffer.get() ? pInSttBuffer->getImageBufferHeap() : NULL;
        packParam.pRrzoImageBufferHeap = pInRRzBuffer.get() ? pInRRzBuffer->getImageBufferHeap() : NULL;
        packParam.pRrzoStreamInfo = streamInRRzInfo;
        // remove rrzo tag first
        packParam.halResultMeta.remove(MTK_FEATURE_BSS_RRZO_DATA);

        // check is cshot or not
        /*bool ret = IMetadata::getEntry<MINT32>(&(packParam.appControlMeta), MTK_CSHOT_FEATURE_CAPTURE, isCShot);
        if (ret && isCShot && mSourceType == YuvFromP2 && mForcePackTuning == 0)
        {
            MY_LOGD("is cshot with yuv pack, skip pack to use jpeg sw encode");
            goto _EXIT_;
        }*/
        //
        // FD Info
        packParam.pFDData = pfdDataBuffer;
        packParam.FDDataSize = fdDataSize;
        switch (mSourceType)
        {
            case YuvFromP2:
                packParam.appResultMeta = (*pInMeta_P1AppResult) + (*pInMeta_P2AppResult);
                updateTag = MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_YUV;
                break;
            case RawFromP1:
                packParam.appResultMeta = *pInMeta_P1AppResult;
                updateTag = MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_RAW;
                break;
            case RawFromP2:
                packParam.appResultMeta = (*pInMeta_P1AppResult) + (*pInMeta_P2AppResult);
                updateTag = MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_RAW;
                break;
            default:
                break;
        }
        if (pOutImageBuffer != NULL)
        {
            pOutImgBufHeap = pOutImageBuffer->getImageBufferHeap();
        }
        else if (mpInternalPackBuffer != NULL)
        {
            needAPPRet = true;
            pOutImgBufHeap = mpInternalPackBuffer->getImageBufferHeap();
        }
        else
        {
            MY_LOGE("Neither has pOutImageBuffer nor mpInternalPackBuffer");
            errorHandle(pFrame);
            err = UNKNOWN_ERROR;
            return err;
        }
        pack_ret = PackUtils.packIspTuningDataToHeap(pOutImgBufHeap, totalPackSize, packParam);
        if(pack_ret == NO_MEMORY)
        {
            //if internal buffer size is not enough, need to allocate bigger buffer and retry pack flow again
            size_t retryCnt = 0;
            const size_t MAX_RETRY_CNT = 5;
            bool retrySuccess = false;
            for(retryCnt=0; retryCnt < MAX_RETRY_CNT; retryCnt++)
            {
                size_t newSize = mInternalPackBufferSize + PackUtils.getInsufficientBufferSize() + MIN_INCREASE_PACK_BUFFER_SIZE;
                MY_LOGW("retry packIspTuningDataToHeap:(%zu/%zu) mInternalPackBufferSize:old(%zu) new(%zu)",retryCnt,MAX_RETRY_CNT,mInternalPackBufferSize,newSize);
                //(1) release old buffer
                releaseInternalBuffer();
                pOutImgBufHeap = NULL;
                //(2) calcu the new buffer size (original size + PackUtils Insufficient Buffer Size + min backup buffer size)
                mInternalPackBufferSize = newSize;
                //(3) allocate bigger buffer
                MERROR ret = allocateInternalBuffer(mInternalPackBufferSize);
                if(ret!=OK)
                {
                    MY_LOGE("allocateInternalBuffer fail! mInternalPackBufferSize(%zu)",mInternalPackBufferSize);
                }
                else
                {
                    //(4) retry pack flow
                    pOutImgBufHeap = mpInternalPackBuffer->getImageBufferHeap();
                    pack_ret = PackUtils.packIspTuningDataToHeap(pOutImgBufHeap, totalPackSize, packParam);
                    if(pack_ret!=NO_MEMORY)
                    {
                        retrySuccess = true;
                        break;
                    }
                }
            }
            if(retrySuccess)
            {
                MY_LOGD("retry packIspTuningDataToHeap successfully! (mInternalPackBufferSize=%zu)",mInternalPackBufferSize);
            }
            else
            {
                MY_LOGE("retry packIspTuningDataToHeap fail! Still NO_MEMORY (mInternalPackBufferSize=%zu)",mInternalPackBufferSize);
            }
        }
        if(pack_ret == OK)
        {
            bPackSuccess = MTRUE;
            MY_LOGD("Pack isp meta(srcType:%d) to heap successfully!", mSourceType);
            if (needAPPRet)
            {
                MY_LOGD("Pack to APP result. size = %d", totalPackSize);
                IMetadata::IEntry entry(updateTag);
                entry.push_back((MUINT8 *)(mpInternalPackBuffer->getBufVA(0)), totalPackSize, Type2Type<MUINT8>());
                //entry.setCapacity(totalPackSize);
                //entry.setSize(totalPackSize);
                //memcpy((void *)(entry.data()), (void *)(mpInternalPackBuffer->getBufVA(0)), totalPackSize);
                pOutMeta_AppResult->update(updateTag, entry);
                // debug
                //pOutMeta_AppResult->dump();
            }
        }
        else
        {
            bPackSuccess = MFALSE;
            MY_LOGE("Pack isp meta(srcType:%d) to heap fail! ret=%d", mSourceType, pack_ret);
        }
 _EXIT_:
        fdReader->queryUnlock(fdData);
        //
        if (streamInRRzInfo != NULL)
            unlockImgStream(pFrame, pInRRzStreamBuffer, pInRRzBuffer);
        if (streamInSttInfo != NULL)
            unlockImgStream(pFrame, pInSttStreamBuffer, pInSttBuffer);
        if (streamOutInfo != NULL)
            unlockImgStream(pFrame, pOutImageStreamBuffer, pOutImageBuffer, MTRUE, bPackSuccess);
        //
        if(streamInP1AppMetaInfo.get())
        {
            returnMetadataAndUnlock(
                    pFrame,
                    streamInP1AppMetaInfo->getStreamId(),
                    pInMetaStream_P1AppResult,
                    pInMeta_P1AppResult,
                    MFALSE
                    );
        }
        if(streamInP2AppMetaInfo.get())
        {
            returnMetadataAndUnlock(
                    pFrame,
                    streamInP2AppMetaInfo->getStreamId(),
                    pInMetaStream_P2AppResult,
                    pInMeta_P2AppResult,
                    MFALSE
                    );
        }
        if(streamInHalMetaInfo.get())
        {
            returnMetadataAndUnlock(
                    pFrame,
                    streamInHalMetaInfo->getStreamId(),
                    pInMetaStream_HalResult,
                    pInMeta_HalResult,
                    MFALSE
                    );
        }
        if(streamInCtrlAppMetaInfo.get())
        {
            returnMetadataAndUnlock(
                    pFrame,
                    streamInCtrlAppMetaInfo->getStreamId(),
                    pInMetaStream_AppControl,
                    pInMeta_AppControl,
                    MFALSE
                    );
        }
        if(streamOutMetaInfo.get())
        {
            returnMetadataAndUnlock(
                    pFrame,
                    streamOutMetaInfo->getStreamId(),
                    pOutMetaStream_AppPackRet,
                    pOutMeta_AppResult,
                    MTRUE
                    );
        }

        //////////////////////////////////////////////////////////////
#if 0   //Test Code for unpack isp tuning data
        IspTuningDataPackUtil::UnpackOutputParam unpackParam;
        android::sp<IImageBufferHeap> pInImgBufHeap = pOutImageBuffer->getImageBufferHeap();
        pInImgBufHeap->lockBuf("TestUnpack");
        MERROR ret = IspTuningDataPackUtil::unpackIspTuningDataFromHeap(pInImgBufHeap,unpackParam);
         pInImgBufHeap->unlockBuf("TestUnpack");
        if(ret == OK)
        {
            MY_LOGD("unpackIspTuningDataFromHeap Test: success");
        }
        else
        {
            MY_LOGE("unpackIspTuningDataFromHeap Test: fail! ret=%d",ret);
        }
#endif  //Test Code End
        //////////////////////////////////////////////////////////////

        // release
        //streamBufferSet.applyRelease(getNodeId());
    }


    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
releaseAllBuffers(
        android::sp<IPipelineFrame> const& pFrame,
        IPipelineFrame::InfoIOMapSet const& IOMapSet
)
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();

    #define RELEASE_BUFFER(_streamid_) \
        rStreamBufferSet.markUserStatus( \
            _streamid_, getNodeId(), \
            IUsersManager::UserStatus::USED | \
            IUsersManager::UserStatus::RELEASE \
            );
    for (size_t i=0; i<IOMapSet.mImageInfoIOMapSet.size(); i++)
    {
        for ( size_t j = 0; j < IOMapSet.mImageInfoIOMapSet[i].vIn.size(); j++ )
        {
            StreamId_T const streamId = IOMapSet.mImageInfoIOMapSet[i].vIn.keyAt(j);
            RELEASE_BUFFER(streamId);
        }
        for ( size_t j = 0; j < IOMapSet.mImageInfoIOMapSet[i].vOut.size(); j++ )
        {
            StreamId_T const streamId = IOMapSet.mImageInfoIOMapSet[i].vOut.keyAt(j);
            RELEASE_BUFFER(streamId);
        }
    }

    for (size_t i=0; i<IOMapSet.mMetaInfoIOMapSet.size(); i++)
    {
        for ( size_t j = 0; j < IOMapSet.mMetaInfoIOMapSet[i].vIn.size(); j++ )
        {
            StreamId_T const streamId = IOMapSet.mMetaInfoIOMapSet[i].vIn.keyAt(j);
            RELEASE_BUFFER(streamId);
        }
        for ( size_t j = 0; j < IOMapSet.mMetaInfoIOMapSet[i].vOut.size(); j++ )
        {
            StreamId_T const streamId = IOMapSet.mMetaInfoIOMapSet[i].vOut.keyAt(j);
            RELEASE_BUFFER(streamId);
        }
    }

    rStreamBufferSet.applyRelease(getNodeId());
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
IspTuningDataPackNodeImp::
onProcessFrame(
        android::sp<IPipelineFrame> const& pFrame
        )
{
    FUNC_START;
    //
    IPipelineFrame::InfoIOMapSet IOMapSet;//IOMapSet.mImageInfoIOMapSet.begin()
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )
            || IOMapSet.mImageInfoIOMapSet.size() == 0
      ) {
        MY_LOGE("queryInfoIOMap failed, IOMap img/meta: %zu/%zu",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
               );
        errorHandle(pFrame);
        return;
    }
    //
    if(IOMapSet.mImageInfoIOMapSet.size() != IOMapSet.mMetaInfoIOMapSet.size())
    {
        MY_LOGE("IOMapSet.mImageInfoIOMapSet.size(%zu) != IOMapSet.mMetaInfoIOMapSet.size(%zu)",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
               );
        errorHandle(pFrame);
        return;
    }
    //
    for (size_t i=0; i<IOMapSet.mImageInfoIOMapSet.size(); i++)
    {
        MY_LOGD("process IOMap : (%zu/%zu)",i,IOMapSet.mImageInfoIOMapSet.size());
        IPipelineFrame::ImageInfoIOMap imageIOMap = IOMapSet.mImageInfoIOMapSet[i];
        IPipelineFrame::MetaInfoIOMap  metaIOMap =  IOMapSet.mMetaInfoIOMapSet[i];
        IPipelineFrame::ImageInfoIOMap packedImageIOMap;
        IPipelineFrame::MetaInfoIOMap packedMetaIOMap;
        MINT32 id = 0;
        while (processOnePackIOMapInfo(metaIOMap, imageIOMap, id, packedImageIOMap, packedMetaIOMap))
        {
            MERROR err = processFrameWithIOMap(pFrame, id, packedImageIOMap, packedMetaIOMap);
            if(err != OK)
            {
                errorHandle(pFrame);
                return;
            }
            packedImageIOMap.vIn.clear();
            packedImageIOMap.vOut.clear();
        }
    }

    releaseAllBuffers(pFrame, IOMapSet);

    onDispatchFrame(pFrame);

    FUNC_END;
    return;
}

MERROR
IspTuningDataPackNodeImp::
unlockImgStream(android::sp<IPipelineFrame> const& pFrame,
    sp<IImageStreamBuffer>& pStreamBuffer,
    sp<IImageBuffer>& pImageBuffer,
    MBOOL const isOut,
    MBOOL const packSuccess)
{
    FUNC_START;
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    //unlock inputstream
    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());

    if (isOut) {
        pStreamBuffer->markStatus(
                packSuccess ?
                STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                );
    }

    /*streamBufferSet.markUserStatus(
            pStreamBuffer->getStreamInfo()->getStreamId(),
            getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE
            );*/

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
IspTuningDataPackNodeImp::
waitForRequestDrained()
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained ) {
        MY_LOGD("wait for request drained");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
getImageBufferAndLock(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IImageStreamInfo> const streamInfo,
        sp<IImageStreamBuffer>& rpStreamBuffer,
        sp<IImageBuffer>& rpImageBuffer,
        MBOOL const isOutStream
        )
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
        return err;
    //
    //  Query the group usage.
    MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());

    if(!isOutStream){
        pImageBufferHeap = rpStreamBuffer->tryReadLock(getNodeName());
    }
    else{
        pImageBufferHeap = rpStreamBuffer->tryWriteLock(getNodeName());
    }

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    MY_LOGD("@pImageBufferHeap->getBufSizeInBytes(0) = %zu", pImageBufferHeap->getBufSizeInBytes(0));
    if (pImageBufferHeap->getImgFormat() == eImgFmt_BLOB) {
        std::vector<IImageBuffer*> vImageBuffer
            = pImageBufferHeap->createImageBuffers_FromBlobHeap(streamInfo->getImageBufferInfo(), LOG_TAG);
        if (vImageBuffer.size() > 0)
            rpImageBuffer = vImageBuffer[0];
    } else {
        rpImageBuffer = pImageBufferHeap->createImageBuffer();
    }

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    //
    if(groupUsage==0x0)
    {
        rpImageBuffer->lockBuf(getNodeName());
    }
    else
    {
        rpImageBuffer->lockBuf(getNodeName(), groupUsage);
    }
    //
    MY_LOGD("stream buffer: (%#" PRIx64 ") %p, heap: %p, buffer: %p, usage: 0x%x",
            streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (ISPPACKTHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, ISPPACKTHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, ISPPACKTHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = ISPPACKTHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, ISPPACKTHREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), ISPPACKTHREAD_POLICY, ISPPACKTHREAD_PRIORITY);

    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
void
IspTuningDataPackNodeImp::PackThread::
requestExit()
{
    //TODO: refine this
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
IspTuningDataPackNodeImp::PackThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}

/******************************************************************************
 *
 ******************************************************************************/
bool
IspTuningDataPackNodeImp::PackThread::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            !exitPending()
            &&  OK == mpNodeImp->onDequeRequest(pFrame)
            &&  pFrame != 0
        )
    {
        mpNodeImp->onProcessFrame(pFrame);
        return true;
    }

    MY_LOGD("exit unpack thread");
    return  false;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
errorHandle(
        android::sp<IPipelineFrame> const& pFrame
        )
{
    MY_LOGE("Discard frameNo=%d", pFrame->getRequestNo());
    return BaseNode::flush(pFrame);
}

/******************************************************************************
 *
 ******************************************************************************/

MVOID
IspTuningDataPackNodeImp::
unlockImage(
        sp<IImageStreamBuffer>& rpStreamBuffer,
        sp<IImageBuffer>& rpImageBuffer
        )
{
    if( rpStreamBuffer == NULL || rpImageBuffer == NULL ) {
        MY_LOGE("rpStreamBuffer %p, rpImageBuffer %p should not be NULL",
                rpStreamBuffer.get(), rpImageBuffer.get());
        return;
    }
    rpImageBuffer->unlockBuf(getNodeName());
    rpStreamBuffer->unlock(getNodeName(), rpImageBuffer->getImageBufferHeap());
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
IspTuningDataPackNodeImp::
getMetadataAndLock(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        MBOOL isOut,
        sp<IMetaStreamBuffer>& rpStreamBuffer,
        IMetadata*& rpMetadata
        )
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    MY_LOGD_IF(1, "nodeID %#" PRIxPTR " streamID %#" PRIxPTR " ",getNodeId(), streamId);
    MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
    {
        MY_LOGD_IF( rpStreamBuffer == NULL, "streamId(%#" PRIxPTR ") meta streamBuf not exit", streamId);
        return err;
    }

    rpMetadata = !isOut ?
        rpStreamBuffer->tryReadLock(getNodeName()) :
        rpStreamBuffer->tryWriteLock(getNodeName());

    if( rpMetadata == NULL ) {
        MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] cannot get metadata",
                pFrame->getFrameNo(), getNodeId(), rpStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(1,"stream %#" PRIx64 ": stream buffer %p, metadata: %p",
            streamId, rpStreamBuffer.get(), rpMetadata);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
IspTuningDataPackNodeImp::
returnMetadataAndUnlock(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IMetaStreamBuffer>  rpStreamBuffer,
        IMetadata* rpMetadata,
        MBOOL isOut,
        MBOOL success
        )
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    //
    if( rpStreamBuffer.get() == NULL ) {
        MY_LOGE("StreamId %#" PRIx64 ": rpStreamBuffer == NULL",
                streamId);
        return;
    }
    //
    //Buffer Producer must set this status.
    if( isOut ) {
        if  ( success ) {
            rpStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        }
        else {
            rpStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }
    //
    if( rpMetadata )
        rpStreamBuffer->unlock(getNodeName(), rpMetadata);
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    /*rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE
            );*/
}
