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
#include <cutils/properties.h>
#include <mtkcam/def/common.h>
#include <mtkcam/feature/3dnr/util_3dnr.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>
#include "hal/inc/camera_custom_3dnr.h"
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/feature/eis/eis_ext.h>

/*******************************************************************************
*
********************************************************************************/

#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define UNUSED(var)                 (void)(var)

/*******************************************************************************
*
********************************************************************************/

using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using NSCam::Utils::SENSOR_TYPE_GYRO;

template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/*******************************************************************************
*
********************************************************************************/

Util3dnr::Util3dnr(MINT32 openId)
: mOpenId(openId)
, mLogLevel(0)
, mDebugLevel(0)
, mforce3dnr(0)
, mforceFrameReset(MFALSE)
{
    MY_LOGD("ID(%d)", mOpenId);
}

Util3dnr::~Util3dnr()
{
    MY_LOGD("ID(%d)", mOpenId);

    if (mpSensorProvider != NULL)
        mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
}

void Util3dnr::init(MINT32 force3dnr)
{
    mforce3dnr = force3dnr;
    if (mforce3dnr)
    {
        MY_LOGI("force 3DNR(%d)", ::property_get_int32("vendor.debug.camera.3dnr.enable", 1));
    }
    mLogLevel = ::property_get_int32(NR3D::LOG_LEVEL_PROPERTY, 0);
    mDebugLevel = ::property_get_int32(NR3D::DEBUG_LEVEL_PROPERTY, 0);
    MY_LOGD("mforce3dnr(%d), mLogLevel(%d)", mforce3dnr, mLogLevel);

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

        if (mDebugLevel > 0)
        {
            if (::property_get_int32(NR3D::DEBUG_RESET_GMV_PROPERTY, 0) != 0)
            {
                mvInfo.gmvX = 0;
                mvInfo.gmvY = 0;
            }
        }
    }
}

void Util3dnr::prepareFeatureData(MBOOL canEnable3dnr, const NR3DMVInfo &mvInfo,
                    MINT32 iso, MINT32 isoThreshold, MBOOL isCRZMode,
                    FeaturePipeParam &featureEnqueParams)
{
#define INVALID_ISO_VALUE (-1)
    if (canEnable3dnr == MFALSE)
    {
        featureEnqueParams.setFeatureMask(MASK_3DNR, 0);
        featureEnqueParams.setFeatureMask(MASK_3DNR_RSC, 0);
        return;
    }

    if (isCRZMode)
    {
        featureEnqueParams.setVar<MBOOL>(VAR_3DNR_EIS_IS_CRZ_MODE, isCRZMode);
    }

    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_EIS_X_INT, mvInfo.x_int);
    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_EIS_Y_INT, mvInfo.y_int);
    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_EIS_GMVX, mvInfo.gmvX);
    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_EIS_GMVY, mvInfo.gmvY);
    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_EIS_CONFX, mvInfo.confX);
    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_EIS_CONFY, mvInfo.confY);

    MY_LOGD_IF(mLogLevel, "ID(%d) 3dnr.eis: x_int=%d, y_int=%d, gmvX=%d, gmvY=%d, confX=%d, confY=%d, iso(%d) CRZ(%d)",
        mOpenId,
        mvInfo.x_int,
        mvInfo.y_int,
        mvInfo.gmvX,
        mvInfo.gmvY,
        mvInfo.confX,
        mvInfo.confY,
        iso,
        isCRZMode
        );

    // === ISO value ===
    if (iso != INVALID_ISO_VALUE)
    {
        featureEnqueParams.setVar<MUINT32>(VAR_3DNR_ISO, iso);
    }
    featureEnqueParams.setVar<MUINT32>(VAR_3DNR_ISO_THRESHOLD, isoThreshold);
}

