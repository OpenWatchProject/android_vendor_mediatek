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
{
    this->addWaitQueue(&mJobQueue);
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
    mJobQueue.clear();
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
    mpN3DHAL_CAP = N3D_HAL::createInstance(initParam_CAP);
    // create N3DHAL instance - Preview/Record  when not denoise only mode
    if(mpPipeOption->mFeatureMode != eDEPTHNODE_MODE_DENOISE)
    {
        N3D_HAL_INIT_PARAM initParam_VRPV;
        initParam_VRPV.eScenario = eSTEREO_SCENARIO_RECORD;
        initParam_VRPV.fefmRound = VSDOF_CONST_FE_EXEC_TIMES;
        mpN3DHAL_VRPV  = N3D_HAL::createInstance(initParam_VRPV);
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
    VSDOF_LOGD("+ : dataID=%d reqId=%d", data, pRequest->getRequestNo());

    switch(data)
    {
        case P2A_TO_N3D_FEFM_CCin:
            VSDOF_PRFLOG("+ : reqID=%d size=%d", pRequest->getRequestNo(), mJobQueue.size());
            mJobQueue.enque(pRequest);
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

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mJobQueue.deque(pRequest) )
    {
        MY_LOGE("N3DNode mJobQueue.deque() failed");
        return MFALSE;
    }
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_PRFLOG("threadLoop start, reqID=%d", pRequest->getRequestNo());
    CAM_TRACE_NAME("N3DNode::onThreadLoop");

    MBOOL ret;
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        ret = performN3DALGO_CAP(pRequest);
    }
    else
    {
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

    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  N3DNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
N3DNode::
performN3DALGO_VRPV(DepthMapRequestPtr& pRequest)
{
    CAM_TRACE_NAME("N3DNode::performN3DALGO_VRPV");
    N3D_HAL_PARAM n3dParams;
    N3D_HAL_OUTPUT n3dOutput;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // prepare input params
    MBOOL bRet = prepareN3DInputParam(pRequest, n3dParams);
    // prepare output params
    bRet &= prepareN3DOutputParam(pBufferHandler, n3dOutput);
    // check valid
    if(!bRet)
    {
        MY_LOGE("reqID=%d, failed to prepare PV/VR N3D params to run N3D Hal.", pRequest->getRequestNo());
        return MFALSE;
    }

    // debug param
    debugN3DParams(n3dParams, n3dOutput);
    // timer
    pRequest->mTimer.startN3D();
    VSDOF_PRFLOG("start N3D(PV/VR) ALGO, reqID=%d", pRequest->getRequestNo());
    CAM_TRACE_BEGIN("N3DNode::N3DHALRun");
    bool ret = mpN3DHAL_VRPV->N3DHALRun(n3dParams, n3dOutput);
    CAM_TRACE_END();
    //
    pRequest->mTimer.stopN3D();
    if(ret)
    {
        VSDOF_PRFTIME_LOG("finsished N3D(PV/VR) ALGO, reqID=%d, exec-time=%d msec",
                pRequest->getRequestNo(), pRequest->mTimer.getElapsedN3D());
        writeN3DResultToMeta(n3dOutput, pRequest);
        // pass LDC notify to OCC
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_LDC, eDPETHMAP_PIPE_NODEID_OCC);
        this->handleDataAndDump(N3D_TO_OCC_LDC, pRequest);
        // pass MV/SV/MASK to DPE
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_M, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MASK_S, eDPETHMAP_PIPE_NODEID_DPE);
        this->handleDataAndDump(N3D_TO_DPE_MVSV_MASK, pRequest);
    }

    return ret;

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
    VSDOF_LOGD("output distance:%d", n3dOutput.distance);
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

    // prepare input params
    MBOOL bRet = prepareN3DInputParam_CAP(pRequest, n3dParams);
    // prepare output params
    bRet &= prepareN3DOutputParam_CAP(pRequest, n3dOutput);
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
        // pass LDC notify to OCC
        pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_LDC, eDPETHMAP_PIPE_NODEID_OCC);
        handleDataAndDump(N3D_TO_OCC_LDC, pRequest);
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
prepareN3DInputParam(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM& rN3dParam
)
{
    VSDOF_LOGD("prepareN3DInputParam");
    CAM_TRACE_NAME("prepareN3DInputParam");
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // prepare FEFM input
    prepareFEFMData(pBufferHandler, rN3dParam.hwfefmData);

    MBOOL bRet = MTRUE;
    // prepare Rect/CC input
    IImageBuffer *pCCIn1Buf = nullptr, *pCCIn2Buf = nullptr;
    IImageBuffer *pRectIn1Buf = nullptr, *pRectIn2Buf = nullptr;
    // get
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_CC_IN1, pCCIn1Buf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_CC_IN2, pCCIn2Buf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN1, pRectIn1Buf);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, pRectIn2Buf);

    if(!bRet)
        return MFALSE;
    // sync cache
    pRectIn1Buf->syncCache(eCACHECTRL_INVALID);
    pRectIn2Buf->syncCache(eCACHECTRL_INVALID);
    pCCIn1Buf->syncCache(eCACHECTRL_INVALID);
    pCCIn2Buf->syncCache(eCACHECTRL_INVALID);
    // insert param
    rN3dParam.rectifyImgMain1 = pRectIn1Buf;
    rN3dParam.rectifyImgMain2 = pRectIn2Buf;
    rN3dParam.ccImage[0] = pCCIn1Buf;
    rN3dParam.ccImage[1] = pCCIn2Buf;

    MINT32 depthAFON = 0;
    MINT32 disMeasureON = 0;
    MUINT8 isAFTrigger = 0;
    MINT32 magicNum1 = 0;
    MINT32 magicNum2 = 0;

    DepthMapBufferID inAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_APP);
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), inAppBID);
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DEPTH_AF_ON, depthAFON))
        VSDOF_LOGD("reqID=%d, Cannot find MTK_STEREO_FEATURE_DEPTH_AF_ON meta!", pRequest->getRequestNo());

    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DISTANCE_MEASURE_ON, disMeasureON))
        VSDOF_LOGD("reqID=%d, Cannot find MTK_STEREO_FEATURE_DISTANCE_MEASURE_ON meta!", pRequest->getRequestNo());

    if(!tryGetMetadata<MUINT8>(pInAppMeta, MTK_CONTROL_AF_TRIGGER, isAFTrigger))
        MY_LOGE("reqID=%d, Cannot find MTK_CONTROL_AF_TRIGGER meta!", pRequest->getRequestNo());

    DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
    DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);

    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum1))
        MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main1", pRequest->getRequestNo());
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum2))
        MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main2", pRequest->getRequestNo());

    // prepare params
    rN3dParam.magicNumber[0] = magicNum1;
    rN3dParam.magicNumber[1] = magicNum2;
    rN3dParam.requestNumber = pRequest->getRequestNo();
    rN3dParam.isAFTrigger = isAFTrigger;
    rN3dParam.isDepthAFON = depthAFON;
    rN3dParam.isDistanceMeasurementON = disMeasureON;
    // prepare EIS data
    prepareEISData(pInAppMeta, pInHalMeta_Main1, rN3dParam.eisData);
    return MTRUE;
}

