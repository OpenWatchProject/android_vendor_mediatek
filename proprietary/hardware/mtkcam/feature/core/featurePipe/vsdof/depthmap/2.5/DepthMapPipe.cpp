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

// Standard C header file

// Android system/core header file
#include <utils/String8.h>
#include <utils/Vector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file

// Local header file
#include "DepthMapPipe.h"
#include "DepthMapFactory.h"
#include "DepthMapEffectRequest.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "DepthMapPipe"
#include <featurePipe/core/include/PipeLog.h>

/******************************************************************************
 *
 ******************************************************************************/

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#if(TARGET_BUILD_VARIANT == eng)
    #define MAX_QUEUE_FRAME 300
#else
    #define MAX_QUEUE_FRAME 60
#endif

IDepthMapPipe*
IDepthMapPipe::createInstance(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption)
{
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

MBOOL
IDepthMapPipe::destroyInstance()
{
    delete this;
    return MTRUE;
}

DepthMapPipe::DepthMapPipe(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption
)
: CamPipe<DepthMapPipeNode>("DepthMapPipe")
, mpSetting(pSetting)
, mpPipeOption(pPipeOption)
{
    DepthMapPipeNode::miTimestamp = time(NULL);
    // init signal
    mpNodeSignal = new NodeSignal();
    // init storage
    mpDepthStorage = new DepthInfoStorage();
    MY_LOGD("mSensorType=%d mFeatureMode=%d  FlowType=%d", mpPipeOption->mSensorType, mpPipeOption->mFeatureMode, mpPipeOption->mFlowType);
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
    bRet &= queryBufferPoolMgr(mpPipeOption, mpFlowOption, mpBuffPoolMgr);

    if(!bRet)
    {
        MY_LOGE("Failed to query DepthMapFlowOption and BufferPoolMgr!!");
        return MFALSE;
    }

    DepthMapPipeNode* pPipeNode;
    PipeNodeBitSet activeNodeBit;
    //query the active pipe nodes
    mpFlowOption->queryPipeNodeBitSet(activeNodeBit);

    #define TEST_AND_CREATE_NODE(nodeIDEnum, nodeClass, nodeName)\
        if(activeNodeBit.test(nodeIDEnum)) { \
            PipeNodeConfigs config(mpFlowOption, mpSetting, mpPipeOption);\
            pPipeNode = new nodeClass(nodeName, nodeIDEnum, config);\
            mNodeMap.add(nodeIDEnum, pPipeNode);\
            pPipeNode->setNodeSignal(mpNodeSignal);\
            pPipeNode->setDepthStorage(mpDepthStorage);\
            pPipeNode->setBufferPoolMgr(mpBuffPoolMgr);}

    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_P2AFM, P2AFMNode, "P2AFM");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_N3D, N3DNode, "N3DNode");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_DPE, DPENode, "DPENode");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_WMF, WMFNode, "WMFNode");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_OCC, OCCNode, "OCCNode");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_FD, StereoFDNode, "SFDNode");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_GF, GFNode, "GFNode");
    TEST_AND_CREATE_NODE(eDPETHMAP_PIPE_NODEID_P2ABAYER, P2ABayerNode, "P2ABayer");

    #undef TEST_AND_CREATE_NODE

    // set Last DMP buffer pool for DPENode
    DPENode *pDPENode = reinterpret_cast<DPENode*>(mNodeMap.valueFor(eDPETHMAP_PIPE_NODEID_DPE));
    sp<ImageBufferPool> pPool = mpBuffPoolMgr->getImageBufferPool(BID_DPE_INTERNAL_LAST_DMP);
    pDPENode->setDMPBufferPool(pPool);

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
    CAM_TRACE_NAME("DepthMapPipe::init");
    MBOOL bRet = CamPipe<DepthMapPipeNode>::init();
    // Use flush on stop by default
    CamPipe<DepthMapPipeNode>::setFlushOnStop(MTRUE);
    return bRet;
}

MBOOL
DepthMapPipe::
uninit()
{
    CAM_TRACE_NAME("DepthMapPipe::uninit");
    return CamPipe<DepthMapPipeNode>::uninit();
}

MVOID
DepthMapPipe::
sync()
{
    CAM_TRACE_NAME("DepthMapPipe::sync");
    return CamPipe<DepthMapPipeNode>::sync();
}

MVOID
DepthMapPipe::
flush()
{
    CAM_TRACE_NAME("DepthMapPipe::flush");
    MY_LOGD("DepthMapPipe::flush +:  req size=%d", mvRequestDataMap.size());
    // set flush status
    mpNodeSignal->setStatus(NodeSignal::STATUS_IN_FLUSH);
    MY_LOGD("flush depthmap storage!!");
    // clear depth storage
    mpDepthStorage->flush();
    // lauch the default flush operations
    CamPipe::flush();

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
                sp<EffectRequest> pReq = (EffectRequest*) pEffectReq.get();
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
    CAM_TRACE_NAME("DepthMapPipe::enque");
    VSDOF_PRFLOG("+");

    if(eERROR_NONE != mPipeError)
    {
        MY_LOGE("Pipe occur critical error, failed to enque! error=%d", mPipeError);
        return MFALSE;
    }

    DepthMapRequestPtr pDpeEffReq = static_cast<DepthMapEffectRequest*>(request.get());

    MUINT32 reqID = pDpeEffReq->getRequestNo();
    // autolock for request map
    {
        VSDOF_PRFLOG("request map add reqId=%d ", reqID);
        android::Mutex::Autolock lock(mReqMapLock);
        if(mvRequestDataMap.indexOfKey(reqID) < 0)
            mvRequestDataMap.add(reqID, {pDpeEffReq, eREQ_STATUS_FG_FLOW});
        else
        {
            mPipeError = eERROR_REQ_REPEATED;
            MY_LOGE("[Critical] Request id:%d already existed, something wrong.", reqID);
            return MFALSE;
        }
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
    VSDOF_LOGD("+");
    // if queued depth pRequest
    if(pRequest->isQueuedDepthRequest(mpPipeOption))
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
        case P2A_OUT_TUNING_BUF:
        case WMF_TO_GF_DMW_MY_S:
        case GF_OUT_DMBG:
        case DEPTHMAP_META_OUT:
        case N3D_OUT_JPS_WARPMTX:
        case FD_OUT_EXTRADATA:
        case DPE_OUT_DISPARITY:
        case GF_OUT_DEPTH_WRAPPER:
        case P2A_OUT_DEPTHMAP:
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
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    android::Mutex::Autolock lock(mReqMapLock);
    MUINT32 reqID = pRequest->getRequestNo();
    if(mvRequestDataMap.indexOfKey(reqID)<0)
    {
        MY_LOGW("FeaturePipe_DepthMap: onRequestDepthNotReady non-existed req_id=%d, might already return.", reqID);
        return MFALSE;
    }
    VSDOF_LOGD("DepthMapPipie request depth not ready! reqID=%d", pRequest->getRequestNo());
    RequestData& requestData = mvRequestDataMap.editValueFor(reqID);

    // if now is FG flow + BG is still ongoing
    if(requestData.status == eREQ_STATUS_FG_FLOW)
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
    return MTRUE;
}

MBOOL
DepthMapPipe::
onQueuedDepthFlowDone(DepthMapRequestPtr pRequest)
{
    MY_LOGD("onQueuedDepthFlowDone reqID=%d", pRequest->getRequestNo());
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

    return MTRUE;
}

MBOOL
DepthMapPipe::
onErrorNotify(DepthMapRequestPtr &request)
{
    android::Mutex::Autolock lock(mReqMapLock);
    MUINT32 reqID = request->getRequestNo();
    if(mvRequestDataMap.indexOfKey(reqID)<0)
    {
        MY_LOGW("FeaturePipe_DepthMap: onErrorNotify non-existed req_id=%d, might already return.", reqID);
        return MTRUE;
    }
    VSDOF_LOGD("DepthMapPipie request occur error! reqID=%d", request->getRequestNo());
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
    VSDOF_LOGD("+ reqID=%d", reqID);
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
        VSDOF_PRFLOG("Request data ready! req_id=%d", request->getRequestNo());
        RequestData& requestData = mvRequestDataMap.editValueAt(reqPos);
        // if the request is queued-flow and the queue-flow has not finished, change status.
        if(request->isQueuedDepthRequest(mpPipeOption) && requestData.status == eREQ_STATUS_FG_FLOW)
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
    VSDOF_LOGD("+ reqID=%d", reqID);
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
