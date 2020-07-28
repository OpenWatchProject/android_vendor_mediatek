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
#include <cmath>
// Android system/core header file
#include <sync/sync.h>
// mtkcam custom header file
#include <camera_custom_stereo.h>
// mtkcam global header file
#include <isp_tuning.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <libion_mtk/include/ion.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Module header file

// Local header file
#include "P2ABayerNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeUtils.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

// Logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "P2ABayerNode"
#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/vsdof/util/vsdof_util.h>
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
P2ABayerNode::
P2ABayerNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, NR3DCommon()
, DataSequential<DepthMapRequestPtr>(reqNoGetter, "P2ABayerSeq")
, miSensorIdx_Main1(config.mpSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(config.mpSetting->miSensorIdx_Main2)
{
    this->addWaitQueue(&mRequestQue);
}

P2ABayerNode::
~P2ABayerNode()
{
    MY_LOGD("[Destructor]");
}

MVOID
P2ABayerNode::
cleanUp()
{
    MY_LOGD("+");
    if(mpINormalStream != nullptr)
    {
        mpINormalStream->uninit(getName());
        mpINormalStream->destroyInstance();
        mpINormalStream = nullptr;
    }

    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance(getName());
        mp3AHal_Main1 = nullptr;
    }

    if(mpDpIspStream != nullptr)
        delete mpDpIspStream;

    MY_LOGD("-");
}

