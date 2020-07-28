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
#include "../StageExecutionTime.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "N3DNode"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_N3D);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {


const char* PHASE2Name(N3D_PVPHASE_ENUM phase)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;

  switch(phase)
  {
    MAKE_NAME_CASE(eN3D_PVPHASE_MAIN1_PADDING);
    MAKE_NAME_CASE(eN3D_PVPHASE_MAIN2_WARPPING);
    MAKE_NAME_CASE(eN3D_PVPHASE_LEARNING);
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
, DataSequential(reqOrderGetter, "N3DSeq", "order")
{
    this->addWaitQueue(&mPriorityQueue);
    // set request order allow not-in-order ready
    DataSequential::allowNotInOrderReady(MTRUE);
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
    CAM_ULOGM_TAGLIFE("N3DNode::onThreadStart");
    // create N3DHAL instance - Capture
    N3D_HAL_INIT_PARAM_WPE initParam_CAP;
    initParam_CAP.eScenario  = eSTEREO_SCENARIO_CAPTURE;
    initParam_CAP.fefmRound  = VSDOF_CONST_FE_EXEC_TIMES;

    MBOOL bRet = mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_WARPMTX_MAIN2_X,
                                        eBUFFER_POOL_SCENARIO_CAPTURE,
                                        initParam_CAP.outputWarpMapMain2[0]);
    bRet &= mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_WARPMTX_MAIN2_Y,
                                        eBUFFER_POOL_SCENARIO_CAPTURE,
                                        initParam_CAP.outputWarpMapMain2[1]);
    if(!bRet)
    {
        MY_LOGE("Failed to get all pool imagebuffer");
        return MFALSE;
    }
    mpN3DHAL_CAP = N3D_HAL::createInstance(initParam_CAP);
    // create N3DHAL instance - Preview/Record
    N3D_HAL_INIT_PARAM_WPE initParam_VRPV;
    initParam_VRPV.eScenario = eSTEREO_SCENARIO_PREVIEW;
    initParam_VRPV.fefmRound = VSDOF_CONST_FE_EXEC_TIMES;
    //
    bRet = mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_WARPMTX_MAIN2_X,
                                        eBUFFER_POOL_SCENARIO_PREVIEW,
                                        initParam_VRPV.outputWarpMapMain2[0]);
    bRet &= mpBuffPoolMgr->getAllPoolImageBuffer(BID_N3D_OUT_WARPMTX_MAIN2_Y,
                                        eBUFFER_POOL_SCENARIO_PREVIEW,
                                        initParam_VRPV.outputWarpMapMain2[1]);
    mpN3DHAL_VRPV  = N3D_HAL::createInstance(initParam_VRPV);

    if(!bRet)
    {
        MY_LOGE("Failed to get all preview pool imagebuffer");
        return MFALSE;
    }
    //
    miMaxWorkingSize = CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS);
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
    VSDOF_LOGD("+ : dataID=%s reqId=%d size=%d", ID2Name(data), pRequest->getRequestNo(), mPriorityQueue.size());

    switch(data)
    {
        // first phase
        case P2A_TO_N3D_PADDING_MATRIX:
        // capture flow
        case P2A_TO_N3D_CAP_RECT2:
        // need FEFM - second phase
        case P2A_TO_N3D_RECT2_FEO:
        // NOFEFM - second phase
        case P2A_TO_N3D_NOFEFM_RECT2:
        // learning - third phase
        case P2A_TO_N3D_FEOFMO:
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

MBOOL
N3DNode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;

    if( !waitAnyQueue() )
    {
        return MFALSE;
    }
    //
    if( !mPriorityQueue.deque(pRequest) )
    {
        MY_LOGE("mPriorityQueue.deque() failed");
        return MFALSE;
    }
    // not enough working set -> wait for latest
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL &&
        DataSequential::getQueuedDataSize() >= miMaxWorkingSize)
    {
        DepthMapRequestPtr pOldestRequest = DataSequential::getOldestDataOnGoingSequence();
        VSDOF_LOGD("Enter not enough working set situation, deque oldest reqID=%d order=%d",
                    pOldestRequest->getRequestNo(), pOldestRequest->getRequestOrder());
        // if not the oldest
        if(pOldestRequest->getRequestOrder() != pRequest->getRequestOrder())
        {
            MY_LOGW("Failed to get oldest request! oldest order=%d   deque request order=%d",
                        pOldestRequest->getRequestOrder(), pRequest->getRequestOrder());
            // push back
            mPriorityQueue.enque(pRequest);
            // and return
            return MTRUE;
        }
    }
    // mark on-going-request start
    this->incExtThreadDependency();
    VSDOF_LOGD("threadLoop start, reqID=%d order=%d needFEFM=%d",
                pRequest->getRequestNo(), pRequest->getRequestOrder(), pRequest->getRequestAttr().needFEFM);
    CAM_ULOGM_TAGLIFE("N3DNode::onThreadLoop");
    //
    MBOOL ret;
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        ret = performN3DALGO_CAP(pRequest);
    }
    else
    {
        // process done are launched inside it
        ret = performN3DALGO_VRPV(pRequest);
    }
    pRequest->mTimer.stopN3D();
    // error handling
    if(!ret)
    {
        #ifdef UNDER_DEVELOPMENT
        AEE_ASSERT("N3DRun fail, reqID=%d", pRequest->getRequestNo());
        #endif
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  N3DNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

N3D_PVPHASE_ENUM
N3DNode::
getPreviewPhase(DepthMapRequestPtr pRequest)
{
    return pRequest->mN3DPhase;
}

MBOOL
N3DNode::
performN3DALGO_VRPV(DepthMapRequestPtr& pRequest)
{
    CAM_ULOGM_TAGLIFE("N3DNode::performN3DALGO_VRPV");
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    N3D_PVPHASE_ENUM phase = this->getPreviewPhase(pRequest);

    VSDOF_LOGD("reqID=%d performN3DALGO_VRPV phase=%s", pRequest->getRequestNo(), PHASE2Name(phase));
    bool bRet = true;
    if(phase == eN3D_PVPHASE_MAIN1_PADDING)
    {
        // start timer
        pRequest->mTimer.startN3D();
        // mark on going
        this->markOnGoingData(pRequest);
        // input
        IImageBuffer* pRectIn1Buf=pBufferHandler->requestBuffer(getNodeId(), BID_P2A_IN_RECT_IN1);
        pRectIn1Buf->syncCache(eCACHECTRL_INVALID);
        // output
        IImageBuffer* pImgBuf_MV_Y = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MV_Y);
        IImageBuffer* pImgBuf_MASK_M = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_MASK_M);
        //
        CAM_ULOGM_TAG_BEGIN("N3DNode::N3DHALRun Main1 Warping");
        pRequest->mTimer.startN3DMain1Padding();
        bRet = mpN3DHAL_VRPV->N3DHALWarpMain1(pRectIn1Buf, pImgBuf_MV_Y, pImgBuf_MASK_M);
        pRequest->mTimer.stopN3DMain1Padding();
        if(!bRet)
            MY_LOGE("reqID=%d, Generate MV_Y/MASK_M failed.", pRequest->getRequestNo());
        // generate warping mtx
        N3D_HAL_PARAM_WPE inParam;
        if(!prepareN3DInputMeta(pRequest, inParam))
        {
            MY_LOGE("reqID=%d, prepare input meta failed.", pRequest->getRequestNo());
            bRet = MFALSE;
        }
        N3D_HAL_OUTPUT_WPE outParam;
        outParam.warpMapMain2[0] = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_X);
        outParam.warpMapMain2[1] = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_Y);
        debugN3DParams(outParam);
        //
        pRequest->mTimer.startN3DMaskWarping();
        bRet &= mpN3DHAL_VRPV->getWarpMapMain2(inParam, outParam);
        pRequest->mTimer.stopN3DMaskWarping();
        if(!bRet)
            MY_LOGE("reqID=%d, Generate WarpingMatrix failed.", pRequest->getRequestNo());
        else
            writeN3DResultToMeta(outParam, pRequest);
        VSDOF_LOGD("start N3D(PV/VR) ALGO - Main1 Padding + Generate Main2 Mask, reqID=%d time=%d/%d ms",
                    pRequest->getRequestNo(), pRequest->mTimer.getElapsedN3DMain1Padding(), pRequest->mTimer.getElapsedN3DMaskWarping());
        CAM_ULOGM_TAG_END();
    }
    else if(phase == eN3D_PVPHASE_MAIN2_WARPPING)
    {
        IImageBuffer* pBuf = nullptr;
        bRet = pBufferHandler->getEnqueBuffer(eDPETHMAP_PIPE_NODEID_WPE, BID_N3D_OUT_WARPMTX_MAIN2_X, pBuf);
        bRet &= pBufferHandler->getEnqueBuffer(eDPETHMAP_PIPE_NODEID_WPE, BID_N3D_OUT_WARPMTX_MAIN2_Y, pBuf);
        VSDOF_LOGD("N3D(PV/VR) Main2 warping stage: check the matrix is ready: %d, reqID=%d",
                         bRet, pRequest->getRequestNo());
        if(!bRet)
        {
            MY_LOGE("WarpingMatrix is not ready.");
        }

    }
    else if(phase == eN3D_PVPHASE_LEARNING)
    {
        HWFEFM_DATA fefmData;
        if(!prepareFEFMData(pBufferHandler, fefmData))
            return MFALSE;

        pRequest->mTimer.startN3DLearning();
        bRet = mpN3DHAL_VRPV->runN3DLearning(fefmData);
        pRequest->mTimer.stopN3DLearning();
        VSDOF_LOGD("N3D(PV/VR) Run learningstage. reqID=%d time=%d ms"
                         , pRequest->getRequestNo(), pRequest->mTimer.getElapsedN3DLearning());
    }
    //
    if(bRet)
    {
        if(phase == eN3D_PVPHASE_MAIN1_PADDING)
        {
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_M, eDPETHMAP_PIPE_NODEID_DPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_X, eDPETHMAP_PIPE_NODEID_WPE);
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_Y, eDPETHMAP_PIPE_NODEID_WPE);
            handleDataAndDump(N3D_TO_P2ABYER_P1YUV_USED, pRequest);
            pRequest->mN3DPhase = eN3D_PVPHASE_MAIN2_WARPPING;
        }
        else if(phase == eN3D_PVPHASE_MAIN2_WARPPING)
        {
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_WPE);
            handleData(N3D_TO_WPE_RECT2_WARPMTX, pRequest);
            //
            if(!pRequest->getRequestAttr().needFEFM)
            {
                // sequential: mark finish
                this->markFinishNoInOder(pRequest);
                //
                pBufferHandler->onProcessDone(getNodeId());
            }
            else{
                mvToDoLearningReqIDs.add(pRequest->getRequestNo(), MTRUE);
                pRequest->mN3DPhase = eN3D_PVPHASE_LEARNING;
            }
            // n3d stop timer
            pRequest->mTimer.stopN3D();
        }
        else
        {
            // sequential: mark finish
            this->markFinishNoInOder(pRequest);
            // remove todo item
            mvToDoLearningReqIDs.removeItem(pRequest->getRequestNo());
            pBufferHandler->onProcessDone(getNodeId());
        }
    }

    return bRet;
}

