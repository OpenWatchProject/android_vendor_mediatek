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

 /**
 * @file DepthMapFlowOption_BMVSDOF.cpp
 * @brief DepthMapFlowOption for bayer+mono VSDOF
 */

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_stereo.h>
// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <drv/isp_reg.h>
#include <isp_tuning.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "DepthMapFlowOption_BMVSDOF.h"
#include "../DepthMapPipe.h"
#include "../DepthMapPipeNode.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
#include "../nodes/P2ABayerNode.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "DepthMapFlowOption_BMVSDOF"
#include <featurePipe/core/include/PipeLog.h>


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DepthMapFlowOption_BMVSDOF::
DepthMapFlowOption_BMVSDOF(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pOption,
    sp<DepthInfoStorage> pStorage
)
: BayerMonoQTemplateProvider(pSetting, pOption, this)
, mpPipeOption(pOption)
{
    mpDepthStorage = pStorage;
    //
    mpQParamPvdr_BB = new BayerBayerQTemplateProvider(pSetting, pOption, this);
    mpSizeMgr = new NodeBufferSizeMgr(mpPipeOption);
    // flow option config
    mConfig.mbCaptureFDEnable = MTRUE;
    const P2ABufferSize& P2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    mConfig.mFDSize = P2ASize.mFD_IMG_SIZE;
}

DepthMapFlowOption_BMVSDOF::
~DepthMapFlowOption_BMVSDOF()
{
    MY_LOGD("[Destructor] +");
    if(mpQParamPvdr_BB != nullptr)
        delete mpQParamPvdr_BB;
    if(mpSizeMgr != nullptr)
        delete mpSizeMgr;
    MY_LOGD("[Destructor] -");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2AFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

INPUT_RAW_TYPE
DepthMapFlowOption_BMVSDOF::
getInputRawType(
    sp<DepthMapEffectRequest> pReq,
    StereoP2Path path
)
{
    INPUT_RAW_TYPE rawType;
    if(pReq->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        if(path == eP2APATH_MAIN1_BAYER)
            rawType = eFULLSIZE_RAW;
        // when disable LCE
        else if(!mpPipeOption->mbEnableLCE)
            rawType = (path == eP2APATH_MAIN1) ? eFULLSIZE_RAW : eRESIZE_RAW;
        // when enable LCE
        else
            rawType = eRESIZE_RAW;
    }
    else
    {
        rawType = eRESIZE_RAW;
    }
    return rawType;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
buildQParam(
    DepthMapRequestPtr pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParam
)
{
    MBOOL bRet = MFALSE;
    DepthMapPipeOpState opState = pRequest->getRequestAttr().opState;
    if(opState == eSTATE_NORMAL)
    {
        // Disable LCE -> Run BB flow (no bayer node)
        if(!mpPipeOption->mbEnableLCE)
            bRet = mpQParamPvdr_BB->buildQParams_NORMAL(pRequest, tuningResult, rOutParam);
        else
            bRet = TemplateProvider::buildQParams_NORMAL(pRequest, tuningResult, rOutParam);
    }
    else if(opState == eSTATE_CAPTURE)
        bRet = TemplateProvider::buildQParams_CAPTURE(pRequest, tuningResult, rOutParam);

    return bRet;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
onP2ProcessDone(
    P2AFMNode* pNode,
    sp<DepthMapEffectRequest> pRequest
)
{
    // notes: skip depth frame will NOT be go to this stage, bcz it will be bypass at threadloop
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // disable LCE(no P2ABayer node) -> MY_S is generate by P2AFM
    if(!mpPipeOption->mbEnableLCE && eSTATE_NORMAL == pRequest->getRequestAttr().opState)
    {
        // output to P2ABayer
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_P2ABAYER);
    }
    // enque request into P2ABayer Node
    pNode->handleData(BAYER_ENQUE, pRequest);

    // config output buffers for N3D input
    // FEO
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FE1BO, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FE2BO, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FE1CO, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FE2CO, eDPETHMAP_PIPE_NODEID_N3D);
    // FMO
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FMBO_LR, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FMBO_RL, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FMCO_LR, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_FMCO_RL, eDPETHMAP_PIPE_NODEID_N3D);
    // CC_in/Rect_in
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_CC_IN1, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_CC_IN2, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_RECT_IN1, eDPETHMAP_PIPE_NODEID_N3D);
    pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2AFM, BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_N3D);
    // pass to N3D
    pNode->handleDataAndDump(P2A_TO_N3D_FEFM_CCin, pRequest);

    return MTRUE;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
buildQParam_Bayer(
    DepthMapRequestPtr pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParam
)
{
    MBOOL bRet = MFALSE;
    DepthMapPipeOpState opState = pRequest->getRequestAttr().opState;
    if(opState == eSTATE_NORMAL)
        bRet = TemplateProvider::buildQParams_BAYER_NORMAL(pRequest, tuningResult, rOutParam);
    else if(opState == eSTATE_CAPTURE)
        bRet = TemplateProvider::buildQParams_BAYER_CAPTURE(pRequest, tuningResult, rOutParam);

    return bRet;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
onP2ProcessDone_Bayer(
    P2ABayerNode* pNode,
    sp<DepthMapEffectRequest> pRequest
)
{
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // mark buffer ready and handle data
    // FD
    if(pRequest->setOutputBufferReady(BID_P2A_OUT_FDIMG))
        pNode->handleDataAndDump(P2A_OUT_FD, pRequest);
    // MV_F
    if(pRequest->setOutputBufferReady(BID_P2A_OUT_MV_F))
        pNode->handleDataAndDump(P2A_OUT_MV_F, pRequest);
    // MV_F_CAP
    if(pRequest->setOutputBufferReady(BID_P2A_OUT_MV_F_CAP))
        pNode->handleDataAndDump(P2A_OUT_MV_F_CAP, pRequest);
    // config main1 tuning ready
    if(pRequest->setOutputBufferReady(BID_P2A_OUT_TUNING_MAIN1))
        pNode->handleData(P2A_OUT_TUNING_BUF, pRequest);

    // output to OCC if not skip depth
    if(!pRequest->isSkipDepth(mpPipeOption))
    {
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_OCC);
        pNode->handleDataAndDump(P2A_TO_OCC_MY_S, pRequest);
    }
    // if capture
    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        // PostView
        pRequest->setOutputBufferReady(BID_P2A_OUT_POSTVIEW);
        // notify YUV done
        pNode->handleDataAndDump(P2A_OUT_YUV_DONE, pRequest);
        // pass to FD node
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_FDIMG, eDPETHMAP_PIPE_NODEID_FD);
        pNode->handleDataAndDump(P2A_TO_FD_IMG, pRequest);
    }

    VSDOF_LOGD("- reqID=%d", pRequest->getRequestNo());

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
DepthMapFlowOption_BMVSDOF::
init()
{
    // prepare template
    MBOOL bRet = BayerMonoQTemplateProvider::init(mpSizeMgr);
    // init the QParamPvdr_BB
    bRet &= mpQParamPvdr_BB->init(mpSizeMgr);
    return bRet;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
queryReqAttrs(
    sp<DepthMapEffectRequest> pRequest,
    EffectRequestAttrs& rReqAttrs
)
{
    // deccide EIS on/off
    IMetadata* pInAppMeta = nullptr;
    pRequest->getRequestMetadata({.bufferID=BID_META_IN_APP, .ioType=eBUFFER_IOTYPE_INPUT}
                                   , pInAppMeta);

    rReqAttrs.isEISOn = isEISOn(pInAppMeta);
    // decide OpState and BufferScenario
    IImageBuffer* pImgBuf_MV_F_CAP = nullptr;
    MBOOL bRet= pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_MV_F_CAP,
                                                    .ioType=eBUFFER_IOTYPE_OUTPUT}, pImgBuf_MV_F_CAP);
    if(bRet)
    {
        rReqAttrs.opState = eSTATE_CAPTURE;
        rReqAttrs.bufferScenario = eBUFFER_POOL_SCENARIO_CAPTURE;
    }
    else
    {
        rReqAttrs.opState = eSTATE_NORMAL;
        if(rReqAttrs.isEISOn)
            rReqAttrs.bufferScenario = eBUFFER_POOL_SCENARIO_RECORD;
        else
            rReqAttrs.bufferScenario = eBUFFER_POOL_SCENARIO_PREVIEW;
    }

    return MTRUE;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
queryPipeNodeBitSet(PipeNodeBitSet& nodeBitSet)
{
    nodeBitSet.reset();
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2AFM, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_N3D, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_DPE, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_WMF, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_OCC, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_FD, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_GF, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2ABAYER, 1);
    return MTRUE;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
buildPipeGraph(DepthMapPipe* pPipe, const DepthMapPipeNodeMap& nodeMap)
{
    DepthMapPipeNode* pP2AFMNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_P2AFM);
    DepthMapPipeNode* pN3DNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_N3D);
    DepthMapPipeNode* pDPENode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_DPE);
    DepthMapPipeNode* pOCCNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_OCC);
    DepthMapPipeNode* pWMFNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_WMF);
    DepthMapPipeNode* pGFNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_GF);
    DepthMapPipeNode* pFDNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_FD);
    DepthMapPipeNode* pP2ABayerNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_P2ABAYER);

    #define CONNECT_DATA(DataID, src, dst)\
        pPipe->connectData(DataID, DataID, src, dst);\
        mvAllowDataIDMap.add(DataID, MTRUE);

    // P2AFM to N3D + P2A_OUTput
    CONNECT_DATA(P2A_TO_N3D_FEFM_CCin, *pP2AFMNode, *pN3DNode);
    CONNECT_DATA(TO_DUMP_BUFFERS, *pP2AFMNode, pPipe);
    // P2ABayer
    CONNECT_DATA(BAYER_ENQUE, *pP2AFMNode, *pP2ABayerNode);
    CONNECT_DATA(P2A_OUT_MV_F, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_FD, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_MV_F_CAP, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_YUV_DONE, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_TO_FD_IMG, *pP2ABayerNode, *pFDNode); // to FD
    CONNECT_DATA(REQUEST_DEPTH_NOT_READY, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_TUNING_BUF, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_TO_OCC_MY_S, *pP2ABayerNode, *pOCCNode);
    // P2AFM buffer dump
    CONNECT_DATA(TO_DUMP_RAWS, *pP2AFMNode, pPipe);
    // DPE to OCC
    CONNECT_DATA(DPE_TO_OCC_MVSV_DMP_CFM, *pDPENode, *pOCCNode);
    // N3D to DPE/OCC
    CONNECT_DATA(N3D_TO_DPE_MVSV_MASK, *pN3DNode, *pDPENode);
    CONNECT_DATA(N3D_TO_OCC_LDC, *pN3DNode, *pOCCNode);
    CONNECT_DATA(N3D_OUT_JPS_WARPMTX, *pN3DNode, pPipe);
    // Hal Meta frame output
    CONNECT_DATA(DEPTHMAP_META_OUT, *pN3DNode, pPipe);
    // N3D to FD
    CONNECT_DATA(N3D_TO_FD_EXTDATA_MASK, *pN3DNode, *pFDNode);
    //OCC to WMF + OCC_output
    CONNECT_DATA(OCC_TO_WMF_OMYSN, *pOCCNode, *pWMFNode);
    // WMF to GF
    CONNECT_DATA(WMF_TO_GF_DMW_MY_S, *pWMFNode, *pGFNode);
    CONNECT_DATA(WMF_TO_GF_OND, *pWMFNode, *pGFNode);
    // GF output
    CONNECT_DATA(GF_OUT_DMBG, *pGFNode, pPipe);
    CONNECT_DATA(GF_OUT_DEPTH_WRAPPER, *pGFNode, pPipe);
    //FDNode output
    CONNECT_DATA(FD_OUT_EXTRADATA, *pFDNode, pPipe);
    // QUEUED DEPTH specific
    if(mpPipeOption->mFlowType == eDEPTH_FLOW_TYPE_QUEUED_DEPTH)
    {
        CONNECT_DATA(P2A_TO_GF_DMW_MYS, *pP2ABayerNode, *pGFNode);
        // queue flow done
        for(size_t index=0;index<nodeMap.size();++index)
        {
            CONNECT_DATA(QUEUED_FLOW_DONE, *nodeMap.valueAt(index), pPipe);
        }

    }
    // default node graph - Error handling
    for(size_t index=0;index<nodeMap.size();++index)
    {
        CONNECT_DATA(ERROR_OCCUR_NOTIFY, *nodeMap.valueAt(index), pPipe);
    }
    pPipe->setRootNode(pP2AFMNode);

    return MTRUE;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
checkConnected(
    DepthMapDataID dataID
)
{
    // check data id is allowed to handledata or not.
    if(mvAllowDataIDMap.indexOfKey(dataID)>=0)
    {
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
config3ATuningMeta(
    sp<DepthMapEffectRequest> pRequest,
    StereoP2Path path,
    MetaSet_T& rMetaSet
)
{
    // P2AFM node always input the RRZO buffer --> only Preview scenario
    // Disable LCE(no bayer node) -> not to_W profile
    if((path == eP2APATH_MAIN1
            && !mpPipeOption->mbEnableLCE
            && pRequest->getRequestAttr().opState == eSTATE_NORMAL)
        || path == eP2APATH_MAIN2)
    {
        trySetMetadata<MUINT8>(&rMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Preview);
    }
    else
    {
        //trySetMetadata<MUINT8>(&rMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Preview_toW);
    }

    return MTRUE;
}

DepthMapBufferID
DepthMapFlowOption_BMVSDOF::
reMapBufferID(
    const EffectRequestAttrs& reqAttr,
    DepthMapBufferID bufferID
)
{
    return bufferID;
}

MBOOL
DepthMapFlowOption_BMVSDOF::
needBypassP2AFM(sp<DepthMapEffectRequest> pRequest)
{
    // bypass the P2AFM node when skip request
    return pRequest->isSkipDepth(mpPipeOption);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_BMVSDOF Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam








