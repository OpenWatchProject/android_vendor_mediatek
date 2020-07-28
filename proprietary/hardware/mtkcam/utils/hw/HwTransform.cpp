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

#define LOG_TAG "MtkCam/HwTransHelper"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/drv/IHalSensor.h>

#include <utils/Mutex.h>
#include <utils/KeyedVector.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace NSCamHW;
using namespace NSCam;

#define ACTIVEARRAY_MODE            SENSOR_SCENARIO_ID_NORMAL_CAPTURE

/*******************************************************************************
*
********************************************************************************/
MVOID
simplifiedMatrix::
dump(const char* const str) const
{
    MY_LOGD("%s: (%f, %f, %f, %f)",
            str,
            c_00, c_02, c_11, c_12);
}


/*******************************************************************************
*
********************************************************************************/
namespace {
class SensorInfo
{
public:
                                    SensorInfo(MINT32 const openId)
                                        : mOpenId(openId)
                                    {}
    MBOOL                           getMatrix(
                                        MUINT32 const sensorMode,
                                        HwMatrix* mat,// mat: transform from active to sensor mode
                                        HwMatrix* mat_inv,
                                        MBOOL forceAspRatioAlign = MFALSE
                                    );
    // Note: getMatrix(...) should be called before this getCropInfo(...)
    MBOOL                           getCropInfo(
                                        MUINT32 const sensorMode,
                                        SensorCropWinInfo* cropInfo_mode,
                                        SensorCropWinInfo* cropInfo_active
                                    );

private:
    android::Mutex                  mLock;
    MINT32 const                    mOpenId;

    struct cropItem
    {
        SensorCropWinInfo    sensor_crop_info;
        HwMatrix             trans;
        HwMatrix             inv_trans;
        HwMatrix             trans_ratio_align;
        HwMatrix             inv_trans_ratio_align;
    };
    // key: sensor mode
    // value: cropItem
    android::KeyedVector<
        MUINT32, cropItem
        >                           mvCropInfos;
};
}   //namesapce


