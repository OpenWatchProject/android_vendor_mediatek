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
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <isp_tuning.h>
// Module header file

// Local header file
#include "P2ANode.h"
// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "P2ANode"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Const Definition
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2ANode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
P2ANode::
P2ANode(
    const char *name,
    IspPipeNodeID nodeID,
    const PipeNodeConfigs& config
)
: IspPipeNode(name, nodeID, config)
, miSensorIdx_Main1(config.mpPipeSetting->miSensorIdx_Main1)
, miSensorIdx_Main2(config.mpPipeSetting->miSensorIdx_Main2)
{
    MY_LOGD("ctor +");
    this->addWaitQueue(&mRequestQue);
    MY_LOGD("ctor -");
}

P2ANode::
~P2ANode()
{
    MY_LOGD("+");
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2ANode Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
P2ANode::
cleanUp()
{
    MY_LOGD("+");
    if(mpINormalStream != NULL)
    {
        mpINormalStream->uninit("IspPipe_P2ANode");
        mpINormalStream->destroyInstance();
        mpINormalStream = NULL;
    }

    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("IspPipe_P2A_Main1");
        mp3AHal_Main1 = NULL;
    }

    if(mp3AHal_Main2)
    {
        mp3AHal_Main2->destroyInstance("IspPipe_P2A_Main2");
        mp3AHal_Main2 = NULL;
    }
    MY_LOGD("-");
}

MBOOL
P2ANode::
onInit()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
P2ANode::
onUninit()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
P2ANode::
onThreadStart()
{
    MY_LOGD("+");
    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);
    if(mpINormalStream == nullptr)
    {
        MY_LOGE("mpINormalStream create instance for P2A Node failed!");
        cleanUp();
        return MFALSE;
    }
    mpINormalStream->init("IspPipe_P2ANode");
    #ifndef GTEST
    mp3AHal_Main1 = MAKE_Hal3A(miSensorIdx_Main1, "IspPipe_P2A_Main1");
    mp3AHal_Main2 = MAKE_Hal3A(miSensorIdx_Main2, "IspPipe_P2A_Main2");
    #endif
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
P2ANode::
onThreadStop()
{
    cleanUp();
    return MTRUE;
}

MBOOL
P2ANode::
checkEnqueValid(const IspPipeRequestPtr& pRequest)
{
    return MTRUE;
}

MBOOL
P2ANode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    MBOOL ret = MTRUE;
    PIPE_LOGD("+ : reqID=%d", pRequest->getRequestNo());
    switch(dataID)
    {
        case ROOT_ENQUE:
            mRequestQue.enque(pRequest);
            break;
        default:
            MY_LOGW("Un-recognized dataID ID, id=%s reqID=%d", ID2Name(dataID), pRequest->getRequestNo());
            ret = MFALSE;
            break;
    }

  PIPE_LOGD("-");
  return ret;
}

MVOID
P2ANode::
onFlush()
{
    MY_LOGD("+, extDep=%d", this->getExtThreadDependency());
    IspPipeRequestPtr pRequest;
    while( mRequestQue.deque(pRequest) )
    {
        sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
    IspPipeNode::onFlush();
    MY_LOGD("-");
}

MBOOL
P2ANode::
onThreadLoop()
{
    IspPipeRequestPtr pRequest;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mRequestQue.deque(pRequest) )
    {
        MY_LOGE("mRequestQue.deque() failed");
        return MFALSE;
    }
    // capture request does not need P2ANode, which just use input YUV
    if(pRequest->getRequestAttr().reqType == PIPE_REQ_CAPTURE)
    {
        PIPE_LOGD("reqID=%d, capture request bypass P2ANode!", pRequest->getRequestNo());
        handleDataAndDump(P2A_TO_TP_YUV_DATA, pRequest);
        return MTRUE;
    }
    // mark on-going-request start
    this->incExtThreadDependency();
    AutoProfileLogging profile("P2A::threadLoop", pRequest->getRequestNo());
    // enque QParams
    QParams enqueParams;
    // prepare & build enquaQParam
    if(!prepareEnqueQParam(pRequest, enqueParams) ||
        !buildEnqueQParam(pRequest, enqueParams))
    {
        MY_LOGE("reqID=%d prepare/build QParam failed!", pRequest->getRequestNo());
        return MFALSE;
    }
    // enque cookie instance
    EnqueCookieContainer* pCookieIns = new EnqueCookieContainer(pRequest, this);
    enqueParams.mpfnCallback = onP2Callback;
    enqueParams.mpfnEnQFailCallback = onP2FailedCallback;
    enqueParams.mpCookie = (MVOID*) pCookieIns;
    // debug
    debugQParams(enqueParams);
    // start P2A timer
    pRequest->mTimer.startP2A();
    {
        AutoProfileLogging profile_2("P2A::enque");
        if(!mpINormalStream->enque(enqueParams))
        {
            MY_LOGE("reqID=%d, Failed to enque Pass2!", pRequest->getRequestNo());
            return MFALSE;
        }
    }
    return MTRUE;
}

MVOID
P2ANode::
handleP2Done(
    QParams& rParams,
    EnqueCookieContainer* pEnqueData
)
{
    IspPipeRequestPtr pRequest = pEnqueData->mRequest;
    AutoProfileLogging profile("P2A::handleP2Done", pRequest->getRequestNo());
    DumpConfig config;
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // stop timer
    pRequest->mTimer.stopP2A();
    // logging
    profile.logging("p2 exec-time=%d ms", pRequest->mTimer.getElapsedP2A());
    // config output
    if(mpPipeOption->mbEnableLCE)
        pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_PV_YUV, eISP_PIPE_NODEID_TP);
    pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN1, eISP_PIPE_NODEID_TP);
    pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN2, eISP_PIPE_NODEID_TP);
    // dump buffer first
    config = DumpConfig(NULL, ".raw", MTRUE);
    this->handleDump(TO_DUMP_RAWS, pRequest, &config);
    // set buffer ready
    pRequest->setOutputBufferReady(BID_PIPE_OUT_PV_FD);
    // pass to next node
    handleDataAndDump(P2A_TO_TP_YUV_DATA, pRequest);

lbExit:
    delete pEnqueData;
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();
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
    MY_LOGE("reqID=%d P2A operations failed!!Check the following log:", iReqNo);
    debugQParams(rParams);
    pP2ANode->handleData(ERROR_OCCUR_NOTIFY, pEnqueData->mRequest);
    //
    pEnqueData->mRequest->getBufferHandler()->onProcessDone(pP2ANode->getNodeId());
    delete pEnqueData;
    // mark on-going-request end
    pP2ANode->decExtThreadDependency();
    MY_LOGD("-");
}


TuningParam
P2ANode::
applyISPTuning(
    IspP2Pass pass,
    const IspPipeRequestPtr& rpRequest
)
{
    PIPE_LOGD("+, reqID=%d pass=%d", rpRequest->getRequestNo(), pass);
    sp<PipeBufferHandler> pBufferHandler = rpRequest->getBufferHandler();
    MBOOL bIsMain1Path = (pass == eP2_DPE_MAIN2) ? MFALSE : MTRUE;
    // get in/out APP/HAL meta
    IspPipeBufferID halMetaBID = bIsMain1Path ? BID_META_IN_HAL_MAIN1 : BID_META_IN_HAL_MAIN2;
    //RMV
    halMetaBID = BID_META_IN_HAL_MAIN1;
    IMetadata* pMeta_InApp  = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    IMetadata* pMeta_InHal  = pBufferHandler->requestMetadata(getNodeId(), halMetaBID);
    IMetadata* pMeta_OutApp = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_APP);
    IMetadata* pMeta_OutHal = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL);
    //LCSO
    IspPipeBufferID bidLCSO = (pass == eP2_DPE_MAIN2) ? PBID_IN_LCSO_MAIN2: PBID_IN_LCSO_MAIN1;
    MVOID* pLcsBuf =  (MVOID*)pBufferHandler->requestBuffer(getNodeId(), bidLCSO);
    // in/out meta set
    MetaSet_T inMetaSet(*pMeta_InApp, *pMeta_InHal);
    MetaSet_T outMetaSet(*pMeta_OutApp, *pMeta_OutHal);
    // set PGN
    if(pass == eP2_FULLSIZE)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    // set isp profile
    if(pass == eP2_DPE_MAIN1 || pass == eP2_DPE_MAIN2)
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_N3D_Preview);
    else if(pass == eP2_RESIZE)
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_Preview);
    else if(pass == eP2_FULLSIZE)
        trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, NSIspTuning::EIspProfile_Capture);
    // bypass LCE
    const MBOOL enableLCE = (pass == eP2_RESIZE) && mpPipeOption->mbEnableLCE;
    updateEntry<MINT32>(&(inMetaSet.halMeta), MTK_3A_ISP_BYPASS_LCE, !enableLCE);
    // UT do not test setIsp
    #ifndef GTEST
    IHal3A* p3AHAL = bIsMain1Path ? mp3AHal_Main1 : mp3AHal_Main2;
    // get tuning buf
    MVOID* pTuningBuf = pBufferHandler->requestWorkingTuningBuf(BID_P2A_TUNING);
    // 3A result
    TuningParam tuningParam(pTuningBuf);
    // enable LCE when human looking pass
    if(pass == eP2_RESIZE && mpPipeOption->mbEnableLCE)
        tuningParam.pLcsBuf = pLcsBuf;
    // set isp
    p3AHAL->setIsp(0, inMetaSet, &tuningParam, &outMetaSet);

    // only FULLRAW(capture) need to get exif result
    if(pass == eP2_FULLSIZE)
    {
        *pMeta_OutApp += outMetaSet.appMeta;
        *pMeta_OutHal += outMetaSet.halMeta;
    }
    #endif
    PIPE_LOGD("-, reqID=%d", rpRequest->getRequestNo());
    return tuningParam;
}


MBOOL
P2ANode::
prepareEnqueQParam(
    const IspPipeRequestPtr& pRequest,
    QParams& rQParams
)
{
    PIPE_LOGD("reqID=%d", pRequest->getRequestNo());
    MUINT iFrameNum = 0;
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MBOOL bRet = MTRUE;
    MSize szTBD(0, 0);

    // preview YUV with LCE on
    if(mpPipeOption->mbEnableLCE && pRequest->getRequestAttr().reqType == PIPE_REQ_PREVIEW)
    {
        PIPE_LOGD("reqID=%d preview pass", pRequest->getRequestNo());
        IImageBuffer *pImgBuf_RSRaw = pBufferHandler->requestBuffer(getNodeId(), BID_IN_RSRAW_MAIN1);
        QParamTemplateGenerator qTemplate(iFrameNum++, miSensorIdx_Main1, ENormalStreamTag_Normal);
        qTemplate.addInput(PORT_IMGI).
                  addInput(PORT_LCEI).
                  addCrop(eCROP_WROT, MPoint(0,0), szTBD, szTBD). //fill in runtime
                  addOutput(PORT_WROTO);

        bRet &= qTemplate.generate(rQParams);
    }

    // thrid-party input Main1/Main2 YUV
    {
        PIPE_LOGD("reqID=%d depth pass", pRequest->getRequestNo());
        IImageBuffer *pImgBuf_RSRaw_Main1 = pBufferHandler->requestBuffer(getNodeId(), BID_IN_RSRAW_MAIN1);
        IImageBuffer *pImgBuf_RSRaw_Main2 = pBufferHandler->requestBuffer(getNodeId(), BID_IN_RSRAW_MAIN2);
        IImageBuffer *pImgBuf_YUV_Main1 = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN1);
        IImageBuffer *pImgBuf_YUV_Main2 = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN2);

        // Main1 section
        QParamTemplateGenerator tmpleGen =
                    QParamTemplateGenerator(iFrameNum++, miSensorIdx_Main1, ENormalStreamTag_Normal).
                        addInput(PORT_IMGI).
                        addCrop(eCROP_WROT, MPoint(0, 0), pImgBuf_RSRaw_Main1->getImgSize(), pImgBuf_YUV_Main1->getImgSize()).
                        addOutput(PORT_WROTO);
        // generate FD buffer
        if(pRequest->isRequestBuffer(BID_PIPE_OUT_PV_FD))
        {
            IImageBuffer *pImgBuf_FD = pBufferHandler->requestBuffer(getNodeId(), BID_PIPE_OUT_PV_FD);
            tmpleGen.addCrop(eCROP_CRZ, MPoint(0, 0), pImgBuf_RSRaw_Main1->getImgSize(), pImgBuf_FD->getImgSize()).
                        addOutput(PORT_IMG2O);
        }
        bRet &= tmpleGen.generate(rQParams);

        // Main2 section
        bRet &= QParamTemplateGenerator(iFrameNum++, miSensorIdx_Main2, ENormalStreamTag_Normal).
                        addInput(PORT_IMGI).
                        addCrop(eCROP_WROT, MPoint(0, 0), pImgBuf_RSRaw_Main2->getImgSize(), pImgBuf_YUV_Main2->getImgSize()).
                        addOutput(PORT_WROTO).
                        generate(rQParams);
    }

    if(!bRet)
    {
        MY_LOGE("QParamTemplateGenerator generation failure!!");
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
P2ANode::
buildEnqueQParam(
    const IspPipeRequestPtr& pRequest,
    QParams& rQParams
)
{
    PIPE_LOGD("reqID=%d", pRequest->getRequestNo());
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // Fill template
    QParamTemplateFiller qParamFiller(rQParams);
    MUINT iFrameNum = 0;

    // preview YUV with LCE on
    if(mpPipeOption->mbEnableLCE && pRequest->getRequestAttr().reqType == PIPE_REQ_PREVIEW)
    {
        PIPE_LOGD("reqID=%d preview pass", pRequest->getRequestNo());
        TuningParam tuning = applyISPTuning(eP2_RESIZE, pRequest);
        IImageBuffer *pImgBuf_RSRaw = pBufferHandler->requestBuffer(getNodeId(), BID_IN_RSRAW_MAIN1);
        IImageBuffer *pImgBuf_YUV = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_PV_YUV);

        MCropRect crop;
        if(calcDstImageCrop(pImgBuf_RSRaw->getImgSize(), pImgBuf_YUV->getImgSize(), crop))
        {
            qParamFiller.setCrop(iFrameNum, eCROP_WROT, crop.p_integral, crop.s, pImgBuf_YUV->getImgSize()).
                    insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_YUV);
        }
        else
        {
            MY_LOGE("Failed to calculate crop info.");
            return MFALSE;
        }


        // check LCSO exist
        if(!mpPipeOption->mbEnableLCE || tuning.pLcsBuf == nullptr)
            qParamFiller.delInputPort(iFrameNum, PORT_LCEI);
        else
        {
            IImageBuffer* pLCSOBuf = pBufferHandler->requestBuffer(getNodeId(), PBID_IN_LCSO_MAIN1);
            qParamFiller.insertInputBuf(iFrameNum, PORT_LCEI, pLCSOBuf);
        }

        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, pImgBuf_RSRaw).
                    insertTuningBuf(iFrameNum, tuning.pRegBuf).
                    setCropResize(iFrameNum, eCROP_WROT, pImgBuf_YUV->getImgSize()).
                    insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_YUV);
        iFrameNum++;
    }

    // thrid-party input Main2 YUV
    {
        PIPE_LOGD("reqID=%d depth pass, hadFD:%d", pRequest->getRequestNo(), pRequest->isRequestBuffer(BID_PIPE_OUT_PV_FD));
        IImageBuffer *pImgBuf_RSRaw_Main1 = pBufferHandler->requestBuffer(getNodeId(), BID_IN_RSRAW_MAIN1);
        IImageBuffer *pImgBuf_RSRaw_Main2 = pBufferHandler->requestBuffer(getNodeId(), BID_IN_RSRAW_MAIN2);
        IImageBuffer *pImgBuf_YUV_Main1 = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN1);
        IImageBuffer *pImgBuf_YUV_Main2 = pBufferHandler->requestBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN2);
        TuningParam tuning_main1 = applyISPTuning(eP2_DPE_MAIN1, pRequest);
        TuningParam tuning_main2 = applyISPTuning(eP2_DPE_MAIN2, pRequest);

        // Main1 section
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, pImgBuf_RSRaw_Main1).
                        insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_YUV_Main1).
                        insertTuningBuf(iFrameNum, tuning_main1.pRegBuf);
        // insert FD buffer
        if(pRequest->isRequestBuffer(BID_PIPE_OUT_PV_FD))
        {
            IImageBuffer *pImgBuf_FD = pBufferHandler->requestBuffer(getNodeId(), BID_PIPE_OUT_PV_FD);
            qParamFiller.insertOutputBuf(iFrameNum, PORT_IMG2O, pImgBuf_FD);
        }
        iFrameNum++;
        // Main2 section
        qParamFiller.insertInputBuf(iFrameNum, PORT_IMGI, pImgBuf_RSRaw_Main2).
                        insertOutputBuf(iFrameNum, PORT_WROTO, pImgBuf_YUV_Main2).
                        insertTuningBuf(iFrameNum, tuning_main2.pRegBuf);
    }

    if(!qParamFiller.validate())
    {
        MY_LOGE("reqID=%d Failed to build QParam!", pRequest->getRequestNo());
        return MFALSE;
    }
    else
        return MTRUE;


}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
