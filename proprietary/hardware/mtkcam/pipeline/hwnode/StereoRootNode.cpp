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

#define LOG_TAG "MtkCam/StereoRootNode"
//
#include "hwnode_utilities.h"

#include <mtkcam/pipeline/hwnode/StereoRootNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

/******************************************************************************
 *
 ******************************************************************************/
#define STEREOROOTTHREAD_NAME       ("Cam@StereoRootNode")
#define STEREOROOTTHREAD_POLICY     (SCHED_OTHER)
#define STEREOROOTTHREAD_PRIORITY   (0)

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
class StereoRootNodeImp
    : public BaseNode
    , public StereoRootNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;
private:
    class process_frame
    {
        public:
            typedef IPipelineFrame::NodeId_T            NodeId_T;
            sp<IPipelineFrame> const    mpFrame = nullptr;
            //
            IMetadata*                  mpAppInMeta = nullptr;
            IMetadata*                  mpAppOutMeta = nullptr;
            IMetadata*                  mpHalInMeta = nullptr;
            IMetadata*                  mpHalOutMeta = nullptr;
            IMetadata*                  mpHalOutMeta_Main2 = nullptr;
            sp<IMetaStreamBuffer>       mpAppInMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpAppOutMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalInMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalOutMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalOutMetaStreamBuffer_Main2 = nullptr;
            //
            sp<IImageBuffer>            mpMain1RRZOImgBuffer = nullptr;
            sp<IImageStreamBuffer>      mpMain1RRZOStreamBuffer = nullptr;
            //
            process_frame(
                        sp<IPipelineFrame> const pFrame,
                        NodeId_T nodeId,
                        char const* nodeName
                        ):
                        mpFrame(pFrame),
                        mNodeId(nodeId),
                        mNodeName(nodeName)
            {
            }
            MBOOL                       uninit(MBOOL process_state);
        private:
            NodeId_T                    mNodeId;
            char const*                 mNodeName;
    };

protected:

    class UnpackThread
        : public Thread
    {

    public:

                                    UnpackThread(StereoRootNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~UnpackThread()
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

                    StereoRootNodeImp*       mpNodeImp;

    };

    //
public:     ////                    Operations.

                                    StereoRootNodeImp();

                                    ~StereoRootNodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.

    MERROR                          onDequeRequest( //TODO: check frameNo
                                        android::sp<IPipelineFrame>& rpFrame
                                    );

    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           waitForRequestDrained();

    MERROR                          suspendThisFrame(
                                        sp<IPipelineFrame> const& pFrame,
                                        process_frame*& data);

    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        MBOOL const isInStream
                                    );
protected:

    MERROR                          threadSetting();

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit;

private:   ////                     Threads
    sp<UnpackThread>                mpUnpackThread;
    //
    sp<IMetaStreamInfo>             mpInAppMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutAppMetadata = nullptr;
    sp<IMetaStreamInfo>             mpInHalMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutHalMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutHalMetadata_Main2 = nullptr;
    //sp<IImageStreamInfo>            mpInHalImageRRZMain1 = NULL;

};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<StereoRootNode>
StereoRootNode::
createInstance()
{
    MY_LOGD("createInstance");
    return new StereoRootNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
StereoRootNodeImp::
StereoRootNodeImp()
    : BaseNode()
    , StereoRootNode()
    //
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    //
    , mpUnpackThread(NULL)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
StereoRootNodeImp::
~StereoRootNodeImp()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoRootNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %d, name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpUnpackThread = new UnpackThread(this);
    if( mpUnpackThread->run(STEREOROOTTHREAD_NAME) != OK ) {
        return UNKNOWN_ERROR;
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoRootNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    mpInAppMetadata = rParams.pInAppMeta;
    mpOutAppMetadata = rParams.pOutAppMeta;

    mpInHalMetadata = rParams.pInHalMeta;
    mpOutHalMetadata = rParams.pOutHalMeta;
    mpOutHalMetadata_Main2 = rParams.pOutHalMeta_Main2;
    //mpInHalImageRRZMain1 = rParams.pInResizeImageInfo;
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoRootNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpUnpackThread->requestExit();
    // join
    mpUnpackThread->join();
    //
    mpUnpackThread = NULL;
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoRootNodeImp::
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
StereoRootNodeImp::
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
StereoRootNodeImp::
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
MERROR
StereoRootNodeImp::
suspendThisFrame(
    sp<IPipelineFrame> const& pFrame,
    process_frame*& data)
{
    MY_LOGE("Discard request id=%d", pFrame->getRequestNo());

    data->uninit(MFALSE);
    delete data;
    data = nullptr;

    MERROR err = BaseNode::flush(pFrame);

    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoRootNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;
    //
    process_frame* pProcessFrame = NULL;
    //
    {
        pProcessFrame = new process_frame(pFrame, getNodeId(), getNodeName());
        IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
        // app input metadata
        if(OK!=ensureMetaBufferAvailable_(
                                pFrame->getFrameNo(),
                                mpInAppMetadata->getStreamId(),
                                streamBufferSet,
                                pProcessFrame->mpAppInMetaStreamBuffer))
        {
            MY_LOGE("get in app meta fail.");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        // get in hal meta ====================================================
        if(OK!=ensureMetaBufferAvailable_(
                                pFrame->getFrameNo(),
                                mpInHalMetadata->getStreamId(),
                                streamBufferSet,
                                pProcessFrame->mpHalInMetaStreamBuffer))
        {
            MY_LOGE("get In Hal meta fail.");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        // app output metadata
        if(OK!=ensureMetaBufferAvailable_(
                                pFrame->getFrameNo(),
                                mpOutAppMetadata->getStreamId(),
                                streamBufferSet,
                                pProcessFrame->mpAppOutMetaStreamBuffer))
        {
            MY_LOGE("get Out app meta fail.");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        // get out hal meta ====================================================
        if(OK!=ensureMetaBufferAvailable_(
                                pFrame->getFrameNo(),
                                mpOutHalMetadata->getStreamId(),
                                streamBufferSet,
                                pProcessFrame->mpHalOutMetaStreamBuffer))
        {
            MY_LOGE("get Out Hal meta fail.");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        // get out hal meta main2 ====================================================
        if(OK!=ensureMetaBufferAvailable_(
                                pFrame->getFrameNo(),
                                mpOutHalMetadata_Main2->getStreamId(),
                                streamBufferSet,
                                pProcessFrame->mpHalOutMetaStreamBuffer_Main2))
        {
            MY_LOGE("get Out Hal meta main2 fail.");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        MY_LOGD("---- get main2 Hal meta ----");
        IMetadata pSplitHalMeta_Main2;
        // get main2 metadata
        pProcessFrame->mpHalOutMeta = pProcessFrame->mpHalOutMetaStreamBuffer->tryReadLock(getNodeName());
        if( ! tryGetMetadata<IMetadata>(const_cast<IMetadata*>(pProcessFrame->mpHalOutMeta),
                                        MTK_P1NODE_MAIN2_HAL_META,
                                        pSplitHalMeta_Main2) )
        {
            MY_LOGE("get hal main2 meta data fail.");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }
        //
        IMetadata pSplitAppMeta_Main2;
        MBOOL bContainMain2App = MFALSE;
        if( tryGetMetadata<IMetadata>(const_cast<IMetadata*>(pProcessFrame->mpHalOutMeta),
                                        MTK_P1NODE_MAIN2_APP_META,
                                        pSplitAppMeta_Main2) )
        {
            MY_LOGD("get app main2 meta data.");
            bContainMain2App = MTRUE;
        }
        pProcessFrame->mpHalOutMetaStreamBuffer->unlock(getNodeName(), pProcessFrame->mpHalOutMeta);

        MY_LOGD("---- set main2 Hal meta to main2 out meta ----");
        pProcessFrame->mpHalOutMeta_Main2 = pProcessFrame->mpHalOutMetaStreamBuffer_Main2->tryWriteLock(getNodeName());
        *pProcessFrame->mpHalOutMeta_Main2 = pSplitHalMeta_Main2;
        pProcessFrame->mpHalOutMetaStreamBuffer_Main2->unlock(getNodeName(), pProcessFrame->mpHalOutMeta_Main2);

        if(bContainMain2App)
        {
            MY_LOGD("---- set main2 App meta to stereo out meta ----");
            pProcessFrame->mpAppOutMeta = pProcessFrame->mpAppOutMetaStreamBuffer->tryWriteLock(getNodeName());
            if(pProcessFrame->mpAppOutMeta != nullptr)
            {
                *pProcessFrame->mpAppOutMeta = pSplitAppMeta_Main2;
                pProcessFrame->mpAppOutMetaStreamBuffer->unlock(getNodeName(), pProcessFrame->mpAppOutMeta);
            }
            else
            {
                MY_LOGE("cannot get output metadata");
            }
        }

        // debug info
        MY_LOGD("mpAppInMetaStreamBuffer:%p, nodeName:%s, id:%#" PRIx64 ", name:%s",
            pProcessFrame->mpAppInMetaStreamBuffer.get(),
            getNodeName(),
            pProcessFrame->mpAppInMetaStreamBuffer->getStreamInfo()->getStreamId(),
            pProcessFrame->mpAppInMetaStreamBuffer->getStreamInfo()->getStreamName());
        MY_LOGD("mpHalInMetaStreamBuffer:%p, nodeName:%s, id:%#" PRIx64 ", name:%s",
            pProcessFrame->mpHalInMetaStreamBuffer.get(),
            getNodeName(),
            pProcessFrame->mpHalInMetaStreamBuffer->getStreamInfo()->getStreamId(),
            pProcessFrame->mpHalInMetaStreamBuffer->getStreamInfo()->getStreamName());
        MY_LOGD("mpHalOutMetaStreamBuffer:%p, nodeName:%s, id:%#" PRIx64 ", name:%s",
            pProcessFrame->mpHalOutMetaStreamBuffer.get(),
            getNodeName(),
            pProcessFrame->mpHalOutMetaStreamBuffer->getStreamInfo()->getStreamId(),
            pProcessFrame->mpHalOutMetaStreamBuffer->getStreamInfo()->getStreamName());
        MY_LOGD("mpAppOutMetaStreamBuffer:%p, nodeName:%s, id:%#" PRIx64 ", name:%s",
            pProcessFrame->mpAppOutMetaStreamBuffer.get(),
            getNodeName(),
            pProcessFrame->mpAppOutMetaStreamBuffer->getStreamInfo()->getStreamId(),
            pProcessFrame->mpAppOutMetaStreamBuffer->getStreamInfo()->getStreamName());
        MY_LOGD("mpHalOutMetaStreamBuffer_Main2:%p, nodeName:%s, id:%#" PRIx64 ", name:%s",
            pProcessFrame->mpHalOutMetaStreamBuffer_Main2.get(),
            getNodeName(),
            pProcessFrame->mpHalOutMetaStreamBuffer_Main2->getStreamInfo()->getStreamId(),
            pProcessFrame->mpHalOutMetaStreamBuffer_Main2->getStreamInfo()->getStreamName());
        if(eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2 == pProcessFrame->mpHalOutMetaStreamBuffer_Main2->getStreamInfo()->getStreamId())
        {
            MY_LOGD("IS eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2.");
        }
        else
        {
            MY_LOGD("NOT eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2.");
        }

        MY_LOGD("---- done. ----");
        /*MERROR const err1 = getImageBufferAndLock(
                            pFrame,
                            mpInHalImageRRZMain1->getStreamId(),
                            pProcessFrame->mpMain1RRZOStreamBuffer,
                            pProcessFrame->mpMain1RRZOImgBuffer,
                            MTRUE
                            );

        if( err1 != OK) {
            MY_LOGE("getImageBufferAndLock err = %d StreamId(%d)", err1, mpInHalImageRRZMain1->getStreamId());
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }*/
    }
    {
        /*if(pProcessFrame->mpAppInMetaStreamBuffer!=nullptr &&
            pProcessFrame->mpAppOutMetaStreamBuffer!=nullptr)
        {
            // maybe can do something here.
        }*/
    }
    pProcessFrame->uninit(MTRUE);
    delete pProcessFrame;
    pProcessFrame = NULL;
    onDispatchFrame(pFrame);

    FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoRootNodeImp::
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
StereoRootNodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (STEREOROOTTHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, STEREOROOTTHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, STEREOROOTTHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = STEREOROOTTHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, STEREOROOTTHREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), STEREOROOTTHREAD_POLICY, STEREOROOTTHREAD_PRIORITY);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoRootNodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    MBOOL const isInStream
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    if(streamId == 0)
    {
        MY_LOGE("StreamId is 0");
        return BAD_VALUE;
    }
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

    if(isInStream){
        pImageBufferHeap = rpStreamBuffer->tryReadLock(getNodeName());
    }
    else{
        pImageBufferHeap = rpStreamBuffer->tryWriteLock(getNodeName());
    }

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    //MY_LOGD_IF(mLogLevel>0, "@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    //MY_LOGD_IF(mLogLevel>0, "stream buffer: (%p) %p, heap: %p, buffer: %p, usage: %p",
    //    streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
StereoRootNodeImp::UnpackThread::
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
StereoRootNodeImp::UnpackThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
StereoRootNodeImp::UnpackThread::
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
    return  false;

}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoRootNodeImp::process_frame::
uninit(MBOOL process_state)
{
    MBOOL ret = MTRUE;

    IStreamBufferSet& streamBufferSet = mpFrame->getStreamBufferSet();

    {
        /******************************************************************************
         *
         ******************************************************************************/
        auto releaseMetaStream = [](sp<IPipelineFrame> const& pFrame,
                                    sp<IMetaStreamBuffer> const pStreamBuffer,
                                    Pipeline_NodeId_T nodeId)
        {
            IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
            StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
            //
            pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
            //
            //  Mark this buffer as USED by this user.
            //  Mark this buffer as RELEASE by this user.
            rStreamBufferSet.markUserStatus(
                                            streamId, nodeId,
                                            IUsersManager::UserStatus::USED |
                                            IUsersManager::UserStatus::RELEASE
                                            );
        };
        // unlock meta
        //mpAppInMetaStreamBuffer->unlock(mNodeName, mpAppInMeta);
        releaseMetaStream(mpFrame, mpAppInMetaStreamBuffer, mNodeId);
        //mpAppOutMetaStreamBuffer->unlock(mNodeName, mpAppOutMeta);
        releaseMetaStream(mpFrame, mpAppOutMetaStreamBuffer, mNodeId);

        releaseMetaStream(mpFrame, mpHalInMetaStreamBuffer, mNodeId);
        releaseMetaStream(mpFrame, mpHalOutMetaStreamBuffer, mNodeId);
        releaseMetaStream(mpFrame, mpHalOutMetaStreamBuffer_Main2, mNodeId);
    }
    /*{
        if(mpMain1RRZOImgBuffer != nullptr && mpMain1RRZOStreamBuffer!=nullptr)
        {
            mpMain1RRZOImgBuffer->unlockBuf(mNodeName);
            mpMain1RRZOStreamBuffer->unlock(mNodeName, mpMain1RRZOImgBuffer->getImageBufferHeap());
            IStreamBufferSet& rStreamBufferSet = mpFrame->getStreamBufferSet();
            StreamId_T const streamId = mpMain1RRZOStreamBuffer->getStreamInfo()->getStreamId();
            mpMain1RRZOStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
            //
            //  Mark this buffer as USED by this user.
            //  Mark this buffer as RELEASE by this user.
            rStreamBufferSet.markUserStatus(
                    streamId, mNodeId,
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
            );
}
    }*/

    streamBufferSet.applyRelease(mNodeId);
    mpAppInMeta = nullptr;
    mpAppInMetaStreamBuffer = nullptr;
    mpAppOutMeta = nullptr;
    mpAppOutMetaStreamBuffer = nullptr;
    mpHalInMeta = nullptr;
    mpHalInMetaStreamBuffer = nullptr;
    mpMain1RRZOStreamBuffer = nullptr;
    mpMain1RRZOImgBuffer = nullptr;
    return ret;
}