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

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file

// Local header file
#include "N3DNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeUtils.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "N3DNode"
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {


const char* PHASE2Name(N3D_PVPHASE_ENUM phase)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;

  switch(phase)
  {
    MAKE_NAME_CASE(eN3D_PVPHASE_COMPLETE);
    MAKE_NAME_CASE(eN3D_PVPHASE_FULL);
    MAKE_NAME_CASE(eN3D_PVPHASE_MAIN1_PADDING);
    MAKE_NAME_CASE(eN3D_PVPHASE_MAIN2_WARPPING);
  };
  return "UNKNOWN";
#undef MAKE_NAME_CASE
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

N3DNode::
N3DNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, DataSequential(reqNoGetter, "N3DSeq")
, DataOrdering(reqNoGetter, "N3DOrder")
{
    this->addWaitQueue(&mPriorityQueue);
}

N3DNode::
~N3DNode()
{
    MY_LOGD("[Destructor]");
}

MVOID
N3DNode::
cleanUp()
{
    VSDOF_LOGD("+");

    if(mpN3DHAL_CAP)
    {
        delete mpN3DHAL_CAP;
        mpN3DHAL_CAP = NULL;
    }

    if(mpN3DHAL_VRPV)
    {
        delete mpN3DHAL_VRPV;
        mpN3DHAL_VRPV = NULL;
    }
    mPriorityQueue.clear();
    {
        Mutex::Autolock _l(mModeLock);
        while(!mBlockReqIDQueue.empty())
            mBlockReqIDQueue.pop();
        mbReverseMode = MFALSE;
    }


    VSDOF_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
N3DNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
N3DNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
N3DNode::
onThreadStart()
{
    VSDOF_INIT_LOG("+");
    CAM_TRACE_NAME("N3DNode::onInitonThreadStart");
    // create N3DHAL instance - Capture
    N3D_HAL_INIT_PARAM initParam_CAP;
    initParam_CAP.eScenario  = eSTEREO_SCENARIO_CAPTURE;
    initParam_CAP.fefmRound  = VSDOF_CONST_FE_EXEC_TIMES;
    MBOOL bRet = mpBuffPoolMgr->getAllPoolImageBuffer(BID_P2A_OUT_RECT_IN2,
                                        eBUFFER_POOL_SCENARIO_CAPTURE,
                                        initParam_CAP.inputImageBuffers);
    bRet &= mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_SV_Y,
                                        eBUFFER_POOL_SCENARIO_CAPTURE,
                                        initParam_CAP.outputImageBuffers);
    bRet &= mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_MASK_S,
                                        eBUFFER_POOL_SCENARIO_CAPTURE,
                                        initParam_CAP.outputMaskBuffers);
    if(!bRet)
    {
        MY_LOGE("Failed to get all pool imagebuffer");
        return MFALSE;
    }

    mpN3DHAL_CAP = N3D_HAL::createInstance(initParam_CAP);
    // create N3DHAL instance - Preview/Record
    N3D_HAL_INIT_PARAM initParam_VRPV;
    initParam_VRPV.eScenario = eSTEREO_SCENARIO_RECORD;
    initParam_VRPV.fefmRound = VSDOF_CONST_FE_EXEC_TIMES;
    //
    bRet = mpBuffPoolMgr->getAllPoolImageBuffer(BID_P2A_OUT_RECT_IN2,
                                        eBUFFER_POOL_SCENARIO_PREVIEW,
                                        initParam_VRPV.inputImageBuffers);
    bRet &= mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_SV_Y,
                                        eBUFFER_POOL_SCENARIO_PREVIEW,
                                        initParam_VRPV.outputImageBuffers);
    bRet &= mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_MASK_S,
                                        eBUFFER_POOL_SCENARIO_PREVIEW,
                                        initParam_VRPV.outputMaskBuffers);

    mpN3DHAL_VRPV  = N3D_HAL::createInstance(initParam_VRPV);

    if(!bRet)
    {
        MY_LOGE("Failed to get all preview pool imagebuffer");
        return MFALSE;
    }

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
N3DNode::
onThreadStop()
{
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}


MBOOL
N3DNode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    MBOOL ret = MTRUE;
    VSDOF_LOGD("+ : dataID=%s reqId=%d", ID2Name(data), pRequest->getRequestNo());

    switch(data)
    {
        case P2A_TO_N3D_FEFM_CCin:
        case P2A_TO_N3D_NOFEFM_RECT2:
        {
            Mutex::Autolock _l(mModeLock);
            MUINT32 dequeIdx = -1;
            if(DataOrdering::getLatestEnqueIdx(dequeIdx)&&
                dequeIdx > pRequest->getRequestNo())
            {
                VSDOF_LOGD("N3DNode enque reverse mode on! reqID=%d latestOnGoing=%d"
                            , pRequest->getRequestNo(), dequeIdx);
                DataOrdering::getEnqueDataSeq(mBlockReqIDQueue);
                mbReverseMode = MTRUE;
            }
            mPriorityQueue.enque(pRequest);
            DataOrdering::markEnqueData(pRequest);
            break;
        }
        case P2A_TO_N3D_NOFEFM_RECT1:
            mPriorityQueue.enque(pRequest);
            break;
        default:
            MY_LOGW("Unrecongnized DataID=%d", data);
            ret = MFALSE;
            break;
    }

    VSDOF_LOGD("-");
    return ret;
}

