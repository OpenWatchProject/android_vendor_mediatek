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

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_NR3D);

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace NSCam::NR3D;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define BID2Name DepthMapPipeNode::onDumpBIDToName
MBOOL
NR3DCommon::
init(MUINT32 sensorIdx)
{
#ifdef GTEST
    return MTRUE;
#endif
    miSensorIdx = sensorIdx;

    // 3dnr
    mp3dnrHal = NSCam::NSIoPipe::NSPostProc::Hal3dnrBase::createInstance(DEPTH_3DNR_USER, miSensorIdx);
    mDebugEnable  = ::property_get_int32(NR3D_PROP_DEBUG_ENABLE, 0);
    return mp3dnrHal->init(DEPTH_3DNR_USER);
}

MBOOL
NR3DCommon::
uninit()
{
    if(mp3dnrHal != nullptr)
    {
        mp3dnrHal->uninit(DEPTH_3DNR_USER);
        mp3dnrHal->destroyInstance(DEPTH_3DNR_USER, miSensorIdx);
        mp3dnrHal = nullptr;
    }

    return MTRUE;
}

#define GET_INPUT_META_AND_CHK(request, bid, targetPtr)\
    if(!pRequest->getRequestMetadata({.bufferID=bid, .ioType=eBUFFER_IOTYPE_INPUT}\
                                   , targetPtr))\
    {\
        MY_LOGE("reqID=%d, metadata(id:%d, %s) not exist, please check!",\
                    request->getRequestNo(), bid, BID2Name(bid));\
        return MFALSE;\
    }


MBOOL NR3DCommon::perform3dnr(
    DepthMapRequestPtr& pRequest,
    IHal3A* p3AHAL,
    sp<DepthMapFlowOption> pFlowOption,
    Stereo3ATuningRes& tuningResult)
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
    if(!tryGetMetadata<MINT32>(pMeta_P1Ret, MTK_SENSOR_SENSITIVITY, iISOVal))
    {
        MY_LOGE("Failed to get ISO value!");
        return MFALSE;
    }
    // query eis info, eis region = lmv info
    eis_region eisInfo;
    MBOOL bIsQueryLmvSuccess = queryEisRegion(pMeta_InHal, eisInfo);
    if(!bIsQueryLmvSuccess)
    {
        MY_LOGW("Failed to query EIS region! May causes bad 3dnr performance");
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

    // feat mask
    nr3dHalParam.featMask = NR3D_FEAT_MASK_VSDOF;
    // 3a related
    nr3dHalParam.pTuningData = tuningResult.tuningRes_main1.tuningBuffer;
    nr3dHalParam.p3A = p3AHAL;

    // generic
    nr3dHalParam.frameNo = pRequest->getRequestNo();
    nr3dHalParam.needChkIso = MTRUE;
    nr3dHalParam.iso = iISOVal;
    nr3dHalParam.isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(-1, mDebugEnable);

    // imgi related
    nr3dHalParam.isCRZUsed = MFALSE;
    nr3dHalParam.isIMGO = MFALSE;
    nr3dHalParam.isBinning = MFALSE; // no need for now
    // lmv related info
    if (bIsQueryLmvSuccess)
    {
        nr3dHalParam.gmvInfo.gmvX = eisInfo.gmvX;  // from Hal Metadata:  MTK_EIS_REGION
        nr3dHalParam.gmvInfo.gmvY = eisInfo.gmvY; // same as above
        nr3dHalParam.gmvInfo.x_int = eisInfo.x_int; // same as above
        nr3dHalParam.gmvInfo.y_int = eisInfo.y_int; // same as above
        nr3dHalParam.gmvInfo.confX = eisInfo.confX; // same as above
        nr3dHalParam.gmvInfo.confY= eisInfo.confY; // same as above

        MINT32 isLmvValid = 1;
        if (!tryGetMetadata<MINT32>(pMeta_InHal, MTK_LMV_VALIDITY, isLmvValid))
        {
            isLmvValid = 1; // Absent. Default behavior is 1 on single cam
        }
        nr3dHalParam.gmvInfo.status = (isLmvValid == 1) ? NR3DMVInfo::VALID : NR3DMVInfo::INVALID;
    }
    // vipi related
    IImageBuffer *pVipiImgBuf = pFlowOption->get3DNRVIPIBuffer();
    if (pVipiImgBuf != nullptr)
    {
        nr3dHalParam.vipiInfo = NR3DHALParam::VipiInfo(MTRUE,
            pVipiImgBuf->getImgFormat(), pVipiImgBuf->getBufStridesInBytes(0),
            pVipiImgBuf->getImgSize()
            );
    }
    // output related, ex: img3o
    nr3dHalParam.dstRect = MRect(MPoint(0, 0), frameBuf_RSRAW1->getImgSize());
    if (mp3dnrHal->update3DNRMvInfo(nr3dHalParam, mNr3dResult.gmvInfo, mNr3dResult.isGMVInfoUpdated) != MTRUE)
    {
        MY_LOGW("!!warn: 3dnr->update3DNRMVInfo return false, which may affect 3dnr quality");
    }

    if (mp3dnrHal->do3dnrFlow(nr3dHalParam, mNr3dResult) == MTRUE)
    {
        // new G_3dnr_hal
        NR3DTuningInfo tuningInfo;
        tuningInfo.canEnable3dnrOnFrame = mNr3dResult.nr3dHwParam.ctrl_onEn;
        tuningInfo.isoThreshold = nr3dHalParam.isoThreshold;
        tuningInfo.mvInfo = mNr3dResult.gmvInfo;
        tuningInfo.inputSize = frameBuf_RSRAW1->getImgSize();
        tuningInfo.inputCrop = MRect(MPoint(0,0), frameBuf_RSRAW1->getImgSize());
        tuningInfo.gyroData = GyroData(); // don't use Gyro for now
        tuningInfo.nr3dHwParam = mNr3dResult.nr3dHwParam;
        bRet = mp3dnrHal->updateISPMetadata(pMeta_InHal, tuningInfo);

        mp3dnrHal->configNR3D_legacy(nr3dHalParam.pTuningData, p3AHAL,  tuningInfo.inputCrop,
            mNr3dResult.nr3dHwParam);
    }
    else
    {
        MY_LOGW("!!warn: do3dnrFlow failed");
    }


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
    MINT32 frameReset = (e3dnrMode == MTK_NR_FEATURE_3DNR_MODE_OFF);
    MINT32 isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(-1, mDebugEnable);
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
