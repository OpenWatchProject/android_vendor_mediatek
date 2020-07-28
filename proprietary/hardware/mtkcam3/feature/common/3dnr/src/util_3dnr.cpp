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


#define LOG_TAG "util_3dnr"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <cutils/properties.h>
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/3dnr/util_3dnr.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include "hal/inc/camera_custom_3dnr.h"
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/lmv/lmv_ext.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_3DNR_HAL);

/*******************************************************************************
*
********************************************************************************/

#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define UNUSED(var)                 (void)(var)

/*******************************************************************************
*
********************************************************************************/

using namespace NSCam;
using namespace NSCam::NR3D;
using namespace NSCam::NSIoPipe::NSPostProc;
using NSCam::Utils::SENSOR_TYPE_GYRO;

/*******************************************************************************
*
********************************************************************************/

static MINT32 extractIso(const IMetadata *appMeta)
{
    MINT32 iso = 100;
    if (appMeta == NULL) {
        MY_LOGW("pMetadata == NULL");
    }
    else
    {
        IMetadata::IEntry entry = appMeta->entryFor(MTK_SENSOR_SENSITIVITY);
        if (!entry.isEmpty()) {
            iso = entry.itemAt(0, Type2Type<MINT32>());
        }
    }

    return iso;
}

Util3dnr::Util3dnr(MINT32 openId)
: mOpenId(openId)
{
    MY_LOGD("s(%d)", mOpenId);
}

Util3dnr::~Util3dnr()
{
    MY_LOGD("s(%d)", mOpenId);

    if (mpSensorProvider != NULL)
        mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
}

void Util3dnr::init(MINT32 debugEnable)
{
    mDebugEnable = debugEnable;
    mLogLevel = ::property_get_int32(NR3D_PROP_LOG_LEVEL, 0);
    MY_LOGD("DebugEnable(%d), LogLevel(%d)", mDebugEnable, mLogLevel);

    mpSensorProvider = NSCam::Utils::SensorProvider::createInstance("Util3dnr");
    if (!mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, 10))
        mpSensorProvider = NULL;
}

MBOOL Util3dnr::canEnable3dnr(MBOOL isUIEnable, MINT32 iso, MINT32 isoThreshold)
{
    MBOOL canEnable3dnr = MFALSE;

    // must turn off nr3d either UI off or iso < threshold
    if (isUIEnable)
    {
        if (iso < isoThreshold)
        {
            MY_LOGD_IF(mLogLevel, "DISABLE 3DNR: due to iso(%d) < %d",
                iso, isoThreshold);

            canEnable3dnr = MFALSE;
        }
        else
        {
            canEnable3dnr = MTRUE;
        }
    }

    return canEnable3dnr;
}

void Util3dnr::modifyMVInfo(MBOOL canEnable3dnr, MBOOL isIMGO, const MRect &cropP1Sensor,
                const MSize &dstSizeResizer, NR3DMVInfo &mvInfo)
{
    if (canEnable3dnr)
    {
        // Modify mvInfo for IMGO->IMGI
        if (isIMGO)
        {
            MINT32 x_int = mvInfo.gmvX/LMV_GMV_VALUE_TO_PIXEL_UNIT;
            MINT32 y_int = mvInfo.gmvY/LMV_GMV_VALUE_TO_PIXEL_UNIT;
            MINT32 max_gmv = mvInfo.maxGMV;
            mvInfo.gmvX = (x_int * cropP1Sensor.s.w / dstSizeResizer.w * LMV_GMV_VALUE_TO_PIXEL_UNIT);
            mvInfo.gmvY = (y_int * cropP1Sensor.s.h / dstSizeResizer.h * LMV_GMV_VALUE_TO_PIXEL_UNIT);
            mvInfo.maxGMV = (max_gmv * cropP1Sensor.s.h / dstSizeResizer.h);

            MY_LOGD_IF(mLogLevel, "[IMGO2IMGI] modify gmv(%d,%d)->(%d,%d) maxGMV(%d)->(%d)", x_int, y_int,
                mvInfo.gmvX/LMV_GMV_VALUE_TO_PIXEL_UNIT, mvInfo.gmvY/LMV_GMV_VALUE_TO_PIXEL_UNIT, max_gmv, mvInfo.maxGMV);
        }

        if (mDebugEnable > 0)
        {
            if (::property_get_int32(NR3D_PROP_DEBUG_RESET_GMV, 0) != 0)
            {
                mvInfo.gmvX = 0;
                mvInfo.gmvY = 0;
            }
        }
    }
}

void Util3dnr::adjustMVInfo(
    const MRect &cropP1Sensor,
    const MSize &dstSizeResizer,
    NR3DMVInfo &mvInfo)
{
    // Modify mvInfo for IMGO->IMGI
    MINT32 x_int = mvInfo.gmvX/LMV_GMV_VALUE_TO_PIXEL_UNIT;
    MINT32 y_int = mvInfo.gmvY/LMV_GMV_VALUE_TO_PIXEL_UNIT;
    MINT32 max_gmv = mvInfo.maxGMV;
    mvInfo.gmvX = (x_int * cropP1Sensor.s.w / dstSizeResizer.w * LMV_GMV_VALUE_TO_PIXEL_UNIT);
    mvInfo.gmvY = (y_int * cropP1Sensor.s.h / dstSizeResizer.h * LMV_GMV_VALUE_TO_PIXEL_UNIT);
    mvInfo.maxGMV = (max_gmv * cropP1Sensor.s.h / dstSizeResizer.h);

    MY_LOGD_IF(mLogLevel, "3dnr: adjustMVInfo: gmv(%d,%d)->(%d,%d) maxGMV(%d)->(%d)", x_int, y_int,
        mvInfo.gmvX/LMV_GMV_VALUE_TO_PIXEL_UNIT, mvInfo.gmvY/LMV_GMV_VALUE_TO_PIXEL_UNIT, max_gmv, mvInfo.maxGMV);

    if (mDebugEnable)
    {
        if (::property_get_int32(NR3D_PROP_DEBUG_RESET_GMV, 0) != 0) {
            mvInfo.gmvX = 0;
            mvInfo.gmvY = 0;
            MY_LOGD_IF(mLogLevel, "3dnr: reset gmv to 0 by property");
        }
    }
}

void Util3dnr::prepareFeatureData(const Util3dnrParam& in, Util3dnrResult &out)
{
    // MV info
    if (in.isIMGO_p2a)
    {
        adjustMVInfo(in.p1CropRect, in.dstSize_p2a, out.mvInfo);
    }

    MY_LOGD_IF(mLogLevel,
        "s(%d) 3dnr.eis: x,y=%d,%d, gmv=%d,%d, conf=%d,%d, iso(%d) CRZ(%d)",
        mOpenId,
        out.mvInfo.x_int,
        out.mvInfo.y_int,
        out.mvInfo.gmvX,
        out.mvInfo.gmvY,
        out.mvInfo.confX,
        out.mvInfo.confY,
        in.iso,
        in.isCRZ_p2a
        );

    // gyro
    prepareGyro(out.pGyroData);
}

void Util3dnr::prepareGyro(NSCam::NR3D::GyroData *out_pGyroData)
{
    if (out_pGyroData == NULL)
    {
        return;
    }
    NSCam::Utils::SensorData sensorData;
    out_pGyroData->isValid =
       (mpSensorProvider != NULL &&
        mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData));

    if (out_pGyroData->isValid)
    {
        out_pGyroData->x = sensorData.gyro[0];
        out_pGyroData->y = sensorData.gyro[1];
        out_pGyroData->z = sensorData.gyro[2];
    }

    return;
}

