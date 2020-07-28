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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <libion_mtk/include/ion.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "P2ANode.h"
#include "../DepthMapPipe_Common.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

// Logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "P2A_Node"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_P2A);

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe::NSPostProc;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
P2ANode::
P2ANode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, DataSequential<DepthMapRequestPtr>(reqNoGetter, "P2ANode")
, miSensorIdx_Main1(config.mpSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(config.mpSetting->miSensorIdx_Main2)
{
    this->addWaitQueue(&mRequestQue);
}

P2ANode::
~P2ANode()
{
    MY_LOGD("[Destructor]");
}

MVOID
P2ANode::
cleanUp()
{
    MY_LOGD("+");
    if(mpINormalStream != NULL)
    {
        mpINormalStream->uninit("VSDOF_P2A");
        mpINormalStream->destroyInstance();
        mpINormalStream = NULL;
    }

    if(mpIspHal_Main1)
    {
        mpIspHal_Main1->destroyInstance("VSDOF_3A_MAIN1");
        mpIspHal_Main1 = NULL;
    }

    if(mpIspHal_Main2)
    {
        mpIspHal_Main2->destroyInstance("VSDOF_3A_MAIN2");
        mpIspHal_Main2 = NULL;
    }

    MY_LOGD("-");
}

MBOOL
P2ANode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ANode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ANode::
onThreadStart()
{
    CAM_ULOGM_TAGLIFE("P2ANode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // Create NormalStream
    VSDOF_LOGD("NormalStream create instance: idx=%d", miSensorIdx_Main1);
    CAM_ULOGM_TAG_BEGIN("P2ANode::NormalStream::createInstance+init");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);

    if (mpINormalStream == NULL)
    {
        MY_LOGE("mpINormalStream create instance for P2A Node failed!");
        cleanUp();
        return MFALSE;
    }
    mpINormalStream->init("VSDOF_P2A");
    CAM_ULOGM_TAG_END();
    // 3A: create instance
    // UT does not test 3A
    CAM_ULOGM_TAG_BEGIN("P2ANode::create_3A_instance");
    #ifndef GTEST
    mpIspHal_Main1 = MAKE_HalISP(miSensorIdx_Main1, "VSDOF_3A_MAIN1");
    mpIspHal_Main2 = MAKE_HalISP(miSensorIdx_Main2, "VSDOF_3A_MAIN2");
    MY_LOGD("ISP Hal create instance, Main1: %x Main2: %x", mpIspHal_Main1, mpIspHal_Main2);
    #endif
    CAM_ULOGM_TAG_END();

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ANode::
onThreadStop()
{
    CAM_ULOGM_TAGLIFE("P2ANode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ANode::
onData(DataID data, DepthMapRequestPtr &request)
{
  MBOOL ret = MTRUE;
  VSDOF_LOGD("+ : reqID=%d", request->getRequestNo());
  CAM_ULOGM_TAGLIFE("P2ANode::onData");

  switch(data)
  {
  case ROOT_ENQUE:
    VSDOF_PRFLOG("+ : reqID=%d size=%d", request->getRequestNo(), mRequestQue.size());
    mRequestQue.enque(request);
    // notify N3D to do padding & matrix
    if(request->getRequestAttr().opState == eSTATE_NORMAL)
        this->handleData(P2A_TO_N3D_PADDING_MATRIX, request);
    // dump mapping
    this->dumpSensorMapping(request);
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
P2ANode::
perform3AIspTuning(
    DepthMapRequestPtr& pRequest,
    Stereo3ATuningRes& rOutTuningRes
)
{
#ifdef GTEST
    return MTRUE;
#endif
    pRequest->mTimer.startP2ASetIsp();
    // preview only need main2 tuning, capture no needs setIsp
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
        rOutTuningRes.tuningRes_main2 = applyISPTuning(pRequest, eP2APATH_MAIN2);
        rOutTuningRes.tuningRes_FE_main2 = applyISPTuning(pRequest, eP2APATH_FE_MAIN2);;
    }
    else if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
        {
        rOutTuningRes.tuningRes_FE_main1 = applyISPTuning(pRequest, eP2APATH_FE_MAIN1);;
        rOutTuningRes.tuningRes_FE_main2 = applyISPTuning(pRequest, eP2APATH_FE_MAIN2);;
    }
    pRequest->mTimer.stopP2ASetIsp();
    return MTRUE;
}

AAATuningResult
P2ANode::
applyISPTuning(
    DepthMapRequestPtr& pRequest,
    StereoP2Path p2aPath
)
{
    CAM_ULOGM_TAGLIFE("P2ANode::applyISPTuning");
    MBOOL bIsMain1Path = (p2aPath == eP2APATH_MAIN1 || p2aPath == eP2APATH_FE_MAIN1);
    VSDOF_PRFTIME_LOG("+, reqID=%d bIsMain1Path=%d", pRequest->getRequestNo(), bIsMain1Path);
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // get in/out APP/HAL meta
    DepthMapBufferID halMetaBID = bIsMain1Path ? BID_META_IN_HAL_MAIN1 : BID_META_IN_HAL_MAIN2;
    IMetadata* pMeta_InApp  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    IMetadata* pMeta_InHal  = pBufferHandler->requestMetadata(getNodeId(), halMetaBID);
    IMetadata* pMeta_OutApp = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_APP);
    IMetadata* pMeta_OutHal = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL);
    // get tuning buf
    IImageBuffer* pTuningImgBuf = nullptr;
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(getNodeId(), BID_P2A_TUNING);
    // in/out meta set
    MetaSet_T inMetaSet(*pMeta_InApp, *pMeta_InHal);
    MetaSet_T outMetaSet(*pMeta_OutApp, *pMeta_OutHal);
    // check raw type
    INPUT_RAW_TYPE rawType = mpFlowOption->getInputRawType(pRequest, p2aPath);
    if(rawType == eRESIZE_RAW)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
    // manully tuning for current flow option
    mpFlowOption->config3ATuningMeta(pRequest, p2aPath, inMetaSet);
    // Main1 path need LCE when preview
    MVOID* pLcsBuf = nullptr;
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL && bIsMain1Path)
        pLcsBuf= (void*)pBufferHandler->requestBuffer(getNodeId(), PBID_IN_LCSO1);
    else
        updateEntry<MINT32>(&inMetaSet.halMeta, MTK_3A_ISP_BYPASS_LCE, true);
    // config meta for FE path
    if(p2aPath == eP2APATH_FE_MAIN1 || p2aPath == eP2APATH_FE_MAIN2)
    {
        updateEntry<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, ISP_TUNING_FE_PASS_MODE);
    }
    // UT do not test setP2Isp
    AAATuningResult result(pTuningBuf, pLcsBuf);
    #ifndef GTEST
    IHalISP* p3AHAL = bIsMain1Path ? mpIspHal_Main1 : mpIspHal_Main2;
    p3AHAL->setP2Isp(0, inMetaSet, &result.tuningResult, &outMetaSet);
    #endif
    VSDOF_PRFTIME_LOG("-, reqID=%d", pRequest->getRequestNo());
    return result;
}


MBOOL
P2ANode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mRequestQue.deque(pRequest) )
    {
        MY_LOGE("mRequestQue.deque() failed");
        return MFALSE;
    }

    CAM_ULOGM_TAGLIFE("P2ANode::onThreadLoop");
    // mark on-going-pRequest start
    this->incExtThreadDependency();
    const EffectRequestAttrs& attr = pRequest->getRequestAttr();
    VSDOF_LOGD("threadLoop start, reqID=%d eState=%d needFEFM=%d",
                    pRequest->getRequestNo(), attr.opState, attr.needFEFM);
    // standalone request -> skip P2ANode
    if(attr.opState == eSTATE_STANDALONE)
    {
        SequenceOPState state = this->sequentialize(pRequest);
        if (state == SEQ_DISABLE)
        {
            VSDOF_LOGD("Seq: No need to sequential, reqID=%d", pRequest->getRequestNo());
            this->handleData(BAYER_ENQUE, pRequest);
        }
        else
            VSDOF_LOGD("Seq: push sequential done, reqID=%d", pRequest->getRequestNo());
        this->decExtThreadDependency();
        return MTRUE;
    }
    // overall P2ANode timer
    pRequest->mTimer.startP2A();
    // enque QParams
    QParams enqueParams;
    // enque cookie instance
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(pRequest, this);
    // apply 3A Isp tuning
    Stereo3ATuningRes tuningRes;
    //
    MBOOL bRet = MTRUE;
    // apply ISP
    if(!perform3AIspTuning(pRequest, tuningRes))
        goto lbExit;

    // call flow option to build QParams
    bRet = mpFlowOption->buildQParam(pRequest, tuningRes, enqueParams);
    // debug param
    debugQParams(enqueParams);
    if(!bRet)
    {
        AEE_ASSERT("[P2ANode]Failed to build P2 enque parameters.");
        goto lbExit;
    }
    if(attr.needFEFM == 1)
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        IImageBuffer *tempB = nullptr, *tempC = nullptr;
        pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2BO, tempB);
        pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2CO, tempC);
        MY_LOGD("reqID=%d, P2 Enque, P2CB BID_P2A_OUT_FE2BO:%p, BID_P2A_OUT_FE2CO:%p", pRequest->getRequestNo(), tempB, tempC);
    }
    // callback
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // only preview needs sequential: mark on going
    if(pRequest->getRequestAttr().opState != eSTATE_CAPTURE)
        this->markOnGoingData(pRequest);
    // enque
    CAM_ULOGM_TAG_BEGIN("P2ANode::NormalStream::enque");
    VSDOF_LOGD("mpINormalStream enque start! reqID=%d", pRequest->getRequestNo());
    pRequest->mTimer.startP2ADrv();
    bRet = mpINormalStream->enque(enqueParams);
    CAM_ULOGM_TAG_END();
    VSDOF_LOGD("mpINormalStream enque end! reqID=%d", pRequest->getRequestNo());
    if(!bRet)
    {
        AEE_ASSERT("[P2ANode] NormalStream enque failed");
    }
    return MTRUE;

lbExit:
    delete pCookieIns;
    // mark on-going-request end
    this->decExtThreadDependency();
    return MFALSE;

}

MVOID
P2ANode::
onP2Callback(QParams& rParams)
{
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    P2ANode* pP2ANode = (P2ANode*) (pEnqueData->mpNode);
    pP2ANode->handleP2Done(rParams, pEnqueData);
}

MVOID
P2ANode::
handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_ULOGM_TAGLIFE("P2ANode::handleP2Done");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    // stop timer
    pRequest->mTimer.stopP2ADrv();
    DumpConfig config;
    VSDOF_LOGD("reqID=%d , p2 exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ADrv());
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // dump buffer first
    config = DumpConfig(NULL, ".raw", MTRUE);
    this->handleDump(TO_DUMP_RAWS, pRequest, &config);
    config = DumpConfig(NULL, NULL, MFALSE);
    this->handleDump(TO_DUMP_BUFFERS, pRequest, &config);
    if(pRequest->getRequestAttr().needFEFM)
        this->handleDump(TO_DUMP_IMG3O, pRequest);
    //
    mpFlowOption->onP2ProcessDone(this, pRequest);
    this->configureToNext(pRequest);
lbExit:
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    delete pEnqueCookie;
    VSDOF_LOGD("- :reqID=%d", pRequest->getRequestNo());
    // only preview need sequential
    if(pRequest->getRequestAttr().opState != eSTATE_CAPTURE)
        this->onHandleOnGoingReqReady(pRequest->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();
    // stop overall timer
    pRequest->mTimer.stopP2A();
}


MBOOL
P2ANode::
onHandleOnGoingReqReady(
    MUINT32 iReqID
)
{
    // sequential: mark finish
    Vector<DepthMapRequestPtr> popReqVec;
    this->markFinishAndPop(iReqID, popReqVec);
    // pass to P2ABayer node
    for(size_t idx = 0;idx<popReqVec.size();++idx)
    {
        DepthMapRequestPtr pReq = popReqVec.itemAt(idx);
        VSDOF_LOGD("Seq: free the reqID=%d", pReq->getRequestNo());
        this->handleData(BAYER_ENQUE, pReq);

    }
    return MTRUE;
}


MBOOL
P2ANode::
configureToNext(DepthMapRequestPtr pRequest)
{
    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // if do depth
    if(!pRequest->isSkipDepth(mpPipeOption))
    {
        if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
        {
            // FEO
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE1BO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE2BO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE1CO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE2CO, eDPETHMAP_PIPE_NODEID_N3D);
            // FMO
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMBO_LR, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMBO_RL, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMCO_LR, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMCO_RL, eDPETHMAP_PIPE_NODEID_N3D);
            // Rect_in2
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_N3D);
            // MV_Y
            pBufferHandler->configOutBuffer(getNodeId(), BID_N3D_OUT_MV_Y, eDPETHMAP_PIPE_NODEID_DLDEPTH);
            // pass to N3D
            this->handleDataAndDump(P2A_TO_N3D_CAP_RECT2, pRequest);
        }
        else
        {
            // config output buffers for N3D input
            if(pRequest->getRequestAttr().needFEFM)
            {
                IImageBuffer *tempB = nullptr, *tempC = nullptr;
                pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2BO, tempB);
                pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FE2CO, tempC);
                MY_LOGD("reqID=%d, P2 CB, BID_P2A_OUT_FE2BO:%p, BID_P2A_OUT_FE2CO:%p", pRequest->getRequestNo(), tempB, tempC);
                // FEO - send to N3d for learning/P2ABayer for FM
                pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE2BO, eDPETHMAP_PIPE_NODEID_P2ABAYER);
                pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE2CO, eDPETHMAP_PIPE_NODEID_P2ABAYER);
                // Rect_in
                pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_N3D);
                // pass to N3D
                this->handleDataAndDump(P2A_TO_N3D_RECT2_FEO, pRequest);
                if(!pRequest->isRequestBuffer(BID_P2A_OUT_FDIMG))
                    pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_INTERNAL_FD, eDPETHMAP_PIPE_NODEID_P2ABAYER);
            }
            else
            {
                // only Rect_in2
                pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_N3D);
                // pass to N3D
                this->handleDataAndDump(P2A_TO_N3D_NOFEFM_RECT2, pRequest);
            }
        }
    }
    // enque request into P2ABayer Node
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
        this->handleData(BAYER_ENQUE, pRequest);

    VSDOF_LOGD("-, reqID=%d", pRequest->getRequestNo());
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2ANode::
onFlush()
{
    MY_LOGD("+ extDep=%d", this->getExtThreadDependency());
    DepthMapRequestPtr pRequest;
    while( mRequestQue.deque(pRequest) )
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

