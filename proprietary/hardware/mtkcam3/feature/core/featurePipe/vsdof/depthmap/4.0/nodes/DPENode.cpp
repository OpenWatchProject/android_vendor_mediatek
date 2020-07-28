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
#include <camera_custom_stereo.h>
// mtkcam global header file
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "DPENode.h"
#include "../DepthMapPipe_Common.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
// Logging header
#define PIPE_CLASS_TAG "DPENode"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_DPE);


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {
#define DPE_USER_NAME "DEPTHPIP_DPE"

DPENode::
DPENode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, mbFirst(MTRUE)
{
    this->addWaitQueue(&mJobQueue);
    this->addWaitQueue(&mJobQueue_MYS);
    mbIsWaitDepth = ::property_get_bool("vendor.depthmap.pipe.enableWaitDepth", 0);
}
/*******************************************************************************
 *
 ********************************************************************************/
DPENode::
~DPENode()
{
    MY_LOGD("[Destructor]");
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
cleanUp()
{
    VSDOF_LOGD("+");
    //
    if(mpDPEStream != nullptr)
    {
        mpDPEStream->uninit();
        mpDPEStream->destroyInstance(DPE_USER_NAME);
        mpDPEStream = nullptr;
    }
    mJobQueue.clear();
    mbFirst = MTRUE;
    mLastDV_LR = nullptr;
    //
    VSDOF_LOGD("-");
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
onThreadStart()
{
    VSDOF_INIT_LOG("+");
    CAM_ULOGM_TAGLIFE("DPENode::onThreadStart");
    // init DPEStream
    mpDPEStream = IEgnStream<DPEConfig>::createInstance(DPE_USER_NAME);
    if(mpDPEStream == NULL || !mpDPEStream->init())
    {
        MY_LOGE("DPE Stream create instance failed!");
        return MFALSE;
    }
    VSDOF_INIT_LOG("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
onThreadStop()
{
    VSDOF_LOGD("+");
    CAM_ULOGM_TAGLIFE("DPENode::onThreadStop");

    cleanUp();
    VSDOF_LOGD("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
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
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    MBOOL ret = MTRUE;
    VSDOF_LOGD("+ : reqId=%d id=%s", pRequest->getRequestNo(), ID2Name(data));

    switch(data)
    {
        case WPE_TO_DPE_WARP_IMG:
            mJobQueue.enque(pRequest);
            break;
        case P2A_TO_DPE_MY_S:
            mJobQueue_MYS.enque(pRequest);
            break;
        default:
            MY_LOGW("Unrecongnized DataID=%d", data);
            ret = MFALSE;
            break;
    }

    VSDOF_LOGD("-");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;
    DepthMapRequestPtr pRequest_MYS;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mJobQueue.deque(pRequest) )
    {
        MY_LOGE("mJobQueue.deque() failed");
        return MFALSE;
    }
    if( !mJobQueue_MYS.deque(pRequest_MYS) )
    {
        MY_LOGE("mJobQueue.deque() failed");
        return MFALSE;
    }
    //
    if(pRequest->getRequestNo() != pRequest_MYS->getRequestNo())
    {
        MY_LOGE("Request number not consistent! reqID=%d   MYS reqID=%d",
                pRequest->getRequestNo(), pRequest_MYS->getRequestNo());
        return MFALSE;
    }
    // timer
    pRequest->mTimer.startDPE();
    // mark on-going-request start
    this->incExtThreadDependency();
    MUINT32 iReqIdx = pRequest->getRequestNo();

    VSDOF_PRFLOG("threadLoop start, reqID=%d", iReqIdx);
    CAM_ULOGM_TAGLIFE("DPENode::onThreadLoop");

    EGNParams<DPEConfig> enqueParams;
    enqueParams.mpEngineID = eDPE;
    enqueParams.mpfnCallback = onDPEEnqueDone;

    MBOOL bRet = MFALSE;
    // prepare the enque configs
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
        bRet = prepareDPEEnqueConfig_PVVR(pRequest, enqueParams);
    }
    if(!bRet)
    {
        MY_LOGE("Failed to prepare DPE enque paramters! isCap: %d",
                pRequest->getRequestAttr().opState != eSTATE_NORMAL);
        return MFALSE;
    }
    // enque cookie instance
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(pRequest, this);
    enqueParams.mpCookie = (void*) pCookieIns;
    // timer
    pRequest->mTimer.startDPEDrv();
    CAM_ULOGM_TAG_BEGIN("DPENODE::DPEStream::enque");
    bRet = mpDPEStream->EGNenque(enqueParams);
    VSDOF_LOGD("DVE Enque, reqID=%d", iReqIdx);
    CAM_ULOGM_TAG_END();
    if(!bRet)
    {
        MY_LOGE("DPE enque failed!!");
        goto lbExit;
    }
    return MTRUE;

lbExit:
    delete pCookieIns;
    // mark on-going-request end
    this->decExtThreadDependency();
    return MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
debugDPEConfig(DPEConfig& config)
{
    if(DepthPipeLoggingSetup::mbDebugLog<2)
        return;

    #define DEBUG_BUFFER_SETUP(buf) \
            MY_LOGD("DPE buf:" # buf);\
            if(buf != nullptr){\
                MY_LOGD("Image buffer size=%dx%d:", buf->getImgSize().w, buf->getImgSize().h);\
                MY_LOGD("Image buffer format=%x planeCount=%d", buf->getImgFormat(), buf->getPlaneCount());\
                for(int i=0;i<buf->getPlaneCount();++i)\
                {\
                    MY_LOGD("bufferSize in bytes of plane(%d): %d", i, buf->getBufSizeInBytes(i));\
                }}\
            else\
                MY_LOGD("null buffer: " # buf);

    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_SrcImg_Y_L);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_SrcImg_Y_R);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_SrcImg_C);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_ValidMap_L);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_ValidMap_R);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_DV);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_DV);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_CONF);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_OCC);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_OCC_Ext);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_OCC);
    DEBUG_BUFFER_SETUP(config.Dpe_InBuf_OCC_Ext);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_CRM);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_ASF_RD);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_ASF_RD_Ext);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_ASF_HF);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_ASF_HF_Ext);
    DEBUG_BUFFER_SETUP(config.Dpe_OutBuf_WMF_FILT);

    #undef DEBUG_BUFFER_SETUP
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
onDPEEnqueDone(EGNParams<DPEConfig>& rParams)
{
    EnqueCookieContainer* pEnqueCookie = reinterpret_cast<EnqueCookieContainer*>(rParams.mpCookie);
    DPENode* pDPENode = reinterpret_cast<DPENode*>(pEnqueCookie->mpNode);
    pDPENode->handleDPEEnqueDone(rParams, pEnqueCookie);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
handleDPEEnqueDone(
    EGNParams<DPEConfig>& rParams,
    EnqueCookieContainer* pEnqueCookie
)
{
    CAM_ULOGM_TAGLIFE("DPENode::handleDPEEnqueDone");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    // stop timer
    pRequest->mTimer.stopDPEDrv();
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* pImgBuf_DMW = nullptr;
    MBOOL bRet = MFALSE;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    VSDOF_PRFTIME_LOG("+, reqID=%d, DPE exec-time=%d msec",
            pRequest->getRequestNo(), pRequest->mTimer.getElapsedDPEDrv());

    // get the output buffers and invalid
    bRet = pBufferHandler->getEnqueBuffer(getNodeId(), BID_WMF_OUT_DMW, pImgBuf_DMW);
    if(!bRet)
    {
        MY_LOGE("Failed to get DPE output buffers");
        goto lbExit;
    }
    // invalidate
    pImgBuf_DMW->syncCache(eCACHECTRL_INVALID);
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF)
    {
        // prepare input for GF
        pBufferHandler->configOutBuffer(getNodeId(), BID_WMF_OUT_DMW, eDPETHMAP_PIPE_NODEID_GF);
        pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_GF);
        pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_CFM_M, eDPETHMAP_PIPE_NODEID_GF);
        // post to GF node
        handleDataAndDump(DPE_TO_GF_DMW_N_DEPTH, pRequest);
    }
    else
    {
        // push NOC into storage
        DepthBufferInfo depthInfo;
        // get internal dmbg buffer
        MBOOL bRet = pRequest->getBufferHandler()->getEnquedSmartBuffer(
                                    getNodeId(), BID_OCC_OUT_NOC_M, depthInfo.mpDepthBuffer);
        if(bRet)
        {
            depthInfo.mpDepthBuffer->mImageBuffer->syncCache(eCACHECTRL_INVALID);
            depthInfo.miReqIdx = pRequest->getRequestNo();
            if(mbIsWaitDepth)
                mpDepthStorage->push_back(depthInfo);
            else
                mpDepthStorage->setStoredData(depthInfo);
            // dump
            handleDump(DPE_OUT_INTERNAL_DEPTH, pRequest);
            // notify queue flow done
            if(pRequest->isQueuedDepthRequest(mpPipeOption))
                handleData(QUEUED_FLOW_DONE, pRequest);
        }
        else
            MY_LOGE("Cannot find NOC buffer to store into storage.");
    }

