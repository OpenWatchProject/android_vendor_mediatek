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

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/OweUtility.h>
#include <stereo_tuning_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file
// Local header file
#include "WMFNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeUtils.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
// Logging header file
#define PIPE_CLASS_TAG "WMFNode"
#include <featurePipe/core/include/PipeLog.h>

using namespace StereoHAL;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define WMF_USER_NAME "DEPTHPIP_WMF"

WMFNode::
WMFNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
{
    this->addWaitQueue(&mJobQueue);
}

WMFNode::
~WMFNode()
{
    MY_LOGD("[Destructor]");
}

MVOID
WMFNode::
cleanUp()
{
    VSDOF_LOGD("+");
    if(mpOweStream != NULL)
    {
        mpOweStream->uninit();
        mpOweStream->destroyInstance(WMF_USER_NAME);
        mpOweStream = NULL;
    }
    mJobQueue.clear();
    mvWMFECtrl.clear();
    // release Tbli buffer pool
    mvTbliImgBuf.clear();
    FatImageBufferPool::destroy(mpTbliImgBufPool);
    VSDOF_LOGD("-");
}

MBOOL
WMFNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WMFNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WMFNode::
onThreadStart()
{
    CAM_TRACE_NAME("WMFNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // init DPEStream
    mpOweStream = IEgnStream<OWMFEConfig>::createInstance(WMF_USER_NAME);
    if(mpOweStream == NULL)
    {
        MY_LOGE("OWE WMF Stream create instance failed!");
        return MFALSE;
    }
    MBOOL bRet = mpOweStream->init();
    if(!bRet)
    {
        MY_LOGE("OWE WFM Stream init failed!!");
        return MFALSE;
    }
    // Tbli buffer pool
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    MUINT32 iWMFInputWidth = pSizeProvder->getBufferSize(E_MY_S, eSTEREO_SCENARIO_RECORD).size.w;
    MUINT32 iTbliStride = OWEQueryInDMAStride(DMA_OWMFE_TBLI, WMFE_DPI_D_FMT, iWMFInputWidth);
    mpTbliImgBufPool = FatImageBufferPool::create("TbliBufPool", iTbliStride, 1,
                                        eImgFmt_Y8, FatImageBufferPool::USAGE_SW|eBUFFER_USAGE_HW_CAMERA_READWRITE);
    // allocate one buffer
    if(mpTbliImgBufPool == nullptr || !mpTbliImgBufPool->allocate(3))
    {
        MY_LOGE("mpTbliImgBufPool create or allocate failed!!");
        return MFALSE;
    }
    // get Tbli tuning buffer
    mvTbliImgBuf.push_back(mpTbliImgBufPool->request());
    mvTbliImgBuf.push_back(mpTbliImgBufPool->request());
    mvTbliImgBuf.push_back(mpTbliImgBufPool->request());
    // WMF control
    mvWMFECtrl.push_back(NSIoPipe::OWMFECtrl());
    mvWMFECtrl.push_back(NSIoPipe::OWMFECtrl());
    mvWMFECtrl.push_back(NSIoPipe::OWMFECtrl());
    // query tuning mgr to get Tbli + filter size
    vector<void*> vTbli = { (void*)mvTbliImgBuf[0]->mImageBuffer->getBufVA(0),
                            (void*)mvTbliImgBuf[1]->mImageBuffer->getBufVA(0),
                            (void*)mvTbliImgBuf[2]->mImageBuffer->getBufVA(0)};
    StereoTuningProvider::getWMFTuningInfo(mvWMFECtrl, vTbli);
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WMFNode::
onThreadStop()
{
    CAM_TRACE_NAME("WMFNode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
WMFNode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    MBOOL ret=MTRUE;
    VSDOF_LOGD("+, dataID=%d reqId=%d", data, pRequest->getRequestNo());

    switch(data)
    {
        case HWOCC_TO_WMF_NOC:
        case OCC_TO_WMF_OMYSN:
            VSDOF_PRFLOG("+ : reqID=%d size=%d", pRequest->getRequestNo(), mJobQueue.size());
            mJobQueue.enque(pRequest);
            break;
        default:
            ret = MFALSE;
            break;
    }

    VSDOF_LOGD("-");
    return ret;
}

MBOOL
WMFNode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;

    if( !waitAnyQueue() )
    {
        return MFALSE;
    }
    CAM_TRACE_NAME("WMFNode::onThreadLoop");

    if( !mJobQueue.deque(pRequest) )
    {
        MY_LOGD("mJobQueue.deque() failed");
        return MFALSE;
    }
    return performWMFOperation(pRequest);
}

MVOID
WMFNode::
debugWMFParam(EGNParams<OWMFEConfig> &enqConfig)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    OWMFEConfig config = enqConfig.mEGNConfigVec[0];

    #define LOG_BUFFER_INFO(buff_info)\
        MY_LOGD(#buff_info ".dmaport=%d", buff_info.dmaport);\
        MY_LOGD(#buff_info ".memID=%d", buff_info.memID);\
        MY_LOGD(#buff_info ".u4BufVA=%x", buff_info.u4BufVA);\
        MY_LOGD(#buff_info ".u4BufPA=%x", buff_info.u4BufPA);\
        MY_LOGD(#buff_info ".u4BufSize=%d", buff_info.u4BufSize);\
        MY_LOGD(#buff_info ".u4Stride=%d", buff_info.u4Stride);

    for (unsigned int j=0; j<config.mWMFECtrlVec.size(); j++)
    {
        OWMFECtrl ctrl = config.mWMFECtrlVec[j];

        MY_LOGD("========== Wmfe_Ctrl_%d section==============", j);
        MY_LOGD("Wmfe_Enable=%d", ctrl.Wmfe_Enable);
        MY_LOGD("WmfeFilterSize=%d", ctrl.WmfeFilterSize);
        MY_LOGD("Wmfe_Width=%d", ctrl.Wmfe_Width);
        MY_LOGD("Wmfe_Height=%d", ctrl.Wmfe_Height);
        MY_LOGD("WmfeImgiFmt=%d", ctrl.WmfeImgiFmt);
        MY_LOGD("WmfeDpiFmt=%d", ctrl.WmfeDpiFmt);
        MY_LOGD("Wmfe_Dpnd_En=%d", ctrl.Wmfe_Dpnd_En);
        MY_LOGD("Wmfe_Mask_En=%d", ctrl.Wmfe_Mask_En);
        MY_LOGD("WmfeHorzScOrd=%d", ctrl.WmfeHorzScOrd);
        MY_LOGD("WmfeVertScOrd=%d", ctrl.WmfeVertScOrd);
        MY_LOGD("Wmfe_Mask_Value=%d", ctrl.Wmfe_Mask_Value);
        MY_LOGD("Wmfe_Mask_Mode=%d", ctrl.Wmfe_Mask_Mode);

        LOG_BUFFER_INFO(ctrl.Wmfe_Imgi);
        LOG_BUFFER_INFO(ctrl.Wmfe_Dpi);
        LOG_BUFFER_INFO(ctrl.Wmfe_Maski);
        LOG_BUFFER_INFO(ctrl.Wmfe_Tbli);
        LOG_BUFFER_INFO(ctrl.Wmfe_Dpo);


        MY_LOGD("Tbli: ");
        short *addr = (short*) ctrl.Wmfe_Tbli.u4BufVA;
        int index = 0;
        while(index < 128)
        {
          MY_LOGD("%d \t%d \t%d \t%d \t%d \t%d \t%d \t%d \n",
                     addr[index], addr[index+1], addr[index+2], addr[index+3],
                     addr[index+4], addr[index+5], addr[index+6], addr[index+7]);
          index +=8;
        }
    }
}

MBOOL
WMFNode::
performWMFOperation(DepthMapRequestPtr& pRequest)
{
    CAM_TRACE_NAME("WMFNode::performWMFOperation");
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_LOGD("+, reqID=%d", pRequest->getRequestNo());
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    EnqueCookieContainer *pEnqueCookie = new EnqueCookieContainer(pRequest, this);

    // enque param
    EGNParams<OWMFEConfig> enqueParams;
    // prepare the enque params
    if(!prepareWMFEnqueConfig(pBufferHandler, enqueParams))
    {
        AEE_ASSERT("[WMFNode]Failed to prepare the EnqueConfig!!");
        return MFALSE;
    }
    enqueParams.mpfnCallback = onWMFEnqueDone;
    enqueParams.mpCookie = (void*) pEnqueCookie;
    // debug params
    debugWMFParam(enqueParams);

    // timer
    pRequest->mTimer.startWMF();
    pRequest->mTimer.startWMFEnque();
    CAM_TRACE_BEGIN("WMFNode::WMFEEnque");
    MBOOL bRet = mpOweStream->EGNenque(enqueParams);
    //
    pRequest->mTimer.stopWMFEnque();
    VSDOF_PRFLOG("WMFE enque start - first pass, reqID=%d, config-time=%d ms",
                    pRequest->getRequestNo(), pRequest->mTimer.getElapsedWMFEnque());
    CAM_TRACE_END();
    if(!bRet)
    {
        MY_LOGE("WMF enque failed!!");
        goto lbExit;
    }
    return MTRUE;

lbExit:
    delete pEnqueCookie;
    // mark on-going-request end
    this->decExtThreadDependency();
    return MFALSE;
}

MVOID
WMFNode::
onWMFEnqueDone(EGNParams<OWMFEConfig>& rParams)
{
    EnqueCookieContainer* pEnqueCookie = reinterpret_cast<EnqueCookieContainer*>(rParams.mpCookie);
    WMFNode* pWMFNode = reinterpret_cast<WMFNode*>(pEnqueCookie->mpNode);
    pWMFNode->handleWMFEnqueDone(rParams, pEnqueCookie);
}

MVOID
WMFNode::
handleWMFEnqueDone(
    EGNParams<OWMFEConfig>& rParams,
    EnqueCookieContainer* pEnqueCookie
)
{
    CAM_TRACE_NAME("WMFNode::handleWMFEnqueDone");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MUINT32 iReqNo =  pRequest->getRequestNo();
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // timer
    pRequest->mTimer.stopWMF();
    VSDOF_PRFLOG("+, reqID=%d exec-time=%d msec", iReqNo, pRequest->mTimer.getElapsedWMF());

    if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
    {
        pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_OCC, eDPETHMAP_PIPE_NODEID_GF);
        pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_NOC, eDPETHMAP_PIPE_NODEID_GF);
        pBufferHandler->configOutBuffer(getNodeId(), BID_WMF_OUT_DMW, eDPETHMAP_PIPE_NODEID_GF);
        handleDataAndDump(WMF_TO_GF_OND, pRequest);
    }
    else
    {
        // config MY_S/DMW to GF Node
        pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_GF);
        pBufferHandler->configOutBuffer(getNodeId(), BID_WMF_OUT_DMW, eDPETHMAP_PIPE_NODEID_GF);
        // get buffer and invalidate
        IImageBuffer* pImgBuf_MY_S = nullptr, *pImgBuf_DMW = nullptr;
        pBufferHandler->getEnqueBuffer(getNodeId(), BID_WMF_OUT_DMW, pImgBuf_DMW);
        pImgBuf_DMW->syncCache(eCACHECTRL_INVALID);
        // handle data and dump
        handleDataAndDump(WMF_TO_GF_DMW_MY_S, pRequest);
    }

lbExit:
    // launch onProcessDone
    pBufferHandler->onProcessDone(getNodeId());
    delete pEnqueCookie;
    // mark on-going-request end
    this->decExtThreadDependency();
}

MBOOL
WMFNode::
prepareWMFECtrl(
    IImageBuffer* pSrcImgBuf,
    IImageBuffer* pDepthMaskBuf,
    IImageBuffer* pOutputBuf,
    IImageBuffer* pTbliBuf,
    OWMFECtrl& rWMFCtrl
)
{
    VSDOF_LOGD("pSrcImgBuf=%x pDepthMaskBuf=%x pOutputBuf=%x pTbliBuf=%x", pSrcImgBuf, pDepthMaskBuf, pOutputBuf,pTbliBuf);
    //
    rWMFCtrl.Wmfe_Enable = true;
    rWMFCtrl.Wmfe_Width = (mWMFInputChannel == E_WMF_Y_CHANNEL) ? pSrcImgBuf->getImgSize().w : pSrcImgBuf->getImgSize().w/2;
    rWMFCtrl.Wmfe_Height = (mWMFInputChannel == E_WMF_Y_CHANNEL) ? pSrcImgBuf->getImgSize().h : pSrcImgBuf->getImgSize().h/2 ;

    // IMGI FMT setting
    if(pSrcImgBuf->getImgFormat() == eImgFmt_YV12 || pSrcImgBuf->getImgFormat() == eImgFmt_Y8)
    {
        rWMFCtrl.WmfeImgiFmt = OWE_IMGI_Y_FMT;
        // chroma config enable check
        if(rWMFCtrl.Wmfe_Chroma_En!=0)
        {
            MY_LOGW("WMFE tuning non-consistent! imgi is y-only, but Wmfe_Chroma_En=1!");
            rWMFCtrl.Wmfe_Chroma_En = 0;
        }
    }
    else if(pSrcImgBuf->getImgFormat() == eImgFmt_YUY2)
        rWMFCtrl.WmfeImgiFmt = OWE_IMGI_YC_FMT;
    else
    {
        MY_LOGE("WMF CTRL:IMGI Format not-suporrted!! ImageFormat=%x", pSrcImgBuf->getImgFormat());
        return MFALSE;
    }
    // DPI fmt
    rWMFCtrl.WmfeDpiFmt = OWMFE_DPI_D_FMT;
    // input buffer IMGI
    MUINT8 index = (mWMFInputChannel == E_WMF_Y_CHANNEL) ? 0
                   :(mWMFInputChannel == E_WMF_U_CHANNEL) ? 1
                   : 2;
    setupOWEBufInfo(DMA_OWMFE_IMGI, pSrcImgBuf, rWMFCtrl.Wmfe_Imgi, index);
    // input buffer DPI
    setupOWEBufInfo(DMA_OWMFE_DPI, pDepthMaskBuf, rWMFCtrl.Wmfe_Dpi);
    // input buffer MASKI
    setupOWEBufInfo(DMA_OWMFE_MASKI, pDepthMaskBuf, rWMFCtrl.Wmfe_Maski);
    // input buffer TBLI
    setupOWEBufInfo(DMA_OWMFE_TBLI, pTbliBuf, rWMFCtrl.Wmfe_Tbli);
    // output buffer DPO
    setupOWEBufInfo(DMA_OWMFE_DPO, pOutputBuf, rWMFCtrl.Wmfe_Dpo);

    return MTRUE;
}

MBOOL
WMFNode::
prepareWMFEnqueConfig(
    sp<BaseBufferHandler> pBufferHandler,
    EGNParams<OWMFEConfig> &rEnqParam
)
{
    OWMFEConfig wmfeconfig;
    MBOOL bRet = MTRUE;
    IImageBuffer *pImgBuf_SrcImg = nullptr, *pImgBuf_SrcDepthMask = nullptr;
    IImageBuffer *pImgBuf_HoleFillOut_1st = nullptr, *pImgBuf_HoleFillOut_2nd, *pImgBuf_DMW = nullptr;
    OWMFECtrl ctrl_0 = mvWMFECtrl[E_HOLE_FILLING_ROUND_1];
    OWMFECtrl ctrl_1 = mvWMFECtrl[E_HOLE_FILLING_ROUND_2];
    OWMFECtrl ctrl_2 = mvWMFECtrl[E_WMF_ROUND];

    // get input buffers
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_MY_S, pImgBuf_SrcImg);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_NOC, pImgBuf_SrcDepthMask);
    pImgBuf_SrcImg->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_SrcDepthMask->syncCache(eCACHECTRL_FLUSH);

    if(!bRet)
    {
        MY_LOGE("Failed to get WMF input buffers!");
        return MFALSE;
    }
    // output  - first run hole-filling
    pImgBuf_HoleFillOut_1st = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_HOLEFILL_INTERNAL);
    // parpare control
    bRet = this->prepareWMFECtrl(pImgBuf_SrcImg, pImgBuf_SrcDepthMask, pImgBuf_HoleFillOut_1st,
                                    mvTbliImgBuf[E_HOLE_FILLING_ROUND_1]->mImageBuffer.get(), ctrl_0);
    // push
    wmfeconfig.mWMFECtrlVec.push_back(ctrl_0);

    // to enable the third run is controlled by tuning parameter
    MBOOL isNeedThirdRun = mvWMFECtrl[E_WMF_ROUND]. Wmfe_Enable;
    // output  - second run hole-filling
    if(isNeedThirdRun)
        pImgBuf_HoleFillOut_2nd = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_DMW_INTERNAL);
    else
        pImgBuf_HoleFillOut_2nd = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_OUT_DMW);
    // prepare control
    bRet &= this->prepareWMFECtrl(pImgBuf_SrcImg, pImgBuf_HoleFillOut_1st, pImgBuf_HoleFillOut_2nd,
                                    mvTbliImgBuf[E_HOLE_FILLING_ROUND_2]->mImageBuffer.get(), ctrl_1);
    wmfeconfig.mWMFECtrlVec.push_back(ctrl_1);

    // output  - third wmf run
    if(isNeedThirdRun)
    {
        pImgBuf_DMW = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_OUT_DMW);
        // prepare control
        bRet &= this->prepareWMFECtrl(pImgBuf_SrcImg, pImgBuf_HoleFillOut_2nd, pImgBuf_DMW,
                                        mvTbliImgBuf[E_WMF_ROUND]->mImageBuffer.get(), ctrl_2);
        wmfeconfig.mWMFECtrlVec.push_back(ctrl_2);
    }

    if(!bRet)
    {
        MY_LOGE("Failed to prepare the OWMFECtrl!");
        return MFALSE;
    }

    rEnqParam.mpEngineID = eWMFE;
    // push enque param
    rEnqParam.mEGNConfigVec.push_back(wmfeconfig);

    #define DEBUG_BUFFER_SETUP(buf) \
        if(buf!=NULL)\
        {\
            MY_LOGD("DPE buf:" # buf);\
            MY_LOGD("Image buffer size=%dx%d:", buf->getImgSize().w, buf->getImgSize().h);\
            MY_LOGD("Image buffer format=%x", buf->getImgFormat());\
        }\

    // debug section
    if(DepthPipeLoggingSetup::mbDebugLog)
    {
        DEBUG_BUFFER_SETUP(pImgBuf_SrcImg);
        DEBUG_BUFFER_SETUP(pImgBuf_SrcDepthMask);
        DEBUG_BUFFER_SETUP(pImgBuf_HoleFillOut_1st);
        DEBUG_BUFFER_SETUP(pImgBuf_HoleFillOut_2nd);
        DEBUG_BUFFER_SETUP(pImgBuf_DMW);
    }

    #undef DEBUG_BUFFER_SETUP

    return MTRUE;
}


MVOID
WMFNode::
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

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam
