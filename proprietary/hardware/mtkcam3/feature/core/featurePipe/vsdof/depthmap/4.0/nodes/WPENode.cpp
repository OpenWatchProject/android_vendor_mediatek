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

// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "WPENode.h"
#include "../DepthMapPipe_Common.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

// Logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "WPENode"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_WPE);

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;

#define WPENODE_USERNAME "VSDOF_WPE"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
WPENode::
WPENode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, miSensorIdx_Main1(config.mpSetting->miSensorIdx_Main1)
{
    this->addWaitQueue(&mJobQueue);
}

WPENode::
~WPENode()
{
    MY_LOGD("[Destructor]");
}

MVOID
WPENode::
cleanUp()
{
    MY_LOGD("+");
    if(mpINormalStream != NULL)
    {
        mpINormalStream->uninit(WPENODE_USERNAME);
        mpINormalStream->destroyInstance();
        mpINormalStream = NULL;
    }

    MY_LOGD("-");
}

MBOOL
WPENode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WPENode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WPENode::
onThreadStart()
{
    CAM_ULOGM_TAGLIFE("WPENode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // Create NormalStream, 0xFFF for WPE
    VSDOF_LOGD("NormalStream create instance");
    CAM_ULOGM_TAG_BEGIN("WPENode::NormalStream::createInstance+init");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);

    if (mpINormalStream == NULL ||
        !mpINormalStream->init(WPENODE_USERNAME, NSCam::NSIoPipe::EStreamPipeID_WarpEG))
    {
        MY_LOGE("mpINormalStream create instance for WPE Node failed!");
        cleanUp();
        return MFALSE;
    }
    CAM_ULOGM_TAG_END();
    //
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WPENode::
onThreadStop()
{
    CAM_ULOGM_TAGLIFE("WPENode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WPENode::
onData(DataID data, DepthMapRequestPtr &request)
{
  MBOOL ret = MTRUE;
  VSDOF_LOGD("+ : reqID=%d", request->getRequestNo());
  CAM_ULOGM_TAGLIFE("WPENode::onData");

  switch(data)
  {
  case N3D_TO_WPE_RECT2_WARPMTX:
    mJobQueue.enque(request);
    break;
  default:
    MY_LOGW("Un-recognized data ID, id=%d reqID=%d", data, request->getRequestNo());
    ret = MFALSE;
    break;
  }

  VSDOF_LOGD("-");
  return ret;
}

MBOOL
WPENode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mJobQueue.deque(pRequest) )
    {
        MY_LOGE("mJobQueue.deque() failed");
        return MFALSE;
    }

    CAM_ULOGM_TAGLIFE("WPENode::onThreadLoop");
    pRequest->mTimer.startWPE();
    // mark on-going-pRequest start
    this->incExtThreadDependency();
    const EffectRequestAttrs& attr = pRequest->getRequestAttr();
    VSDOF_PRFLOG("threadLoop start, reqID=%d eState=%d needFEFM=%d",
                    pRequest->getRequestNo(), attr.opState, attr.needFEFM);
    //
    MBOOL bRet = MFALSE;
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(pRequest, this);
    QParams enqueParams;
    if(!prepareWPEEnqueParams(pRequest, enqueParams, pCookieIns))
        goto lbExit;
    // callback
    enqueParams.mpfnCallback = onWPECallback;
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // enque
    CAM_ULOGM_TAG_BEGIN("WPENode::NormalStream::enque");
    VSDOF_LOGD("WPE enque start! reqID=%d", pRequest->getRequestNo());
    pRequest->mTimer.startWPEDrv();
    bRet = mpINormalStream->enque(enqueParams);
    CAM_ULOGM_TAG_END();
    VSDOF_LOGD("WPE enque end! reqID=%d, result=%d", pRequest->getRequestNo(), bRet);
    if(!bRet)
    {
        MY_LOGE("WPE enque failed! reqID=%d", pRequest->getRequestNo());
        goto lbExit;
    }
    return MTRUE;

lbExit:
    delete pCookieIns;
    // mark on-going-request end
    this->decExtThreadDependency();
    return MFALSE;

}

MBOOL
WPENode::
prepareWPEEnqueParams(
    DepthMapRequestPtr& pRequest,
    QParams& rQParam,
    EnqueCookieContainer* pEnqueContainer
)
{
    VSDOF_LOGD("prepareWPEEnqueParams reqID=%d", pRequest->getRequestNo());
    //
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* pImgBuf_RectIn2 = nullptr;
    IImageBuffer* pImgBuf_WarpMtx_Main2_X = nullptr;
    IImageBuffer* pImgBuf_WarpMtx_Main2_Y = nullptr;
    //
    if(!pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, pImgBuf_RectIn2) ||
       !pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_X, pImgBuf_WarpMtx_Main2_X) ||
       !pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_WARPMTX_MAIN2_Y, pImgBuf_WarpMtx_Main2_Y))
    {
        return MFALSE;
    }
    //warp sub image
    IImageBuffer* pImgBuf_SV_Y = pBufferHandler->requestBuffer(getNodeId(), BID_WPE_OUT_SV_Y);
    //
    MBOOL bRet = MTRUE;
    bRet &= prepareImgWarpParams(pImgBuf_RectIn2, pImgBuf_WarpMtx_Main2_X, pImgBuf_WarpMtx_Main2_Y, pImgBuf_SV_Y, rQParam, pEnqueContainer);
    // only preview request needs to warp mask
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
        // refine warp input mask
        IImageBuffer* pImgBuf_DefaultMask2 = pBufferHandler->requestWorkingBuffer(getNodeId(), BID_WPE_IN_MASK_S);
        ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().opState == eSTATE_CAPTURE) ?
                                        eSTEREO_SCENARIO_CAPTURE : eSTEREO_SCENARIO_PREVIEW;
        N3D_HAL::refineMask(scenario, pImgBuf_DefaultMask2, true);
        pImgBuf_DefaultMask2->syncCache(eCACHECTRL_FLUSH);

        IImageBuffer* pImgBuf_MASK_S = pBufferHandler->requestBuffer(getNodeId(), BID_WPE_OUT_MASK_S);
        bRet &= prepareImgWarpParams(pImgBuf_DefaultMask2, pImgBuf_WarpMtx_Main2_X, pImgBuf_WarpMtx_Main2_Y, pImgBuf_MASK_S, rQParam, pEnqueContainer);
    }

    if(!bRet)
    {
        MY_LOGE("Failed to prepare the WPE param data!");
    }
    return bRet;
}

