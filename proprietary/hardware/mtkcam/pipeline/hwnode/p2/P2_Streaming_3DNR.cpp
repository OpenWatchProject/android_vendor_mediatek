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
#include <mtkcam/feature/3dnr/3dnr_defs.h>
#include <mtkcam/feature/eis/eis_ext.h>
#include <camera_custom_3dnr.h>

#define UNUSED(var) (void)(var)

using namespace NSCam;
using namespace NSCam::NR3D;
using namespace NSCam::v3;
using namespace NSCam::NSCamFeature::NSFeaturePipe;


namespace P2
{

MINT32 getIso3DNR(const IMetadata &meta)
{
    static const MINT32 DEFAULT_ISO = 100;

    MINT32 iso = DEFAULT_ISO;
    if (!tryGet(meta, MTK_SENSOR_SENSITIVITY, iso))
        iso = DEFAULT_ISO;

    return iso;
}


void StreamingProcessor::init3DNR()
{
    m3dnrDebugLevel = property_get_int32("vendor.camera.3dnr.log.level", 0);

    mUtil3dnr = new Util3dnr(mP2Info->mSensorID);
    mUtil3dnr->init(mConfigParam.mUsageHint.m3DNRMode &
        E3DNR_MODE_MASK_HAL_FORCE_SUPPORT);

    MY_LOGD("usageHint.3DNRMode(0x%x)", mConfigParam.mUsageHint.m3DNRMode);
}


void StreamingProcessor::uninit3DNR()
{
    mUtil3dnr = NULL;
}


MBOOL StreamingProcessor::isAPEnabled_3DNR(MINT32 force3DNR, IMetadata *appInMeta, IMetadata *halInMeta)
{
    UNUSED(halInMeta);

    MINT32 e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;

    if( appInMeta == NULL ||
        !tryGet<MINT32>(appInMeta, MTK_NR_FEATURE_3DNR_MODE, e3DnrMode) )
    {
        MY_LOGW("no MTK_NR_FEATURE_3DNR_MODE: appInMeta: %p", appInMeta);
    }

    if (force3DNR)
    {
        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.camera.3dnr.enable", EnableOption, "n");
        if (EnableOption[0] == '1')
        {
            e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
        }
        else if (EnableOption[0] == '0')
        {
            e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
        }
    }

    return (e3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON);
}


MBOOL StreamingProcessor::getInputCrop3DNR(
    MBOOL &isEIS4K, MBOOL &isIMGO, MRect &inputCrop,
    const sp<Payload> &payload, P2MetaSet &metaSet, const Logger &logger)
{
    MSize nr3dInputSize = payload->mIO.getInputSize();

    isEIS4K = isEIS12() &&
              ((nr3dInputSize.w >= VR_UHD_W && nr3dInputSize.h >= VR_UHD_H) ||
               (nr3dInputSize.h >= VR_UHD_W && nr3dInputSize.w >= VR_UHD_H));

    isIMGO = !(payload->mIO.isResized());
    Cropper cropper = payload->mRequest->getCropper();

    if (isEIS4K)
    {
        MUINT32 cropFlag = Cropper::USE_EIS_12;
        cropFlag |= payload->mIO.isResized() ? Cropper::USE_RESIZED : 0;
        MCropRect cropRect = cropper.calcViewAngle(
            cropper.getActiveCrop().s,
            cropFlag);
        inputCrop.p = cropRect.p_integral;
        inputCrop.s = cropRect.s;
    }
    else if (isIMGO)
    {
        inputCrop = cropper.getP1Crop();
    }
    else
    {
        inputCrop.p.x = 0;
        inputCrop.p.y = 0;
        inputCrop.s = nr3dInputSize;
    }

    if (m3dnrDebugLevel >= 2)
    {
        MY_S_LOGD(logger,
            "[3DNR] isEIS4K: %d, isIMGO: %d, input(%d,%d), inputCrop(%d,%d;%d,%d)",
            (isEIS4K ? 1 : 0), (isIMGO ? 1 : 0),
            nr3dInputSize.w, nr3dInputSize.h,
            inputCrop.p.x, inputCrop.p.y, inputCrop.s.w, inputCrop.s.h);
    }

    return (isEIS4K || isIMGO);
}


MBOOL StreamingProcessor::prepare3DNR(const sp<Payload> &payload, P2MetaSet &metaSet, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    FeaturePipeParam &featureParam = payload->mFeatureParam;
    Cropper cropper = payload->mRequest->getCropper();
    LMVInfo lmv = cropper.getLMVInfo();

    MBOOL forceSupport =
        ((mConfigParam.mUsageHint.m3DNRMode & E3DNR_MODE_MASK_HAL_FORCE_SUPPORT) != 0);
    MBOOL appEn3DNR = isAPEnabled_3DNR(
        forceSupport, &metaSet.mInApp, &metaSet.mInHal);

    if (m3dnrDebugLevel >= 2)
    {
        MY_S_LOGD(logger, "[3DNR] appEn3DNR: %d, forceSupport: %d",
                (appEn3DNR ? 1 : 0), (forceSupport ? 1 : 0));
    }

    if (appEn3DNR)
    {
        featureParam.setFeatureMask(MASK_3DNR, appEn3DNR);
        featureParam.setFeatureMask(MASK_3DNR_RSC, (mConfigParam.mUsageHint.m3DNRMode &
            NSCam::NR3D::E3DNR_MODE_MASK_RSC_EN));
    }

    MBOOL isEIS4K, isIMGO;
    MRect inputCrop;
    MBOOL isInputCrop = getInputCrop3DNR(isEIS4K, isIMGO, inputCrop,
            payload, metaSet, logger);


    MUINT8 ispProfile = 0;
    if (!tryGet<MUINT8>(&metaSet.mInHal, MTK_3A_ISP_PROFILE, ispProfile))
    {
        MY_LOGD_IF(m3dnrDebugLevel,"no ISPProfile from HalMeta");
    }

    MBOOL canEnable3DNR =
        prepare3DNR_FeatureData(appEn3DNR, isEIS4K, isIMGO, payload, metaSet, ispProfile, logger);
    prepare3DNR_ISPData(canEnable3DNR, inputCrop, payload, metaSet, ispProfile, logger);

    TRACE_S_FUNC_EXIT(logger);
    return appEn3DNR;
}


MBOOL StreamingProcessor::prepare3DNR_FeatureData(
    MBOOL appEn3DNR, MBOOL isEIS4K, MBOOL isIMGO,
    const sp<Payload> &payload, P2MetaSet &metaSet, MUINT8 ispProfile, const Logger &logger)
{
    UNUSED(logger);
    TRACE_S_FUNC_EXIT(logger);

    FeaturePipeParam &featureParam = payload->mFeatureParam;
    Cropper cropper = payload->mRequest->getCropper();

    LMVInfo lmv = cropper.getLMVInfo();
    NR3DMVInfo mvInfo;
    mvInfo.status = NR3DMVInfo::VALID; // TODO refer to lmv.isvalid
    mvInfo.x_int = lmv.x_int;
    mvInfo.y_int = lmv.y_int;
    mvInfo.gmvX = lmv.gmvX;
    mvInfo.gmvY = lmv.gmvY;
    mvInfo.confX = lmv.confX;
    mvInfo.confY = lmv.confY;
    mvInfo.maxGMV = lmv.gmvMax;

    MINT32 iso = getIso3DNR(metaSet.mInApp);

    MINT32 isoThreshold = get3DNRIsoThreshold(ispProfile);
    MBOOL canEnable3dnr = mUtil3dnr->canEnable3dnr(appEn3DNR, iso, isoThreshold);
    MSize rrzoSize = cropper.getP1OutSize();
    MRect p1Crop = cropper.getP1Crop();

    mUtil3dnr->modifyMVInfo(canEnable3dnr, isIMGO, p1Crop, rrzoSize, mvInfo);

    mUtil3dnr->prepareFeatureData(canEnable3dnr, mvInfo, iso, isoThreshold, isEIS4K, featureParam);
    mUtil3dnr->prepareGyro(NULL, &featureParam);

    TRACE_S_FUNC_EXIT(logger);
    return canEnable3dnr;
}


MVOID StreamingProcessor::prepare3DNR_ISPData(
    MBOOL canEnable3dnr, const MRect &inputCrop,
    const sp<Payload> &payload, P2MetaSet &metaSet, MUINT8 ispProfile, const Logger &logger)
{
    UNUSED(logger);

    FeaturePipeParam &featureParam = payload->mFeatureParam;
    NSCam::NSIoPipe::QParams &qParams = featureParam.mQParams;
    Cropper cropper = payload->mRequest->getCropper();

    LMVInfo lmv = cropper.getLMVInfo();
    NR3DMVInfo mvInfo;
    mvInfo.status = NR3DMVInfo::VALID; // TODO refer to lmv.isvalid
    mvInfo.x_int = lmv.x_int;
    mvInfo.y_int = lmv.y_int;
    mvInfo.gmvX = lmv.gmvX;
    mvInfo.gmvY = lmv.gmvY;
    mvInfo.confX = lmv.confX;
    mvInfo.confY = lmv.confY;
    mvInfo.maxGMV = lmv.gmvMax;

    MBOOL isSl2eEnabled =
        ((mConfigParam.mUsageHint.m3DNRMode & E3DNR_MODE_MASK_SL2E_EN) != 0);
    MINT32 iso = getIso3DNR(metaSet.mInApp);
    MSize nr3dInputSize = payload->mIO.getInputSize();

    MINT32 isoThreshold = get3DNRIsoThreshold(ispProfile);

    mUtil3dnr->prepareISPData(canEnable3dnr, mvInfo,
        nr3dInputSize, inputCrop, iso, isoThreshold, isSl2eEnabled,
        &metaSet.mInHal);
}

MINT32 StreamingProcessor::get3DNRIsoThreshold(MUINT8 ispProfile)
{
    MINT32 isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(ispProfile, mUtil3dnr->is3dnrDebugMode());
    MY_LOGD_IF(m3dnrDebugLevel,"Get isoThreshold : %d", isoThreshold);
    return isoThreshold;
}


}; // namespace P2
