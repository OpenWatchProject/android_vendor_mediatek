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
#include <mtkcam/drv/iopipe/PostProc/DpeUtility.h>
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

using namespace NSCam::NSIoPipe::NSDpe;
using namespace StereoHAL;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define STREAM_USER_NAME "WMFNode"

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
    if(mpDPEStream != NULL)
    {
        mpDPEStream->uninit();
        mpDPEStream->destroyInstance(STREAM_USER_NAME);
        mpDPEStream = NULL;
    }
    mJobQueue.clear();
    mvWMFECtrl.clear();
    // release Tbli buffer pool
    mvTbliImgBuf.clear();
    ImageBufferPool::destroy(mpTbliImgBufPool);
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
    mpDPEStream = NSCam::NSIoPipe::NSDpe::IDpeStream::createInstance(STREAM_USER_NAME);
    if(mpDPEStream == NULL)
    {
        MY_LOGE("DPE Stream create instance failed!");
        return MFALSE;
    }
    MBOOL bRet = mpDPEStream->init();
    if(!bRet)
    {
        MY_LOGE("DPEStream init failed!!");
        return MFALSE;
    }
    // Tbli buffer pool
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    MUINT32 iWMFInputWidth = pSizeProvder->getBufferSize(E_MY_S, eSTEREO_SCENARIO_RECORD).size.w;
    MUINT32 iTbliStride = DPEQueryInDMAStride(DMA_WMFE_TBLI, WMFE_DPI_D_FMT, iWMFInputWidth);
    mpTbliImgBufPool = ImageBufferPool::create("TbliBufPool", iTbliStride, 1,
                                        eImgFmt_Y8, ImageBufferPool::USAGE_SW|eBUFFER_USAGE_HW_CAMERA_READWRITE);
    // allocate one buffer
    if(mpTbliImgBufPool != nullptr)
        mpTbliImgBufPool->allocate(3);
    else
    {
        MY_LOGE("mpTbliImgBufPool create failed!!");
        return MFALSE;
    }
    // get Tbli tuning buffer
    mvTbliImgBuf.push_back(mpTbliImgBufPool->request());
    // WMF control
    mvWMFECtrl.push_back(NSIoPipe::WMFECtrl());
    // query tuning mgr to get Tbli + filter size
    vector<void*> vTbli = {(void*)mvTbliImgBuf[0]->mImageBuffer->getBufVA(0)};
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
debugWMFParam(NSIoPipe::WMFEConfig &config)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;

    #define LOG_BUFFER_INFO(buff_info)\
        MY_LOGD(#buff_info ".dmaport=%d", buff_info.dmaport);\
        MY_LOGD(#buff_info ".memID=%d", buff_info.memID);\
        MY_LOGD(#buff_info ".u4BufVA=%x", buff_info.u4BufVA);\
        MY_LOGD(#buff_info ".u4BufPA=%x", buff_info.u4BufPA);\
        MY_LOGD(#buff_info ".u4BufSize=%d", buff_info.u4BufSize);\
        MY_LOGD(#buff_info ".u4Stride=%d", buff_info.u4Stride);

    for (unsigned int j=0; j<config.mWMFECtrlVec.size(); j++)
    {
        WMFECtrl ctrl = config.mWMFECtrlVec.at(j);

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
    WMFEParams enqueParams;
    NSIoPipe::WMFEConfig wmfConfig;
    // prepare the enque params
    prepareWMFEnqueConfig(pRequest->getRequestAttr().opState, pBufferHandler, wmfConfig);
    enqueParams.mpfnCallback = onWMFEnqueDone_FirstPass;
    enqueParams.mWMFEConfigVec.push_back(wmfConfig);
    enqueParams.mpCookie = (void*) pEnqueCookie;
    // debug params
    debugWMFParam(wmfConfig);

    // timer
    pRequest->mTimer.startWMF();
    pRequest->mTimer.startWMFEnque();
    CAM_TRACE_BEGIN("WMFNode::WMFEEnque");
    MBOOL bRet = mpDPEStream->WMFEenque(enqueParams);
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
onWMFEnqueDone_FirstPass(WMFEParams& rParams)
{
    EnqueCookieContainer* pEnqueCookie = reinterpret_cast<EnqueCookieContainer*>(rParams.mpCookie);
    WMFNode* pWMFNode = reinterpret_cast<WMFNode*>(pEnqueCookie->mpNode);
    pWMFNode->handleWMFEnqueDone_FirstPass(rParams, pEnqueCookie);
}

MVOID
WMFNode::
handleWMFEnqueDone_FirstPass(
    WMFEParams& rParams,
    EnqueCookieContainer* pEnqueCookie
)
{
    CAM_TRACE_NAME("WMFNode::handleWMFEnqueDone_FirstPass");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MUINT32 iReqNo =  pRequest->getRequestNo();
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    // timer
    pRequest->mTimer.stopWMF();
    VSDOF_PRFLOG("+, reqID=%d exec-time=%d msec", iReqNo, pRequest->mTimer.getElapsedWMF());

    if(pRequest->isQueuedDepthRequest(mpPipeOption))
    {
        VSDOF_LOGD("reqID=%d, Store depth info to data storage!.", iReqNo);
        DepthBufferInfo depthInfo;
        // get depth map
        pBufferHandler->getEnquedSmartBuffer(getNodeId(), BID_WMF_OUT_DMW, depthInfo.mpDepthBuffer);
        depthInfo.mpDepthBuffer->mImageBuffer->syncCache(eCACHECTRL_INVALID);
        // get convergence offset
        IMetadata* pOutHalMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_OUT_HAL_QUEUED);
        if(!tryGetMetadata<MFLOAT>(pOutHalMeta, MTK_CONVERGENCE_DEPTH_OFFSET, depthInfo.mfConvOffset))
        {
            MY_LOGE("Failed to get MTK_CONVERGENCE_DEPTH_OFFSET!!");
            return;
        }
        // config magic number
        DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
        IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
        if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, depthInfo.magicNumber)) {
            MY_LOGE("reqID=%d, Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta! of Main1", pRequest->getRequestNo());
            return;
        }
        // push into storage
        mpDepthStorage->push_back(depthInfo);
        // dump buffer
        DumpConfig config("BID_WMF_OUT_DMW_INTO_QUEUE", NULL, MFALSE);
        handleDump(QUEUED_FLOW_DONE, pRequest, &config);
        // notify queued flow type finished
        handleData(QUEUED_FLOW_DONE, pRequest);
    }
    else
    {
        if(pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
        {
            pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_OCC, eDPETHMAP_PIPE_NODEID_GF);
            pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_NOC, eDPETHMAP_PIPE_NODEID_GF);
            pBufferHandler->configOutBuffer(getNodeId(), BID_WMF_OUT_DMW, eDPETHMAP_PIPE_NODEID_GF);
            pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_GF);
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
    WMFECtrl& rWMFCtrl
)
{
    VSDOF_LOGD("pSrcImgBuf=%x pDepthMaskBuf=%x pOutputBuf=%x pTbliBuf=%x", pSrcImgBuf, pDepthMaskBuf, pOutputBuf,pTbliBuf);

    // WMF input channel, default use Y channel, no other channels now due to the input is from OCC which is single plane
    ENUM_WMF_CHANNEL in_channel = E_WMF_Y_CHANNEL;
    //
    rWMFCtrl.Wmfe_Enable = true;
    rWMFCtrl.Wmfe_Width = (in_channel == E_WMF_Y_CHANNEL) ? pSrcImgBuf->getImgSize().w : pSrcImgBuf->getImgSize().w/2;
    rWMFCtrl.Wmfe_Height = (in_channel == E_WMF_Y_CHANNEL) ? pSrcImgBuf->getImgSize().h : pSrcImgBuf->getImgSize().h/2 ;

    // IMGI FMT setting
    if(pSrcImgBuf->getImgFormat() == eImgFmt_YV12 || pSrcImgBuf->getImgFormat() == eImgFmt_Y8)
        rWMFCtrl.WmfeImgiFmt = DPE_IMGI_Y_FMT;
    else if(pSrcImgBuf->getImgFormat() == eImgFmt_YUY2)
        rWMFCtrl.WmfeImgiFmt = DPE_IMGI_YC_FMT;
    else
    {
        MY_LOGE("WMF CTRL:IMGI Format not-suporrted!! ImageFormat=%x", pSrcImgBuf->getImgFormat());
        return MFALSE;
    }
    // DPI fmt
    rWMFCtrl.WmfeDpiFmt = WMFE_DPI_D_FMT;
    // input buffer IMGI
    MUINT8 index = (in_channel == E_WMF_Y_CHANNEL) ? 0
                   :(in_channel == E_WMF_U_CHANNEL) ? 1
                   : 2;
    setupDPEBufInfo(DMA_WMFE_IMGI, pSrcImgBuf, rWMFCtrl.Wmfe_Imgi, index);
    // input buffer DPI
    setupDPEBufInfo(DMA_WMFE_DPI, pDepthMaskBuf, rWMFCtrl.Wmfe_Dpi);
    // input buffer MASKI
    setupDPEBufInfo(DMA_WMFE_MASKI, pDepthMaskBuf, rWMFCtrl.Wmfe_Maski);
    // input buffer TBLI
    setupDPEBufInfo(DMA_WMFE_TBLI, pTbliBuf, rWMFCtrl.Wmfe_Tbli);
    // output buffer DPO
    setupDPEBufInfo(DMA_WMFE_DPO, pOutputBuf, rWMFCtrl.Wmfe_Dpo);

    return MTRUE;
}

MBOOL
WMFNode::
prepareWMFEnqueConfig(
    DepthMapPipeOpState state,
    sp<BaseBufferHandler> pBufferHandler,
    NSIoPipe::WMFEConfig &rWMFConfig
)
{
    MBOOL bRet = MTRUE;
    IImageBuffer *pImgBuf_SrcImg = nullptr, *pImgBuf_SrcDepthMask = nullptr;
    IImageBuffer *pImgBuf_DMW_Internal = nullptr, *pImgBuf_DMW = nullptr;
    // apply same template
    WMFECtrl ctrl_0 = mvWMFECtrl[0];
    WMFECtrl ctrl_1 = mvWMFECtrl[0];
    // SrcImg: use BID_P2A_OUT_MY_S when preview, BID_OCC_OUT_DS_MVY when capture
    if(state == eSTATE_CAPTURE)
        bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_DS_MVY, pImgBuf_SrcImg);
    else
        bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_MY_S, pImgBuf_SrcImg);
    //
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_DMH, pImgBuf_SrcDepthMask);
    pImgBuf_SrcImg->syncCache(eCACHECTRL_FLUSH);
    pImgBuf_SrcDepthMask->syncCache(eCACHECTRL_FLUSH);
    if(!bRet)
    {
        MY_LOGE("Failed to get WMF input buffers!");
        return MFALSE;
    }
    // output  - DMW internal
    pImgBuf_DMW_Internal = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_DMW_INTERNAL);
    // parpare control
    bRet = this->prepareWMFECtrl(pImgBuf_SrcImg, pImgBuf_SrcDepthMask, pImgBuf_DMW_Internal,
                                    mvTbliImgBuf[0]->mImageBuffer.get(), ctrl_0);
    // push
    rWMFConfig.mWMFECtrlVec.push_back(ctrl_0);
    // output  - DMW internal
    pImgBuf_DMW = pBufferHandler->requestBuffer(getNodeId(), BID_WMF_OUT_DMW);
    bRet = this->prepareWMFECtrl(pImgBuf_SrcImg, pImgBuf_DMW_Internal, pImgBuf_DMW,
                                    mvTbliImgBuf[0]->mImageBuffer.get(), ctrl_1);
    // push
    rWMFConfig.mWMFECtrlVec.push_back(ctrl_1);

    if(!bRet)
    {
        MY_LOGE("Failed to prepare the WMFECtrl!");
        return MFALSE;
    }

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
        DEBUG_BUFFER_SETUP(pImgBuf_DMW_Internal);
        DEBUG_BUFFER_SETUP(pImgBuf_DMW);
    }

    #undef DEBUG_BUFFER_SETUP

    return MTRUE;
}

MBOOL
WMFNode::
setupDPEBufInfo(
    NSIoPipe::DPEDMAPort dmaPort,
    IImageBuffer* pImgBuf,
    NSIoPipe::DPEBufInfo& rBufInfo,
    MUINT8 iPlaneIndex)
{
    // plane 0 address
    rBufInfo.memID = pImgBuf->getFD(0);
    rBufInfo.dmaport = dmaPort;
    rBufInfo.u4BufVA = pImgBuf->getBufVA(iPlaneIndex);
    rBufInfo.u4BufPA = pImgBuf->getBufPA(iPlaneIndex);
    rBufInfo.u4BufSize = pImgBuf->getBufSizeInBytes(iPlaneIndex);
    rBufInfo.u4Stride = pImgBuf->getBufStridesInBytes(iPlaneIndex);

    return MTRUE;
}


MVOID
WMFNode::
onFlush()
{
    MY_LOGD("+");
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