DEQUE_STATUS
N3DNode::
dequePrioritizedRequest(
    DepthMapRequestPtr& rpRequest
)
{
    Mutex::Autolock _l(mModeLock);
    // when req id reverse(restart from beginning ex. 0)
    if(mbReverseMode)
    {
        if(mBlockReqIDQueue.empty())
        {
            AEE_ASSERT("Empty block req queue, should not happened!");
            return DEQUE_FAILED;
        }
        //wait for onging request
        MUINT32 dequeID = mBlockReqIDQueue.front();
        // wait for this
        VSDOF_LOGD("Reverse mode on: wait for reqID=%d", dequeID);
        if(!mPriorityQueue.deque(dequeID, rpRequest))
        {
            VSDOF_LOGD("Reverse mode on: wait for reqID=%d", dequeID);
            return DEQUE_WAITING;
        }
        VSDOF_LOGD("Reverse mode on: deque reqID=%d", dequeID);
        return DEQUE_SUCCESS;
    }
    else
    {
        // not enough working set
        if(DataSequential::getQueuedDataSize() >= VSDOF_WORKING_EXTRA_BUF_SET)
        {
            MUINT32 dequeID = DataSequential::getOldestOnGoingSequence();
            VSDOF_LOGD("Wait mode on: wait for reqID=%d", dequeID);
            if(!mPriorityQueue.deque(dequeID, rpRequest))
            {
                VSDOF_LOGD("Wait mode on: wait for reqID=%d", dequeID);
                return DEQUE_WAITING;
            }
            VSDOF_LOGD("Wait mode on: deque reqID=%d", dequeID);
            return DEQUE_SUCCESS;
        }
    }
    //normal case
    if(!mPriorityQueue.deque(rpRequest))
    {
        return DEQUE_FAILED;
    }
    return DEQUE_SUCCESS;
}

MBOOL
N3DNode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;

    if( !waitAnyQueue() )
    {
        return MFALSE;
    }
    // deque prioritized request
    DEQUE_STATUS status = dequePrioritizedRequest(pRequest);
    if(status != DEQUE_SUCCESS)
    {
        MBOOL bRet = (status == DEQUE_FAILED) ? MFALSE : MTRUE;
        return bRet;
    }
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_PRFLOG("threadLoop start, reqID=%d", pRequest->getRequestNo());
    CAM_TRACE_NAME("N3DNode::onThreadLoop");

    MBOOL ret;
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        ret = performN3DALGO_CAP(pRequest);
        this->_handleDequeDone(pRequest);
    }
    else
    {
        // process done are launched inside it
        ret = performN3DALGO_VRPV(pRequest);
    }
    // error handling
    if(!ret)
    {
        MY_LOGE("N3D operation failed: reqID=%d", pRequest->getRequestNo());
        // if error occur in the queued-flow, skip this operation and call queue-done
        if(pRequest->isQueuedDepthRequest(mpPipeOption))
            handleData(QUEUED_FLOW_DONE, pRequest);
        else
            handleData(ERROR_OCCUR_NOTIFY, pRequest);
    }
    // mark on-going-request end
    this->decExtThreadDependency();
    return ret;
}

MBOOL
N3DNode::
onHandleOnGoingReqDataReady(
    DepthMapRequestPtr pRequest
)
{
    auto iReqID = pRequest->getRequestNo();
    VSDOF_LOGD("+ reqID=%d", iReqID);
    //
    this->_handleDequeDone(pRequest);
    // sequential: mark finish
    Vector<DepthMapRequestPtr> popReqVec;
    this->markFinishAndPop(iReqID, popReqVec);
    for(auto index=0;index<popReqVec.size();++index)
    {
        auto pDeqReq = popReqVec.itemAt(index);
        VSDOF_LOGD("reqID=%d, release id:%d", iReqID, pDeqReq->getRequestNo());
        //
        this->handleData(N3D_TO_DPE_MVSV_MASK, pDeqReq);
        this->_handleDequeDone(pDeqReq);
    }
    return MTRUE;
}

MVOID
N3DNode::
_handleDequeDone(DepthMapRequestPtr pRequest)
{
    Mutex::Autolock _l(mModeLock);
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    DataOrdering::markDequeData(pRequest);
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // handle reverse mode request
    if(mbReverseMode)
    {
        if(mBlockReqIDQueue.front() != pRequest->getRequestNo())
        {
            MY_LOGE("Block reqID=%d is not same with current deque reqID=%d",
                    mBlockReqIDQueue.front(), pRequest->getRequestNo());
        }
        else
        {
            VSDOF_LOGD("rmv reqID=%d", pRequest->getRequestNo());
            mBlockReqIDQueue.pop();
            if(mBlockReqIDQueue.empty())
            {
                VSDOF_LOGD("reverse mode off! reqID=%d", pRequest->getRequestNo());
                mbReverseMode = MFALSE;
            }
        }
    }
    VSDOF_LOGD("- reqID=%d", pRequest->getRequestNo());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  N3DNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

N3D_PVPHASE_ENUM
N3DNode::
getPreviewPhase(DepthMapRequestPtr pRequest)
{
    if(pRequest->getRequestAttr().needFEFM)
        return eN3D_PVPHASE_FULL;
    else
    {
        IImageBuffer* pImgBuf;
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        // if the main2 warpping has not complete -> do warping first (check depth node)
        if(!pBufferHandler->getEnqueBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_SV_Y, pImgBuf))
            return eN3D_PVPHASE_MAIN2_WARPPING;
        else if(!pBufferHandler->getEnqueBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_MV_Y, pImgBuf))
            return eN3D_PVPHASE_MAIN1_PADDING;
    }

    VSDOF_LOGD("Complete! Preview phase complete, reqID=%d", pRequest->getRequestNo());
    return eN3D_PVPHASE_COMPLETE;
}