MBOOL
WPENode::
prepareImgWarpParams(
    IImageBuffer* pImgBuffer,
    IImageBuffer* pWarpMtx_X,
    IImageBuffer* pWarpMtx_Y,
    IImageBuffer* pOutputBuffer,
    QParams& rQParam,
    EnqueCookieContainer* pEnqueContainer
)
{
    if(pImgBuffer == nullptr ||
        pWarpMtx_X == nullptr ||
        pWarpMtx_Y == nullptr ||
        pOutputBuffer == nullptr)
    {
        MY_LOGE("Buffer is null!!!pImgBuffer=%x, pWarpMtx_X=%x, pWarpMtx_Y=%x, pOutputBuffer=%d",
                pImgBuffer, pWarpMtx_X, pWarpMtx_Y, pOutputBuffer);
        return MFALSE;
    }
    VSDOF_LOGD("pImgBuffer=%dx%d pWarpMtx_X=%dx%d pWarpMtx_Y=%dx%d pOutputBuffer=%dx%d",
                pImgBuffer->getImgSize().w, pImgBuffer->getImgSize().h,
                pWarpMtx_X->getImgSize().w, pWarpMtx_X->getImgSize().h,
                pWarpMtx_Y->getImgSize().w, pWarpMtx_Y->getImgSize().h,
                pOutputBuffer->getImgSize().w, pOutputBuffer->getImgSize().h);
    FrameParams frameParam;
    // secure flag
    frameParam.mSecureFra = MFALSE;
    // prepare input
    Input input;
    input.mPortID = PORT_WPEI;
    input.mBuffer = pImgBuffer;
    if(MFALSE)
        input.mSecureTag = EDIPSecure_SECURE;
    frameParam.mvIn.push_back(input);
    // prepare output
    Output output;
    output.mPortID = PORT_WPEO;
    output.mBuffer = pOutputBuffer;
    if(MFALSE)
        output.mSecureTag = EDIPSecure_SECURE;
    frameParam.mvOut.push_back(output);
    // prepare WPEParam
    WPEQParams* pWpeParam = new WPEQParams();
    prepareWPEQParam(pImgBuffer, pWarpMtx_X, pWarpMtx_Y, *pWpeParam);
    pEnqueContainer->mvData.push_back((void*)pWpeParam);
    // extra param
    ExtraParam exParams;
    exParams.CmdIdx = EPIPE_WPE_INFO_CMD;
    exParams.moduleStruct = (void*) pWpeParam;
    frameParam.mvExtraParam.push_back(exParams);
    // insert frame to QParam
    rQParam.mvFrameParams.push_back(frameParam);

    return MTRUE;
}

MVOID
WPENode::
prepareWPEQParam(
    IImageBuffer* pInputImg,
    IImageBuffer* pWarpMatrix_X,
    IImageBuffer* pWarpMatrix_Y,
    WPEQParams& rWPEQParam
)
{
    // standalone mode
    rWPEQParam.wpe_mode = WPE_MODE_WPEO;
    // no z plane
    rWPEQParam.vgen_hmg_mode = 0;
    // plane 0, x
    rWPEQParam.warp_veci_info.width = pWarpMatrix_X->getImgSize().w;
    rWPEQParam.warp_veci_info.height = pWarpMatrix_X->getImgSize().h;
    rWPEQParam.warp_veci_info.stride = pWarpMatrix_X->getBufStridesInBytes(0);
    rWPEQParam.warp_veci_info.virtAddr = pWarpMatrix_X->getBufVA(0);
    rWPEQParam.warp_veci_info.phyAddr = pWarpMatrix_X->getBufPA(0);
    rWPEQParam.warp_veci_info.bus_size = WPE_BUS_SIZE_32_BITS;
    rWPEQParam.warp_veci_info.addr_offset = 0;
    rWPEQParam.warp_veci_info.veci_v_flip_en = 0;
    // plane 1, y
    rWPEQParam.warp_vec2i_info.width = pWarpMatrix_Y->getImgSize().w;
    rWPEQParam.warp_vec2i_info.height = pWarpMatrix_Y->getImgSize().h;
    rWPEQParam.warp_vec2i_info.stride = pWarpMatrix_Y->getBufStridesInBytes(0);
    rWPEQParam.warp_vec2i_info.virtAddr = pWarpMatrix_Y->getBufVA(0);
    rWPEQParam.warp_vec2i_info.phyAddr = pWarpMatrix_Y->getBufPA(0);
    rWPEQParam.warp_vec2i_info.bus_size = WPE_BUS_SIZE_32_BITS;
    rWPEQParam.warp_vec2i_info.addr_offset = 0;
    rWPEQParam.warp_vec2i_info.veci_v_flip_en = 0;
    //
    rWPEQParam.wpecropinfo.x_start_point = 0;
    rWPEQParam.wpecropinfo.y_start_point = 0;
    rWPEQParam.wpecropinfo.x_end_point = pInputImg->getImgSize().w-1;
    rWPEQParam.wpecropinfo.y_end_point = pInputImg->getImgSize().h-1;
    // vgen crop
    MCrpRsInfo vgenCropInfo;
    vgenCropInfo.mCropRect = MCropRect(MPoint(0,0), pWarpMatrix_X->getImgSize());
    rWPEQParam.mwVgenCropInfo.push_back(vgenCropInfo);
    // tuning parameters
    rWPEQParam.tbl_sel_v = 24;
    rWPEQParam.tbl_sel_h = 24;
    //
    auto logWPEParam = [](char* prefix, WarpMatrixInfo& veciInfo)
    {
        VSDOF_LOGD2("%s: width=%d height=%d stride=%d virtAddr=%x phyAddr=%x bus_size=%d offset=%d flip=%d",
                    prefix, veciInfo.width, veciInfo.height, veciInfo.stride, veciInfo.virtAddr,
                    veciInfo.phyAddr, veciInfo.bus_size, veciInfo.addr_offset, veciInfo.veci_v_flip_en);
    };

    logWPEParam("warp_veci_info", rWPEQParam.warp_veci_info);
    logWPEParam("warp_vec2i_info", rWPEQParam.warp_vec2i_info);
}

MVOID
WPENode::
onWPECallback(QParams& rParams)
{
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    WPENode* pWPENode = (WPENode*) (pEnqueData->mpNode);
    pWPENode->handleWPEDone(rParams, pEnqueData);
}

MVOID
WPENode::
handleWPEDone(QParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_ULOGM_TAGLIFE("WPENode::handleP2Done");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    // stop timer
    pRequest->mTimer.stopWPEDrv();
    DumpConfig config;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    VSDOF_PRFTIME_LOG("+ :reqID=%d , WPE exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedWPEDrv());
    //
    this->configureToNext(pRequest);
lbExit:
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    for(void* data : pEnqueCookie->mvData )
    {
        WPEQParams* pData = reinterpret_cast<WPEQParams*>(data);
        delete pData;
    }
    delete pEnqueCookie;
    VSDOF_LOGD("- :reqID=%d", pRequest->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();
    // stop overall timer
    pRequest->mTimer.stopWPE();
}

MBOOL
WPENode::
configureToNext(DepthMapRequestPtr pRequest)
{
    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    //
    IImageBuffer *pImgBuf_SV_Y = nullptr;
    if(pBufferHandler->getEnqueBuffer(getNodeId(), BID_WPE_OUT_SV_Y, pImgBuf_SV_Y))
        pImgBuf_SV_Y->syncCache(eCACHECTRL_INVALID);
    // set output buffer
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        pBufferHandler->configOutBuffer(getNodeId(), BID_WPE_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_DLDEPTH);
        this->handleDataAndDump(WPE_TO_DLDEPTH_MV_SV, pRequest);
    }
    else
    {
        IImageBuffer *pImgBuf_MASK_S = nullptr;
        if(pBufferHandler->getEnqueBuffer(getNodeId(), BID_WPE_OUT_MASK_S, pImgBuf_MASK_S))
            pImgBuf_MASK_S->syncCache(eCACHECTRL_INVALID);
        // refine mask
        ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().opState == eSTATE_CAPTURE) ?
                                        eSTEREO_SCENARIO_CAPTURE : eSTEREO_SCENARIO_PREVIEW;
        N3D_HAL::refineMask(scenario, pImgBuf_MASK_S, false);

        pBufferHandler->configOutBuffer(getNodeId(), BID_WPE_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_DPE);
        pBufferHandler->configOutBuffer(getNodeId(), BID_WPE_OUT_MASK_S, eDPETHMAP_PIPE_NODEID_DPE);
        this->handleDataAndDump(WPE_TO_DPE_WARP_IMG, pRequest);
    }
    VSDOF_LOGD("-, reqID=%d", pRequest->getRequestNo());
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
WPENode::
onFlush()
{
    MY_LOGD("+ extDep=%d", this->getExtThreadDependency());
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

