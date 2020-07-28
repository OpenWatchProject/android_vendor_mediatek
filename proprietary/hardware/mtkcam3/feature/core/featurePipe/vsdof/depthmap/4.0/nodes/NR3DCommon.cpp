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
#include <camera_custom_3dnr.h>
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

using namespace NS3Av3;
#define BID2Name DepthMapPipeNode::onDumpBIDToName
MBOOL
NR3DCommon::
init(
    char* userName,
    MUINT32 iSensorIdx
)
{
#ifdef GTEST
    return MTRUE;
#endif
    miSensorIdx= iSensorIdx;
    msUserName = userName;
    //
    mDebugEnable  = ::property_get_int32(NR3D_PROP_DEBUG_ENABLE, 0);
    // 3dnr
    mp3dnrHal = NSCam::NSIoPipe::NSPostProc::Hal3dnrBase::createInstance(msUserName, miSensorIdx);
    mp3dnrUtil = new Util3dnr(miSensorIdx);
    mp3dnrUtil->init(mDebugEnable);
    //
    mp3AHal = MAKE_Hal3A(miSensorIdx, msUserName);

    return mp3dnrHal->init(msUserName);
}

MBOOL
NR3DCommon::
uninit()
{
    if(mp3dnrHal != nullptr)
    {
        mp3dnrHal->uninit(msUserName);
        mp3dnrHal->destroyInstance(msUserName, miSensorIdx);
        mp3dnrHal = nullptr;
    }

    mp3dnrUtil = nullptr;

    if(mp3AHal != nullptr)
    {
        mp3AHal->destroyInstance(msUserName);
        mp3AHal = nullptr;
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

MBOOL
NR3DCommon::
perform3dnr(
    DepthMapRequestPtr& pRequest,
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
    nr3dHalParam.featMask = NR3D_FEAT_MASK_VSDOF;
    // 3a related
    nr3dHalParam.pTuningData = tuningResult.tuningRes_main1.tuningBuffer;
    nr3dHalParam.p3A = mp3AHal;
    // generic
    nr3dHalParam.frameNo = pRequest->getRequestNo();
    nr3dHalParam.needChkIso = MTRUE;
    nr3dHalParam.iso = iISOVal;
    nr3dHalParam.isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(pFlowOption->getIspProfile_Bayer(pRequest), mDebugEnable);
    // imgi related
    nr3dHalParam.isCRZUsed = MFALSE;
    nr3dHalParam.isIMGO = MFALSE;
    nr3dHalParam.isBinning = MFALSE; // no need for now

    // lmv related info, No lmv -> bad 3dnr performance
    if(bIsQueryLmvSuccess)
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
    //

    if (mp3dnrHal->update3DNRMvInfo(nr3dHalParam, mNr3dResult.gmvInfo, mNr3dResult.isGMVInfoUpdated) != MTRUE)
    {
        MY_LOGW("!!warn: 3dnr->update3DNRMVInfo return false, which may affect 3dnr quality");
    }

    bRet = mp3dnrHal->do3dnrFlow(nr3dHalParam, mNr3dResult);

    // prepare tuning info
    NR3DTuningInfo tuningInfo;
    MINT32 isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(pFlowOption->getIspProfile_Bayer(pRequest), mDebugEnable);
    MINT32 iso = iISOVal;
    tuningInfo.canEnable3dnrOnFrame = mNr3dResult.nr3dHwParam.ctrl_onEn;
    tuningInfo.isoThreshold = nr3dHalParam.isoThreshold;
    tuningInfo.mvInfo = mNr3dResult.gmvInfo;
    tuningInfo.inputSize = frameBuf_RSRAW1->getImgSize();
    tuningInfo.inputCrop = MRect(MPoint(0,0), frameBuf_RSRAW1->getImgSize());
    tuningInfo.gyroData = GyroData(); // don't use Gyro for now
    tuningInfo.nr3dHwParam = mNr3dResult.nr3dHwParam;
    bRet = mp3dnrHal->updateISPMetadata(pMeta_InHal, tuningInfo);

    VSDOF_LOGD("reqID=%d 3ndr on/off = %d", pRequest->getRequestNo(), bRet);
    return bRet;
}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