MBOOL
N3DNode::
performN3DALGO_VRPV(DepthMapRequestPtr& pRequest)
{
    CAM_TRACE_NAME("N3DNode::performN3DALGO_VRPV");
    N3D_HAL_PARAM n3dParams;
    N3D_HAL_OUTPUT n3dOutput;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    N3D_PVPHASE_ENUM phase = this->getPreviewPhase(pRequest);

    VSDOF_LOGD("performN3DALGO_VRPV phase=%s", PHASE2Name(phase));
    bool bRet = true;
    if(phase == eN3D_PVPHASE_MAIN1_PADDING)
    {
        // input
        IImageBuffer* pRectIn1Buf=nullptr;
        bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN1, pRectIn1Buf);
        pRectIn1Buf->syncCache(eCACHECTRL_INVALID);
        // output
        IImageBuffer* pImgBuf_MV_Y = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MV_Y);
        IImageBuffer* pImgBuf_MASK_M = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MASK_M);
        //
        VSDOF_PRFLOG("start N3D(PV/VR) ALGO - Main1 Padding, reqID=%d pImgBuf_MV_Y=%dx%d pImgBuf_MASK_M=%dx%d",
                        pRequest->getRequestNo(), pImgBuf_MV_Y->getImgSize().w, pImgBuf_MV_Y->getImgSize().h,
                        pImgBuf_MASK_M->getImgSize().w, pImgBuf_MASK_M->getImgSize().h);
        CAM_TRACE_BEGIN("N3DNode::N3DHALRun Main1 Warping");
        bRet = mpN3DHAL_VRPV->N3DHALWarpMain1(pRectIn1Buf, pImgBuf_MV_Y, pImgBuf_MASK_M);
        CAM_TRACE_END();
    }
    else
    {
        // prepare params
        if(!prepareN3DParam_NORMAL(phase, pRequest, n3dParams, n3dOutput))
            return MFALSE;
        // debug param
        debugN3DParams(n3dParams, n3dOutput);
        // timer
        pRequest->mTimer.startN3D();
        VSDOF_PRFLOG("start N3D(PV/VR) ALGO, reqID=%d phase=%s", pRequest->getRequestNo(), PHASE2Name(phase));
        CAM_TRACE_BEGIN("N3DNode::N3DHALRun");
        bRet = mpN3DHAL_VRPV->N3DHALRun(n3dParams, n3dOutput);
        CAM_TRACE_END();
        //
        pRequest->mTimer.stopN3D();
        // write meta
        writeN3DResultToMeta(n3dOutput, pRequest);
        //
        VSDOF_PRFTIME_LOG("finsished N3D(PV/VR) ALGO, reqID=%d, phase=%s, exec-time=%d msec",
                pRequest->getRequestNo(), PHASE2Name(phase), pRequest->mTimer.getElapsedN3D());
    }

    if(bRet)
    {
        if(phase == eN3D_PVPHASE_FULL)
        {
            // pass MV/SV/MASK to DPE
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_M, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_S, eDPETHMAP_PIPE_NODEID_DPE);
            // check sequential
            SequenceOPState state = this->sequentialize(pRequest);
            // no need to sequentialize
            if (state == SEQ_DISABLE)
            {
                VSDOF_LOGD("Seq: No need to sequential, reqID=%d", pRequest->getRequestNo());
                this->handleDataAndDump(N3D_TO_DPE_MVSV_MASK, pRequest);
                this->_handleDequeDone(pRequest);
            }
            else
            {
               VSDOF_LOGD("Seq: push sequential done, reqID=%d", pRequest->getRequestNo());
               this->handleDump(N3D_TO_DPE_MVSV_MASK, pRequest);
            }
        }
        else if(phase == eN3D_PVPHASE_MAIN2_WARPPING)
        {
            // just config output and wait for padding result
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_S, eDPETHMAP_PIPE_NODEID_DPE);
            // mark on-going request for not ready one.
            if(this->getPreviewPhase(pRequest) != eN3D_PVPHASE_COMPLETE)
                this->markOnGoingData(pRequest);
            else
            {
                this->handleDataAndDump(N3D_TO_DPE_MVSV_MASK, pRequest);
                this->onHandleOnGoingReqDataReady(pRequest);
            }
        }
        else if(phase == eN3D_PVPHASE_MAIN1_PADDING)
        {
            // padding done, notify DPENode
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_M, eDPETHMAP_PIPE_NODEID_DPE);
            // mark on-going request for not ready one.
            if(this->getPreviewPhase(pRequest) != eN3D_PVPHASE_COMPLETE)
                this->markOnGoingData(pRequest);
            else
            {
                this->handleDataAndDump(N3D_TO_DPE_MVSV_MASK, pRequest);
                this->onHandleOnGoingReqDataReady(pRequest);
            }
        }
    }

    return bRet;
}

MBOOL
N3DNode::
prepareN3DParam_NORMAL(
    N3D_PVPHASE_ENUM phase,
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM& rN3dInParam,
    N3D_HAL_OUTPUT& rN3dOutParam
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MBOOL bRet = MTRUE;

    if(phase == eN3D_PVPHASE_FULL)
    {
        bRet &= prepareN3DInputBuffer(pRequest, rN3dInParam);
        bRet &= prepareN3DInputMeta(pRequest, rN3dInParam);
        bRet &= prepareN3DOutputYUVMask(pBufferHandler, rN3dOutParam);
    }
    else if(phase == eN3D_PVPHASE_MAIN2_WARPPING)
    {
        // input
        IImageBuffer* pRectIn2Buf=nullptr;
        bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, pRectIn2Buf);
        pRectIn2Buf->syncCache(eCACHECTRL_INVALID);
        bRet &= prepareN3DInputMeta(pRequest, rN3dInParam);
        // output
        IImageBuffer* pImgBuf_SV_Y = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_SV_Y);
        IImageBuffer* pImgBuf_MASK_S = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MASK_S);
        // fill into in/out param
        rN3dInParam.rectifyImgMain2 = pRectIn2Buf;
        rN3dOutParam.rectifyImgMain2 = pImgBuf_SV_Y;
        rN3dOutParam.maskMain2 = pImgBuf_MASK_S;
    }
    //
    if(!bRet)
        MY_LOGE("Failed to prepare the N3DParams, reqID=%d", pRequest->getRequestNo());

    return bRet;
}

MBOOL
N3DNode::
prepareN3DParam_CAP(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM_CAPTURE& rN3dInParam,
    N3D_HAL_OUTPUT_CAPTURE& rN3dOutParam
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MBOOL bRet = MTRUE;
    bRet &= prepareN3DInputBuffer(pRequest, rN3dInParam);
    bRet &= prepareN3DInputMeta_CAP(pRequest, rN3dInParam);
    bRet &= prepareN3DOutputBuffer_CAP(pRequest, rN3dOutParam);
    //
    if(!bRet)
        MY_LOGE("Failed to prepare the Capture N3DParams, reqID=%d", pRequest->getRequestNo());

    return bRet;

}

MBOOL
N3DNode::
prepareN3DInputMeta(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM& rN3dParam
)
{
    VSDOF_LOGD("prepareN3DInputParam");
    CAM_TRACE_NAME("prepareN3DInputParam");
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    //
    MINT32 depthAFON = 0;
    MINT32 disMeasureON = 0;
    MUINT8 isAFTrigger = 0;
    MINT32 magicNum1 = 0;
    MINT32 magicNum2 = 0;
    //
    DepthMapBufferID inAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_APP);
    DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
    //
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), inAppBID);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);
    //
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DEPTH_AF_ON, depthAFON))
        VSDOF_LOGD("reqID=%d, Cannot find MTK_STEREO_FEATURE_DEPTH_AF_ON meta!", pRequest->getRequestNo());
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DISTANCE_MEASURE_ON, disMeasureON))
        VSDOF_LOGD("reqID=%d, Cannot find MTK_STEREO_FEATURE_DISTANCE_MEASURE_ON meta!", pRequest->getRequestNo());
    if(!tryGetMetadata<MUINT8>(pInAppMeta, MTK_CONTROL_AF_TRIGGER, isAFTrigger))
        MY_LOGE("reqID=%d, Cannot find MTK_CONTROL_AF_TRIGGER meta!", pRequest->getRequestNo());

    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum1)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
        return MFALSE;
    }
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum2)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main2!");
        return MFALSE;
    }
    // prepare EIS data
    prepareEISData(pInAppMeta, pInHalMeta_Main1, rN3dParam.eisData);
    // prepare params
    rN3dParam.isAFTrigger = isAFTrigger;
    rN3dParam.isDepthAFON = depthAFON;
    rN3dParam.isDistanceMeasurementON = disMeasureON;
    rN3dParam.magicNumber[0] = magicNum1;
    rN3dParam.magicNumber[1] = magicNum2;
    rN3dParam.requestNumber = pRequest->getRequestNo();
    rN3dParam.timestamp = pRequest->mTimeStamp;

    if(!checkDumpIndex(pRequest->getRequestNo())) {
        rN3dParam.dumpHint = nullptr;
    } else {
        extract(&mDumpHint_Main1, pInHalMeta_Main1);
        rN3dParam.dumpHint = &mDumpHint_Main1;
    }
    return MTRUE;
}

MBOOL
N3DNode::
prepareN3DInputMeta_CAP(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM_CAPTURE& rN3dParam
)
{
    VSDOF_LOGD("prepareN3DInputParam");
    CAM_TRACE_NAME("prepareN3DInputParam");
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    //
    MINT32 magicNum1 = 0;
    MINT32 magicNum2 = 0;
    MINT32 jpegOrientation = 0;
    //
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN2);
    //
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!");
    }
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum1)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
        return MFALSE;
    }
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum2)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main2!");
        return MFALSE;
    }
    // config touch info
    IMetadata::IEntry entry = pInAppMeta->entryFor(MTK_STEREO_FEATURE_TOUCH_POSITION);
    if( !entry.isEmpty() ) {
        rN3dParam.touchPosX = entry.itemAt(0, Type2Type<MINT32>());
        rN3dParam.touchPosY = entry.itemAt(1, Type2Type<MINT32>());
    }
    // prepare params
    rN3dParam.magicNumber[0] = magicNum1;
    rN3dParam.magicNumber[1] = magicNum2;
    rN3dParam.requestNumber = pRequest->getRequestNo();
    rN3dParam.timestamp = pRequest->mTimeStamp;
    rN3dParam.captureOrientation = jpegOrientation;

    return MTRUE;
}

MBOOL
N3DNode::
prepareN3DOutputBuffer_CAP(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_OUTPUT_CAPTURE& rN3dParam
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // prepare YUV/MASK
    prepareN3DOutputYUVMask(pBufferHandler, rN3dParam);
    // prepare LDC buffer, denosie capture does not need this
    IImageBuffer* pImgBuf = nullptr;
    MBOOL bRet= pRequest->getRequestImageBuffer({.bufferID=BID_N3D_OUT_LDC,
                                                .ioType=eBUFFER_IOTYPE_OUTPUT}, pImgBuf);
    if(bRet)
        rN3dParam.ldcMain1 = pImgBuf;
    else
        rN3dParam.ldcMain1 = NULL;

    IImageBuffer* pImgBuf_DepthDebug = nullptr;
    MBOOL bExistDepthDbg = pRequest->getRequestImageBuffer({.bufferID=BID_N3D_OUT_DEPTH_DBG,
                                            .ioType=eBUFFER_IOTYPE_OUTPUT}, pImgBuf_DepthDebug);

   if(bExistDepthDbg)
        rN3dParam.debugBuffer = (MUINT8*) pImgBuf_DepthDebug->getBufVA(0);
    else
        rN3dParam.debugBuffer = NULL;
    return MTRUE;
}

MBOOL
N3DNode::
writeN3DResultToMeta(
    const N3D_HAL_OUTPUT& n3dOutput,
    DepthMapRequestPtr& pRequest
)
{
    DepthMapBufferID outAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_APP);
    IMetadata* pOutAppMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outAppBID);
    VSDOF_LOGD("output distance:%f", n3dOutput.distance);
    trySetMetadata<MFLOAT>(pOutAppMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, n3dOutput.distance);
    // set outAppMeta ready
    pRequest->setOutputBufferReady(outAppBID);

    DepthMapBufferID outHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
    IMetadata* pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outHalBID);
    VSDOF_LOGD("output convOffset:%f", n3dOutput.convOffset);
    trySetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, n3dOutput.convOffset);
    // set outHalMeta ready
    pRequest->setOutputBufferReady(outHalBID);
    // pass data finish
    if(!pRequest->isQueuedDepthRequest(mpPipeOption))
        this->handleData(DEPTHMAP_META_OUT, pRequest);
    return MTRUE;
}

MBOOL
N3DNode::
writeN3DResultToMeta_CAP(
    const N3D_HAL_OUTPUT_CAPTURE& n3dOutput,
    DepthMapRequestPtr& pRequest
)
{
    DepthMapBufferID outAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_APP);
    IMetadata* pOutAppMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outAppBID);
    VSDOF_LOGD("output distance:%d", n3dOutput.distance);
    trySetMetadata<MFLOAT>(pOutAppMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, n3dOutput.distance);
    // set outAppMeta ready
    pRequest->setOutputBufferReady(outAppBID);

    DepthMapBufferID outHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
    IMetadata* pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outHalBID);
    VSDOF_LOGD("output convOffset:%f", n3dOutput.convOffset);
    trySetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, n3dOutput.convOffset);
    // warping matrix size
    VSDOF_LOGD("output warpingMatrixSize:%d", n3dOutput.warpingMatrixSize);
    trySetMetadata<MINT32>(pOutHalMeta, MTK_N3D_WARPING_MATRIX_SIZE, n3dOutput.warpingMatrixSize);
    // set outHalMeta ready
    pRequest->setOutputBufferReady(outHalBID);
    // pass data finish
    this->handleData(DEPTHMAP_META_OUT, pRequest);
    return MTRUE;
}

