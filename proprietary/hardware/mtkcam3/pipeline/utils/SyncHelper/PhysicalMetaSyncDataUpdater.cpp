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

#define LOG_TAG "MtkCam/PhysicalMetaSyncDataUpdater"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>

#include "PhysicalMetaSyncDataUpdater.h"
#include <mtkcam/utils/std/ULog.h>
//#include <cmath>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P1_SYNCHELPER);

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace NSCam::v3::Utils::Imp;

/******************************************************************************
 *
 ******************************************************************************/
PhysicalMetaSyncDataUpdater::
PhysicalMetaSyncDataUpdater(int32_t openId)
// : __OPEN_ID(openId)
{
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if(!pHalDeviceList)
    {
        MY_LOGE("Cannot get logical device list");
        return;
    }

    MRect logicalActiveArrayRegion;
    MFLOAT logicalFocalLength = 1;
    MFLOAT logicalFov = 0;

    auto pLogicalMetadataProvider = NSMetadataProviderManager::valueForByDeviceId(openId);
    if(pLogicalMetadataProvider)
    {
        auto logicalStaticMeta = pLogicalMetadataProvider->getMtkStaticCharacteristics();

        if(IMetadata::getEntry<MRect>(&logicalStaticMeta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, logicalActiveArrayRegion) &&
           IMetadata::getEntry<MFLOAT>(&logicalStaticMeta, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, logicalFocalLength))
        {
            logicalFov = logicalActiveArrayRegion.s.w / logicalFocalLength;
            MY_LOGD("Logical Sensor %d Fov: %.2f, LENS_INFO_AVAILABLE_FOCAL_LENGTHS: %.2f, SENSOR_INFO_ACTIVE_ARRAY_REGION: (%d, %d) %dx%d",
                    openId, logicalFov, logicalFocalLength,
                    logicalActiveArrayRegion.p.x, logicalActiveArrayRegion.p.y,
                    logicalActiveArrayRegion.s.w, logicalActiveArrayRegion.s.h);
        }
    }

    vector<MINT32> sensorIDs = pHalDeviceList->getSensorId(openId);
    for(auto &sensorId : sensorIDs)
    {
        auto pMetadataProvider = NSMetadataProviderManager::valueFor(sensorId);
        if(pMetadataProvider)
        {
            auto staticMeta = pMetadataProvider->getMtkStaticCharacteristics();

            MFLOAT focalLength = 0;
            if(IMetadata::getEntry<MFLOAT>(&staticMeta, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, focalLength))
            {
                MY_LOGD("Sensor %d LENS_INFO_AVAILABLE_FOCAL_LENGTHS: %.2f", sensorId, focalLength);
                __focalLengthMap[sensorId] = focalLength;
            }
            else
            {
                MY_LOGE("Cannot get LENS_INFO_AVAILABLE_FOCAL_LENGTHS of sensor %d", sensorId);
            }

            MRect activeArrayRegion;
            if(IMetadata::getEntry<MRect>(&staticMeta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArrayRegion))
            {
                // if physical sensor larger then logical sensor fov,
                // scale down phyiscal fov as same as logical sensor
                //MFLOAT physicalDiagonal = sqrt(physicalWidth*physicalWidth+physicalHeight*physicalHeight);
                //auto fov = 2.0f*StereoHAL::radiansToDegree(atan(physicalDiagonal/(2.0f*focalLength)));
                auto fov = activeArrayRegion.s.w / focalLength;
                if(fov > logicalFov)
                {
                    MY_LOGD("Sensor %d FOV > logical, scale down!", sensorId);
                    activeArrayRegion.p.x = (logicalActiveArrayRegion.s.w - activeArrayRegion.s.w) / 2;
                    activeArrayRegion.p.y = (logicalActiveArrayRegion.s.h - activeArrayRegion.s.h) / 2;

                    MFLOAT rate = focalLength / logicalFocalLength;
                    activeArrayRegion.s.w = logicalActiveArrayRegion.s.w * rate;
                    activeArrayRegion.s.h = logicalActiveArrayRegion.s.h * rate;
                }

                MY_LOGD("Sensor %d SENSOR_INFO_ACTIVE_ARRAY_REGION: (%d, %d) %dx%d", sensorId,
                        activeArrayRegion.p.x, activeArrayRegion.p.y,
                        activeArrayRegion.s.w, activeArrayRegion.s.h);
                __activeArrayRegionMap[sensorId] = std::move(activeArrayRegion);
            }
            else
            {
                MY_LOGE("Cannot get SENSOR_INFO_ACTIVE_ARRAY_REGION of sensor %d", sensorId);
            }

            //Check RAW capability
            IMetadata::IEntry availableCapabilities = staticMeta.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
            if(IMetadata::IEntry::indexOf(availableCapabilities, (MUINT8)MTK_REQUEST_AVAILABLE_CAPABILITIES_RAW) != -1) {
                __postRawSensitivityBoostMap[sensorId] = 1;
            }
        }
        else
        {
            MY_LOGE("Cannot get metadata provider of sensor %d", sensorId);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
PhysicalMetaSyncDataUpdater::
update(
    SyncData &data
)
{
    for(auto&& item : data.mvOutputParams) {
        //Update SCALER_CROP_REGION
        {
            auto iter = __activeArrayRegionMap.find(item.first);
            if(iter != __activeArrayRegionMap.end())
            {
                IMetadata::IEntry scalerCropR(MTK_SCALER_CROP_REGION);
                scalerCropR.push_back(iter->second, Type2Type< MRect >());
                item.second->AppDynamic->update(MTK_SCALER_CROP_REGION, scalerCropR);
                // MY_LOGD("Update sensor %d SCALER_CROP_REGION: (%d, %d) %dx%d", item.first,
                //         iter->second.p.x, iter->second.p.y,
                //         iter->second.s.w, iter->second.s.h);
            }
        }

        //Update FOCAL_LENGTH
        {
            auto iter = __focalLengthMap.find(item.first);
            if(iter != __focalLengthMap.end())
            {
                IMetadata::IEntry focalLengthEntry(MTK_LENS_FOCAL_LENGTH);
                focalLengthEntry.push_back(iter->second, Type2Type< MFLOAT >());
                item.second->AppDynamic->update(MTK_LENS_FOCAL_LENGTH, focalLengthEntry);
                MY_LOGD("Update sensor %d LENS_FOCAL_LENGTH: %.2f", item.first, iter->second);
            }
        }

        //Update POST_RAW_SENSITIVITY_BOOST
        {
            auto iter = __postRawSensitivityBoostMap.find(item.first);
            if(iter != __postRawSensitivityBoostMap.end())
            {
                auto postRawSensitivityBoost = item.second->AppDynamic->entryFor(MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST);
                if ( postRawSensitivityBoost.isEmpty() ) {
                    int defaultValue = 100;
                    IMetadata::IEntry entry(MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST);
                    entry.push_back(defaultValue, Type2Type<MINT32>());
                    item.second->AppDynamic->update(entry.tag(), entry);
                    // MY_LOGD("Update sensor %d MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST: %d", item.first, defaultValue);
                }
            }
        }

        //Update videoStabilizationMode
        {
            auto videoStabilizationMode = item.second->AppDynamic->entryFor(MTK_CONTROL_VIDEO_STABILIZATION_MODE);
            if ( videoStabilizationMode.isEmpty() ) {
                bool defaultValue = false;
                IMetadata::IEntry entry(MTK_CONTROL_VIDEO_STABILIZATION_MODE);
                entry.push_back(defaultValue, Type2Type<MUINT8>());
                item.second->AppDynamic->update(entry.tag(), entry);
                // MY_LOGD("Update sensor %d MTK_CONTROL_VIDEO_STABILIZATION_MODE: %d", item.first, defaultValue);
            }
        }
    }

    return true;
}
