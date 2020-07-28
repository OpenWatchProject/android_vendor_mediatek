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
#include <DpBlitStream.h>
#include <vsdof/util/vsdof_util.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Module header file

// Local header file
#include "GFNode.h"
#include "../DepthMapPipeUtils.h"
#include "../bokeh_buffer_packer.h"
// Logging header file
#define PIPE_CLASS_TAG "GFNode"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_GF);
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe_DepthMap{

using namespace VSDOF::util;

//************************************************************************
//
//************************************************************************
GFNode::
GFNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, mDistanceUtil(E_DIST_SRC_DMW)
{
    this->addWaitQueue(&mJobQueue);
    this->addWaitQueue(&mJobQueue_Capture);
}

//************************************************************************
//
//************************************************************************
GFNode::
~GFNode()
{
    MY_LOGD("[Destructor]");
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MVOID
GFNode::
cleanUp()
{
    MY_LOGD("+");
    mJobQueue.clear();
    // release gf_hal
    if(mpGf_Hal != nullptr)
    {
        delete mpGf_Hal;
        mpGf_Hal = nullptr;
    }
    MY_LOGD("-");
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
onThreadStart()
{
    CAM_ULOGM_TAGLIFE("GFNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // create gf_hal for PV/VR
    mpGf_Hal = GF_HAL::createInstance(
                            eSTEREO_SCENARIO_PREVIEW,
                            StereoSettingProvider::isMTKDepthmapMode()
                            );
    if(!mpGf_Hal)
    {
        MY_LOGE("Create GF_HAL fail.");
        cleanUp();
    }

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
onThreadStop()
{
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    VSDOF_LOGD("reqID=%d + data=%d", pRequest->getRequestNo(), data);
    MBOOL ret = MFALSE;
    //
    switch(data)
    {
        case P2A_TO_GF_DMW_MYS:
        case WMF_TO_GF_DMW_MY_S:
            mJobQueue.enque(pRequest);
            ret = MTRUE;
            break;
        case WMF_TO_GF_OND:
            mJobQueue_Capture.enque(pRequest);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    //
    VSDOF_LOGD("-");
    return ret;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;;
    //
    if( !waitAnyQueue() )
    {
        return MFALSE;
    }
    // capture queue first
    if( !mJobQueue_Capture.deque(pRequest) )
    {
        if( !mJobQueue.deque(pRequest) )
        {
            return MFALSE;
        }
    }
    CAM_ULOGM_TAGLIFE("GFNode::onThreadLoop");

    VSDOF_LOGD("reqID=%d threadLoop", pRequest->getRequestNo());
    MBOOL bRet = MTRUE;
    if(!executeAlgo(pRequest))
    {
        MY_LOGE("reqID=%d, GF executeAlgo failed.", pRequest->getRequestNo());
        handleData(ERROR_OCCUR_NOTIFY, pRequest);
        bRet = MFALSE;
    }
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    //
    return bRet;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
packDepthWrapper(DepthMapRequestPtr pRequest)
{
    VSDOF_LOGD("+");
    using namespace StereoHAL;
    AutoProfileUtil profile(PIPE_CLASS_TAG, "packDepthWrapper");

    IImageBuffer *pImgBuf_OCC = nullptr;
    IImageBuffer *pImgBuf_NOC = nullptr;
    IImageBuffer *pImgBuf_DMW = nullptr;

    MBOOL bRet = MTRUE;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_OCC, pImgBuf_OCC);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_NOC, pImgBuf_NOC);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_WMF_OUT_DMW, pImgBuf_DMW);

    IImageBuffer* pImgBuf_DepthWrapper = pBufferHandler->requestBuffer(getNodeId(), BID_GF_OUT_DEPTH_WRAPPER);

    if(!bRet)
        return MFALSE;

    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    MINT32 jpegOrientation;
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!");
        return MFALSE;
    }

    PackBufferParam param;
    param.captureOrientation = jpegOrientation;
    param.depthMap = (MUINT8*)pImgBuf_DMW;
    param.occMap = (MUINT8*)pImgBuf_OCC;
    param.nocMap = (MUINT8*)pImgBuf_NOC;
    param.packedBuffer = (MUINT8*)pImgBuf_DepthWrapper;

    VSDOF_LOGD("jpegOrientation=%d", jpegOrientation);
    VSDOF_LOGD("pImgBuf_DMW=%dx%d", pImgBuf_DMW->getImgSize().w, pImgBuf_DMW->getImgSize().h);
    VSDOF_LOGD("pImgBuf_OCC=%dx%d", pImgBuf_OCC->getImgSize().w, pImgBuf_OCC->getImgSize().h);
    VSDOF_LOGD("pImgBuf_NOC=%dx%d", pImgBuf_NOC->getImgSize().w, pImgBuf_NOC->getImgSize().h);
    VSDOF_LOGD("pImgBuf_DepthWrapper=%dx%d", pImgBuf_DepthWrapper->getImgSize().w, pImgBuf_DepthWrapper->getImgSize().h);

    CaptureBufferPacker::packBuffer(param);

    VSDOF_LOGD("-");
    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
executeAlgo(
    DepthMapRequestPtr pRequest)
{
    MBOOL ret = MFALSE;
    VSDOF_LOGD("+ reqId=%d", pRequest->getRequestNo());

    // Normal pass
    if(!runNormalPass(pRequest))
    {
        MY_LOGE("GF NormalPass failed!");
        goto lbExit;
    }
    // handle
    if(pRequest->isQueuedDepthRequest(mpPipeOption)
        && mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
        handleData(QUEUED_FLOW_DONE, pRequest);
    ret = MTRUE;
    VSDOF_LOGD("- reqId=%d", pRequest->getRequestNo());
lbExit:
    return ret;
}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
runNormalPass(
    DepthMapRequestPtr pRequest)
{
    CAM_ULOGM_TAG_BEGIN("GFNode::runNormalPass");

    GF_HAL_IN_DATA sInData;
    GF_HAL_OUT_DATA sOutData;

    sInData.isCapture = false;
    MINT32 iReqIdx = pRequest->getRequestNo();

    if(!requireAlgoDataFromRequest(pRequest, sInData, sOutData))
    {
        MY_LOGE("get algo buffer fail, reqID=%d", iReqIdx);
        return MFALSE;
    }
    //
    debugGFParams(sInData, sOutData);
    //
    pRequest->mTimer.startGFNormal();
    //
    if(!mpGf_Hal->GFHALRun(sInData, sOutData))
    {
        MY_LOGE("GFHalRun fail, reqID=%d", iReqIdx);
        return MFALSE;
    }
    //
    pRequest->mTimer.stopGFNormal();

    VSDOF_PRFTIME_LOG("[NormalPass]gf algo processing time(%d ms) reqID=%d",
                                pRequest->mTimer.getElapsedGFNormal(), iReqIdx);
    MBOOL bRet = MTRUE;
    if(pRequest->isQueuedDepthRequest(mpPipeOption)
        && mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
    {
        DepthBufferInfo depthInfo;

        // get convergence offset
        DepthMapBufferID outHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
        IMetadata* pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outHalBID);
        if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, depthInfo.mfConvOffset))
        {
            MY_LOGE("Failed to get MTK_CONVERGENCE_DEPTH_OFFSET!!");
        }
        // config magic number
        DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
        IMetadata* pInHalMeta_Main1 = pRequest->getBufferHandler()->requestMetadata(getNodeId(), inHalBID_Main1);
        if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, depthInfo.magicNumber)) {
            MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main1", pRequest->getRequestNo());
        }
        // config distance(fb)
        bRet &= addDepthInfoToStorage(pRequest, depthInfo);

        // for mtk post-processed depth flow
        bRet &= pRequest->getBufferHandler()->getEnquedSmartBuffer(
                                getNodeId(), BID_GF_INTERNAL_DEPTHMAP, depthInfo.mpDepthBuffer);
        if(bRet)
        {
            depthInfo.mpDepthBuffer->mImageBuffer->syncCache(eCACHECTRL_FLUSH);
            depthInfo.miReqIdx = iReqIdx;
            mpDepthStorage->push_back(depthInfo);
        }
        else
            MY_LOGE("Cannot find the depthMap buffer!");
    }
    else
    {
        // flush DMBG
        IImageBuffer* pImgBuf = nullptr;
        if(pRequest->getBufferHandler()->getEnqueBuffer(getNodeId(), BID_GF_OUT_DMBG, pImgBuf))
        {
            pImgBuf->syncCache(eCACHECTRL_FLUSH);
        }
        // mark DMBG ready
        pRequest->setOutputBufferReady(BID_GF_OUT_DMBG);
        // notify DMBG done
        handleDataAndDump(GF_OUT_DMBG, pRequest);
    }

    CAM_ULOGM_TAG_END();
    return MTRUE;
}


//************************************************************************
//
//************************************************************************
MVOID
GFNode::
debugGFParams(
    const GF_HAL_IN_DATA& inData,
    const GF_HAL_OUT_DATA& outData
)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    MY_LOGD("Input GFParam: GF_HAL_IN_DATA");
    MY_LOGD("magicNumber=%d", inData.magicNumber);
    MY_LOGD("scenario=%d", inData.scenario);
    MY_LOGD("dofLevel=%d", inData.dofLevel);
    MY_LOGD("depthMap=%p", inData.depthMap);
    MY_LOGD("images.size()=%d", inData.images.size());
    for(ssize_t idx=0;idx<inData.images.size();++idx)
    {
        MY_LOGD("images[%d]=%p", idx, inData.images[idx]);
        if(inData.images[idx])
        {
            MY_LOGD("images[%d], image size=%dx%d", idx,
                inData.images[idx]->getImgSize().w, inData.images[idx]->getImgSize().h);
        }
    }
    MY_LOGD("convOffset=%f", inData.convOffset);
    MY_LOGD("focalLensFactor=%f", inData.focalLensFactor);
    if(outData.dmbg != nullptr)
        MY_LOGD("Output dmbg size =%dx%d", outData.dmbg->getImgSize().w
                                         , outData.dmbg->getImgSize().h);
    else
        MY_LOGD("Output dmbg null!");

    if(outData.depthMap != nullptr)
        MY_LOGD("Output depthMap size=%dx%d", outData.depthMap->getImgSize().w
                                         , outData.depthMap->getImgSize().h);
    else
        MY_LOGD("Output depthMap null!");


}

//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
requireInputMetaFromRequest(
    DepthMapRequestPtr pRequest,
    GF_HAL_IN_DATA& inData
)
{
    IMetadata *pInAppMeta  = nullptr, *pInHalMeta  = nullptr;
    IMetadata *pOutHalMeta = nullptr, *pOutAppMeta = nullptr;
    MINT32 iReqIdx = pRequest->getRequestNo();
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
    {
        DepthMapBufferID inAppMetaID  = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_APP);
        DepthMapBufferID inHalMetaID  = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
        DepthMapBufferID outHalMetaID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
        DepthMapBufferID outAppBID    = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_APP);
        pInAppMeta  = pBufferHandler->requestMetadata(getNodeId(), inAppMetaID);
        pInHalMeta  = pBufferHandler->requestMetadata(getNodeId(), inHalMetaID);
        pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(), outHalMetaID);
        pOutAppMeta = pBufferHandler->requestMetadata(getNodeId(), outAppBID);
    }
    else    // get online metadata buffer
    {
        pInAppMeta  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
        pInHalMeta  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
        pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL);
        pOutAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_APP);
    }
    // config request number
    inData.requestNumber = iReqIdx;
    // config DOF level
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DOF_LEVEL, inData.dofLevel))
    {
        MY_LOGE("reqID=%d Cannot find MTK_STEREO_FEATURE_DOF_LEVEL meta in AppMeta!", iReqIdx);
        return MFALSE;
    }
    // config touch info
    IMetadata::IEntry entry = pInAppMeta->entryFor(MTK_STEREO_FEATURE_TOUCH_POSITION);
    if( !entry.isEmpty() ) {
        inData.touchPosX = entry.itemAt(0, Type2Type<MINT32>());
        inData.touchPosY = entry.itemAt(1, Type2Type<MINT32>());
    }
    if(!checkDumpIndex(iReqIdx)) {
        inData.dumpHint = nullptr;
    } else {
        extract(&mDumpHint_Main1, pInHalMeta);
        inData.dumpHint = &mDumpHint_Main1;
    }
    MINT32 iSensorMode;
    // config scenario
    if(!tryGetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_SENSOR_MODE, iSensorMode))
    {
        MY_LOGE("reqID=%d Cannot find MTK_P1NODE_SENSOR_MODE meta in HalMeta!", iReqIdx);
        return MFALSE;
    }
    inData.scenario = getStereoSenario(iSensorMode);
    // config magic number
    if(!tryGetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, inData.magicNumber))
    {
        MY_LOGE("reqID=%d Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta in HalMeta!", iReqIdx);
        return MFALSE;
    }
    // config convergence offset
    if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, inData.convOffset))
    {
        MY_LOGE("reqID=%d Cannot find MTK_CONVERGENCE_DEPTH_OFFSET meta in outHalMeta!", iReqIdx);
        return MFALSE;
    }
    // config focalLensFactor
    if(!tryGetMetadata<MFLOAT>(pOutAppMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, inData.focalLensFactor))
    {
        MY_LOGW("reqID=%d, Failed to get MTK_STEREO_FEATURE_RESULT_DISTANCE!", pRequest->getRequestNo());
    }
    return MTRUE;
}


//************************************************************************
//
//************************************************************************
MBOOL
GFNode::
requireAlgoDataFromRequest(
    DepthMapRequestPtr pRequest,
    GF_HAL_IN_DATA& inData,
    GF_HAL_OUT_DATA& outData)
{
    sp<IImageBuffer> pImgBuf_DMBG = NULL, pImgBuf_DepthMap = NULL;

    MINT32 iReqIdx = pRequest->getRequestNo();
    VSDOF_LOGD("reqID=%d", pRequest->getRequestNo());

    // require input meta
    if(!requireInputMetaFromRequest(pRequest, inData))
        return MFALSE;

    //
    MBOOL bRet = MTRUE;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // config MYS/DMW buffer
    IImageBuffer *pImgMYS = nullptr, *pImgDMW = nullptr;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_MY_S, pImgMYS);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_WMF_OUT_DMW, pImgDMW);

    if(bRet)
    {   // MY_S
        inData.images.push_back(pImgMYS);
        // DMW : In queue flow, the depth map may be nullptr.
        if(pImgDMW == nullptr)
        {
            inData.depthMap = nullptr;
            if(!pRequest->isQueuedDepthRequest(mpPipeOption))
            {
                MY_LOGE("DMW buffer is null, please check!");
                return MFALSE;
            }
        }
        else
            inData.depthMap = pImgDMW;
    }
    else
    {
        MY_LOGE("Cannot get MYS/DMW buffers!!");
        return MFALSE;
    }

    // output
    pImgBuf_DMBG = pBufferHandler->requestBuffer(getNodeId(), BID_GF_OUT_DMBG);
    outData.dmbg = pImgBuf_DMBG.get();
    outData.depthMap = NULL;

    if(pRequest->isQueuedDepthRequest(mpPipeOption)
        && mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
        outData.depthMap = pBufferHandler->requestBuffer(getNodeId(), BID_GF_INTERNAL_DEPTHMAP);
    else if(pRequest->isRequestBuffer(BID_GF_OUT_DEPTH))
        outData.depthMap = pBufferHandler->requestBuffer(getNodeId(), BID_GF_OUT_DEPTH);

    return MTRUE;
}

MBOOL
GFNode::
addDepthInfoToStorage(
    DepthMapRequestPtr pRequest,
    DepthBufferInfo &depthInfo
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
    DepthMapBufferID outAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_APP);
    IMetadata* pOutAppMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outAppBID);
    if(!tryGetMetadata<MFLOAT>(pOutAppMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, fLensFactor))
    {
        MY_LOGE("reqID=%d, Cannot find MTK_STEREO_FEATURE_RESULT_DISTANCE meta!", pRequest->getRequestNo());
        return MFALSE;
    }
    // use no hole-filling NOC buffer
    IImageBuffer* pDepthBuffer = nullptr;
    DepthMapBufferID outHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_HAL);
    IMetadata* pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outHalBID);
    if(pBufferHandler->getEnqueBuffer(getNodeId(), BID_WMF_OUT_DMW, pDepthBuffer))
    {
        depthInfo.mfDistance = mDistanceUtil.getDistance(pDepthBuffer, fLensFactor, magicNum_Main1, magicNum_Main2);
        VSDOF_LOGD("reqID=%d distance measure (WMF) =%.2f", pRequest->getRequestNo(), depthInfo.mfDistance);
    }
    // pDepthBuffer = nullptr;
    // if(pBufferHandler->getEnqueBuffer(getNodeId(), BID_GF_INTERNAL_DEPTHMAP, pDepthBuffer))
    // {
    //     depthInfo.mfDistance = mDistanceUtil.getDistance(pDepthBuffer, fLensFactor, magicNum_Main1, magicNum_Main2);
    //     VSDOF_LOGD("reqID=%d distance measure (GF) =%.2f", pRequest->getRequestNo(), depthInfo.mfDistance);
    // }
    return MTRUE;
}

MVOID
GFNode::
onFlush()
{
    MY_LOGD("+ extDep=%d", this->getExtThreadDependency());
    DepthMapRequestPtr pRequest;
    while( mJobQueue.deque(pRequest) )
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
	while( mJobQueue_Capture.deque(pRequest) )
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
    DepthMapPipeNode::onFlush();
    MY_LOGD("-");
}

};
};
};