/*******************************************************************************
*
********************************************************************************/
MBOOL
SensorInfo::
getMatrix(MUINT32 const sensorMode, HwMatrix* mat, HwMatrix* mat_inv, MBOOL forceAspRatioAlign)
{
    struct updater
    {
        IHalSensorList* pSensorList;
        IHalSensor*     pSensorHalObj;
        //
                        updater() : pSensorList(NULL), pSensorHalObj(NULL) {}
                        ~updater() {
                            if( pSensorHalObj ) pSensorHalObj->destroyInstance(LOG_TAG);
                        }
        //
        MBOOL
            operator() (MINT32 const openId, MUINT32 const sensorMode, SensorCropWinInfo& cropInfo)
        {
            pSensorList = MAKE_HalSensorList();
            if( ! pSensorList ||
                ! (pSensorHalObj = pSensorList->createSensor(LOG_TAG, openId))
              )
            {
                MY_LOGE("fail pSensorList(%p), pSensorHal(%p)", pSensorList, pSensorHalObj);
                return MFALSE;
            }
            //
            MINT32 err = pSensorHalObj->sendCommand(
                    pSensorList->querySensorDevIdx(openId),
                    SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                    (MUINTPTR)&sensorMode,
                    (MUINTPTR)&cropInfo,
                    0);
            //
            if( err != 0 ||
                    cropInfo.full_w == 0 || cropInfo.full_h == 0 ||
                    cropInfo.w0_size == 0 || cropInfo.h0_size == 0)
            {
                MY_LOGW("cannot get proper sensor %d crop win info of mode (%d): use default",
                        openId, sensorMode);
                //
                SensorStaticInfo staticInfo;
                memset(&staticInfo, 0, sizeof(SensorStaticInfo));
                //
                pSensorList->querySensorStaticInfo(
                    pSensorList->querySensorDevIdx(openId),
                    &staticInfo);
                //
                MSize tgsize;
                switch( sensorMode )
                {
                #define sensor_mode_case( _mode_, _key_, _size_ ) \
                    case _mode_:                                  \
                        _size_.w = staticInfo._key_##Width;       \
                        _size_.h = staticInfo._key_##Height;      \
                        break;
                    sensor_mode_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, tgsize );
                    sensor_mode_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, tgsize );
                    sensor_mode_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, tgsize );
                    sensor_mode_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, tgsize );
                    sensor_mode_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, tgsize );
                    sensor_mode_case( SENSOR_SCENARIO_ID_CUSTOM1, SensorCustom1, tgsize);
                    sensor_mode_case( SENSOR_SCENARIO_ID_CUSTOM2, SensorCustom2, tgsize);
                    sensor_mode_case( SENSOR_SCENARIO_ID_CUSTOM3, SensorCustom3, tgsize);
                    sensor_mode_case( SENSOR_SCENARIO_ID_CUSTOM4, SensorCustom4, tgsize);
                    sensor_mode_case( SENSOR_SCENARIO_ID_CUSTOM5, SensorCustom5, tgsize);
                #undef sensor_mode_case
                    default:
                        MY_LOGE("not support sensor scenario(0x%x)", sensorMode);
                        return MFALSE;
                }
                //
                //cropInfo.full_w;
                //cropInfo.full_h;
                cropInfo.x0_offset = 0;
                cropInfo.y0_offset = 0;
                cropInfo.w0_size = tgsize.w;
                cropInfo.h0_size = tgsize.h;
                //
                cropInfo.scale_w = tgsize.w;
                cropInfo.scale_h = tgsize.h;
                //
                cropInfo.x1_offset = 0;
                cropInfo.y1_offset = 0;
                cropInfo.w1_size = tgsize.w;
                cropInfo.h1_size = tgsize.w;
                //
                cropInfo.x2_tg_offset = 0;
                cropInfo.y2_tg_offset = 0;
                cropInfo.w2_tg_size = tgsize.w;
                cropInfo.h2_tg_size = tgsize.h;
            }
            MY_LOGD("senor %d, mode %d: crop infos", openId, sensorMode);
            MY_LOGD("full %dx%d, crop0(%d,%d,%dx%d), resized(%d,%d)",
                    cropInfo.full_w, cropInfo.full_h,
                    cropInfo.x0_offset, cropInfo.y0_offset, cropInfo.w0_size, cropInfo.h0_size,
                    cropInfo.scale_w, cropInfo.scale_h
                    );
            MY_LOGD("crop1(%d,%d,%dx%d), tg(%d,%d,%dx%d)",
                    cropInfo.x1_offset, cropInfo.y1_offset, cropInfo.w1_size, cropInfo.h1_size,
                    cropInfo.x2_tg_offset, cropInfo.y2_tg_offset, cropInfo.w2_tg_size, cropInfo.h2_tg_size
                   );
            return MTRUE;
        }
    };
    //
    android::Mutex::Autolock _l(mLock);
    //
    ssize_t idxTarget = mvCropInfos.indexOfKey(sensorMode);
    if( 0 > idxTarget ) {
        // check active array
        ssize_t idxActive = mvCropInfos.indexOfKey(ACTIVEARRAY_MODE);
        if( 0 > idxActive ) {
            cropItem itemActive;
            if( ! updater()(mOpenId, ACTIVEARRAY_MODE, itemActive.sensor_crop_info) )
                return MFALSE;
            //
            itemActive.trans     = HwMatrix(1.f, 0.f, 1.f, 0.f);
            itemActive.inv_trans = HwMatrix(1.f, 0.f, 1.f, 0.f);
            //
            idxActive = mvCropInfos.add(ACTIVEARRAY_MODE, itemActive);
        }

        SensorCropWinInfo crop_active;
        memset(&crop_active, 0, sizeof(SensorCropWinInfo));
        if( idxActive>=0 )
            crop_active = mvCropInfos.valueAt((size_t)idxActive).sensor_crop_info;
        //
        cropItem item;
        if( ! updater()(mOpenId, sensorMode, item.sensor_crop_info) )
                return MFALSE;
        //
        SensorCropWinInfo const& crop_target = item.sensor_crop_info;
        //
#if 0
        // check left-top
        if( crop_target.x0_offset <= crop_active.x0_offset ||
            crop_target.y0_offset <= crop_active.y0_offset )
        {
            MY_LOGW("left-top should in active array");
        }
        //
        // check right-bottom
        if(
                (crop_target.x0_offset + crop_target.w0_size <=
                 crop_active.x0_offset + crop_active.w0_size) ||
                (crop_target.y0_offset + crop_target.h0_size <=
                 crop_active.y0_offset + crop_active.h0_size)
          )
        {
            MY_LOGW("right-bottom should in active array");
        }
#endif
        //
        struct forward_matrix
        {
            static
                HwMatrix get(SensorCropWinInfo const& info) {
                    return
                        HwMatrix(
                                1.f,
                                -(info.x1_offset + info.x2_tg_offset),
                                1.f,
                                -(info.y1_offset + info.y2_tg_offset)
                                )
                        * HwMatrix(
                                float(info.scale_w) / float(info.w0_size), 0.f,
                                float(info.scale_h) / float(info.h0_size), 0.f
                                )
                        * HwMatrix(
                                1.f,
                                -(info.x0_offset),
                                1.f,
                                -(info.y0_offset)
                                );
                }
            static
                HwMatrix getAlign(SensorCropWinInfo const& info) {
#define abs(x,y) ((x)>(y)?(x)-(y):(y)-(x))
#define THRESHOLD 0.1
                    if (abs((float)info.full_w / (float)info.full_h , (float)info.w0_size/(float)info.h0_size) < THRESHOLD)
                        return get(info); // aspect ratio not change
#undef THRESHOLD
#undef abs
                    MRect crop0;
                    MRect src(MPoint(info.x0_offset, info.y0_offset), MSize(info.w0_size, info.h0_size));
                    MSize target(info.full_w, info.full_h);
                    HwTransHelper::cropAlignRatio(src, target, crop0);
                    MY_LOGD("align crop(%d,%d,%dx%d)", crop0.p.x, crop0.p.y, crop0.s.w, crop0.s.h );

                    return
                        HwMatrix(
                                1.f,
                                -(info.x1_offset + info.x2_tg_offset),
                                1.f,
                                -(info.y1_offset + info.y2_tg_offset)
                                )
                        * HwMatrix(
                                float(info.scale_w) / float(info.w0_size), 0.f,
                                float(info.scale_h) / float(info.h0_size), 0.f
                                )
                        * HwMatrix(
                                1.f,
                                -(info.x0_offset - crop0.p.x),
                                1.f,
                                -(info.y0_offset - crop0.p.y)
                                )
                        * HwMatrix(
                                float(crop0.s.w) / float(info.full_w), 0.f,
                                float(crop0.s.h) / float(info.full_h), 0.f
                                );
                }
        };
        //
        HwMatrix active_forward = forward_matrix::get(crop_active);
        //active_forward.dump("active forward"); //debug
        //
        HwMatrix active_inv;
        if( ! active_forward.getInverse(active_inv) ) {
            MY_LOGE("cannot get proper inverse matrix of active");
            return MFALSE;
        }
        //active_inv.dump("active_inv"); //debug
        //
        HwMatrix target_forward = forward_matrix::get(crop_target);
        HwMatrix target_forward_align = forward_matrix::getAlign(crop_target);
        //target_forward.dump("target_forward"); //debug
        target_forward_align.dump("target_forward_align"); //debug
        //
        item.trans =  target_forward * active_inv;
        item.trans_ratio_align = target_forward_align * active_inv;
        //item.trans.dump("trans"); //debug
        //
        if( ! item.trans.getInverse(item.inv_trans) || ! item.trans_ratio_align.getInverse(item.inv_trans_ratio_align)) {
            MY_LOGE("cannot get proper inverse matrix");
        }
        else {
            //item.inv_trans.dump("inv_trans"); //debug
            idxTarget = mvCropInfos.add(sensorMode, item);
        }
    }
    //
    if( idxTarget >= 0 ) {
        if( mat )    *mat     = forceAspRatioAlign
                            ? mvCropInfos.valueAt(idxTarget).trans_ratio_align
                            : mvCropInfos.valueAt(idxTarget).trans;
        if( mat_inv) *mat_inv = forceAspRatioAlign
                            ? mvCropInfos.valueAt(idxTarget).inv_trans_ratio_align
                            : mvCropInfos.valueAt(idxTarget).inv_trans;
    }
    //
    return idxTarget >= 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SensorInfo::
