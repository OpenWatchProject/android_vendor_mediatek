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

#define SUPPORT_LMV_MV_API 0

#include "P2_Cropper.h"
#include "P2_Common.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Cropper
#define P2_TRACE        TRACE_CROPPER
#include "P2_LogHeader.h"

#define TRACE_S_CROP(logger, name, p, s) \
    TRACE_S_FUNC(logger, name " = (%dx%d)@(%d,%d)", s.w, s.h, p.x, p.y)

#define USE_SENSOR_DOMAIN_VIEW_ANGLE 1

namespace P2
{

Cropper::Cropper()
    : mIsValid(MFALSE)
    , mSensorMode(0)
    , mIsEISAppOn(MFALSE)
    , mEISFactor(EISCustom::getEIS12Factor())
{
}

Cropper::Cropper(const sp<P2Info> &p2Info, const LMVInfo &lmvInfo, const IMetadata *inApp, const IMetadata *inHal, const Logger &logger)
    : mP2Info(p2Info)
    , mLMVInfo(lmvInfo)
    , mLogger(logger)
    , mIsValid(MFALSE)
    , mSensorMode(0)
    , mIsEISAppOn(MFALSE)
    , mEISFactor(EISCustom::getEIS12Factor())
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( p2Info != NULL && inApp && inHal )
    {
        mActiveSize = p2Info->mActiveArray.s;
        mIsValid = initAppInfo(inApp) &&
                   initHalInfo(inHal) &&
                   initTransform();
        initLMV();
        if( TRACE_CROPPER || mLogger->getLogLevel() >= 1 )
        {
            this->dump(mLogger);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

Cropper::~Cropper()
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
}

MBOOL Cropper::isValid() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mIsValid;
}

MBOOL Cropper::isEISAppOn() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mIsEISAppOn;
}

MSize Cropper::getSensorSize() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mSensorSize;
}

MRect Cropper::getResizedCrop() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mResizedCrop;
}

MRect Cropper::getP1Crop() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mP1Crop;
}

MSize Cropper::getP1OutSize() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mP1OutSize;
}

const LMVInfo& Cropper::getLMVInfo() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mLMVInfo;
}

MCropRect Cropper::calcViewAngle(const MSize &size, MUINT32 cropFlag) const
{
    return calcViewAngle(size, cropFlag, MSize(0,0), mLogger);
}

MCropRect Cropper::calcViewAngle(const MSize &size, MUINT32 cropFlag, const MSize &margin) const
{
    return calcViewAngle(size, cropFlag, margin, mLogger);
}

MCropRect Cropper::calcViewAngle(const MSize &size, MUINT32 cropFlag, const Logger &logger) const
{
    return calcViewAngle(size, cropFlag, MSize(0,0), logger);
}

MCropRect transform(const simpleTransform &trans, const MCropRect &src)
{
    MCropRect dst;
    vector_f fracOffset;
    fracOffset = transform(trans, vector_f(src.p_integral, src.p_fractional));
    dst.p_integral = fracOffset.p + transform(trans, MPoint(0,0));
    dst.p_fractional = fracOffset.pf;
    dst.s = transform(trans, src.s);
    TRACE_FUNC("src(%dx%d)=>dst(%dx%d)", src.s.w, src.s.h, dst.s.w, dst.s.h);
    return dst;
}

MCropRect Cropper::calcViewAngle(const MSize &size, MUINT32 cropFlag, const MSize &margin, const Logger &logger) const
{
    TRACE_S_FUNC_ENTER(logger);
    MBOOL useResize = !!(cropFlag & Cropper::USE_RESIZED);
    MBOOL useEIS12 = !!(cropFlag & Cropper::USE_EIS_12);
    MBOOL useMargin = !!(cropFlag & Cropper::USE_MARGIN);
    MBOOL useRrzoDomainView = useResize && !USE_SENSOR_DOMAIN_VIEW_ANGLE;
    MBOOL needSensorToRrzo = useResize && !useRrzoDomainView;
    TRACE_S_FUNC(logger, "size=%dx%d flag=0x%x useResize=%d useEIS12=%d useMargin=%d(%dx%d), validCropper=%d, useRrzoDomainView=%d", size.w, size.h, cropFlag, useResize, useEIS12, useMargin, margin.w, margin.h, mIsValid, useRrzoDomainView);

    MCropRect view(MPoint(0, 0), size);
    if( mIsValid )
    {
        TRACE_S_CROP(logger, "activeCrop", mActiveCrop.p, mActiveCrop.s);
        TRACE_S_CROP(logger, "sensorCrop", mSensorCrop.p, mSensorCrop.s);
        TRACE_S_CROP(logger, "resizedCrop", mResizedCrop.p, mResizedCrop.s);

        view = useRrzoDomainView ? mResizedCrop : mSensorCrop;
        TRACE_S_CROP(logger, "original", view.p_integral, view.s);
        if( useEIS12 && mIsEISAppOn )
        {
            view = applyEIS12(logger, view, useRrzoDomainView);
            TRACE_S_CROP(logger, "with lmv", view.p_integral, view.s);
        }
        else if( useMargin )
        {
            double ratioW = (double)margin.w / mP1OutSize.w;
            double ratioH = (double)margin.h / mP1OutSize.h;
            view = applyMargin(logger, view, ratioW, ratioH);
            TRACE_S_FUNC(logger, "margin: rrzoMargin(%dx%d)=ratio(%fx%f)", margin.w, margin.h, ratioW, ratioH);
            TRACE_S_CROP(logger, "with margin", view.p_integral, view.s);
        }
        view = applyViewRatio(logger, view, size);
        TRACE_S_CROP(logger, "with ratio", view.p_integral, view.s);
        if( needSensorToRrzo )
        {
            MCropRect sensorDomainView = view;
            view = transform(mSensor2Resized, sensorDomainView);
            TRACE_S_CROP(logger, "with sensor2Resized", view.p_integral, view.s);
        }
        view.s.w &= ~(0x01);
        view.s.h &= ~(0x01);
        if( refineBoundary(useResize ? mP1OutSize : mSensorSize, view) )
        {
            TRACE_S_CROP(logger, "with refine", view.p_integral, view.s);
            this->dump(logger);
        }
        TRACE_S_CROP(logger, "result", view.p_integral, view.s);
    }
    TRACE_S_FUNC_EXIT(logger);
    return view;
}

MBOOL Cropper::refineBoundary(const MSize &size, MCropRect &crop) const
{
    return refineBoundary(size, crop, mLogger);
}

MBOOL Cropper::refineBoundary(const MSize &size, MCropRect &crop, const Logger &logger) const
{
    TRACE_S_FUNC_ENTER(logger);
    MBOOL isRefined = MFALSE;
    if( mIsValid )
    {
        MCropRect refined = crop;
        if( crop.p_integral.x < 0 )
        {
            refined.p_integral.x = 0;
            isRefined = MTRUE;
        }
        if( crop.p_integral.y < 0 )
        {
            refined.p_integral.y = 0;
            isRefined = MTRUE;
        }
        int carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
        int carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
        if( (refined.p_integral.x + crop.s.w + carry_x) > size.w )
        {
            refined.s.w = size.w - refined.p_integral.x - carry_x;
            isRefined = MTRUE;
        }
        if( (refined.p_integral.y + crop.s.h + carry_y) > size.h )
        {
            refined.s.h = size.h - refined.p_integral.y - carry_y;
            isRefined = MTRUE;
        }
        if( isRefined )
        {
            refined.s.w &= ~(0x01);
            refined.s.h &= ~(0x01);

            MY_S_LOGW(logger, "size:(%dx%d), crop:(%d.%d,%d.%d)(%dx%d) -> crop:(%d.%d,%d.%d)(%dx%d)",
                size.w, size.h,
                crop.p_integral.x, crop.p_fractional.x,
                crop.p_integral.y, crop.p_fractional.y,
                crop.s.w, crop.s.h,
                refined.p_integral.x, refined.p_fractional.x,
                refined.p_integral.y, refined.p_fractional.y,
                refined.s.w, refined.s.h);
            crop = refined;
        }
    }
    TRACE_S_FUNC_EXIT(logger);
    return isRefined;
}

MRect Cropper::getCropRegion() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MRect cropRegion;
    if( mIsValid )
    {
        cropRegion = mActiveCrop;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return cropRegion;
}

MRect Cropper::getActiveCrop() const
{
    return mActiveCrop;
}

MRect Cropper::toActive(const MCropRect &crop, MBOOL resize) const
{
    MRect s_crop, a_crop;
    if( resize )
    {
        s_crop.p = inv_transform(mSensor2Resized, crop.p_integral);
        s_crop.s = inv_transform(mSensor2Resized, crop.s);
    }
    else
    {
        s_crop.p = crop.p_integral;
        s_crop.s = crop.s;
    }

    mSensor2Active.transform(s_crop, a_crop);
    TRACE_S_CROP(mLogger, "ViewCrop", crop.p_fractional, crop.s);
    TRACE_S_CROP(mLogger, "sensorViewCrop", s_crop.p, s_crop.s);
    TRACE_S_CROP(mLogger, "activeViewCrop", a_crop.p, a_crop.s);
    return a_crop;
}

MVOID Cropper::dump() const
{
    dump(mLogger);
}

MVOID Cropper::dump(const Logger &logger) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MUINT32 sensorID = INVALID_SENSOR_ID;
    if( mP2Info != NULL )
    {
        sensorID = mP2Info->mSensorID;
    }
    MY_S_LOGD(logger, "sensorID=%d isValid=%d sensorMode=%d",
        sensorID, mIsValid, mSensorMode);
    MY_S_LOGD(logger, "sensorSize(%dx%d)", mSensorSize.w, mSensorSize.h);
    MY_S_LOGD(logger, "p1 crop(%dx%d)@(%d,%d) size(%dx%d) dma(%dx%d)@(%d,%d)",
        mP1Crop.s.w, mP1Crop.s.h, mP1Crop.p.x, mP1Crop.p.y,
        mP1OutSize.w, mP1OutSize.h,
        mP1DMA.s.w, mP1DMA.s.h, mP1DMA.p.x, mP1DMA.p.y);
    MY_S_LOGD(logger, "sensor to resized (%d,%d) size(%dx%d)->(%dx%d)",
        mSensor2Resized.tarOrigin.x, mSensor2Resized.tarOrigin.y,
        mSensor2Resized.oldScale.w, mSensor2Resized.oldScale.h,
        mSensor2Resized.newScale.w, mSensor2Resized.newScale.h);
    MY_S_LOGD(logger, "Active crop (%dx%d)@(%d,%d)",
        mActiveCrop.s.w, mActiveCrop.s.h,
        mActiveCrop.p.x, mActiveCrop.p.y);
    MY_S_LOGD(logger, "IsEISAppOn=%d",
        mIsEISAppOn);
    MY_S_LOGD(logger, "active mv (%d,%d)(%d,%d)",
        mActiveLMV.p.x, mActiveLMV.pf.x,
        mActiveLMV.p.y, mActiveLMV.pf.y);
    MY_S_LOGD(logger, "sensor mv (%d,%d)(%d,%d)",
        mSensorLMV.p.x, mSensorLMV.pf.x,
        mSensorLMV.p.y, mSensorLMV.pf.y);
    MY_S_LOGD(logger, "resized mv (%d,%d)(%d,%d)",
        mResizedLMV.p.x, mResizedLMV.pf.x,
        mResizedLMV.p.y, mResizedLMV.pf.y);
    TRACE_S_FUNC_EXIT(mLogger);
}