MBOOL
N3DNode::
prepareN3DInputParam_CAP(
    DepthMapRequestPtr& pRequest,
    N3D_HAL_PARAM_CAPTURE& rN3dParam
)
{
    CAM_TRACE_NAME("prepareN3DInputParam_CAP");
    VSDOF_LOGD("prepareN3DInputParam_CAP");

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
    rN3dParam.rectifyGBMain2 = pRectIn2Buf;

    // input magic number/orientation
    MINT32 magicNum1 = 0;
    MINT32 magicNum2 = 0;
    MINT32 jpegOrientation = 0;

    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
    DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);

    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum1)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
    }
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum2)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main2!");
    }
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!");
    }
    //prepare EIS data
    rN3dParam.magicNumber[0] = magicNum1;
    rN3dParam.magicNumber[1] = magicNum2;
    rN3dParam.requestNumber = pRequest->getRequestNo();
    rN3dParam.captureOrientation = jpegOrientation;

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
    MY_LOGD("output.ldcMain1=%x",output.ldcMain1);
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
    MY_LOGD("inParam.rectifyGBMain2=%x",inParam.rectifyGBMain2);
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
prepareN3DOutputParam(
    sp<BaseBufferHandler>& pBufferHandler,
    N3D_HAL_OUTPUT& rN3dParam
)
{
    // prepare YUV/MASK
    prepareN3DOutputYUVMask(pBufferHandler, rN3dParam);

    // request LDC buffer
    IImageBuffer* pImgBuf_LDC = pBufferHandler->requestBuffer(getNodeId(), BID_N3D_OUT_LDC);
    rN3dParam.ldcMain1 = (MUINT8*) pImgBuf_LDC->getBufVA(0);

    VSDOF_LOGD("rN3dParam.ldcMain1 size=%dx%d plane_bytes=(%d)", pImgBuf_LDC->getImgSize().w,
                pImgBuf_LDC->getImgSize().h, pImgBuf_LDC->getBufSizeInBytes(0));

    return MTRUE;
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
    rN3dParam.maskMain1 = (MUINT8*) pImgBuf_MASK_M->getBufVA(0);
    rN3dParam.maskMain2 = (MUINT8*) pImgBuf_MASK_S->getBufVA(0);

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
prepareN3DOutputParam_CAP(
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
        rN3dParam.ldcMain1 = (MUINT8*) pImgBuf->getBufVA(0);
    else
        rN3dParam.ldcMain1 = NULL;
    // warping matrix
    IImageBuffer* pImgBuf_WarpingMatrix = nullptr;
    MBOOL bExistWarpMtx = pRequest->getRequestImageBuffer({.bufferID=BID_N3D_OUT_WARPING_MATRIX,
                                            .ioType=eBUFFER_IOTYPE_OUTPUT}, pImgBuf_WarpingMatrix);
    if(bExistWarpMtx)
        rN3dParam.warpingMatrix = (MFLOAT*) pImgBuf_WarpingMatrix->getBufVA(0);
    else
        rN3dParam.warpingMatrix = NULL;

     // SceneInfo
    IImageBuffer* pImgBuf_SceneInfo = nullptr;
    MBOOL bExistSceneInfo = pRequest->getRequestImageBuffer({.bufferID=BID_N3D_OUT_SCENE_INFO,
                                            .ioType=eBUFFER_IOTYPE_OUTPUT}, pImgBuf_SceneInfo);

    if(bExistSceneInfo)
        rN3dParam.sceneInfo = (MINT32*) pImgBuf_SceneInfo->getBufVA(0);
    else
        rN3dParam.sceneInfo = NULL;

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
    rFefmData.geoDataMain1[0] = (MUINT16*)pFe1boBuf->getBufVA(0);
    rFefmData.geoDataMain1[1] = (MUINT16*)pFe1coBuf->getBufVA(0);
    rFefmData.geoDataMain1[2] = NULL;
    rFefmData.geoDataMain2[0] = (MUINT16*)pFe2boBuf->getBufVA(0);
    rFefmData.geoDataMain2[1] = (MUINT16*)pFe2coBuf->getBufVA(0);
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
    rFefmData.geoDataLeftToRight[0] = (MUINT16*) pFmboLRBuf->getBufVA(0);
    rFefmData.geoDataLeftToRight[1] = (MUINT16*) pFmcoLRBuf->getBufVA(0);
    rFefmData.geoDataLeftToRight[2] = NULL;
    rFefmData.geoDataRightToLeft[0] = (MUINT16*) pFmboRLBuf->getBufVA(0);
    rFefmData.geoDataRightToLeft[1] = (MUINT16*) pFmcoRLBuf->getBufVA(0);
    rFefmData.geoDataRightToLeft[2] = NULL;

    return MTRUE;
}

MVOID
N3DNode::
onFlush()
{
    MY_LOGD("+");
    DepthMapRequestPtr pRequest;
    while( mJobQueue.deque(pRequest) )
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