MBOOL
P2ABayerNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ABayerNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ABayerNode::
onThreadStart()
{
    CAM_TRACE_NAME("P2ABayerNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // Create NormalStream
    VSDOF_LOGD("NormalStream create instance: idx=%d", miSensorIdx_Main1);
    CAM_TRACE_BEGIN("P2ABayerNode::NormalStream::createInstance+init");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);

    if (mpINormalStream == nullptr)
    {
        MY_LOGE("mpINormalStream create instance for P2A Node failed!");
        cleanUp();
        return MFALSE;
    }
    mpINormalStream->init(getName());
    CAM_TRACE_END();
    // 3A: create instance
    // UT does not test 3A
    CAM_TRACE_BEGIN("P2ABayerNode::create_3A_instance");
    #ifndef GTEST
    mp3AHal_Main1 = MAKE_Hal3A(miSensorIdx_Main1, getName());
    MY_LOGD("3A create instance, Main1: %x", mp3AHal_Main1);
    #endif
    // create MDP stream
    mpDpIspStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    CAM_TRACE_END();
    // nr3d
    if(!NR3DCommon::init(miSensorIdx_Main1))
    {
        MY_LOGE("Failed to init NR3D.");
        //return MFALSE;
    }
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ABayerNode::
onThreadStop()
{
    CAM_TRACE_NAME("P2ABayerNode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ABayerNode::
onData(DataID data, DepthMapRequestPtr &request)
{
  MBOOL ret = MTRUE;
  VSDOF_LOGD("+ : reqID=%d", request->getRequestNo());
  CAM_TRACE_NAME("P2ABayerNode::onData");

  switch(data)
  {
  case BAYER_ENQUE:
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

AAATuningResult
P2ABayerNode::
applyISPTuning(
    DepthMapRequestPtr& rpRequest
)
{
    CAM_TRACE_NAME("P2ABayerNode::applyISPTuning");
    VSDOF_PRFTIME_LOG("+, reqID=%d", rpRequest->getRequestNo());

    sp<BaseBufferHandler> pBufferHandler = rpRequest->getBufferHandler();
    // get in/out APP/HAL meta
    IMetadata* pMeta_InApp  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    IMetadata* pMeta_InHal  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
    IMetadata* pMeta_OutApp  = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_APP);
    IMetadata* pMeta_OutHal = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL);
    // P2ABayer default enable LCEI
    MVOID* pLcsBuf = (void*)pBufferHandler->requestBuffer(getNodeId(), PBID_IN_LCSO1);;
    // get tuning buf
    IImageBuffer* pTuningImgBuf = nullptr;
    MVOID* pTuningBuf = pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    // in/out meta set
    MetaSet_T inMetaSet(*pMeta_InApp, *pMeta_InHal);
    MetaSet_T outMetaSet(*pMeta_OutApp, *pMeta_OutHal);
    // get raw type
    INPUT_RAW_TYPE rawType = mpFlowOption->getInputRawType(rpRequest, eP2APATH_MAIN1_BAYER);
    // USE resize raw-->set PGN 0
    if(rawType == eRESIZE_RAW)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
    // config profile
    mpFlowOption->config3ATuningMeta_Bayer(rpRequest, inMetaSet);
    // UT do not test setIsp
    AAATuningResult result(pTuningBuf, pLcsBuf);
    #ifndef GTEST
    //
    mp3AHal_Main1->setIsp(0, inMetaSet, &result.tuningResult, &outMetaSet);
    // only FULLRAW(capture) need to get exif result
    if(rawType == eFULLSIZE_RAW)
    {
        *pMeta_OutApp += outMetaSet.appMeta;
        *pMeta_OutHal += outMetaSet.halMeta;
        // Get standard EXIF info from input HAL metadata and set it to output HAL
        IMetadata exifMeta;
        if( tryGetMetadata<IMetadata>(pMeta_InHal, MTK_3A_EXIF_METADATA, exifMeta) ) {
            trySetMetadata<IMetadata>(pMeta_OutHal, MTK_3A_EXIF_METADATA, exifMeta);
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    #endif
    VSDOF_PRFTIME_LOG("-, reqID=%d", rpRequest->getRequestNo());
    return result;
}

MBOOL
P2ABayerNode::
perform3AIspTuning(
    DepthMapRequestPtr& rpRequest,
    Stereo3ATuningRes& rOutTuningRes
)
{
#ifdef GTEST
    return MTRUE;
#endif
    // only main1
    rOutTuningRes.tuningRes_main1 = applyISPTuning(rpRequest);
    return MTRUE;
}

MBOOL
P2ABayerNode::
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
    CAM_TRACE_NAME("P2ABayerNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_PRFLOG("threadLoop start, reqID=%d eState=%d isSkip:%d needFEFM:%d",
                    pRequest->getRequestNo(), pRequest->getRequestAttr().opState,
                    pRequest->isSkipDepth(mpPipeOption), pRequest->getRequestAttr().needFEFM);

    MBOOL bRet = MTRUE;
    // handle NEED-FEFM request
    if(eSTATE_NORMAL == pRequest->getRequestAttr().opState &&
        pRequest->getRequestAttr().needFEFM)
    {
        bRet = handleNeedFEFMFrame(pRequest);
        // mark on-going-request end
        this->decExtThreadDependency();
    }
    else
    {
        bRet = handleFrames(pRequest);
        if(!bRet)
            this->decExtThreadDependency();
    }

    return bRet;
}

MBOOL
P2ABayerNode::
handleNeedFEFMFrame(sp<DepthMapEffectRequest> pRequest)
{
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    MBOOL bRet = MTRUE;
    // skip frame -> no need to generate MY_S
    if(!pRequest->isSkipDepth(mpPipeOption))
    {
        // generate MY_S by resizing FD or internal fd
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        IImageBuffer* pFdBuf = nullptr;
        MBOOL bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                    .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
        if(!bExistFD && !pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_INTERNAL_FD, pFdBuf))
        {
            MY_LOGE("Cannot get the fd/internal fd buffer.");
            bRet = MFALSE;
            goto lbExit;
        }
        // output MY_S
        IImageBuffer* pMY_S = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_MY_S);

        VSDOF::util::sDpIspConfig config;
        config.pDpIspStream = mpDpIspStream;
        config.pSrcBuffer = pFdBuf;
        config.pDstBuffer = pMY_S;
        config.rotAngle = (int)StereoSettingProvider::getModuleRotation();
        // PQParam
        DpPqParam pqParam;
        if(!configureDpPQParam(11, pRequest, miSensorIdx_Main1, pqParam, true))
        {
            MY_LOGE("PQ config failed!.");
            bRet = MFALSE;
            goto lbExit;
        }
        config.pDpPqParam = &pqParam;

        pRequest->mTimer.startP2ABayer();
        if(!excuteDpIspStream(config))
        {
            MY_LOGE("excuteMDP fail: Cannot perform MDP operation on target1.");
            bRet = MFALSE;
            goto lbExit;
        }
        pRequest->mTimer.stopP2ABayer();
        VSDOF_PRFTIME_LOG("+ :reqID=%d , pure mdp exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ABayer());

        // prepare output
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_HWOCC);
        // check data sequential
        SequenceOPState state = this->sequentialize(pRequest);
        if (state == SEQ_DISABLE)
        {
            VSDOF_LOGD("Seq: No need to sequential, reqID=%d", pRequest->getRequestNo());
            // notify YUV done callback
            this->handleDataAndDump(P2A_OUT_YUV_DONE, pRequest);
            this->handleDataAndDump(P2A_TO_HWOCC_MY_S, pRequest);
        }
        else
        {
            VSDOF_LOGD("Seq: push sequential done, reqID=%d", pRequest->getRequestNo());
            goto lbExit;
        }
    }
    // handle queue flow
    if(!onHandleFlowTypeP2Done(pRequest))
    {
        MY_LOGE("onHandleFlowTypeP2Done failed!");
        bRet = MFALSE;
    }
lbExit:
    VSDOF_LOGD("- reqID=%d", pRequest->getRequestNo());
    return bRet;
}

MBOOL
P2ABayerNode::
handleFrames(sp<DepthMapEffectRequest> pRequest)
{
    // enque QParams
    QParams enqueParams;
    // enque cookie instance
    EnqueCookieContainer *pCookieIns = new EnqueCookieContainer(pRequest, this);
    // apply 3A Isp tuning
    Stereo3ATuningRes tuningRes;
    MBOOL bRet = MTRUE;
    // only normal req need nr3d
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
        bRet &= setup3DNRMeta(getNodeId(), pRequest);
    bRet = perform3AIspTuning(pRequest, tuningRes);
    // perform 3dnr
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
        bRet &= perform3dnr(pRequest, mp3AHal_Main1, mpFlowOption, tuningRes);
    if(!bRet)
        goto lbExit;

    // call flow option to build QParams
    bRet = mpFlowOption->buildQParam_Bayer(pRequest, tuningRes, enqueParams);
    //
    debugQParams(enqueParams);
    if(!bRet)
    {
        AEE_ASSERT("[P2ABayerNode]Failed to build P2 enque parametes.");
        goto lbExit;
    }

    // callback
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpfnEnQFailCallback = onP2FailedCallback;
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // start P2A timer
    pRequest->mTimer.startP2ABayer();
    pRequest->mTimer.startP2ABayerEnque();
    // sequential: mark on going
    this->markOnGoingData(pRequest);
    // enque
    CAM_TRACE_BEGIN("P2ABayerNode::NormalStream::enque");
    VSDOF_PRFTIME_LOG("mpINormalStream enque start! reqID=%d", pRequest->getRequestNo());
    bRet = mpINormalStream->enque(enqueParams);
    CAM_TRACE_END();
    // stop P2A Enque timer
    pRequest->mTimer.stopP2ABayerEnque();
    VSDOF_PRFTIME_LOG("mpINormalStream enque end! reqID=%d, exec-time(enque)=%d msec",
                    pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ABayerEnque());
    if(!bRet)
    {
        MY_LOGE("mpINormalStream enque failed! reqID=%d", pRequest->getRequestNo());
        goto lbExit;
    }
    return MTRUE;
lbExit:
    delete pCookieIns;
    return MFALSE;
}

MBOOL
P2ABayerNode::
onHandleOnGoingReqReady(
    MUINT32 iReqID
)
{
    // sequential: mark finish
    Vector<DepthMapRequestPtr> popReqVec;
    this->markFinishAndPop(iReqID, popReqVec);
    // bypass all the poped request
    for(size_t idx = 0;idx<popReqVec.size();++idx)
    {
        DepthMapRequestPtr pReq = popReqVec.itemAt(idx);
        VSDOF_LOGD("Seq: free the reqID=%d", pReq->getRequestNo());
        this->handleDataAndDump(P2A_OUT_YUV_DONE, pReq);
        this->handleDataAndDump(P2A_TO_HWOCC_MY_S, pReq);
        // handle queue flow
        if(!onHandleFlowTypeP2Done(pReq))
        {
            MY_LOGE("onHandleFlowTypeP2Done failed!");
            return MFALSE;
        }
    }
    return MTRUE;
}

MVOID
P2ABayerNode::
onP2Callback(QParams& rParams)
{
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    P2ABayerNode* pP2ABayerNode = (P2ABayerNode*) (pEnqueData->mpNode);
    pP2ABayerNode->handleP2Done(rParams, pEnqueData);
}

MVOID
P2ABayerNode::
onP2FailedCallback(QParams& rParams)
{
    MY_LOGE("P2A operations failed!!Check the following log:");
    EnqueCookieContainer* pEnqueData = (EnqueCookieContainer*) (rParams.mpCookie);
    P2ABayerNode* pP2ABayerNode = (P2ABayerNode*) (pEnqueData->mpNode);
    MUINT32 iReqNo = pEnqueData->mRequest->getRequestNo();
    debugQParams(rParams);
    pP2ABayerNode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mRequest);
    // launch onProcessDone
    pEnqueData->mRequest->getBufferHandler()->onProcessDone(pP2ABayerNode->getNodeId());
    delete pEnqueData;
    // handle request data for sequential use
    pP2ABayerNode->onHandleOnGoingReqReady(iReqNo);
    // mark on-going-request end
    pP2ABayerNode->decExtThreadDependency();
}

MVOID
P2ABayerNode::
handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_TRACE_NAME("P2ABayerNode::handleP2Done");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // stop timer
    pRequest->mTimer.stopP2ABayer();
    VSDOF_PRFTIME_LOG("+ :reqID=%d , p2 exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ABayer());
    // dump buffer
    this->handleDump(TO_DUMP_IMG3O, pRequest);
    // callbacks
    mpFlowOption->onP2ProcessDone_Bayer(this, pRequest);
    this->configureToNext(pRequest);
    // handle flow type task
    if(!onHandleFlowTypeP2Done(pRequest))
    {
        MY_LOGE("onHandleFlowTypeP2Done failed!");
        return;
    }
lbExit:
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    VSDOF_PRFLOG("- :reqID=%d", pRequest->getRequestNo());
    delete pEnqueCookie;
    // handle request data for sequential use
    this->onHandleOnGoingReqReady(pRequest->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();
}

MBOOL
P2ABayerNode::
configureToNext(DepthMapRequestPtr pRequest)
{
    // only capture frame goes here
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // mark buffer ready and handle data
    if(eSTATE_NORMAL == pRequest->getRequestAttr().opState &&
        !pRequest->getRequestAttr().needFEFM ||
        eSTATE_STANDALONE == pRequest->getRequestAttr().opState)
    {
        // FD
        if(pRequest->setOutputBufferReady(BID_P2A_OUT_FDIMG))
            this->handleDataAndDump(P2A_OUT_FD, pRequest);
        // MV_F
        if(pRequest->setOutputBufferReady(BID_P2A_OUT_MV_F))
            this->handleDataAndDump(P2A_OUT_MV_F, pRequest);
    }
    // MV_F_CAP
    if(pRequest->setOutputBufferReady(BID_P2A_OUT_MV_F_CAP))
        this->handleDataAndDump(P2A_OUT_MV_F_CAP, pRequest);

    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
       pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_HWOCC);
       this->handleDataAndDump(P2A_TO_HWOCC_MY_S, pRequest);
       // not need fefm -> output Rect_in1 to N3D to generate DPE input
       if(!pRequest->getRequestAttr().needFEFM)
       {
            pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_RECT_IN1, eDPETHMAP_PIPE_NODEID_N3D);
            this->handleDataAndDump(P2A_TO_N3D_NOFEFM_RECT1, pRequest);
       }
       // notify YUV done
        this->handleDataAndDump(P2A_OUT_YUV_DONE, pRequest);
    }
    else if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_OCC);
        this->handleDataAndDump(P2A_TO_OCC_MY_S, pRequest);
        // PostView
        pRequest->setOutputBufferReady(BID_P2A_OUT_POSTVIEW);
        // notify YUV done
        this->handleDataAndDump(P2A_OUT_YUV_DONE, pRequest);
        // pass to FD node
        pBufferHandler->configOutBuffer(eDPETHMAP_PIPE_NODEID_P2ABAYER, BID_P2A_OUT_FDIMG, eDPETHMAP_PIPE_NODEID_FD);
        this->handleDataAndDump(P2A_TO_FD_IMG, pRequest);
    }

    VSDOF_LOGD("- reqID=%d", pRequest->getRequestNo());
    return MTRUE;
}

DpColorFormat
P2ABayerNode::
mapToDpColorFmt(MINT format)
{
    switch(format)
    {
        case eImgFmt_YV12:
            return DP_COLOR_YV12;
        default:
            MY_LOGE("No support format:%d", format);
    }
    return DP_COLOR_UNKNOWN;
}


MBOOL
P2ABayerNode::
onHandleFlowTypeP2Done(
    sp<DepthMapEffectRequest> pRequest
)
{
    auto markOutBufferAndStatus = [&](DepthMapBufferID bufferID, int val) {
                                IImageBuffer* pBuf = nullptr;
                                if(pRequest->getRequestImageBuffer({.bufferID=bufferID, .ioType=eBUFFER_IOTYPE_OUTPUT}, pBuf))
                                {
                                    ::memset((void*)pBuf->getBufVA(0), val, pBuf->getBufSizeInBytes(0));
                                    pBuf->syncCache(eCACHECTRL_FLUSH);
                                    // mark ready
                                    pRequest->setOutputBufferReady(bufferID);
                                }
                            };
    // default value
    const int DMBG_DEFAULT_VAL = 0;
    const int DEPTH_DEFAULT_VAL = 255;

    MUINT32 iReqID = pRequest->getRequestNo();
    VSDOF_LOGD("reqID=%d +", iReqID);
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    if(pRequest->isQueuedDepthRequest(mpPipeOption))
    {
        IMetadata* pOutHalMeta = nullptr;
        // get queue depth info
        DepthBufferInfo depthInfo;
        if(!mpDepthStorage->getStoredData(depthInfo))
        {
            VSDOF_LOGD("reqID=%d, depth not ready!", pRequest->getRequestNo());
            // clear DMBG
            markOutBufferAndStatus(BID_GF_OUT_DMBG, DMBG_DEFAULT_VAL);
            handleData(REQUEST_DEPTH_NOT_READY, pRequest);
            goto lbExit;
        }
        MY_LOGD("[DepthPipe] reqID=%d use the reqID=%d's blur map, depth delay is %d frames!",
            iReqID, depthInfo.miReqIdx, iReqID - depthInfo.miReqIdx);
        // handle stereo warning
        IMetadata* pMeta_Ret = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_P1_RETURN);
        IMetadata* pMeta_OutApp = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_APP);
        MINT32 iWarningMsg = 0;
        if(tryGetMetadata<MINT32>(pMeta_Ret, MTK_STEREO_FEATURE_WARNING, iWarningMsg))
        {
            VSDOF_LOGD("reqID=%d warning msg=%d", pRequest->getRequestNo(), iWarningMsg);
            if(iWarningMsg != 0)
            {
                // clear DMBG
                markOutBufferAndStatus(BID_GF_OUT_DMBG, DMBG_DEFAULT_VAL);
                // clear depth
                markOutBufferAndStatus(BID_GF_OUT_DEPTH, DEPTH_DEFAULT_VAL);
            }
            // set meta
            trySetMetadata<MINT32>(pMeta_OutApp, MTK_STEREO_FEATURE_WARNING, iWarningMsg);
        }
        // set distance
        trySetMetadata<MFLOAT>(pMeta_OutApp, MTK_STEREO_FEATURE_RESULT_DISTANCE, depthInfo.mfDistance);
        // copy queued DMBG into request
        if(iWarningMsg == 0 &&
            pRequest->isRequestBuffer(BID_GF_OUT_DMBG) &&
            !_copyBufferIntoRequest(depthInfo.mpDMBGBuffer->mImageBuffer.get(),
                                    pRequest, BID_GF_OUT_DMBG))
        {
            AEE_ASSERT("[P2ABayerNode] Failed to copy BID_GF_OUT_DMBG");
            return MFALSE;
        }
        // copy queued DepthMap into request
        if(iWarningMsg == 0 &&
            pRequest->isRequestBuffer(BID_GF_OUT_DEPTH) &&
            !_copyBufferIntoRequest(depthInfo.mpDepthBuffer->mImageBuffer.get(),
                                    pRequest, BID_GF_OUT_DEPTH))
        {
            AEE_ASSERT("[P2ABayerNode] Failed to copy BID_GF_OUT_DEPTH");
            return MFALSE;
        }
        // set meta ready
        pRequest->setOutputBufferReady(BID_META_OUT_APP);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL);
        pRequest->setOutputBufferReady(BID_META_OUT_APP_QUEUED);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL_QUEUED);
        // notify pipe
        handleDataAndDump(P2A_OUT_DMBG, pRequest);
    }
    VSDOF_LOGD("reqID=%d -", pRequest->getRequestNo());
