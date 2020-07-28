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

#define LOG_TAG "MtkCam/Util/VirtualDeivceIdsMapHelper"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_UTILITY
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);

#include <sstream>  //For ostringstream
#include "MyUtils.h"
#include "VirtualDeviceIdsMapHelper.h"
#include <mtkcam/utils/json/json.hpp>
#if (3 == MTKCAM_HAL_VERSION)
#include <mtkcam3/3rdparty/customer/customer_virtual_device_id_map.h>
#elif (1 == MTKCAM_HAL_VERSION)
#include "custom/customer_virtual_device_id_map.h"
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <mtkcam/drv/IHalSensor.h>
#pragma GCC diagnostic pop

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace android;
using namespace NSCam::NSVirtualDeviceIdsMapHelper;
using json = nlohmann::json;

#define VID_MAPS_TAG "VID_MAPS"
#define SENSOR_DRVNAME_PREFIX std::string("SENSOR_DRVNAME_")
#define DELIMITER std::string(":")
#define VID_NAME_TAG "Name"
#define VID_ID_TAG "Id"
#define VID_Hidden_TAG "Hidden"
/******************************************************************************
 *
 ******************************************************************************/

VirtualDeviceIdsMapHelper::
VirtualDeviceIdsMapHelper()
{
    MY_LOGD("load vids map");
    // check data is vaild or not.
    json loadedJson = json::parse(DEFAULT_VIRTUAL_DEVICE_IDS_MAP);

    if(loadedJson.type() == nlohmann::detail::value_t::discarded)
    {
        mSupportCustomized = false;
    }
    else
    {
        auto hasMember = [&loadedJson](const char *key)
        {
            return (loadedJson.find(key) != loadedJson.end());
        };
        if(hasMember(VID_MAPS_TAG))
        {
            //Construct sensor list
            NSCam::IHalSensorList *pSensorList = MAKE_HalSensorList();
            if (NULL == pSensorList) {
                MY_LOGE("Cannot get sensor list");
                return;
            }

            int32_t sensorCount = pSensorList->queryNumberOfSensors();
            for(int index = 0; index < sensorCount; ++index) {
                std::string s(pSensorList->queryDriverName(index));
                size_t prefixPos = s.find(SENSOR_DRVNAME_PREFIX);
                if(prefixPos != std::string::npos) {
                    s.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
                }

                std::transform(s.begin(), s.end(), s.begin(), ::toupper);
                auto iter = mSensorNames.find(s);
                if(iter == mSensorNames.end())
                {
                    std::vector<int32_t> v;
                    v.push_back(index);
                    mSensorNames.emplace(s, v);
                }
                else
                {
                    iter->second.push_back(index);
                }
            }

            auto vid_maps_json = loadedJson[VID_MAPS_TAG];
            for(auto& item : vid_maps_json)
            {
                //Remove unnecessary delimiter for un-dup sensors
                std::string sensorName = item[VID_NAME_TAG].get<std::string>();
                size_t prefixPos = sensorName.find(SENSOR_DRVNAME_PREFIX);
                if(prefixPos != std::string::npos) {
                    sensorName.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
                }

                size_t deliPos = sensorName.find(DELIMITER);
                int customizeId = -1;
                if(deliPos != std::string::npos) {
                    auto idString = sensorName.substr(deliPos+DELIMITER.length());
                    customizeId = std::stoi(idString);
                    sensorName = sensorName.substr(0, deliPos);
                }

                std::transform(sensorName.begin(), sensorName.end(), sensorName.begin(), ::toupper);
                auto nameIt = mSensorNames.find(sensorName);
                bool hasError = false;
                if(nameIt != mSensorNames.end())
                {
                    auto &ids = nameIt->second;
                    if(ids.size() > 1) {
                        auto idIt = std::find(ids.begin(), ids.end(), customizeId);
                        if(idIt != ids.end())
                        {
                            sensorName = sensorName + DELIMITER + std::to_string(customizeId);
                        }
                        else
                        {
                            hasError = true;
                            std::ostringstream oss;
                            oss << " Need to append sensor ID to sensor name: \"" << sensorName << "\"" << std::endl
                                << " Since there are " << ids.size() << " " << sensorName
                                << " installed, you need to append sensor ID to sensor name, available settings:" << std::endl;
                            for(auto &id : ids) {
                                oss << "    \"" << sensorName << ":" << id << "\"" << std::endl;
                            }
                            MY_LOGI("%s", oss.str().c_str());
                        }
                    }
                }

                if(!hasError) {
                    int32_t id = item[VID_ID_TAG].get<int32_t>();
                    bool hidden = item[VID_Hidden_TAG].get<bool>();
                    mVirtualDeviceCusMap.emplace(sensorName, VirtualDeviceCusStruct(id, hidden));
                }
            }

            // dump
            for(auto&& item : mVirtualDeviceCusMap)
            {
                auto cusStruct = item.second;
                MY_LOGD("Customized : name(%s) vid(%u) isHidden(%u)", item.first.c_str(), cusStruct.getId(), cusStruct.isHidden());
            }
            mSupportCustomized = true;
        }
        else
        {
            mSupportCustomized = false;
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
VirtualDeviceIdsMapHelper::
~VirtualDeviceIdsMapHelper()
{
}
/******************************************************************************
 *
 ******************************************************************************/
bool
VirtualDeviceIdsMapHelper::
queryVID(
    std::string deviceName,
    int32_t sensorId,
    uint32_t &vid
)
{
    if(!mSupportCustomized) {
        return false;
    }
    bool ret = false;
    size_t prefixPos = deviceName.find(SENSOR_DRVNAME_PREFIX);
    if(prefixPos != std::string::npos) {
       deviceName.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
    }
    auto nameIt = mSensorNames.find(deviceName);
    if(nameIt != mSensorNames.end())
    {
        std::string queryName = deviceName;
        auto ids = nameIt->second;
        if(ids.size() > 1)
        {
            queryName = queryName + DELIMITER + std::to_string(sensorId);
        }

        auto iter = mVirtualDeviceCusMap.find(queryName);
        if(iter != mVirtualDeviceCusMap.end())
        {
            vid = iter->second.getId();
            ret = true;
            MY_LOGD("Remap %s from %d to %d", deviceName.c_str(), sensorId, vid);
        }
        else
        {
            iter = mVirtualDeviceCusMap.find(deviceName);
            if(iter != mVirtualDeviceCusMap.end() &&
               ids.size() > 1)
            {
                std::ostringstream oss;
                oss << " Need to append sensor ID to sensor name: \"" << deviceName << "\"" << std::endl
                    << " Since there are " << ids.size() << " " << deviceName
                    << " installed, you need to append sensor ID to sensor name, available settings:" << std::endl;
                for(auto &id : ids) {
                    oss << "    \"" << deviceName << ":" << id << "\"" << std::endl;
                }
                MY_LOGI("%s", oss.str().c_str());
            }
        }
    }
    return ret;
}

bool
VirtualDeviceIdsMapHelper::
queryHidden(
    std::string deviceName
)
{
    if(!mSupportCustomized) {
        return false;
    }
    size_t prefixPos = deviceName.find(SENSOR_DRVNAME_PREFIX);
    if(prefixPos != std::string::npos) {
       deviceName.erase(prefixPos, SENSOR_DRVNAME_PREFIX.length());
    }

    auto iter = mVirtualDeviceCusMap.find(deviceName);
    if(iter != mVirtualDeviceCusMap.end())
    {
        return iter->second.isHidden();
    }

    return false;
}