lbExit:
    // launch onProcessDone
    pBufferHandler->onProcessDone(getNodeId());
    delete pEnqueCookie;
    // mark on-going-request end
    this->decExtThreadDependency();
    // stop timer
    pRequest->mTimer.stopDPE();

}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
prepareDPEEnqueConfig_PVVR(
    DepthMapRequestPtr pRequest,
    EGNParams<DPEConfig>& rEnqParam
)
{
    VSDOF_LOGD("+");
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // DPE enque config
    DPEConfig dpeConfig;
    // query tuning provider
    StereoTuningProvider::getDPETuningInfo(&dpeConfig, eSTEREO_SCENARIO_PREVIEW);
    // not 16bit mode for vsdof
    dpeConfig.Dpe_is16BitMode = 0;
    // insert the dynamic data
    IImageBuffer *pImgBuf_MV_Y = nullptr, *pImgBuf_SV_Y = nullptr;
    IImageBuffer *pImgBuf_MASK_M = nullptr , *pImgBuf_MASK_S = nullptr;
    IImageBuffer *pImgBuf_MY_S = nullptr;
    // input buffer
    MBOOL bRet = pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MV_Y, pImgBuf_MV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_WPE_OUT_SV_Y, pImgBuf_SV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MASK_M, pImgBuf_MASK_M);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_WPE_OUT_MASK_S, pImgBuf_MASK_S);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_MY_S, pImgBuf_MY_S);
    if(!bRet)
        return MFALSE;
    pImgBuf_SV_Y->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MV_Y->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MASK_M->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MASK_S->syncCache(eCACHECTRL_FLUSH);
    // check main eye loc
    if(STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())
    {
        // Main1/2 is L/R
        dpeConfig.Dpe_InBuf_SrcImg_Y_L = pImgBuf_MV_Y;
        dpeConfig.Dpe_InBuf_SrcImg_Y_R = pImgBuf_SV_Y;
        dpeConfig.Dpe_InBuf_ValidMap_L = pImgBuf_MASK_M;
        dpeConfig.Dpe_InBuf_ValidMap_R = pImgBuf_MASK_S;
    }
    else
    {
        // Main1/2 is R/L
        dpeConfig.Dpe_InBuf_SrcImg_Y_L = pImgBuf_SV_Y;
        dpeConfig.Dpe_InBuf_SrcImg_Y_R = pImgBuf_MV_Y;
        dpeConfig.Dpe_InBuf_ValidMap_L = pImgBuf_MASK_S;
        dpeConfig.Dpe_InBuf_ValidMap_R = pImgBuf_MASK_M;
    }
    dpeConfig.Dpe_InBuf_SrcImg_C = pImgBuf_MY_S;
    if(mLastDV_LR.get() != nullptr)
        dpeConfig.Dpe_InBuf_DV = mLastDV_LR->mImageBuffer.get();
    else
        dpeConfig.Dpe_InBuf_DV = nullptr;
    // check HasDvInput setting
    if(dpeConfig.Dpe_DVSSettings.haveDVInput && dpeConfig.Dpe_InBuf_DV == nullptr)
        dpeConfig.Dpe_DVSSettings.haveDVInput = false;
    // output buffers
    dpeConfig.Dpe_OutBuf_DV = pBufferHandler->requestBuffer(getNodeId(), BID_DVS_OUT_DV_LR);
    dpeConfig.Dpe_OutBuf_CONF = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_OUT_CFM_M);
    dpeConfig.Dpe_OutBuf_OCC = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_OUT_NOC_M);
    dpeConfig.Dpe_InBuf_OCC = dpeConfig.Dpe_OutBuf_OCC;
    dpeConfig.Dpe_OutBuf_CRM = pBufferHandler->requestBuffer(getNodeId(), BID_ASF_OUT_CRM);
    dpeConfig.Dpe_OutBuf_ASF_RD = pBufferHandler->requestBuffer(getNodeId(), BID_ASF_OUT_RD);
    dpeConfig.Dpe_OutBuf_ASF_HF = pBufferHandler->requestBuffer(getNodeId(), BID_ASF_OUT_HF);
    dpeConfig.Dpe_OutBuf_WMF_FILT = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_OUT_DMW);
    //
    debugDPEConfig(dpeConfig);
    rEnqParam.mEGNConfigVec.push_back(dpeConfig);
    // update last DV_LR, record SmartFatImageBuffer to avoid release
    pBufferHandler->getEnquedSmartBuffer(getNodeId(), BID_DVS_OUT_DV_LR, mLastDV_LR);

    VSDOF_LOGD("-");
    return MTRUE;
}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam
