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

// Local header file
#include "OCCNode.h"
#include "../DepthMapPipe_Common.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

// Logging header file
#define PIPE_CLASS_TAG "OCCNode"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using StereoHAL::StereoDistanceUtil;
OCCNode::
OCCNode(
   const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
{
    this->addWaitQueue(&mJobQueue);
    this->addWaitQueue(&mLDCReqIDQueue);
    this->addWaitQueue(&mMYSReqIDQueue);
}

OCCNode::
~OCCNode()
{
    MY_LOGD("[Destructor]");
}

MVOID
OCCNode::
cleanUp()
{
    VSDOF_LOGD("+");
    if(mpOCCHAL!=NULL)
    {
        delete mpOCCHAL;
        mpOCCHAL = NULL;
    }
    mJobQueue.clear();
    VSDOF_LOGD("-");
}

MBOOL
OCCNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
OCCNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
OCCNode::
onThreadStart()
{
    VSDOF_INIT_LOG("+");
    CAM_TRACE_NAME("OCCNode::onThreadStart");
    // OCC HAL initialization
    mpOCCHAL = OCC_HAL::createInstance();
    VSDOF_INIT_LOG("-");

    return MTRUE;
}

MBOOL
OCCNode::
onThreadStop()
{
    CAM_TRACE_NAME("OCCNode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
OCCNode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    MBOOL bRet = MTRUE;
    VSDOF_LOGD("+, DataID=%d reqId=%d", data, pRequest->getRequestNo());

    switch(data)
    {
        case P2A_TO_OCC_MY_S:
            mMYSReqIDQueue.enque(pRequest->getRequestNo());
            break;
        case N3D_TO_OCC_LDC:
            mLDCReqIDQueue.enque(pRequest->getRequestNo());
            break;
        case DPE_TO_OCC_MVSV_DMP_CFM:
            VSDOF_PRFLOG("+ : reqID=%d size=%d", pRequest->getRequestNo(), mJobQueue.size());
            mJobQueue.enque(pRequest);
            break;
        default:
            MY_LOGW("Unrecongnized DataID=%d", data);
            bRet = MFALSE;
            break;
    }

    TRACE_FUNC_EXIT();
    return bRet;
}

MBOOL
OCCNode::
onThreadLoop()
{
    MUINT32 iLDCReadyReqID, iMYSReadyReqID;
    DepthMapRequestPtr pRequest;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    // get request
    if( !mJobQueue.deque(pRequest) )
    {
        MY_LOGE("mJobQueue.deque() failed");
        return MFALSE;
    }
    // get LDC request id
    if( !mLDCReqIDQueue.deque(iLDCReadyReqID) )
    {
        MY_LOGE("mLDCReqIDQueue.deque() failed");
        return MFALSE;
    }
    // get MYS request id
    if(!mMYSReqIDQueue.deque(iMYSReadyReqID) )
    {
        MY_LOGE("mMYSReqIDQueue.deque() failed");
        return MFALSE;
    }

    MUINT32 iReqNo = pRequest->getRequestNo();

    if(iReqNo != iLDCReadyReqID || iReqNo != iMYSReadyReqID)
    {
        MY_LOGE("The deque request is not consistent. iReqNo=%d  iLDCReadyReqID=%d iMYSReadyReqID=%d", iReqNo, iLDCReadyReqID, iMYSReadyReqID);
        return MFALSE;
    }

    CAM_TRACE_NAME("OCCNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();
    VSDOF_LOGD("OCC threadloop start, reqID=%d", iReqNo);

    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    OCC_HAL_PARAMS occInputParams;
    OCC_HAL_OUTPUT occOutputParams;
    // prepare in/out params
    MBOOL bRet = prepareOCCParams(pRequest, occInputParams, occOutputParams);
    if(!bRet)
    {
        MY_LOGE("reqID=%d OCC ALGO stopped because of the enque parameter error.", iReqNo);
        goto lbExit;
    }

    VSDOF_PRFLOG("OCC ALGO start, reqID=%d", pRequest->getRequestNo());
    // timer
    pRequest->mTimer.startOCC();
    CAM_TRACE_BEGIN("OCCNode::OCCHALRun");
    bRet = mpOCCHAL->OCCHALRun(occInputParams, occOutputParams);
    CAM_TRACE_END();
    // timer
    pRequest->mTimer.stopOCC();
    VSDOF_PRFTIME_LOG("OCC ALGO end, reqID=%d, exec-time=%d msec",
                pRequest->getRequestNo(), pRequest->mTimer.getElapsedOCC());

    if(bRet)
    {
        if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
        {
            if(!this->addDepthInfoToStorage(pRequest))
            {
                MY_LOGE("reqID=%d, failed to add depth info.", iReqNo);
                goto lbExit;
            }
            // dump buffer
            DumpConfig config("BID_OCC_OUT_DEPTHMAP_INTO_QUEUE", NULL, MFALSE);
            handleDump(OCC_OUT_INTERNAL_DEPTHMAP, pRequest, &config);
            // notify queued flow type finished
            handleData(QUEUED_FLOW_DONE, pRequest);
        }
        else
        {
            if(!udpateDistanceIntoMeta(pRequest))
            {
                MY_LOGE("reqID=%d, Failed to update distance!", iReqNo);
                goto lbExit;
            }
            // config output to WMF node
            pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_OCC,  eDPETHMAP_PIPE_NODEID_WMF);
            pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_NOC,  eDPETHMAP_PIPE_NODEID_WMF);
            // config buffer for WMF input, capture: BID_OCC_OUT_DS_MVY, preview:BID_P2A_OUT_MY_S
            if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
                pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_DS_MVY,  eDPETHMAP_PIPE_NODEID_WMF);
            else
                pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_WMF);

            // pass data
            handleDataAndDump(OCC_TO_WMF_OMYSN, pRequest);
        }
    }
    else
    {
        MY_LOGE("OCC ALGO failed: reqID=%d", pRequest->getRequestNo());
lbExit:
        // if error occur in the queued-flow, skip this operation and call queue-done
        if(pRequest->isQueuedDepthRequest(mpPipeOption))
            handleData(QUEUED_FLOW_DONE, pRequest);
        else
            handleData(ERROR_OCCUR_NOTIFY, pRequest);
    }
    // launch onProcessDone
    pBufferHandler->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();

    return MTRUE;
}

MBOOL
OCCNode::
udpateDistanceIntoMeta(
    DepthMapRequestPtr pRequest
)
{
    MINT32 magicNum_Main1;
    MINT32 magicNum_Main2;
    MFLOAT fLensFactor  = 0;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // config magic number of Main1
    DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum_Main1)) {
        MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main1", pRequest->getRequestNo());
        return MFALSE;
    }
    // magic num of Main2
    DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum_Main2)) {
        MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main2", pRequest->getRequestNo());
        return MFALSE;
    }
    // get focal lens factor
    DepthMapBufferID outHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
    IMetadata* pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outHalBID);
    if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, fLensFactor))
    {
        MY_LOGE("reqID=%d, Cannot find MTK_STEREO_FEATURE_RESULT_DISTANCE meta!", pRequest->getRequestNo());
        return MFALSE;
    }
    // use no hole-filling NOC buffer
    IImageBuffer* pDepthBuffer = nullptr;
    if(pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_NOC, pDepthBuffer))
    {
        MFLOAT newDistance = mDistanceUtil.getDistance(pDepthBuffer, fLensFactor, magicNum_Main1, magicNum_Main2);
        trySetMetadata<MFLOAT>(pOutHalMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, newDistance);
    }
    return MTRUE;
}

MBOOL
OCCNode::
addDepthInfoToStorage(
    DepthMapRequestPtr pRequest
)
{
    DepthBufferInfo depthInfo;
    MBOOL bRet = pRequest->getBufferHandler()->getEnquedSmartBuffer(
                            getNodeId(), BID_OCC_INTERAL_DEPTHMAP, depthInfo.mpDepthBuffer);
    if(!bRet)
    {
        MY_LOGE("reqID=%d Cannot get the depthmap enqued smart buffer", pRequest->getRequestNo());
        return MFALSE;
    }
    // sw buffer --> flush
    depthInfo.mpDepthBuffer->mImageBuffer->syncCache(eCACHECTRL_FLUSH);

    // get convergence offset
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IMetadata* pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL_QUEUED);
    if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, depthInfo.mfConvOffset))
    {
        MY_LOGE("reqID=%d Failed to get MTK_CONVERGENCE_DEPTH_OFFSET!!");
        return MFALSE;
    }
    // config magic number of Main1
    DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, depthInfo.magicNumber)) {
        MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main1", pRequest->getRequestNo());
        return MFALSE;
    }
    // magic num of Main2
    MINT32 magicNum_Main2 = 0;
    DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
    IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);
    if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum_Main2)) {
        MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main2", pRequest->getRequestNo());
        return MFALSE;
    }
    // get focal lens factor
    MFLOAT fLensFactor  = 0;
    if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, fLensFactor))
    {
        MY_LOGE("reqID=%d, Cannot find MTK_STEREO_FEATURE_RESULT_DISTANCE meta!", pRequest->getRequestNo());
        return MFALSE;
    }
    // get distance
    depthInfo.mfDistance = mDistanceUtil.getDistance(depthInfo.mpDepthBuffer->mImageBuffer.get(), fLensFactor, depthInfo.magicNumber, magicNum_Main2);
    VSDOF_LOGD("reqID=%d distance measure =%f", pRequest->getRequestNo(), depthInfo.mfDistance);
    // push into storage
    mpDepthStorage->push_back(depthInfo);
    return MTRUE;
}

MBOOL
OCCNode::
prepareOCCParams(
    DepthMapRequestPtr& pRequest,
    OCC_HAL_PARAMS& rOCCParams,
    OCC_HAL_OUTPUT& rOutParams
)
{
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    MBOOL bRet = MTRUE;
    IImageBuffer *pImgBuf_MV_Y = nullptr, *pImgBuf_SV_Y = nullptr;
    IImageBuffer *pImgBuf_DMP_L=nullptr,  *pImgBuf_DMP_R=nullptr, *pImgBuf_LDC = nullptr;
    IImageBuffer *pImgBuf_CFM = nullptr;
    // input buffers
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MV_Y, pImgBuf_MV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_SV_Y, pImgBuf_SV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_DMP_L, pImgBuf_DMP_L);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_DMP_R, pImgBuf_DMP_R);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_LDC, pImgBuf_LDC);
    DepthMapBufferID cfmID = (0 == StereoSettingProvider::getSensorRelativePosition())
                             ? BID_DPE_OUT_CFM_L
                             : BID_DPE_OUT_CFM_R;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), cfmID, pImgBuf_CFM);

    if(!bRet)
    {
        MY_LOGE("Cannot get enque buffers!");
        return MFALSE;
    }
    // insert input buffer
    rOCCParams.eScenario = (pRequest->getRequestAttr().opState == eSTATE_CAPTURE) ? eSTEREO_SCENARIO_CAPTURE
                           : (pRequest->getRequestAttr().isEISOn) ? eSTEREO_SCENARIO_RECORD
                           : eSTEREO_SCENARIO_PREVIEW;
    rOCCParams.imageMain1 = pImgBuf_MV_Y;
    rOCCParams.imageMain2 = pImgBuf_SV_Y;
    rOCCParams.disparityLeftToRight = (MUINT16*) pImgBuf_DMP_L->getBufVA(0);
    rOCCParams.disparityRightToLEft = (MUINT16*) pImgBuf_DMP_R->getBufVA(0);
    rOCCParams.confidenceMap = pImgBuf_CFM;
    rOCCParams.requestNumber = pRequest->getRequestNo();
    // rOCCParams.ldcMain1 = pImgBuf_LDC;   //No need for this version

    // get convergence offset
    DepthMapBufferID outHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
    IMetadata* pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(), outHalBID);
    if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, rOCCParams.cOffset))
    {
        MY_LOGE("Failed to get MTK_CONVERGENCE_DEPTH_OFFSET!!");
        return MFALSE;
    }

    IImageBuffer *pOutBuf_OCC = nullptr;
    IImageBuffer *pOutBuf_NOC = nullptr;
    IImageBuffer *pOutBuf_DepthMap = nullptr;
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
    {
        // get output DEPTHMAP
        pOutBuf_DepthMap = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_INTERAL_DEPTHMAP);

        // depthMap
        rOutParams.depthMap = pOutBuf_DepthMap;
    }
    else
    {
        // get output OCC & NOC
        IImageBuffer *pOutBuf_OCC = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_OUT_OCC);
        IImageBuffer *pOutBuf_NOC = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_OUT_NOC);

        // OCC&NOC
        rOutParams.occMap = pOutBuf_OCC;
        rOutParams.nocMap = pOutBuf_NOC;
    }

    // DS_MVY
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        rOutParams.downScaledImg = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_OUT_DS_MVY);
    }

    debugOCCParams({pImgBuf_MV_Y, pImgBuf_SV_Y, pImgBuf_DMP_L, pImgBuf_DMP_R,
                    rOCCParams.imageMain1Bayer, pOutBuf_OCC, pOutBuf_NOC, pOutBuf_DepthMap});

    return MTRUE;
}

MVOID
OCCNode::
debugOCCParams(DebugBufParam param)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;
    #define OUTPUT_IMG_BUFF(imageBuf)\
        if(imageBuf!=NULL)\
        {\
            MY_LOGD("=======================:" # imageBuf);\
            MY_LOGD("imageBuff size=%dx%d", imageBuf->getImgSize().w, imageBuf->getImgSize().h);\
            MY_LOGD("imageBuff plane count=%d", imageBuf->getPlaneCount());\
            MY_LOGD("imageBuff format=%x", imageBuf->getImgFormat());\
            MY_LOGD("imageBuff getImgBitsPerPixel=%d", imageBuf->getImgBitsPerPixel());\
            MY_LOGD("=======================");\
        }\
        else\
            MY_LOGD("=======================:" # imageBuf " is NULL!!!!");\


    MY_LOGD("Input::");
    OUTPUT_IMG_BUFF(param.imgBuf_MV_Y);
    OUTPUT_IMG_BUFF(param.imgBuf_SV_Y);
    OUTPUT_IMG_BUFF(param.imgBuf_DMP_L);
    OUTPUT_IMG_BUFF(param.imgBuf_DMP_R);
    OUTPUT_IMG_BUFF(param.imageMain1Bayer);

    MY_LOGD("Output::");
    OUTPUT_IMG_BUFF(param.occMap);
    OUTPUT_IMG_BUFF(param.nocMap);
    OUTPUT_IMG_BUFF(param.depthMap);

    #undef OUTPUT_IMG_BUFF
}

MVOID
OCCNode::
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
