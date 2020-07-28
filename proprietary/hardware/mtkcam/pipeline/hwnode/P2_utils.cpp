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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "MtkCam/P2_utils"
//
#include <mtkcam/utils/std/Log.h>

#include "P2_utils.h"

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//
#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

inline MCropRect getCropRect(const MRectF &rectF) {
    #define MAX_MDP_FRACTION_BIT (20) // MDP use 20bits
    MCropRect cropRect(rectF.p.toMPoint(), rectF.s.toMSize());
    cropRect.p_fractional.x = (rectF.p.x-cropRect.p_integral.x) * (1<<MAX_MDP_FRACTION_BIT);
    cropRect.p_fractional.y = (rectF.p.y-cropRect.p_integral.y) * (1<<MAX_MDP_FRACTION_BIT);
    cropRect.w_fractional = (rectF.s.w-cropRect.s.w) * (1<<MAX_MDP_FRACTION_BIT);
    cropRect.h_fractional = (rectF.s.h-cropRect.s.h) * (1<<MAX_MDP_FRACTION_BIT);
    return cropRect;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
calcViewAngle(
    MBOOL bEnableLog,
    crop_info_t const& cropInfos,
    MSize const& dstSize,
    MCropRect& result
)
{
    MBOOL const isResized = cropInfos.isResized;
    //coordinates: s_: sensor
    // MRect s_crop = transform(cropInfos.tranActive2Sensor, cropInfos.crop_a);
    MRect s_crop;
    cropInfos.matActive2Sensor.transform(cropInfos.crop_a, s_crop);

    MRect s_viewcrop;
    //
    if( s_crop.s.w * dstSize.h > s_crop.s.h * dstSize.w ) { // pillarbox
        s_viewcrop.s.w = div_round(s_crop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = s_crop.s.h;
        s_viewcrop.p.x = s_crop.p.x + ((s_crop.s.w - s_viewcrop.s.w) >> 1);
        s_viewcrop.p.y = s_crop.p.y;
    }
    else { // letterbox
        s_viewcrop.s.w = s_crop.s.w;
        s_viewcrop.s.h = div_round(s_crop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = s_crop.p.x;
        s_viewcrop.p.y = s_crop.p.y + ((s_crop.s.h - s_viewcrop.s.h) >> 1);
    }
    MY_LOGD_IF(bEnableLog, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
            s_crop.p.x     , s_crop.p.y     ,
            s_crop.s.w     , s_crop.s.h     ,
            dstSize.w      , dstSize.h      ,
            s_viewcrop.p.x , s_viewcrop.p.y ,
            s_viewcrop.s.w , s_viewcrop.s.h
           );
    //
    MRect resizedView;
    if( isResized ) {
        MRect r_viewcrop = transform(cropInfos.tranSensor2Resized, s_viewcrop);
        result.s            = r_viewcrop.s;
        result.p_integral   = r_viewcrop.p + cropInfos.eis_mv_r.p;
        result.p_fractional = cropInfos.eis_mv_r.pf;

        resizedView.p = result.p_integral;
        resizedView.s = result.s;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if( refineBoundary(cropInfos.dstsize_resizer, result) ) {
            MY_LOGD("[FIXME] need to check crop!");
            Cropper::dump(cropInfos);
        }
    }
    else {
        result.s            = s_viewcrop.s;
        result.p_integral   = s_viewcrop.p + cropInfos.eis_mv_s.p;
        result.p_fractional = cropInfos.eis_mv_s.pf;

        resizedView.p = result.p_integral;
        resizedView.s = result.s;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if( refineBoundary(cropInfos.sensor_size, result) ) {
            MY_LOGD("[FIXME] need to check crop!");
            Cropper::dump(cropInfos);
        }
    }
    MY_LOGD_IF(bEnableLog,"Normalized Crop(%d,%d)(%dx%d), DstSize(%dx%d), Active2Sensor(%d,%d)(%dx%d), SensorView(%d,%d)(%dx%d), ResizedView(%d,%d)(%dx%d)->(%d,%d)(%dx%d)",
               cropInfos.crop_a.p.x, cropInfos.crop_a.p.y, cropInfos.crop_a.s.w, cropInfos.crop_a.s.h,
               dstSize.w, dstSize.h, s_crop.p.x, s_crop.p.y, s_crop.s.w, s_crop.s.h,
               s_viewcrop.p.x, s_viewcrop.p.y, s_viewcrop.s.w, s_viewcrop.s.h,
               resizedView.p.x, resizedView.p.y, resizedView.s.w, resizedView.s.h,
               result.p_integral.x, result.p_integral.y, result.s.w, result.s.h
              );
    //
    MY_LOGD_IF(bEnableLog, "resized %d, crop %d/%d, %d/%d, %dx%d",
            isResized,
            result.p_integral.x,
            result.p_integral.y,
            result.p_fractional.x,
            result.p_fractional.y,
            result.s.w,
            result.s.h
            );
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
calcViewAngleF(
    MBOOL bEnableLog,
    MBOOL bUseSubPixel,
    crop_info_t const& cropInfos,
    MSize const& dstSize,
    MCropRect& result,
    MRectF& resultf
)
{
    MBOOL const isResized = cropInfos.isResized;

    MRectF s_crop_f;
    cropInfos.matActive2Sensor.transform(cropInfos.cropf_a, s_crop_f);

    MRectF s_viewcrop_f;
    //
    MINT32 orientation = 0;
    if( s_crop_f.s.w * dstSize.h > s_crop_f.s.h * dstSize.w ) { // pillarbox
        s_viewcrop_f.s.w = (s_crop_f.s.h * dstSize.w / dstSize.h);
        s_viewcrop_f.s.h = s_crop_f.s.h;
        s_viewcrop_f.p.x = s_crop_f.p.x + ((s_crop_f.s.w - s_viewcrop_f.s.w) / 2);
        s_viewcrop_f.p.y = s_crop_f.p.y;
        orientation = 0;
    }
    else { // letterbox
        s_viewcrop_f.s.w = s_crop_f.s.w;
        s_viewcrop_f.s.h = (s_crop_f.s.w * dstSize.h / dstSize.w);
        s_viewcrop_f.p.x = s_crop_f.p.x;
        s_viewcrop_f.p.y = s_crop_f.p.y + ((s_crop_f.s.h - s_viewcrop_f.s.h) / 2);
        orientation = 1;
    }
    MY_LOGD_IF(bEnableLog, "o(%d) s_cropRegionf(%f, %f, %fx%f), dst %dx%d, view cropf(%f, %f, %fx%f)",
            orientation,
            s_crop_f.p.x     , s_crop_f.p.y     ,
            s_crop_f.s.w     , s_crop_f.s.h     ,
            dstSize.w      , dstSize.h      ,
            s_viewcrop_f.p.x , s_viewcrop_f.p.y ,
            s_viewcrop_f.s.w , s_viewcrop_f.s.h
           );
    //
    MRectF resizedViewf;
    if( isResized ) {
        MRectF r_viewcrop_f = transform(cropInfos.tranSensor2Resized, s_viewcrop_f);
        resizedViewf.s = r_viewcrop_f.s;
        resizedViewf.p = r_viewcrop_f.p + cropInfos.eis_mv_r.p;

        resultf = resizedViewf;
        result = getCropRect(resizedViewf);
        if( bUseSubPixel != MTRUE)
        {
            result.p_fractional.x = 0;
            result.p_fractional.y = 0;
            result.w_fractional = 0;
            result.h_fractional = 0;
        }

        MBOOL hwLimit = MFALSE;
        if( bEnableLog && ::property_get_int32("debug.fsc.p2.hwlimit", 0))
        {
            // make sure hw limitation
            result.s.w &= ~(0x1);
            result.s.h &= ~(0x1);
            hwLimit = MTRUE;
        }

        // check boundary
        if( refineBoundary(cropInfos.dstsize_resizer, result, hwLimit) ) {
            MY_LOGD("[FIXME] need to check crop!");
            Cropper::dump(cropInfos);
        }
    }
    else {
        MY_LOGW("[FIXME] FSC not support full image!");
        resizedViewf.s = s_viewcrop_f.s;
        resizedViewf.p = s_viewcrop_f.p + cropInfos.eis_mv_s.p;

        resultf = resizedViewf;
        result = getCropRect(resizedViewf);
        if( bUseSubPixel != MTRUE)
        {
            result.p_fractional.x = 0;
            result.p_fractional.y = 0;
            result.w_fractional = 0;
            result.h_fractional = 0;
        }

        MBOOL hwLimit = MFALSE;
        if( bEnableLog && ::property_get_int32("debug.fsc.p2.hwlimit", 0))
        {
            // make sure hw limitation
            result.s.w &= ~(0x1);
            result.s.h &= ~(0x1);
            hwLimit = MTRUE;
        }

        // check boundary
        if( refineBoundary(cropInfos.sensor_size, result, hwLimit) ) {
            MY_LOGD("[FIXME] need to check crop!");
            Cropper::dump(cropInfos);
        }
    }
    MY_LOGD_IF(bEnableLog, "Normalized Cropf(%f,%f)(%fx%f), DstSize(%dx%d), Active2Sensorf(%f,%f)(%fx%f), SensorViewf(%f,%f)(%fx%f), ResizedViewf(%f,%f)(%fx%f)->(%d,%d)(0x%x,0x%x)(%dx%d)",
               cropInfos.cropf_a.p.x, cropInfos.cropf_a.p.y, cropInfos.cropf_a.s.w, cropInfos.cropf_a.s.h,
               dstSize.w, dstSize.h, s_crop_f.p.x, s_crop_f.p.y, s_crop_f.s.w, s_crop_f.s.h,
               s_viewcrop_f.p.x, s_viewcrop_f.p.y, s_viewcrop_f.s.w, s_viewcrop_f.s.h,
               resizedViewf.p.x, resizedViewf.p.y, resizedViewf.s.w, resizedViewf.s.h,
               result.p_integral.x, result.p_integral.y, result.p_fractional.x, result.p_fractional.y, result.s.w, result.s.h
              );
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
Cropper::
refineBoundary(
    MSize const& bufSize,
    MCropRect& crop,
    MBOOL hwLimit
)
{
    MBOOL isRefined = MFALSE;
    MCropRect refined = crop;
    if( crop.p_integral.x < 0 ) {
        refined.p_integral.x = 0;
        isRefined = MTRUE;
    }
    if( crop.p_integral.y < 0 ) {
        refined.p_integral.y = 0;
        isRefined = MTRUE;
    }
    //
    int const carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
    if( (refined.p_integral.x + crop.s.w + carry_x) > bufSize.w ) {
        refined.s.w = bufSize.w - refined.p_integral.x - carry_x;
        isRefined = MTRUE;
    }
    int const carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
    if( (refined.p_integral.y + crop.s.h + carry_y) > bufSize.h ) {
        refined.s.h = bufSize.h - refined.p_integral.y - carry_y;
        isRefined = MTRUE;
    }
    //
    if( isRefined ) {
        if( hwLimit )
        {
            // make sure hw limitation
            refined.s.w &= ~(0x1);
            refined.s.h &= ~(0x1);
        }

        MY_LOGD("buf size %dx%d, crop(%d/%d, %d/%d, %dx%d) -> crop(%d/%d, %d/%d, %dx%d)",
                bufSize.w, bufSize.h,
                crop.p_integral.x,
                crop.p_integral.y,
                crop.p_fractional.x,
                crop.p_fractional.y,
                crop.s.w,
                crop.s.h,
                refined.p_integral.x,
                refined.p_integral.y,
                refined.p_fractional.x,
                refined.p_fractional.y,
                refined.s.w,
                refined.s.h
                );
        crop = refined;
    }
    return isRefined;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
dump(
     crop_info_t const& crop
)
{
    MY_LOGD("isResized %d", crop.isResized);
    MY_LOGD("p1 info (%d,%d,%dx%d), (%dx%d), (%d,%d,%dx%d)",
        crop.crop_p1_sensor.p.x,
        crop.crop_p1_sensor.p.y,
        crop.crop_p1_sensor.s.w,
        crop.crop_p1_sensor.s.h,
        crop.dstsize_resizer.w,
        crop.dstsize_resizer.h,
        crop.crop_dma.p.x,
        crop.crop_dma.p.y,
        crop.crop_dma.s.w,
        crop.crop_dma.s.h
    );
/*
    MY_LOGD("tran active to sensor o %d, %d, s %dx%d -> %dx%d",
        crop.tranActive2Sensor.tarOrigin.x,
        crop.tranActive2Sensor.tarOrigin.y,
        crop.tranActive2Sensor.oldScale.w,
        crop.tranActive2Sensor.oldScale.h,
        crop.tranActive2Sensor.newScale.w,
        crop.tranActive2Sensor.newScale.h
    );
*/
    MY_LOGD("tran sensor to resized o %d, %d, s %dx%d -> %dx%d",
        crop.tranSensor2Resized.tarOrigin.x,
        crop.tranSensor2Resized.tarOrigin.y,
        crop.tranSensor2Resized.oldScale.w,
        crop.tranSensor2Resized.oldScale.h,
        crop.tranSensor2Resized.newScale.w,
        crop.tranSensor2Resized.newScale.h
    );
    MY_LOGD("modified active crop %d, %d, %dx%d",
        crop.crop_a.p.x,
        crop.crop_a.p.y,
        crop.crop_a.s.w,
        crop.crop_a.s.h
    );
    MY_LOGD("isEisOn %d", crop.isEisEabled);
    MY_LOGD("mv in active %d/%d, %d/%d",
        crop.eis_mv_a.p.x, crop.eis_mv_a.pf.x,
        crop.eis_mv_a.p.y, crop.eis_mv_a.pf.y
    );
    MY_LOGD("mv in sensor %d/%d, %d/%d",
        crop.eis_mv_s.p.x, crop.eis_mv_s.pf.x,
        crop.eis_mv_s.p.y, crop.eis_mv_s.pf.y
    );
    MY_LOGD("mv in resized %d/%d, %d/%d",
        crop.eis_mv_r.p.x, crop.eis_mv_r.pf.x,
        crop.eis_mv_r.p.y, crop.eis_mv_r.pf.y
    );
}


/******************************************************************************
 *
 ******************************************************************************/
} // namespace v3
} // namespace NSCam


/******************************************************************************
 *
 ******************************************************************************/
