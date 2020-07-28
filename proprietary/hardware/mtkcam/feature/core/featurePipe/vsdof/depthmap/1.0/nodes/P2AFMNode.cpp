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
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <libion_mtk/include/ion.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Module header file
#include <stereo_tuning_provider.h>
// Local header file
#include "P2AFMNode.h"
#include "../DepthMapPipe_Common.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

// Logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "P2A_FMNode"
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
P2AFMNode::
P2AFMNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, DataSequential<DepthMapRequestPtr>(reqNoGetter, "P2AFMSeq")
, miSensorIdx_Main1(config.mpSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(config.mpSetting->miSensorIdx_Main2)
{
    this->addWaitQueue(&mRequestQue);
}

P2AFMNode::
~P2AFMNode()
{
    MY_LOGD("[Destructor]");
}

MVOID
P2AFMNode::
cleanUp()
{
    MY_LOGD("+");
    if(mpINormalStream != NULL)
    {
        mpINormalStream->uninit("VSDOF_P2AFM");
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

    MY_LOGD("-");
}

MBOOL
P2AFMNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2AFMNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2AFMNode::
onThreadStart()
{
    CAM_TRACE_NAME("P2AFMNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // Create NormalStream
    VSDOF_LOGD("NormalStream create instance: idx=%d", miSensorIdx_Main1);
    CAM_TRACE_BEGIN("P2AFMNode::NormalStream::createInstance+init");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);

    if (mpINormalStream == NULL)
    {
        MY_LOGE("mpINormalStream create instance for P2AFM Node failed!");
        cleanUp();
        return MFALSE;
    }
    mpINormalStream->init("VSDOF_P2AFM");
    CAM_TRACE_END();

    // 3A: create instance
    // UT does not test 3A
    CAM_TRACE_BEGIN("P2AFMNode::create_3A_instance");
    #ifndef GTEST
    mp3AHal_Main1 = MAKE_Hal3A(miSensorIdx_Main1, "VSDOF_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(miSensorIdx_Main2, "VSDOF_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x Main2: %x", mp3AHal_Main1, mp3AHal_Main2);
    #endif
    CAM_TRACE_END();

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2AFMNode::
onThreadStop()
{
    CAM_TRACE_NAME("P2AFMNode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2AFMNode::
onData(DataID data, DepthMapRequestPtr &request)
{
  MBOOL ret = MTRUE;
  VSDOF_LOGD("+ : reqID=%d", request->getRequestNo());
  CAM_TRACE_NAME("P2AFMNode::onData");

  switch(data)
  {
  case ROOT_ENQUE:
    VSDOF_PRFLOG("+ : reqID=%d size=%d", request->getRequestNo(), mRequestQue.size());
    mRequestQue.enque(request);
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
P2AFMNode::
perform3AIspTuning(
    DepthMapRequestPtr& rpRequest,
    Stereo3ATuningRes& rOutTuningRes
)
{
    rOutTuningRes.tuningRes_main1 = applyISPTuning(rpRequest, eP2APATH_MAIN1);
    rOutTuningRes.tuningRes_main2 = applyISPTuning(rpRequest, eP2APATH_MAIN2);
    return MTRUE;
}

AAATuningResult
P2AFMNode::
applyISPTuning(
    DepthMapRequestPtr& rpRequest,
    StereoP2Path p2aPath
)
{
    CAM_TRACE_NAME("P2AFMNode::applyISPTuning");
    MBOOL bIsMain1Path = (p2aPath == eP2APATH_MAIN1);
    VSDOF_LOGD("+, reqID=%d bIsMain1Path=%d", rpRequest->getRequestNo(), bIsMain1Path);
    sp<BaseBufferHandler> pBufferHandler = rpRequest->getBufferHandler();
    // get in/out APP/HAL meta
    DepthMapBufferID halMetaBID = bIsMain1Path ? BID_META_IN_HAL_MAIN1 : BID_META_IN_HAL_MAIN2;
    IMetadata* pMeta_InApp  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    IMetadata* pMeta_InHal  = pBufferHandler->requestMetadata(getNodeId(), halMetaBID);
    IMetadata* pMeta_OutApp = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_APP);
    IMetadata* pMeta_OutHal = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL);
    // get tuning buf
    IImageBuffer* pTuningImgBuf = nullptr;
    const NodeBufferSetting& setting = {BID_P2A_OUT_TUNING_MAIN1, eBUFFER_IOTYPE_OUTPUT};
    MVOID* pTuningBuf = nullptr;
    if(!mpPipeOption->mbEnableLCE
        && rpRequest->getRequestImageBuffer(setting, pTuningImgBuf)
        && p2aPath == eP2APATH_MAIN1)
    {
        pTuningBuf = (MVOID*) pTuningImgBuf->getBufVA(0);
        memset(pTuningBuf, 0 , INormalStream::getRegTableSize());
    }
    else
        pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    // in/out meta set
    MetaSet_T inMetaSet(*pMeta_InApp, *pMeta_InHal);
    // check raw type
    INPUT_RAW_TYPE rawType = mpFlowOption->getInputRawType(rpRequest, p2aPath);
    if(rawType == eRESIZE_RAW)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
    // manully tuning for current flow option
    mpFlowOption->config3ATuningMeta(rpRequest, p2aPath, inMetaSet);
    // UT do not test setIsp
    #ifndef GTEST
    IHal3A* p3AHAL = bIsMain1Path ? mp3AHal_Main1 : mp3AHal_Main2;
    // 3A result
    AAATuningResult result(pTuningBuf);
    p3AHAL->setIsp(0, inMetaSet, &result.tuningResult, NULL);
    #endif
    VSDOF_LOGD("-, reqID=%d", rpRequest->getRequestNo());
    return result;
}

MBOOL
P2AFMNode::
onThreadLoop()
{
    DepthMapRequestPtr request;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mRequestQue.deque(request) )
    {
        MY_LOGE("mRequestQue.deque() failed");
        return MFALSE;
    }

    CAM_TRACE_NAME("P2AFMNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();
    VSDOF_PRFLOG("threadLoop start, reqID=%d eState=%d",
                    request->getRequestNo(), request->getRequestAttr().opState);

    // enque QParams
    QParams enqueParams;
    // enque cookie instance
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(request, this);

    // apply 3A Isp tuning
    Stereo3ATuningRes tuningRes;
    MBOOL bRet = perform3AIspTuning(request, tuningRes);
    if(!bRet)
        goto lbExit;

    // call flow option to build QParams
    bRet = mpFlowOption->buildQParam(request, tuningRes, enqueParams);
    // debug param
    debugQParams(enqueParams);
    if(!bRet)
    {
        MY_LOGE("Failed to build P2 enque parametes.");
        goto lbExit;
    }
    // callback
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpfnEnQFailCallback = onP2FailedCallback;
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // start P2A timer
    request->mTimer.startP2AFM();
    request->mTimer.startP2AFMEnque();
    // sequential: mark on going
    this->markOnGoingData(request);
    // enque
    CAM_TRACE_BEGIN("P2AFMNode::NormalStream::enque");
    VSDOF_LOGD("mpINormalStream enque start! reqID=%d", request->getRequestNo());
    bRet = mpINormalStream->enque(enqueParams);
    CAM_TRACE_END();
    // stop P2A Enque timer
    request->mTimer.stopP2AFMEnque();
    VSDOF_PRFTIME_LOG("mpINormalStream enque end! reqID=%d, exec-time(enque)=%d msec",
                    request->getRequestNo(), request->mTimer.getElapsedP2AFMEnque());
    if(!bRet)
    {
        MY_LOGE("mpINormalStream enque failed! reqID=%d", request->getRequestNo());
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
P2AFMNode::
onP2Callback(QParams& rParams)
{
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    P2AFMNode* pP2AFMNode = (P2AFMNode*) (pEnqueData->mpNode);
    pP2AFMNode->handleP2Done(rParams, pEnqueData);
}

MVOID
P2AFMNode::
onP2FailedCallback(QParams& rParams)
{
    MY_LOGD("+");
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    P2AFMNode* pP2AFMNode = (P2AFMNode*) (pEnqueData->mpNode);
    MUINT32 iReqNo = pEnqueData->mRequest->getRequestNo();
    //
    MY_LOGE("reqID=%d P2A operations failed!!Check the following log:", iReqNo);
    debugQParams(rParams);
    pP2AFMNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mRequest);
    // launch onProcessDone
    pEnqueData->mRequest->getBufferHandler()->onProcessDone(pP2AFMNode->getNodeId());
    delete pEnqueData;
    // mark on-going-request end
    pP2AFMNode->decExtThreadDependency();
    MY_LOGD("-");
}

MVOID
P2AFMNode::
handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_TRACE_NAME("P2AFMNode::handleP2Done");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    DumpConfig config;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // stop timer
    pRequest->mTimer.stopP2AFM();
    VSDOF_PRFTIME_LOG("+ :reqID=%d , p2 exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2AFM());
    // dump buffer first
    config = DumpConfig(NULL, ".raw", MTRUE);
    this->handleDump(TO_DUMP_RAWS, pRequest, &config);
    config = DumpConfig(NULL, NULL, MFALSE);
    this->handleDump(TO_DUMP_BUFFERS, pRequest, &config);
    // launch flow option p2 done
    mpFlowOption->onP2ProcessDone(this, pRequest);
    // handle flow type task
    if(!onHandleFlowTypeP2Done(pRequest))
    {
        MY_LOGE("onHandleFlowTypeP2Done failed!");
        return;
    }

lbExit:
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    delete pEnqueCookie;
    VSDOF_LOGD("- :reqID=%d", pRequest->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2AFMNode::
onHandleFlowTypeP2Done(
    sp<DepthMapEffectRequest> pRequest
)
{
    if(pRequest->isQueuedDepthRequest(mpPipeOption))
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        IMetadata* pOutHalMeta = nullptr;
        // get queue depth info
        DepthBufferInfo depthInfo;
        if(mpDepthStorage->pop_front(depthInfo)) {
            mLastQueueDepthInfo = depthInfo;
            VSDOF_LOGD("reqID=%d use queued depth info!",
                        pRequest->getRequestNo());
        }else
        {
            depthInfo = mLastQueueDepthInfo;
            VSDOF_LOGD("reqID=%d, no queued depth, use last depth info!",
                        pRequest->getRequestNo());
        }

        // check depthmap is not ready, notify and return
        if(depthInfo.mpDepthBuffer.get() == nullptr &&
            mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF)
        {
            VSDOF_LOGD("reqID=%d, depth not ready!", pRequest->getRequestNo());
            handleData(REQUEST_DEPTH_NOT_READY, pRequest);
            goto lbExit;
        }

        // config queued Depth buffer as output buffer to GF Node
        pBufferHandler->configExternalOutBuffer(getNodeId(), BID_WMF_OUT_DMW,
                                                eDPETHMAP_PIPE_NODEID_GF, depthInfo.mpDepthBuffer);
        // config MY_S buffer
        pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_GF);
        // write meta
        {
            pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), BID_META_OUT_HAL);
            trySetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, depthInfo.mfConvOffset);

            IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
            trySetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, depthInfo.magicNumber);
        }
        // set meta ready
        pRequest->setOutputBufferReady(BID_META_OUT_APP);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL);
        pRequest->setOutputBufferReady(BID_META_OUT_APP_QUEUED);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL_QUEUED);
        // pass to GF
        handleDataAndDump(P2A_TO_GF_DMW_MYS, pRequest);
    }

lbExit:
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2AFMNode::
onFlush()
{
    MY_LOGD("+");
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

