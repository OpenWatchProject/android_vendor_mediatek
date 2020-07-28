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

#define LOG_TAG "MtkCam/DualImageTransformNode"
//
#include "hwnode_utilities.h"

#include <mtkcam/pipeline/hwnode/DualImageTransformNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/feature/stereo/effecthal/DualITEffectHal.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <string>
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
/******************************************************************************
 *
 ******************************************************************************/
#define DUALIMAGETRANSFORMTHREAD_NAME       ("Cam@DualImageTransform")
#define DUALIMAGETRANSFORMTHREAD_POLICY     (SCHED_OTHER)
#define DUALIMAGETRANSFORMTHREAD_PRIORITY   (0)
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
static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}

/******************************************************************************
 *
 ******************************************************************************/
class DualImageTransformNodeImp
    : public BaseNode
    , public DualImageTransformNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:
    class TransformThread
        : public Thread
    {

    public:

                                    TransformThread(DualImageTransformNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~TransformThread()
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

                    DualImageTransformNodeImp*       mpNodeImp = nullptr;

    };

private:
    class process_frame
    {
        public:
            sp<IPipelineFrame> const    mpFrame = nullptr;

            sp<IImageBuffer>            mpJpeg_Main_buf = nullptr;
            sp<IImageBuffer>            mpJpeg_Main2_buf = nullptr;
            sp<IImageStreamBuffer>      mpInImgStreamBuffer_Main1 = nullptr;
            sp<IImageStreamBuffer>      mpInImgStreamBuffer_Main2 = nullptr;

            sp<IImageBuffer>            mpJps_Output_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImgStreamBuffer = nullptr;//
            //
            IMetadata*                  mpAppInMeta = nullptr;
            IMetadata*                  mpAppOutMeta = nullptr;
            IMetadata*                  mpHalInMeta = nullptr;
            IMetadata*                  mpHalOutMeta = nullptr;
            sp<IMetaStreamBuffer>       mpAppInMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpAppOutMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalInMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalOutMetaStreamBuffer = nullptr;
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
            sp<EffectRequest>           getRequest();
        private:
            NodeId_T                    mNodeId;
            char const*                 mNodeName;
    };
    //
public:     ////                    Operations.

                                    DualImageTransformNodeImp();

                                    ~DualImageTransformNodeImp();

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


    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        MBOOL const isInStream
                                    );
    MERROR                          suspendThisFrame(
                                        sp<IPipelineFrame> const& pFrame,
                                        process_frame*& data);
protected:

    MERROR                          threadSetting();

protected:  ////                    Data Members. (Request Queue)
    mutable RWLock                  mConfigRWLock;
    // meta
    sp<IMetaStreamInfo>             mpInAppMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutAppMetadata = nullptr;
    sp<IMetaStreamInfo>             mpInHalMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutHalMetadata = nullptr;
    // image
    sp<IImageStreamInfo>            mpInHalImageJpsMain1 = nullptr;
    sp<IImageStreamInfo>            mpInHalImageJpsMain2 = nullptr;
    sp<IImageStreamInfo>            mpOutHalImageJPS = nullptr;

    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained  = MFALSE;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit  = MFALSE;

private:   ////                     Threads
    sp<TransformThread>             mpTransformThread = nullptr;
    DualITEffectHal*                mpDualITEffectHal = nullptr;
    MBOOL                           metaInLock  = MFALSE;
};
/******************************************************************************
 *
 ******************************************************************************/
android::sp<DualImageTransformNode>
DualImageTransformNode::
createInstance()
{
    MY_LOGD("createInstance");
    return new DualImageTransformNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
DualImageTransformNodeImp::
DualImageTransformNodeImp()
    : BaseNode()
    , DualImageTransformNode()
    //
    , mConfigRWLock()
{
}


/******************************************************************************
 *
 ******************************************************************************/
DualImageTransformNodeImp::
~DualImageTransformNodeImp()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualImageTransformNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    MERROR ret = UNKNOWN_ERROR;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %d, name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpTransformThread = new TransformThread(this);
    if( mpTransformThread->run(DUALIMAGETRANSFORMTHREAD_NAME) != OK )
    {
        MY_LOGE("run thread failed.");
        goto lbExit;
    }
    //
    mpDualITEffectHal = new DualITEffectHal();
    if(mpDualITEffectHal== nullptr)
    {
        MY_LOGE("create dual it effecthal fail.");
        goto lbExit;
    }
    mpDualITEffectHal->init();
    ret = OK;

lbExit:
    FUNC_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualImageTransformNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    flush();

    {
        // meta
        mpInAppMetadata = rParams.pInAppMeta;
        mpOutAppMetadata = rParams.pOutAppMeta;
        mpInHalMetadata = rParams.pInHalMeta;
        mpOutHalMetadata = rParams.pOutHalMeta;
        // image
        mpInHalImageJpsMain1 = rParams.pInHalImageJpsMain1;
        mpInHalImageJpsMain2 = rParams.pInHalImageJpsMain2;
        mpOutHalImageJPS = rParams.pOutHalImageJPS;
        if(mpInAppMetadata==nullptr)
        {
            MY_LOGE("mpInAppMetadata is null");
            return UNKNOWN_ERROR;
        }
        if(mpOutAppMetadata==nullptr)
        {
            MY_LOGE("mpOutAppMetadata is null");
            return UNKNOWN_ERROR;
        }
        if(mpInHalMetadata==nullptr)
        {
            MY_LOGE("mpInHalMetadata is null");
            return UNKNOWN_ERROR;
        }
        if(mpOutHalMetadata==nullptr)
        {
            MY_LOGE("mpOutHalMetadata is null");
            return UNKNOWN_ERROR;
        }
    }
    //
    MY_LOGD("mpInHalImageJpsMain1:%dx%d mpInHalImageJpsMain2:%dx%d",
        mpInHalImageJpsMain1->getImgSize().w, mpInHalImageJpsMain1->getImgSize().h,
        mpInHalImageJpsMain2->getImgSize().w, mpInHalImageJpsMain2->getImgSize().h);
    //
    MY_LOGD("mpOutHalImageJPS:%dx%d",
        mpOutHalImageJPS->getImgSize().w, mpOutHalImageJPS->getImgSize().h);
    //
    if(mpDualITEffectHal!=nullptr)
    {
        mpDualITEffectHal->configure();
        mpDualITEffectHal->prepare();
        mpDualITEffectHal->start();
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualImageTransformNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpTransformThread->requestExit();
    // join
    mpTransformThread->join();
    //
    mpTransformThread = NULL;
    // release effectHal
    if(NULL != mpDualITEffectHal)
    {
        mpDualITEffectHal->abort();
        mpDualITEffectHal->release();
        mpDualITEffectHal->unconfigure();
        mpDualITEffectHal->uninit();
        delete mpDualITEffectHal;
        mpDualITEffectHal = NULL;
        MY_LOGD("Destroy dit effectHal.");
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualImageTransformNodeImp::
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
DualImageTransformNodeImp::
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
DualImageTransformNodeImp::
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
DualImageTransformNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;
    //
    IPipelineFrame::InfoIOMapSet IOMapSet;

    if(OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet ))
    {
        MY_LOGE("queryInfoIOMap failed, IOMap img/meta: %d/%d",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
                );
        return;
    }

    sp<IImageBuffer> main1;
    sp<IImageBuffer> main2;
    process_frame* pProcessFrame = NULL;
    {
        pProcessFrame = new process_frame(pFrame, getNodeId(), getNodeName());
        //
        {
            IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
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
            // get out app meta
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
            // get in hal meta
            if(OK!=ensureMetaBufferAvailable_(
                                    pFrame->getFrameNo(),
                                    mpInHalMetadata->getStreamId(),
                                    streamBufferSet,
                                    pProcessFrame->mpHalInMetaStreamBuffer))
            {
                MY_LOGE("get in hal meta fail.");
                suspendThisFrame(pFrame, pProcessFrame);
                return;
            }
            // get out hal meta
            if(OK!=ensureMetaBufferAvailable_(
                                    pFrame->getFrameNo(),
                                    mpOutHalMetadata->getStreamId(),
                                    streamBufferSet,
                                    pProcessFrame->mpHalOutMetaStreamBuffer))
            {
                MY_LOGE("get out hal meta fail.");
                suspendThisFrame(pFrame, pProcessFrame);
                return;
            }
        }
        // 1. get image stream buf and image buf.
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
        for( size_t i=0; i<imageIOMap.vIn.size() ; ++i )
        {
            StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
            if( isStream(mpInHalImageJpsMain1, streamId) )
            {
                // Main1 stream.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpInHalImageJpsMain1->getStreamId(),
                pProcessFrame->mpInImgStreamBuffer_Main1,
                pProcessFrame->mpJpeg_Main_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[main1] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpInHalImageJpsMain2, streamId) )
            {
                // Main2 stream.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpInHalImageJpsMain2->getStreamId(),
                pProcessFrame->mpInImgStreamBuffer_Main2,
                pProcessFrame->mpJpeg_Main2_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[main2] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }
        }
        {
            // 2. get JPS output stream
            StreamId_T const streamId_out = mpOutHalImageJPS->getStreamId();

            MERROR const err = getImageBufferAndLock(
                    pFrame,
                    mpOutHalImageJPS->getStreamId(),
                    pProcessFrame->mpOutImgStreamBuffer,
                    pProcessFrame->mpJps_Output_buf,
                    MFALSE
                    );

            if( err != OK ) {
                MY_LOGE("getImageBufferAndLock err = %d", err);
                suspendThisFrame(pFrame, pProcessFrame);
                return;
            }

            // we need to unlock JPS buf here because doTransform() will try to lock this buffer later
            pProcessFrame->mpJps_Output_buf->unlockBuf(mNodeName);
        }
    }

    {
        if(pProcessFrame->mpAppInMetaStreamBuffer!=nullptr &&
            pProcessFrame->mpAppOutMetaStreamBuffer!=nullptr)
        {
            MY_LOGD("copy data +");
            pProcessFrame->mpAppInMeta = pProcessFrame->mpAppInMetaStreamBuffer->tryReadLock(getNodeName());
            pProcessFrame->mpAppOutMeta = pProcessFrame->mpAppOutMetaStreamBuffer->tryWriteLock(getNodeName());
            *pProcessFrame->mpAppOutMeta = *pProcessFrame->mpAppInMeta;
            // JPS always does not rotate.
            IMetadata::IEntry entry(MTK_JPEG_ORIENTATION);
            entry.push_back(0, Type2Type<MINT32>());
            pProcessFrame->mpAppOutMeta->update(MTK_JPEG_ORIENTATION, entry);
            //
            MY_LOGD("copy data -");
        }
    }
    //produce p1 main2 hal dynamic
    {
        MY_LOGD("produce p1 main2 hal dynamic +");
        pProcessFrame->mpHalInMeta = pProcessFrame->mpHalInMetaStreamBuffer->tryReadLock(getNodeName());
        pProcessFrame->mpHalOutMeta = pProcessFrame->mpHalOutMetaStreamBuffer->tryWriteLock(getNodeName());

        if(pProcessFrame->mpHalInMeta == nullptr){
            MY_LOGE("cant get mpHalInMeta!");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        if(pProcessFrame->mpHalOutMeta == nullptr){
            MY_LOGE("cant get mpHalOutMeta!");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }

        if( ! tryGetMetadata<IMetadata>(pProcessFrame->mpHalInMeta, MTK_P1NODE_MAIN2_HAL_META, (*pProcessFrame->mpHalOutMeta)) ){
            MY_LOGE("cant get pProcessFrame->mpHalInMeta to pProcessFrame->mpHalOutMeta");
        }

        // {
        //     IMetadata exifMeta;

        //     MERROR ret = tryGetMetadata<IMetadata>(pProcessFrame->mpHalInMeta, MTK_3A_EXIF_METADATA, exifMeta);

        //     MY_LOGD_IF(!ret, "tryGetMetadata MTK_3A_EXIF_METADATA from mpHalInMeta failed!");
        // }

        // {
        //     IMetadata exifMeta;

        //     MERROR ret = tryGetMetadata<IMetadata>(pProcessFrame->mpHalOutMeta, MTK_3A_EXIF_METADATA, exifMeta);

        //     MY_LOGD_IF(!ret, "tryGetMetadata MTK_3A_EXIF_METADATA from mpHalOutMeta failed!");

        //     pProcessFrame->mpHalOutMeta->dump(0);
        // }

        MY_LOGD("produce p1 main2 hal dynamic -");
    }

    {
        // do some processing
        sp<EffectRequest> pRequest = pProcessFrame->getRequest();
        MBOOL transform_result = mpDualITEffectHal->updateEffectRequest(pRequest);
        pProcessFrame->uninit(transform_result);
        delete pProcessFrame;
        pProcessFrame = NULL;
    }
    onDispatchFrame(pFrame);

    FUNC_END;
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualImageTransformNodeImp::
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
DualImageTransformNodeImp::
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
MERROR
DualImageTransformNodeImp::
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
    MY_LOGD("@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD("stream buffer: (%p) %p, heap: %p, buffer: %p, usage: %p",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualImageTransformNodeImp::
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
    if (DUALIMAGETRANSFORMTHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, DUALIMAGETRANSFORMTHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, DUALIMAGETRANSFORMTHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = DUALIMAGETRANSFORMTHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, DUALIMAGETRANSFORMTHREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), DUALIMAGETRANSFORMTHREAD_POLICY, DUALIMAGETRANSFORMTHREAD_PRIORITY);

    return OK;

}
/******************************************************************************
 *
 ******************************************************************************/
void
DualImageTransformNodeImp::TransformThread::
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
DualImageTransformNodeImp::TransformThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
DualImageTransformNodeImp::TransformThread::
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
MBOOL
DualImageTransformNodeImp::process_frame::
uninit(MBOOL process_state)
{
    MBOOL ret = MTRUE;

    if(mpFrame == nullptr) return MTRUE;
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
        if(mpAppInMetaStreamBuffer!=nullptr)
        {
            mpAppInMetaStreamBuffer->unlock(mNodeName, mpAppInMeta);
            releaseMetaStream(mpFrame, mpAppInMetaStreamBuffer, mNodeId);
        }
        if(mpAppOutMetaStreamBuffer!=nullptr)
        {
            mpAppOutMetaStreamBuffer->unlock(mNodeName, mpAppOutMeta);
            releaseMetaStream(mpFrame, mpAppOutMetaStreamBuffer, mNodeId);
        }
        if(mpHalInMetaStreamBuffer!=nullptr)
        {
            mpHalInMetaStreamBuffer->unlock(mNodeName, mpHalInMeta);
            releaseMetaStream(mpFrame, mpHalInMetaStreamBuffer, mNodeId);
        }
        if(mpHalOutMetaStreamBuffer!=nullptr)
        {
            mpHalOutMetaStreamBuffer->unlock(mNodeName, mpHalOutMeta);
            releaseMetaStream(mpFrame, mpHalOutMetaStreamBuffer, mNodeId);
        }
    }

    // unlock and mark buffer state to main1 inStream.
    if(mpJpeg_Main_buf!=nullptr)
    {
        mpJpeg_Main_buf->unlockBuf(mNodeName);
        if(mpInImgStreamBuffer_Main1!=nullptr)
        {
            mpInImgStreamBuffer_Main1->unlock(mNodeName, mpJpeg_Main_buf->getImageBufferHeap());
            streamBufferSet.markUserStatus(
                        mpInImgStreamBuffer_Main1->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    // unlock and mark buffer state to main2 inStream.
    if(mpJpeg_Main2_buf!=nullptr)
    {
        mpJpeg_Main2_buf->unlockBuf(mNodeName);
        if(mpInImgStreamBuffer_Main2!=nullptr)
        {
            mpInImgStreamBuffer_Main2->unlock(mNodeName, mpJpeg_Main2_buf->getImageBufferHeap());
            streamBufferSet.markUserStatus(
                        mpInImgStreamBuffer_Main2->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream
    if(mpJps_Output_buf!=nullptr)
    {
        mpJps_Output_buf->unlockBuf(mNodeName);
        if(mpOutImgStreamBuffer!=nullptr)
        {
            mpOutImgStreamBuffer->unlock(mNodeName, mpJps_Output_buf->getImageBufferHeap());
            mpOutImgStreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImgStreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    streamBufferSet.applyRelease(mNodeId);
    mpAppInMeta = nullptr;
    mpAppInMetaStreamBuffer = nullptr;
    mpAppOutMeta = nullptr;
    mpAppOutMetaStreamBuffer = nullptr;
    mpHalInMeta = nullptr;
    mpHalInMetaStreamBuffer = nullptr;
    mpHalOutMeta = nullptr;
    mpHalOutMetaStreamBuffer = nullptr;
    //
    mpJpeg_Main_buf = nullptr;
    mpInImgStreamBuffer_Main1 = nullptr;
    mpJpeg_Main2_buf = nullptr;
    mpInImgStreamBuffer_Main2 = nullptr;
    mpJps_Output_buf = nullptr;
    mpOutImgStreamBuffer = nullptr;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<EffectRequest>
DualImageTransformNodeImp::process_frame::
getRequest()
{
    MINT32 bMainRight = StereoSettingProvider::getSensorRelativePosition();
    sp<EffectFrameInfo> pLeftFrame = new EffectFrameInfo(
                                            mpFrame->getRequestNo(),
                                            DualITEffectHal::DIT_BUF_JPS_LEFT,
                                            nullptr,
                                            nullptr);
    //
    sp<EffectFrameInfo> pRightFrame = new EffectFrameInfo(
                                            mpFrame->getRequestNo(),
                                            DualITEffectHal::DIT_BUF_JPS_RIGHT,
                                            nullptr,
                                            nullptr);
    MY_LOGD("bMainRight(%d)", bMainRight);
    if(bMainRight)
    {
        pLeftFrame->setFrameBuffer(mpJpeg_Main2_buf.get());
        pRightFrame->setFrameBuffer(mpJpeg_Main_buf.get());
    }
    else
    {
        pLeftFrame->setFrameBuffer(mpJpeg_Main_buf.get());
        pRightFrame->setFrameBuffer(mpJpeg_Main2_buf.get());
    }
    //
    sp<EffectFrameInfo> pOutFrame = new EffectFrameInfo(
                                            mpFrame->getRequestNo(),
                                            DualITEffectHal::DIT_BUF_JPS_OUTPUT,
                                            nullptr,
                                            nullptr);
    pOutFrame->setFrameBuffer(mpJps_Output_buf.get());
    //
    sp<EffectParameter> reqst_para = new EffectParameter();
    reqst_para->set(DIT_CONVERT_LEFT_IMG_FMT, MFALSE);
    reqst_para->set(DIT_CONVERT_RIGHT_IMG_FMT, MFALSE);
    //
    EffectRequestPtr pRequest = new EffectRequest(mpFrame->getRequestNo(), NULL, NULL);
    pRequest->setRequestParameter(reqst_para);
    pRequest->vInputFrameInfo.add(DualITEffectHal::DIT_BUF_JPS_LEFT, pLeftFrame);
    pRequest->vInputFrameInfo.add(DualITEffectHal::DIT_BUF_JPS_RIGHT, pRightFrame);
    pRequest->vOutputFrameInfo.add(DualITEffectHal::DIT_BUF_JPS_OUTPUT, pOutFrame);
    return pRequest;
}
