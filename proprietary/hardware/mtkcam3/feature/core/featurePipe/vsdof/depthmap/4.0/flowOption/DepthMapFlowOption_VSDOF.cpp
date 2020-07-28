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

/**
 * @file DepthMapFlowOption_VSDOF.cpp
 * @brief DepthMapFlowOption for VSDOF
 */

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_stereo.h>
// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <isp_tuning.h>
// Module header file
#include <stereo_tuning_provider.h>
#include <fefm_setting_provider.h>
// Local header file
#include "DepthMapFlowOption_VSDOF.h"
#include "../DepthMapPipe.h"
#include "../DepthMapPipeNode.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "DepthMapFlowOption_VSDOF"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSIspTuning;
typedef NodeBufferSizeMgr::DPEBufferSize DPEBufferSize;
typedef NodeBufferSizeMgr::P2ABufferSize P2ABufferSize;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DepthMapFlowOption_VSDOF::
DepthMapFlowOption_VSDOF(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pOption,
    sp<DepthInfoStorage> pStorage
)
: DepthQTemplateProvider(pSetting, pOption, this)
, mpPipeOption(pOption)
{
    mpDepthStorage = pStorage;
    //
    mpSizeMgr = new NodeBufferSizeMgr(mpPipeOption);
    // flow option config
    const P2ABufferSize& P2ASize = mpSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    mConfig.mbCaptureFDEnable = MTRUE;
    mConfig.mFDSize = P2ASize.mFD_IMG_SIZE;
}

DepthMapFlowOption_VSDOF::
~DepthMapFlowOption_VSDOF()
{
    MY_LOGD("[Destructor] +");
    if(mpSizeMgr != nullptr)
        delete mpSizeMgr;
    MY_LOGD("[Destructor] -");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2AFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


MBOOL
DepthMapFlowOption_VSDOF::
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
        if(pRequest->getRequestAttr().needFEFM)
            bRet = TemplateProvider::buildQParams_NORMAL(pRequest, tuningResult, rOutParam);
        else
            bRet = TemplateProvider::buildQParams_NORMAL_NOFEFM(pRequest, tuningResult, rOutParam);
    }
    else if(opState == eSTATE_CAPTURE)
    {
        bRet = TemplateProvider::buildQParams_CAPTURE(pRequest, tuningResult, rOutParam);
    }
    return bRet;
}

MBOOL
DepthMapFlowOption_VSDOF::
onP2ProcessDone(
    P2ANode* pNode,
    sp<DepthMapEffectRequest> pRequest
)
{
    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    // notify template provider p2 done
    TemplateProvider::onHandleP2Done(eDPETHMAP_PIPE_NODEID_P2A, pRequest);
    VSDOF_LOGD("-, reqID=%d", pRequest->getRequestNo());
    return MTRUE;
}

INPUT_RAW_TYPE
DepthMapFlowOption_VSDOF::
getInputRawType(
    sp<DepthMapEffectRequest> pReq,
    StereoP2Path path
)
{
    INPUT_RAW_TYPE rawType;
    if(pReq->getRequestAttr().opState == eSTATE_CAPTURE &&
        path == eP2APATH_MAIN1_BAYER)
    {
        rawType = eFULLSIZE_RAW;
    }
    else
    {
        rawType = eRESIZE_RAW;
    }
    return rawType;
}


MBOOL
DepthMapFlowOption_VSDOF::
buildQParam_Bayer(
    DepthMapRequestPtr pRequest,
    const Stereo3ATuningRes& tuningResult,
    QParams& rOutParam
)
{
    // only Preview_NOFEFM/Capture can go here.
    MBOOL bRet = MFALSE;
    DepthMapPipeOpState opState = pRequest->getRequestAttr().opState;
    MBOOL bNeedFEFM = pRequest->getRequestAttr().needFEFM;
    if(opState == eSTATE_NORMAL && bNeedFEFM)
        bRet = TemplateProvider::buildQParams_BAYER_NORMAL(pRequest, tuningResult, rOutParam);
    else if(opState == eSTATE_NORMAL && !bNeedFEFM)
        bRet = TemplateProvider::buildQParams_BAYER_NORMAL_NOFEFM(pRequest, tuningResult, rOutParam);
    else if(opState == eSTATE_STANDALONE)
        bRet = TemplateProvider::buildQParams_BAYER_STANDALONE(pRequest, tuningResult, rOutParam);
    else
        MY_LOGE("Not valid request!!bNeedFEFM :%d", bNeedFEFM);
    return bRet;
}

MBOOL
DepthMapFlowOption_VSDOF::
onP2ProcessDone_Bayer(
    P2ABayerNode* pNode,
    sp<DepthMapEffectRequest> pRequest
)
{
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    // notify template provider p2 done
    TemplateProvider::onHandleP2Done(eDPETHMAP_PIPE_NODEID_P2ABAYER, pRequest);
    VSDOF_LOGD("- reqID=%d", pRequest->getRequestNo());

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption_VSDOF private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
DepthMapFlowOption_VSDOF::
init()
{
    // prepare template
    MBOOL bRet = TemplateProvider::init(mpSizeMgr);
    return bRet;
}

MBOOL
DepthMapFlowOption_VSDOF::
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
    if(pRequest->isRequestBuffer(BID_P2A_IN_YUV1))
    {
        rReqAttrs.opState = eSTATE_CAPTURE;
        rReqAttrs.bufferScenario = eBUFFER_POOL_SCENARIO_CAPTURE;
    }
    else
    {
        // check standalone request
        if(!pRequest->isRequestBuffer(BID_P2A_IN_RSRAW2))
        {
            rReqAttrs.opState = eSTATE_STANDALONE;
        }
        else
        {
            rReqAttrs.opState = eSTATE_NORMAL;
            if(rReqAttrs.isEISOn)
                rReqAttrs.bufferScenario = eBUFFER_POOL_SCENARIO_RECORD;
            else
                rReqAttrs.bufferScenario = eBUFFER_POOL_SCENARIO_PREVIEW;
        }
    }

    if(rReqAttrs.opState != eSTATE_STANDALONE)
    {
        //
        MINT32 magicNum1 = 0, magicNum2 = 0;
        IMetadata* pInHalMeta_Main1 = nullptr, *pInHalMeta_Main2 = nullptr;
        MBOOL bRet = pRequest->getRequestMetadata({.bufferID=BID_META_IN_HAL_MAIN1,
                                        .ioType=eBUFFER_IOTYPE_INPUT}, pInHalMeta_Main1);
        bRet &= pRequest->getRequestMetadata({.bufferID=BID_META_IN_HAL_MAIN2,
                                        .ioType=eBUFFER_IOTYPE_INPUT}, pInHalMeta_Main2);
        if(!bRet)
        {
            MY_LOGE("Cannot get the input metadata!");
            return MFALSE;
        }
        if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum1)) {
            MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
        }
        if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum2)) {
            MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main2!");
        }
    #ifndef GTEST
        rReqAttrs.needFEFM = FEFMSettingProvider::getInstance()->needToRunFEFM(
                                    magicNum1, magicNum2, (rReqAttrs.opState==eSTATE_CAPTURE));
    #else
        rReqAttrs.needFEFM = 1;
    #endif
    }
    return MTRUE;
}


MBOOL
DepthMapFlowOption_VSDOF::
queryPipeNodeBitSet(PipeNodeBitSet& nodeBitSet)
{
    nodeBitSet.reset();
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2A, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2ABAYER, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_N3D, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_DPE, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_WPE, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_DLDEPTH, 1);
    // check mode
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF)
        nodeBitSet.set(eDPETHMAP_PIPE_NODEID_GF, 1);
    else
        nodeBitSet.set(eDPETHMAP_PIPE_NODEID_GF, 0);
    return MTRUE;
}

