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

/**
* @file LinkedPipeItem.cpp
* @brief Use for feature sequential execution flow
*/
#define LOG_TAG "LinkedPipeItem"

// Standard C header file
// Android system/core header file
// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
// Module header file
// Local header file
#include "LinkedPipeItem.h"
#include <vector>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

#define DEFINE_CONTAINER_LOGGING(funcIdx, containerMap)\
    std::string ids="";\
    auto fn##funcIdx = [&]() -> const char*{\
                    for(auto index=0;index<containerMap.size();index++)\
                        ids += std::to_string(containerMap.keyAt(index)) + "|";\
                    return ids.c_str();};

#define CONT_LOGGING(funcIdx)\
    fn##funcIdx()

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature{
namespace NSDualFeature {

using namespace StereoHAL;


template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

DepthPipeItem::
DepthPipeItem(IDepthMapPipe* pPipe)
:mpPipe(pPipe)
{
    MY_LOGD("DepthPipeItem ctor");
    miLogLevel = ::property_get_int32("vendor.debug.camera.log.dualHal", 1);
}

DepthPipeItem::
~DepthPipeItem()
{
    MY_LOGD("DepthPipeItem destr");
}

MBOOL
DepthPipeItem::
enque(sp<IDualFeatureRequest> pRequest)
{
    MY_LOGD_IF(miLogLevel, "DepthPipeItem reqID=%d enque", pRequest->getRequestNo());
    {
        android::Mutex::Autolock lock(mOpLock);
        auto iReqNo = pRequest->getRequestNo();
        if(mRequestMap.indexOfKey(iReqNo) < 0)
            mRequestMap.add(pRequest->getRequestNo(), pRequest);
        else
        {
            MY_LOGE("[DepthPipeItem]Duplicated enque request, reqID=%d!!Please check.", iReqNo);
            return MFALSE;
        }
        // logging
        DEFINE_CONTAINER_LOGGING(idsGen, mRequestMap);
        MY_LOGD_IF(miLogLevel, "reqID=%d mRequestMap size=%d ids=%s", iReqNo, mRequestMap.size(), CONT_LOGGING(idsGen));
    }
    // create depth request
    sp<IDepthMapEffectRequest> pDepthReq = IDepthMapEffectRequest::createInstance(
                                                                    pRequest->getRequestNo(),
                                                                    onPipeReady,
                                                                    this);
    if(pRequest->copyTo(pDepthReq.get(), &mapToDepthBufferID))
        return mpPipe->enque(pDepthReq);
    else
        return MFALSE;

}

DepthMapBufferID
DepthPipeItem::
mapToDepthBufferID(RequestBufferID bufferID)
{
    switch(bufferID)
    {
        case RBID_IN_FULLRAW_MAIN1:
            return PBID_IN_FSRAW1;
        case RBID_IN_FULLRAW_MAIN2:
            return PBID_IN_FSRAW2;
        case RBID_IN_RESIZERAW_MAIN1:
            return PBID_IN_RSRAW1;
        case RBID_IN_RESIZERAW_MAIN2:
            return PBID_IN_RSRAW2;
        case RBID_IN_LCSO_MAIN1:
            return PBID_IN_LCSO1;
        case RBID_IN_LCSO_MAIN2:
            return PBID_IN_LCSO2;
        case RBID_IN_APP_META_MAIN1:
            return PBID_IN_APP_META;
        case RBID_IN_HAL_META_MAIN1:
            return PBID_IN_HAL_META_MAIN1;
        case RBID_IN_HAL_META_MAIN2:
            return PBID_IN_HAL_META_MAIN2;
        case RBID_IN_P1_RETURN_META:
            return PBID_IN_P1_RETURN_META;
        case RBID_OUT_MAINIMAGE:
            return PBID_OUT_MV_F;
        case RBID_OUT_BLUR_MAP:
            return PBID_OUT_DMBG;
        case RBID_OUT_APP_META:
            return PBID_OUT_APP_META;
        case RBID_OUT_HAL_META:
            return PBID_OUT_HAL_META;
        case RBID_OUT_FD:
            return PBID_OUT_FDIMG;
        case RBID_OUT_DEPTHMAP:
            return PBID_OUT_DEPTHMAP;
        default:
            return PBID_INVALID;
    }
}

MVOID
DepthPipeItem::
onPipeReady(
    MVOID* tag,
    ResultState state,
    sp<IDualFeatureRequest>& request
)
{
    DepthPipeItem* pDepthItem = (DepthPipeItem*) tag;
    pDepthItem->onHandlePipeReady(state, request);
}

MVOID
DepthPipeItem::
onHandlePipeReady(
    ResultState state,
    sp<IDualFeatureRequest>& pRequest
)
{
    MY_LOGD_IF(miLogLevel, "DepthPipeItem reqID=%d onHandlePipeReady state=%d %s",
                pRequest->getRequestNo(), state, ResultState2Name(state));
    sp<IDualFeatureRequest> pDualRequest = nullptr;
    // get request from shared container
    ssize_t index = -1;
    {
        android::Mutex::Autolock lock(mOpLock);
        index = mRequestMap.indexOfKey(pRequest->getRequestNo());
        if(index < 0)
        {
            MY_LOGE("[DepthPipeItem]reqID=%d is missing, not exist inside request map! Might released before.",
                        pRequest->getRequestNo());
            return;
        }
        pDualRequest = mRequestMap.valueAt(index);
    }
    // if complete/not_ready -> enue to next item
    if(state == eRESULT_COMPLETE || state == eRESULT_DEPTH_NOT_READY)
    {
        // enque to next pipe item
        mpNext->enque(pDualRequest);
        android::Mutex::Autolock lock(mOpLock);
        mRequestMap.removeItem(pRequest->getRequestNo());
        // logging
        DEFINE_CONTAINER_LOGGING(idsGen, mRequestMap);
        MY_LOGD_IF(miLogLevel, "reqID=%d after mRequestMap size=%d ids=%s",
                    pRequest->getRequestNo(), mRequestMap.size(), CONT_LOGGING(idsGen));
    }
    else
    {
        // other status, callback to caller
        pDualRequest->launchFinishCallback(state);
    }

}

BokehPipeItem::
BokehPipeItem(sp<IBokehPipe> pPipe)
: mpPipe(pPipe)
{
    MY_LOGD("BokehPipeItem ctor");
    miLogLevel = ::property_get_int32("vendor.debug.camera.log.dualHal", 1);
}

BokehPipeItem::
~BokehPipeItem()
{
    MY_LOGD("BokehPipeItem dtor");
}


MBOOL
BokehPipeItem::
enque(sp<IDualFeatureRequest> pRequest)
{
    MY_LOGD_IF(miLogLevel, "BokehPipeItem reqID=%d enque", pRequest->getRequestNo());
    {
        android::Mutex::Autolock lock(mOpLock);
        auto iReqNo = pRequest->getRequestNo();
        if(mRequestMap.indexOfKey(iReqNo) < 0)
            mRequestMap.add(pRequest->getRequestNo(), pRequest);
        else
        {
            MY_LOGE("[BokehPipeItem]Duplicated enque request, reqID=%d!!Please check.", iReqNo);
            return MFALSE;
        }
    }


    sp<NSCam::EffectRequest> pBokehReq = new NSCam::EffectRequest(pRequest->getRequestNo(), onPipeReady, this);
    if(copyToBokehRequest(pRequest, pBokehReq))
    {
        return mpPipe->enque(pBokehReq);
    }
    else
        return MFALSE;
}

MBOOL
BokehPipeItem::
_copyMetaToFrame(
    sp<IDualFeatureRequest> pSourceReq,
    RequestBufferID copyBufID,
    sp<EffectFrameInfo> rOutFrame
)
{
    if(mapToBokehBufferID(copyBufID) == BOKEH_ER_BUF_INVALID)
        return MFALSE;

    BufferIOType ioType;
    IMetadata* pMeta;
    if( pSourceReq->getBufferIOType(copyBufID, ioType) &&
        pSourceReq->getRequestMetadata({.bufferID=copyBufID, .ioType=ioType}, pMeta))
    {
        sp<EffectParameter> param = new EffectParameter();
        param->setPtr(BOKEH_META_KEY_STRING, pMeta);
        rOutFrame->setFrameParameter(param);
    }
    else
    {
        MY_LOGE("Failed to get metadata!! bufferID=%d", copyBufID);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
BokehPipeItem::
_copyBufferToFrame(
    sp<IDualFeatureRequest> pSourceReq,
    RequestBufferID copyBufID,
    sp<EffectFrameInfo> rOutFrame
)
{
    if(mapToBokehBufferID(copyBufID) == BOKEH_ER_BUF_INVALID)
        return MFALSE;

    BufferIOType ioType;
    IImageBuffer* pBuf;
    if( pSourceReq->getBufferIOType(copyBufID, ioType) &&
        pSourceReq->getRequestImageBuffer({.bufferID=copyBufID, .ioType=ioType}, pBuf))
    {
        rOutFrame->setFrameBuffer(pBuf);
    }
    else
    {
        MY_LOGE("Failed to get buffer!! bufferID=%d", copyBufID);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
BokehPipeItem::
copyToBokehRequest(
    sp<IDualFeatureRequest> pSourceReq,
    sp<EffectRequest> pTargetReq
)
{
    // Input Meta - RBID_IN_APP_META_MAIN1/RBID_IN_HAL_META_MAIN1/RBID_IN_HAL_META_MAIN2
    RequestBufferID inputMetaArray[] = {RBID_IN_APP_META_MAIN1, RBID_IN_HAL_META_MAIN1, RBID_IN_P1_RETURN_META};
    for(RequestBufferID bufferID_req : inputMetaArray)
    {
        BokehEffectRequestBufferType bufferID_bokeh = mapToBokehBufferID(bufferID_req);
        sp<EffectFrameInfo> pFrame = new EffectFrameInfo(pSourceReq->getRequestNo(), bufferID_bokeh);
        if(!_copyMetaToFrame(pSourceReq, bufferID_req, pFrame))
        {
            MY_LOGE("Error to copy input Meta frame!bufferID=%d", bufferID_req);
            return MFALSE;
        }
        pTargetReq->vInputFrameInfo.add(bufferID_bokeh, pFrame);
    }
    // Input Meta: RBID_IN_HAL_META_MAIN2(optional)
    RequestBufferID inputMetaOptinalArray[] = {RBID_IN_HAL_META_MAIN2};
    for(RequestBufferID bufferID_req : inputMetaOptinalArray)
    {
        if(pSourceReq->isRequestBuffer(bufferID_req))
        {
            BokehEffectRequestBufferType bufferID_bokeh = mapToBokehBufferID(bufferID_req);
            sp<EffectFrameInfo> pFrame = new EffectFrameInfo(pSourceReq->getRequestNo(), bufferID_bokeh);
            if(!_copyMetaToFrame(pSourceReq, bufferID_req, pFrame))
            {
                MY_LOGW("No avalibale Meta frame!bufferID=%d", bufferID_req);
            }
            pTargetReq->vInputFrameInfo.add(bufferID_bokeh, pFrame);
        }
    }

    // Input Buffer: RBID_OUT_MAINIMAGE(this is the output of input request)
    std::vector<RequestBufferID> inputBufArray;
    if (mpPipe->getPipeMode() == HW_BOKEH)
    {
        inputBufArray = {RBID_OUT_MAINIMAGE, RBID_OUT_BLUR_MAP};
    }
    else if (mpPipe->getPipeMode() == VENDOR_BOKEH)
    {
        inputBufArray = {RBID_OUT_MAINIMAGE, RBID_OUT_DEPTHMAP};
    }
    else
    {
        MY_LOGE("Not support BokehPipe mode ! mode=%d", mpPipe->getPipeMode());
        return MFALSE;
    }
    for(RequestBufferID bufferID_req : inputBufArray)
    {
        BokehEffectRequestBufferType bufferID_bokeh = mapToBokehBufferID(bufferID_req);
        sp<EffectFrameInfo> pFrame = new EffectFrameInfo(pSourceReq->getRequestNo(), bufferID_bokeh);
        if(!_copyBufferToFrame(pSourceReq, bufferID_req, pFrame))
        {
            MY_LOGE("Error to copy input buffer frame!bufferID=%d", bufferID_req);
            return MFALSE;
        }
        pTargetReq->vInputFrameInfo.add(bufferID_bokeh, pFrame);
    }
    // Output Meta - RBID_OUT_APP_META/RBID_OUT_HAL_META
    RequestBufferID outputMetaArray[] = {RBID_OUT_APP_META, RBID_OUT_HAL_META};
    for(RequestBufferID bufferID_req : outputMetaArray)
    {
        BokehEffectRequestBufferType bufferID_bokeh = mapToBokehBufferID(bufferID_req);
        sp<EffectFrameInfo> pFrame = new EffectFrameInfo(pSourceReq->getRequestNo(), bufferID_bokeh);
        if(!_copyMetaToFrame(pSourceReq, bufferID_req, pFrame))
        {
            MY_LOGE("Error to copy output Meta frame!bufferID=%d", bufferID_req);
            return MFALSE;
        }
        pTargetReq->vOutputFrameInfo.add(bufferID_bokeh, pFrame);
    }
    // Output Buffer: RBID_OUT_PREVIEW
    sp<IImageBuffer> pBuf = nullptr;
    BokehEffectRequestBufferType bufferID_bokeh = mapToBokehBufferID(RBID_OUT_PREVIEW);
    sp<EffectFrameInfo> pFrame = new EffectFrameInfo(pSourceReq->getRequestNo(), bufferID_bokeh);
    if(!_copyBufferToFrame(pSourceReq, RBID_OUT_PREVIEW, pFrame))
    {
        MY_LOGE("Error to copy input buffer frame!bufferID=%d", RBID_OUT_PREVIEW);
        return MFALSE;
    }
    pTargetReq->vOutputFrameInfo.add(bufferID_bokeh, pFrame);
    // Output Buffer: RBID_OUT_PREVIEW_CALLBACK(optional)
    if(pSourceReq->isRequestBuffer(RBID_OUT_PREVIEW_CALLBACK))
    {
        bufferID_bokeh = mapToBokehBufferID(RBID_OUT_PREVIEW_CALLBACK);
        sp<EffectFrameInfo> pFrame = new EffectFrameInfo(pSourceReq->getRequestNo(), bufferID_bokeh);
        if(!_copyBufferToFrame(pSourceReq, RBID_OUT_PREVIEW_CALLBACK, pFrame))
        {
            MY_LOGE("Error to copy input buffer frame!bufferID=%d", RBID_OUT_PREVIEW);
            return MFALSE;
        }
        pTargetReq->vOutputFrameInfo.add(bufferID_bokeh, pFrame);
    }
    //
    MBOOL bRet = MTRUE;
    IMetadata* pInAppMeta = nullptr;
    IMetadata* pInHalMeta = nullptr;
    bRet &= pSourceReq->getRequestMetadata(
                {.bufferID=RBID_IN_APP_META_MAIN1, .ioType=eBUFFER_IOTYPE_INPUT},
                pInAppMeta);
    bRet &= pSourceReq->getRequestMetadata(
                {.bufferID=RBID_IN_HAL_META_MAIN1, .ioType=eBUFFER_IOTYPE_INPUT},
                pInHalMeta);

    if(!bRet)
    {
        MY_LOGE("Failed to get metadata!");
        return MFALSE;
    }

    MINT32 iDofLevel = 0;
    MINT32 magicNum = 0;
    if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_STEREO_FEATURE_DOF_LEVEL, iDofLevel))
    {
        MY_LOGE("reqID=%d, Get bokeh level fail.", pSourceReq->getRequestNo());
        return MFALSE;
    }
    //
    if(!tryGetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum)) {
        MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
        return MFALSE;
    }
    //
    sp<EffectParameter> reqParam = new EffectParameter();
    // config scenario - only preview currently
    reqParam->set(VSDOF_FRAME_SCENARIO, eSTEREO_SCENARIO_PREVIEW);
    reqParam->set(VSDOF_FRAME_BOKEH_LEVEL, iDofLevel);
    reqParam->set(VSDOF_FEAME_MAGIC_NUMBER, magicNum);
    // set parameter
    pTargetReq->setRequestParameter(reqParam);
    return MTRUE;
}

BokehEffectRequestBufferType
BokehPipeItem::
mapToBokehBufferID(RequestBufferID bufferID)
{
    switch(bufferID)
    {
        case RBID_OUT_MAINIMAGE:
            return BOKEH_ER_BUF_MAIN1;
        case RBID_OUT_BLUR_MAP:
            return BOKEH_ER_BUF_DMBG;
        case RBID_OUT_PREVIEW:
            return BOKEH_ER_BUF_DISPLAY;
        case RBID_IN_APP_META_MAIN1:
            return BOKEH_ER_IN_APP_META;
        case RBID_IN_HAL_META_MAIN1:
            return BOKEH_ER_IN_HAL_META_MAIN1;
        case RBID_IN_HAL_META_MAIN2:
            return BOKEH_ER_IN_HAL_META_MAIN2;
        case RBID_OUT_APP_META:
            return BOKEH_ER_OUT_APP_META;
        case RBID_OUT_HAL_META:
            return BOKEH_ER_OUT_HAL_META;
        case RBID_OUT_PREVIEW_CALLBACK:
            return BOKEH_ER_BUF_PRVCB;
        case RBID_OUT_DEPTHMAP:
            return BOKEH_ER_OUTPUT_DEPTHMAP;
        case RBID_IN_P1_RETURN_META:
            return BOKEH_ER_IN_P1_RETURN_META;
        default:
            return BOKEH_ER_BUF_INVALID;
    }
}

MVOID
BokehPipeItem::
onPipeReady(
    MVOID* tag,
    String8 status,
    sp<NSCam::EffectRequest>& request
)
{
    BokehPipeItem* pBokehItem = (BokehPipeItem*) tag;
    if(status == BOKEH_YUV_DONE_KEY)
        pBokehItem->onHandlePipeReady(eRESULT_COMPLETE, request);
    else if(status == BOKEH_FLUSH_KEY)
        pBokehItem->onHandlePipeReady(eRESULT_FLUSH, request);
}

MVOID
BokehPipeItem::
onHandlePipeReady(
    ResultState state,
    sp<NSCam::EffectRequest>& pRequest
)
{
    MY_LOGD_IF(miLogLevel, "BokehPipeItem reqID=%d onHandlePipeReady state=%d  %s",
                    pRequest->getRequestNo(), state, ResultState2Name(state));
    sp<IDualFeatureRequest> pDualRequest = nullptr;
    {
        android::Mutex::Autolock lock(mOpLock);
        ssize_t index = mRequestMap.indexOfKey(pRequest->getRequestNo());
        if(index < 0)
        {
            MY_LOGE("[BokehPipeItem] reqID=%d already relesed, something might go wrong!",
                        pRequest->getRequestNo());
            return;
        }
        pDualRequest = mRequestMap.valueAt(index);
        mRequestMap.removeItemsAt(index);
    }
    // launch callback
    pDualRequest->launchFinishCallback(state);
}

};
};
};
