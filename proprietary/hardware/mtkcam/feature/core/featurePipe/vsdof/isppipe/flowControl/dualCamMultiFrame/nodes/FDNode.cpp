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
#include <utils/Mutex.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
// Module header file

// Local header file
#include "FDNode.h"
#include "../../../IspPipe_Common.h"

// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "FDNode"
#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/vsdof/util/vsdof_util.h>

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using namespace VSDOF::util;
// jpeg roataion
#define ANGLE_DEGREE_0 0
#define ANGLE_DEGREE_90 90
#define ANGLE_DEGREE_180 180
#define ANGLE_DEGREE_270 270
//
const MUINT32 FD_WORKING_BUF_SIZE = 1024*1024*5;
const MUINT32 DDP_WORKING_BUF_SIZE = 640*640*2;
const MUINT32 EXTRACT_Y_WORKING_BUF_SIZE = 1920*1080;
const MUINT32 FACE_DIMENTION_HALF_WIDTH = 1000;
const MUINT32 FACE_DIMENTION_HALF_HEIGHT = 1000;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FDNode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FDNode::
FDNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: FDNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

FDNode::
FDNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority
)
: IspPipeNode(name, nodeId, config, policy, priority)
{
    SCOPED_TRACER();
    this->addWaitQueue(&mRequestQue);
    mszFDImg = config.mpPipeSetting->mszFDImg;
}


FDNode::
~FDNode()
{
    SCOPED_TRACER();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FDNode Functions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
FDNode::
onInit()
{
    SCOPED_TRACER();
    return MTRUE;;
}

MBOOL
FDNode::
onUninit()
{
    SCOPED_TRACER();
    return MTRUE;
}

MBOOL
FDNode::
onThreadStart()
{
    AutoProfileLogging profile("MDPNode::onThreadStart");
    // create FDHAL
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    // allocate FD working buffer
    mpFDWorkingBuffer = new unsigned char[FD_WORKING_BUF_SIZE];
    mpDDPBuffer = new unsigned char[DDP_WORKING_BUF_SIZE];
    mpExtractYBuffer = new unsigned char[EXTRACT_Y_WORKING_BUF_SIZE];
    // create metadata
    mpDetectedFaces = new MtkCameraFaceMetadata();
    if ( nullptr != mpDetectedFaces )
    {
        mpDetectedFaces->number_of_faces = 0;
    }
    //
    mpDpStream = new DpBlitStream();
    return MTRUE;
}

MBOOL
FDNode::
onThreadStop()
{
    if(mpFDHalObj != nullptr)
    {
        mpFDHalObj->destroyInstance();
        mpFDHalObj = nullptr;
    }

    if(mpFDWorkingBuffer != nullptr)
    {
        delete mpFDWorkingBuffer;
        mpFDWorkingBuffer = nullptr;
    }

    if(mpDDPBuffer != nullptr)
    {
        delete mpDDPBuffer;
        mpDDPBuffer = nullptr;
    }

    if(mpExtractYBuffer != nullptr)
    {
        delete mpExtractYBuffer;
        mpExtractYBuffer = nullptr;
    }

    if(mpDetectedFaces != nullptr)
    {
        delete mpDetectedFaces;
        mpDetectedFaces = nullptr;
    }

    if(mpDpStream != nullptr)
        delete mpDpStream;

    return MTRUE;
}

MBOOL
FDNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();
    MY_LOGD("reqID=%d", pRequest->getRequestNo());
    MBOOL ret = MTRUE;
    switch(dataID)
    {
        case ROOT_TO_FD:
        {
            mRequestQue.enque(pRequest);
            break;
        }
        default:
            MY_LOGW("Un-recognized dataID ID, id=%s reqID=%d", ID2Name(dataID), pRequest->getRequestNo());
            ret = MFALSE;
            break;
    }
    return ret;
}

MBOOL
FDNode::
handleOutputFrame(IspPipeRequestPtr& pRequest)
{
    SCOPED_TRACER();
    android::Mutex::Autolock lock(mLock);
    if(!mbFDResultReady)
    {
        MY_LOGE("reqID=%d the FD result has not ready! The flow might have some problems.", pRequest->getRequestNo());
        return MFALSE;
    }
    IImageBuffer* pImgBuf_CleanYUV = pRequest->getBufferHandler()->requestBuffer(getNodeId(), BID_OUT_CLEAN_FSYUV);
    // domain tranform, degree transform
    if(!tranformFaceDomain(pImgBuf_CleanYUV->getImgSize()))
    {
        MY_LOGE("reqID=%d tranformFaceDomain failed!", pRequest->getRequestNo());
        return MFALSE;
    }

    // set the request has FD result
    pRequest->getRequestAttr().fdResult = std::make_unique<FDResultInfo>(mLatestFDResult);
    mbFDResultReady = MFALSE;
    return MTRUE;
}

MVOID
FDNode::
onFlush()
{
    SCOPED_TRACER();

    MY_LOGD("extDep=%d", this->getExtThreadDependency());
    IspPipeRequestPtr pRequest;
    while( mRequestQue.deque(pRequest) )
    {
        sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
    IspPipeNode::onFlush();
}

MBOOL
FDNode::
onThreadLoop()
{
    SCOPED_TRACER();
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
    MBOOL isSuccess = MTRUE;
    AutoProfileLogging profile("FDNode::threadLoop", pRequest->getRequestNo());
    // check input frame
    if(pRequest->isRequestBuffer(BID_IN_RSYUV_MAIN2))
    {
        if(!this->generateFDYUV(pRequest) || !this->runFaceDetection(pRequest))
        {
            MY_LOGE("reqID=%d FD execution failed.", pRequest->getRequestNo());
            isSuccess = MFALSE;
            goto lbExit;
        }
    }
    // check output frame
    if(pRequest->isRequestBuffer(BID_OUT_DEPTHMAP))
    {
        MY_LOGD("reqID=%d exist output buffer!", pRequest->getRequestNo());
        if(!this->handleOutputFrame(pRequest))
        {
            MY_LOGE("reqID=%d, Failed to handle output frame!", pRequest->getRequestNo());
            isSuccess = MFALSE;
            goto lbExit;
        }
    }
lbExit:
    if(isSuccess)
    {
        const MBOOL isFDToHDRFlow = pRequest->getRequestAttr().isFDToHDRFlow;
        const MBOOL isFDToDepthFlow = pRequest->getRequestAttr().isFDToDepthFlow;

        MY_LOGD("reqID=%d, dispatch flow, isFDToHDRFlow:%d, isFDToDepthFlow;%d", isFDToHDRFlow, isFDToDepthFlow);
        if(isFDToHDRFlow)
        {
            this->handleData(FD_TO_HDR, pRequest);
            this->handleData(FD_TO_BOKEH, pRequest);
            MY_LOGD("reqID=%d dispatch to next node:FD_TO_HDR and FD_TO_BOKEH", pRequest->getRequestNo());
        }
        else if(isFDToDepthFlow)
        {
            this->handleData(FD_TO_DEPTH, pRequest);
            this->handleData(FD_TO_BOKEH, pRequest);
            MY_LOGD("reqID=%d dispatch to next node:FD_TO_DEPTH and FD_TO_BOKEH", pRequest->getRequestNo());
        }
        else
        {
            this->handleData(FD_TO_BOKEH, pRequest);
            MY_LOGD("reqID=%d dispatch to next node:FD_TO_BOKEH", pRequest->getRequestNo());
        }
    }
    else
    {
        this->handleData(ERROR_OCCUR_NOTIFY, pRequest);
        MY_LOGE("reqID=%d ERROR, dispatch to next node:ERROR_OCCUR_NOTIFY", pRequest->getRequestNo());
    }
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();
    return isSuccess;
}

MBOOL
FDNode::
generateFDYUV(IspPipeRequestPtr& pRequest)
{
    AutoProfileLogging profile("FDNode::generateFDYUV", pRequest->getRequestNo());
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // input main1 resize yuv
    IImageBuffer* pImfBuf_FSYUV = pBufferHandler->requestBuffer(getNodeId(), BID_IN_FSYUV_MAIN1);
    // output: FD YUV
    IImageBuffer* pImfBuf_FDYUV = pBufferHandler->requestBuffer(getNodeId(), BID_FD_INERNAL_YUV);

    MY_LOGD("src buf=%dx%d (pImfBuf_FSYUV) resize size=%dx%d", pImfBuf_FSYUV->getImgSize().w, pImfBuf_FSYUV->getImgSize().h,
                pImfBuf_FDYUV->getImgSize().w, pImfBuf_FDYUV->getImgSize().h);
    // apply MDP utility
    sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = pImfBuf_FSYUV;
    config.pDstBuffer = pImfBuf_FDYUV;
    config.rotAngle = 0;
    if(!excuteMDP(config))
    {
        MY_LOGE("excuteMDP fail: Cannot perform MDP operation.");
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
FDNode::
runFaceDetection(IspPipeRequestPtr& pRequest)
{
    AutoProfileLogging profile("FDNode::runFaceDetection", pRequest->getRequestNo());
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // get input YUV
    IImageBuffer* pImfBuf_FDYUV = nullptr;
    if(!pBufferHandler->getEnqueBuffer(getNodeId(), BID_FD_INERNAL_YUV, pImfBuf_FDYUV))
    {
        MY_LOGE("redID=%d, Failed to get FD YUV buffer!", pRequest->getRequestNo());
        return MFALSE;
    }
    // app meta: get rotation
    MINT32 jpegOrientation;
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation))
    {
        MY_LOGE("reqID=%d, Cannot find MTK_JPEG_ORIENTATION meta!", pRequest->getRequestNo());
        return MFALSE;
    }
    // FD HAL init (need per-frame run)
    MINT32 bErr = mpFDHalObj->halFDInit(mszFDImg.w, mszFDImg.h,
                            mpFDWorkingBuffer, FD_WORKING_BUF_SIZE, 1, 0, FDResultInfo::MAX_DETECT_FACE_NUM);
    if(bErr)
    {
        MY_LOGE("FDHal Init Failed! status:%d", bErr);
        return MFALSE;
    }

    MY_LOGD("halFTBufferCreate, reqId=%d", pRequest->getRequestNo());

    MUINT8* pSrcBuf = (MUINT8*) pImfBuf_FDYUV->getBufVA(0);
    bErr = mpFDHalObj->halFTBufferCreate(mpDDPBuffer, pSrcBuf, pImfBuf_FDYUV->getPlaneCount(),
                                            mszFDImg.w, mszFDImg.h);
    bErr |= mpFDHalObj->halFDYUYV2ExtractY(mpExtractYBuffer, pSrcBuf, mszFDImg.w, mszFDImg.h);

    MY_LOGD("halFDDo start, reqId=%d   %x  %x   %d  %x", pRequest->getRequestNo(),
                    mpDDPBuffer,  mpExtractYBuffer, jpegOrientation, pImfBuf_FDYUV->getBufPA(0));

    struct FD_Frame_Parameters Param;
    Param.pScaleImages = NULL;
    Param.pRGB565Image = (MUINT8 *)mpDDPBuffer;
    Param.pPureYImage  = (MUINT8 *)mpExtractYBuffer;
    Param.pImageBufferVirtual = (MUINT8 *)pImfBuf_FDYUV->getBufVA(0);
    Param.pImageBufferPhyP0 = (MUINT8 *)pImfBuf_FDYUV->getBufPA(0);
    Param.pImageBufferPhyP1 = NULL;
    Param.pImageBufferPhyP2 = NULL;
    Param.Rotation_Info = jpegOrientation;
    Param.SDEnable = 0;
    Param.AEStable = 0;
    bErr |= mpFDHalObj->halFDDo(Param);

    if(bErr)
    {
        MY_LOGE("FD operations failed!! status: %d", bErr);
        return MFALSE;
    }
    MY_LOGD("reqId=%d, halFDDo Done", pRequest->getRequestNo());
    // get FD result
    {
        android::Mutex::Autolock lock(mLock);
        mbFDResultReady = MTRUE;
        mLatestFDResult.miFaceNum = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
        MY_LOGD("reqId=%d, face num =%d", pRequest->getRequestNo(), mLatestFDResult.miFaceNum);
        for(int index=0;index<mLatestFDResult.miFaceNum;++index)
        {
            MtkCameraFace& inputFaceInfo = mpDetectedFaces->faces[index];
            // face: left top right bottom
            MPoint startPt(inputFaceInfo.rect[0], inputFaceInfo.rect[1]);
            int width = inputFaceInfo.rect[2]-inputFaceInfo.rect[0];
            int height = inputFaceInfo.rect[3]-inputFaceInfo.rect[1];
            mLatestFDResult.mFaceRegion[index] = MRect(startPt, MSize(width, height));
            mLatestFDResult.mRIPDegree[index] = mpDetectedFaces->fld_rip[index];
        }
    }
    //FD Hal uninit
    mpFDHalObj->halFDUninit();
    return MTRUE;
}

MBOOL
FDNode::
tranformFaceDomain(
    const MSize& domainSize
)
{
    // FD original domain -1000 ~ 1000, origin is centre
    // new centre is left-top.
    auto domainTransform_X = [&domainSize] (int xPos)
                            {
                                int newCentre = FACE_DIMENTION_HALF_WIDTH + xPos;
                                int newPos = 1.0 * newCentre/(2*FACE_DIMENTION_HALF_WIDTH) * domainSize.w;
                                return newPos;
                            };
    auto domainTransform_Y = [&domainSize] (int yPos)
                            {
                                int newCentre = FACE_DIMENTION_HALF_HEIGHT + yPos;
                                int newPos = 1.0 * newCentre/(2*FACE_DIMENTION_HALF_HEIGHT) * domainSize.h;
                                return newPos;
                            };
    MY_LOGD("detect face num=%d domainSize=%dx%d", mLatestFDResult.miFaceNum, domainSize.w, domainSize.h);
    for(auto i=0;i<mLatestFDResult.miFaceNum;++i)
    {
        MRect& fdRegion = mLatestFDResult.mFaceRegion[i];
        MY_LOGD("index: %d, origin rect=(%d,%d) %dx%d", i, fdRegion.p.x, fdRegion.p.y, fdRegion.s.w, fdRegion.s.h);
        fdRegion.p.x = domainTransform_X(fdRegion.p.x);
        fdRegion.p.y = domainTransform_Y(fdRegion.p.y);
        fdRegion.s.w = 1.0 * fdRegion.s.w/(2*FACE_DIMENTION_HALF_WIDTH) * domainSize.w;
        fdRegion.s.h = 1.0 * fdRegion.s.h/(2*FACE_DIMENTION_HALF_HEIGHT) * domainSize.h;
        MY_LOGD("index: %d, after rect=(%d,%d) %dx%d", i, fdRegion.p.x, fdRegion.p.y, fdRegion.s.w, fdRegion.s.h);
        // degree change from -30, 0, 30  to  30, 0, 330
         mLatestFDResult.mRIPDegree[i] = (-1 * mLatestFDResult.mRIPDegree[i] + 360) % 360;
        MY_LOGD("index: %d, RIP=%d", i, mLatestFDResult.mRIPDegree[i]);
    }
    return MTRUE;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam