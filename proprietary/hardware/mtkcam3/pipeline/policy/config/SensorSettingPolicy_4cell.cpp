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

#define LOG_TAG "mtkcam-SensorSettingPolicy4cell"

#include <mtkcam3/pipeline/policy/IConfigSensorSettingPolicy.h>
//
#include "MyUtils.h"

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCamHW;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( CC_UNLIKELY( err != OK ) ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

#define parseSensorSetting(res, _scenarioId_, infoHelper)                        \
    do {                                                                       \
        int32_t fps;                                                           \
        MSize   size;                                                          \
        if ( CC_UNLIKELY( ! infoHelper.getSensorFps( _scenarioId_, fps) ) ) {    \
            MY_LOGW("getSensorFps fail"); break;                               \
        }                                                                      \
        if ( CC_UNLIKELY( ! infoHelper.getSensorSize(_scenarioId_, size) ) ) {   \
            MY_LOGW("getSensorSize fail"); break;                              \
        }                                                                      \
        res.sensorFps  = static_cast<uint32_t>(fps);                           \
        res.sensorSize = size;                                                 \
        res.sensorMode = _scenarioId_;                                           \
    } while(0)
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {

static auto isSensorSupport(
    SensorSetting& res,
    const uint32_t hdrHalMode
) -> bool
{
    if ( hdrHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW ){
        const uint32_t MSTREAM_SUPPORT_MIN_FPS = 60;
        if ( res.sensorFps >= MSTREAM_SUPPORT_MIN_FPS ) {
            return true;
        }
    }
    else
    {
        return true;
    }
    return false;
}

static auto
checkVhdrSensor(
    SensorSetting& res,
    const uint32_t hdrSensorMode,
    const uint32_t hdrHalMode,
    const HwInfoHelper& rHelper
) -> int
{
    uint32_t supportHDRMode = 0;
    char forceSensorMode[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.force.vhdr.sensormode", forceSensorMode, "0");
    switch( forceSensorMode[0] )
    {
        case '0':
            break;
        case 'P':
        case 'p':
            parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_PREVIEW,rHelper);
            MY_LOGD("set sensor mode to NORMAL_PREVIEW(%d)",SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
            return OK;
        case 'V':
        case 'v':
            parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_VIDEO,rHelper);
            MY_LOGD("set sensor mode to NORMAL_VIDEO(%d)",SENSOR_SCENARIO_ID_NORMAL_VIDEO);
            return OK;
        case 'C':
        case 'c':
            parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_CAPTURE,rHelper);
            MY_LOGD("set sensor mode to NORMAL_CAPTURE(%d)",SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
            return OK;
        default:
            MY_LOGW("unknown force sensor mode(%s), not used", forceSensorMode);
            MY_LOGW("usage : setprop debug.force.vhdr.sensormode P/V/C");
            break;
        }

    // 1. Current sensor mode is VHDR support, use it.
    if ( CC_UNLIKELY( ! rHelper.querySupportVHDRMode( res.sensorMode,
                                                    supportHDRMode) ) )
    {
        MY_LOGE("[vhdrhal] HwInfoHelper querySupportVHDRMode fail");
        return -EINVAL;
    }
    if (hdrSensorMode == supportHDRMode)
    {
        if ( isSensorSupport(res, hdrHalMode) ) {
            return OK;
        }
    }

    // 2. Check sensor mode in order: preview -> video -> capture
    // Find acceptable sensor mode for this hdrSensorMode
    /*
           SENSOR_SCENARIO_ID_NORMAL_PREVIEW
           SENSOR_SCENARIO_ID_NORMAL_CAPTURE
           SENSOR_SCENARIO_ID_NORMAL_VIDEO
           SENSOR_SCENARIO_ID_SLIM_VIDEO1
           SENSOR_SCENARIO_ID_SLIM_VIDEO2
      */
#define CHECK_SENSOR_MODE_VHDR_SUPPORT(senMode, scenariomode)                \
do {                                                                         \
       if ( ! rHelper.querySupportVHDRMode(senMode, supportHDRMode) )        \
       {                                                                     \
           return -EINVAL;                                                   \
       }                                                                     \
       if ( hdrSensorMode == supportHDRMode )                                \
       {                                                                     \
           parseSensorSetting(res ,senMode ,rHelper);                        \
           if ( isSensorSupport(res, hdrHalMode) ) {                         \
               return OK;                                                    \
           }                                                                 \
       }                                                                     \
} while (0)
       CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_PREVIEW,eNORMAL_PREVIEW);
       CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_VIDEO,eNORMAL_VIDEO);
       CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_CAPTURE, eNORMAL_CAPTURE);
#undef CHECK_SENSOR_MODE_VHDR_SUPPORT

    //3.  PREVIEW & VIDEO & CAPTURE mode are all not acceptable
    MY_LOGE("[vhdrhal] VHDR not support preview & video & capture mode.");
    return -EINVAL;
}
/**
 * Make a function target - 4cell version
 */
FunctionType_Configuration_SensorSettingPolicy makePolicy_Configuration_SensorSetting_4Cell()
{
    return [](Configuration_SensorSetting_Params const& params) -> int {
        auto pvOut = params.pvOut;
        auto pStreamingFeatureSetting = params.pStreamingFeatureSetting;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;

        if ( CC_UNLIKELY( ! pvOut || ! pStreamingFeatureSetting ||
                          ! pPipelineStaticInfo || ! pPipelineUserConfiguration ) ) {
            MY_LOGE("error input params");
            return -EINVAL;
        }

        auto const& pParsedAppImageInfo = pPipelineUserConfiguration->pParsedAppImageStreamInfo;

        for ( auto const id : pPipelineStaticInfo->sensorId ) {
            SensorSetting res;  // output
            HwInfoHelper  infoHelper = HwInfoHelper(id);
            infoHelper.updateInfos();

            // sensor mode decision policy:
            // 1. has video consumer
            if ( pParsedAppImageInfo->hasVideoConsumer ) {
                parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_VIDEO,infoHelper);
                goto lbDone;
            }
            // 2. default preview mode
            parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_PREVIEW,infoHelper);

lbDone:
            if (pStreamingFeatureSetting->hdrHalMode != MTK_HDR_FEATURE_HDR_HAL_MODE_OFF){
                CHECK_ERROR( checkVhdrSensor(res, pStreamingFeatureSetting->hdrSensorMode, pStreamingFeatureSetting->hdrHalMode, infoHelper) );
            }
            {
                MINT32 forceSensorMode = ::property_get_int32("vendor.debug.cameng.force_sensormode", -1);
                if( forceSensorMode != -1 )
                {
                    switch( forceSensorMode )
                    {
                    case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                         parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_PREVIEW,infoHelper);
                         break;
                    case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                         parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_CAPTURE,infoHelper);
                         break;
                    case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                         parseSensorSetting(res,SENSOR_SCENARIO_ID_NORMAL_VIDEO,infoHelper);
                         break;
                    case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_SLIM_VIDEO1,infoHelper);
                        break;
                    case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_SLIM_VIDEO2,infoHelper);
                        break;
                    case SENSOR_SCENARIO_ID_CUSTOM1:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_CUSTOM1,infoHelper);
                        break;
                    case SENSOR_SCENARIO_ID_CUSTOM2:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_CUSTOM2,infoHelper);
                        break;
                    case SENSOR_SCENARIO_ID_CUSTOM3:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_CUSTOM3,infoHelper);
                        break;
                    case SENSOR_SCENARIO_ID_CUSTOM4:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_CUSTOM4,infoHelper);
                        break;
                    case SENSOR_SCENARIO_ID_CUSTOM5:
                        parseSensorSetting(res,SENSOR_SCENARIO_ID_CUSTOM5,infoHelper);
                        break;
                    default:
                        MY_LOGW("Unknown sensorMode: %d", forceSensorMode);
                        break;
                    }
                    CAM_ULOGMI("Force set sensorMode: %d. Selected sensorMode: %d", forceSensorMode, res.sensorMode);
                }
            }
            CAM_ULOGMI("select size(%4dx%-4d)@%-3d sensorMode:%d hdrSensorMode:%d hdrHalMode:%d HfpsMode:%d",
                res.sensorSize.w, res.sensorSize.h, res.sensorFps,
                res.sensorMode,
                pStreamingFeatureSetting->hdrSensorMode, pStreamingFeatureSetting->hdrHalMode);
            pvOut->push_back(res);
        }
        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

