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

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

#define TRACE_S_CROP(log, name, p, s) \
    TRACE_S_FUNC(log, name " = (%dx%d)@(%d,%d)", s.w, s.h, p.x, p.y)
#define TRACE_S_CROPF(log, name, p, s) \
        TRACE_S_FUNC(log, name " = (%fx%f)@(%f,%f)", s.w, s.h, p.x, p.y)

#define USE_SENSOR_DOMAIN_VIEW_ANGLE 1

namespace P2 {

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

MBOOL isValidCrop(const MRect &crop)
{
    return (crop.s.w > 0 && crop.s.h > 0);
}

P2Cropper::P2Cropper()
    : mEISFactor(EISCustom::getEIS12Factor())
{
}

P2Cropper::P2Cropper(const ILog &log, const P2SensorInfo *sensorInfo, const P2SensorData *sensorData, const LMVInfo &lmvInfo)
    : mLog(log)
    , mSensorID(sensorInfo ? sensorInfo->mSensorID : INVALID_SENSOR_ID)
    , mLMVInfo(lmvInfo)
    , mEISFactor(EISCustom::getEIS12Factor())
{
    TRACE_S_FUNC_ENTER_2(mLog);
    if( sensorInfo && sensorData )
    {
        mActiveSize = sensorInfo->mActiveArray.s;
        mIsValid = initAppInfo(sensorData) &&
                   initHalInfo(sensorData) &&
                   initTransform();
        initLMV();
        if( TRACE_CROPPER || mLog.getLogLevel() >= 1 || needCropperDump() )
        {
            this->dump(mLog);
        }
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

P2Cropper::~P2Cropper()
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
}

MBOOL P2Cropper::isValid() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mIsValid;
}

MBOOL P2Cropper::isEISAppOn() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mIsEISAppOn;
}

MSize P2Cropper::getSensorSize() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mSensorSize;
}

MRect P2Cropper::getResizedCrop() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mResizedCrop;
}

MRect P2Cropper::getP1Crop() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mP1Crop;
}

MSize P2Cropper::getP1OutSize() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mP1OutSize;
}

MRect P2Cropper::getP1BinCrop() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mP1BinCrop;
}

MSize P2Cropper::getP1BinSize() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mP1BinSize;
}

const LMVInfo& P2Cropper::getLMVInfo() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    TRACE_S_FUNC_EXIT_2(mLog);
    return mLMVInfo;
}

MCropRect P2Cropper::calcViewAngle(const ILog &log, const MSize &size, MUINT32 cropFlag) const
{
    return calcViewAngle(log, size, cropFlag, 1.0f);
}

MCropRect P2Cropper::calcViewAngle(const ILog &log, const MSize &size, MUINT32 cropFlag, const float cropRatio) const
{
    MRectF retRectF = calcViewAngleF(log, size, cropFlag, cropRatio, cropRatio, (DMAConstrain::DEFAULT));
    MCropRect cropRect = getCropRect(retRectF);
    MBOOL useResize = !!(cropFlag & Cropper::USE_RESIZED);
    if( refineBoundary(log, useResize ? mP1OutSize : mSensorSize, cropRect) )
    {
        TRACE_S_CROP(log, "with refine", cropRect.p_integral, cropRect.s);
        this->dump(log);
    }
    return cropRect;
}

MRectF P2Cropper::calcViewAngleF(const ILog &log, const MSize &size, MUINT32 cropFlag,
    const float widthCropRatio, const float heightCropRatio, const MUINT32 dmaConstrain) const
{
    TRACE_S_FUNC_ENTER_2(log);
    MBOOL useResize = !!(cropFlag & Cropper::USE_RESIZED);
    MBOOL useEIS12 = !!(cropFlag & Cropper::USE_EIS_12);
    MBOOL useCropRatio = !!(cropFlag & Cropper::USE_CROP_RATIO);
    MBOOL useRrzoDomainView = useResize && !USE_SENSOR_DOMAIN_VIEW_ANGLE;
    MBOOL needSensorToRrzo = useResize && !useRrzoDomainView;
    TRACE_S_FUNC(log, "size=%dx%d flag=0x%x useResize=%d useEIS12=%d useCropRatio=%d(%f,%f), dmaConstrain(%d), validCropper=%d, useRrzoDomainView=%d",
        size.w, size.h, cropFlag, useResize, useEIS12, useCropRatio, widthCropRatio, heightCropRatio, dmaConstrain, mIsValid, useRrzoDomainView);

    MRectF view(size.w, size.h);
    if( mIsValid )
    {
        TRACE_S_CROP(log, "activeCrop", mActiveCrop.p, mActiveCrop.s);
        TRACE_S_CROP(log, "sensorCrop", mSensorCrop.p, mSensorCrop.s);
        TRACE_S_CROP(log, "resizedCrop", mResizedCrop.p, mResizedCrop.s);

        view = useRrzoDomainView ? mResizedCrop : mSensorCrop;
        TRACE_S_CROPF(log, "original", view.p, view.s);
        if( useEIS12 && mIsEISAppOn )
        {
            view = applyEIS12(log, view, useRrzoDomainView);
        }
        else if( useCropRatio )
        {
            view = applyCropRatio(log, view, widthCropRatio, heightCropRatio);
        }
        view = applyViewRatio(log, view, size);
        if( needSensorToRrzo )
        {
            MRectF sensorDomainView = view;
            view = transform(mSensor2Resized, sensorDomainView);
            TRACE_S_CROPF(log, "with sensor2Resized", view.p, view.s);
        }
        MSizeF boundary = useResize ? mP1OutSize : mSensorSize;
        if( refineBoundaryF(log, boundary, view) )
        {
            this->dump(log);
        }
        if( (dmaConstrain & DMAConstrain::NOSUBPIXEL) ||
            (dmaConstrain & DMAConstrain::ALIGN2BYTE) )
        {
            view.p.x += 0.5;
            view.p.y += 0.5;
            view.p = MPoint(view.p.x, view.p.y);
            MSize viewSizeI(view.s.w, view.s.h);
            if( dmaConstrain & DMAConstrain::ALIGN2BYTE )
            {
                viewSizeI.w &= ~(0x01);
                viewSizeI.h &= ~(0x01);
            }
            view.s = viewSizeI;
        }

        TRACE_S_CROPF(log, "result", view.p, view.s);
    }
    TRACE_S_FUNC_EXIT_2(log);
    return view;
}

MBOOL P2Cropper::refineBoundaryF(const ILog &log, const MSizeF &size, MRectF &crop) const
{
    MBOOL isRefined = MFALSE;
    if( mIsValid )
    {
        MRectF refined = crop;
        if( crop.p.x < 0 )
        {
            refined.p.x = 0;
            isRefined = MTRUE;
        }
        if( crop.p.y < 0 )
        {
            refined.p.y = 0;
            isRefined = MTRUE;
        }
        if( (refined.p.x + crop.s.w) > size.w )
        {
            refined.s.w = size.w - refined.p.x;
            isRefined = MTRUE;
        }
        if( (refined.p.y + crop.s.h) > size.h )
        {
            refined.s.h = size.h - refined.p.y;
            isRefined = MTRUE;
        }
        if( isRefined )
        {
            MY_S_LOGW(log, "apply refine: boundary(%.0fx%.0f), crop:(%f,%f,%f,%f) -> crop:(%f,%f,%f,%f)",
                size.w, size.h,
                crop.p.x, crop.p.y, crop.s.w, crop.s.h,
                refined.p.x, refined.p.y, refined.s.w, refined.s.h);
            crop = refined;
        }
    }
    return isRefined;
}

MBOOL P2Cropper::refineBoundary(const ILog &log, const MSize &size, MCropRect &crop) const
{
    TRACE_S_FUNC_ENTER_2(log);
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
        int carry_w = (crop.w_fractional != 0) ? 1 : 0;
        int carry_h = (crop.h_fractional != 0) ? 1 : 0;
        if( (refined.p_integral.x + crop.s.w + carry_x + carry_w) > size.w )
        {
            refined.s.w = size.w - carry_w - refined.p_integral.x - carry_x;
            isRefined = MTRUE;
        }
        if( (refined.p_integral.y + crop.s.h + carry_y + carry_h) > size.h )
        {
            refined.s.h = size.h - carry_h - refined.p_integral.y - carry_y;
            isRefined = MTRUE;
        }
        if( isRefined )
        {
            refined.s.w &= ~(0x01);
            refined.s.h &= ~(0x01);

            MY_S_LOGW(log, "size:(%dx%d), crop:(%d.%d,%d.%d)(%dx%d) -> crop:(%d.%d,%d.%d)(%dx%d)",
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
    TRACE_S_FUNC_EXIT_2(log);
    return isRefined;
}

MRect P2Cropper::getCropRegion() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    MRect cropRegion;
    if( mIsValid )
    {
        cropRegion = mActiveCrop;
    }
    TRACE_S_FUNC_EXIT_2(mLog);
    return cropRegion;
}

MRect P2Cropper::getActiveCrop() const
{
    return mActiveCrop;
}

MRect P2Cropper::toActive(const MRectF &cropF, MBOOL resize) const
{
    MRect crop = cropF.toMRect();
    return toActive(crop, resize);
}

MRect P2Cropper::toActive(const MCropRect &cropRect, MBOOL resize) const
{
    MRect crop = MRect(cropRect.p_integral, cropRect.s);
    return toActive(crop, resize);
}

MRect P2Cropper::toActive(const MRect &crop, MBOOL resize) const
{
    MRect s_crop, a_crop;
    if( resize )
    {
        s_crop.p = inv_transform(mSensor2Resized, crop.p);
        s_crop.s = inv_transform(mSensor2Resized, crop.s);
    }
    else
    {
        s_crop.p = crop.p;
        s_crop.s = crop.s;
    }

    mSensor2Active.transform(s_crop, a_crop);
    TRACE_S_CROP(mLog, "ViewCrop", crop.p, crop.s);
    TRACE_S_CROP(mLog, "sensorViewCrop", s_crop.p, s_crop.s);
    TRACE_S_CROP(mLog, "activeViewCrop", a_crop.p, a_crop.s);
    return a_crop;
}

MVOID P2Cropper::dump(const ILog &log) const
{
    TRACE_S_FUNC_ENTER_2(log);
    MY_S_LOGI(log, "sensorID=%d isValid=%d sensorMode=%d",
        mSensorID, mIsValid, mSensorMode);
    MY_S_LOGI(log, "sensorSize(%dx%d)", mSensorSize.w, mSensorSize.h);
    MY_S_LOGI(log, "p1 crop(%dx%d)@(%d,%d) size(%dx%d) dma(%dx%d)@(%d,%d)",
        mP1Crop.s.w, mP1Crop.s.h, mP1Crop.p.x, mP1Crop.p.y,
        mP1OutSize.w, mP1OutSize.h,
        mP1DMA.s.w, mP1DMA.s.h, mP1DMA.p.x, mP1DMA.p.y);
    MY_S_LOGI(log, "sensor to resized (%d,%d) size(%dx%d)->(%dx%d)",
        mSensor2Resized.tarOrigin.x, mSensor2Resized.tarOrigin.y,
        mSensor2Resized.oldScale.w, mSensor2Resized.oldScale.h,
        mSensor2Resized.newScale.w, mSensor2Resized.newScale.h);
    MY_S_LOGI(log, "Active crop (%dx%d)@(%d,%d), AppCrop(%dx%d)@(%d,%d), SimulateAppCrop(%dx%d)@(%d,%d)",
        mActiveCrop.s.w, mActiveCrop.s.h,
        mActiveCrop.p.x, mActiveCrop.p.y,
        mAppCrop.s.w, mAppCrop.s.h,
        mAppCrop.p.x, mAppCrop.p.y,
        mSimulatedAppCrop.s.w, mSimulatedAppCrop.s.h,
        mSimulatedAppCrop.p.x, mSimulatedAppCrop.p.y);
    MY_S_LOGI(log, "IsEISAppOn=%d",
        mIsEISAppOn);
    MY_S_LOGI(log, "active mv (%d,%d)(%d,%d)",
        mActiveLMV.p.x, mActiveLMV.pf.x,
        mActiveLMV.p.y, mActiveLMV.pf.y);
    MY_S_LOGI(log, "sensor mv (%d,%d)(%d,%d)",
        mSensorLMV.p.x, mSensorLMV.pf.x,
        mSensorLMV.p.y, mSensorLMV.pf.y);
    MY_S_LOGI(log, "resized mv (%d,%d)(%d,%d)",
        mResizedLMV.p.x, mResizedLMV.pf.x,
        mResizedLMV.p.y, mResizedLMV.pf.y);
    TRACE_S_FUNC_EXIT_2(log);
}

MBOOL P2Cropper::needCropperDump()
{
    static MBOOL sEnableDump = (property_get_int32(KEY_CROPPER_DUMP,0) > 0);
    return sEnableDump;
}

MBOOL P2Cropper::initAppInfo(const P2SensorData *data)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    MBOOL ret = MFALSE;
    if( data )
    {
        mIsEISAppOn = data->mAppEISOn;
        mAppCrop = data->mAppCrop;
        mSimulatedAppCrop = data->mSimulatedAppCrop;
        mActiveCrop = isValidCrop(mSimulatedAppCrop) ? mSimulatedAppCrop : mAppCrop;
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT_2(mLog);
    return ret;
}

MBOOL P2Cropper::initHalInfo(const P2SensorData *data)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    MBOOL ret = MFALSE;
    if( data)
    {
        mSensorMode = data->mSensorMode;
        mSensorSize = data->mSensorSize;
        mP1Crop = data->mP1Crop;
        mP1DMA = data->mP1DMA;
        mP1OutSize = data->mP1OutSize;
        mP1BinCrop = data->mP1BinCrop;
        mP1BinSize = data->mP1BinSize;
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT_2(mLog);
    return ret;
}

MBOOL P2Cropper::initTransform()
{
    TRACE_S_FUNC_ENTER_2(mLog);
    MBOOL ret = MTRUE;
    HwTransHelper trans(mSensorID);
    if( !trans.getMatrixToActive(mSensorMode, mSensor2Active) ||
        !trans.getMatrixFromActive(mSensorMode, mActive2Sensor) )
    {
        MY_S_LOGW(mLog, "cannot get active matrix");
        ret = MFALSE;
    }
    else
    {
        mSensor2Resized = simpleTransform(mP1Crop.p, mP1Crop.s, mP1OutSize);
        mActive2Sensor.transform(mActiveCrop, mSensorCrop);
        mSensorCrop = clip(mSensorCrop, mP1Crop);
        mResizedCrop = transform(mSensor2Resized, mSensorCrop);
    }
    TRACE_S_FUNC_EXIT_2(mLog);
    return ret;
}

MBOOL P2Cropper::initLMV()
{
    TRACE_S_FUNC_ENTER_2(mLog);
    MBOOL ret = MTRUE;
    prepareLMV();
    TRACE_S_FUNC_EXIT_2(mLog);
    return ret;
}

MVOID P2Cropper::prepareLMV()
{
    TRACE_S_FUNC_ENTER_2(mLog);
#if SUPPORT_LMV_MV_API
    if( mLMVInfo.is_from_zzr )
    {
        mResizedLMV.p.x = mLMVInfo.x_mv_int;
        mResizedLMV.pf.x = mLMVInfo.x_mv_float;
        mResizedLMV.p.y = mLMVInfo.y_mv_int;
        mResizedLMV.pf.y = mLMVInfo.y_mv_float;
        mSensorLMV = inv_transform(mSensor2Resized, mResizedLMV);
    }
    else
    {
        mSensorLMV.p.x = mLMVInfo.x_mv_int;
        mSensorLMV.pf.x = mLMVInfo.x_mv_int;
        mSensorLMV.p.y = mLMVInfo.y_mv_int;
        mSensorLMV.pf.y = mLMVInfo.y_mv_float;
        mResizedLMV = transform(mSensor2Resized, mSensorLMV);
    }
#else
    mResizedLMV.p.x = mLMVInfo.x_int;
    mResizedLMV.pf.x = mLMVInfo.x_float;
    mResizedLMV.p.y = mLMVInfo.y_int;
    mResizedLMV.pf.y = mLMVInfo.y_float;
    mSensorLMV = inv_transform(mSensor2Resized, mResizedLMV);
#endif

    mSensor2Active.transform(mSensorLMV.p, mActiveLMV.p);
    TRACE_S_FUNC_EXIT_2(mLog);
}

MRect P2Cropper::clip(const MRect &src, const MRect &box) const
{
    TRACE_S_FUNC_ENTER_2(mLog);
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
    TRACE_S_FUNC_EXIT_2(mLog);
    return result;
}

MRectF P2Cropper::applyEIS12(const ILog &log, const MRectF &src, MBOOL useResize) const
{
    #define LMV_FRACTION_MAX (0x100000000)
    MRectF view = src;
    MSize viewSizeI(view.s.w, view.s.h);

    view.s = viewSizeI * 100 / mEISFactor;

    vector_f lmvVector = useResize ? mResizedLMV : mSensorLMV;
    MPointF lmvPointF(lmvVector.pf.x, lmvVector.pf.y);
    view.p += lmvVector.p;
    view.p.x += (lmvPointF.x/LMV_FRACTION_MAX);
    view.p.y += (lmvPointF.y/LMV_FRACTION_MAX);
    TRACE_S_FUNC(log, " lmv vector xy(%d,%d) subpixel(%d,%d) = (%f,%f)",
        lmvVector.p.x, lmvVector.p.y, lmvVector.pf.x, lmvVector.pf.y, lmvPointF.x/LMV_FRACTION_MAX, lmvPointF.y/LMV_FRACTION_MAX);

    if( !mLMVInfo.is_valid )
    {
        MY_S_LOGD(log, "invalid LMVInfo, use latest result = lmv vector xy(%d,%d) subpixel(%d,%d) = (%f,%f)",
            lmvVector.p.x, lmvVector.p.y, lmvVector.pf.x, lmvVector.pf.y, lmvPointF.x/LMV_FRACTION_MAX, lmvPointF.y/LMV_FRACTION_MAX);
    }
    TRACE_S_CROPF(log, "applyEIS12: src", src.p, src.s);
    TRACE_S_CROPF(log, "with EIS1.2", view.p, view.s);
    return view;
}

MRectF P2Cropper::applyCropRatio(const ILog &log, const MRectF &src, const float widthRatio, const float heightRatio) const
{
    MRectF view = src;
    if( widthRatio > 1.0f || heightRatio > 1.0f )
    {
        MY_S_LOGW(log, "skip invalid ratio(%f,%f) for view(%fx%f)", widthRatio, heightRatio, src.s.w, src.s.h);
    }
    else
    {
        view.p.x += (view.s.w * (1.0f - widthRatio) / 2);
        view.p.y += (view.s.h * (1.0f - heightRatio) / 2);
        view.s.w    = (view.s.w * widthRatio);
        view.s.h    = (view.s.h * heightRatio);
    }
    TRACE_S_FUNC(log, "applyCropRatio: src(%fx%f) ratio(%f,%f)", src.s.w, src.s.h, widthRatio, heightRatio);
    TRACE_S_CROPF(log, "with crop ratio", view.p, view.s);
    return view;
}

MRectF P2Cropper::applyViewRatio(const ILog &log, const MRectF &src, const MSize &size) const
{
    MRectF view = src;
    if( src.s.w * size.h > size.w * src.s.h )
    {
        view.s.w = (src.s.h * size.w / size.h);
        view.p.x += (src.s.w - view.s.w)/2;
    }
    else
    {
        view.s.h = (src.s.w * size.h / size.w);
        view.p.y += (src.s.h - view.s.h)/2;
    }
    TRACE_S_CROPF(log, "with aspect ratio", view.p, view.s);
    return view;
}

} // namespace P2