MBOOL
N3DNode::
performN3DALGO_CAP(DepthMapRequestPtr& pRequest)
{
    CAM_TRACE_NAME("N3DNode::performN3DALGO_CAP");

    N3D_HAL_PARAM_CAPTURE n3dParams;
    N3D_HAL_OUTPUT_CAPTURE n3dOutput;
    MBOOL bRet = prepareN3DParam_CAP(pRequest, n3dParams, n3dOutput);
    // check valid
    if(!bRet)
    {
        MY_LOGE("reqID=%d, failed to prepare CAP N3D params to run N3D Hal.", pRequest->getRequestNo());
        return MFALSE;
    }
    // debug params
    debugN3DParams_Cap(n3dParams, n3dOutput);
    // timer
    pRequest->mTimer.startN3D();
    VSDOF_PRFLOG("start N3D(CAP) ALGO, reqID=%d", pRequest->getRequestNo());

    CAM_TRACE_BEGIN("N3DNode::N3DHALRun");
    bool ret = mpN3DHAL_CAP->N3DHALRun(n3dParams, n3dOutput);
    CAM_TRACE_END();
    //
    pRequest->mTimer.stopN3D();
    if(ret)
    {
        VSDOF_PRFTIME_LOG("finsished N3D(CAP) ALGO, reqID=%d, exe-time=%d msec",
            pRequest->getRequestNo(), pRequest->mTimer.getElapsedN3D());
        // buffer handler
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        // mark LDC buffer ready
        pRequest->setOutputBufferReady(BID_N3D_OUT_LDC);
        // write meta
        writeN3DResultToMeta_CAP(n3dOutput, pRequest);
        // pass MV/SV/MASK to DPE
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_M, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_S, eDPETHMAP_PIPE_NODEID_DPE);
        handleDataAndDump(N3D_TO_DPE_MVSV_MASK, pRequest);
        // mark buffer ready
        pRequest->setOutputBufferReady(BID_N3D_OUT_DEPTH_DBG);
        pRequest->setOutputBufferReady(BID_N3D_OUT_WARPING_MATRIX);
        pRequest->setOutputBufferReady(BID_N3D_OUT_SCENE_INFO);
        // output
        handleDataAndDump(N3D_OUT_JPS_WARPMTX, pRequest);
        // output ExtraData
        char* sExtraData = mpN3DHAL_CAP->getStereoExtraData();
        // captrue FD
        if(!outputExtraData(sExtraData, pRequest))
            return MFALSE;
    }

    return ret;
}

MBOOL
N3DNode::
outputExtraData(
    char* sExtraData,
    DepthMapRequestPtr& pRequest
)
{
    if(mpFlowOption->getOptionConfig().mbCaptureFDEnable)
    {
        pRequest->mpData = (MVOID*) sExtraData;
        // pass to FD
        handleData(N3D_TO_FD_EXTDATA_MASK, pRequest);
        // clear data
        pRequest->mpData = NULL;
    }
    else
    {   // no capture FD
        IImageBuffer* pExtraDataImgBuf = nullptr;
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pExtraDataImgBuf = pBufferHandler->requestBuffer(getNodeId(), BID_FD_OUT_EXTRADATA);
        if(pExtraDataImgBuf == NULL)
        {
            MY_LOGE("pExtraDataImgBuf is NULL!! Failed to generate extra data!");
            return MFALSE;
        }
        else if(strlen(sExtraData)+1 > pExtraDataImgBuf->getBufSizeInBytes(0))
        {
            MY_LOGE("Extra data length is larger than the output buffer size, ExtraData size=%d , output buffer size=%d", strlen(sExtraData), pExtraDataImgBuf->getBufSizeInBytes(0));
            return MFALSE;
        }
        else
        {   // output extradata
            VSDOF_LOGD("Copy result to frame!! result=%s", sExtraData);
            memcpy((MUINT8*) pExtraDataImgBuf->getBufVA(0), sExtraData, strlen(sExtraData)+1);
            pRequest->setOutputBufferReady(BID_FD_OUT_EXTRADATA);
            handleDataAndDump(FD_OUT_EXTRADATA, pRequest);
        }
    }

    return MTRUE;
}


MBOOL
N3DNode::
copyImageBuffer(sp<IImageBuffer> srcImgBuf, sp<IImageBuffer> dstImgBuf)
{
    if(srcImgBuf->getPlaneCount() != dstImgBuf->getPlaneCount())
    {
        MY_LOGE("source/destination image buffer has different plane count! cannot copy!");
        return MFALSE;
    }
    for(int index=0;index<srcImgBuf->getPlaneCount();++index)
    {
        if(srcImgBuf->getBufSizeInBytes(index) != dstImgBuf->getBufSizeInBytes(index))
        {
            MY_LOGE("The %d-st plane of source/destination image buffer has different buffer size! cannot copy!", index);
            return MFALSE;
        }
        // memory copy
        memcpy((MUINT8*)dstImgBuf->getBufVA(index), (MUINT8*)srcImgBuf->getBufVA(index), srcImgBuf->getBufSizeInBytes(index));
    }
    return MTRUE;
}

MBOOL
N3DNode::
prepareN3DInputBuffer(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM_COMMON& rN3dParam
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // prepare FEFM input
    prepareFEFMData(pBufferHandler, rN3dParam.hwfefmData);

    MBOOL bRet=MTRUE;
    // prepare Rect/CC input
    IImageBuffer *pCCIn1Buf = nullptr, *pCCIn2Buf = nullptr;
    IImageBuffer *pRectIn1Buf = nullptr, *pRectIn2Buf = nullptr;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_CC_IN1, pCCIn1Buf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_CC_IN2, pCCIn2Buf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN1, pRectIn1Buf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, pRectIn2Buf);

    if(!bRet)
        return MFALSE;

    rN3dParam.ccImage[0] = pCCIn1Buf;
    rN3dParam.ccImage[1] = pCCIn2Buf;
    rN3dParam.rectifyImgMain1 = pRectIn1Buf;
    rN3dParam.rectifyImgMain2 = pRectIn2Buf;
    // sync cache
    pRectIn1Buf->syncCache(eCACHECTRL_INVALID);
    pRectIn2Buf->syncCache(eCACHECTRL_INVALID);
    pCCIn1Buf->syncCache(eCACHECTRL_INVALID);
    pCCIn2Buf->syncCache(eCACHECTRL_INVALID);



    return MTRUE;
}

MVOID
N3DNode::
debugN3DParams(N3D_HAL_PARAM& inParam, N3D_HAL_OUTPUT& output)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    MY_LOGD("+");
    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataMain1[%d]=%x",i, inParam.hwfefmData.geoDataMain1[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataMain2[%d]=%x",i, inParam.hwfefmData.geoDataMain2[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataLeftToRight[%d]=%x",i, inParam.hwfefmData.geoDataLeftToRight[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataRightToLeft[%d]=%x",i, inParam.hwfefmData.geoDataRightToLeft[i]);
    }

    MY_LOGD("inParam.rectifyImgMain1=%x",inParam.rectifyImgMain1);
    MY_LOGD("inParam.rectifyImgMain2=%x",inParam.rectifyImgMain2);
    MY_LOGD("inParam.ccImage[0]=%x",inParam.ccImage[0]);
    MY_LOGD("inParam.ccImage[1]=%x", inParam.ccImage[1]);
    MY_LOGD("inParam.magicNumber=%d, %d", inParam.magicNumber[0], inParam.magicNumber[1]);
    MY_LOGD("inParam.requestNumber=%d", inParam.requestNumber);
    MY_LOGD("inParam.isAFTrigger=%d", inParam.isAFTrigger);
    MY_LOGD("inParam.isDepthAFON=%d", inParam.isDepthAFON);
    MY_LOGD("inParam.isDistanceMeasurementON=%d", inParam.isDistanceMeasurementON);

    MY_LOGD("inParam.eisData.isON=%d", inParam.eisData.isON);
    MY_LOGD("inParam.eisData.eisOffset=%d", inParam.eisData.eisOffset);
    MY_LOGD("inParam.eisData.eisImgSize=%dx%d", inParam.eisData.eisImgSize.w, inParam.eisData.eisImgSize.h);

    MY_LOGD("output.rectifyImgMain1=%x",output.rectifyImgMain1);
    MY_LOGD("output.maskMain1=%x",output.maskMain1);
    MY_LOGD("output.rectifyImgMain2=%x",output.rectifyImgMain2);
    MY_LOGD("output.maskMain2=%x",output.maskMain2);
    MY_LOGD("-");
}

MVOID
N3DNode::
debugN3DParams_Cap(N3D_HAL_PARAM_CAPTURE& inParam, N3D_HAL_OUTPUT_CAPTURE& output)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    MY_LOGD("+");
    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataMain1[%d]=%x",i, inParam.hwfefmData.geoDataMain1[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataMain2[%d]=%x",i, inParam.hwfefmData.geoDataMain2[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataLeftToRight[%d]=%x",i, inParam.hwfefmData.geoDataLeftToRight[i]);
    }

    for(int i=0;i<3;i++)
    {
        MY_LOGD("inParam.hwfefmData.geoDataRightToLeft[%d]=%x",i, inParam.hwfefmData.geoDataRightToLeft[i]);
    }

    MY_LOGD("inParam.rectifyImgMain1=%x",inParam.rectifyImgMain1);
    MY_LOGD("inParam.rectifyGBMain2=%x",inParam.rectifyImgMain2);
    MY_LOGD("inParam.ccImage[0]=%x",inParam.ccImage[0]);
    MY_LOGD("inParam.ccImage[1]=%x", inParam.ccImage[1]);
    MY_LOGD("inParam.magicNumber=%d, %d", inParam.magicNumber[0], inParam.magicNumber[1]);
    MY_LOGD("inParam.captureOrientation=%x", inParam.captureOrientation);
    MY_LOGD("output.rectifyImgMain1=%x",output.rectifyImgMain1);
    MY_LOGD("output.maskMain1=%x",output.maskMain1);
    MY_LOGD("output.rectifyImgMain2=%x",output.rectifyImgMain2);
    MY_LOGD("output.maskMain2=%x",output.maskMain2);
    MY_LOGD("output.ldcMain1=%x",output.ldcMain1);
    MY_LOGD("-");
}

