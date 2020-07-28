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

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_P2A_BAYER);
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
, miSensorIdx_Main1(config.mpSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(config.mpSetting->miSensorIdx_Main2)
{
    this->addWaitQueue(&mRequestQue);
    //
    mbIsWaitDepth = ::property_get_bool("vendor.depthmap.pipe.enableWaitDepth", 0);
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

    if(mpIspHal_Main1)
    {
        mpIspHal_Main1->destroyInstance(getName());
        mpIspHal_Main1 = nullptr;
    }

     // nr3d
    if(!NR3DCommon::uninit())
    {
        MY_LOGE("Failed to uninit NR3D.");
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
    CAM_ULOGM_TAGLIFE("P2ABayerNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // Create NormalStream
    VSDOF_LOGD("NormalStream create instance: idx=%d", miSensorIdx_Main1);
    CAM_ULOGM_TAG_BEGIN("P2ABayerNode::NormalStream::createInstance+init");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);

    if (mpINormalStream == nullptr)
    {
        MY_LOGE("mpINormalStream create instance for P2A Node failed!");
        cleanUp();
        return MFALSE;
    }
    mpINormalStream->init(getName());
    CAM_ULOGM_TAG_END();
    // 3A: create instance
    // UT does not test 3A
    CAM_ULOGM_TAG_BEGIN("P2ABayerNode::create_3A_instance");
    #ifndef GTEST
    mpIspHal_Main1 = MAKE_HalISP(miSensorIdx_Main1, getName());
    MY_LOGD("3A create instance, Main1: %p", mpIspHal_Main1);
    #endif
    CAM_ULOGM_TAG_END();
    // nr3d
    if(!NR3DCommon::init("P2ABayer3DNR", miSensorIdx_Main1))
    {
        MY_LOGE("Failed to init NR3D.");
        return MFALSE;
    }
    // create MDP stream
    mpDpIspStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
P2ABayerNode::
onThreadStop()
{
    CAM_ULOGM_TAGLIFE("P2ABayerNode::onThreadStop");
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
    VSDOF_LOGD("+ : reqID=%d dataid=%s", request->getRequestNo(), ID2Name(data));
    CAM_ULOGM_TAGLIFE("P2ABayerNode::onData");

    switch(data)
    {
        case BAYER_ENQUE:
        {
            VSDOF_PRFLOG("+ : reqID=%d size=%d", request->getRequestNo(), mRequestQue.size());
            mRequestQue.enque(request);
            break;
        }
        case N3D_TO_P2ABYER_P1YUV_USED:
        {
            Mutex::Autolock _l(mLock);
            mvN3DReadyReq.add(request->getRequestNo(), MTRUE);
            mCondition.broadcast();
            break;
        }
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
    DepthMapRequestPtr& rpRequest,
    MBOOL isFEPass
)
{
    CAM_ULOGM_TAGLIFE("P2ABayerNode::applyISPTuning");
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
    MVOID* pTuningBuf = pTuningBuf = pBufferHandler->requestWorkingTuningBuf(getNodeId(), BID_P2A_TUNING);
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
    // FEPass
    if(isFEPass)
    {
        updateEntry<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, ISP_TUNING_FE_PASS_MODE);
    }
    // UT do not test setP2Isp
    AAATuningResult result(pTuningBuf, pLcsBuf);
    #ifndef GTEST
    //
    mpIspHal_Main1->setP2Isp(0, inMetaSet, &result.tuningResult, &outMetaSet);
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
    rpRequest->mTimer.startP2ABayerSetIsp();
    // only main1
    rOutTuningRes.tuningRes_main1 = applyISPTuning(rpRequest);
    rOutTuningRes.tuningRes_FE_main1 = applyISPTuning(rpRequest, MTRUE);;
    rpRequest->mTimer.stopP2ABayerSetIsp();
    return MTRUE;
}

MBOOL
P2ABayerNode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;
    DepthMapRequestPtr pP1YUVDoneReq;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mRequestQue.deque(pRequest) )
    {
        MY_LOGE("mRequestQue.deque() failed");
        return MFALSE;
    }
    CAM_ULOGM_TAGLIFE("P2ABayerNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_LOGD("threadLoop start, reqID=%d eState=%d isSkip:%d needFEFM:%d",
                    pRequest->getRequestNo(), pRequest->getRequestAttr().opState,
                    pRequest->isSkipDepth(mpPipeOption), pRequest->getRequestAttr().needFEFM);
    // start P2A timer
    pRequest->mTimer.startP2ABayer();
    //
    MBOOL bRet = handleFrames(pRequest);
    if(!bRet)
        this->decExtThreadDependency();

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
    // perform 3dnr
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
        NSIspTuning::EIspProfile_T profile = mpFlowOption->getIspProfile_Bayer(pRequest);
        bRet &= perform3dnr(pRequest, mpFlowOption, tuningRes);
    }
    bRet &= perform3AIspTuning(pRequest, tuningRes);
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
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // enque
    CAM_ULOGM_TAG_BEGIN("P2ABayerNode::NormalStream::enque");
    VSDOF_LOGD("mpINormalStream enque start! reqID=%d", pRequest->getRequestNo());
    pRequest->mTimer.startP2ABayerDrv();
    bRet = mpINormalStream->enque(enqueParams);
    CAM_ULOGM_TAG_END();
    VSDOF_LOGD("mpINormalStream enque end!");
    if(!bRet)
    {
        AEE_ASSERT("[P2ABayerNode] NormalStream enque failed");
    }
    return MTRUE;
lbExit:
    delete pCookieIns;
    return MFALSE;
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
handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueCookie)
{
    CAM_ULOGM_TAGLIFE("P2ABayerNode::handleP2Done");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    // stop timer
    pRequest->mTimer.stopP2ABayerDrv();
    VSDOF_LOGD("reqID=%d , p2 exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ABayerDrv());
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // dump buffer
    this->handleDump(TO_DUMP_IMG3O, pRequest);
    // wait for former MYS resize
    if(mResizeFuture.valid())
    {
        VSDOF_LOGD("reqID=%d wait for former MYS resize", pRequest->getRequestNo());
        mResizeFuture.wait();
        VSDOF_LOGD("reqID=%d wait for former MYS resize, done!", pRequest->getRequestNo());
    }
    // if need FEFM, need to generate the MY_S using MDP
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL &&
        pRequest->getRequestAttr().needFEFM)
    {
        VSDOF_LOGD("reqID=%d create thread for MYS resize", pRequest->getRequestNo());
        mResizeFuture = std::async(
                        std::launch::async,
                        &P2ABayerNode::resizeFDIntoMYS, this, pEnqueCookie);
        return;
    }
    else
    {
        // wait
        if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
            this->waitForN3DFinish(pRequest);
        // callbacks
        mpFlowOption->onP2ProcessDone_Bayer(this, pRequest);
        this->configureToNext(pRequest);
        // handle flow type task
        if(!onHandleFlowTypeP2Done(pRequest))
        {
            MY_LOGE("onHandleFlowTypeP2Done failed!");
            goto lbExit;
        }
lbExit:
        // launch onProcessDone
        pRequest->getBufferHandler()->onProcessDone(getNodeId());
        VSDOF_PRFLOG("- :reqID=%d", pRequest->getRequestNo());
        delete pEnqueCookie;
        // mark on-going-request end
        this->decExtThreadDependency();
        return;
    }
}

MBOOL
P2ABayerNode::
waitForN3DFinish(sp<DepthMapEffectRequest> pRequest)
{
    Mutex::Autolock _l(mLock);
    VSDOF_LOGD("reqID=%d wait for N3D Finish +", pRequest->getRequestNo());
    ssize_t index = -1;
    pRequest->mTimer.startP2ABayerN3DWaiting();
    while(mvN3DReadyReq.indexOfKey(pRequest->getRequestNo()) < 0)
    {
        VSDOF_LOGD("reqID=%d wait for N3D Finish waiting!", pRequest->getRequestNo());
        mCondition.wait(mLock);
    }
    pRequest->mTimer.stopP2ABayerN3DWaiting();
    mvN3DReadyReq.removeItem(pRequest->getRequestNo());
    VSDOF_LOGD("reqID=%d wait for N3D Finish :%dms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ABayerN3DWaiting());
    return MTRUE;
}

MBOOL
P2ABayerNode::
resizeFDIntoMYS(EnqueCookieContainer* pEnqueCookie)
{
    MBOOL bRet = MTRUE;
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    VSDOF_LOGD("reqID=%d, do resizeFDIntoMYS", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IImageBuffer* pFdBuf = nullptr;
    IImageBuffer* pMY_S = nullptr;
    VSDOF::util::sDpIspConfig config;
    DpPqParam pqParam;
    MBOOL bExistFD;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // generate MY_S by resizing FD or internal fd
    bExistFD = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_OUT_FDIMG,
                                                .ioType=eBUFFER_IOTYPE_OUTPUT}, pFdBuf);
    if(!bExistFD && !pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_INTERNAL_FD, pFdBuf))
    {
        MY_LOGE("Cannot get the fd/internal fd buffer, reqID=%d", pRequest->getRequestNo());
        bRet = MFALSE;
        goto lbExit;
    }
    // output MY_S
    pMY_S = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_MY_S);

    // fill config
    config.pDpIspStream = mpDpIspStream;
    config.pSrcBuffer = pFdBuf;
    config.pDstBuffer = pMY_S;
    config.rotAngle = (int)StereoSettingProvider::getModuleRotation();
    // PQParam
    if(!configureDpPQParam(7, pRequest, miSensorIdx_Main1, pqParam))
    {
        MY_LOGE("PQ config failed!.");
        bRet = MFALSE;
        goto lbExit;
    }
    config.pDpPqParam = &pqParam;

    pRequest->mTimer.startP2ABayerMYSResize();
    if(!excuteDpIspStream(config))
    {
        MY_LOGE("excuteMDP fail: Cannot perform MDP operation on target1.");
        bRet = MFALSE;
        goto lbExit;
    }
    pRequest->mTimer.stopP2ABayerMYSResize();
    VSDOF_PRFTIME_LOG("+ :reqID=%d , pure mdp exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedP2ABayerMYSResize());
    // wait for N3D Done
    this->waitForN3DFinish(pRequest);
    // callbacks
    mpFlowOption->onP2ProcessDone_Bayer(this, pRequest);
    this->configureToNext(pRequest);
    // handle flow type task
    if(!onHandleFlowTypeP2Done(pRequest))
    {
        MY_LOGE("onHandleFlowTypeP2Done failed!");
        bRet = MFALSE;
    }
lbExit:
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    VSDOF_LOGD("- :reqID=%d", pRequest->getRequestNo());
    delete pEnqueCookie;
    // mark on-going-request end
    this->decExtThreadDependency();
    return bRet;
}

MBOOL
P2ABayerNode::
configureToNext(DepthMapRequestPtr pRequest)
{
    // timer
    pRequest->mTimer.stopP2ABayer();
    // only capture frame goes here
    VSDOF_LOGD("+ reqID=%d", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // mark buffer ready and handle data
    if(eSTATE_NORMAL == pRequest->getRequestAttr().opState ||
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
        // pass MY_S to DPE
       pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_DPE);
       this->handleDataAndDump(P2A_TO_DPE_MY_S, pRequest);
       // Need FEFM -> output FEFM to N3D for learning
       if(pRequest->getRequestAttr().needFEFM)
       {
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE2BO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE2CO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE1BO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FE1CO, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMBO_LR, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMBO_RL, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMCO_LR, eDPETHMAP_PIPE_NODEID_N3D);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_FMCO_RL, eDPETHMAP_PIPE_NODEID_N3D);
            this->handleDataAndDump(P2A_TO_N3D_FEOFMO, pRequest);
       }
       // notify YUV done
        this->handleDataAndDump(P2A_OUT_YUV_DONE, pRequest);
    }
    else if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        // PostView
        pRequest->setOutputBufferReady(BID_P2A_OUT_POSTVIEW);
        // notify YUV done
        this->handleDataAndDump(P2A_OUT_YUV_DONE, pRequest);
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
        MBOOL bDepthReady = MFALSE;
        // check
        if(mbIsWaitDepth)
        {
            if(pRequest->getRequestAttr().opState == eSTATE_STANDALONE &&
                mpDepthStorage->getLatestData(depthInfo))
            {
                bDepthReady = MTRUE;
            }
            else if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
            {
                bDepthReady = mpDepthStorage->wait_pop(depthInfo);
            }
        }
        else
            bDepthReady = mpDepthStorage->getStoredData(depthInfo);
        //
        if(!bDepthReady)
        {
            VSDOF_LOGD("reqID=%d, depth not ready!", pRequest->getRequestNo());
            // timer
            pRequest->mTimer.stopP2ABayer();
            // clear DMBG
            markOutBufferAndStatus(BID_GF_OUT_DMBG, DMBG_DEFAULT_VAL);
            handleData(REQUEST_DEPTH_NOT_READY, pRequest);
            goto lbExit;
        }
        VSDOF_LOGD("[DepthPipe] reqID=%d use the reqID=%d's blur map, depth delay is %d frames!",
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
                markOutBufferAndStatus(BID_DPE_OUT_UNPROCESS_DEPTH, DEPTH_DEFAULT_VAL);
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
            pRequest->isRequestBuffer(BID_DPE_OUT_UNPROCESS_DEPTH) &&
            !_copyBufferIntoRequestWithCrop(depthInfo.mpDepthBuffer->mImageBuffer.get(),
                                    pRequest, BID_DPE_OUT_UNPROCESS_DEPTH))
        {
            AEE_ASSERT("[P2ABayerNode] Failed to copy BID_DPE_OUT_UNPROCESS_DEPTH");
            return MFALSE;
        }
        // set meta ready
        pRequest->setOutputBufferReady(BID_META_OUT_APP);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL);
        pRequest->setOutputBufferReady(BID_META_OUT_APP_QUEUED);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL_QUEUED);
        // notify pipe
        handleDataAndDump(P2A_NORMAL_FRAME_DONE, pRequest);
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

MBOOL
P2ABayerNode::
_copyBufferIntoRequestWithCrop(
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
        MUINT8 *pSrc = (MUINT8*)pSrcBuffer->getBufVA(0);
        MSize srcSize = pSrcBuffer->getImgSize();
        MUINT8 *pDst = (MUINT8*)pImgBuf->getBufVA(0);
        MSize dstSize = pImgBuf->getImgSize();

        if(srcSize.w == dstSize.w &&
           srcSize.h == dstSize.h)
        {
            ::memcpy(pDst, pSrc, dstSize.w*dstSize.h);
        }
        else if(srcSize.w >= dstSize.w &&
                srcSize.h >= dstSize.h)
        {
            for(int row = 0; row < dstSize.h; ++row)
            {
                ::memcpy(pDst, pSrc, dstSize.w);
                pDst += dstSize.w;
                pSrc += srcSize.w;
            }
        }
        else
        {
            MY_LOGE("Dst size(%dx%d) > src size(%dx%d)", dstSize.w, dstSize.h, srcSize.w, srcSize.h);
        }

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
    //
    if(mResizeFuture.valid())
        mResizeFuture.wait();
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

