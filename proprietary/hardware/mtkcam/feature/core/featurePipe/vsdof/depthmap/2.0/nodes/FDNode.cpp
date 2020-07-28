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
#include <vector>
#include <algorithm>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
// Local header file
#include "FDNode.h"
#include "../DepthMapPipeUtils.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
// Logging module header
#define PIPE_CLASS_TAG "StereoFDNode"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Global Define
********************************************************************************/
// jpeg roataion
#define ANGLE_DEGREE_0 0
#define ANGLE_DEGREE_90 90
#define ANGLE_DEGREE_180 180
#define ANGLE_DEGREE_270 270
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

const MUINT32 FD_WORKING_BUF_SIZE = 1024*1024*5;
const MUINT32 DDP_WORKING_BUF_SIZE = 640*640*2;
const MUINT32 EXTRACT_Y_WORKING_BUF_SIZE = 1920*1080;
const MUINT DETECT_FACE_NUM = 15;

StereoFDNode::
StereoFDNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
{
    this->addWaitQueue(&mJobQueue);
    mFD_IMG_SIZE = mpFlowOption->getOptionConfig().mFDSize;
}

StereoFDNode::
~StereoFDNode()
{
    MY_LOGD("[Destructor]");
}

MBOOL
StereoFDNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;

}

MBOOL
StereoFDNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
StereoFDNode::
onThreadStart()
{
    CAM_TRACE_NAME("StereoFDNode::onThreadStart");
    VSDOF_INIT_LOG("+");
    // create FDHAL
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    // allocate FD working buffer
    mpFDWorkingBuffer = new unsigned char[FD_WORKING_BUF_SIZE];
    mpDDPBuffer = new unsigned char[DDP_WORKING_BUF_SIZE];
    mpExtractYBuffer = new unsigned char[EXTRACT_Y_WORKING_BUF_SIZE];
    // FD HAL init
    MINT32  bErr;
    bErr = mpFDHalObj->halFDInit(mFD_IMG_SIZE.w, mFD_IMG_SIZE.h,
                            mpFDWorkingBuffer, FD_WORKING_BUF_SIZE, 1, 0);

    if(bErr)
    {
        MY_LOGE("FDHal Init Failed! status:%d", bErr);
        return MFALSE;
    }
    // create metadata
    mpDetectedFaces = new MtkCameraFaceMetadata();
    if ( nullptr != mpDetectedFaces )
    {
        MtkCameraFace *faces = new MtkCameraFace[DETECT_FACE_NUM];
        MtkFaceInfo *posInfo = new MtkFaceInfo[DETECT_FACE_NUM];

        if ( nullptr != faces &&  nullptr != posInfo)
        {
            mpDetectedFaces->faces = faces;
            mpDetectedFaces->posInfo = posInfo;
            mpDetectedFaces->number_of_faces = 0;
        }
    }
    // Json Utils
    mpJsonUtil = new JSON_Util();
    VSDOF_INIT_LOG("-");

    return MTRUE;
}

MBOOL
StereoFDNode::
onThreadStop()
{
    CAM_TRACE_NAME("StereoFDNode::onThreadStop");
    if(mpFDHalObj != nullptr)
    {
        mpFDHalObj->halFDUninit();
        mpFDHalObj->destroyInstance();
        mpFDHalObj = nullptr;
    }

    if(mpJsonUtil != nullptr)
    {
        delete mpJsonUtil;
        mpJsonUtil = nullptr;
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
        delete [] mpDetectedFaces->faces;
        delete [] mpDetectedFaces->posInfo;
        delete mpDetectedFaces;
        mpDetectedFaces = nullptr;
    }
    return MTRUE;
}

MBOOL
StereoFDNode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    MBOOL ret = MTRUE;
    MUINT32 iReqID =  pRequest->getRequestNo();

    VSDOF_LOGD("+ : reqId=%d dataID=%d (N3D)", iReqID, data);
    switch(data)
    {
        case N3D_TO_FD_EXTDATA_MASK:
            {
                android::Mutex::Autolock lock(mJsonMutex);

                mvN3DJsonMap.add(iReqID, String8((char*)pRequest->mpData));
            }
            mergeExtraData(pRequest);
            break;
        case P2A_TO_FD_IMG:
            mJobQueue.enque(pRequest);
            ret = MTRUE;
            break;
        default:
            MY_LOGW("Unrecongnized DataID=%d", data);
            ret = MFALSE;
            break;
    }

    VSDOF_LOGD("-");
    return ret;
}

MBOOL
StereoFDNode::
onThreadLoop()
{
    DepthMapRequestPtr pRequest;

    if(!waitAllQueue())
    {
        return MFALSE;
    }

    if(!mJobQueue.deque(pRequest))
    {
        MY_LOGE("mJobQueue.deque() failed.");
        return MFALSE;
    }
    // mark on-going-request start
    this->incExtThreadDependency();

    VSDOF_PRFLOG("FD threapLoop start, reqId=%d", pRequest->getRequestNo());
    CAM_TRACE_NAME("StereoFDNode::onThreadLoop");

    MBOOL bRet = MTRUE;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // FD input src buffer
    IImageBuffer* pFDBuf = nullptr;
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_FDIMG, pFDBuf);
    MUINT8* pSrcBuf = (MUINT8*) pFDBuf->getBufVA(0);

    // app meta: get rotation
    MINT32 jpegOrientation;
    IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation))
    {
        VSDOF_LOGD("reqID=%d, Cannot find MTK_JPEG_ORIENTATION meta!", pRequest->getRequestNo());
        return MFALSE;
    }

    VSDOF_PRFLOG("halFTBufferCreate, reqId=%d", pRequest->getRequestNo());

    MINT32 bErr;
    // Performa FD ALGO
    CAM_TRACE_BEGIN("StereoFDNode::halFTBufferCreate_Extract");
    bErr = mpFDHalObj->halFTBufferCreate(mpDDPBuffer, pSrcBuf, pFDBuf->getPlaneCount(),
                                    mFD_IMG_SIZE.w, mFD_IMG_SIZE.h);
    bErr |= mpFDHalObj->halFDYUYV2ExtractY(mpExtractYBuffer, pSrcBuf,
                                    mFD_IMG_SIZE.w, mFD_IMG_SIZE.h);
    CAM_TRACE_END();

    //Do FD
    {
        VSDOF_PRFLOG("halFDDo start, reqId=%d   %x  %x   %d  %x", pRequest->getRequestNo(), mpDDPBuffer,  mpExtractYBuffer, jpegOrientation, pFDBuf->getBufPA(0));
    	// timer
        pRequest->mTimer.startFD();
        CAM_TRACE_BEGIN("StereoFDNode::halFDDo");

        struct FD_Frame_Parameters Param;
        Param.pScaleImages = NULL;
        Param.pRGB565Image = (MUINT8 *)mpDDPBuffer;
        Param.pPureYImage  = (MUINT8 *)mpExtractYBuffer;
        Param.pImageBufferVirtual = (MUINT8 *)pFDBuf->getBufVA(0);
        Param.pImageBufferPhyP0 = (MUINT8 *)pFDBuf->getBufPA(0);
        Param.pImageBufferPhyP1 = NULL;
        Param.pImageBufferPhyP2 = NULL;
        Param.Rotation_Info = jpegOrientation;
        Param.SDEnable = 0;
        Param.AEStable = 0;
        bErr |= mpFDHalObj->halFDDo(Param);

        CAM_TRACE_END();
    }
    // stop timer
    pRequest->mTimer.stopFD();
    if(bErr)
    {
        MY_LOGE("FD　operations failed!! status: %d", bErr);
        return MFALSE;
    }
    else
    {
        VSDOF_PRFTIME_LOG("halFDDo Done, reqId=%d exec-time=%d ms", pRequest->getRequestNo(), pRequest->mTimer.getElapsedFD());
    }

    // Get FD result
    CAM_TRACE_BEGIN("StereoFDNode::halFDGetFaceResult");
    int numFace = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
    VSDOF_LOGD("halFDGetFaceResult jpegOrientation=%d numFace=%d", jpegOrientation,  numFace);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("StereoFDNode::calcRotatedFDResult");
    // pack fd result into the structure
    std::vector<FD_DATA_STEREO_T> vFDResult;
    for(int index=0;index<numFace;++index)
    {
        FD_DATA_STEREO_T fdData;
        calcRotatedFDResult(mpDetectedFaces->faces[index], jpegOrientation, fdData);
        vFDResult.push_back(fdData);
    }
    CAM_TRACE_END();
    // gen FD json string
    char *sJsonStr = mpJsonUtil->facesToJSON(vFDResult);
    // add to map
    {
        android::Mutex::Autolock lock(mJsonMutex);
        mvFDJsonMap.add(pRequest->getRequestNo(), String8(sJsonStr));
    }
    // merge json if can
    mergeExtraData(pRequest);
    // mark on-going-request end
    this->decExtThreadDependency();

    return MTRUE;

}

MBOOL
StereoFDNode::
mergeExtraData(DepthMapRequestPtr& pRequest)
{
    MUINT32 iReqID = pRequest->getRequestNo();
    int iFDIdx, iN3DIdx;

    VSDOF_LOGD("+");

    android::Mutex::Autolock lock(mJsonMutex);

    if((iFDIdx = mvFDJsonMap.indexOfKey(iReqID)) < 0 ||
        (iN3DIdx = mvN3DJsonMap.indexOfKey(iReqID)) < 0)
        {
            MY_LOGD("extra data not ready, reqID=%d", iReqID);
            return MFALSE;
        }
    // output extra buffer
    IImageBuffer* pExtraDataImgBuf = nullptr;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    pExtraDataImgBuf = pBufferHandler->requestBuffer(getNodeId(), BID_FD_OUT_EXTRADATA);
    // extra data of FD/N3D
    String8 sFdJson = mvFDJsonMap.valueAt(iFDIdx);
    String8 sN3DJson = mvN3DJsonMap.valueAt(iN3DIdx);

    if(pExtraDataImgBuf != nullptr)
    {
        // merge FD and N3D json
        char* sExtraData = mpJsonUtil->mergeJSON(sFdJson.string(), sN3DJson.string());

        IMetadata* pInAppMeta = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_APP);
        // retrieve DOF level
        MINT32 iDoFLevel;
        if(tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DOF_LEVEL, iDoFLevel))
        {
            char* sDofJson = mpJsonUtil->dofLevelToJSON(iDoFLevel);
            sExtraData = mpJsonUtil->mergeJSON(sDofJson, sExtraData);
        }
        else
            MY_LOGE("Cannot get DOF level in the metadata!");

        if(strlen(sExtraData)+1 > pExtraDataImgBuf->getBufSizeInBytes(0))
        {
            MY_LOGE("Extra data length is larger than the output buffer size, ExtraData size=%d , output buffer size=%d", strlen(sExtraData), pExtraDataImgBuf->getBufSizeInBytes(0));
        }
        else
        {
            VSDOF_LOGD("Copy result to frame!! result=%s", sExtraData);
            memcpy((MUINT8*) pExtraDataImgBuf->getBufVA(0), sExtraData, strlen(sExtraData)+1);
            pRequest->setOutputBufferReady(BID_FD_OUT_EXTRADATA);
            handleDataAndDump(FD_OUT_EXTRADATA, pRequest);
        }
    }

    // launch onProcessDone
    pBufferHandler->onProcessDone(getNodeId());

    mvFDJsonMap.removeItemsAt(iFDIdx);
    mvN3DJsonMap.removeItemsAt(iN3DIdx);
    VSDOF_LOGD("-");

    return MTRUE;
}

MVOID
StereoFDNode::
calcRotatedFDResult(
    MtkCameraFace inputFaceInfo,
    MUINT32 iRotation,
    FD_DATA_STEREO_T &rRotatedFDResult
)
{
    int srcWidth = mFD_IMG_SIZE.w;
    int srcHeight = mFD_IMG_SIZE.h;
    switch(iRotation){
        case ANGLE_DEGREE_0:{
            // rotation = 0 ; mirror = 0
            rRotatedFDResult.left = (MUINT32)(inputFaceInfo.rect[0]);
            rRotatedFDResult.top = (MUINT32)(inputFaceInfo.rect[1]);
            rRotatedFDResult.right = (MUINT32)(inputFaceInfo.rect[2]);
            rRotatedFDResult.bottom = (MUINT32)(inputFaceInfo.rect[3]);
            break;
        }
        case ANGLE_DEGREE_90:{
            // rotation = 90 ; mirror = 0
            rRotatedFDResult.left = (MUINT32)(0 - inputFaceInfo.rect[3]);
            rRotatedFDResult.top = (MUINT32)(inputFaceInfo.rect[0]);
            rRotatedFDResult.right = (MUINT32)(0 - inputFaceInfo.rect[1]);
            rRotatedFDResult.bottom = (MUINT32)(inputFaceInfo.rect[2]);
            break;
        }
        case ANGLE_DEGREE_180:{
            // rotation = 180 ; mirror = 0
            rRotatedFDResult.left = (MUINT32)(0 - inputFaceInfo.rect[2]);
            rRotatedFDResult.top = (MUINT32)(0 - inputFaceInfo.rect[3]);
            rRotatedFDResult.right = (MUINT32)(0 - inputFaceInfo.rect[0]);
            rRotatedFDResult.bottom = (MUINT32)(0 - inputFaceInfo.rect[1]);
            break;
        }
        case ANGLE_DEGREE_270:{
            // rotation = 270 ; mirror = 0
            rRotatedFDResult.left = (MUINT32)(inputFaceInfo.rect[1]);
            rRotatedFDResult.top = (MUINT32)(0 - inputFaceInfo.rect[2]);
            rRotatedFDResult.right = (MUINT32)(inputFaceInfo.rect[3]);
            rRotatedFDResult.bottom = (MUINT32)(0 - inputFaceInfo.rect[0]);
            break;
        }
        default:{
            MY_LOGE("mTransform=%d, no such case, should not happend!", iRotation);
            break;
        }
    }
}

MVOID
StereoFDNode::
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
    mvFDJsonMap.clear();
    mvN3DJsonMap.clear();
    MY_LOGD("-");
}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam
