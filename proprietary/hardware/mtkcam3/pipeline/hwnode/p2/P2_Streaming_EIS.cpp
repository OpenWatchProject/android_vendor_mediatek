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

#include <mtkcam3/feature/fsc/fsc_defs.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

namespace P2
{

using NSCam::NSCamFeature::NSFeaturePipe::MASK_EIS;
using NSCam::NSCamFeature::NSFeaturePipe::MASK_EIS_30;
using namespace NSCam::NSCamFeature::NSFeaturePipe;

static MVOID getEISRatio(float &widthRatio, float &heightRatio, const MSize inputSize, const MSize outputSize, float factor)
{
    MSizeF inSize(inputSize.w, inputSize.h);
    MSizeF outSize(outputSize.w, outputSize.h);
    float tolerance = 1.05;
    MBOOL isWideIn = (inSize.h/inSize.w) < (9.0/16)*tolerance;
    MBOOL isWideOut = (outSize.h/outSize.w) < (9.0/16)*tolerance;
    float ratio = 100/factor;
    MSizeF warpOutSize;

    //Different ratio between in/out
    if (isWideIn != isWideOut)
    {
        if( (inSize.w/inSize.h) >= (outSize.w/outSize.h) )
        {
            warpOutSize.w = (inSize.h*ratio/(outSize.h/outSize.w)) * tolerance;
            warpOutSize.h = inSize.h*ratio;
        }
        else
        {
            warpOutSize.w = inSize.w*ratio;
            warpOutSize.h = (inSize.w*ratio/(outSize.w/outSize.h)) * tolerance;
        }
    }
    else
    {
        warpOutSize.w = inSize.w*ratio;
        warpOutSize.h = inSize.h*ratio;
    }

    widthRatio = warpOutSize.w / inputSize.w;
    heightRatio = warpOutSize.h / inputSize.h;

    TRACE_FUNC("in(%d,%d)->out(%d,%d) EisRatio:%f Final ratio=(%f,%f)", inputSize.w, inputSize.h, outputSize.w, outputSize.h,
        ratio, widthRatio, heightRatio);
}

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
    return eisMode && EIS_MODE_IS_EIS_ADVANCED_ENABLED(eisMode);
}

MRectF StreamingProcessor::getEISRRZOMargin(P2Util::SimpleIn& input) const
{
    TRACE_FUNC_ENTER();
    MUINT32 eisFactor = mPipeUsageHint.mEISInfo.factor;
    float inCropRatio = 1.0f;
    const sp<P2Request> &request = input.mRequest;
    const sp<Cropper> cropper = request->getCropper();
    FeaturePipeParam &featureParam = input.mFeatureParam;

    featureParam.setVar<MFLOAT>(SFP_VAR::EIS_WIDTH_RATIO, mEisWidthRatio);
    featureParam.setVar<MFLOAT>(SFP_VAR::EIS_HEIGHT_RATIO, mEisHeightRatio);
    input.addCropRatio("eis", mEisWidthRatio, mEisHeightRatio);

    if( HAS_FSC(featureParam.mFeatureMask) )
    {
        IMetadata *inHal = input.mRequest->getMetaPtr(IN_P1_HAL);
        if( inHal )
        {
            // Get FSC crop data
            IMetadata::IEntry cropEntry = inHal->entryFor(MTK_FSC_CROP_DATA);
            if( cropEntry.count() )
            {
                IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
                NSCam::FSC::FSC_CROPPING_DATA_STRUCT *cropData = (NSCam::FSC::FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
                inCropRatio = cropData->image_scale;
            }
        }
    }

    float finalRatioW = mEisWidthRatio;
    float finalRatiotH = mEisHeightRatio;
    if( inCropRatio > 0 )
    {
        finalRatioW = inCropRatio * mEisWidthRatio;
        finalRatiotH = inCropRatio * mEisHeightRatio;
    }
    else
    {
        MY_LOGW("Ignore invalid inCropRatio(%f). Apply EIS CropRatio(%f,%f) only", inCropRatio, finalRatioW, finalRatiotH);
    }

    MRectF eisMargin;
    if( finalRatioW && finalRatiotH )
    {
        MPointF offset( cropper->getP1OutSize().w * (1.0f - finalRatioW) / 2,
                        cropper->getP1OutSize().h * (1.0f - finalRatiotH) / 2);
        eisMargin = MRectF(offset, MSizeF(cropper->getP1OutSize().w * finalRatioW,
                                          cropper->getP1OutSize().h * finalRatiotH));
    }
    TRACE_FUNC("inCropRatio(%f), eisFactor(%d), outCropRatio(%f, %f). P1OutSize(%d,%d)=>RRZOMargin(%f,%f)(%fx%f)",
                inCropRatio, eisFactor, finalRatioW, finalRatiotH,
                cropper->getP1OutSize().w, cropper->getP1OutSize().h,
                eisMargin.p.x, eisMargin.p.y, eisMargin.s.w, eisMargin.s.h);
    TRACE_FUNC_EXIT();
    return eisMargin;
}

MBOOL StreamingProcessor::prepareEISVar(P2Util::SimpleIn& input, FeaturePipeParam &featureParam,
    const sp<P2Request> &request, const sp<Cropper> &cropper, const MRectF &eisMargin) const
{
    (void)input;
    TRACE_FUNC_ENTER();
    const LMVInfo &lmvInfo = cropper->getLMVInfo();

    featureParam.setVar<MRectF>(SFP_VAR::EIS_RRZO_CROP, eisMargin);

    featureParam.setVar<MINT32>(SFP_VAR::EIS_GMV_X, lmvInfo.gmvX);
    featureParam.setVar<MINT32>(SFP_VAR::EIS_GMV_Y, lmvInfo.gmvY);
    featureParam.setVar<MUINT32>(SFP_VAR::EIS_CONF_X, lmvInfo.confX);
    featureParam.setVar<MUINT32>(SFP_VAR::EIS_CONF_Y, lmvInfo.confY);
    featureParam.setVar<MINT64>(SFP_VAR::EIS_TIMESTAMP, lmvInfo.ts);

    featureParam.setVar<MSize>(SFP_VAR::EIS_SENSOR_SIZE, cropper->getSensorSize());
    MRect scalerCrop = cropper->getP1Crop();
    if( lmvInfo.isFrontBin )
    {
        scalerCrop.p.x >>= 1;
        scalerCrop.p.y >>= 1;
        scalerCrop.s.w >>= 1;
        scalerCrop.s.h >>= 1;
    }
    featureParam.setVar<MRect>(SFP_VAR::EIS_SCALER_CROP, scalerCrop);
    featureParam.setVar<MSize>(SFP_VAR::EIS_SCALER_SIZE, cropper->getP1OutSize());

    MINT32 expTime = 0, longExpTime = 0;
    getEISExpTime(expTime, longExpTime, lmvInfo, request->mMeta[IN_P1_HAL]);
    featureParam.setVar<MINT32>(SFP_VAR::EIS_EXP_TIME, expTime);
    featureParam.setVar<MINT32>(SFP_VAR::EIS_LONGEXP_TIME, longExpTime);

    MINT32 sensorMode = request->mP2Pack.getSensorData().mSensorMode;
    featureParam.setVar<MINT32>(SFP_VAR::EIS_SENSOR_MODE, sensorMode);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingProcessor::getEISExpTime(MINT32 &expTime, MINT32 &longExpTime, const LMVInfo &lmvInfo, const sp<P2Meta> &inHal) const
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

MVOID StreamingProcessor::setLMVOParam(FeaturePipeParam &featureParam, const sp<P2Request> &request) const
{
    TRACE_FUNC_ENTER();
    sp<P2Meta> inHal = request->mMeta[IN_P1_HAL];
    if( isValid(inHal) )
    {
        IMetadata::IEntry lmvoEntry = inHal->getEntry(MTK_EIS_LMV_DATA);
        if( lmvoEntry.count() )
        {
            IMetadata::Memory lmvoMem = lmvoEntry.itemAt(0, Type2Type<IMetadata::Memory>());
            EIS_STATISTIC_STRUCT *lmvoStat = (EIS_STATISTIC_STRUCT*)lmvoMem.array();
            featureParam.setVar<EIS_STATISTIC_STRUCT>(SFP_VAR::EIS_LMV_DATA, *lmvoStat);
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID StreamingProcessor::calcEISRatio(const MSize &streamingSize, const std::vector<P2AppStreamInfo> &streamInfo)
{
    float widthRatio, heightRatio;
    MUINT32 eisFactor = mPipeUsageHint.mEISInfo.factor;
    MSize outputSize = streamingSize;

    for( const P2AppStreamInfo &info : streamInfo )
    {
        if (info.mImageType == IMG_TYPE_RECORD)
        {
            outputSize = info.mImageSize;
            break;
        }
        if (info.mImageType == IMG_TYPE_DISPLAY)
        {
            outputSize = info.mImageSize;
        }
    }

    getEISRatio(widthRatio, heightRatio, streamingSize, outputSize, eisFactor);

    mEisWidthRatio = widthRatio;
    mEisHeightRatio = heightRatio;
}

MBOOL StreamingProcessor::prepareEISMask(FeaturePipeParam &featureParam) const
{
    TRACE_FUNC_ENTER();
    MINT32 eisMode = mPipeUsageHint.mEISInfo.mode;

    if( EIS_MODE_IS_EIS_30_ENABLED(eisMode) )
    {
        featureParam.setFeatureMask(MASK_EIS, MTRUE);
        featureParam.setFeatureMask(MASK_EIS_30, MTRUE);
    }
    else if( EIS_MODE_IS_EIS_22_ENABLED(eisMode) )
    {
        featureParam.setFeatureMask(MASK_EIS, MTRUE);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL StreamingProcessor::prepareEIS(P2Util::SimpleIn& input, const ILog &log) const
{
    TRACE_S_FUNC_ENTER(log);

    const sp<P2Request> &request = input.mRequest;
    const sp<Cropper> cropper = request->getCropper();
    FeaturePipeParam &featureParam = input.mFeatureParam;

    if( isAdvEIS() )
    {
        MRectF eisMargin = getEISRRZOMargin(input);
        TRACE_S_FUNC(log, "eisMargin=(%f,%f)(%fx%f)", eisMargin.p.x, eisMargin.p.y, eisMargin.s.w, eisMargin.s.h);

        if( (cropper->isEISAppOn() && (request->mP2Pack.getFrameData().mIsRecording)) ||
             mPipeUsageHint.mEISInfo.previewEIS )
        {
            prepareEISVar(input, featureParam, request, cropper, eisMargin);
            prepareEISMask(featureParam);
            setLMVOParam(featureParam, request);
        }
    }

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MINT64 StreamingProcessor::getTSQ(const sp<Payload> &payload, const FeaturePipeParam &/*param*/)
{
    TRACE_S_FUNC_ENTER(payload->mLog);
    MINT64 ts = 0;
    for(const auto& pp : payload->mPartialPayloads)
    {
        MINT64 t = pp->mRequestPack->getVRTimestamp();
        if(t != 0)
        {
            ts = t;
            break;
        }
    }
    TRACE_S_FUNC_EXIT(payload->mLog, "ts(%" PRId64 ")",ts);
    return ts;
}

MVOID StreamingProcessor::processTSQ(const sp<P2Request> &request, MINT64 ts)
{
    TRACE_S_FUNC_ENTER(request->mLog);
    sp<P2Meta> inHal = request->mMeta[IN_P1_HAL];
    sp<P2Meta> appOut = request->mMeta[OUT_APP];
    MBOOL updated = MFALSE;
    if( inHal != NULL && appOut != NULL )
    {
        MBOOL needOverride = getMeta<MBOOL>(inHal, MTK_EIS_NEED_OVERRIDE_TIMESTAMP, MFALSE);
        if( needOverride )
        {
            MINT64 timestamp;
            timestamp = ts ? ts : request->mP2Pack.getSensorData().mP1TS;
            IMetadata::IEntry entry(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP);
            entry.push_back(1, Type2Type<MUINT8>());
            entry.push_back(1, Type2Type<MUINT8>());
            appOut->setEntry(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP, entry);
            appOut->trySet<MINT64>(MTK_SENSOR_TIMESTAMP, timestamp);
            appOut->trySet<MINT64>(MTK_EIS_FEATURE_NEW_SHUTTER_TIMESTAMP, timestamp);
            updated = MTRUE;
        }
    }
    TRACE_S_FUNC(request->mLog, "update=%d tsq=%" PRId64 " p1TS=%" PRId64, updated, ts, request->mP2Pack.getSensorData().mP1TS);
    TRACE_S_FUNC_EXIT(request->mLog);
}

}; // namespace P2
