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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

// Standard C header file

// Android system/core header file
#include <utils/String8.h>
#include <utils/Vector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam3/feature/stereo/pipe/IDualFeatureRequest.h>
// Module header file

// Local header file
#include <string>
#include "DepthMapPipe.h"
#include "DepthMapFactory.h"
#include "DepthMapEffectRequest.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "DepthMapPipe"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);

/******************************************************************************
 *
 ******************************************************************************/

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSCamFeature::NSDualFeature;

#if(TARGET_BUILD_VARIANT == eng)
    #define MAX_QUEUE_FRAME 300
#else
    #define MAX_QUEUE_FRAME 60
#endif

std::atomic<SmartDepthMapPipe*> DepthPipeHolder::mpInstance;
android::Condition DepthPipeHolder::mCond;
android::Mutex DepthPipeHolder::mMutex;

/*******************************************************************************
* SmartDepthMapPipe Definition
********************************************************************************/

sp<SmartDepthMapPipe>
SmartDepthMapPipe::
createInstance(
    android::sp<DepthMapPipeSetting> pSetting,
    android::sp<DepthMapPipeOption> pPipeOption
)
{
    MY_LOGD("SmartDepthMapPipe::createInstance");
#ifdef GTEST
    DepthPipeLoggingSetup::mbProfileLog = MTRUE;
    DepthPipeLoggingSetup::mbDebugLog = MTRUE;
#else
    DepthPipeLoggingSetup::mbProfileLog = StereoSettingProvider::isProfileLogEnabled();
    DepthPipeLoggingSetup::mbDebugLog = StereoSettingProvider::isLogEnabled(PERPERTY_DEPTHMAP_NODE_LOG);
#endif

    MY_LOGD("LoggingSetup: mbProfileLog =%d mbDebugLog=%d ",
            DepthPipeLoggingSetup::mbProfileLog, DepthPipeLoggingSetup::mbDebugLog);

    return new DepthMapPipe(pSetting, pPipeOption);

}

void
SmartDepthMapPipe::
onLastStrongRef(const void* /*id*/)
{
    MY_LOGD("SmartDepthMapPipe::onLastStrongRef");
    DepthPipeHolder::clearHolder();
}

/*******************************************************************************
* DepthPipeHolder Definition
********************************************************************************/
sp<SmartDepthMapPipe>
DepthPipeHolder::createPipe(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption)
{
    MY_LOGD("DepthPipeHolder::createPipe + ");
    SmartDepthMapPipe* tmp = mpInstance.load(std::memory_order_acquire);
    sp<SmartDepthMapPipe> pPipe = tmp;
    if(tmp == nullptr)
    {
        android::Mutex::Autolock lock(mMutex);
        tmp = mpInstance.load(std::memory_order_relaxed);
        if(tmp == nullptr)
        {
            // create & init
            pPipe = SmartDepthMapPipe::createInstance(pSetting, pPipeOption);
            pPipe->init();
            tmp = pPipe.get();
            mpInstance.store(tmp, std::memory_order_release);
            MY_LOGD("DepthPipeHolder::createPipe broadcast");
            mCond.broadcast();
        }
    }
    MY_LOGD("DepthPipeHolder::createPipe - ");
    return pPipe;
}

sp<SmartDepthMapPipe>
DepthPipeHolder::
getInstance()
{
    sp<SmartDepthMapPipe> pPipe = mpInstance.load(std::memory_order_acquire);
    MY_LOGD("DepthPipeHolder::getInstance  %x", pPipe.get());
    return pPipe;
}

sp<SmartDepthMapPipe>
DepthPipeHolder::
waitInstance()
{
    MY_LOGD("DepthPipeHolder::waitInstance + ");
    android::Mutex::Autolock lock(mMutex);
    SmartDepthMapPipe* pPipe = mpInstance.load(std::memory_order_acquire);
    while(pPipe == nullptr)
    {
        MY_LOGD("DepthPipeHolder::waitInstance  wait!! ");
        mCond.wait(mMutex);
        MY_LOGD("DepthPipeHolder::waitInstance  wait done!! ");
        pPipe = mpInstance.load(std::memory_order_acquire);
    }
    MY_LOGD("DepthPipeHolder::waitInstance - ");
    return pPipe;
}

void
DepthPipeHolder::
clearHolder()
{
    MY_LOGD("DepthPipeHolder::clearHolder");
    mpInstance.store(nullptr, std::memory_order_release);
}

/*******************************************************************************
* IDepthMapPipe & DepthMapPipe Definition
********************************************************************************/
IDepthMapPipe*
IDepthMapPipe::createInstance(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption)
{
    MY_LOGE("Not support in this version. Please use DepthPipeHolder to create instance.");
    return nullptr;
}

MBOOL
IDepthMapPipe::destroyInstance()
{
    MY_LOGE("Not support in this version. Please use DepthPipeHolder to destroy instance.");
    return MFALSE;
}

DepthMapPipe::DepthMapPipe(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption
)
: CamPipe<DepthMapPipeNode>("DepthMapPipe")
, mpSetting(pSetting)
, mpPipeOption(pPipeOption)
{
    // init signal
    mpNodeSignal = new NodeSignal();
    // init storage
    mpDepthStorage = new DepthInfoStorage([&](DepthBufferInfo info){return info.miReqIdx;});
    mpDepthStorage->setDataDelay(::property_get_int32("vendor.depthmap.pipe.enableWaitDepth.delay", 0));
    // reset serial number
    DepthMapEffectRequest::SerialNum = 0;
    //
    MY_LOGD("mSensorType=%d mFeatureMode=%d  FlowType=%d delay=%d",
        mpPipeOption->mSensorType, mpPipeOption->mFeatureMode, mpPipeOption->mFlowType,
        ::property_get_int32("vendor.depthmap.pipe.enableWaitDepth.delay", 0));
}

DepthMapPipe::~DepthMapPipe()
{
    MY_LOGD("[Destructor] +");
    // must call dispose to free CamGraph
    this->dispose();
    // free all nodes
    for(size_t idx=0;idx<mNodeMap.size();++idx)
    {
        DepthMapPipeNode *pNode = mNodeMap.valueAt(idx);
        delete pNode;
    }
    mNodeMap.clear();
    MY_LOGD("[Destructor] -");
}

MBOOL DepthMapPipe::onInit()
{
    VSDOF_INIT_LOG("+");
    // query FlowOption
    MBOOL bRet = queryDepthMapFlowOption(mpSetting, mpPipeOption, mpDepthStorage, mpFlowOption);
    // query buffer pool mgr
    bRet &= queryBufferPoolMgr(mpSetting, mpPipeOption, mpFlowOption, mpBuffPoolMgr);

    if(!bRet)
    {
        MY_LOGE("Failed to query DepthMapFlowOption and BufferPoolMgr!!");
        return MFALSE;
    }

    DepthMapPipeNode* pPipeNode;
    PipeNodeBitSet activeNodeBit;
    //query the active pipe nodes
    mpFlowOption->queryPipeNodeBitSet(activeNodeBit);

    #define TEST_AND_CREATE_NODE(nodeIDEnum, nodeClass)\
        if(activeNodeBit.test(nodeIDEnum)) { \
            PipeNodeConfigs config(mpFlowOption, mpSetting, mpPipeOption);\
            pPipeNode = new nodeClass("DepthPipe."#nodeClass, nodeIDEnum, config);\
            mNodeMap.add(nodeIDEnum, pPipeNode);\
            pPipeNode->setNodeSignal(mpNodeSignal);\
            pPipeNode->setDepthStorage(mpDepthStorage);\
            pPipeNode->setBufferPoolMgr(mpBuffPoolMgr);}

    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_P2A, P2ANode);
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_P2ABAYER, P2ABayerNode);
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_N3D, N3DNode);
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_DPE, DPENode);
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_GF, GFNode);
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_WPE, WPENode);
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_DLDEPTH, DLDepthNode);
    #undef TEST_AND_CREATE_NODE
    // build nodes into graph
    mpFlowOption->buildPipeGraph(this, mNodeMap);
    // default node graph - Error handling
    for(size_t index=0;index<mNodeMap.size();++index)
    {
        this->connectData(ERROR_OCCUR_NOTIFY, ERROR_OCCUR_NOTIFY, *mNodeMap.valueAt(index), this);
    }
    VSDOF_INIT_LOG("-");

    return MTRUE;
}

MVOID DepthMapPipe::onUninit()
{
    VSDOF_INIT_LOG("+");
    // release mgrs
    mpFlowOption = nullptr;
    mpBuffPoolMgr = nullptr;
    mpDepthStorage = nullptr;
    VSDOF_INIT_LOG("-");
}

MBOOL
DepthMapPipe::
init()
{
    CAM_ULOGM_TAGLIFE("DepthMapPipe::init");
    MBOOL bRet = CamPipe<DepthMapPipeNode>::init();
    // Use flush on stop by default
    CamPipe<DepthMapPipeNode>::setFlushOnStop(MTRUE);
    return bRet;
}

MBOOL
DepthMapPipe::
uninit()
{
    CAM_ULOGM_TAGLIFE("DepthMapPipe::uninit");
    return CamPipe<DepthMapPipeNode>::uninit();
}

MVOID
DepthMapPipe::
sync()
{
    CAM_ULOGM_TAGLIFE("DepthMapPipe::sync");
    return CamPipe<DepthMapPipeNode>::sync();
}

MVOID
DepthMapPipe::
flush()
{
    CAM_ULOGM_TAGLIFE("DepthMapPipe::flush");
    MY_LOGD("DepthMapPipe::flush +:  req size=%d", mvRequestDataMap.size());
    // set flush status
    mpNodeSignal->setStatus(NodeSignal::STATUS_IN_FLUSH);
    MY_LOGD("flush depthmap storage!!");
    // clear depth storage
    mpDepthStorage->flush();
    // lauch the default flush operations
    CamPipe::flush();
    MY_LOGD("[lock], reqID=%d!");
    // relase all the effectRequest
    android::Mutex::Autolock lock(mReqMapLock);
    {
        for(size_t index=0;index<mvRequestDataMap.size();++index)
        {
            MUINT32 iFlushReqID = mvRequestDataMap.keyAt(index);
            RequestData requestData = mvRequestDataMap.valueAt(index);
            DepthMapRequestPtr pEffectReq = requestData.pRequest;
            VSDOF_LOGD("flush ReqID=%d requestData.status=%d", iFlushReqID, requestData.status);
            // flush buffer handler
            pEffectReq->getBufferHandler()->flush();

            // if queued depth request and queued-flow not finished--> handle queue flow done
            if(pEffectReq->isQueuedDepthRequest(mpPipeOption) &&
                requestData.status != eREQ_STATUS_WAIT_FG_FLOW)
            {
                onQueuedDepthFlowDone(pEffectReq);
            }

            // callback to pipeline node with FLUSH KEY if FG flow has not finished
            if(requestData.status != eREQ_STATUS_WAIT_BG_QUEUED)
            {
                // callback to pipeline node with FLUSH KEY
                pEffectReq->launchFinishCallback(eRESULT_FLUSH);
            }
        }
        // clear all request map
        mvRequestDataMap.clear();
    }
    // clear flush status
    mpNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);

    MY_LOGD("-");
}

MBOOL
DepthMapPipe::
enque(sp<IDepthMapEffectRequest>& request)
{
    CAM_ULOGM_TAGLIFE("DepthMapPipe::enque");
    VSDOF_PRFLOG("+");
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
    {
        MY_LOGE("reqID=%d CANNOT Enque when flush!", request->getRequestNo());
        return MFALSE;
    }
    if(eERROR_NONE != mPipeError)
    {
        MY_LOGE("Pipe occur critical error, failed to enque! error=%d", mPipeError);
        return MFALSE;
    }

    DepthMapRequestPtr pDpeEffReq = static_cast<DepthMapEffectRequest*>(request.get());
    // overall timer
    pDpeEffReq->mTimer.startOverall();
    //
    MUINT32 reqID = pDpeEffReq->getRequestNo();
    // autolock for request map
    {
        VSDOF_PRFLOG("[lock]request map add reqId=%d ", reqID);
        android::Mutex::Autolock lock(mReqMapLock);
        if(mvRequestDataMap.indexOfKey(reqID) < 0)
            mvRequestDataMap.add(reqID, {pDpeEffReq, eREQ_STATUS_FG_FLOW});
        else
        {
            mPipeError = eERROR_REQ_REPEATED;
            MY_LOGE("[Critical] Request id:%d already existed, something wrong.", reqID);
            return MFALSE;
        }
        // logging all request
        DEFINE_CONTAINER_LOGGING(idsGen, mvRequestDataMap);
        VSDOF_LOGD("reqID=%d mvRequestDataMap size=%d ids=%s",
                        reqID, mvRequestDataMap.size(), CONT_LOGGING(idsGen));
    }

    // size chk
    if(mvRequestDataMap.size()>MAX_QUEUE_FRAME)
    {
        mPipeError = eERROR_QUEUE_REQ_OVERFLOW;
        MY_LOGE("[Critical] Might occur memory leak, request inside DepthMapPipe is too much, size=%d", mvRequestDataMap.size());
        return MFALSE;
    }
    // create BufferHandler for each request
    sp<BaseBufferHandler> pBufferPoolHandler = mpBuffPoolMgr->createBufferPoolHandler();
    EffectRequestAttrs reqAttrs;
    if(!mpFlowOption->queryReqAttrs(pDpeEffReq, reqAttrs))
    {
        MY_LOGE("Failed to query request attributes!");
        return MFALSE;
    }
    pDpeEffReq->init(pBufferPoolHandler, reqAttrs);
    // handle flow type task
    MBOOL ret = handleFlowTypeOnEnque(pDpeEffReq);
    // call parent class:FeaturePipe enque function
    ret = ret && CamPipe::enque(ROOT_ENQUE, pDpeEffReq);

    VSDOF_PRFLOG("-");
    return ret;
}

MBOOL
DepthMapPipe::
handleFlowTypeOnEnque(DepthMapRequestPtr pRequest)
{
    VSDOF_LOGD("handleFlowTypeOnEnque reqID=%d", pRequest->getRequestNo());
    // if queued depth pRequest
    if(pRequest->isQueuedDepthRequest(mpPipeOption) &&
        pRequest->getRequestAttr().opState != eSTATE_STANDALONE)
    {
        VSDOF_LOGD("+ inside");
        MINT32 iReqID= pRequest->getRequestNo();
        IMetadata* pMeta = nullptr;
        // create new metadata for the existed metadata
        for(auto metaBID : INPUT_METADATA_PBID_LIST)
        {
            MBOOL bRet = pRequest->getRequestMetadata({.bufferID=metaBID,
                                        .ioType=eBUFFER_IOTYPE_INPUT}, pMeta);
            if(!bRet)
            {
                MY_LOGE("Cannot get pRequest metadata, BID=%d", metaBID);
                return MFALSE;
            }

            // create new meta
            IMetadata* pCopyMeta = new IMetadata(*pMeta);
            DepthMapBufferID queuedMetaBID = mapQueuedBufferID(pRequest, mpPipeOption, metaBID);
            // push into pRequest
            pRequest->pushRequestMetadata({.bufferID=queuedMetaBID,
                                        .ioType=eBUFFER_IOTYPE_INPUT}, pCopyMeta);

            if(!bRet)
            {
                MY_LOGE("Failed to push pRequest metadata, BID=%d", queuedMetaBID);
                return MFALSE;
            }
        }
        // output
        for(auto metaBID : OUTPUT_METADATA_PBID_LIST)
        {
            MBOOL bRet = pRequest->getRequestMetadata({.bufferID=metaBID,
                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pMeta);
            if(!bRet)
            {
                MY_LOGE("Cannot get pRequest out metadata, BID=%d", metaBID);
                return MFALSE;
            }
            // create new meta
            IMetadata* pCopyMeta = new IMetadata(*pMeta);
            DepthMapBufferID queuedMetaBID = mapQueuedBufferID(pRequest, mpPipeOption, metaBID);
            // push into request
            pRequest->pushRequestMetadata({.bufferID=queuedMetaBID,
                                        .ioType=eBUFFER_IOTYPE_OUTPUT}, pCopyMeta);
            if(!bRet)
            {
                MY_LOGE("Failed to push pRequest out metadata, BID=%d", queuedMetaBID);
                return MFALSE;
            }
        }
    }
    VSDOF_LOGD("-");
    return MTRUE;
}


MBOOL DepthMapPipe::onData(DataID id, DepthMapRequestPtr &request)
{
    MUINT32 iReqID = request->getRequestNo();
    VSDOF_LOGD("+ : reqID=%d DataID=%d", iReqID, id);

    MBOOL bRet = MFALSE;
    switch(id)
    {
        case P2A_OUT_MV_F:
        case P2A_OUT_MV_F_CAP:
        case P2A_OUT_FD:
        case P2A_NORMAL_FRAME_DONE:
        case GF_OUT_DMBG:
        case DEPTHMAP_META_OUT:
        case DLDEPTH_OUT_DEPTHMAP:
        {
            bRet = onDataSuccess(request);
            break;
        }
        case P2A_OUT_YUV_DONE:
        {
            bRet = onHandleYUVDone(request);
            break;
        }
        case ERROR_OCCUR_NOTIFY:
        {
            bRet = onErrorNotify(request);
            break;
        }
        case QUEUED_FLOW_DONE:
        {
            // request map check
            VSDOF_LOGD("[lock] QUEUED_FLOW_DONE reqID=%d", iReqID);
            android::Mutex::Autolock lock(mReqMapLock);
            bRet = onQueuedDepthFlowDone(request);
            ssize_t index = mvRequestDataMap.indexOfKey(iReqID);
            if(index>=0)
            {
                RequestData& requestData = mvRequestDataMap.editValueAt(index);
                // if the FG flow is finished, remove it from map
                if(requestData.status == eREQ_STATUS_WAIT_BG_QUEUED)
                    mvRequestDataMap.removeItem(iReqID);
                // if the FG flow has not finished, change status to WAIT FG flow
                else
                    requestData.status = eREQ_STATUS_WAIT_FG_FLOW;
            }
            break;
        }
        case REQUEST_DEPTH_NOT_READY:
        {
            VSDOF_LOGD("REQUEST_DEPTH_NOT_READY reqID=%d", iReqID);
            request->mTimer.stopOverall();
            request->mTimer.showPerFrameSummary(request->getRequestNo(), request->getRequestAttr());
            bRet = onRequestDepthNotReady(request);
            break;
        }
        default:
            MY_LOGW("FeaturePipe_DepthMap: onData non-avaiilable id=%d", id);
            return MFALSE;
    }


    VSDOF_LOGD("-");
    return bRet;
}

MBOOL
DepthMapPipe::
onRequestDepthNotReady(DepthMapRequestPtr pRequest)
{
    VSDOF_LOGD("+ [lock] reqID=%d", pRequest->getRequestNo());
    android::Mutex::Autolock lock(mReqMapLock);
    MUINT32 reqID = pRequest->getRequestNo();
    if(mvRequestDataMap.indexOfKey(reqID)<0)
    {
        MY_LOGW("FeaturePipe_DepthMap: onRequestDepthNotReady non-existed req_id=%d, might already return.", reqID);
        return MFALSE;
    }
    VSDOF_LOGD("DepthMapPipie request depth not ready! reqID=%d", pRequest->getRequestNo());
    RequestData& requestData = mvRequestDataMap.editValueFor(reqID);

    if(pRequest->getRequestAttr().opState == eSTATE_STANDALONE)
    {
        // callback
        pRequest->launchFinishCallback(eRESULT_DEPTH_NOT_READY);
        // FG ends here-> remove item
        mvRequestDataMap.removeItem(reqID);
    }
    // if now is FG flow + BG is still ongoing
    else if(requestData.status == eREQ_STATUS_FG_FLOW)
    {
        // change status to wait BG
        requestData.status = eREQ_STATUS_WAIT_BG_QUEUED;
        // callback
        pRequest->launchFinishCallback(eRESULT_DEPTH_NOT_READY);
    }
    // if BG is finish and wait FG flow
    else if(requestData.status == eREQ_STATUS_WAIT_FG_FLOW)
    {
        // callback
        pRequest->launchFinishCallback(eRESULT_DEPTH_NOT_READY);
        // FG ends here-> remove item
        mvRequestDataMap.removeItem(reqID);
    }
    else
    {
        MY_LOGE("non-possible status=%d", requestData.status);
        return MFALSE;
    }
    VSDOF_LOGD("- reqID=%d", pRequest->getRequestNo());
    return MTRUE;
}

MBOOL
DepthMapPipe::
onQueuedDepthFlowDone(DepthMapRequestPtr pRequest)
{
    VSDOF_LOGD("onQueuedDepthFlowDone reqID=%d", pRequest->getRequestNo());

    IMetadata* pMeta = nullptr;
    // release the queued metadata instance(input)
    for(auto metaBID : INPUT_METADATA_PBID_LIST)
    {
        DepthMapBufferID queuedMetaBID = mapQueuedBufferID(pRequest, mpPipeOption, metaBID);
        MBOOL bRet = pRequest->getRequestMetadata({.bufferID=queuedMetaBID,
                                                    .ioType=eBUFFER_IOTYPE_INPUT}, pMeta);
        if(!bRet)
        {
            MY_LOGE("Cannot get request in metadata, BID=%d", queuedMetaBID);
            return MFALSE;
        }
        delete pMeta;
    }
    // release the queued metadata instance(output)
    for(auto metaBID : OUTPUT_METADATA_PBID_LIST)
    {
        DepthMapBufferID queuedMetaBID = mapQueuedBufferID(pRequest, mpPipeOption, metaBID);
        MBOOL bRet = pRequest->getRequestMetadata({.bufferID=queuedMetaBID,
                                                    .ioType=eBUFFER_IOTYPE_OUTPUT}, pMeta);
        if(!bRet)
        {
            MY_LOGE("Cannot get request out metadata, BID=%d", queuedMetaBID);
            return MFALSE;
        }
        delete pMeta;
    }

    VSDOF_LOGD("onQueuedDepthFlowDone  reqID=%d -", pRequest->getRequestNo());
    return MTRUE;
}

MBOOL
DepthMapPipe::
onErrorNotify(DepthMapRequestPtr &request)
{
    android::Mutex::Autolock lock(mReqMapLock);
    MUINT32 reqID = request->getRequestNo();
    VSDOF_LOGD("DepthMapPipie ! lockk reqID=%d", request->getRequestNo());
    if(mvRequestDataMap.indexOfKey(reqID)<0)
    {
        MY_LOGW("FeaturePipe_DepthMap: onErrorNotify non-existed req_id=%d, might already return.", reqID);
        return MTRUE;
    }
    MY_LOGE("DepthMapPipie request occur error! reqID=%d", request->getRequestNo());
    RequestData requestData = mvRequestDataMap.valueFor(reqID);
    // callback if FG flow not finished
    if(requestData.status != eREQ_STATUS_WAIT_BG_QUEUED)
    {
        request->launchFinishCallback(eRESULT_ERROR);
    }
    // remove request
    mvRequestDataMap.removeItem(reqID);

    return MTRUE;
}

MBOOL
DepthMapPipe::
onDataSuccess(DepthMapRequestPtr &request)
{
    MUINT32 reqID = request->getRequestNo();
    VSDOF_LOGD("+ [lock] reqID=%d", reqID);
    // autolock for request map
    android::Mutex::Autolock lock(mReqMapLock);
    ssize_t reqPos =  mvRequestDataMap.indexOfKey(reqID);

    // if the FG flow is finished, return
    if(reqPos>=0 && mvRequestDataMap.valueAt(reqPos).status == eREQ_STATUS_WAIT_BG_QUEUED)
        return MTRUE;
    else if(reqPos < 0)
    {
        MY_LOGW("FeaturePipe_DepthMap: onDataSuccess non-existed reqID=%d, might already return.", reqID);
        return MTRUE;
    }

    // if ready callback to client
    if(request->checkAllOutputReady())
    {
        request->mTimer.stopOverall();
        request->mTimer.showPerFrameSummary(request->getRequestNo(), request->getRequestAttr());
        VSDOF_LOGD("Request data ready! reqID=%d", request->getRequestNo());
        RequestData& requestData = mvRequestDataMap.editValueAt(reqPos);
        // if the request is queued-flow and the queue-flow has not finished, change status.
        if(request->getRequestAttr().opState != eSTATE_STANDALONE
            && request->isQueuedDepthRequest(mpPipeOption)
            && requestData.status == eREQ_STATUS_FG_FLOW
        )
            requestData.status = eREQ_STATUS_WAIT_BG_QUEUED;
        // if not, remove it
        else
            mvRequestDataMap.removeItem(reqID);
        // callback
        request->launchFinishCallback(eRESULT_COMPLETE);
    }

    VSDOF_LOGD("-");

    return MTRUE;
}

MBOOL
DepthMapPipe::
onHandleYUVDone(DepthMapRequestPtr request)
{
    MUINT32 reqID = request->getRequestNo();
    VSDOF_LOGD("+ [lock] reqID=%d", reqID);
    // autolock for request map
    android::Mutex::Autolock lock(mReqMapLock);
    ssize_t reqPos =  mvRequestDataMap.indexOfKey(reqID);

    if(reqPos < 0)
    {
        MY_LOGE("'non-existed reqID=%d, need to check!", reqID);
        return MTRUE;
    }
    // callback
    request->launchFinishCallback(eRESULT_YUV_DONE);
    VSDOF_LOGD("-");
    return MTRUE;

}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
