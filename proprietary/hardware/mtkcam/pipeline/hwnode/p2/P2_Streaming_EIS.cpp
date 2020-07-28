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

#include <cutils/properties.h>
#include "P2_StreamingProcessor.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Streaming_EIS
#define P2_TRACE        TRACE_STREAMING_EIS
#include "P2_LogHeader.h"

namespace P2
{

using NSCam::NSCamFeature::NSFeaturePipe::MASK_EIS;
using NSCam::NSCamFeature::NSFeaturePipe::MASK_EIS_25;
using NSCam::NSCamFeature::NSFeaturePipe::MASK_EIS_30;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

MBOOL StreamingProcessor::isEIS12() const
{
    TRACE_FUNC_ENTER();
    MUINT32 eisMode = mPipeUsageHint.mEISInfo.mode;
    TRACE_FUNC_EXIT();
    return eisMode && EIS_MODE_IS_EIS_12_ENABLED(eisMode);
}

MBOOL StreamingProcessor::isAdvEIS() const
{
    TRACE_FUNC_ENTER();
    MUINT32 eisMode = mPipeUsageHint.mEISInfo.mode;
    TRACE_FUNC("usagehint eisMode=0x%x", eisMode);
    TRACE_FUNC_EXIT();
    return eisMode && !EIS_MODE_IS_EIS_12_ENABLED(eisMode);
}

MBOOL StreamingProcessor::prepareEIS(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload);

    const sp<P2Request> &request = payload->mRequest;
    const P2Util::SimpleIO &io = payload->mIO;
    const Cropper &cropper = request->getCropper();
    FeaturePipeParam &featureParam = payload->mFeatureParam;

    if( isAdvEIS() )
    {
        MRectF eisMargin = getEISRRZOMargin(cropper);
        TRACE_S_FUNC(payload, "eisMargin=(%d,%d)(%dx%d)", eisMargin.p.x, eisMargin.p.y, eisMargin.s.w, eisMargin.s.h);
        payload->mIO.addMargin("eis", MSize(eisMargin.p.x, eisMargin.p.y));
        if( request->getExtraData().mIsRecording && cropper.isEISAppOn() )
        {
            prepareEISVar(featureParam, request, cropper, eisMargin);
            prepareEISMask(featureParam);
        }
    }

    TRACE_S_FUNC_EXIT(payload);
    return MTRUE;
}

MRectF StreamingProcessor::getEISRRZOMargin(const Cropper &cropper)
{
    TRACE_FUNC_ENTER();
    MUINT32 eisFactor = mPipeUsageHint.mEISInfo.factor;
    MRectF eisMargin;
    if( eisFactor )
    {
        MPointF offset( cropper.getP1OutSize().w * (eisFactor - 100) / eisFactor / 2,
                        cropper.getP1OutSize().h * (eisFactor - 100) / eisFactor / 2);
        eisMargin = MRectF(offset, MSizeF(cropper.getP1OutSize().w * 100 / eisFactor,
                                          cropper.getP1OutSize().h * 100 / eisFactor));
    }
    TRACE_FUNC_EXIT();
    return eisMargin;
}

MBOOL StreamingProcessor::prepareEISOutMeta(P2MetaSet& metaSet)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingProcessor::prepareEISVar(FeaturePipeParam &featureParam, const sp<P2Request> &request, const Cropper &cropper, const MRectF &eisMargin)
{
    TRACE_FUNC_ENTER();
    const LMVInfo &lmvInfo = cropper.getLMVInfo();

    featureParam.setVar<MRectF>(VAR_EIS_RRZO_CROP, eisMargin);
    featureParam.setVar<MINT32>(VAR_EIS_GMV_X, lmvInfo.gmvX);
    featureParam.setVar<MINT32>(VAR_EIS_GMV_Y, lmvInfo.gmvY);
    featureParam.setVar<MUINT32>(VAR_EIS_CONF_X, lmvInfo.confX);
    featureParam.setVar<MUINT32>(VAR_EIS_CONF_Y, lmvInfo.confY);
    featureParam.setVar<MINT64>(VAR_EIS_TIMESTAMP, lmvInfo.ts);

    featureParam.setVar<MSize>(VAR_EIS_SENSOR_SIZE, cropper.getSensorSize());
    MRect scalerCrop = cropper.getP1Crop();
    if( lmvInfo.isFrontBin )
    {
        scalerCrop.p.x >>= 1;
        scalerCrop.p.y >>= 1;
        scalerCrop.s.w >>= 1;
        scalerCrop.s.h >>= 1;
    }
    featureParam.setVar<MRect>(VAR_EIS_SCALER_CROP, scalerCrop);
    featureParam.setVar<MSize>(VAR_EIS_SCALER_SIZE, cropper.getP1OutSize());

    MINT32 expTime = 0, longExpTime = 0;
    getEISExpTime(expTime, longExpTime, lmvInfo, request->mMeta[IN_P1_HAL]);
    featureParam.setVar<MINT32>(VAR_EIS_EXP_TIME, expTime);
    featureParam.setVar<MINT32>(VAR_EIS_LONGEXP_TIME, longExpTime);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingProcessor::getEISExpTime(MINT32 &expTime, MINT32 &longExpTime, const LMVInfo &lmvInfo, const sp<P2Meta> &inHal)
{
    TRACE_FUNC_ENTER();
    MUINT32 expRatio = 0;
    IMetadata::Memory meta;
    RAWIspCamInfo *camInfo = NULL;
    expTime = longExpTime = lmvInfo.expTime;
    if( !isValid(inHal) )
    {
        MY_LOGW("cannot get in HAL metadata");
    }
    else if( !inHal->tryGet<IMetadata::Memory>(MTK_PROCESSOR_CAMINFO, meta) )
    {
        MY_LOGW("cannot get MTK_PROCESSOR_CAMINFO");
    }
    else if( (camInfo = (RAWIspCamInfo*)meta.array()) == NULL )
    {
        MY_LOGW("invalid MTK_PROCESSOR_CAMINFO");
    }
    else if( (expRatio = camInfo->rAEInfo.u4EISExpRatio) <= 0 )
    {
        MY_LOGW("invalid u4EISExpRatio(%d)", expRatio);
    }
    else
    {
        expTime = longExpTime * 100 / camInfo->rAEInfo.u4EISExpRatio;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingProcessor::prepareEISMask(FeaturePipeParam &featureParam)
{
    TRACE_FUNC_ENTER();
    MINT32 eisMode = mPipeUsageHint.mEISInfo.mode;

    if( EIS_MODE_IS_EIS_30_ENABLED(eisMode) )
    {
        featureParam.setFeatureMask(MASK_EIS, MTRUE);
        featureParam.setFeatureMask(MASK_EIS_30, MTRUE);
    }
    else if( EIS_MODE_IS_EIS_25_ENABLED(eisMode) )
    {
        featureParam.setFeatureMask(MASK_EIS, MTRUE);
        featureParam.setFeatureMask(MASK_EIS_25, MTRUE);
    }
    else if( EIS_MODE_IS_EIS_22_ENABLED(eisMode) )
    {
        featureParam.setFeatureMask(MASK_EIS, MTRUE);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

}; // namespace P2