MBOOL Cropper::initAppInfo(const IMetadata *inApp)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    if( inApp != NULL )
    {
        mIsEISAppOn = queryEISAppOn(inApp);
        mActiveCrop = queryCropRegion(inApp, mIsEISAppOn);
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL Cropper::initHalInfo(const IMetadata *inHal)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    if( inHal != NULL )
    {
        if( !tryGet<MINT32>(inHal, MTK_P1NODE_SENSOR_MODE, mSensorMode) )
        {
            MY_S_LOGW(mLogger, "cannot get MTK_P1NODE_SENSOR_MODE");
        }
        else if( !tryGet<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSize) )
        {
            MY_S_LOGW(mLogger, "cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        }
        else
        {
            if( !tryGet<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, mP1Crop) ||
                !tryGet<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION, mP1DMA) ||
                !tryGet<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE, mP1OutSize) )
            {
                mP1Crop = MRect(MPoint(0,0), mSensorSize);
                mP1DMA = MRect(MPoint(0,0), mSensorSize);
                mP1OutSize = mSensorSize;
            }
            ret = MTRUE;
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL Cropper::initTransform()
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MTRUE;
    HwTransHelper trans(mP2Info->mSensorID);
    if( !trans.getMatrixToActive(mSensorMode, mSensor2Active) ||
        !trans.getMatrixFromActive(mSensorMode, mActive2Sensor) )
    {
        MY_S_LOGW(mLogger, "cannot get active matrix");
        ret = MFALSE;
    }
    else
    {
        mSensor2Resized = simpleTransform(mP1Crop.p, mP1Crop.s, mP1OutSize);
        mActive2Sensor.transform(mActiveCrop, mSensorCrop);
        mSensorCrop = clip(mSensorCrop, mP1Crop);
        mResizedCrop = transform(mSensor2Resized, mSensorCrop);
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL Cropper::initLMV()
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    if( mLMVInfo.is_valid )
    {
        prepareLMV();
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL Cropper::queryEISAppOn(const IMetadata *inApp)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if( !tryGet<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode) )
    {
        MY_S_LOGW(mLogger, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return (eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON);
}

MRect Cropper::queryCropRegion(const IMetadata *inApp, MBOOL eis)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MRect crop;
    if( !inApp || !tryGet<MRect>(inApp, MTK_SCALER_CROP_REGION, crop) )
    {
        crop = MRect(MPoint(0, 0), mActiveSize);
        MY_S_LOGW(mLogger, "no MTK_SCALER_CROP_REGION, use full size (%dx%d)", mActiveSize.w, mActiveSize.h);
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return crop;
}

MVOID Cropper::prepareLMV()
{
    TRACE_S_FUNC_ENTER(mLogger);
#if SUPPORT_LMV_MV_API
    if( mLMVInfo.is_from_zzr )
    {
        mResizedLMV.p.x = mLMVInfo.mv.p.x;
        mResizedLMV.pf.x = mLMVInfo.mv.pf.x;
        mResizedLMV.p.y = mLMVInfo.mv.p.y;
        mResizedLMV.pf.y = mLMVInfo.mv.pf.y;
        mSensorLMV = inv_transform(mSensor2Resized, mResizedLMV);
    }
    else
    {
        mSensorLMV.p.x = mLMVInfo.mv.p.x;
        mSensorLMV.pf.x = mLMVInfo.mv.pf.x;
        mSensorLMV.p.y = mLMVInfo.mv.p.y;
        mSensorLMV.pf.y = mLMVInfo.mv.pf.y;
        mResizedLMV = transform(mSensor2Resized, mSensorLMV);
    }
#else
    mResizedLMV.p.x = mLMVInfo.x_int - (mP1OutSize.w * (mEISFactor-100)/2/mEISFactor);
    mResizedLMV.pf.x = mLMVInfo.x_float;
    mResizedLMV.p.y = mLMVInfo.y_int - (mP1OutSize.h * (mEISFactor-100)/2/mEISFactor);
    mResizedLMV.pf.y = mLMVInfo.y_float;
    mSensorLMV = inv_transform(mSensor2Resized, mResizedLMV);
#endif

    mSensor2Active.transform(mSensorLMV.p, mActiveLMV.p);
    TRACE_S_FUNC_EXIT(mLogger);
}

MRect Cropper::clip(const MRect &src, const MRect &box) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MRect result = src;
    if( src.p.x < box.p.x )
    {
        result.p.x = box.p.x;
    }
    if( src.p.y < box.p.y )
    {
        result.p.y = box.p.y;
    }
    MINT32 maxW = box.p.x + box.s.w - result.p.x;
    if( maxW && (result.s.w > maxW) )
    {
        result.s.w = maxW;
    }
    MINT32 maxH = box.p.y + box.s.h - result.p.y;
    if( maxH && (result.s.h > maxH) )
    {
        result.s.h = maxH;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return result;
}

MCropRect Cropper::applyEIS12(const Logger &logger, const MCropRect &src, MBOOL useResize) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MCropRect view = src;

    view.s = src.s * 100 / mEISFactor;
    if( mLMVInfo.is_valid )
    {
        view.p_integral += useResize ? mResizedLMV.p : mSensorLMV.p;
        view.p_fractional = useResize ? mResizedLMV.pf : mSensorLMV.pf;
    }
    else
    {
        double marginRatio = 0.5 * (mEISFactor-100) / mEISFactor;
        view.p_integral.x += (src.s.w * marginRatio);
        view.p_integral.y += (src.s.h * marginRatio);
        MY_S_LOGD(logger, "invalid LMVInfo, use mid position");
    }
    TRACE_S_CROP(logger, "src", src.p_integral, src.s);
    TRACE_S_CROP(logger, "with eis1.2", view.p_integral, view.s);
    TRACE_S_FUNC_EXIT(mLogger);
    return view;
}

MCropRect Cropper::applyMargin(const Logger &logger, const MCropRect &src, const double &ratioW, const double &ratioH) const
{
    MCropRect view = src;
    MSize margin(std::ceil(src.s.w*ratioW), std::ceil(src.s.h*ratioH));
    if( (margin.w*2) >= src.s.w || (margin.h*2) >= src.s.h )
    {
        MY_S_LOGW(logger, "skip invalid margin(%dx%d) for view(%dx%d)", margin.w, margin.h, src.s.w, src.s.h);
    }
    else
    {
        view.p_integral.x += margin.w;
        view.p_integral.y += margin.h;
        view.s.w -= 2*margin.w;
        view.s.h -= 2*margin.h;
    }
    TRACE_S_CROP(logger, "src", src.p_integral, src.s);
    TRACE_S_CROP(logger, "with margin", view.p_integral, view.s);
    return view;
}

MCropRect Cropper::applyViewRatio(const Logger &logger, const MCropRect &src, const MSize &size) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    MCropRect view = src;
    if( src.s.w * size.h > size.w * src.s.h )
    {
        view.s.w = div_round(size.w * src.s.h, size.h);
        view.p_integral.x += (src.s.w - view.s.w)/2;
    }
    else
    {
        view.s.h = div_round(src.s.w * size.h, size.w);
        view.p_integral.y += (src.s.h - view.s.h)/2;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return view;
}

} // namespace P2
