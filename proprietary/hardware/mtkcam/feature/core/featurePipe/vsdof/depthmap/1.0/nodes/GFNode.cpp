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
    CAM_TRACE_NAME("GFNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // create gf_hal for PV/VR
    mpGf_Hal = GF_HAL::createInstance(eSTEREO_SCENARIO_PREVIEW);
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
    CAM_TRACE_NAME("GFNode::onThreadLoop");

    VSDOF_LOGD("reqID=%d threadLoop", pRequest->getRequestNo());
    MBOOL bRet = MTRUE;

    // run algo
    if(!executeAlgo(pRequest))
    {
        MY_LOGE("reqID=%d, GF executeAlgo failed.", pRequest->getRequestNo());
        handleData(ERROR_OCCUR_NOTIFY, pRequest);
        bRet = MFALSE;
    }

    if(bRet && pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        bRet = packDepthWrapper(pRequest);
        if(!bRet)
        {
            MY_LOGE("reqID=%d, packDepthWrapper failed.", pRequest->getRequestNo());
            handleData(ERROR_OCCUR_NOTIFY, pRequest);
            bRet = MFALSE;
        }
        else
        {
            // mark Depth wrapper ready
            pRequest->setOutputBufferReady(BID_GF_OUT_DEPTH_WRAPPER);
            // notify Depth wrapper done
            handleDataAndDump(GF_OUT_DEPTH_WRAPPER, pRequest);
        }
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
    if(pInAppMeta == nullptr || !tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("Cannot find MTK_JPEG_ORIENTATION meta!pInAppMeta=%x", pInAppMeta);
        return MFALSE;
    }

    PackBufferParam param;
    param.captureOrientation = jpegOrientation;
    param.depthMap = (MUINT8*)pImgBuf_DMW->getBufVA(0);
    param.occMap = (MUINT8*)pImgBuf_OCC->getBufVA(0);
    param.nocMap = (MUINT8*)pImgBuf_NOC->getBufVA(0);
    param.packedBuffer = (MUINT8*)pImgBuf_DepthWrapper->getBufVA(0);

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

    // Normal pass - preview and capture all needs DMBG
    if(!runNormalPass(pRequest))
    {
        MY_LOGE("GF NormalPass failed!");
        goto lbExit;
    }
    //
    ret = MTRUE;
    VSDOF_LOGD("- reqId=%d", pRequest->getRequestNo());
    CAM_TRACE_END();
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
    CAM_TRACE_BEGIN("GFNode::runNormalPass");

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
    CAM_TRACE_END();
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
    MY_LOGD("depthMap=%x", inData.depthMap);
    MY_LOGD("images.size()=%d", inData.images.size());
    for(ssize_t idx=0;idx<inData.images.size();++idx)
    {
        MY_LOGD("images[%d]=%x", idx, inData.images[idx]);
        if(inData.images[idx])
        {
            MY_LOGD("images[%d], image size=%dx%d", idx,
                inData.images[idx]->getImgSize().w, inData.images[idx]->getImgSize().h);
        }
    }

    MY_LOGD("convOffset=%f", inData.convOffset);

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
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MINT32 iReqIdx = pRequest->getRequestNo();
    // InAppMeta
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
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

    // InHalMeta
    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
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
    IMetadata* pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), BID_META_OUT_HAL);
    // config convergence offset
    if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, inData.convOffset))
    {
        MY_LOGE("reqID=%d Cannot find MTK_CONVERGENCE_DEPTH_OFFSET meta in outHalMeta!", iReqIdx);
        return MFALSE;
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
            inData.depthMap = (MUINT8*)pImgDMW->getBufVA(0);
    }
    else
    {
        MY_LOGE("Cannot get MYS/DMW buffers!!");
        return MFALSE;
    }

    {
        // config output buffers: DMBG
        pImgBuf_DMBG = pBufferHandler->requestBuffer(getNodeId(), BID_GF_OUT_DMBG);
        outData.dmbg = (MUINT8*) pImgBuf_DMBG->getBufVA(0);
        outData.depthMap = NULL;
    }
    return MTRUE;
}

MVOID
GFNode::
onFlush()
{
    MY_LOGD("+");
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
