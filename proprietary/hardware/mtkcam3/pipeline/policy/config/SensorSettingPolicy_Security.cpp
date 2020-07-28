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

#define LOG_TAG "mtkcam-SensorSettingPolicy-Security"

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

/**
 * Make a function target - Security version
 */
FunctionType_Configuration_SensorSettingPolicy makePolicy_Configuration_SensorSetting_Security()
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

        for ( auto const id : pPipelineStaticInfo->sensorId ) {
            SensorSetting res;  // output
            HwInfoHelper  infoHelper = HwInfoHelper(id);
            infoHelper.updateInfos();

            // sensor mode decision policy:
            // default full mode
            MUINT32 sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            //
            if(!infoHelper.getSecureSensorModeSupported(sensorMode))
            {
                MY_LOGD("ISP don't support secure sensor mode(%d), change to use preview sensor mode",sensorMode,SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
                sensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            }

            if(sensorMode!=SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
            {
                // 4cell sensor need to use preview sensor mode
                if(infoHelper.get4CellSensorSupported())
                {
                    MY_LOGD("4cell sensor need to use preview sensor mode");
                    sensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                }
            }

            parseSensorSetting(res,sensorMode,infoHelper);

            MY_LOGI("select size(%4dx%-4d)@%-3d sensorMode:%d",
                res.sensorSize.w, res.sensorSize.h, res.sensorFps,
                res.sensorMode);
            pvOut->push_back(res);
        }
        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