MBOOL
N3DNode::
prepareN3DInputMeta(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM_WPE& rN3dParam
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    //
    MINT32 magicNum1 = 0;
    MINT32 magicNum2 = 0;
    //
    DepthMapBufferID inHalMetaBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    DepthMapBufferID inHalMetaBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalMetaBID_Main1);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalMetaBID_Main2);
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum1)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
        return MFALSE;
    }
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum2)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main2!");
        return MFALSE;
    }
    // prepare params
    rN3dParam.magicNumber[0] = magicNum1;
    rN3dParam.magicNumber[1] = magicNum2;
    rN3dParam.requestNumber = pRequest->getRequestNo();
    //
    if(!checkToDump(N3D_TO_P2ABYER_P1YUV_USED, pRequest))
    {
        rN3dParam.dumpHint = nullptr;
    }
    else
    {
        extract(&mDumpHint_Main1, pInHalMeta_Main1);
        rN3dParam.dumpHint = &mDumpHint_Main1;
    }
    return MTRUE;
}

MBOOL
N3DNode::
writeN3DResultToMeta(
    const N3D_HAL_OUTPUT_WPE& n3dOutput,
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
performN3DALGO_CAP(DepthMapRequestPtr& pRequest)
{
    CAM_ULOGM_TAGLIFE("N3DNode::performN3DALGO_CAP");
    // start timer
    pRequest->mTimer.startN3D();
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    MY_LOGD("performN3DALGO_CAP reqID=%d", pRequest->getRequestNo());
    // do learning first
    MBOOL bRet = MTRUE;
    N3D_HAL_PARAM_WPE inParam;
    if(!prepareN3DInputMeta(pRequest, inParam))
    {
        MY_LOGE("reqID=%d, prepare input meta failed.", pRequest->getRequestNo());
        bRet = MFALSE;
    }

    HWFEFM_DATA fefmData;
    fefmData.magicNumber[0] = inParam.magicNumber[0];
    fefmData.magicNumber[1] = inParam.magicNumber[1];
    fefmData.dumpHint = inParam.dumpHint;
    if(!prepareFEFMData(pBufferHandler, fefmData))
        return MFALSE;

    pRequest->mTimer.startN3DLearning();
    bRet = mpN3DHAL_CAP->runN3DLearning(fefmData);
    pRequest->mTimer.stopN3DLearning();
    VSDOF_PRFLOG("N3D(CAP) Run learningstage. reqID=%d time=%d ms"
                    , pRequest->getRequestNo(), pRequest->mTimer.getElapsedN3DLearning());
    // generate warping mtx next
    N3D_HAL_OUTPUT_WPE outParam;
    outParam.warpMapMain2[0] = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_X);
    outParam.warpMapMain2[1] = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_Y);
    debugN3DParams(outParam);
    //
    pRequest->mTimer.startN3DMaskWarping();
    bRet &= mpN3DHAL_CAP->getWarpMapMain2(inParam, outParam);
    pRequest->mTimer.stopN3DMaskWarping();
    if(!bRet)
        MY_LOGE("reqID=%d, Generate capture WarpingMatrix failed.", pRequest->getRequestNo());
    else
    {
        VSDOF_PRFLOG("N3D(CAP) Run : Generate Main2 Mask, reqID=%d time=%d ms",
                    pRequest->getRequestNo(), pRequest->mTimer.getElapsedN3DMaskWarping());
        // write meta
        writeN3DResultToMeta(outParam, pRequest);
        // config output
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_X, eDPETHMAP_PIPE_NODEID_WPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_Y, eDPETHMAP_PIPE_NODEID_WPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_WPE);
        // pass to WPE
        this->handleDataAndDump(N3D_TO_WPE_RECT2_WARPMTX, pRequest);
    }
    // handle process done
    pBufferHandler->onProcessDone(getNodeId());

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