lbExit:
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL
P2ABayerNode::
_copyBufferIntoRequest(
    IImageBuffer* pSrcBuffer,
    sp<DepthMapEffectRequest> pRequest,
    DepthMapBufferID bufferID
)
{
    VSDOF_LOGD("reqID=%d copy bufferID:%d (%s) into request!",
                pRequest->getRequestNo(), bufferID, onDumpBIDToName(bufferID));
    if(pRequest->isRequestBuffer(bufferID))
    {
        IImageBuffer* pImgBuf = pRequest->getBufferHandler()->requestBuffer(getNodeId(), bufferID);
        if(pImgBuf->getBufSizeInBytes(0) != pSrcBuffer->getBufSizeInBytes(0))
        {
            MY_LOGE("buffer size(id:%d %s) is not consistent!request buffer size=%dx%d(%d bytes) src_buffer size=%dx%d(%d bytes)",
                        bufferID, onDumpBIDToName(bufferID),
                        pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, pImgBuf->getBufSizeInBytes(0),
                        pSrcBuffer->getImgSize().w, pSrcBuffer->getImgSize().h, pSrcBuffer->getBufSizeInBytes(0));
            return MFALSE;
        }
        memcpy( (void*)pImgBuf->getBufVA(0),
                (void*)pSrcBuffer->getBufVA(0),
                pImgBuf->getBufSizeInBytes(0));
        pImgBuf->syncCache(eCACHECTRL_FLUSH);
        // mark ready
        pRequest->setOutputBufferReady(bufferID);
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/

MVOID
P2ABayerNode::
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

