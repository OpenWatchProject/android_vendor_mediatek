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

#define LOG_TAG "mtkcam-aaa"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/IHalSensor.h>
//
#include <private/aaa_utils.h>
#include <map>
//
#include <camera_custom_imgsensor_cfg.h>
//
using namespace NSCam;
using namespace NS3Av3;
using namespace NSIspTuning;

/******************************************************************************
*
*******************************************************************************/
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


/******************************************************************************
 *
 ******************************************************************************/
namespace NS3Av3
{

struct SENSOR_IDX_INFO_T
{
    MUINT32 devId;
    MUINT32 sensorId;
    MUINT32 facing;
    MUINT32 facingNum;
};

#define FACING_MAX (10)
static std::once_flag flag;
static std::map<int, SENSOR_IDX_INFO_T> gSensorMap;
static void makeSensorMap()
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (pHalSensorList)
    {
        call_once(flag, [&] {
                auto cnt = pHalSensorList->queryNumberOfSensors();
                MUINT32 facingCnt[FACING_MAX] = { 0 };
                for(auto i = 0; i < cnt; ++i)
                {
                    auto devId = pHalSensorList->querySensorDevIdx(i);
                    SensorStaticInfo rSensorStaticInfo;
                    pHalSensorList->querySensorStaticInfo(devId, &rSensorStaticInfo);
                    auto sensorId = rSensorStaticInfo.sensorDevID;
                    auto facing = rSensorStaticInfo.facingDirection;
                    auto facingNum = facingCnt[facing];
                    gSensorMap[i] = { devId, sensorId, facing, facingNum };
                    facingCnt[facing] ++;
                    CAM_LOGI("[%s] idx(%d): dev(%d), sensorId(%d), facing(%d), facingNum(%d)", __FUNCTION__, i, devId, sensorId, facing, facingNum);
                }
        } );

    } else
        CAM_LOGE("[%s] MAKE_HalSensorList fail !");
}

MINT32 mapSensorIdxToDev(MINT32 idx)
{
    makeSensorMap();

    auto iter = gSensorMap.find(idx);
    if (iter != gSensorMap.end())
        return (iter->second).devId;

    CAM_LOGE("[%s] Unsupport sensor index: %d", __FUNCTION__, idx);
    return -1;
}

MINT32 mapSensorDevToIdx(MINT32 devId)
{
    makeSensorMap();

    for(auto iter = gSensorMap.begin(); iter != gSensorMap.end(); ++iter)
        if((iter->second).devId == devId)
            return iter->first;

    CAM_LOGE("[%s] Unsupport sensor device ID: %d", __FUNCTION__, devId);
    return -1;
}

MINT32 mapSensorIdxToSensorId(MINT32 idx)
{
    makeSensorMap();

    auto iter = gSensorMap.find(idx);
    if (iter != gSensorMap.end())
        return (iter->second).sensorId;

    CAM_LOGE("[%s] Unsupport sensor index: %d", __FUNCTION__, idx);
    return -1;
}

MINT32 mapSensorDevToSensorId(MINT32 devId)
{
    makeSensorMap();

    for(auto iter = gSensorMap.begin(); iter != gSensorMap.end(); ++iter)
        if((iter->second).devId == devId)
            return (iter->second).sensorId;

    CAM_LOGE("[%s] Unsupport sensor device ID: %d", __FUNCTION__, devId);
    return -1;
}

MINT32 mapSensorIdxToFace(MINT32 idx, MINT32& facing, MINT32& num)
{
    makeSensorMap();

    auto iter = gSensorMap.find(idx);
    if (iter != gSensorMap.end()) {
        facing = (iter->second).facing;
        num = (iter->second).facingNum;
        return 0;
    }

    CAM_LOGE("[%s] Unsupport sensor index: %d", __FUNCTION__, idx);
    return -1;
}

MINT32 mapSensorDevToFace(MINT32 devId, MINT32& facing, MINT32& num)
{
    makeSensorMap();

    for(auto iter = gSensorMap.begin(); iter != gSensorMap.end(); ++iter) {
        if((iter->second).devId == devId) {
            facing = (iter->second).facing;
            num = (iter->second).facingNum;
            return 0;
        }
    }

    CAM_LOGE("[%s] Unsupport sensor device ID: %d", __FUNCTION__, devId);
    return -1;
}
};  //namespace