getCropInfo(
    MUINT32 const sensorMode,
    SensorCropWinInfo* pcropInfo_mode,
    SensorCropWinInfo* pcropInfo_active
)
{
    android::Mutex::Autolock _l(mLock);
    //
    ssize_t idxTarget = mvCropInfos.indexOfKey(sensorMode);
    if( idxTarget < 0 )
        return MFALSE;
    if( pcropInfo_mode ) *pcropInfo_mode = mvCropInfos[idxTarget].sensor_crop_info;
    //
    ssize_t idxActive = mvCropInfos.indexOfKey(ACTIVEARRAY_MODE);
    if( idxActive < 0 )
        return MFALSE;
    if( pcropInfo_active ) *pcropInfo_active = mvCropInfos[idxActive].sensor_crop_info;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
static android::Mutex&                                   gLock = *new android::Mutex();
static android::KeyedVector< MUINT32, SensorInfo* >     gvSensorInfos;


/*******************************************************************************
*
********************************************************************************/
HwTransHelper::
HwTransHelper(MINT32 const openId)
    : mOpenId(openId)
{
    android::Mutex::Autolock _l(gLock);
    if( gvSensorInfos.indexOfKey(openId) < 0 ) {
        gvSensorInfos.add(openId, new SensorInfo(openId));
    }
}


/*******************************************************************************
*
********************************************************************************/
HwTransHelper::
~HwTransHelper()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HwTransHelper::
getMatrixFromActive(MUINT32 const sensorMode, HwMatrix& mat)
{
    SensorInfo* pSensorInfo = NULL;
    //
    {
        android::Mutex::Autolock _l(gLock);
        ssize_t idx = gvSensorInfos.indexOfKey(mOpenId);
        if( idx < 0 ) {
            return MFALSE;
        }
        pSensorInfo = gvSensorInfos.valueAt(idx);
    }
    //
    return pSensorInfo->getMatrix(sensorMode, &mat, NULL);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HwTransHelper::
getMatrixToActive(MUINT32 const sensorMode, HwMatrix& mat)
{
    SensorInfo* pSensorInfo = NULL;
    //
    {
        android::Mutex::Autolock _l(gLock);
        ssize_t idx = gvSensorInfos.indexOfKey(mOpenId);
        if( idx < 0 ) {
            return MFALSE;
        }
        pSensorInfo = gvSensorInfos.valueAt(idx);
    }
    //
    return pSensorInfo->getMatrix(sensorMode, NULL, &mat);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
HwTransHelper::
getMatrixFromActiveRatioAlign(MUINT32 const sensorMode, HwMatrix& mat)
{
    SensorInfo* pSensorInfo = NULL;
    //
    {
        android::Mutex::Autolock _l(gLock);
        ssize_t idx = gvSensorInfos.indexOfKey(mOpenId);
        if( idx < 0 ) {
            return MFALSE;
        }
        pSensorInfo = gvSensorInfos.valueAt(idx);
    }
    //
    return pSensorInfo->getMatrix(sensorMode, &mat, NULL, MTRUE);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HwTransHelper::
getMatrixToActiveRatioAlign(MUINT32 const sensorMode, HwMatrix& mat)
{
    SensorInfo* pSensorInfo = NULL;
    //
    {
        android::Mutex::Autolock _l(gLock);
        ssize_t idx = gvSensorInfos.indexOfKey(mOpenId);
        if( idx < 0 ) {
            return MFALSE;
        }
        pSensorInfo = gvSensorInfos.valueAt(idx);
    }
    //
    return pSensorInfo->getMatrix(sensorMode, NULL, &mat, MTRUE);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HwTransHelper::
calculateFovDifference(MUINT32 const sensorMode, float* fov_diff_x, float* fov_diff_y)
{
    SensorInfo* pSensorInfo = NULL;
    //
    {
        android::Mutex::Autolock _l(gLock);
        ssize_t idx = gvSensorInfos.indexOfKey(mOpenId);
        if( idx < 0 ) {
            return MFALSE;
        }
        pSensorInfo = gvSensorInfos.valueAt(idx);
    }
    //
    HwMatrix mat_mode2active;
    SensorCropWinInfo cropInfo_mode;
    SensorCropWinInfo cropInfo_active;
    //
    if( ! pSensorInfo->getMatrix(sensorMode, NULL, &mat_mode2active) ) {
        MY_LOGW("cannot get infos of mode %d", sensorMode);
        return MFALSE;
    }
    //
    if( ! pSensorInfo->getCropInfo(sensorMode, &cropInfo_mode, &cropInfo_active) ) {
        MY_LOGW("cannot get crop infos of mode %d", sensorMode);
        return MFALSE;
    }
    //
    MPoint topleft;
    mat_mode2active.transform(MPoint(0,0), topleft);
    //
    MPoint bottomright;
    mat_mode2active.transform(MPoint(cropInfo_mode.w2_tg_size-1,cropInfo_mode.h2_tg_size-1), bottomright);
    //
    // calculate the fov difference
#define abs(x,y) ((x)>(y)?(x)-(y):(y)-(x))
    float diff_x = abs(topleft.x,0) + abs(bottomright.x,cropInfo_active.w2_tg_size-1);
    float diff_y = abs(topleft.y,0) + abs(bottomright.y,cropInfo_active.h2_tg_size-1);
#undef abs
    MY_LOGD("sensorMode(%d), topleft(%d,%d), btmRight(%d,%d),diff(%f, %f), cropMode(%d,%d), cropAct(%d,%d)",
            sensorMode, topleft.x, topleft.y, bottomright.x, bottomright.y, diff_x, diff_y,
            cropInfo_mode.w2_tg_size, cropInfo_mode.h2_tg_size, cropInfo_active.w2_tg_size, cropInfo_active.h2_tg_size) ;
    //
    if( fov_diff_x ) *fov_diff_x = diff_x / float(cropInfo_active.w2_tg_size);
    if( fov_diff_y ) *fov_diff_y = diff_y / float(cropInfo_active.h2_tg_size);
    //
    return MTRUE;
}