void Util3dnr::prepareISPDataHAL1(MBOOL canEnable3dnr, const NR3DMVInfo &mvInfo, MBOOL isIMGO,
                const MRect &cropP1Sensor, MINT32 iso, MINT32 isoThreshold, MBOOL isCRZMode, MBOOL isSl2eEnable,
                NSCam::IImageBuffer *iBuffer, const NSCam::NSIoPipe::MCropRect &mCropRect,
                NSCam::IMetadata *pMeta_InHal)
{
    MSize inputSize(0, 0);

    if (iBuffer == NULL)
    {
        MY_LOGE("ID(%d) no input buffer: %p", mOpenId, iBuffer);
    }
    else
    {
        inputSize.w = iBuffer->getImgSize().w;
        inputSize.h = iBuffer->getImgSize().h;
    }

    MRect inputCrop;
    if (isCRZMode)
    {
        inputCrop.p = mCropRect.p_integral;
        inputCrop.s = mCropRect.s;
    }
    else if (isIMGO)
    {
        inputCrop = cropP1Sensor;
    }
    else
    {
        inputCrop.p.x = 0;
        inputCrop.p.y = 0;
        inputCrop.s = inputSize;
    }

    MY_LOGD_IF(mLogLevel, "mOpenId(%d): isCRZMode(%d), isIMGO(%d)", mOpenId, isCRZMode, isIMGO);

    prepareISPData(canEnable3dnr, mvInfo,
        inputSize, inputCrop, iso, isoThreshold, isSl2eEnable,
        pMeta_InHal);
}

void Util3dnr::prepareISPData(MBOOL canEnable3dnr, const NR3DMVInfo &mvInfo,
                const MSize &inputSize, const MRect &inputCrop, MINT32 iso, MINT32 isoThreshold, MBOOL isSl2eEnable,
                NSCam::IMetadata *pMeta_InHal)
{
    UNUSED(iso);

    if (pMeta_InHal == NULL)
    {
        MY_LOGE("ID(%d) no meta inHal: %p",
            mOpenId, pMeta_InHal);
        return;
    }

    // must reset frame info for first frame
    MINT32 frameReset = ((canEnable3dnr == MFALSE) || (mforceFrameReset == MTRUE));
    MINT32 i4IsoThreshold = isoThreshold;

    if (mforceFrameReset)
    {
        MY_LOGD_IF(mLogLevel, "ID(%d) force frameReset", mOpenId);
        mforceFrameReset = MFALSE;
    }

    // We had better use the data from prepareGyro
    // to guarantee the identical of gyro data
    // But the algo in setIsp() does not really need Gyro yet
    NSCam::Utils::SensorData sensorData;
    MINT32 isGyroValid = (
        mpSensorProvider != NULL &&
        mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData) ? 1 : 0);
    MINT32 gyroX1000 = 0, gyroY1000 = 0, gyroZ1000 = 0;
    MUINT32 gyroTimestampHigh = 0, gyroTimestampLow = 0;
    if (isGyroValid == 1)
    {
        gyroX1000 = static_cast<MINT32>(sensorData.gyro[0] * 1000.0f + 0.5f);
        gyroY1000 = static_cast<MINT32>(sensorData.gyro[1] * 1000.0f + 0.5f);
        gyroZ1000 = static_cast<MINT32>(sensorData.gyro[2] * 1000.0f + 0.5f);
        gyroTimestampHigh = static_cast<MUINT32>((sensorData.timestamp >> 32) & 0xffffffffULL);
        gyroTimestampLow = static_cast<MUINT32>(sensorData.timestamp & 0xffffffffULL);
    }

    // always set information to ISP for smoothNR3D

    IMetadata::IEntry entry(MTK_3A_ISP_NR3D_SW_PARAMS);//refer to ISP_NR3D_META_INFO_T
    entry.push_back(mvInfo.gmvX, Type2Type< MINT32 >());
    entry.push_back(mvInfo.gmvY, Type2Type< MINT32 >());
    entry.push_back(mvInfo.confX, Type2Type< MINT32 >());
    entry.push_back(mvInfo.confY, Type2Type< MINT32 >());
    entry.push_back(mvInfo.maxGMV, Type2Type< MINT32 >());
    entry.push_back(frameReset, Type2Type< MINT32 >());
    entry.push_back(mvInfo.status, Type2Type< MINT32 >()); //GMV_Status 0: invalid state
    entry.push_back(i4IsoThreshold, Type2Type< MINT32 >());
    entry.push_back(isGyroValid, Type2Type< MINT32 >());
    entry.push_back(gyroX1000, Type2Type< MINT32 >());
    entry.push_back(gyroY1000, Type2Type< MINT32 >());
    entry.push_back(gyroZ1000, Type2Type< MINT32 >());
    entry.push_back(static_cast<MINT32>(gyroTimestampHigh), Type2Type< MINT32 >());
    entry.push_back(static_cast<MINT32>(gyroTimestampLow), Type2Type< MINT32 >());

    pMeta_InHal->update(MTK_3A_ISP_NR3D_SW_PARAMS, entry);

    MY_LOGD_IF(mLogLevel, "ID(%d), gmv(%d,%d) conf(%d,%d) maxGMV(%d) th(%d) Reset(%d) en3(%d) enS(%d) Gyro(%d:%d,%d,%d)",
        mOpenId, mvInfo.gmvX, mvInfo.gmvY, mvInfo.confX, mvInfo.confY, mvInfo.maxGMV, i4IsoThreshold,
        frameReset, canEnable3dnr, isSl2eEnable,
        isGyroValid, gyroX1000, gyroY1000, gyroZ1000);

    if (canEnable3dnr && isSl2eEnable)
    {
        MSize sl2eOriSize;
        MRect sl2eCropInfo;
        MSize sl2eRrzSize;

        if (inputCrop.s.w == 0 || inputCrop.s.h == 0)
        {
            MY_LOGE("ID(%d) zero input size", mOpenId);
            return;
        }

        sl2eOriSize = inputSize;
        sl2eCropInfo = inputCrop;
        sl2eRrzSize = inputCrop.s;

        trySetMetadata<MSize>(pMeta_InHal, MTK_ISP_P2_ORIGINAL_SIZE, sl2eOriSize);
        trySetMetadata<MRect>(pMeta_InHal, MTK_ISP_P2_CROP_REGION, sl2eCropInfo);
        trySetMetadata<MSize>(pMeta_InHal, MTK_ISP_P2_RESIZER_SIZE, sl2eRrzSize);

        MY_LOGD_IF(mLogLevel, "sl2e.imgi(%d,%d), sl2e.crop(%d,%d,%d,%d), sl2e.rrz(%d,%d)",
            sl2eOriSize.w, sl2eOriSize.h,
            sl2eCropInfo.p.x,  sl2eCropInfo.p.y, sl2eCropInfo.s.w, sl2eCropInfo.s.h,
            sl2eRrzSize.w, sl2eRrzSize.h);
    }
}

MBOOL Util3dnr::prepareGyro(
    NSCam::NR3D::GyroData *outGyroData,
    NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam *featureEnqueParams)
{
    NSCam::Utils::SensorData sensorData;
    MBOOL isGyroValid = (
        mpSensorProvider != NULL &&
        mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData));

    if (isGyroValid)
    {
        NSCam::NR3D::GyroData gyroData;
        gyroData.isValid = MTRUE;
        gyroData.x = sensorData.gyro[0];
        gyroData.y = sensorData.gyro[1];
        gyroData.z = sensorData.gyro[2];

        if (outGyroData != NULL)
        {
            *outGyroData = gyroData;
        }
        if (featureEnqueParams != NULL)
        {
            featureEnqueParams->setVar<NSCam::NR3D::GyroData>(VAR_3DNR_GYRO, gyroData);
        }
    }
    else
    {
        if (outGyroData != NULL)
            outGyroData->isValid = MFALSE;
    }

    return isGyroValid;
}

MBOOL Util3dnr::is3dnrDebugMode(void)
{
    return ((mforce3dnr > 0) ? MTRUE : MFALSE);
}

