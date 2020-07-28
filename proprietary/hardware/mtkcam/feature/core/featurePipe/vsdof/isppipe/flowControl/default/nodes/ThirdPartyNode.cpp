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
#include <DpBlitStream.h>
#include <mtkcam/utils/hw/IFDContainer.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file
// Local header file
#include "ThirdPartyNode.h"
#include "../util/vsdof_util.h" // if no need to use mdp, it can be remove
// Logging
#undef PIPE_CLASS_TAG
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "TPNode"
#include <featurePipe/core/include/PipeLog.h>

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
//  P2ANode Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ThirdPartyNode::
ThirdPartyNode(
    const char *name,
    IspPipeNodeID nodeID,
    const PipeNodeConfigs& config
)
: IspPipeNode(name, nodeID, config)
{
    MY_LOGD("ctor +");
    miThirdpartyEnable = ::property_get_int32("vendor.debug.vsdof.3rdflow", 1);
    // Get FD info timestamp tolerence = 600ms (Default)
    miThirdpartyFdTolerence = ::property_get_int64("vendor.debug.vsdof.3rdflow.fd.tolerence",  600000000);

    this->addWaitQueue(&mRequestQue);
    MY_LOGD("ctor -");
}

ThirdPartyNode::
~ThirdPartyNode()
{
    MY_LOGD("+");
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2ANode Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
ThirdPartyNode::
onInit()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
ThirdPartyNode::
onUninit()
{
    MY_LOGD("+");
    MY_LOGD("-");
    return MTRUE;
}

MBOOL
ThirdPartyNode::
onThreadStart()
{
    // used for tmp flow
    mpDpStream = new DpBlitStream();
    // Put Third-party SW initialization
    return MTRUE;
}

MBOOL
ThirdPartyNode::
onThreadStop()
{
    // used for tmp flow
    delete mpDpStream;
    // Put third-party SW un-initilization
    return MTRUE;
}

MBOOL
ThirdPartyNode::
onData(DataID dataID, const IspPipeRequestPtr& pRequest)
{
    MBOOL ret = MTRUE;
    PIPE_LOGD("+ : reqID=%d", pRequest->getRequestNo());
    switch(dataID)
    {
        case P2A_TO_TP_YUV_DATA:
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
ThirdPartyNode::
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
ThirdPartyNode::
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
    AutoProfileLogging profile("TPNode::threadLoop", pRequest->getRequestNo());
    // timer
    pRequest->mTimer.startTP();
    MBOOL bRet = MFALSE;
    if(bRet = executeThirdParty(pRequest))
    {
        pRequest->mTimer.stopTP();
        profile.logging("third-party exec-time=%d ms", pRequest->mTimer.getElapsedTP());
    }
    else
    {
        MY_LOGE("reqID=%d Third-party ALGO execute failed!", pRequest->getRequestNo());
        this->handleData(ERROR_OCCUR_NOTIFY, pRequest);
    }
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();
    return bRet;
}

MBOOL
ThirdPartyNode::
executeThirdParty(
    const IspPipeRequestPtr& pRequest
)
{
    AutoProfileLogging profile("TPNode::executeThirdParty", pRequest->getRequestNo());
    if(pRequest->getRequestAttr().reqType == PIPE_REQ_PREVIEW)
    {
        ThirdpartyInputParameter inputParam;
        if( miThirdpartyEnable == 0 ||
            (prepareThirdpartyInputParameter(pRequest, inputParam)
            && onHandlePreviewRequest(pRequest, inputParam))
        )
        {
            this->handleDataAndDump(TP_TO_MDP_PVYUV, pRequest);
            return MTRUE;
        }
    }
    else
    {
        // not handle PIPE_REQ_CAPTURE/PIPE_REQ_META type currently
        pRequest->setOutputBufferReady(BID_META_OUT_APP);
        pRequest->setOutputBufferReady(BID_META_OUT_HAL);
        this->handleDataAndDump(TP_OUT_DEPTH_BOKEH, pRequest);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
ThirdPartyNode::
onHandlePreviewRequest(
    const IspPipeRequestPtr& pRequest,
    const ThirdpartyInputParameter& inputParam
)
{
    AutoProfileLogging profile("TPNode::onHandlePreviewRequest", pRequest->getRequestNo());
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();

    // input Main1 PV YUV (preview size)
    IImageBuffer *pImgBuf_PVYUVMain1 = nullptr;
    // input synced Main1 YUV (preview size)
    IImageBuffer *pImgBuf_SyncedMain1 = nullptr;
    // input synced Main2 YUV (preview size)
    IImageBuffer *pImgBuf_SyncedMain2 = nullptr;
    // preview output YUV0
    IImageBuffer *pImgBuf_OutYUV = nullptr;

    MBOOL bRet = MTRUE;

    MBOOL isCaptureMode = (inputParam.featureModeStatus == MTK_STEREO_FEATURE_STATUS_CAPTURE);
    // get input buffer
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN1, pImgBuf_SyncedMain1);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN2, pImgBuf_SyncedMain2);
    if(!bRet)
    {
        MY_LOGE("reqID=%d Failed to get input buffers", pRequest->getRequestNo());
        return MFALSE;
    }
    // get output buffer of preview, could be anyone of BID_PIPE_OUT_PV_YUV0/BID_PIPE_OUT_PV_YUV1/BID_PIPE_OUT_PV_FD
    IspPipeBufferID pvOutBufferID = (pRequest->isRequestBuffer(BID_PIPE_OUT_PV_YUV0)) ? BID_PIPE_OUT_PV_YUV0 : BID_PIPE_OUT_PV_YUV1;
    PIPE_LOGD("Target output preview buffer id =%d", pvOutBufferID);
    pImgBuf_OutYUV = pBufferHandler->requestBuffer(getNodeId(), pvOutBufferID);

    // TODO: execute the third-party SW for preview frame
    {
        // for example, copy pImgBuf_SyncedMain1 to pImgBuf_OutYUV directly
        // if third-party algo will be apply, please remove this section code.
        if(pImgBuf_SyncedMain1 != nullptr && pImgBuf_OutYUV !=nullptr)
        {
            //
            VSDOF::util::sMDP_Config config;
            config.pDpStream = mpDpStream;
            config.pSrcBuffer = pImgBuf_SyncedMain1;
            config.pDstBuffer = pImgBuf_OutYUV;
            config.rotAngle = 0;
            if(!excuteMDP(config))
            {
                MY_LOGE("excuteMDP fail.");
                goto lbExit;
            }
        }
        else
        {
            MY_LOGE("pvyuv(%p) out(%p)", pImgBuf_SyncedMain1, pImgBuf_OutYUV);
        }
    }

    // Mark buffer ready
    pRequest->setOutputBufferReady(pvOutBufferID);
    // config out meta ready
    pRequest->setOutputBufferReady(BID_META_OUT_APP);
    pRequest->setOutputBufferReady(BID_META_OUT_HAL);
    // Notes: the left preview buffers will be filled in MDP nodes
lbExit:
    return MTRUE;
}

MBOOL
ThirdPartyNode::
prepareThirdpartyInputParameter(
    const IspPipeRequestPtr& pRequest,
    ThirdpartyInputParameter& rInputParam
)
{
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IMetadata* pMeta_inHal = pBufferHandler->requestMetadata(getNodeId(), BID_META_IN_HAL_MAIN1);
    IMetadata* pMeta_inApp = pBufferHandler->requestMetadata(getNodeId(), PBID_IN_APP_META);

    // preview size
    {
        IImageBuffer *pImgBuf = nullptr;
        if(!pBufferHandler->getEnqueBuffer(getNodeId(), BID_P2A_OUT_YUV_MAIN1, pImgBuf))
        {
            MY_LOGE("reqID=%d, faile to get buf(BID_P2A_OUT_YUV_MAIN1)", pRequest->getRequestNo());
            return MFALSE;
        }
        rInputParam.mPreviewSize = pImgBuf->getImgSize();
    }
    // actual af roi
    {
        // active array
        sp<IMetadataProvider> pMetadataProvider = NSCam::NSMetadataProviderManager::valueFor(0);
        if(!pMetadataProvider.get())
        {
            MY_LOGE("reqID=%d, failed to get MtkStaticCharacteristics", pRequest->getRequestNo());
            return MFALSE;
        }

        MRect activeArray;
        const IMetadata& staticMetadata = pMetadataProvider->getMtkStaticCharacteristics();
        if(!IMetadata::getEntry<MRect>(&staticMetadata, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray))
        {
            MY_LOGE("reqID=%d, failed to get activeArray(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION)", pRequest->getRequestNo());
            return MFALSE;
        }
        // preview crop region
        MRect previewCropRegion;
        if(!tryGetMetadata<MRect>(pMeta_inApp, MTK_3A_PRV_CROP_REGION, previewCropRegion))
        {
            MY_LOGE("reqID=%d, failed to get previewCropRegion(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION)", pRequest->getRequestNo());
            return MFALSE;
        }
        // af roi
        IMetadata::IEntry entry = pMeta_inApp->entryFor(MTK_3A_FEATURE_AF_ROI);
        MPoint afTopLeftTemp;
        MPoint afBottomRight;
        if(entry.isEmpty())
        {
            MY_LOGE("reqID=%d, failed to get af roi info(MTK_3A_FEATURE_AF_ROI)", pRequest->getRequestNo());
            return MFALSE;
        }
        else
        {
            afTopLeftTemp.x = entry.itemAt(2, Type2Type<MINT32>());
            afTopLeftTemp.y = entry.itemAt(3, Type2Type<MINT32>());
            afBottomRight.x = entry.itemAt(4, Type2Type<MINT32>());
            afBottomRight.y = entry.itemAt(5, Type2Type<MINT32>());
            rInputParam.mAFType = entry.itemAt(6, Type2Type<MINT32>());

            PIPE_LOGD("get af roi info from metadata, type:%d, roi:(%d, %d, %d, %d)",
                rInputParam.mAFType, afTopLeftTemp.x, afTopLeftTemp.y, afBottomRight.x, afBottomRight.y);
        }

        // caculate mapping
        MINT32 offsetX =  previewCropRegion.p.x - activeArray.p.x;
        MINT32 offsetY =  previewCropRegion.p.y - activeArray.p.y;
        rInputParam.mAFTopLeft.x = (afTopLeftTemp.x - offsetX)*rInputParam.mPreviewSize.w/previewCropRegion.s.w;
        rInputParam.mAFTopLeft.y = (afTopLeftTemp.y - offsetY)*rInputParam.mPreviewSize.h/previewCropRegion.s.h;
        rInputParam.mAFBottomRight.x = (afBottomRight.x - offsetX)*rInputParam.mPreviewSize.w/previewCropRegion.s.w;
        rInputParam.mAFBottomRight.y = (afBottomRight.y - offsetY)*rInputParam.mPreviewSize.h/previewCropRegion.s.h;

        PIPE_LOGD("offset:(%d, %d), activeArray:(%d, %d, %d, %d), actual af roi:(%d, %d, %d, %d)",
            offsetX, offsetY,
            activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h,
            rInputParam.mAFTopLeft.x, rInputParam.mAFTopLeft.y, rInputParam.mAFBottomRight.x, rInputParam.mAFBottomRight.y);
    }
    // bokeh level
    {
        if(!tryGetMetadata<MINT32>(pMeta_inApp, MTK_STEREO_FEATURE_DOF_LEVEL, rInputParam.mPreviewBokehLevel))
        {
            MY_LOGE("reqID=%d, failed to get previewBokehLevel(MTK_STEREO_FEATURE_DOF_LEVEL)", pRequest->getRequestNo());
            return MFALSE;
        }
        PIPE_LOGD("get previewBokehLevel:%d", rInputParam.mPreviewBokehLevel);
    }
    // af status
    {
        if(!tryGetMetadata<MUINT8>(pMeta_inApp, MTK_CONTROL_AF_STATE, rInputParam.mAFState) )
        {
            MY_LOGE("reqID=%d, failed to get afState(MTK_CONTROL_AF_STATE)", pRequest->getRequestNo());
            return MFALSE;
        }
        PIPE_LOGD("get afState:%d", rInputParam.mAFState);
    }
    // sensor orientation
    {
         if(!tryGetMetadata<MINT32>(pMeta_inApp, MTK_SENSOR_ORIENTATION, rInputParam.mSensorOrientation))
         {
            MY_LOGE("reqID=%d, failed to get sensorOrientation(MTK_SENSOR_ORIENTATION)", pRequest->getRequestNo());
            return MFALSE;
         }
         PIPE_LOGD("get sensorOrientatio:%d", rInputParam.mSensorOrientation);
    }
    // af distance
    {
        if(!tryGetMetadata<MFLOAT>(pMeta_inApp, MTK_LENS_FOCUS_DISTANCE, rInputParam.mAFDistance) )
        {
            MY_LOGE("reqID=%d, failed to get afDistance(MTK_LENS_FOCUS_DISTANCE)", pRequest->getRequestNo());
            return MFALSE;
        }
        PIPE_LOGD("get afDistance:%d", rInputParam.mAFDistance);
    }
    // FD
    {
        MINT64 p1Timestamp = 0;
        if(!tryGetMetadata<MINT64>(pMeta_inHal, MTK_P1NODE_FRAME_START_TIMESTAMP, p1Timestamp))
        {
            MY_LOGE("reqID=%d Cant get p1 timestamp meta!", pRequest->getRequestNo());
            return MFALSE;
        }

        auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
        auto fdData = fdReader->queryLock(p1Timestamp - miThirdpartyFdTolerence, p1Timestamp);
        if (fdData.size() > 0)
        {
            auto fdChunk = fdData.back();
            if (CC_LIKELY( fdChunk != nullptr ))
            {
                PIPE_LOGD("Number_of_faces: %d",fdChunk->facedata.number_of_faces);
                // Check if exceed the maxium faces count
                MINT32 facesNum = (fdChunk->facedata.number_of_faces > MAX_THIRDPARTY_FD_FACES_NUM) ? MAX_THIRDPARTY_FD_FACES_NUM : fdChunk->facedata.number_of_faces;
                rInputParam.miFaceCount = facesNum;

                // get output buffer size as domain
                IspPipeBufferID pvOutBufferID = (pRequest->isRequestBuffer(BID_PIPE_OUT_PV_YUV0)) ? BID_PIPE_OUT_PV_YUV0 : BID_PIPE_OUT_PV_YUV1;
                PIPE_LOGD("Target output preview buffer id =%d", pvOutBufferID);
                auto pImgBuf_OutYUV = pBufferHandler->requestBuffer(getNodeId(), pvOutBufferID);

                // face, 15 is the max number of faces
                for (size_t i = 0 ; i < facesNum; i++)
                {
                    // Origianl FD ROI set is from -1000 to 1000, need to map to preview size domain
                    MINT32 width  = pImgBuf_OutYUV->getImgSize().w;
                    MINT32 height = pImgBuf_OutYUV->getImgSize().h;
                    MPoint TopLeft(0,0);
                    MPoint BotRight(0,0);
                    float ratioTopX = (float)(fdChunk->facedata.faces[i].rect[0] + 1000 ) / (float) 2000;
                    float ratioTopY = (float)(fdChunk->facedata.faces[i].rect[1] + 1000 ) / (float) 2000;
                    float ratioBotX = (float)(fdChunk->facedata.faces[i].rect[2] + 1000 ) / (float) 2000;
                    float ratioBotY = (float)(fdChunk->facedata.faces[i].rect[3] + 1000 ) / (float) 2000;
                    TopLeft.x = ratioTopX * width;
                    TopLeft.y = ratioTopY * height;
                    BotRight.x= ratioBotX * width;
                    BotRight.y= ratioBotY * height;
                    //
                    rInputParam.mFdInfo[i].roiTopLeft.x  = TopLeft.x;
                    rInputParam.mFdInfo[i].roiTopLeft.y  = TopLeft.y;
                    rInputParam.mFdInfo[i].roiBotRight.x = BotRight.x;
                    rInputParam.mFdInfo[i].roiBotRight.y = BotRight.y;
                    rInputParam.mFdInfo[i].degreeRotInPlane   = fdChunk->facedata.fld_rip[i];
                    rInputParam.mFdInfo[i].degreeRotOffPlane   = fdChunk->facedata.fld_rop[i];
                    PIPE_LOGD("FD[%d] ROI %d %d %d %d, RIP: %d  ROP:%d", i, TopLeft.x, TopLeft.y, BotRight.x, BotRight.y,
                                rInputParam.mFdInfo[i].degreeRotInPlane, rInputParam.mFdInfo[i].degreeRotOffPlane);
                }
            }
        }
        else
        {
            rInputParam.miFaceCount = 0;
            PIPE_LOGD("FD size is zero.");
        }
        // fdData must be return after use
        fdReader->queryUnlock(fdData);
    }
    // focus center
    {
        // get touch focus center
        if(1 == rInputParam.mAFType) // touch af
        {
            rInputParam.mFocusCenter.x = (rInputParam.mAFTopLeft.x + rInputParam.mAFBottomRight.x)/2;
            rInputParam.mFocusCenter.y = (rInputParam.mAFTopLeft.y + rInputParam.mAFBottomRight.y)/2;
            PIPE_LOGD("set touch focus center:(%d, %d)", rInputParam.mFocusCenter.x, rInputParam.mFocusCenter.y);
        }
        else if(rInputParam.miFaceCount > 0) //face af
        {
            rInputParam.mFocusCenter.x = (rInputParam.mFdInfo[0].roiTopLeft.x + rInputParam.mFdInfo[0].roiBotRight.x)/2;
            rInputParam.mFocusCenter.y = (rInputParam.mFdInfo[0].roiTopLeft.y + rInputParam.mFdInfo[0].roiBotRight.y)/2;
            PIPE_LOGD("set fd focus center:(%d, %d)", rInputParam.mFocusCenter.x, rInputParam.mFocusCenter.y);
        }
        else // default
        {
            rInputParam.mFocusCenter.x = rInputParam.mPreviewSize.w/2;
            rInputParam.mFocusCenter.y = rInputParam.mPreviewSize.h/2;
            PIPE_LOGD("set default focus center:(%d, %d)", rInputParam.mFocusCenter.x, rInputParam.mFocusCenter.y);
        }

    }
    return MTRUE;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