MBOOL
DepthMapFlowOption_VSDOF::
buildPipeGraph(DepthMapPipe* pPipe, const DepthMapPipeNodeMap& nodeMap)
{
    DepthMapPipeNode* pP2ANode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_P2A);
    DepthMapPipeNode* pN3DNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_N3D);
    DepthMapPipeNode* pDPENode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_DPE);
    DepthMapPipeNode* pP2ABayerNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_P2ABAYER);
    DepthMapPipeNode* pWPENode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_WPE);
    DepthMapPipeNode* pDLDepthNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_DLDEPTH);

    #define CONNECT_DATA(DataID, src, dst)\
        pPipe->connectData(DataID, DataID, src, dst);\
        mvAllowDataIDMap.add(DataID, MTRUE);

    // P2A to N3D + P2A_OUTput
    CONNECT_DATA(P2A_TO_N3D_PADDING_MATRIX, *pP2ANode, *pN3DNode);
    CONNECT_DATA(P2A_TO_N3D_RECT2_FEO, *pP2ANode, *pN3DNode);
    CONNECT_DATA(P2A_TO_N3D_NOFEFM_RECT2, *pP2ANode, *pN3DNode);
    CONNECT_DATA(P2A_TO_N3D_CAP_RECT2, *pP2ANode, *pN3DNode);
    CONNECT_DATA(TO_DUMP_BUFFERS, *pP2ANode, pPipe);
    CONNECT_DATA(TO_DUMP_IMG3O, *pP2ANode, pPipe);
    CONNECT_DATA(TO_DUMP_RAWS, *pP2ANode, pPipe);
    // used when need fefm
    CONNECT_DATA(P2A_OUT_MV_F, *pP2ANode, pPipe);
    CONNECT_DATA(P2A_OUT_FD, *pP2ANode, pPipe);
    // P2ABayer
    CONNECT_DATA(BAYER_ENQUE, *pP2ANode, *pP2ABayerNode);
    CONNECT_DATA(P2A_TO_N3D_FEOFMO, *pP2ABayerNode, *pN3DNode);
    CONNECT_DATA(P2A_OUT_MV_F, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_FD, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_MV_F_CAP, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_OUT_YUV_DONE, *pP2ABayerNode, pPipe);
    CONNECT_DATA(REQUEST_DEPTH_NOT_READY, *pP2ABayerNode, pPipe);
    CONNECT_DATA(P2A_TO_DPE_MY_S, *pP2ABayerNode, *pDPENode);
    CONNECT_DATA(TO_DUMP_IMG3O, *pP2ABayerNode, pPipe);
    // N3DNode
    CONNECT_DATA(N3D_TO_WPE_RECT2_WARPMTX, *pN3DNode, *pWPENode);
    CONNECT_DATA(N3D_TO_P2ABYER_P1YUV_USED, *pN3DNode, *pP2ABayerNode);
    // WPENode
    CONNECT_DATA(WPE_TO_DPE_WARP_IMG, *pWPENode, *pDPENode);
    CONNECT_DATA(WPE_TO_DLDEPTH_MV_SV, *pWPENode, *pDLDepthNode);


    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF)
    {
        DepthMapPipeNode* pGFNode = nodeMap.valueFor(eDPETHMAP_PIPE_NODEID_GF);
        // DPENode
        CONNECT_DATA(DPE_TO_GF_DMW_N_DEPTH, *pDPENode, *pGFNode);
        // GF output
        CONNECT_DATA(GF_OUT_DMBG, *pGFNode, pPipe);
        CONNECT_DATA(GF_OUT_INTERNAL_DMBG, *pGFNode, pPipe);
    }
    else
    {
        CONNECT_DATA(DPE_OUT_INTERNAL_DEPTH, *pDPENode, pPipe);
    }
    // DLDepth
    CONNECT_DATA(DLDEPTH_OUT_DEPTHMAP, *pDLDepthNode, pPipe);
    // Hal Meta frame output
    CONNECT_DATA(DEPTHMAP_META_OUT, *pN3DNode, pPipe);

    // QUEUED DEPTH specific
    if(mpPipeOption->mFlowType == eDEPTH_FLOW_TYPE_QUEUED_DEPTH)
    {
        CONNECT_DATA(P2A_NORMAL_FRAME_DONE, *pP2ABayerNode, pPipe);
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
    pPipe->setRootNode(pP2ANode);

    return MTRUE;
}

MBOOL
DepthMapFlowOption_VSDOF::
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
DepthMapFlowOption_VSDOF::
config3ATuningMeta(
    sp<DepthMapEffectRequest> pRequest,
    StereoP2Path path,
    MetaSet_T& rMetaSet
)
{
    // for P2ANode
    // YUV input when capture scenaroi -> no need setIsp
    trySetMetadata<MUINT8>(&rMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Preview);

    return MTRUE;
}

MBOOL
DepthMapFlowOption_VSDOF::
config3ATuningMeta_Bayer(
    sp<DepthMapEffectRequest> pRequest,
    MetaSet_T& rMetaSet
)
{
    EIspProfile_T profile = getIspProfile_Bayer(pRequest);
    trySetMetadata<MUINT8>(&rMetaSet.halMeta, MTK_3A_ISP_PROFILE, profile);
    return MTRUE;
}

EIspProfile_T
DepthMapFlowOption_VSDOF::
getIspProfile_Bayer(sp<DepthMapEffectRequest> pRequest)
{
    // only for preview scenaios, no capture
    if(mpPipeOption->mSensorType == v1::Stereo::BAYER_AND_BAYER)
        return EIspProfile_N3D_Preview;
    else if(mpPipeOption->mSensorType == v1::Stereo::BAYER_AND_MONO)
        return EIspProfile_N3D_Preview_toW;
    else
    {
        MY_LOGE("Unsupport sensor type = %d", mpPipeOption->mSensorType);
        return EIspProfile_N3D_Preview;
    }
}

DepthMapBufferID
DepthMapFlowOption_VSDOF::
reMapBufferID(
    const EffectRequestAttrs& reqAttr,
    DepthMapBufferID bufferID
)
{
    return bufferID;
}

IImageBuffer*
DepthMapFlowOption_VSDOF::
get3DNRVIPIBuffer()
{
    return DepthQTemplateProvider::get3DNRVIPIBuffer();
}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
