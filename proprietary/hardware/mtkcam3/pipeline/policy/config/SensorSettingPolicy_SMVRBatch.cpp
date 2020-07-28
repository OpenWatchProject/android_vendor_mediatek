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

#define LOG_TAG "mtkcam-SensorSettingPolicySMVR"

#include <mtkcam3/pipeline/policy/IConfigSensorSettingPolicy.h>
//
#include "MyUtils.h"

#include <isp_tuning_sensor.h>
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

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {

/**
 * Make a function target - SMVRBatch
 */
FunctionType_Configuration_SensorSettingPolicy makePolicy_Configuration_SensorSetting_SMVRBatch()
{
    return [](Configuration_SensorSetting_Params const& params) -> int {
        auto pvOut = params.pvOut;
        //auto pStreamingFeatureSetting = params.pStreamingFeatureSetting;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;

        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;
        if ( CC_UNLIKELY( ! pvOut || ! pPipelineStaticInfo ) ) {
            MY_LOGE("error input params");
            return -EINVAL;
        }

        auto pParsedAppConfiguration = (pPipelineUserConfiguration != nullptr) ? pPipelineUserConfiguration->pParsedAppConfiguration : nullptr;
        auto pParsedSMVRBatchInfo = (pParsedAppConfiguration != nullptr) ? pParsedAppConfiguration->pParsedSMVRBatchInfo : nullptr;
        if ( CC_UNLIKELY( ! pParsedSMVRBatchInfo ) ) {
            MY_LOGE("error input for pParsedSMVRBatchInfo");
            return -EINVAL;
        }

// print all scenario scenario info
        IHalSensorList * const pIHalSensorList = MAKE_HalSensorList();
        if ( CC_UNLIKELY( pIHalSensorList == nullptr) ) {
            MY_LOGE("error when MAKE_HalSensorList");
            return -EINVAL;
        }

        for ( auto const id : pPipelineStaticInfo->sensorId ) {
            NSCam::SensorStaticInfo sensorStaticInfo;
            memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
            NSIspTuning::ESensorDev_T sensorDev = (NSIspTuning::ESensorDev_T)pIHalSensorList->querySensorDevIdx(id);
//                pIHalSensorList->querySensorStaticInfo(id, &sensorStaticInfo);
            pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            MY_LOGD("SMVRBatch: sensorId=%d, video1-info(fps=%d, %dx%d), "
                   "custom1-info(fps=%d, %dx%d), "
                   "custom2-info(fps=%d, %dx%d), "
                   "custom3-info(fps=%d, %dx%d), "
                   "custom4-info(fps=%d, %dx%d), "
                   "custom5-info(fps=%d, %dx%d) "
                   ,
               id,
               sensorStaticInfo.video1FrameRate/10, sensorStaticInfo.video1Width, sensorStaticInfo.video1Height,
               sensorStaticInfo.custom1FrameRate/10, sensorStaticInfo.SensorCustom1Width, sensorStaticInfo.SensorCustom1Height,
               sensorStaticInfo.custom2FrameRate/10, sensorStaticInfo.SensorCustom2Width, sensorStaticInfo.SensorCustom2Height,
               sensorStaticInfo.custom3FrameRate/10, sensorStaticInfo.SensorCustom3Width, sensorStaticInfo.SensorCustom3Height,
               sensorStaticInfo.custom4FrameRate/10, sensorStaticInfo.SensorCustom4Width, sensorStaticInfo.SensorCustom4Height,
               sensorStaticInfo.custom5FrameRate/10, sensorStaticInfo.SensorCustom5Width, sensorStaticInfo.SensorCustom5Height
               );

            SensorSetting res;  // output
            int32_t       fps = pParsedSMVRBatchInfo->maxFps;
            MSize         size;
            HwInfoHelper  infoHelper = HwInfoHelper(id);
            infoHelper.updateInfos();

            int32_t sensorScen = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
            MUINT targetW = pParsedSMVRBatchInfo->imgW;
            MUINT targetH = pParsedSMVRBatchInfo->imgH;
            if (targetW <= sensorStaticInfo.video1Width && targetH <= sensorStaticInfo.video1Height && fps*10 <= sensorStaticInfo.video1FrameRate)
            {
                fps = sensorStaticInfo.video1FrameRate/10;
                size = MSize(sensorStaticInfo.video1Width, sensorStaticInfo.video1Height);
                sensorScen = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
            }
            else if (targetW <= sensorStaticInfo.SensorCustom1Width && targetH <= sensorStaticInfo.SensorCustom1Height && fps*10 <= sensorStaticInfo.custom1FrameRate)
            {
                fps = sensorStaticInfo.custom1FrameRate/10;
                size = MSize(sensorStaticInfo.SensorCustom1Width, sensorStaticInfo.SensorCustom1Height);
                sensorScen = SENSOR_SCENARIO_ID_CUSTOM1;
            }
            else if (targetW <= sensorStaticInfo.SensorCustom2Width && targetH <= sensorStaticInfo.SensorCustom2Height && fps*10 <= sensorStaticInfo.custom2FrameRate)
            {
                fps = sensorStaticInfo.custom2FrameRate/10;
                size = MSize(sensorStaticInfo.SensorCustom2Width, sensorStaticInfo.SensorCustom2Height);
                sensorScen = SENSOR_SCENARIO_ID_CUSTOM2;
            }
            else if (targetW <= sensorStaticInfo.SensorCustom3Width && targetH <= sensorStaticInfo.SensorCustom3Height && fps*10 <= sensorStaticInfo.custom3FrameRate)
            {
                fps = sensorStaticInfo.custom3FrameRate/10;
                size = MSize(sensorStaticInfo.SensorCustom3Width, sensorStaticInfo.SensorCustom3Height);
                sensorScen = SENSOR_SCENARIO_ID_CUSTOM3;
            }
            else if (targetW <= sensorStaticInfo.SensorCustom4Width && targetH <= sensorStaticInfo.SensorCustom5Height && fps*10 <= sensorStaticInfo.custom4FrameRate)
            {
                fps = sensorStaticInfo.custom4FrameRate/10;
                size = MSize(sensorStaticInfo.SensorCustom4Width, sensorStaticInfo.SensorCustom4Height);
                sensorScen = SENSOR_SCENARIO_ID_CUSTOM4;
            }
            else if (targetW <= sensorStaticInfo.SensorCustom5Width && targetH <= sensorStaticInfo.SensorCustom5Height && fps*10 <= sensorStaticInfo.custom5FrameRate)
            {
                fps = sensorStaticInfo.custom5FrameRate/10;
                size = MSize(sensorStaticInfo.SensorCustom5Width, sensorStaticInfo.SensorCustom5Height);
                sensorScen = SENSOR_SCENARIO_ID_CUSTOM5;
            }
            else
            {
                // couldn't find a proper sensor scenario
                MY_LOGE("SMVRBatch:: couldn't find a proper sensor scenario");
            }

            // !!NOTES: fps and  size are determined at sensor-meta and passed by App
            res.sensorFps  = static_cast<uint32_t>(fps);;
            res.sensorSize = size;
            res.sensorMode = sensorScen;

            MY_LOGD("SMVRBatch:: select sensorScen: %d, size(%dx%d) @ %d", sensorScen, size.w, size.h, fps);
            pvOut->push_back(res);

            // ONLY single cam
            break;
        }

        return OK;
    };
}

};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
