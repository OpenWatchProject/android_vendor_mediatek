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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <cutils/properties.h>
#include "P2_StreamingProcessor.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Streaming_3DNR
#define P2_TRACE        TRACE_STREAMING_3DNR
#include "P2_LogHeader.h"
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include <mtkcam3/feature/eis/eis_ext.h>
#include <camera_custom_3dnr.h>
#include <camera_custom_dsdn.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

#define UNUSED(var) (void)(var)

using namespace NSCam;
using namespace NSCam::NR3D;
using namespace NSCam::v3;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

// anonymous namespace
namespace
{
    static MINT32 getCurrIso(const sp<P2::P2Request> &request)
    {
        const MINT32 DEFAULT_ISO = 100;
        MINT iso = request->mP2Pack.getSensorData().mISO;
        if( iso == INVALID_P1_ISO_VAL )
        {
            iso = DEFAULT_ISO;
        }
        return iso;
    }
}

namespace P2
{

MVOID StreamingProcessor::init3DNR()
{
    m3dnrLogLevel = property_get_int32(NR3D_PROP_LOG_LEVEL, 0);
    m3dnrEnable = (mP2Info.getConfigInfo().mUsageHint.m3DNRMode & E3DNR_MODE_MASK_HAL_FORCE_SUPPORT);
    m3dnrDebugEnable = property_get_int32(NR3D_PROP_DEBUG_ENABLE, 0);

    for( MUINT32 sensorID : mP2Info.getConfigInfo().mAllSensorID )
    {
        mUtil3dnrMap[sensorID] = new Util3dnr(sensorID);
        mUtil3dnrMap[sensorID]->init(m3dnrDebugEnable);
    }

    // MY_LOGD("3dnr: usageHint.3DNRMode(0x%x)", mP2Info.getConfigInfo().mUsageHint.m3DNRMode);
}


MVOID StreamingProcessor::uninit3DNR()
{
    mUtil3dnrMap.clear();
}

MBOOL StreamingProcessor::is3DNRFlowEnabled(
    P2Util::SimpleIn& input, const ILog &log)
{
    sp<P2Request> &request = input.mRequest;
    IMetadata *inApp = input.mRequest->getMetaPtr(IN_APP);
    IMetadata *inHal = input.mRequest->getMetaPtr(IN_P1_HAL);

    // === check 3dnr-p2a ===
    MBOOL enable = MTRUE;

    // rule: check 3dnrUISupport and 3dnrHalSupport
    MINT32 e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    if( inApp == NULL ||
        !tryGetMetadata<MINT32>(inApp, MTK_NR_FEATURE_3DNR_MODE, e3DnrMode) )
    {
        MY_LOGW("!!warn: 3dnr: no MTK_NR_FEATURE_3DNR_MODE: appInMeta: %p", inApp);
    }

    MINT32 eHal3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    if (inHal == NULL ||
        !tryGetMetadata<MINT32>(inHal, MTK_DUALZOOM_3DNR_MODE, eHal3DnrMode))
    {
        // Only valid for dual cam. On single cam, we don't care HAL meta,
        // and can assume HAL is "ON" on single cam.
        eHal3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
    }
    MY_S_LOGD_IF(m3dnrLogLevel >= 1, log, "3dnr: Meta App: %d, Hal(dual-cam): %d", e3DnrMode, eHal3DnrMode);

    enable = (e3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON || eHal3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON);
    if (enable == MFALSE)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr@p2a: off due to e3DnrMode=%d, eHal3DnrMode=%d", e3DnrMode, eHal3DnrMode);
        enable = MFALSE;
    }
    // rule: if 3dnr-dsdn needed --> 3dnr-p2a off
    if (mIs3dnrEnabled_dsdn)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr@p2a: off: due to 3dnr@dsdn on");
        enable = MFALSE;
    }
    // rule: > 60fps, 3dnr-p2a off
    MINT32 minFps = input.mRequest->mP2Pack.getFrameData().mMinFps;
    if (mIs3dnrEnabled_dsdn && minFps >= 60)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr@p2a: off: (3dnr@dsdn on + minFps >= 60fps)");
        enable = MFALSE;
    }
    // rule: if 4k -> 3dnr@p2a: off
    if (mIs3dnrEnabled_dsdn && mP2Info.getConfigInfo().mUsageHint.mEisInfo.videoConfig == EIS::VIDEO_CFG_4K2K)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr@p2a: off: (3dnr@dsdn on + 4k2k case)");
        enable = MFALSE;
    }
    // rule: if iso < isoThreshold, 3dnr@p2a: off
    const MINT32 currIso = getCurrIso(request);
    const MINT32 isoThr_p2a = get3DNRIsoThreshold(request->getSensorID(), mIspProfile);
    if (currIso < isoThr_p2a)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 1, "3dnr@p2a: off: due to iso(%d) < isoThr(%d)", currIso, isoThr_p2a);
        enable = MFALSE;
    }

    // property: 3dnr@p2a property works only when loglevel >= 2
    if (m3dnrLogLevel >= 2)
    {
        int32_t is3dnrOnProp = property_get_int32(NR3D_PROP_3DNR_P2A_ENABLE,
            enable);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "3dnr@p2a: as-is: %s, to-be: %s (by property)",
            (enable != 0) ? "on" : "off",
            (is3dnrOnProp != 0) ? "on" : "off"
            );
        enable = is3dnrOnProp;

    }

    mIs3dnrEnabled_p2a = enable;
    return enable;
}


MBOOL StreamingProcessor::is3DNRDsdnFlowEnabled(
    P2Util::SimpleIn& input, const ILog &log)
{
    int32_t enable = MFALSE;

    // rule: enable when (dsdnState != 0 && imgWidth >=640
    if (mDSDNState /* && input.getInputSize().w >= 640 */)
    {
        enable = MTRUE;
    }

    // 3dnr@dsdn: set on/off by property
    if (m3dnrDebugEnable)
    {
        int32_t is3dnrOnProp = property_get_int32(NR3D_PROP_3DNR_DSDN_ENABLE, enable);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "3dnr@dsdn: as-is: %s, to-be: %s (by property)",
            (enable != 0) ? "on" : "off",
            (is3dnrOnProp != 0) ? "on" : "off"
            );

        enable = is3dnrOnProp;
    }

    MY_LOGD_IF(m3dnrLogLevel >= 1 ,
        "3dnr: enable=%d, DSDN.mode=%d, inputSize(%dx%d)",
        enable,
        mPipeUsageHint.mDSDNParam.mMode,
        input.getInputSize().w, input.getInputSize().h);

    mIs3dnrEnabled_dsdn = enable;
    return enable;
}


MBOOL StreamingProcessor::getInputCrop3DNR(
    MBOOL &isEIS4K, MBOOL &isIMGO, MRect &inputCrop,
    P2Util::SimpleIn& input, const ILog &log) const
{
    MSize nr3dInputSize = input.getInputSize();

    isEIS4K = isEIS12() &&
              ((nr3dInputSize.w >= VR_UHD_W && nr3dInputSize.h >= VR_UHD_H) ||
               (nr3dInputSize.h >= VR_UHD_W && nr3dInputSize.w >= VR_UHD_H));

    isIMGO = !(input.isResized());
    sp<Cropper> cropper = input.mRequest->getCropper();

    if (isEIS4K)
    {
        MUINT32 cropFlag = Cropper::USE_EIS_12;
        cropFlag |= input.isResized() ? Cropper::USE_RESIZED : 0;
        MCropRect cropRect = cropper->calcViewAngle(
            log,
            cropper->getActiveCrop().s,
            cropFlag);
        inputCrop.p = cropRect.p_integral;
        inputCrop.s = cropRect.s;
    }
    else if (isIMGO)
    {
        inputCrop = cropper->getP1Crop();
    }
    else
    {
        inputCrop.p.x = 0;
        inputCrop.p.y = 0;
        inputCrop.s = nr3dInputSize;
    }

    if (m3dnrLogLevel >= 2)
    {
        MY_S_LOGD(log,
            "[3DNR] isEIS4K: %d, isIMGO: %d, input(%d,%d), inputCrop(%d,%d;%d,%d)",
            (isEIS4K ? 1 : 0), (isIMGO ? 1 : 0),
            nr3dInputSize.w, nr3dInputSize.h,
            inputCrop.p.x, inputCrop.p.y, inputCrop.s.w, inputCrop.s.h);
    }

    return (isEIS4K || isIMGO);
}

MINT32 StreamingProcessor::get3DNRIsoThreshold(MUINT32 /*sensorID*/, MUINT8 ispProfile) const
{
    MINT32 isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(ispProfile, m3dnrDebugEnable);
    MY_LOGD_IF(m3dnrLogLevel >= 1,"Get isoThreshold : %d", isoThreshold);
    return isoThreshold;
}

MBOOL StreamingProcessor::prepare3DNR_FeatureData(
   P2Util::SimpleIn& input, const ILog &log
    ) const
{
    UNUSED(log);
    TRACE_S_FUNC_ENTER(log);

    FeaturePipeParam &featureParam = input.mFeatureParam;
    sp<P2Request> &request = input.mRequest;
    const sp<Util3dnr>& util3dnr = mUtil3dnrMap.at(request->getSensorID());
    if (util3dnr == NULL)
    {
        MY_LOGW("No util3dnr!");
        return MFALSE;
    }

    Util3dnrParam in;
    Util3dnrResult out;

    sp<Cropper> cropper = request->getCropper();

    // common
    in.logLevel = m3dnrLogLevel;
    in.isRscNeeded = HAS_RSC(featureParam.mFeatureMask);
    in.iso = getCurrIso(request);
    in.p1CropRect = cropper->getP1Crop(); // p1 crop rect

    LMVInfo lmv = cropper->getLMVInfo();
    out.mvInfo.status = lmv.is_valid ? NR3DMVInfo::VALID : NR3DMVInfo::INVALID;
    out.mvInfo.x_int = lmv.x_int;
    out.mvInfo.y_int = lmv.y_int;
    out.mvInfo.gmvX = lmv.gmvX;
    out.mvInfo.gmvY = lmv.gmvY;
    out.mvInfo.confX = lmv.confX;
    out.mvInfo.confY = lmv.confY;
    out.mvInfo.maxGMV = lmv.gmvMax;
    out.mvInfo.maxGMVX = lmv.gmvMax;
    out.mvInfo.maxGMVY = lmv.gmvMax;
    out.mvInfo.ts = lmv.ts;

    // p2a related
    in.is3dnrOn_p2a = mIs3dnrEnabled_p2a;
    if (mIs3dnrEnabled_p2a)
    {
        MBOOL isEIS4k = MFALSE;
        MRect inputCrop;

        in.isIMGO_p2a = isEIS4k;
        in.isoThr_p2a = get3DNRIsoThreshold(request->getSensorID(), mIspProfile);
        in.isCRZ_p2a = MFALSE;
        in.dstSize_p2a =  cropper->getP1OutSize(); // rrzo size
    }
    // dsdn related
    in.is3dnrOn_dsdn = mIs3dnrEnabled_dsdn;
    if (mIs3dnrEnabled_dsdn)
    {
        // get dsdn iso threshold
        const P2PlatInfo::NVRamDSDN &nvramData =
            input.mRequest->mP2Pack.getSensorData().mNvramDsdn;
        in.isoThr_dsdn = (mDSDNDebugISO_H > 0) ? mDSDNDebugISO_H : nvramData.mIsoThreshold;
    }

    util3dnr->prepareFeatureData(in, out);

    // === prepare setVar for FeaturePipe ===
    featureParam.setVar<MBOOL>(SFP_VAR::NR3D_EIS_IS_CRZ_MODE, in.isCRZ_p2a);
    featureParam.setVar<NR3D::NR3DMVInfo>(SFP_VAR::NR3D_MV_INFO, out.mvInfo);
    featureParam.setVar<MUINT32>(SFP_VAR::NR3D_ISO, in.iso);

    // p2a: 3dnr on/off
    featureParam.setVar<MBOOL>(SFP_VAR::NR3D_P2A_CAN_ENABLE_ON_FRAME, in.is3dnrOn_p2a);
    if (in.is3dnrOn_p2a)
    {
        featureParam.setVar<MUINT32>(SFP_VAR::NR3D_P2A_ISO_THRESHOLD, in.isoThr_p2a);
    }
    // dsdn: 3dnr on/off
    featureParam.setVar<MBOOL>(SFP_VAR::NR3D_DSDN_CAN_ENABLE_ON_FRAME, in.is3dnrOn_dsdn);
    if (in.is3dnrOn_dsdn)
    {
        featureParam.setVar<MUINT32>(SFP_VAR::NR3D_DSDN_ISO_THRESHOLD, in.isoThr_dsdn);
    }

    if (out.pGyroData != NULL && (out.pGyroData)->isValid == MTRUE)
    {
        featureParam.setVar<NSCam::NR3D::GyroData>(SFP_VAR::NR3D_GYRO, *(out.pGyroData));
    }

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MBOOL StreamingProcessor::prepare3DNR(P2Util::SimpleIn& input, const ILog &log)
{
    TRACE_S_FUNC_ENTER(log);

    MBOOL uiSupport =
        ((mP2Info.getConfigInfo().mUsageHint.m3DNRMode & E3DNR_MODE_MASK_UI_SUPPORT) != 0);
    if (uiSupport || m3dnrEnable)
    {
        FeaturePipeParam &featureParam = input.mFeatureParam;
        IMetadata *inHal = input.mRequest->getMetaPtr(IN_P1_HAL);

        // get ispProfile
        if (!tryGetMetadata<MUINT8>(inHal, MTK_3A_ISP_PROFILE, mIspProfile))
        {
            MY_LOGD_IF(m3dnrLogLevel >= 1,"no ISPProfile from HalMeta");
            mIspProfile = 0;
        }

        // set RSC for 3dnr
        featureParam.setFeatureMask(MASK_RSC,
            (mP2Info.getConfigInfo().mUsageHint.m3DNRMode & NSCam::NR3D::E3DNR_MODE_MASK_RSC_EN));

        // check 3dnr flow: order: dsdn -> p2a
        mIs3dnrEnabled_dsdn = is3DNRDsdnFlowEnabled(input, log);
        mIs3dnrEnabled_p2a = is3DNRFlowEnabled(input, log);
        prepare3DNR_FeatureData(input, log);
        featureParam.setFeatureMask(MASK_3DNR, (mIs3dnrEnabled_p2a || mIs3dnrEnabled_dsdn));

        MY_LOGD_IF(m3dnrLogLevel >= 1,"3dnr: p2aFlow=%d, dsdnFlow=%d",
            mIs3dnrEnabled_p2a, mIs3dnrEnabled_dsdn);
    }

    MY_S_LOGD_IF((m3dnrLogLevel >= 1), log, "3dnr@p2a=%d, 3dnr@dsdn=%d",
        mIs3dnrEnabled_p2a, mIs3dnrEnabled_dsdn);

    TRACE_S_FUNC_EXIT(log);
    return (mIs3dnrEnabled_p2a || mIs3dnrEnabled_dsdn);
}

}; // namespace P2

