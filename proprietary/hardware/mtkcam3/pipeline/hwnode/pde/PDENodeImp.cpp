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

#define LOG_TAG "MtkCam/PDENode"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Sync.h>
#include <mtkcam3/pipeline/hwnode/PDENode.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
#include <mtkcam3/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/aaa/IHal3A.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
#define PDETHREAD_NAME       ("Cam@PDE")
#define PDETHREAD_POLICY     (SCHED_OTHER)
#define PDETHREAD_PRIORITY   (0)

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

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

/******************************************************************************
 *
 ******************************************************************************/
class PDENodeImp
    : public BaseNode
    , public PDENode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:

    class ExtractThread
        : public Thread
    {

    public:

                                    ExtractThread(PDENodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~ExtractThread()
                                    {}

    public:

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

    private:

                    PDENodeImp*       mpNodeImp;

    };

    //
public:     ////                    Operations.

                                    PDENodeImp();

                                    ~PDENodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  flush(
                                        android::sp<IPipelineFrame> const &pFrame
                                    );

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Data Members. (Request Queue)
    IHal3A*                         p3AHal;

protected:  ////                    Operations.

    MERROR                          onDequeRequest( //TODO: check frameNo
                                        android::sp<IPipelineFrame>& rpFrame
                                    );

    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           waitForRequestDrained();

protected:

    MERROR                          threadSetting();

protected:  ////                    Data Members. (Request Queue)
    mutable RWLock                  mConfigRWLock;
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit;

private:   ////                     Threads
    sp<ExtractThread>               mpExtractThread;

    sp<IMetaStreamInfo>             mpOutMetaStreamInfo_Result;
    sp<IMetaStreamInfo>             mpInMetaStreamInfo_P1Result;
    sp<IImageStreamInfo>            mpInImageStreamInfo_Raw;

};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<PDENode>
PDENode::
createInstance()
{
    MY_LOGD("PDENode createInstance");
    return new PDENodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
PDENodeImp::
PDENodeImp()
    : BaseNode()
    , PDENode()
    //
    , p3AHal(nullptr)
    //
    , mConfigRWLock()
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    //
    , mpExtractThread(NULL)
{
}


/******************************************************************************
 *
 ******************************************************************************/
PDENodeImp::
~PDENodeImp()
{
    MY_LOGD("~PDENodeImp");
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PDENodeImp::
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
    mpExtractThread = new ExtractThread(this);
    if( mpExtractThread->run(PDETHREAD_NAME) != OK ) {
        return UNKNOWN_ERROR;
    }
    //
    p3AHal = MAKE_Hal3A(getOpenId(), getNodeName());
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PDENodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    RWLock::AutoWLock _l(mConfigRWLock);
    //
    mpInMetaStreamInfo_P1Result = rParams.pInP1HalMeta;
    mpInImageStreamInfo_Raw    = rParams.pInRawImage;
    mpOutMetaStreamInfo_Result = rParams.pOutHalMeta;
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PDENodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mpOutMetaStreamInfo_Result = 0;
        mpInMetaStreamInfo_P1Result = 0;
        mpInImageStreamInfo_Raw = 0;
    }
    // exit threads
    mpExtractThread->requestExit();
    // join
    mpExtractThread->join();
    //
    mpExtractThread = NULL;
    //
    if(p3AHal)
    {
        p3AHal->destroyInstance(getNodeName());
        p3AHal = nullptr;
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PDENodeImp::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    return BaseNode::flush(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PDENodeImp::
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
PDENodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    MY_LOGD("+");
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

    size_t size = mRequestQueue.size();
    MY_LOGF_IF((size > 16), "PDE consumes too slow!!! #%zu buffers in queue", size);

    mRequestQueueCond.signal();
    //
    MY_LOGD("- Request queue size(%zu)", size);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PDENodeImp::
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
MVOID
PDENodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;
    //
    StreamId_T const        streamIdInMeta              = mpInMetaStreamInfo_P1Result->getStreamId();
    sp<IMetaStreamBuffer>   pInMetaStreamBuffer         = NULL;
    IMetadata*              pInMetadata                 = NULL;
    //
    StreamId_T const        streamIdOutMeta             = mpOutMetaStreamInfo_Result->getStreamId();
    sp<IMetaStreamBuffer>   pOutMetaStreamBuffer        = NULL;
    IMetadata*              pOutMetadata                = NULL;
    //
    StreamId_T const        streamIdInImageRaw          = mpInImageStreamInfo_Raw->getStreamId();
    sp<IImageStreamBuffer>  pInImageStreamBufferRaw     = NULL;
    sp<IImageBufferHeap>    pInImageBufferHeapRaw       = NULL;
    sp<IImageBuffer>        pInImageBufferRaw           = NULL;
    //
    IStreamInfoSet const& rStreamInfoSet __unused = pFrame->getStreamInfoSet();
    IStreamBufferSet& rStreamBufferSet            = pFrame->getStreamBufferSet();

    MINT32 success = 0;

    ////////////////////////////////////////////////////////////////////////////
    //  Ensure buffers available.
    ////////////////////////////////////////////////////////////////////////////
    //  Output Meta Stream: PDE Hal
    {
        StreamId_T const streamId = streamIdOutMeta;
        MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pOutMetaStreamBuffer
        );
        if( err != OK ) {
            MY_LOGW("cannot get output meta: streamId %#" PRIx64" of frame %d",
                    streamId,
                    pFrame->getFrameNo());
            goto lbExit;
        }
    }
    //  Input Meta Stream: P1 Hal
    {
        StreamId_T const streamId = streamIdInMeta;
        MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pInMetaStreamBuffer
        );
        if( err != OK ) {
            MY_LOGW("cannot get input meta: streamId %#" PRIx64" of frame %d",
                    streamId,
                    pFrame->getFrameNo());
            goto lbExit;
        }
    }
    //  Input Image Stream: Pure Raw
    {
        StreamId_T const streamId = streamIdInImageRaw;
        MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pInImageStreamBufferRaw
        );
        //Should check the returned error code!!!
        if( err != OK ) {
            MY_LOGW("cannot get input YUV: streamId %#" PRIx64" of frame %d",
                    streamId,
                    pFrame->getFrameNo());
            goto lbExit;
        }
    }

    success = 1;

    ////////////////////////////////////////////////////////////////////////////
    //  Prepare buffers before using.
    ////////////////////////////////////////////////////////////////////////////
    if(pInImageStreamBufferRaw)
    {
        {
            pOutMetadata = pOutMetaStreamBuffer->tryWriteLock(getNodeName());
        }
        //
        //  Input Meta Stream: Request
        {
            pInMetadata = pInMetaStreamBuffer->tryReadLock(getNodeName());
        }
        //
        //  Input Image Stream: YUV
        {
            //  Query the group usage.
            MUINT const groupUsage __unused = pInImageStreamBufferRaw->queryGroupUsage(getNodeId());

            //
            pInImageBufferHeapRaw = pInImageStreamBufferRaw->tryReadLock(getNodeName());
            if( !pInImageBufferHeapRaw)
            {
                MY_LOGE("tryReadLock pInImageStreamBufferRaw fail");
                goto lbExit;
            }

            //
            if( pInImageBufferHeapRaw->getImgFormat() == eImgFmt_BLOB)
            {
                std::vector<IImageBuffer*> vImageBuffer = pInImageBufferHeapRaw->createImageBuffers_FromBlobHeap(pInImageStreamBufferRaw->getStreamInfo()->getImageBufferInfo(), mNodeName);

                if( vImageBuffer.size()>0)
                {
                    pInImageBufferRaw = vImageBuffer[0];
                }
                else
                {
                    MY_LOGE("Can not create ImageBuffers from blob heap");
                }
            }
            else
            {
                pInImageBufferRaw = pInImageBufferHeapRaw->createImageBuffer();
            }

            if( !pInImageBufferRaw)
            {
                MY_LOGE("createImageBuffer fail");
                goto lbExit;
            }

            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
            pInImageBufferRaw->lockBuf(getNodeName(), usage);
        }
    }
    else
    {
        MY_LOGE("pInImageStreamBufferRaw is NULL");
        goto lbExit;
    }
    //********************** Do PDE Func **********************************//
#if 0
    MINT32 Type;
    MINT32 FrmNum;
    MINT32 MagicNum;
    {
        IMetadata::IEntry const& entry = pInMetadata->entryFor(MTK_P1NODE_RAW_TYPE);
        if(!entry.isEmpty())
        {
            Type = entry.itemAt(0, Type2Type<MINT32>());
        }
        else
        {
            Type = -1;
        }
    }
    {
        IMetadata::IEntry const& entry = pInMetadata->entryFor(MTK_PIPELINE_FRAME_NUMBER);
        if(!entry.isEmpty())
        {
            FrmNum = entry.itemAt(0, Type2Type<MINT32>());
        }
        else
        {
            FrmNum = -1;
        }
    }
    {
        IMetadata::IEntry const& entry = pInMetadata->entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM);
        if(!entry.isEmpty())
        {
            MagicNum = entry.itemAt(0, Type2Type<MINT32>());
        }
        else
        {
            MagicNum = -1;
        }
    }
    MY_LOGD("Info : type(%d), frmNum(%d), magicNum(%d)", Type, FrmNum, MagicNum);
#endif

    if(p3AHal)
    {
        p3AHal->send3ACtrl(E3ACtrl_SetPureRawData, (MINTPTR)pInMetadata, (MINTPTR)pInImageBufferRaw.get());
    }

    //*********************************************************************//
lbExit:
    //
    {
        StreamId_T const streamId = streamIdOutMeta;
        //
        if  ( pOutMetaStreamBuffer != NULL )
        {
            //Buffer Producer must set this status.
            pOutMetaStreamBuffer->markStatus(
                success ?
                STREAM_BUFFER_STATUS::WRITE_OK :
                STREAM_BUFFER_STATUS::WRITE_ERROR
            );
            pOutMetaStreamBuffer->unlock(getNodeName(), pOutMetadata);
            //
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        else
        {
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE
            );
        }
    }
    //
    //  Input Meta Stream: Request
    {
        StreamId_T const streamId = streamIdInMeta;
        //
        if  ( pInMetaStreamBuffer != NULL )
        {
            pInMetaStreamBuffer->unlock(getNodeName(), pInMetadata);
            //
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        else
        {
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE
            );
        }
    }
    //
    //  Input Image Stream: YUV
    {
        StreamId_T const streamId = streamIdInImageRaw;
        //
        if  ( pInImageStreamBufferRaw != NULL )
        {
            pInImageBufferRaw->unlockBuf(getNodeName());
            pInImageStreamBufferRaw->unlock(getNodeName(), pInImageBufferHeapRaw.get());
            //
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        else
        {
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE
            );
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    rStreamBufferSet.applyRelease(getNodeId());
    onDispatchFrame(pFrame);

    FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PDENodeImp::
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
PDENodeImp::
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
    if (PDETHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, PDETHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, PDETHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = PDETHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, PDETHREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), PDETHREAD_POLICY, PDETHREAD_PRIORITY);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
PDENodeImp::ExtractThread::
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
PDENodeImp::ExtractThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
PDENodeImp::ExtractThread::
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

