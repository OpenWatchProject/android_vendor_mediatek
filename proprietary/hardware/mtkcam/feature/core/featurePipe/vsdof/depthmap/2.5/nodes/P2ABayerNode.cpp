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
        MY_LOGE("mpINormalStream create instance for P2AFM Node failed!");
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
    MY_LOGD("3A create instance, Main1: %x Main2: %x", mp3AHal_Main1);
    #endif
    CAM_TRACE_END();

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
    VSDOF_LOGD("+, reqID=%d", rpRequest->getRequestNo());

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
    MVOID* pTuningBuf = nullptr;
    if(rpRequest->getRequestImageBuffer({BID_P2A_OUT_TUNING_MAIN1, eBUFFER_IOTYPE_OUTPUT}, pTuningImgBuf))
    {
        pTuningBuf = (MVOID*) pTuningImgBuf->getBufVA(0);
        memset(pTuningBuf, 0 , INormalStream::getRegTableSize());
    }
    else
    {
        pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    }
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
    if(rpRequest->getRequestAttr().opState == eSTATE_CAPTURE)
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Capture);
    else
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_Preview);
    // UT do not test setIsp
    #ifndef GTEST
    AAATuningResult result(pTuningBuf, pLcsBuf);
    // 3A result
    MetaSet_T resultMeta;
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
    VSDOF_LOGD("-, reqID=%d", rpRequest->getRequestNo());
    return result;
}

MBOOL
P2ABayerNode::
perform3AIspTuning(
    DepthMapRequestPtr& rpRequest,
    Stereo3ATuningRes& rOutTuningRes
)
{
    // only main1
    rOutTuningRes.tuningRes_main1 = applyISPTuning(rpRequest);
    return MTRUE;
}

MBOOL
P2ABayerNode::
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
    CAM_TRACE_NAME("P2ABayerNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_PRFLOG("threadLoop start, reqID=%d eState=%d isSkip:%d",
                    request->getRequestNo(), request->getRequestAttr().opState,
                    request->isSkipDepth(mpPipeOption));

    // bypass the Bayer operation when disable LCE
    if(!mpPipeOption->mbEnableLCE
        && !request->isSkipDepth(mpPipeOption)
        && eSTATE_NORMAL == request->getRequestAttr().opState)
    {
        // check data sequential
        SequenceOPState state = this->sequentialize(request);
        if (state == SEQ_DISABLE)
        {
            VSDOF_LOGD("Seq: No need to sequential, bypass P2ABayer, reqID=%d",
                        request->getRequestNo());
            // launch flow option p2 done
            mpFlowOption->onP2ProcessDone_Bayer(this, request);
            // handle flow type task
            if(!onHandleFlowTypeP2Done(request))
            {
                MY_LOGE("onHandleFlowTypeP2Done failed!");
                return MFALSE;
            }

        }
        else
            VSDOF_LOGD("Seq: push sequential done, reqID=%d", request->getRequestNo());
        this->decExtThreadDependency();
        return MTRUE;
    }
    else
    {
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
        bRet = mpFlowOption->buildQParam_Bayer(request, tuningRes, enqueParams);
        //
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
        request->mTimer.startP2ABayer();
        request->mTimer.startP2ABayerEnque();
        // sequential: mark on going
        this->markOnGoingData(request);
        // enque
        CAM_TRACE_BEGIN("P2ABayerNode::NormalStream::enque");
        bRet = mpINormalStream->enque(enqueParams);
        CAM_TRACE_END();
        // stop P2A Enque timer
        request->mTimer.stopP2ABayerEnque();
        VSDOF_PRFTIME_LOG("mpINormalStream enque end! reqID=%d, exec-time(enque)=%d msec",
                        request->getRequestNo(), request->mTimer.getElapsedP2ABayerEnque());
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
    pP2ABayerNode->onHandleOnGoingReqDataReady(iReqNo);
    // mark on-going-request end
    pP2ABayerNode->decExtThreadDependency();
}

MBOOL
P2ABayerNode::
onHandleOnGoingReqDataReady(
    MUINT32 iReqNo
)
{
    // sequential: mark finish
    Vector<DepthMapRequestPtr> popReqVec;
    this->markFinishAndPop(iReqNo, popReqVec);
    // bypass all the poped request
    for(size_t idx = 0;idx<popReqVec.size();++idx)
    {
        DepthMapRequestPtr pReq = popReqVec.itemAt(idx);
        VSDOF_LOGD("Seq: free the reqID=%d", pReq->getRequestNo());
        // launch flow option p2 done
        mpFlowOption->onP2ProcessDone_Bayer(this, pReq);
        // handle flow type task
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
    // launch flow option p2 done
    mpFlowOption->onP2ProcessDone_Bayer(this, pRequest);
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
    VSDOF_PRFLOG("- :reqID=%d", pRequest->getRequestNo());
    // handle request data for sequential use
    this->onHandleOnGoingReqDataReady(pRequest->getRequestNo());
    // mark on-going-request end
    this->decExtThreadDependency();
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
    MUINT32 iReqID = pRequest->getRequestNo();
    VSDOF_LOGD("reqID=%d +", iReqID);
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    if(pRequest->isQueuedDepthRequest(mpPipeOption))
    {
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
            (mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF || mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH))
        {
            VSDOF_LOGD("reqID=%d, depth not ready!", pRequest->getRequestNo());
            handleData(REQUEST_DEPTH_NOT_READY, pRequest);
            goto lbExit;
        }

        // write meta
        {
            pOutHalMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), BID_META_OUT_HAL);
            trySetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, depthInfo.mfConvOffset);
            trySetMetadata<MFLOAT>(pOutHalMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, depthInfo.mfDistance);
            //
            IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
            trySetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, depthInfo.magicNumber);
        }

        DepthMapDataID nextDataId = ID_INVALID;
        if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
        {
            if(!_copyBufferIntoRequestAndSetReady(depthInfo.mpDepthBuffer->mImageBuffer.get(),
                                pRequest, BID_P2A_OUT_DEPTHMAP))
            {
                MY_LOGE("Failed to copy BID_P2A_OUT_DEPTHMAP");
                goto lbExit;
            }
            nextDataId = P2A_OUT_DEPTHMAP;
        }
        else
        {
            // config queued Depth buffer as output buffer to GF Node
            pBufferHandler->configExternalOutBuffer(getNodeId(), BID_WMF_OUT_DMW,
                                                    eDPETHMAP_PIPE_NODEID_GF, depthInfo.mpDepthBuffer);
            // config MY_S buffer
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_GF);
            // pass to GF
            nextDataId = P2A_TO_GF_DMW_MYS;
        }
        // set meta ready
        pRequest->setOutputBufferReady(BID_META_OUT_APP);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL);
        pRequest->setOutputBufferReady(BID_META_OUT_APP_QUEUED);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL_QUEUED);
        handleDataAndDump(nextDataId, pRequest);
    }

lbExit:
    return MTRUE;
}

MBOOL
P2ABayerNode::
_copyBufferIntoRequestAndSetReady(
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

MVOID
P2ABayerNode::
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

