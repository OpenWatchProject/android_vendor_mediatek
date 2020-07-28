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

#define LOG_TAG "MtkCam/DualYUVNode"
//
#include "hwnode_utilities.h"

#include <mtkcam/pipeline/hwnode/DualYUVNode.h>
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
#include <mtkcam/feature/stereo/effecthal/YUVEffectHal.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
#include <chrono>
#include <string>
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
/******************************************************************************
 *
 ******************************************************************************/
#define DUALYUVTHREAD_NAME       ("Cam@DualYUV")
#define DUALYUVTHREAD_POLICY     (SCHED_OTHER)
#define DUALYUVTHREAD_PRIORITY   (0)
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

#define MY_LOGD1(...)               MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF(2<=mLogLevel, __VA_ARGS__)

#define FUNC_START     MY_LOGD1("+")
#define FUNC_END       MY_LOGD1("-")
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
class DualYUVNodeImp
    : public BaseNode
    , public DualYUVNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:
    class DualYUVThread
        : public Thread
    {

    public:

                                    DualYUVThread(DualYUVNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~DualYUVThread()
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

                    DualYUVNodeImp*       mpNodeImp = nullptr;

    };

private:
    class process_frame
    {
        public:
            sp<IPipelineFrame> const    mpFrame = nullptr;

            // In
            sp<IImageBuffer>            mpInFullRaw_Main1_buf = nullptr;
            sp<IImageBuffer>            mpInFullRaw_Main2_buf = nullptr;
            sp<IImageBuffer>            mpInReSizeRaw_Main1_buf = nullptr;
            sp<IImageBuffer>            mpInReSizeRaw_Main2_buf = nullptr;
            sp<IImageStreamBuffer>      mpInFullRaw_Main1_StreamBuffer = nullptr;
            sp<IImageStreamBuffer>      mpInFullRaw_Main2_StreamBuffer = nullptr;
            sp<IImageStreamBuffer>      mpInReSizeRaw_Main1_StreamBuffer = nullptr;
            sp<IImageStreamBuffer>      mpInReSizeRaw_Main2_StreamBuffer = nullptr;

            // Out
            // preview
            sp<IImageBuffer>            mpOutImage_Main1_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImage_Main1_StreamBuffer = nullptr;
            sp<IImageBuffer>            mpOutImage_FD_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImage_FD_StreamBuffer = nullptr;
            sp<IImageBuffer>            mpOutImage_Prv_CB_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImage_Prv_CB_StreamBuffer = nullptr;
            // capture
            sp<IImageBuffer>            mpOutImage_Main1_Cap_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImage_Main1_CapStreamBuffer = nullptr;
            sp<IImageBuffer>            mpOutImage_Main2_Cap_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImage_Main2_Cap_StreamBuffer = nullptr;
            // thumbnail
            sp<IImageBuffer>            mpOutImage_Thumbnail_Cap_buf = nullptr;
            sp<IImageStreamBuffer>      mpOutImage_Thumbnail_Cap_StreamBuffer = nullptr;


            // Meta
            IMetadata*                  mpAppInMeta = nullptr;
            IMetadata*                  mpAppOutMeta = nullptr;
            IMetadata*                  mpHalInMeta = nullptr;
            IMetadata*                  mpHalInMeta_Main2 = nullptr;
            IMetadata*                  mpHalOutMeta = nullptr;
            sp<IMetaStreamBuffer>       mpAppInMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpAppOutMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalInMetaStreamBuffer = nullptr;
            sp<IMetaStreamBuffer>       mpHalInMetaStreamBuffer_Main2 = nullptr;
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

                                    DualYUVNodeImp();

                                    ~DualYUVNodeImp();

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
    static MVOID                    onEffectRequestFinished(MVOID* tag, String8 status, sp<EffectRequest>& request);
    MERROR                          onEffectReqDone(String8 status, sp<EffectRequest>& request);



protected:  ////                    Data Members. (Request Queue)
    mutable RWLock                  mConfigRWLock;
    // meta
    sp<IMetaStreamInfo>             mpInAppMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutAppMetadata = nullptr;
    sp<IMetaStreamInfo>             mpInHalMetadata = nullptr;
    sp<IMetaStreamInfo>             mpOutHalMetadata = nullptr;
    // image
    sp<IImageStreamInfo>            mpInFullRaw = nullptr;
    sp<IImageStreamInfo>            mpInFullRaw_Main2 = nullptr;
    sp<IImageStreamInfo>            mpInResizeRaw = nullptr;
    sp<IImageStreamInfo>            mpInResizeRaw_Main2 = nullptr;
    sp<IImageStreamInfo>            mpOutHalImageMainImg = nullptr;
    sp<IImageStreamInfo>            mpOutHalImageFD = nullptr;
    sp<IImageStreamInfo>            mpOutHalImagePrv_CB = nullptr;
    sp<IImageStreamInfo>            mpOutHalImage_Main1_Img_Cap = nullptr;
    sp<IImageStreamInfo>            mpOutHalImage_Main2_Img_Cap = nullptr;
    sp<IImageStreamInfo>            mpOutHalImage_Thumbnail_Img_Cap = nullptr;

    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained  = MFALSE;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit  = MFALSE;

private:   ////                     Threads
    sp<DualYUVThread>               mpDualYUVThread = nullptr;
    YUVEffectHal*                   mpDualYUVEffectHal = nullptr;
    MBOOL                           metaInLock  = MFALSE;
    MINT32                          mLogLevel;
};
/******************************************************************************
 *
 ******************************************************************************/
android::sp<DualYUVNode>
DualYUVNode::
createInstance()
{
    MY_LOGD("createInstance");
    return new DualYUVNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
DualYUVNodeImp::
DualYUVNodeImp()
    : BaseNode()
    , DualYUVNode()
    , mConfigRWLock()
{
    mLogLevel = 1;
}


/******************************************************************************
 *
 ******************************************************************************/
DualYUVNodeImp::
~DualYUVNodeImp()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
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
    mpDualYUVThread = new DualYUVThread(this);
    if( mpDualYUVThread->run(DUALYUVTHREAD_NAME) != OK )
    {
        MY_LOGE("run thread failed.");
        goto lbExit;
    }
    //
    mpDualYUVEffectHal = new YUVEffectHal();
    if(mpDualYUVEffectHal== nullptr)
    {
        MY_LOGE("create DualYUV effecthal fail.");
        goto lbExit;
    }
    mpDualYUVEffectHal->init();
    ret = OK;

lbExit:
    FUNC_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
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
        // In
        mpInFullRaw = rParams.pInFullRaw;
        mpInFullRaw_Main2 = rParams.pInFullRaw_Main2;
        mpInResizeRaw = rParams.pInResizedRaw;
        mpInResizeRaw_Main2 = rParams.pInResizedRaw_Main2;

        // Out
        mpOutHalImageMainImg = rParams.pOutHalImageMainImg;
        mpOutHalImageFD = rParams.pOutHalImageFD;
        mpOutHalImagePrv_CB = rParams.pOutAppPrvCB;
        mpOutHalImage_Main1_Img_Cap = rParams.pOutHalImageMainImg_Cap;
        mpOutHalImage_Main2_Img_Cap = rParams.pOutHalImageMain2Img_Cap;
        mpOutHalImage_Thumbnail_Img_Cap = rParams.pOutHalThumbnailImg_Cap;

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
    auto dumpStreamInfo = [](const sp<IImageStreamInfo>& info)
    {
        if(info == nullptr)
        {
            return;
        }
        MY_LOGD("streamInfo(%#08x) name(%s) format(%#08x) size(%dx%d)",
                info->getStreamId(), info->getStreamName(),
                info->getImgFormat(), info->getImgSize().w, info->getImgSize().h);
    };
    dumpStreamInfo(mpInFullRaw);
    dumpStreamInfo(mpInFullRaw_Main2);
    dumpStreamInfo(mpInResizeRaw);
    dumpStreamInfo(mpInResizeRaw_Main2);
    dumpStreamInfo(mpOutHalImageMainImg);
    dumpStreamInfo(mpOutHalImageFD);
    dumpStreamInfo(mpOutHalImagePrv_CB);
    dumpStreamInfo(mpOutHalImage_Main1_Img_Cap);
    dumpStreamInfo(mpOutHalImage_Main2_Img_Cap);
    dumpStreamInfo(mpOutHalImage_Thumbnail_Img_Cap);
    //
    if(mpDualYUVEffectHal!=nullptr)
    {
        mpDualYUVEffectHal->configure();
        mpDualYUVEffectHal->prepare();
        mpDualYUVEffectHal->start();
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpDualYUVThread->requestExit();
    // join
    mpDualYUVThread->join();
    //
    mpDualYUVThread = NULL;
    // release effectHal
    if(NULL != mpDualYUVEffectHal)
    {
        mpDualYUVEffectHal->abort();
        mpDualYUVEffectHal->release();
        mpDualYUVEffectHal->unconfigure();
        mpDualYUVEffectHal->uninit();
        delete mpDualYUVEffectHal;
        mpDualYUVEffectHal = NULL;
        MY_LOGD("Destroy DualYUV effectHal.");
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
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
DualYUVNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    MUINT32 iReqIdx = pFrame->getRequestNo();
    using std::chrono::system_clock;
    system_clock::time_point bef_enque_tp = system_clock::now();
    MY_LOGD_IF(mLogLevel >= 2,"[DeNoise_Profile] Preview Request before enque! reqID=%d", iReqIdx);

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

    // log enque time
    std::chrono::duration<double> elap = system_clock::now() - bef_enque_tp;
    MY_LOGD_IF(mLogLevel >= 2, "[DeNoise_Profile] Preview Request after enque! reqID=%d  total enque time=%f ms",
             iReqIdx, elap.count()*1000);

    mRequestQueueCond.signal();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
onDequeRequest(android::sp<IPipelineFrame>& rpFrame)
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
DualYUVNodeImp::
onProcessFrame(android::sp<IPipelineFrame> const& pFrame)
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
            else
            {
                pProcessFrame->mpAppInMeta = pProcessFrame->mpAppInMetaStreamBuffer->tryReadLock(getNodeName());
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
            else
            {
                pProcessFrame->mpAppOutMeta = pProcessFrame->mpAppOutMetaStreamBuffer->tryWriteLock(getNodeName());
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
            else
            {
                pProcessFrame->mpHalInMeta = pProcessFrame->mpHalInMetaStreamBuffer->tryReadLock(getNodeName());
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
            else
            {
                pProcessFrame->mpHalOutMeta = pProcessFrame->mpHalOutMetaStreamBuffer->tryWriteLock(getNodeName());
            }
        }

        // [In] get image stream buf and image buf.
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
        for( size_t i=0; i<imageIOMap.vIn.size() ; ++i )
        {
            StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
            if( isStream(mpInFullRaw, streamId) )
            {
                // Main1 full stream.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpInFullRaw->getStreamId(),
                pProcessFrame->mpInFullRaw_Main1_StreamBuffer,
                pProcessFrame->mpInFullRaw_Main1_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[main1 full stream] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpInFullRaw_Main2, streamId) )
            {
                // Main2 full stream.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpInFullRaw_Main2->getStreamId(),
                pProcessFrame->mpInFullRaw_Main2_StreamBuffer,
                pProcessFrame->mpInFullRaw_Main2_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[main2 full stream] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpInResizeRaw, streamId) )
            {
                // Main1 ReSize stream.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpInResizeRaw->getStreamId(),
                pProcessFrame->mpInReSizeRaw_Main1_StreamBuffer,
                pProcessFrame->mpInReSizeRaw_Main1_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[main1 resize stream] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpInResizeRaw_Main2, streamId) )
            {
                // Main1 ReSize stream.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpInResizeRaw_Main2->getStreamId(),
                pProcessFrame->mpInReSizeRaw_Main2_StreamBuffer,
                pProcessFrame->mpInReSizeRaw_Main2_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[main2 resize stream] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }
        }

        // [Out] get image stream buf and image buf.
        for( size_t i=0; i<imageIOMap.vOut.size() ; ++i )
        {
            StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
            if( isStream(mpOutHalImageMainImg, streamId) )
            {
                // Main1 MV_F for preview.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpOutHalImageMainImg->getStreamId(),
                pProcessFrame->mpOutImage_Main1_StreamBuffer,
                pProcessFrame->mpOutImage_Main1_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[Main1 MV_F for preview] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpOutHalImageFD, streamId) )
            {
                // Main1 FD for preview
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpOutHalImageFD->getStreamId(),
                pProcessFrame->mpOutImage_FD_StreamBuffer,
                pProcessFrame->mpOutImage_FD_buf,
                MTRUE
                );

                // FD is tolerant stream for touch AF
                // if( err != OK ) {
                //     MY_LOGE("[Main1 FD for preview] getImageBufferAndLock err = %d", err);
                //     suspendThisFrame(pFrame, pProcessFrame);
                //     return;
                // }
            }

            if( isStream(mpOutHalImagePrv_CB, streamId) )
            {
                // Main1 FD for preview
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpOutHalImagePrv_CB->getStreamId(),
                pProcessFrame->mpOutImage_Prv_CB_StreamBuffer,
                pProcessFrame->mpOutImage_Prv_CB_buf,
                MTRUE
                );
            }

            if( isStream(mpOutHalImage_Main1_Img_Cap, streamId) )
            {
                // Main1 Cap Image.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpOutHalImage_Main1_Img_Cap->getStreamId(),
                pProcessFrame->mpOutImage_Main1_CapStreamBuffer,
                pProcessFrame->mpOutImage_Main1_Cap_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[Main1 Cap Image] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpOutHalImage_Main2_Img_Cap, streamId) )
            {
                // Main2 Cap Image.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpOutHalImage_Main2_Img_Cap->getStreamId(),
                pProcessFrame->mpOutImage_Main2_Cap_StreamBuffer,
                pProcessFrame->mpOutImage_Main2_Cap_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[Main2 Cap Image.] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }

            if( isStream(mpOutHalImage_Thumbnail_Img_Cap, streamId) )
            {
                // Thumbnail Cap Image.
                MERROR const err = getImageBufferAndLock(
                pFrame,
                mpOutHalImage_Thumbnail_Img_Cap->getStreamId(),
                pProcessFrame->mpOutImage_Thumbnail_Cap_StreamBuffer,
                pProcessFrame->mpOutImage_Thumbnail_Cap_buf,
                MTRUE
                );

                if( err != OK ) {
                    MY_LOGE("[Thumbnail Cap Image.] getImageBufferAndLock err = %d", err);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
            }
        }
    }

    {
        // do some processing
        sp<EffectRequest> pRequest = pProcessFrame->getRequest();

        // log time before update effect request
        using std::chrono::system_clock;
        system_clock::time_point bef_update_request = system_clock::now();

        MBOOL result = mpDualYUVEffectHal->updateEffectRequest(pRequest);

        // log enque time
        std::chrono::duration<double> elap = system_clock::now() - bef_update_request;
        MY_LOGD_IF(mLogLevel >= 2, "[DeNoise_Profile] Preview Request data ready! reqID=%d total Pass2 cost time=%f ms",
                pRequest->getRequestNo(), elap.count()*1000);

        pProcessFrame->uninit(result);
        delete pProcessFrame;
        pProcessFrame = NULL;

        MY_LOGD_IF(mLogLevel >= 2,"[DeNoise_Profile] Preview Request after apply release! reqID=%d", pRequest->getRequestNo());
    }
    onDispatchFrame(pFrame);

    FUNC_END;
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualYUVNodeImp::
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
DualYUVNodeImp::
suspendThisFrame(sp<IPipelineFrame> const& pFrame, process_frame*& data)
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
DualYUVNodeImp::
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
    MY_LOGD_IF(mLogLevel >= 1,"@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD_IF(mLogLevel >= 1,"stream buffer: (%p) %p, heap: %p, buffer: %p, usage: %p",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
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
    if (DUALYUVTHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, DUALYUVTHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, DUALYUVTHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = DUALYUVTHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, DUALYUVTHREAD_POLICY, &sched_p);
    }

    MY_LOGD_IF(mLogLevel >= 1,"tid(%d) policy(%d) priority(%d)", ::gettid(), DUALYUVTHREAD_POLICY, DUALYUVTHREAD_PRIORITY);

    return OK;

}
/******************************************************************************
 *
 ******************************************************************************/
void
DualYUVNodeImp::DualYUVThread::
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
DualYUVNodeImp::DualYUVThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
DualYUVNodeImp::DualYUVThread::
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
DualYUVNodeImp::process_frame::
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
        if(mpHalInMetaStreamBuffer_Main2!=nullptr)
        {
            mpHalInMetaStreamBuffer_Main2->unlock(mNodeName, mpHalInMeta_Main2);
            releaseMetaStream(mpFrame, mpHalInMetaStreamBuffer_Main2, mNodeId);
        }
        if(mpHalOutMetaStreamBuffer!=nullptr)
        {
            mpHalOutMetaStreamBuffer->unlock(mNodeName, mpHalOutMeta);
            releaseMetaStream(mpFrame, mpHalOutMetaStreamBuffer, mNodeId);
        }
    }

    // unlock and mark buffer state to main1 fullRaw inStream.
    if(mpInFullRaw_Main1_buf!=nullptr)
    {
        mpInFullRaw_Main1_buf->unlockBuf(mNodeName);
        if(mpInFullRaw_Main1_StreamBuffer!=nullptr)
        {
            mpInFullRaw_Main1_StreamBuffer->unlock(mNodeName, mpInFullRaw_Main1_buf->getImageBufferHeap());
            streamBufferSet.markUserStatus(
                        mpInFullRaw_Main1_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    // unlock and mark buffer state to main2 fullRaw inStream.
    if(mpInFullRaw_Main2_buf!=nullptr)
    {
        mpInFullRaw_Main2_buf->unlockBuf(mNodeName);
        if(mpInFullRaw_Main2_StreamBuffer!=nullptr)
        {
            mpInFullRaw_Main2_StreamBuffer->unlock(mNodeName, mpInFullRaw_Main2_buf->getImageBufferHeap());
            streamBufferSet.markUserStatus(
                        mpInFullRaw_Main2_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    // unlock and mark buffer state to main1 resizeRaw inStream.
    if(mpInReSizeRaw_Main1_buf!=nullptr)
    {
        mpInReSizeRaw_Main1_buf->unlockBuf(mNodeName);
        if(mpInReSizeRaw_Main1_StreamBuffer!=nullptr)
        {
            mpInReSizeRaw_Main1_StreamBuffer->unlock(mNodeName, mpInReSizeRaw_Main1_buf->getImageBufferHeap());
            streamBufferSet.markUserStatus(
                        mpInReSizeRaw_Main1_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    // unlock and mark buffer state to main2 resizeRaw inStream.
    if(mpInReSizeRaw_Main2_buf!=nullptr)
    {
        mpInReSizeRaw_Main2_buf->unlockBuf(mNodeName);
        if(mpInReSizeRaw_Main2_StreamBuffer!=nullptr)
        {
            mpInReSizeRaw_Main2_StreamBuffer->unlock(mNodeName, mpInReSizeRaw_Main2_buf->getImageBufferHeap());
            streamBufferSet.markUserStatus(
                        mpInReSizeRaw_Main2_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream main1 MV_F
    if(mpOutImage_Main1_buf!=nullptr)
    {
        mpOutImage_Main1_buf->unlockBuf(mNodeName);
        if(mpOutImage_Main1_StreamBuffer!=nullptr)
        {
            mpOutImage_Main1_StreamBuffer->unlock(mNodeName, mpOutImage_Main1_buf->getImageBufferHeap());
            mpOutImage_Main1_StreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImage_Main1_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream main1 FD
    if(mpOutImage_FD_buf!=nullptr)
    {
        mpOutImage_FD_buf->unlockBuf(mNodeName);
        if(mpOutImage_FD_StreamBuffer!=nullptr)
        {
            mpOutImage_FD_StreamBuffer->unlock(mNodeName, mpOutImage_FD_buf->getImageBufferHeap());
            mpOutImage_FD_StreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImage_FD_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream Prv_CB
    if(mpOutImage_Prv_CB_buf!=nullptr)
    {
        mpOutImage_Prv_CB_buf->unlockBuf(mNodeName);
        if(mpOutImage_Prv_CB_StreamBuffer!=nullptr)
        {
            mpOutImage_Prv_CB_StreamBuffer->unlock(mNodeName, mpOutImage_Prv_CB_buf->getImageBufferHeap());
            mpOutImage_Prv_CB_StreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImage_Prv_CB_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream main1 Cap
    if(mpOutImage_Main1_Cap_buf!=nullptr)
    {
        mpOutImage_Main1_Cap_buf->unlockBuf(mNodeName);
        if(mpOutImage_Main1_CapStreamBuffer!=nullptr)
        {
            mpOutImage_Main1_CapStreamBuffer->unlock(mNodeName, mpOutImage_Main1_Cap_buf->getImageBufferHeap());
            mpOutImage_Main1_CapStreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImage_Main1_CapStreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream main2 Cap
    if(mpOutImage_Main2_Cap_buf!=nullptr)
    {
        mpOutImage_Main2_Cap_buf->unlockBuf(mNodeName);
        if(mpOutImage_Main2_Cap_StreamBuffer!=nullptr)
        {
            mpOutImage_Main2_Cap_StreamBuffer->unlock(mNodeName, mpOutImage_Main2_Cap_buf->getImageBufferHeap());
            mpOutImage_Main2_Cap_StreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImage_Main2_Cap_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    //unlock and mark buffer state to outputstream Thumbnail Cap
    if(mpOutImage_Thumbnail_Cap_buf!=nullptr)
    {
        mpOutImage_Thumbnail_Cap_buf->unlockBuf(mNodeName);
        if(mpOutImage_Thumbnail_Cap_StreamBuffer!=nullptr)
        {
            mpOutImage_Thumbnail_Cap_StreamBuffer->unlock(mNodeName, mpOutImage_Thumbnail_Cap_buf->getImageBufferHeap());
            mpOutImage_Thumbnail_Cap_StreamBuffer->markStatus(
                    process_state ?
                    STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                    );
            streamBufferSet.markUserStatus(
                        mpOutImage_Thumbnail_Cap_StreamBuffer->getStreamInfo()->getStreamId(),
                        mNodeId,
                        IUsersManager::UserStatus::USED |
                        IUsersManager::UserStatus::RELEASE
                        );
        }
    }

    streamBufferSet.applyRelease(mNodeId);

    // meta
    mpAppInMeta = nullptr;
    mpAppInMetaStreamBuffer = nullptr;
    mpAppOutMeta = nullptr;
    mpAppOutMetaStreamBuffer = nullptr;
    mpHalInMeta = nullptr;
    mpHalInMetaStreamBuffer = nullptr;
    mpHalInMeta_Main2 = nullptr;
    mpHalInMetaStreamBuffer_Main2 = nullptr;
    mpHalOutMeta = nullptr;
    mpHalOutMetaStreamBuffer = nullptr;

    // In
    mpInFullRaw_Main1_buf = nullptr;
    mpInFullRaw_Main2_buf = nullptr;
    mpInReSizeRaw_Main1_buf = nullptr;
    mpInFullRaw_Main1_StreamBuffer = nullptr;
    mpInFullRaw_Main2_StreamBuffer = nullptr;
    mpInReSizeRaw_Main1_StreamBuffer = nullptr;

    // Out
    mpOutImage_Main1_buf = nullptr;
    mpOutImage_Main1_StreamBuffer = nullptr;
    mpOutImage_FD_buf = nullptr;
    mpOutImage_FD_StreamBuffer = nullptr;
    mpOutImage_Prv_CB_buf = nullptr;
    mpOutImage_Prv_CB_StreamBuffer = nullptr;
    mpOutImage_Main1_Cap_buf = nullptr;
    mpOutImage_Main1_CapStreamBuffer = nullptr;
    mpOutImage_Main2_Cap_buf = nullptr;
    mpOutImage_Main2_Cap_StreamBuffer = nullptr;
    mpOutImage_Thumbnail_Cap_buf = nullptr;
    mpOutImage_Thumbnail_Cap_StreamBuffer = nullptr;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<EffectRequest>
DualYUVNodeImp::process_frame::
getRequest()
{
    //EffectRequestPtr pRequest = new EffectRequest(mpFrame->getRequestNo(), onEffectRequestFinished, (void*)this);
    EffectRequestPtr pRequest = new EffectRequest(mpFrame->getRequestNo(), NULL, NULL);
    // metadata
    if(mpAppInMeta != nullptr)
    {
        sp<EffectFrameInfo> pInAppMetaFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_META_IN_APP,
                                                nullptr,
                                                nullptr);
        sp<EffectParameter> reqst_para = new EffectParameter();
        reqst_para->setPtr("Metadata", mpAppInMeta);
        pInAppMetaFrame->setFrameParameter(reqst_para);
        pRequest->vInputFrameInfo.add(YUVEffectHal::BID_META_IN_APP, pInAppMetaFrame);
    }
    if(mpHalInMeta != nullptr)
    {
        sp<EffectFrameInfo> pInHalMetaFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_META_IN_HAL,
                                                nullptr,
                                                nullptr);
        sp<EffectParameter> reqst_para = new EffectParameter();
        reqst_para->setPtr("Metadata", mpHalInMeta);
        pInHalMetaFrame->setFrameParameter(reqst_para);
        pRequest->vInputFrameInfo.add(YUVEffectHal::BID_META_IN_HAL, pInHalMetaFrame);

        // extract main2 Hal Meta from main1 Hal Meta
        {
            IMetadata* pHalMeta_Main2 = new IMetadata();
            if(tryGetMetadata<IMetadata>(const_cast<IMetadata*>(mpHalInMeta), MTK_P1NODE_MAIN2_HAL_META, *pHalMeta_Main2))
            {
                sp<EffectFrameInfo> pInHalMetaFrame_Main2 = new EffectFrameInfo(
                                                            mpFrame->getRequestNo(),
                                                            YUVEffectHal::BID_META_IN_HAL_MAIN2,
                                                            nullptr,
                                                            nullptr);
                sp<EffectParameter> reqst_para = new EffectParameter();
                reqst_para->setPtr("Metadata", pHalMeta_Main2);
                pInHalMetaFrame_Main2->setFrameParameter(reqst_para);
                pRequest->vInputFrameInfo.add(YUVEffectHal::BID_META_IN_HAL_MAIN2, pInHalMetaFrame_Main2);
            }
            else
            {
                delete pHalMeta_Main2;
                MY_LOGE("cannot get MTK_P1NODE_MAIN2_HAL_META after updating request");
            }
        }


    }
    // image
    // In
    if(mpInFullRaw_Main1_buf != nullptr)
    {
        sp<EffectFrameInfo> pInFSRawFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_DualYUV_IN_FSRAW1,
                                                nullptr,
                                                nullptr);
        pInFSRawFrame->setFrameBuffer(mpInFullRaw_Main1_buf.get());
        pRequest->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_FSRAW1, pInFSRawFrame);
    }

    if(mpInFullRaw_Main2_buf != nullptr)
    {
        sp<EffectFrameInfo> pInFSRawFrame_Main2 = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_DualYUV_IN_FSRAW2,
                                                nullptr,
                                                nullptr);
        pInFSRawFrame_Main2->setFrameBuffer(mpInFullRaw_Main2_buf.get());
        pRequest->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_FSRAW2, pInFSRawFrame_Main2);
    }

    if(mpInReSizeRaw_Main1_buf != nullptr)
    {
        sp<EffectFrameInfo> pInRSRawFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_DualYUV_IN_RSRAW1,
                                                nullptr,
                                                nullptr);
        pInRSRawFrame->setFrameBuffer(mpInReSizeRaw_Main1_buf.get());
        pRequest->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_RSRAW1, pInRSRawFrame);
    }

    if(mpInReSizeRaw_Main2_buf != nullptr)
    {
        sp<EffectFrameInfo> pInRSRawFrame_Main2 = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_DualYUV_IN_RSRAW2,
                                                nullptr,
                                                nullptr);
        pInRSRawFrame_Main2->setFrameBuffer(mpInReSizeRaw_Main2_buf.get());
        pRequest->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_RSRAW2, pInRSRawFrame_Main2);
    }

    // Out
    if(mpOutImage_Main1_Cap_buf != nullptr)
    {
        sp<EffectFrameInfo> pOutMain1CapFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_OUT_MV_F_CAP_MAIN1,
                                                nullptr,
                                                nullptr);
        pOutMain1CapFrame->setFrameBuffer(mpOutImage_Main1_Cap_buf.get());
        pRequest->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_MV_F_CAP_MAIN1, pOutMain1CapFrame);
    }

    if(mpOutImage_Main2_Cap_buf != nullptr)
    {
        sp<EffectFrameInfo> pOutMain2CapFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_OUT_MV_F_CAP_MAIN2,
                                                nullptr,
                                                nullptr);
        pOutMain2CapFrame->setFrameBuffer(mpOutImage_Main2_Cap_buf.get());
        pRequest->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_MV_F_CAP_MAIN2, pOutMain2CapFrame);
    }

    if(mpOutImage_FD_buf != nullptr)
    {
        sp<EffectFrameInfo> pOutFDFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_OUT_FD,
                                                nullptr,
                                                nullptr);
        pOutFDFrame->setFrameBuffer(mpOutImage_FD_buf.get());
        pRequest->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_FD, pOutFDFrame);
    }

    if(mpOutImage_Main1_buf != nullptr)
    {
        sp<EffectFrameInfo> pOutMVFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_OUT_MV_F,
                                                nullptr,
                                                nullptr);
        pOutMVFrame->setFrameBuffer(mpOutImage_Main1_buf.get());
        pRequest->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_MV_F, pOutMVFrame);
    }

    if(mpOutImage_Thumbnail_Cap_buf != nullptr)
    {
        sp<EffectFrameInfo> pOutThumbnailCapFrame = new EffectFrameInfo(
                                                mpFrame->getRequestNo(),
                                                YUVEffectHal::BID_OUT_THUMBNAIL_CAP,
                                                nullptr,
                                                nullptr);
        pOutThumbnailCapFrame->setFrameBuffer(mpOutImage_Thumbnail_Cap_buf.get());
        pRequest->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_THUMBNAIL_CAP, pOutThumbnailCapFrame);
    }


    // decide the scenario
    sp<EffectParameter> reqst_para = new EffectParameter();
    YUVEffectHal::DualYUVNodeOpState state;

    if(pRequest->vOutputFrameInfo.indexOfKey(YUVEffectHal::BID_OUT_MV_F_CAP_MAIN1) >=0)
        state = YUVEffectHal::STATE_CAPTURE;
    else if(pRequest->vOutputFrameInfo.indexOfKey(YUVEffectHal::BID_OUT_THUMBNAIL_CAP) >=0)
        state = YUVEffectHal::STATE_THUMBNAIL;
    else
        state = YUVEffectHal::STATE_NORMAL;
    // set state to EffectParameter
    reqst_para->set(DUAL_YUV_REQUEST_STATE_KEY, state);
    pRequest->setRequestParameter(reqst_para);

    return pRequest;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualYUVNodeImp::
onEffectRequestFinished(MVOID* tag, String8 status, sp<EffectRequest>& request)
{
    DualYUVNodeImp *pDualYUVNode = (DualYUVNodeImp *) tag;
    pDualYUVNode->onEffectReqDone(status, request);
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
DualYUVNodeImp::
onEffectReqDone(String8 status, sp<EffectRequest>& request)
{
    MUINT32 reqID = request->getRequestNo();


    if(status == DUALYUV_DONE_KEY)
    {
        //onEffectReqSucess(pFrame, request);
    }
    else
    {
        //onEffectReqFailed(status, pFrame, request);
    }

    return OK;
}





