MVOID
N3DNode::
debugN3DParams(N3D_HAL_OUTPUT_WPE& param)
{
    if(DepthPipeLoggingSetup::mbDebugLog<2)
        return;

    MY_LOGD("+");
    for(int i=0;i<WPE_PLANE_COUNT;i++)
    {
        MY_LOGD("param.warpMapMain2[%d]=%x", i, param.warpMapMain2[i]);
        MY_LOGD("param.warpMapMain1[%d]=%x", i, param.warpMapMain1[i]);
    }
    MY_LOGD("-");
}

MBOOL
N3DNode::
prepareFEFMData(sp<BaseBufferHandler>& pBufferHandler, HWFEFM_DATA& rFefmData)
{
    CAM_ULOGM_TAGLIFE("prepareFEFMData");
    VSDOF_LOGD("prepareFEFMData");
    // N3D input FEO/FMO data
    IImageBuffer *pFe1boBuf = nullptr, *pFe2boBuf = nullptr;
    IImageBuffer *pFe1coBuf = nullptr, *pFe2coBuf = nullptr;
    IImageBuffer *pFmboLRBuf = nullptr, *pFmboRLBuf = nullptr;
    IImageBuffer *pFmcoLRBuf = nullptr, *pFmcoRLBuf = nullptr;

    MBOOL bRet = MTRUE;
    // Get FEO
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE1BO, pFe1boBuf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2BO, pFe2boBuf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE1CO, pFe1coBuf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2CO, pFe2coBuf);
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
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMBO_LR, pFmboLRBuf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMBO_RL, pFmboRLBuf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMCO_LR, pFmcoLRBuf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FMCO_RL, pFmcoRLBuf);
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

    if(!bRet)
        MY_LOGE("Failed to get FEFM buffers: %x %x %x %x %x %x %x %x",
                pFe1boBuf, pFe1coBuf, pFe2boBuf, pFe2coBuf,
                pFmboLRBuf, pFmcoLRBuf, pFmboRLBuf, pFmcoRLBuf);

    return bRet;
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



