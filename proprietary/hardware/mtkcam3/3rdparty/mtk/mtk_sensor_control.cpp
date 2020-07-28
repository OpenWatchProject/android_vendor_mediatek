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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#define LOG_TAG "mtkcam-mtk_sensor_control"
//
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/common.h>
//
#include <mtkcam3/3rdparty/mtk/mtk_sensor_control.h>
//
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>
#include <math.h>

/******************************************************************************
 *
 ******************************************************************************/
#define __DEBUG // enable function scope debug
#ifdef __DEBUG
#include <memory>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);
#define FUNCTION_SCOPE \
 auto __scope_logger__ = [](char const* f)->std::shared_ptr<const char>{ \
    CAM_ULOGMD("(%d)[%s] + ", ::gettid(), f); \
    return std::shared_ptr<const char>(f, [](char const* p){CAM_ULOGMD("(%d)[%s] -", ::gettid(), p);}); \
}(__FUNCTION__)
#else
#define FUNCTION_SCOPE
#endif

#define MAX_IDLE_DECIDE_FRAME_COUNT 160
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
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
//
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
/******************************************************************************
 *
 ******************************************************************************/

/**
 * \brief Convert degree to radians
 *
 * \param degree degree
 * \return positive radians
 */
inline float degreeToRadians(float degree)
{
    degree = fmod(degree, 360.0f);
    if(degree < 0.0f) {
        degree += 360.0f;
    }
    return degree * M_PI/180.0f;
}

/**
 * \brief Convert radians to degree
 *
 * \param radians radians
 * \return positive degree
 */
inline float radiansToDegree(float radians)
{
    radians = fmod(radians, 2.0f*M_PI);
    if(radians < 0.0f) {
        radians += 2.0f*M_PI;
    }
    return radians * 180.0f/M_PI;
}

/**
 * \brief Ratio of degrees degree1/degree2
  *
 * \param degree1 Degree1
 * \param degree2 Degree2
 *
 * \return Ratio of degree, e.g. 50deg/60deg->0.8076
 */
inline float degreeRatio(float degree1, float degree2)
{
    return tan(degreeToRadians(degree1/2.0f))/tan(degreeToRadians(degree2/2.0f));
}


namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace sensorcontrol {

static float sPreZoomRatio = .0f;

static bool convertAppCropRegionForEachSensor(
    SensorControlParamOut &out,
    SensorControlParamIn const& in
)
{
    auto iter = in.vInfo.find(in.vSensorIdList[0]);
    if(iter == in.vInfo.end())
    {
        MY_LOGE("cannot find in vInfo. sensorId(%d)", in.vSensorIdList[0]);
        return false;
    }
    auto masterInfo = iter->second;
    MRect appCropRegion;
    if(in.mZoomRatio != .0f)
    {
        // if zoom ratio not equal to 0, it needs to convert zoom ratio to cam::0 crop size.
        // used to simulate app crop region.
        appCropRegion.s.w = (float)masterInfo->mActiveArrayRegion.s.w / (float)in.mZoomRatio;
        appCropRegion.s.h = (float)masterInfo->mActiveArrayRegion.s.h / (float)in.mZoomRatio;
        appCropRegion.p.x = ((float)masterInfo->mActiveArrayRegion.s.w - (float)appCropRegion.s.w) * .5f;
        appCropRegion.p.y = ((float)masterInfo->mActiveArrayRegion.s.h - (float)appCropRegion.s.h) * .5f;
    }
    else
    {
        appCropRegion = in.mrCropRegion;
    }
    float zoomRatioW = (float)masterInfo->mActiveArrayRegion.s.w / (float)appCropRegion.s.w;
    //float zoomRatioH = (float)masterInfo->mActiveArrayRegion.s.h / (float)appCropRegion.s.h;
    float defaultFov_H  = masterInfo->fSensorFov_H;
    float defaultFov_V  = masterInfo->fSensorFov_V;
    out.fZoomRatio = zoomRatioW;

    //check debug prop
    auto debugZoomRatio = property_get_int32("vendor.debug.camera.zoomRatio", 0);
    if (debugZoomRatio) {
        for(auto& item:in.vInfo)
        {
            auto sSensorControlResult = std::make_shared<SensorControlResult>();
            sSensorControlResult->iSensorControl = SensorStatus::E_STREAMING;
            sSensorControlResult->mrAlternactiveCropRegion = appCropRegion;
            sSensorControlResult->mrAlternactiveCropRegion_Cap = appCropRegion;
            out.vResult.insert({item.first, sSensorControlResult});
            out.fZoomRatio = (debugZoomRatio*1.0f)/10;
        }
        MY_LOGD("debugZoomRatio (%f)", out.fZoomRatio);
    }
    else{
        if(in.mZoomRatio != 0 && in.mZoomRatio < 1.0f && in.mAvailableFocalLength.size()>1)
        {
            // for tricam, workaround
            for(auto& item:in.vInfo)
            {
                auto sSensorControlResult = std::make_shared<SensorControlResult>();
                sSensorControlResult->iSensorControl = SensorStatus::E_STREAMING;
                sSensorControlResult->mrAlternactiveCropRegion = appCropRegion;
                sSensorControlResult->mrAlternactiveCropRegion_Cap = appCropRegion;
                out.vResult.insert({item.first, sSensorControlResult});
                out.fZoomRatio = 0.51f;
            }
        }
        else
        {
            // covert crop region to each sensor.
            for(auto& item:in.vInfo)
            {
                auto sSensorControlResult = std::make_shared<SensorControlResult>();
                sSensorControlResult->iSensorControl = SensorStatus::E_STREAMING;

                auto sensorInfo = item.second;
                //app view
                if (sensorInfo->fSensorFov_H == defaultFov_H)
                {
                    sSensorControlResult->mrAlternactiveCropRegion = appCropRegion;
                    sSensorControlResult->mrAlternactiveCropRegion_Cap = appCropRegion;
                    // MY_LOGD("cam(%d), Zoom ratio(%f), mActiveArrayRegion(%d, %d), %dx%d",
                    //         item.first, out.fZoomRatio, masterInfo->mActiveArrayRegion.p.x, masterInfo->mActiveArrayRegion.p.y, masterInfo->mActiveArrayRegion.s.w, masterInfo->mActiveArrayRegion.s.h);
                    MY_LOGD("cam(%d), Zoom ratio(%f), appCropRegion(%d, %d), %dx%d",
                            item.first, out.fZoomRatio, appCropRegion.p.x, appCropRegion.p.y, appCropRegion.s.w, appCropRegion.s.h);
                }
                else
                {
                    //to cal alternactive Crop
                    auto degreeRatio_H = degreeRatio(sensorInfo->fSensorFov_H, defaultFov_H);
                    auto degreeRatio_V = degreeRatio(sensorInfo->fSensorFov_V, defaultFov_V);

                    // defaultH/targetH = 1.3395x
                    // out.iZoomRatio / 1.3395x = target zoom ratio
                    int targetWidth = int(sensorInfo->mActiveArrayRegion.s.w / ((out.fZoomRatio * degreeRatio_V)));
                    int targetHight = int(sensorInfo->mActiveArrayRegion.s.h / ((out.fZoomRatio * degreeRatio_H)));

                    auto targetPointX = (sensorInfo->mActiveArrayRegion.s.w - targetWidth)/2 + sensorInfo->mActiveArrayRegion.p.x;
                    auto targetPointY = (sensorInfo->mActiveArrayRegion.s.h - targetHight)/2 + sensorInfo->mActiveArrayRegion.p.y;
                    MRect dstRect;
                    if (targetPointX <0 || targetPointY < 0)
                    {
                        dstRect = sensorInfo->mActiveArrayRegion;
                    }
                    else
                    {
                        //centro crop, e.g. ori = 100, target = 50, start point =10 ===>target start point= (100-50)/2 + 10 = 35
                        dstRect = MRect( MPoint(targetPointX, targetPointY),
                                               MSize(targetWidth, targetHight)
                                        );
                    }
                    sSensorControlResult->mrAlternactiveCropRegion = dstRect;
                    sSensorControlResult->mrAlternactiveCropRegion_Cap = dstRect;

                    MY_LOGD("cam(%d), Zoom ratio(%f), degreeRatio_H(%f), degreeRatio_V(%f), target crop rect(%d, %d), %dx%d",
                            item.first, out.fZoomRatio, degreeRatio_H, degreeRatio_V, dstRect.p.x, dstRect.p.y, dstRect.s.w, dstRect.s.h);
                }

                out.vResult.insert({item.first, sSensorControlResult});
            }
        }
    }
    return true;
}

static bool decideSensorState(
    SensorControlParamOut &out,
    SensorControlParamIn const& in
)
{
    auto updateSensorState = [&out](
                                    int32_t sensorId,
                                    bool isMaster,
                                    SensorStatus sensorState)
    {
        auto iter = out.vResult.find(sensorId);
        if(iter != out.vResult.end())
        {
            if(iter->second != nullptr)
            {
                iter->second->isMaster = isMaster;
                iter->second->iSensorControl = sensorState;
            }
            else
                return false;
        }
        return true;
    };
    MY_LOGD("%f", out.fZoomRatio);
    // current decision
    // ========================================================
    // 1.x
    // cam_0: streaming
    // cam_1: standby
    // cam_2: standby (option)
    // ========================================================
    // 2.x
    // cam_0: standby
    // cam_1: streaming
    // cam_2: standby (option)
    // ========================================================
    // 0.x  (option)
    // cam_0: standby
    // cam_1: standby
    // cam_2: streaming (option)
    if(out.fZoomRatio >= 2.0f)
    {
        // cam_0 is slave, and set to standby mode.
        updateSensorState(in.vSensorIdList[0], false, SensorStatus::E_STANDBY);
        // cam_1 is master, and set to streaming.
        updateSensorState(in.vSensorIdList[1], true, SensorStatus::E_STREAMING);
        // cam_2 is slave, and set to standby.
        if(in.vSensorIdList.size()>2)
            updateSensorState(in.vSensorIdList[2], false, SensorStatus::E_STANDBY);
    }
    else if(out.fZoomRatio >= 1.0f)
    {
        // cam_0 is master, and set to streaming.
        updateSensorState(in.vSensorIdList[0], true, SensorStatus::E_STREAMING);
        // cam_1 is slave, and set to standby.
        updateSensorState(in.vSensorIdList[1], false, SensorStatus::E_STANDBY);
        // cam_2 is slave, and set to standby.
        if(in.vSensorIdList.size()>2)
            updateSensorState(in.vSensorIdList[2], false, SensorStatus::E_STANDBY);
    }
    else
    {
        // cam_0 is slave, and set to streaming.
        updateSensorState(in.vSensorIdList[0], false, SensorStatus::E_STANDBY);
        // cam_1 is slave, and set to standby.
        updateSensorState(in.vSensorIdList[1], false, SensorStatus::E_STANDBY);
        // cam_2 is master, and set to streaming.
        if(in.vSensorIdList.size()>2)
            updateSensorState(in.vSensorIdList[2], true, SensorStatus::E_STREAMING);
    }
    // when user zoom in/out, active all cameras
    // For Hw limitation, this flow only support sensor size is 2.
    if (sPreZoomRatio != out.fZoomRatio && in.vSensorIdList.size() == 2)
    {
        //active two cameras
        for(auto&& item:out.vResult)
        {
            item.second->iSensorControl = SensorStatus::E_STREAMING;
        }
        MY_LOGD("sPreZoomRatio=%f, out.fZoomRatio=%f, active slave camera", sPreZoomRatio, out.fZoomRatio);
        sPreZoomRatio = out.fZoomRatio;
        return true;
    }
    return true;
}

bool mtk_decision_sensor_control_zoom(
    SensorControlParamOut &out,
    SensorControlParamIn const& in
)
{
    if(!convertAppCropRegionForEachSensor(out, in))
    {
        return false;
    }
    if(!decideSensorState(out, in))
    {
        return false;
    }
    // main1 always streaming.
    {
        auto iter = out.vResult.find(in.vSensorIdList[0]);
        if(iter != out.vResult.end())
        {
            if(iter->second != nullptr)
            {
                iter->second->iSensorControl = SensorStatus::E_STREAMING;
            }
        }
    }
    return true;
}

bool
mtk_decision_sensor_control_always_streaming(
    SensorControlParamOut &out,
    SensorControlParamIn const& in
)
{
    // if sensor count more than 2, it will be set standby.
    // first element is master cam.
    for(size_t i=0;i<in.vSensorIdList.size();++i)
    {
        auto sensorId = in.vSensorIdList[i];
        SensorStatus iSensorStatus = SensorStatus::E_STREAMING;
        if(i>=2) iSensorStatus = SensorStatus::E_STANDBY;
        auto sSensorControlResult = std::make_shared<SensorControlResult>();
        sSensorControlResult->iSensorControl = iSensorStatus;
        sSensorControlResult->mrAlternactiveCropRegion = in.mrCropRegion;
        sSensorControlResult->mrAlternactiveCropRegion_Cap = in.mrCropRegion;
        if(i==0) sSensorControlResult->isMaster = true;
        out.vResult.insert({sensorId, sSensorControlResult});
        out.fZoomRatio = 1.0f;
    }
    return true;
}

bool mtk_decision_sensor_control(
    SensorControlParamOut &out,
    SensorControlParamIn const& in
)
{
    if(in.vSensorIdList.size() > 3)
    {
        MY_LOGE("Not support");
        return false;
    }
    if(in.mFeatureMode == FeatureMode::E_Zoom)
    {
        mtk_decision_sensor_control_zoom(out, in);
    }
    else if(in.mFeatureMode == FeatureMode::E_Bokeh)
    {
        mtk_decision_sensor_control_always_streaming(out, in);
    }
    else if(in.mFeatureMode == FeatureMode::E_Multicam)
    {
        mtk_decision_sensor_control_always_streaming(out, in);
    }
    else
    {
        MY_LOGA("not support mode(%" PRIu32 ")", in.mFeatureMode);
    }
    return true;
}

};  //namespace sensorcontrol
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
