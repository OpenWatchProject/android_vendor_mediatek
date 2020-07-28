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
#include <camera_custom_stereo.h>
// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/DpeUtility.h>
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

using namespace NSCam::NSIoPipe::NSDpe;

#define STREAM_USER_NAME "DPENode"

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
setDMPBufferPool(sp<ImageBufferPool> pDMPBufPool)
{
    MY_LOGD("+");
    mpDMPBufferPool = pDMPBufPool;
    MY_LOGD("-");
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
        mpDPEStream->destroyInstance(STREAM_USER_NAME);
        mpDPEStream = nullptr;
    }
    mJobQueue.clear();
    mbFirst = MTRUE;
    mLastDMP_L = nullptr;
    mLastDMP_R = nullptr;
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
    mpDPEStream = NSCam::NSIoPipe::NSDpe::IDpeStream::createInstance(STREAM_USER_NAME);
    if(mpDPEStream == NULL)
    {
        MY_LOGE("DPE Stream create instance failed!");
        return MFALSE;
    }
    mpDPEStream->init();
    // init the last DMP
    mLastDMP_L = mpDMPBufferPool->request();
    mLastDMP_R = mpDMPBufferPool->request();

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
    VSDOF_LOGD("+ : reqId=%d", pRequest->getRequestNo());

    switch(data)
    {
        case N3D_TO_DPE_MVSV_MASK:
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
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
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
    // mark on-going-request start
    this->incExtThreadDependency();
    MUINT32 iReqIdx = pRequest->getRequestNo();

    VSDOF_PRFLOG("threadLoop start, reqID=%d", iReqIdx);
    CAM_ULOGM_TAGLIFE("DPENode::onThreadLoop");

    DVEParams enqueParams;
    MBOOL bRet = MFALSE;
    // prepare the enque configs
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
        bRet = prepareDPEEnqueConfig_PVVR(pRequest, enqueParams.mDVEConfigVec);
    }
    else
    {
        bRet = prepareDPEEnqueConfig_CAP(pRequest, enqueParams.mDVEConfigVec);
    }
    if(!bRet)
    {
        MY_LOGE("Failed to prepare DPE enque paramters! isCap: %d",
                pRequest->getRequestAttr().opState != eSTATE_NORMAL);
        return MFALSE;
    }
    // enque cookie instance
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(pRequest, this);
    enqueParams.mpfnCallback = onDPEEnqueDone;
    enqueParams.mpCookie = (void*) pCookieIns;
    // timer
    pRequest->mTimer.startDPE();
    pRequest->mTimer.startDPEEnque();
    CAM_ULOGM_TAG_BEGIN("DPENODE::DPEStream::enque");
    bRet = mpDPEStream->DVEenque(enqueParams);
    pRequest->mTimer.stopDPEEnque();
    VSDOF_PRFLOG("DVE Enque, reqID=%d, config time=%d ms", iReqIdx, pRequest->mTimer.getElapsedDPEEnque());
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
debugDPEConfig(NSIoPipe::DVEConfig& config)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    MY_LOGD("DVEConfig.Dve_Skp_Pre_Dv=%d", config.Dve_Skp_Pre_Dv);
    MY_LOGD("DVEConfig.Dve_Mask_En=%d", config.Dve_Mask_En);
    MY_LOGD("DVEConfig.Dve_l_Bbox_En=%d", config.Dve_l_Bbox_En);
    MY_LOGD("DVEConfig.Dve_r_Bbox_En=%d", config.Dve_r_Bbox_En);
    MY_LOGD("DVEConfig.Dve_Horz_Ds_Mode=%d", config.Dve_Horz_Ds_Mode);
    MY_LOGD("DVEConfig.Dve_Vert_Ds_Mode=%d", config.Dve_Vert_Ds_Mode);
    MY_LOGD("DVEConfig.Dve_Imgi_l_Fmt=%d", config.Dve_Imgi_l_Fmt);
    MY_LOGD("DVEConfig.Dve_Imgi_r_Fmt=%d", config.Dve_Imgi_r_Fmt);
    MY_LOGD("DVEConfig.Dve_Org_l_Bbox (left, right, top, down)=(%d, %d, %d, %d)",
                config.Dve_Org_l_Bbox.DVE_ORG_BBOX_LEFT, config.Dve_Org_l_Bbox.DVE_ORG_BBOX_RIGHT,
                config.Dve_Org_l_Bbox.DVE_ORG_BBOX_TOP, config.Dve_Org_l_Bbox.DVE_ORG_BBOX_BOTTOM);

    MY_LOGD("DVEConfig.Dve_Org_r_Bbox (left, right, top, down)=(%d, %d, %d, %d)",
                config.Dve_Org_r_Bbox.DVE_ORG_BBOX_LEFT, config.Dve_Org_r_Bbox.DVE_ORG_BBOX_RIGHT,
                config.Dve_Org_r_Bbox.DVE_ORG_BBOX_TOP, config.Dve_Org_r_Bbox.DVE_ORG_BBOX_BOTTOM);

    MY_LOGD("DVEConfig.Dve_Org_Width=%d", config.Dve_Org_Width);
    MY_LOGD("DVEConfig.Dve_Org_Height=%d", config.Dve_Org_Height);

    MY_LOGD("DVEConfig.Dve_Org_Horz_Sr_0=%d", config.Dve_Org_Horz_Sr_0);
    MY_LOGD("DVEConfig.Dve_Org_Horz_Sr_1=%d", config.Dve_Org_Horz_Sr_1);
    MY_LOGD("DVEConfig.Dve_Org_Vert_Sr_0=%d", config.Dve_Org_Vert_Sr_0);
    MY_LOGD("DVEConfig.Dve_Org_Start_Vert_Sv=%d", config.Dve_Org_Start_Vert_Sv);
    MY_LOGD("DVEConfig.Dve_Org_Start_Horz_Sv=%d", config.Dve_Org_Start_Horz_Sv);
    MY_LOGD("DVEConfig.Dve_Cand_Num=%d", config.Dve_Cand_Num);

    #define LOG_CANDIDATE(cand) \
        MY_LOGD("DVEConfig." #cand ".DVE_CAND_SEL = %d", config.cand.DVE_CAND_SEL); \
        MY_LOGD("DVEConfig." #cand ".DVE_CAND_TYPE = %d", config.cand.DVE_CAND_TYPE);

    LOG_CANDIDATE(Dve_Cand_0);
    LOG_CANDIDATE(Dve_Cand_1);
    LOG_CANDIDATE(Dve_Cand_2);
    LOG_CANDIDATE(Dve_Cand_3);
    LOG_CANDIDATE(Dve_Cand_4);
    LOG_CANDIDATE(Dve_Cand_5);
    LOG_CANDIDATE(Dve_Cand_6);
    LOG_CANDIDATE(Dve_Cand_7);

    MY_LOGD("DVEConfig.Dve_Rand_Lut_0=%d", config.Dve_Rand_Lut_0);
    MY_LOGD("DVEConfig.Dve_Rand_Lut_1=%d", config.Dve_Rand_Lut_1);
    MY_LOGD("DVEConfig.Dve_Rand_Lut_2=%d", config.Dve_Rand_Lut_2);
    MY_LOGD("DVEConfig.Dve_Rand_Lut_3=%d", config.Dve_Rand_Lut_3);
    MY_LOGD("DVEConfig.DVE_VERT_GMV=%d", config.DVE_VERT_GMV);
    MY_LOGD("DVEConfig.DVE_HORZ_GMV=%d", config.DVE_HORZ_GMV);
    MY_LOGD("DVEConfig.Dve_Horz_Dv_Ini=%d", config.Dve_Horz_Dv_Ini);
    MY_LOGD("DVEConfig.Dve_Coft_Shift=%d", config.Dve_Coft_Shift);
    MY_LOGD("DVEConfig.Dve_Corner_Th=%d", config.Dve_Corner_Th);
    MY_LOGD("DVEConfig.Dve_Smth_Luma_Th_1=%d", config.Dve_Smth_Luma_Th_1);
    MY_LOGD("DVEConfig.Dve_Smth_Luma_Th_0=%d", config.Dve_Smth_Luma_Th_0);
    MY_LOGD("DVEConfig.Dve_Smth_Luma_Ada_Base=%d", config.Dve_Smth_Luma_Ada_Base);
    MY_LOGD("DVEConfig.Dve_Smth_Luma_Horz_Pnlty_Sel=%d", config.Dve_Smth_Luma_Horz_Pnlty_Sel);
    MY_LOGD("DVEConfig.Dve_Smth_Dv_Mode=%d", config.Dve_Smth_Dv_Mode);
    MY_LOGD("DVEConfig.Dve_Smth_Dv_Th_1=%d", config.Dve_Smth_Dv_Th_1);
    MY_LOGD("DVEConfig.Dve_Smth_Dv_Th_0=%d", config.Dve_Smth_Dv_Th_0);
    MY_LOGD("DVEConfig.Dve_Smth_Dv_Ada_Base=%d", config.Dve_Smth_Dv_Ada_Base);
    MY_LOGD("DVEConfig.Dve_Smth_Dv_Vert_Pnlty_Sel=%d", config.Dve_Smth_Dv_Vert_Pnlty_Sel);
    MY_LOGD("DVEConfig.Dve_Smth_Dv_Horz_Pnlty_Sel=%d", config.Dve_Smth_Dv_Horz_Pnlty_Sel);
    MY_LOGD("DVEConfig.Dve_Ord_Pnlty_Sel=%d", config.Dve_Ord_Pnlty_Sel);
    MY_LOGD("DVEConfig.Dve_Ord_Coring=%d", config.Dve_Ord_Coring);
    MY_LOGD("DVEConfig.Dve_Ord_Th=%d", config.Dve_Ord_Th);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_REFINE_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_GMV_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_PREV_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_NBR_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_RAND_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_TMPR_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL=%d",
                config.Dve_Type_Penality_Ctrl.DVE_SPTL_PNLTY_SEL);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_RAND_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_RAND_COST);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_GMV_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_GMV_COST);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_PREV_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_PREV_COST);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_NBR_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_NBR_COST);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_REFINE_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_REFINE_COST);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_TMPR_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_TMPR_COST);
    MY_LOGD("DVEConfig.Dve_Type_Penality_Ctrl.DVE_SPTL_COST=%d",
                config.Dve_Type_Penality_Ctrl.DVE_SPTL_COST);

    #define LOG_DPEBuf(bufInfo) \
        MY_LOGD("DVEConfig." #bufInfo ".dmaport=%d", config.bufInfo.dmaport);\
        MY_LOGD("DVEConfig." #bufInfo ".memID=%d", config.bufInfo.memID); \
        MY_LOGD("DVEConfig." #bufInfo ".u4BufVA=%x", config.bufInfo.u4BufVA); \
        MY_LOGD("DVEConfig." #bufInfo ".u4BufPA=%x", config.bufInfo.u4BufPA); \
        MY_LOGD("DVEConfig." #bufInfo ".u4BufSize=%d", config.bufInfo.u4BufSize); \
        MY_LOGD("DVEConfig." #bufInfo ".u4Stride=%d", config.bufInfo.u4Stride);

    LOG_DPEBuf(Dve_Imgi_l);
    LOG_DPEBuf(Dve_Imgi_r);
    LOG_DPEBuf(Dve_Dvi_l);
    LOG_DPEBuf(Dve_Dvi_r);
    LOG_DPEBuf(Dve_Maski_l);
    LOG_DPEBuf(Dve_Maski_r);
    LOG_DPEBuf(Dve_Dvo_l);
    LOG_DPEBuf(Dve_Dvo_r);
    LOG_DPEBuf(Dve_Confo_l);
    LOG_DPEBuf(Dve_Confo_r);
    LOG_DPEBuf(Dve_Respo_l);
    LOG_DPEBuf(Dve_Respo_r);

    MY_LOGD("DVEConfig.Dve_Vert_Sv=%d", config.Dve_Vert_Sv);
    MY_LOGD("DVEConfig.Dve_Horz_Sv=%d", config.Dve_Horz_Sv);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
onDPEEnqueDone(DVEParams& rParams)
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
handleDPEEnqueDone(DVEParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_ULOGM_TAGLIFE("DPENode::handleDPEEnqueDone");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer *pImgBuf_DMP_L = nullptr, *pImgBuf_DMP_R = nullptr, *pImgBuf_CFM_L = nullptr;
    IImageBuffer *pImgBuf_RESPO_L = nullptr, *pImgBuf_RESPO_R = nullptr, *pImgBuf_CFM_R = nullptr;
    MBOOL bRet = MTRUE;

    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;

    // stop timer
    pRequest->mTimer.stopDPE();
    VSDOF_PRFTIME_LOG("+, reqID=%d, DPE exec-time=%d msec",
            pRequest->getRequestNo(), pRequest->mTimer.getElapsedDPE());

    // get the output buffers and invalid
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_DMP_L, pImgBuf_DMP_L);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_DMP_R, pImgBuf_DMP_R);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_CFM_L, pImgBuf_CFM_L);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_CFM_R, pImgBuf_CFM_R);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_RESPO_L, pImgBuf_RESPO_L);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_RESPO_R, pImgBuf_RESPO_R);

    if(!bRet)
    {
        MY_LOGE("Failed to get DPE output buffers");
    }
    // invalidate
    pImgBuf_DMP_L->syncCache(eCACHECTRL_INVALID);
    pImgBuf_DMP_R->syncCache(eCACHECTRL_INVALID);
    pImgBuf_CFM_L->syncCache(eCACHECTRL_INVALID);
    pImgBuf_CFM_R->syncCache(eCACHECTRL_INVALID);
    pImgBuf_RESPO_L->syncCache(eCACHECTRL_INVALID);
    pImgBuf_RESPO_R->syncCache(eCACHECTRL_INVALID);

    // update last DMP, record SmartImageBuffer to avoid release
    pBufferHandler->getEnquedSmartBuffer(getNodeId(), BID_DPE_OUT_DMP_L, mLastDMP_L);
    pBufferHandler->getEnquedSmartBuffer(getNodeId(), BID_DPE_OUT_DMP_R, mLastDMP_R);

    // prepare input for OCC
    pBufferHandler->configOutBuffer(getNodeId(), BID_DPE_OUT_DMP_L, eDPETHMAP_PIPE_NODEID_OCC);
    pBufferHandler->configOutBuffer(getNodeId(), BID_DPE_OUT_DMP_R, eDPETHMAP_PIPE_NODEID_OCC);
    pBufferHandler->configOutBuffer(getNodeId(), BID_DPE_OUT_CFM_L, eDPETHMAP_PIPE_NODEID_OCC);
    pBufferHandler->configOutBuffer(getNodeId(), BID_DPE_OUT_CFM_R, eDPETHMAP_PIPE_NODEID_OCC);
    // set ready for DMP output
    pRequest->setOutputBufferReady(BID_DPE_OUT_DMP_L);
    pRequest->setOutputBufferReady(BID_DPE_OUT_DMP_R);
    handleDataAndDump(DPE_OUT_DISPARITY, pRequest);
    // pass MV/SV to OCC
    pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_OCC);
    pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_SV_Y, eDPETHMAP_PIPE_NODEID_OCC);
    // post to OCC node
    handleDataAndDump(DPE_TO_OCC_MVSV_DMP_CFM, pRequest);
lbExit:
    // launch onProcessDone
    pBufferHandler->onProcessDone(getNodeId());
    delete pEnqueCookie;
    // mark on-going-request end
    this->decExtThreadDependency();
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
setupDPEBufInfo(
    NSIoPipe::DPEDMAPort dmaPort,
    IImageBuffer* pImgBuf,
    NSIoPipe::DPEBufInfo& rBufInfo
)
{
    // plane 0 address
    rBufInfo.memID = pImgBuf->getFD(0);
    rBufInfo.dmaport = dmaPort;
    rBufInfo.u4BufVA = pImgBuf->getBufVA(0);
    rBufInfo.u4BufPA = pImgBuf->getBufPA(0);
    rBufInfo.u4BufSize = pImgBuf->getBufSizeInBytes(0);
    rBufInfo.u4Stride = pImgBuf->getBufStridesInBytes(0);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
setupDPEConfigIO(
    DPEInputData input,
    DPEOutputData output,
    NSIoPipe::DVEConfig& rDPEConfig
)
{
    // imgi format: YV12 -> DPE_IMGI_Y_FMT
    if(input.pImgBuf_MV_Y->getImgFormat() == eImgFmt_YV12)
    {
        rDPEConfig.Dve_Imgi_l_Fmt = DPE_IMGI_Y_FMT;
        rDPEConfig.Dve_Imgi_r_Fmt = DPE_IMGI_Y_FMT;
    }
    else
    {
        MY_LOGE("Not supported IMGI format!");
        return MFALSE;
    }

    // setup input enque buffers
    if(STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())
    {
        // Main1 locations: L
        setupDPEBufInfo(DMA_DVE_IMGI, input.pImgBuf_MV_Y, rDPEConfig.Dve_Imgi_l);
        setupDPEBufInfo(DMA_DVE_IMGI, input.pImgBuf_SV_Y, rDPEConfig.Dve_Imgi_r);
        setupDPEBufInfo(DMA_DVE_MASKI, input.pImgBuf_MASK_M, rDPEConfig.Dve_Maski_l);
        setupDPEBufInfo(DMA_DVE_MASKI, input.pImgBuf_MASK_S, rDPEConfig.Dve_Maski_r);
    }
    else
    {   // Main1 location: R
        setupDPEBufInfo(DMA_DVE_IMGI, input.pImgBuf_SV_Y, rDPEConfig.Dve_Imgi_l);
        setupDPEBufInfo(DMA_DVE_IMGI, input.pImgBuf_MV_Y, rDPEConfig.Dve_Imgi_r);
        setupDPEBufInfo(DMA_DVE_MASKI, input.pImgBuf_MASK_S, rDPEConfig.Dve_Maski_l);
        setupDPEBufInfo(DMA_DVE_MASKI, input.pImgBuf_MASK_M, rDPEConfig.Dve_Maski_r);
    }

    setupDPEBufInfo(DMA_DVE_DVI, input.pImgBuf_LastDMP_L, rDPEConfig.Dve_Dvi_l);
    setupDPEBufInfo(DMA_DVE_DVI, input.pImgBuf_LastDMP_R, rDPEConfig.Dve_Dvi_r);
    // output
    setupDPEBufInfo(DMA_DVE_DVO, output.pImgBuf_DMP_L, rDPEConfig.Dve_Dvo_l);
    setupDPEBufInfo(DMA_DVE_DVO, output.pImgBuf_DMP_R, rDPEConfig.Dve_Dvo_r);
    setupDPEBufInfo(DMA_DVE_CONFO, output.pImgBuf_CFM_L, rDPEConfig.Dve_Confo_l);
    setupDPEBufInfo(DMA_DVE_CONFO, output.pImgBuf_CFM_R, rDPEConfig.Dve_Confo_r);
    setupDPEBufInfo(DMA_DVE_RESPO, output.pImgBuf_RESPO_L, rDPEConfig.Dve_Respo_l);
    setupDPEBufInfo(DMA_DVE_RESPO, output.pImgBuf_RESPO_R, rDPEConfig.Dve_Respo_r);

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
prepareDPEEnqueConfig_PVVR(
    DepthMapRequestPtr pRequest,
    std::vector<NSIoPipe::DVEConfig>& rDPEConfigVec
)
{
    VSDOF_LOGD("+");

    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // insert the dynamic data
    IImageBuffer *pImgBuf_MV_Y = nullptr, *pImgBuf_SV_Y = nullptr;
    IImageBuffer *pImgBuf_MASK_M = nullptr, *pImgBuf_MASK_S = nullptr;
    // input buffer
    MBOOL bRet = pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MV_Y, pImgBuf_MV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_SV_Y, pImgBuf_SV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MASK_M, pImgBuf_MASK_M);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MASK_S, pImgBuf_MASK_S);
    if(!bRet)
        return MFALSE;
    pImgBuf_SV_Y->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MV_Y->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MASK_M->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MASK_S->syncCache(eCACHECTRL_FLUSH);

    // output buffers
    IImageBuffer* pImgBuf_DMP_L =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_DMP_L);
    IImageBuffer* pImgBuf_DMP_R =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_DMP_R);
    IImageBuffer* pImgBuf_CFM_L =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_CFM_L);
    IImageBuffer* pImgBuf_CFM_R =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_CFM_R);
    IImageBuffer* pImgBuf_RESPO_L =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_RESPO_L);
    IImageBuffer* pImgBuf_RESPO_R =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_RESPO_R);

    // read the tuning data from tuning provider
    NSIoPipe::DVEConfig dpeConfig;
    ENUM_STEREO_SCENARIO scenario = (pRequest->getRequestAttr().isEISOn) ? eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
    StereoTuningProvider::getDPETuningInfo(&dpeConfig, scenario);
    // mask flag: use the tuning provider firstly.
    dpeConfig.Dve_Mask_En &= true;
    // check the first enque or not
    dpeConfig.Dve_Skp_Pre_Dv = (mbFirst) ? true : false;
    mbFirst = MFALSE;

    // setup IO
    DPEInputData input = {pImgBuf_MV_Y, pImgBuf_SV_Y, pImgBuf_MASK_M, pImgBuf_MASK_S,
                          mLastDMP_L->mImageBuffer.get(), mLastDMP_R->mImageBuffer.get()};
    DPEOutputData output = {pImgBuf_DMP_L, pImgBuf_DMP_R, pImgBuf_CFM_L, pImgBuf_CFM_R,
                            pImgBuf_RESPO_L, pImgBuf_RESPO_R};

    if(!setupDPEConfigIO(input, output, dpeConfig))
    {
        MY_LOGE("Setup failed!");
        return MFALSE;
    }
    debugDPEConfig(dpeConfig);
    rDPEConfigVec.push_back(dpeConfig);


    VSDOF_LOGD("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DPENode::
debugDPEIOData(
    DPEInputData input,
    DPEOutputData output
)
{
    // debug section
    if(DepthPipeLoggingSetup::mbDebugLog)
    {
        #define DEBUG_BUFFER_SETUP(buf) \
            MY_LOGD("DPE buf:" # buf);\
            MY_LOGD("Image buffer size=%dx%d:", buf->getImgSize().w, buf->getImgSize().h);\
            MY_LOGD("Image buffer format=%x", buf->getImgFormat());\

        MY_LOGD("STEREO_SENSOR_REAR_MAIN_TOP == getSensorRelativePosition()",
            STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition());

        DEBUG_BUFFER_SETUP(input.pImgBuf_MV_Y);
        DEBUG_BUFFER_SETUP(input.pImgBuf_SV_Y);
        DEBUG_BUFFER_SETUP(input.pImgBuf_MASK_M);
        DEBUG_BUFFER_SETUP(input.pImgBuf_MASK_S);
        DEBUG_BUFFER_SETUP(input.pImgBuf_LastDMP_L);
        DEBUG_BUFFER_SETUP(input.pImgBuf_LastDMP_R);
        DEBUG_BUFFER_SETUP(output.pImgBuf_DMP_L);
        DEBUG_BUFFER_SETUP(output.pImgBuf_DMP_R);
        DEBUG_BUFFER_SETUP(output.pImgBuf_CFM_L);
        DEBUG_BUFFER_SETUP(output.pImgBuf_CFM_R);
        DEBUG_BUFFER_SETUP(output.pImgBuf_RESPO_L);
        DEBUG_BUFFER_SETUP(output.pImgBuf_RESPO_R);

        #undef DEBUG_BUFFER_SETUP
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DPENode::
prepareDPEEnqueConfig_CAP(
    DepthMapRequestPtr pRequest,
    std::vector<NSIoPipe::DVEConfig>& rDPEConfigVec
)
{
    VSDOF_LOGD("+");
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // insert the dynamic data
    IImageBuffer *pImgBuf_MV_Y = nullptr, *pImgBuf_SV_Y = nullptr;
    IImageBuffer *pImgBuf_MASK_M = nullptr, *pImgBuf_MASK_S = nullptr;
    // input buffer
    MBOOL bRet = pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MV_Y, pImgBuf_MV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_SV_Y, pImgBuf_SV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MASK_M, pImgBuf_MASK_M);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MASK_S, pImgBuf_MASK_S);
    if(!bRet)
        return MFALSE;
    pImgBuf_SV_Y->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MV_Y->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MASK_M->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_MASK_S->syncCache(eCACHECTRL_FLUSH);
    // last DMP
    IImageBuffer* pLastDMP_L =  pBufferHandler->requestWorkingBuffer(getNodeId(), BID_DPE_INTERNAL_LAST_DMP);
    IImageBuffer* pLastDMP_R =  pBufferHandler->requestWorkingBuffer(getNodeId(), BID_DPE_INTERNAL_LAST_DMP);
    memset((void*)pLastDMP_L->getBufVA(0), 0, pLastDMP_L->getBufSizeInBytes(0));
    memset((void*)pLastDMP_R->getBufVA(0), 0, pLastDMP_R->getBufSizeInBytes(0));
    pLastDMP_L->syncCache(eCACHECTRL_FLUSH);
    pLastDMP_R->syncCache(eCACHECTRL_FLUSH);

    // output buffers
    IImageBuffer* pImgBuf_DMP_L =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_DMP_L);
    IImageBuffer* pImgBuf_DMP_R =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_DMP_R);
    IImageBuffer* pImgBuf_CFM_L =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_CFM_L);
    IImageBuffer* pImgBuf_CFM_R =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_CFM_R);
    IImageBuffer* pImgBuf_RESPO_L =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_RESPO_L);
    IImageBuffer* pImgBuf_RESPO_R =  pBufferHandler->requestBuffer(getNodeId(), BID_DPE_OUT_RESPO_R);

    MUINT32 DPE_RUN = StereoSettingProvider::getDPECaptureRound();

    for(MUINT32 index=0;index<DPE_RUN;index++)
    {
        NSIoPipe::DVEConfig dpeConfig;
        // read the tuning data from tuning provider
        StereoTuningProvider::getDPETuningInfo(&dpeConfig, eSTEREO_SCENARIO_CAPTURE);
        // mask flag: use the tuning provider firstly.
        dpeConfig.Dve_Mask_En &= true;
        dpeConfig.Dve_Skp_Pre_Dv = (index == 0) ? true : false;
         // setup IO
        DPEInputData input = {pImgBuf_MV_Y, pImgBuf_SV_Y, pImgBuf_MASK_M, pImgBuf_MASK_S,
                                pLastDMP_L, pLastDMP_R};
        DPEOutputData output;
        // last run
        if(index == DPE_RUN-1)
        {
            output = {pImgBuf_DMP_L, pImgBuf_DMP_R, pImgBuf_CFM_L, pImgBuf_CFM_R,
                            pImgBuf_RESPO_L, pImgBuf_RESPO_R};
        }
        else
        {
            IImageBuffer* pInternalDMP_L =  pBufferHandler->requestWorkingBuffer(getNodeId(), BID_DPE_INTERNAL_DMP);
            IImageBuffer* pInternalDMP_R =  pBufferHandler->requestWorkingBuffer(getNodeId(), BID_DPE_INTERNAL_DMP);
            output = {pInternalDMP_L, pInternalDMP_R, pImgBuf_CFM_L, pImgBuf_CFM_R,
                    pImgBuf_RESPO_L, pImgBuf_RESPO_R};
            // update the last DMP
            pLastDMP_L = pInternalDMP_L;
            pLastDMP_R = pInternalDMP_R;
        }

        // setup DPE io
        if(!setupDPEConfigIO(input, output, dpeConfig))
        {
            MY_LOGE("Setup failed!");
            return MFALSE;
        }

        debugDPEConfig(dpeConfig);
        rDPEConfigVec.push_back(dpeConfig);
    }

    VSDOF_LOGD("-");
    return MTRUE;
}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam
