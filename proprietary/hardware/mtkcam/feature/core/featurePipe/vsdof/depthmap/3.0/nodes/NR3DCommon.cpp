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
// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file
#include <camera_custom_3dnr.h>
// Local header file
#include "NR3DCommon.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeNode.h"
#include "../DepthMapEffectRequest.h"
#include "../flowOption/DepthMapFlowOption.h"

// Logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "NR3D_Common"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define BID2Name DepthMapPipeNode::onDumpBIDToName
MBOOL
NR3DCommon::
init(MUINT32 miSensorIdx)
{
#ifdef GTEST
    return MTRUE;
#endif
    mforceFrameReset  = ::property_get_int32("vendor.debug.camera.3dnr.level", 0);
    // 3dnr
    mp3dnrHal = NSCam::NSIoPipe::NSPostProc::hal3dnrBase::createInstance(DEPTH_3DNR_USER, miSensorIdx);
    return mp3dnrHal->init(MTRUE);
}

#define GET_INPUT_META_AND_CHK(request, bid, targetPtr)\
    if(!pRequest->getRequestMetadata({.bufferID=bid, .ioType=eBUFFER_IOTYPE_INPUT}\
                                   , targetPtr))\
    {\
        MY_LOGE("reqID=%d, metadata(id:%d, %s) not exist, please check!",\
                    request->getRequestNo(), bid, BID2Name(bid));\
        return MFALSE;\
    }

MBOOL
NR3DCommon::
perform3dnr(
    DepthMapRequestPtr& pRequest,
    IHal3A* p3AHAL,
    sp<DepthMapFlowOption> pFlowOption,
    Stereo3ATuningRes& tuningResult
)
{
#ifdef GTEST
    return MTRUE;
#endif
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    IMetadata* pMeta_InApp = nullptr;
    IMetadata* pMeta_InHal = nullptr;
    // Get meta
    GET_INPUT_META_AND_CHK(pRequest, BID_META_IN_APP, pMeta_InApp);
    GET_INPUT_META_AND_CHK(pRequest, BID_META_IN_HAL_MAIN1, pMeta_InHal);
    // check AP switch
    if(!is3DNROn(pMeta_InApp))
    {
        VSDOF_LOGD("3DNR AP OFF.");
        return MTRUE;
    }
    //
    IMetadata* pMeta_P1Ret = nullptr;
    GET_INPUT_META_AND_CHK(pRequest, BID_META_IN_P1_RETURN, pMeta_P1Ret);
    //
    MINT32 iISOVal = 0;
    if(!tryGetMetadata<MINT32>(pMeta_P1Ret, MTK_SENSOR_SENSITIVITY , iISOVal))
    {
        MY_LOGE("Failed to get ISO value!");
        return MFALSE;
    }
    // query eis info
    eis_region eisInfo;
    if(!queryEisRegion(pMeta_InHal, eisInfo))
    {
        MY_LOGE("Failed to query EIS region!");
        return MFALSE;
    }
    // RSRAW
    IImageBuffer* frameBuf_RSRAW1;
    MBOOL bRet = pRequest->getRequestImageBuffer({.bufferID=BID_P2A_IN_RSRAW1,
                                    .ioType=eBUFFER_IOTYPE_INPUT}, frameBuf_RSRAW1);
    if(!bRet)
    {
        MY_LOGE("Failed to query input resize raw!");
        return MFALSE;
    }
    NR3DHALParam nr3dHalParam;
    // 3a related
    nr3dHalParam.pTuningData = tuningResult.tuningRes_main1.tuningBuffer;
    nr3dHalParam.p3A = p3AHAL;

    // generic
    nr3dHalParam.frameNo = pRequest->getRequestNo();
    nr3dHalParam.iso = iISOVal;

    // imgi related
    nr3dHalParam.isCRZUsed = MFALSE;
    nr3dHalParam.isIMGO = MFALSE;
    nr3dHalParam.isBinning = MFALSE; // no need for now

    // lmv related info
    nr3dHalParam.GMVInfo.gmvX = eisInfo.gmvX;  // from Hal Metadata:  MTK_EIS_REGION
    nr3dHalParam.GMVInfo.gmvY = eisInfo.gmvY; // same as above
    nr3dHalParam.GMVInfo.x_int = eisInfo.x_int; // same as above
    nr3dHalParam.GMVInfo.y_int = eisInfo.y_int; // same as above
    nr3dHalParam.GMVInfo.confX = eisInfo.confX; // same as above
    nr3dHalParam.GMVInfo.confY= eisInfo.confY; // same as above
    // vipi related
    nr3dHalParam.pIMGBufferVIPI = pFlowOption->get3DNRVIPIBuffer();
    // output related, ex: img3o
    nr3dHalParam.dst_resizer_rect = MRect(MPoint(0, 0), frameBuf_RSRAW1->getImgSize());
    bRet = mp3dnrHal->do3dnrFlow_v2(nr3dHalParam);

    VSDOF_LOGD("reqID=%d 3ndr on/off = %d", pRequest->getRequestNo(), bRet);
    return MTRUE;
}

MBOOL
NR3DCommon::
setup3DNRMeta(
    DepthMapPipeNodeID nodeID,
    DepthMapRequestPtr& rpRequest
)
{
#ifdef GTEST
    return MTRUE;
#endif
    eis_region eisInfo;
    sp<BaseBufferHandler> pBufferHandler = rpRequest->getBufferHandler();
    IMetadata* pMeta_InApp  = pBufferHandler->requestMetadata(nodeID, BID_META_IN_APP);
    IMetadata* pMeta_InHal  = pBufferHandler->requestMetadata(nodeID, BID_META_IN_HAL_MAIN1);
    //
    MINT32 e3dnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    if(!tryGetMetadata<MINT32>(pMeta_InApp, MTK_NR_FEATURE_3DNR_MODE, e3dnrMode))
    {
        VSDOF_LOGD("no MTK_NR_FEATURE_3DNR_MODE data!");
    }
    MINT32 frameReset = ((e3dnrMode == MTK_NR_FEATURE_3DNR_MODE_OFF) || (mforceFrameReset == MTRUE));
    MINT32 isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(MTRUE);
    MINT32 isLmvValid = 1;
    if (!tryGetMetadata<MINT32>(pMeta_InHal, MTK_LMV_VALIDITY, isLmvValid))
    {
        isLmvValid = 1; // Absent. Default behavior is 1 on single cam
    }
    MINT32 status = isLmvValid == 1 ? 1 : 0;
    // get lmv/gmv data when data exist
    if(is3DNROn(pMeta_InApp) && queryEisRegion(pMeta_InHal, eisInfo))
    {
        IMetadata::IEntry entry(MTK_3A_ISP_NR3D_SW_PARAMS);//refer to ISP_NR3D_META_INFO_T
        entry.push_back(eisInfo.gmvX, Type2Type< MINT32 >());
        entry.push_back(eisInfo.gmvY, Type2Type< MINT32 >());
        entry.push_back(eisInfo.confX, Type2Type< MINT32 >());
        entry.push_back(eisInfo.confY, Type2Type< MINT32 >());
        entry.push_back(eisInfo.maxGMV, Type2Type< MINT32 >());
        entry.push_back(frameReset, Type2Type< MINT32 >());
        entry.push_back(status, Type2Type< MINT32 >()); //GMV_Status 0: invalid state
        entry.push_back(isoThreshold, Type2Type< MINT32 >());
        pMeta_InHal->update(MTK_3A_ISP_NR3D_SW_PARAMS, entry);
    }
    else
    {
        VSDOF_LOGD("Failed to query EIS region! 3dnr_on: %d", is3DNROn(pMeta_InApp));
    }
    // Use RRZO -> sl2e no crop
    IImageBuffer* frameBuf_RSRAW1 = pBufferHandler->requestBuffer(nodeID, BID_P2A_IN_RSRAW1);

    MSize sl2eOriSize = frameBuf_RSRAW1->getImgSize();
    MRect sl2eCropInfo = MRect(MPoint(0, 0), sl2eOriSize);
    MSize sl2eRrzSize = frameBuf_RSRAW1->getImgSize();

    VSDOF_LOGD("sl2eOriSize=%dx%d sl2eCropInfo=(%d,%d) %dx%d sl2eRrzSize=%dx%d",
                sl2eOriSize.w, sl2eOriSize.h, sl2eCropInfo.p.x, sl2eCropInfo.p.y,
                sl2eCropInfo.s.w, sl2eCropInfo.s.h, sl2eRrzSize.w, sl2eRrzSize.h);

    trySetMetadata<MSize>(pMeta_InHal, MTK_ISP_P2_ORIGINAL_SIZE, sl2eOriSize);
    trySetMetadata<MRect>(pMeta_InHal, MTK_ISP_P2_CROP_REGION, sl2eCropInfo);
    trySetMetadata<MSize>(pMeta_InHal, MTK_ISP_P2_RESIZER_SIZE, sl2eRrzSize);
    return MTRUE;
}


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
