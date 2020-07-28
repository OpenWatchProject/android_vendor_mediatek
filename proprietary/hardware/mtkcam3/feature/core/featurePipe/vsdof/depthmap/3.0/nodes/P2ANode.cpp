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
, NR3DCommon()
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

    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("VSDOF_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }

    if(mp3AHal_Main2)
    {
        mp3AHal_Main2->destroyInstance("VSDOF_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }

     // nr3d
    if(!NR3DCommon::uninit())
    {
        MY_LOGE("Failed to uninit NR3D.");
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
    mp3AHal_Main1 = MAKE_Hal3A(miSensorIdx_Main1, "VSDOF_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(miSensorIdx_Main2, "VSDOF_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %p Main2: %p", mp3AHal_Main1, mp3AHal_Main2);
    #endif
    CAM_ULOGM_TAG_END();
    // nr3d init function
    if(!NR3DCommon::init(miSensorIdx_Main1))
    {
        MY_LOGE("Failed to init NR3D.");
        //return MFALSE;
    }
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
    VSDOF_PRFLOG("+ : reqID=%d size=%zu", request->getRequestNo(), mRequestQue.size());
    mRequestQue.enque(request);
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
    // preview only need main2 tuning
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL &&
        !pRequest->getRequestAttr().needFEFM)
        rOutTuningRes.tuningRes_main2 = applyISPTuning(pRequest, eP2APATH_MAIN2);
    else
    {
        rOutTuningRes.tuningRes_main1 = applyISPTuning(pRequest, eP2APATH_MAIN1);
        rOutTuningRes.tuningRes_main2 = applyISPTuning(pRequest, eP2APATH_MAIN2);
    }
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
    MBOOL bIsMain1Path = (p2aPath == eP2APATH_MAIN1);
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
    // UT do not test setIsp
    AAATuningResult result(pTuningBuf, pLcsBuf);
    #ifndef GTEST
    IHal3A* p3AHAL = bIsMain1Path ? mp3AHal_Main1 : mp3AHal_Main2;
    p3AHAL->setIsp(0, inMetaSet, &result.tuningResult, &outMetaSet);
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
    VSDOF_PRFLOG("threadLoop start, reqID=%d eState=%d needFEFM=%d",
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
    // enque QParams
    QParams enqueParams;
    // enque cookie instance
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(pRequest, this);
    // apply 3A Isp tuning
    Stereo3ATuningRes tuningRes;
    MBOOL bRet = MTRUE;
    // when need FEFM -> enale nr3d in P2ANode
    if(attr.opState == eSTATE_NORMAL && attr.needFEFM)
        bRet &= setup3DNRMeta(getNodeId(), pRequest);
    // apply ISP
    bRet &= perform3AIspTuning(pRequest, tuningRes);
    // when need FEFM -> enale nr3d in P2ANode
    if(attr.opState == eSTATE_NORMAL && attr.needFEFM)
    {
        bRet &= perform3dnr(pRequest, mp3AHal_Main1, mpFlowOption, tuningRes);
    }
    if(!bRet)
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
    // callback
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpfnEnQFailCallback = onP2FailedCallback;
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // start P2A timer
    pRequest->mTimer.startP2A();
    pRequest->mTimer.startP2AEnque();
    // sequential: mark on going
    this->markOnGoingData(pRequest);
    // enque
    CAM_ULOGM_TAG_BEGIN("P2ANode::NormalStream::enque");
    VSDOF_LOGD("mpINormalStream enque start! reqID=%d", pRequest->getRequestNo());
    bRet = mpINormalStream->enque(enqueParams);
    CAM_ULOGM_TAG_END();
    // stop P2A Enque timer
    pRequest->mTimer.stopP2AEnque();
    VSDOF_PRFTIME_LOG("mpINormalStream enque end! reqID=%d, exec-time(enque)=%d msec",
                    pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2AEnque());
    if(!bRet)
    {
        MY_LOGE("mpINormalStream enque failed! reqID=%d", pRequest->getRequestNo());
        goto lbExit;
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
onP2FailedCallback(QParams& rParams)
{
    MY_LOGD("+");
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    P2ANode* pP2ANode = (P2ANode*) (pEnqueData->mpNode);
    MUINT32 iReqNo = pEnqueData->mRequest->getRequestNo();
    //
    MY_LOGE("reqID=%d P2A operations failed!!Check the following log:", iReqNo);
    debugQParams(rParams);
    pP2ANode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mRequest);
    // launch onProcessDone
    pEnqueData->mRequest->getBufferHandler()->onProcessDone(pP2ANode->getNodeId());
    delete pEnqueData;
    pP2ANode->onHandleOnGoingReqReady(iReqNo);
    // mark on-going-request end
    pP2ANode->decExtThreadDependency();
    MY_LOGD("-");
}

MVOID
P2ANode::
handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_ULOGM_TAGLIFE("P2ANode::handleP2Done");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    DumpConfig config;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // stop timer
    pRequest->mTimer.stopP2A();
    VSDOF_PRFTIME_LOG("+ :reqID=%d , p2 exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2A());
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
    this->onHandleOnGoingReqReady(pRequest->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();
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
    // mark preview frame output this
    if(eSTATE_NORMAL == pRequest->getRequestAttr().opState &&
        pRequest->getRequestAttr().needFEFM)
    {
        // FD
        if(pRequest->setOutputBufferReady(BID_P2A_OUT_FDIMG))
            this->handleDataAndDump(P2A_OUT_FD, pRequest);
        // MV_F
        if(pRequest->setOutputBufferReady(BID_P2A_OUT_MV_F))
            this->handleDataAndDump(P2A_OUT_MV_F, pRequest);
    }
    // config FD output to P2ABayer when needFEFM, which is the input of BayerNode
    if(pRequest->getRequestAttr().needFEFM && !pRequest->isRequestBuffer(BID_P2A_OUT_FDIMG))
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_INTERNAL_FD, eDPETHMAP_PIPE_NODEID_P2ABAYER);
    // enque request into P2ABayer Node
    this->handleData(BAYER_ENQUE, pRequest);

    // if do depth
    if(!pRequest->isSkipDepth(mpPipeOption))
    {
        // config output buffers for N3D input
        if(pRequest->getRequestAttr().needFEFM)
        {
            // FEO
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FE1BO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FE2BO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FE1CO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FE2CO, eDPETHMAP_PIPE_NODEID_N3D);
            // FMO
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FMBO_LR, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FMBO_RL, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FMCO_LR, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_FMCO_RL, eDPETHMAP_PIPE_NODEID_N3D);
            // CC_in
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_CC_IN1, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_CC_IN2, eDPETHMAP_PIPE_NODEID_N3D);
            // Rect_in
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_RECT_IN1, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_N3D);
            // pass to N3D
            this->handleDataAndDump(P2A_TO_N3D_FEFM_CCin, pRequest);
        }
        else
        {
            // only Rect_in2
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2A, BID_P2A_OUT_RECT_IN2, eDPETHMAP_PIPE_NODEID_N3D);
            // pass to N3D
            this->handleDataAndDump(P2A_TO_N3D_NOFEFM_RECT2, pRequest);
        }
    }

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