MBOOL
N3DNode::
prepareN3DOutputYUVMask(
    sp<BaseBufferHandler>& pBufferHandler,
    N3D_HAL_OUTPUT& rN3dParam
)
{
    // request output buffers
    IImageBuffer* pImgBuf_MV_Y = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MV_Y);
    IImageBuffer* pImgBuf_SV_Y = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_SV_Y);
    IImageBuffer* pImgBuf_MASK_M = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MASK_M);
    IImageBuffer* pImgBuf_MASK_S = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MASK_S);

    // insert to OutputInfoMap and fill in the N3D output params
    rN3dParam.rectifyImgMain1 = pImgBuf_MV_Y;
    rN3dParam.rectifyImgMain2 = pImgBuf_SV_Y;
    rN3dParam.maskMain1 = pImgBuf_MASK_M;
    rN3dParam.maskMain2 = pImgBuf_MASK_S;

    VSDOF_LOGD("rN3dParam.rectifyImgMain1 size=%dx%d plane_bytes=(%d, %d, %d)", pImgBuf_MV_Y->getImgSize().w,
                pImgBuf_MV_Y->getImgSize().h, pImgBuf_MV_Y->getBufSizeInBytes(0),
                pImgBuf_MV_Y->getBufSizeInBytes(1),pImgBuf_MV_Y->getBufSizeInBytes(2));
    VSDOF_LOGD("rN3dParam.rectifyImgMain2 size=%dx%d plane_bytes=(%d, %d, %d)", pImgBuf_SV_Y->getImgSize().w,
                pImgBuf_SV_Y->getImgSize().h, pImgBuf_SV_Y->getBufSizeInBytes(0),
                pImgBuf_SV_Y->getBufSizeInBytes(1), pImgBuf_SV_Y->getBufSizeInBytes(2));
    VSDOF_LOGD("rN3dParam.maskMain1 size=%dx%d plane_bytes=(%d)", pImgBuf_MASK_M->getImgSize().w,
                pImgBuf_MASK_M->getImgSize().h, pImgBuf_MASK_M->getBufSizeInBytes(0));
    VSDOF_LOGD("rN3dParam.maskMain2 size=%dx%d plane_bytes=(%d)", pImgBuf_MASK_S->getImgSize().w,
                pImgBuf_MASK_S->getImgSize().h, pImgBuf_MASK_S->getBufSizeInBytes(0));

    return MTRUE;
}

MBOOL
N3DNode::
prepareEISData(
    IMetadata*& pInAppMeta,
    IMetadata*& pInHalMeta_Main1,
    EIS_DATA& rEISData
)
{
    if(isEISOn(pInAppMeta))
    {
        eis_region region;
        if(queryEisRegion(pInHalMeta_Main1, region))
        {
            rEISData.isON = true;
            rEISData.eisOffset.x = region.x_int;
            rEISData.eisOffset.y = region.y_int;
            rEISData.eisImgSize = region.s;
        }
        else
            return MFALSE;
    }
    else
    {
        rEISData.isON = false;
    }
    return MTRUE;
}

MBOOL
N3DNode::
prepareFEFMData(sp<BaseBufferHandler>& pBufferHandler, HWFEFM_DATA& rFefmData)
{
    CAM_TRACE_NAME("prepareFEFMData");
    VSDOF_LOGD("prepareFEFMData");
    // N3D input FEO/FMO data
    IImageBuffer *pFe1boBuf = nullptr, *pFe2boBuf = nullptr;
    IImageBuffer *pFe1coBuf = nullptr, *pFe2coBuf = nullptr;
    IImageBuffer *pFmboLRBuf = nullptr, *pFmboRLBuf = nullptr;
    IImageBuffer *pFmcoLRBuf = nullptr, *pFmcoRLBuf = nullptr;

    // Get FEO
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE1BO, pFe1boBuf);
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2BO, pFe2boBuf);
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE1CO, pFe1coBuf);
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2CO, pFe2coBuf);
    // syncCache
    pFe1boBuf->syncCache(eCACHECTRL_INVALID);
    pFe2boBuf->syncCache(eCACHECTRL_INVALID);
    pFe1coBuf->syncCache(eCACHECTRL_INVALID);
    pFe2coBuf->syncCache(eCACHECTRL_INVALID);

    // insert params
    rFefmData.geoDataMain1[0] = pFe1boBuf;
    rFefmData.geoDataMain1[1] = pFe1coBuf;
    rFefmData.geoDataMain1[2] = NULL;
    rFefmData.geoDataMain2[0] = pFe2boBuf;
    rFefmData.geoDataMain2[1] = pFe2coBuf;
    rFefmData.geoDataMain2[2] = NULL;

    // Get FMO
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMBO_LR, pFmboLRBuf);
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMBO_RL, pFmboRLBuf);
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMCO_LR, pFmcoLRBuf);
    pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMCO_RL, pFmcoRLBuf);
    // sync cache
    pFmboLRBuf->syncCache(eCACHECTRL_INVALID);
    pFmboRLBuf->syncCache(eCACHECTRL_INVALID);
    pFmcoLRBuf->syncCache(eCACHECTRL_INVALID);
    pFmcoRLBuf->syncCache(eCACHECTRL_INVALID);

    // insert params
    rFefmData.geoDataLeftToRight[0] = pFmboLRBuf;
    rFefmData.geoDataLeftToRight[1] = pFmcoLRBuf;
    rFefmData.geoDataLeftToRight[2] = NULL;
    rFefmData.geoDataRightToLeft[0] = pFmboRLBuf;
    rFefmData.geoDataRightToLeft[1] = pFmcoRLBuf;
    rFefmData.geoDataRightToLeft[2] = NULL;

    return MTRUE;
}

MVOID
N3DNode::
onFlush()
{
    MY_LOGD("+ extDep=%d", this->getExtThreadDependency());
    DepthMapRequestPtr pRequest;
    while( mPriorityQueue.deque(pRequest) )
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
    DepthMapPipeNode::onFlush();
    MY_LOGD("-");
}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam



