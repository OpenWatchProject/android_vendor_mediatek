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

#define LOG_TAG "MtkCam/HalSensorList"

#include "MyUtils.h"
#include "HalSensorList.h"
#include <mtkcam/utils/std/ULog.h>

#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif

/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#endif

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

/******************************************************************************
 *
 ******************************************************************************/
IHalSensorList*
IHalSensorList::
get()
{
    return HalSensorList::singleton();
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList*
HalSensorList::
singleton()
{
    static HalSensorList inst;
    return &inst;
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::
HalSensorList()
    : IHalSensorList()

    , mEnumSensorMutex()
    , mEnumSensorList()
    , mOpenSensorMutex()
    , mOpenSensorList()
{
#ifdef CONFIG_MTK_CAM_SECURE
        mEnableSecure = 0;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT
HalSensorList::
searchSensors()
{
    Mutex::Autolock _l(mEnumSensorMutex);

    MY_LOGD("searchSensors");
    return  enumerateSensor_Locked();
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT
HalSensorList::
queryNumberOfSensors() const
{
    Mutex::Autolock _l(mEnumSensorMutex);

    return  mEnumSensorList.size();
}

/******************************************************************************
 *
 ******************************************************************************/
IMetadata const&
HalSensorList::
queryStaticInfo(MUINT const index) const
{
    EnumInfo const* pInfo = queryEnumInfoByIndex(index);
    MY_LOGF_IF(pInfo==NULL, "NULL EnumInfo for sensor %d", index);

    return  pInfo->mMetadata;
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
HalSensorList::
queryDriverName(MUINT const index) const
{
    EnumInfo const* pInfo = queryEnumInfoByIndex(index);
    MY_LOGF_IF(pInfo==NULL, "NULL EnumInfo for sensor %d", index);

    return  pInfo->getSensorDrvName();
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT
HalSensorList::
queryType(MUINT const index) const
{
    EnumInfo const* pInfo = queryEnumInfoByIndex(index);
    MY_LOGF_IF(pInfo==NULL, "NULL EnumInfo for sensor %d", index);

    return  pInfo->getSensorType();
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT
HalSensorList::
queryFacingDirection(MUINT const index) const
{
    if(SensorStaticInfo const* p = querySensorStaticInfo(index)) {
        return  p->facingDirection;
    }
    return  0;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT
HalSensorList::
querySensorDevIdx(MUINT const index) const
{
    return IMGSENSOR_SENSOR_IDX2DUAL((MUINT)queryEnumInfoByIndex(index)->getDeviceId());
}

/******************************************************************************
 *
 ******************************************************************************/
static
SensorStaticInfo const*
gQuerySensorStaticInfo(IMGSENSOR_SENSOR_IDX sensorIDX)
{
    if(sensorIDX >= IMGSENSOR_SENSOR_IDX_MIN_NUM && sensorIDX < IMGSENSOR_SENSOR_IDX_MAX_NUM) {
        return &sensorStaticInfo[sensorIDX];
    } else {
        MY_LOGE("bad sensorDev:%#x", sensorIDX);
        return  NULL;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensorList::
querySensorStaticInfo(MUINT sensorDev, SensorStaticInfo *pSensorStaticInfo) const
{
    Mutex::Autolock _l(mEnumSensorMutex);

    ::memcpy(pSensorStaticInfo, gQuerySensorStaticInfo(IMGSENSOR_SENSOR_IDX_MAP(sensorDev)), sizeof(SensorStaticInfo));
}

/******************************************************************************
 *
 ******************************************************************************/
SensorStaticInfo const*
HalSensorList::
querySensorStaticInfo(MUINT const index) const
{
    EnumInfo const* pEnumInfo = queryEnumInfoByIndex(index);
    if(!pEnumInfo) {
        MY_LOGE("No EnumInfo for index:%d", index);
        return  NULL;
    }

    Mutex::Autolock _l(mEnumSensorMutex);

    return gQuerySensorStaticInfo((IMGSENSOR_SENSOR_IDX)pEnumInfo->getDeviceId());
}

#ifdef CONFIG_MTK_CAM_SECURE
MUINT
HalSensorList::
enableSecure(char const *szCallerName)
{
    Mutex::Autolock _l(mSecureSensorMutex);
    mEnableSecure = 1;
    MY_LOGD("enableSecure, caller = %s",szCallerName);
    return  mEnableSecure;
}

MUINT
HalSensorList::
disableSecure(char const *szCallerName)
{
    Mutex::Autolock _l(mSecureSensorMutex);
    mEnableSecure = 0;
    MY_LOGD("disableSecure, caller = %s",szCallerName);
    return  mEnableSecure;
}

MUINT
HalSensorList::
querySecureState()
{
    Mutex::Autolock _l(mSecureSensorMutex);
    MY_LOGD("Secure State is %d", mEnableSecure);
    return mEnableSecure;
}
#endif

