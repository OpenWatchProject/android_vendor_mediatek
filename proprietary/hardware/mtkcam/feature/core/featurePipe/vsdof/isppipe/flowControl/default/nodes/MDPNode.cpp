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
#include <vector>
// Android system/core header file

// mtkcam custom header file
// mtkcam global header file
#include <sync/sync.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file
#include <DpBlitStream.h>
// Local header file
#include "MDPNode.h"
// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "MDPNode"
#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/vsdof/util/vsdof_util.h>
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using namespace VSDOF::util;

/*******************************************************************************
* Const Definition
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MDPNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MDPNode::
MDPNode(
    const char *name,
    IspPipeNodeID nodeID,
    const PipeNodeConfigs& config
)
: IspPipeNode(name, nodeID, config)
{
    MY_LOGD("ctor +");
    this->addWaitQueue(&mRequestQue);
    MY_LOGD("ctor -");
}

MDPNode::
~MDPNode()
{
    MY_LOGD("+");
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MDPNode Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
MDPNode::
onInit()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
MDPNode::
onUninit()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
MDPNode::
onThreadStart()
{
    AutoProfileLogging profile("MDPNode::onThreadStart");
    // create MDP stream
    #ifdef ENABLE_ASYNC
    mpAsynBlitStream = new DpAsyncBlitStream();
    #else
    mpDpStream = new DpBlitStream();
    #endif
    return MTRUE;
}

MBOOL
MDPNode::
onThreadStop()
{
    AutoProfileLogging profile("MDPNode::onThreadStop");

    #ifdef ENABLE_ASYNC
    if(mpAsynBlitStream != nullptr)
        delete mpAsynBlitStream;
    #else
    if(mpDpStream != nullptr)
        delete mpDpStream;
    #endif

    return MTRUE;
}

MBOOL
MDPNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    MBOOL ret = MTRUE;
    PIPE_LOGD("+ : reqID=%d", pRequest->getRequestNo());
    switch(dataID)
    {
        case TP_TO_MDP_BOKEHYUV:
        case TP_TO_MDP_PVYUV:
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
MDPNode::
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
MDPNode::
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
    // mark on-going-request start
    this->incExtThreadDependency();
    AutoProfileLogging profile("MDPNode::threadLoop", pRequest->getRequestNo());
    MBOOL bRet = MFALSE;
    // timer
    pRequest->mTimer.startMDP();
    if(pRequest->getRequestAttr().reqType == PIPE_REQ_CAPTURE && (bRet = generateThumbnail(pRequest)))
    {
        this->handleDataAndDump(MDP_OUT_THUMBNAIL, pRequest);
    }
    else if(pRequest->getRequestAttr().reqType == PIPE_REQ_PREVIEW && (bRet = generateYUVs(pRequest)))
    {
        this->handleDataAndDump(MDP_OUT_YUVS, pRequest);
    }
    pRequest->mTimer.stopMDP();
    profile.logging("MDPNode exec-time=%d ms", pRequest->mTimer.getElapsedMDP());

lbExit:
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();
    return bRet;
}

MBOOL
MDPNode::
generateThumbnail(IspPipeRequestPtr pRequest)
{
    AutoProfileLogging profile("MDPNode::generateThumbnail", pRequest->getRequestNo());
    IImageBuffer* pImgBuf_Bokeh = nullptr;

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    //
    MBOOL bRet = pBufferHandler->getEnqueBuffer(getNodeId(), BID_PIPE_OUT_BOKEH_YUV, pImgBuf_Bokeh);
    if(!bRet)
    {
        MY_LOGE("reqID=%d Failed to get Bokeh YUV", pRequest->getRequestNo());
        return MFALSE;
    }

    MDPTarget target;
    target.mpImgBuf = pBufferHandler->requestBuffer(getNodeId(), BID_MDP_OUT_THUMBNAIL_YUV);
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, target.miRotAngle))
    {
        MY_LOGE("reqID=%d, Cannot find MTK_JPEG_ORIENTATION meta!", pRequest->getRequestNo());
        return MFALSE;
    }
    //
    if(executeMDPResize(pImgBuf_Bokeh, &target))
    {
        PIPE_LOGD("reqID=%d, generate thumbnail success!", pRequest->getRequestNo());
        pRequest->setOutputBufferReady(BID_MDP_OUT_THUMBNAIL_YUV);
        return MTRUE;
    }
    else
    {
        MY_LOGE("reqID=%d, Failed to generate thumbnail!", pRequest->getRequestNo());
        return MFALSE;
    }
}

MBOOL
MDPNode::
generateYUVs(IspPipeRequestPtr pRequest)
{
    AutoProfileLogging profile("MDPNode::generateYUVs", pRequest->getRequestNo());
    IImageBuffer* pImgBuf_PV = nullptr;

    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    IspPipeBufferID bufIDs[] = {BID_PIPE_OUT_PV_YUV0, BID_PIPE_OUT_PV_YUV1};
    IspPipeBufferID srcBufferID;
    std::vector<MDPTarget> targetMDP;
    for(int idx=0;idx<2;++idx)
    {
        int tgtIdx = 0;
        if(pRequest->getOutputBufferReady(bufIDs[idx]))
            srcBufferID = bufIDs[idx];
        else if(pRequest->isRequestBuffer(bufIDs[idx]))
        {
            MDPTarget target;
            target.mpImgBuf = pBufferHandler->requestBuffer(getNodeId(), bufIDs[idx]);
            target.mBufID = bufIDs[idx];
            targetMDP.push_back(target);
        }
    }
    // check output requirement
    if(targetMDP.size() == 0)
        return MTRUE;

    MBOOL bRet = pBufferHandler->getEnqueBuffer(getNodeId(), srcBufferID, pImgBuf_PV);
    if(!bRet)
    {
        MY_LOGE("reqID=%d Failed to get PV YUV", pRequest->getRequestNo());
        return MFALSE;
    }
    MDPTarget* target2 = (targetMDP.size() == 1) ? nullptr : &targetMDP[1];
    if(executeMDPResize(pImgBuf_PV, &targetMDP[0], target2))
    {
        PIPE_LOGD("reqID=%d, generate YUV success!", pRequest->getRequestNo());
        // mark buffer ready
        for(MDPTarget target: targetMDP)
            pRequest->setOutputBufferReady(target.mBufID);

        return MTRUE;
    }
    else
    {
        MY_LOGE("reqID=%d, Failed to generate YUV!", pRequest->getRequestNo());
        return MFALSE;
    }
}

DpColorFormat
MDPNode::
mapToDpColorFmt(MINT format)
{
    switch(format)
    {
        case eImgFmt_RGBA8888:
            return DP_COLOR_RGBA8888;
        case eImgFmt_YUY2:
            return DP_COLOR_YUYV;
        case eImgFmt_NV16:
            return DP_COLOR_NV16;
        case eImgFmt_NV21:
            return DP_COLOR_NV21;
        case eImgFmt_YV12:
            return DP_COLOR_YV12;
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y8:
            return DP_COLOR_GREY;
        case eImgFmt_NV12:
            return DP_COLOR_NV12;
        case eImgFmt_YV16:
            return DP_COLOR_YV16;
        default:
            MY_LOGE("unsupported(%x)", format);
    }
    return DP_COLOR_UNKNOWN;
}

MBOOL
MDPNode::
executeMDPResize(
    IImageBuffer* pInputImg,
    MDPTarget* pTarget,
    MDPTarget* pTargetSec
)
{
    IImageBuffer* pOutputImg = pTarget->mpImgBuf;
    IImageBuffer* pOutputImgSec = (pTargetSec == nullptr) ?
                                        nullptr : pTargetSec->mpImgBuf;

    PIPE_LOGD("InputImg=%dx%d plane size=%d",
                pInputImg->getImgSize().w, pInputImg->getImgSize().h, pInputImg->getPlaneCount());
    PIPE_LOGD("pOutputImg=%dx%d plane size=%d  rot=%d",
                pOutputImg->getImgSize().w, pOutputImg->getImgSize().h, pOutputImg->getPlaneCount(), pTarget->miRotAngle);
    if(pOutputImgSec != nullptr)
    {
        PIPE_LOGD("pOutputImgSec=%dx%d plane size=%d rot=%d",
                pOutputImgSec->getImgSize().w, pOutputImgSec->getImgSize().h, pOutputImgSec->getPlaneCount(), pTarget->miRotAngle);
    }

#ifdef ENABLE_ASYNC
    // create MDP job
    uint32_t jobID;
    int32_t fenceID;

    mpAsynBlitStream->createJob(jobID, fenceID);
    mpAsynBlitStream->setConfigBegin(jobID);
    {
        // src
        MINTPTR pVAList[3] = {0};
        MINTPTR pMVAList[3] = {0};
        uint32_t pSizeList[3] = {0};

        for(int idx=0;idx<pInputImg->getPlaneCount();++idx)
        {
            pVAList[idx] = pInputImg->getBufVA(idx);
            pMVAList[idx] = pInputImg->getBufPA(idx);
            pSizeList[idx] = (uint32_t)pInputImg->getBufSizeInBytes(idx);
        }

        mpAsynBlitStream->setSrcBuffer((void**)pVAList, (void**)pMVAList,
                                        pSizeList, pInputImg->getPlaneCount());
        MUINT32 yPitch=pInputImg->getBufStridesInBytes(0);
        MUINT32 uvPitch=(pInputImg->getPlaneCount() > 1) ? pInputImg->getBufStridesInBytes(1):0;
        mpAsynBlitStream->setSrcConfig(pInputImg->getImgSize().w,
                                        pInputImg->getImgSize().h,
                                        yPitch,
                                        uvPitch,
                                        mapToDpColorFmt(pInputImg->getImgFormat()),
                                        DP_PROFILE_FULL_BT601);
        // output number 1

        MINTPTR pVAList_OutA[3] = {0};
        MINTPTR pMVAList_OutA[3] = {0};
        uint32_t pSizeList_OutA[3] = {0};
        for(int idx=0;idx<pOutputImg->getPlaneCount();++idx)
        {
            pVAList_OutA[idx] = pOutputImg->getBufVA(idx);
            pMVAList_OutA[idx] = pOutputImg->getBufPA(idx);
            pSizeList_OutA[idx] = (uint32_t)pOutputImg->getBufSizeInBytes(idx);
        }
        // set rotation
        mpAsynBlitStream->setRotate(0, pTarget->miRotAngle);
        // set dst config
        mpAsynBlitStream->setDstBuffer(0, (void**)pVAList_OutA, (void**)pMVAList_OutA ,pSizeList_OutA,
                                        pOutputImg->getPlaneCount());

        yPitch=pOutputImg->getBufStridesInBytes(0);
        uvPitch=(pOutputImg->getPlaneCount() > 1) ? pOutputImg->getBufStridesInBytes(1):0;
        mpAsynBlitStream->setDstConfig(0, pOutputImg->getImgSize().w,
                                        pOutputImg->getImgSize().h,
                                        yPitch,
                                        uvPitch,
                                        mapToDpColorFmt(pOutputImg->getImgFormat()),
                                        DP_PROFILE_FULL_BT601);
        // output number 2
        if(pOutputImgSec != nullptr)
        {
            MINTPTR pVAList_OutB[3] = {0};
            MINTPTR pMVAList_OutB[3] = {0};
            uint32_t pSizeList_OutB[3] = {0};
            for(int idx=0;idx<pOutputImgSec->getPlaneCount();++idx)
            {
                pVAList_OutA[idx] = pOutputImgSec->getBufVA(idx);
                pMVAList_OutA[idx] = pOutputImgSec->getBufPA(idx);
                pSizeList_OutA[idx] = (uint32_t)pOutputImgSec->getBufSizeInBytes(idx);
            }
             // set rotation
            mpAsynBlitStream->setRotate(1, pTargetSec->miRotAngle);
            // set dst config
            mpAsynBlitStream->setDstBuffer(1, (void**)pVAList_OutB, (void**)pMVAList_OutB,
                                            pSizeList_OutB, pOutputImgSec->getPlaneCount());
            yPitch=pOutputImgSec->getBufStridesInBytes(0);
            uvPitch=(pOutputImgSec->getPlaneCount() > 1) ? pOutputImgSec->getBufStridesInBytes(1):0;
            mpAsynBlitStream->setDstConfig(1, pOutputImgSec->getImgSize().w,
                                        pOutputImgSec->getImgSize().h,
                                        yPitch,
                                        uvPitch,
                                        mapToDpColorFmt(pOutputImgSec->getImgFormat()),
                                        DP_PROFILE_FULL_BT601);
        }
    }
    mpAsynBlitStream->setConfigEnd();
    MBOOL bRet = mpAsynBlitStream->invalidate();
    // wait for fence
    sync_wait(fenceID, -1);
    close(fenceID);
    return bRet;
#else
    sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = pInputImg;
    config.pDstBuffer = pOutputImg;
    config.rotAngle = pTarget->miRotAngle;
    if(!excuteMDP(config))
    {
        MY_LOGE("excuteMDP fail: Cannot perform MDP operation on target1.");
        return MFALSE;
    }

    if(pOutputImgSec != nullptr)
    {
        config.pDstBuffer = pOutputImgSec;
        config.rotAngle = pTarget->miRotAngle;
        if(!excuteMDP(config))
        {
            MY_LOGE("excuteMDP fail: Cannot perform MDP operation on target2.");
            return MFALSE;
        }
    }
    return MTRUE;
#endif
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam