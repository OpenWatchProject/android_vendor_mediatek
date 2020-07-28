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

#include <dlfcn.h>
#include <cutils/atomic.h>
// For property_get().
#include <cutils/properties.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/std/ULog.h>

#include "MyUtils.h"
#include "HalSensorList.h"

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
{
#ifdef CONFIG_MTK_CAM_SECURE
    mEnableSecure = 0;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
static
NSSensorType::Type
mapToSensorType(const IMAGE_SENSOR_TYPE sensor_type)
{
    NSSensorType::Type eSensorType;

    switch (sensor_type) {
    case IMAGE_SENSOR_TYPE_RAW:
    case IMAGE_SENSOR_TYPE_RAW8:
    case IMAGE_SENSOR_TYPE_RAW12:
    case IMAGE_SENSOR_TYPE_RAW14:
        eSensorType = NSSensorType::eRAW;
        break;

    case IMAGE_SENSOR_TYPE_YUV:
    case IMAGE_SENSOR_TYPE_YCBCR:
    case IMAGE_SENSOR_TYPE_RGB565:
    case IMAGE_SENSOR_TYPE_RGB888:
    case IMAGE_SENSOR_TYPE_JPEG:
        eSensorType = NSSensorType::eYUV;
        break;

    default:
        eSensorType = NSSensorType::eRAW;
        break;
    }

    return  eSensorType;
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

    SeninfDrv *const pSeninfDrv = SeninfDrv::getInstance();

    if(pSeninfDrv && pSeninfDrv->init() < 0) {
        MY_LOGE("pSeninfDrv->init() fail");
        return 0;
    }

    MUINT max_index_of_camera = IMGSENSOR_SENSOR_IDX_SUB;
#ifdef MTK_CAM_MAX_NUMBER_OF_CAMERA
    max_index_of_camera = MTK_CAM_MAX_NUMBER_OF_CAMERA - 1;
#endif
    MY_LOGD("impSearchSensor search to %d\n", max_index_of_camera);
    for (MUINT i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i <= max_index_of_camera; i++) {


        ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance((IMGSENSOR_SENSOR_IDX)i);
        if(pSensorDrv->init((IMGSENSOR_SENSOR_IDX)i) != SENSOR_NO_ERROR)
            continue;

        MUINT32 mclkSrc;
        pSensorDrv->sendCommand(CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&mclkSrc);
#ifdef MCLK_DRIVE_CURRENT_BY_PINCTRL
        MUINT32 current = ISP_DRIVING_8MA;
        pSensorDrv->sendCommand(CMD_SENSOR_SET_DRIVE_CURRENT, (MUINTPTR)&current);
#else
        pSeninfDrv->setMclkIODrivingCurrent(mclkSrc, ISP_DRIVING_8MA);
#endif

        pSeninfDrv->setMclk(mclkSrc, true, 24);

        if(pSensorDrv->searchSensor() == SENSOR_NO_ERROR) {
            //query sensorinfo
            querySensorInfo((IMGSENSOR_SENSOR_IDX)i);
            //fill in metadata
            buildSensorMetadata((IMGSENSOR_SENSOR_IDX)i);
            addAndInitSensorEnumInfo_Locked(
                (IMGSENSOR_SENSOR_IDX)i,
                mapToSensorType(pSensorDrv->getType()),
                (char *)pSensorDrv->getName());
        }

        pSeninfDrv->setMclk(mclkSrc, false, 24);

        pSensorDrv->uninit();
    }

    if(pSeninfDrv->uninit() < 0) {
        MY_LOGE("pSeninfDrv->uninit() fail");
        return 0;
    }

    return  mEnumSensorList.size();
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

    return pInfo->getSensorDrvName();
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

    return pInfo->getSensorType();
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
    const EnumInfo *pEnumInfo = queryEnumInfoByIndex(index);
    return (pEnumInfo) ? IMGSENSOR_SENSOR_IDX2DUAL(pEnumInfo->getDeviceId()) : 0;
}


/******************************************************************************
 *
 ******************************************************************************/
SensorStaticInfo const*
HalSensorList::
gQuerySensorStaticInfo(IMGSENSOR_SENSOR_IDX sensorIDX) const
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
querySensorStaticInfo(MUINT indexDual, SensorStaticInfo *pSensorStaticInfo) const
{
    Mutex::Autolock _l(mEnumSensorMutex);
    ::memcpy(pSensorStaticInfo, gQuerySensorStaticInfo(IMGSENSOR_SENSOR_IDX_MAP(indexDual)), sizeof(SensorStaticInfo));
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

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::
OpenInfo::
OpenInfo(
    MINT       iRefCount,
    HalSensor *pHalSensor
)
    : miRefCount(iRefCount)
    , mpHalSensor(pHalSensor)
{
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensorList::
closeSensor(
    HalSensor  *pHalSensor,
    char const *szCallerName)
{
    Mutex::Autolock _l(mOpenSensorMutex);

#ifdef DEBUG_SENSOR_OPEN_CLOSE
    MY_LOGD("caller =%s",szCallerName);
#endif

    OpenList_t::iterator it = mOpenSensorList.begin();
    for (; it != mOpenSensorList.end(); ++it)
    {
        if (pHalSensor == it->mpHalSensor)
        {
#ifdef DEBUG_SENSOR_OPEN_CLOSE
            MY_LOGD("closeSensor mpHalSensor : %p, pHalSensor = %p, refcnt= %d", it->mpHalSensor, pHalSensor, it->miRefCount);
#endif
            //  Last one reference ?
            if (1 == ::android_atomic_dec(&it->miRefCount))
            {
                MY_LOGD("<%s> last user", (szCallerName?szCallerName:"Unknown"));

                //  remove from open list.
                mOpenSensorList.erase(it);
                //  destroy and free this instance.
                pHalSensor->onDestroy();
                delete pHalSensor;
            }
            return;
        }
    }

    MY_LOGE("<%s> HalSensor:%p not exist", (szCallerName?szCallerName:"Unknown"), pHalSensor);
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensor*
HalSensorList::
openSensor(
    SortedVector<MUINT> const &vSensorIndex,
    char                const *szCallerName
)
{
    Mutex::Autolock _l(mOpenSensorMutex);

#ifdef DEBUG_SENSOR_OPEN_CLOSE
    MY_LOGD("caller =%s",szCallerName);
#endif

    OpenList_t::iterator it = mOpenSensorList.begin();
    for (; it != mOpenSensorList.end(); ++it)
    {
        if (it->mpHalSensor->isMatch(vSensorIndex))
        {
            //  The open list holds a created instance.
            //  just increment reference count and return the instance.
            ::android_atomic_inc(&it->miRefCount);
#ifdef DEBUG_SENSOR_OPEN_CLOSE
            MY_LOGD("openSensor mpHalSensor : %p,idx %d, %d, %d, refcnt %d",
                it->mpHalSensor, vSensorIndex[0], vSensorIndex[1], vSensorIndex[2], it->miRefCount);
#endif
            return it->mpHalSensor;
        }
    }
#ifdef DEBUG_SENSOR_OPEN_CLOSE
    MY_LOGD("new created vSensorIdx[0] = %d, vSensorIdx[1] = %d, vSensorIdx[2] = %d",vSensorIndex[0],vSensorIndex[1],vSensorIndex[2]);
#endif

    //  It does not exist in the open list.
    //  We must create a new one and add it to open list.
    HalSensor* pHalSensor = NULL;

    pHalSensor = new HalSensor();

    if (NULL != pHalSensor)
    {
        //  onCreate callback
        if (!pHalSensor->onCreate(vSensorIndex))
        {
            MY_LOGE("HalSensor::onCreate");
            delete pHalSensor;
            return NULL;
        }

        //  push into open list (with ref. count = 1).
        mOpenSensorList.push_back(OpenInfo(1, pHalSensor));

        MY_LOGD("<%s> 1st user", (szCallerName?szCallerName:"Unknown"));
        return pHalSensor;
    }

    MY_LOGF("<%s> Never Be Here...No memory ?", (szCallerName?szCallerName:"Unknown"));
    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
IHalSensor*
HalSensorList::
createSensor(
    char  const *szCallerName,
    MUINT const  index
)
{
    Mutex::Autolock _l(mEnumSensorMutex);
    SortedVector<MUINT> vSensorIndex;
    vSensorIndex.add(index);
    return openSensor(vSensorIndex, szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
IHalSensor*
HalSensorList::
createSensor(
    char  const *szCallerName,
    MUINT const  uCountOfIndex,
    MUINT const *pArrayOfIndex
)
{
    Mutex::Autolock _l(mEnumSensorMutex);
    MY_LOGF_IF(0==uCountOfIndex||0==pArrayOfIndex, "<%s> Bad uCountOfIndex:%d pArrayOfIndex:%p", szCallerName, uCountOfIndex, pArrayOfIndex);

    SortedVector<MUINT> vSensorIndex;
    for (MUINT i = 0; i < uCountOfIndex; i++)
    {
        vSensorIndex.add(pArrayOfIndex[i]);
    }
    return openSensor(vSensorIndex, szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::EnumInfo const*
HalSensorList::
queryEnumInfoByIndex(MUINT index) const
{
    Mutex::Autolock _l(mEnumSensorMutex);

    if (index >= mEnumSensorList.size())
    {
        MY_LOGE("bad sensorIdx:%d >= size:%zu", index, mEnumSensorList.size());
        return  NULL;
    }

    return  &mEnumSensorList[index];
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::EnumInfo const*
HalSensorList::
addAndInitSensorEnumInfo_Locked(
    IMGSENSOR_SENSOR_IDX eSensorDev,
    MUINT                eSensorType,
    char                *szSensorDrvName
)
{
    mEnumSensorList.push_back(EnumInfo());

    EnumInfo& rEnumInfo = mEnumSensorList.editTop();
    String8 drvName;

    SensorStaticInfo     *pSensorStaticInfo = &sensorStaticInfo[eSensorDev];
    bool bBackSide = (pSensorStaticInfo->facingDirection==0)? true : false;

    drvName.setTo(szSensorDrvName);
    drvName.toUpper();
    rEnumInfo.setDeviceId(eSensorDev);
    rEnumInfo.setSensorType(eSensorType);
    rEnumInfo.setBackSide(bBackSide);
    rEnumInfo.setSensorDrvName(String8::format("SENSOR_DRVNAME_%s", drvName.string()));

    buildStaticInfo(rEnumInfo, rEnumInfo.mMetadata);

    return  &rEnumInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID HalSensorList::querySensorInfo(IMGSENSOR_SENSOR_IDX idx)
{
    MUINTPTR data1,data2;
    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(idx);

    SensorStaticInfo                   *pSensorStaticInfo = &sensorStaticInfo[idx];
    SENSORDRV_INFO_STRUCT              *pSensorDrvInfo  = pSensorDrv->getDrvInfo();
    ACDK_SENSOR_INFO_STRUCT            *pInfo           = &pSensorDrvInfo->info;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pResolutionInfo = &pSensorDrvInfo->resolutionInfo;

    pSensorStaticInfo->sensorDevID = pSensorDrv->getID();
    pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE, (MUINTPTR)&data1);
    pSensorStaticInfo->orientationAngle = data1;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_FACING_DIRECTION, (MUINTPTR)&data1);
    pSensorStaticInfo->facingDirection = data1;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_VIEWANGLE, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->horizontalViewAngle =data1;
    pSensorStaticInfo->verticalViewAngle = data2;

    data1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->previewFrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->captureFrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->videoFrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->video1FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->video2FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM1;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom1FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM2;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom2FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM3;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom3FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM4;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom4FrameRate = data2;
    data1 = SENSOR_SCENARIO_ID_CUSTOM5;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
    pSensorStaticInfo->custom5FrameRate = data2;

    switch (pSensorDrv->getType()) {
        case IMAGE_SENSOR_TYPE_RAW:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_10BIT;
            break;
        case IMAGE_SENSOR_TYPE_RAW8:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_8BIT;
            break;

        case IMAGE_SENSOR_TYPE_RAW12:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_12BIT;
            break;

        case IMAGE_SENSOR_TYPE_RAW14:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RAW;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_14BIT;
            break;
        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_YUV;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
        case IMAGE_SENSOR_TYPE_RGB565:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_RGB;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
        case IMAGE_SENSOR_TYPE_JPEG:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_JPEG;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
        default:
            pSensorStaticInfo->sensorType = SENSOR_TYPE_UNKNOWN;
            pSensorStaticInfo->rawSensorBit = RAW_SENSOR_ERROR;
            break;
    }

    switch (pInfo->SensorOutputDataFormat) {
        case SENSOR_OUTPUT_FORMAT_RAW_B:
        case SENSOR_OUTPUT_FORMAT_RAW8_B:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_Gb:
        case SENSOR_OUTPUT_FORMAT_RAW8_Gb:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_Gr:
        case SENSOR_OUTPUT_FORMAT_RAW8_Gr:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_R:
        case SENSOR_OUTPUT_FORMAT_RAW8_R:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
        case SENSOR_OUTPUT_FORMAT_UYVY:
        case SENSOR_OUTPUT_FORMAT_CbYCrY:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_UYVY;
            break;
        case SENSOR_OUTPUT_FORMAT_VYUY:
        case SENSOR_OUTPUT_FORMAT_CrYCbY:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_VYUY;
            break;
        case SENSOR_OUTPUT_FORMAT_YUYV:
        case SENSOR_OUTPUT_FORMAT_YCbYCr:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_YUYV;
            break;
        case SENSOR_OUTPUT_FORMAT_YVYU:
        case SENSOR_OUTPUT_FORMAT_YCrYCb:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_YVYU;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_RWB_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_RWB_Wb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_RWB_Wr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_RWB_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_RWB;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_MONO:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_MONO;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_Gb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_Gr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_HW_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_Gb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_HW_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_Gr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_HW_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_HW_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_B:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gb:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gr:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
            break;
        case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_R:
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_4CELL_BAYER;
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
            break;
        default:
            pSensorStaticInfo->sensorFormatOrder = SENSOR_FORMAT_ORDER_NONE;
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_FMT_NONE;
            break;
    }

    /// Add SENSOR_RAW_PD for 3A use to distinguish whether enable pdc table or not
    /// Note: Only pdaf sensor rather than dual pd need to enable pdc table
    /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(Full), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
    switch(pInfo->PDAF_Support) {
        case 1: /* 1: PDAF Raw Data mode */
            pSensorStaticInfo->rawFmtType = SENSOR_RAW_PD;
            break;
        default:
            break;
    }

    pSensorStaticInfo->iHDRSupport = pInfo->IHDR_Support;
    pSensorStaticInfo->PDAF_Support = pInfo->PDAF_Support;
    pSensorStaticInfo->HDR_Support = pInfo->HDR_Support;
    pSensorStaticInfo->previewDelayFrame = pInfo->PreviewDelayFrame;
    pSensorStaticInfo->captureDelayFrame = pInfo->CaptureDelayFrame;
    pSensorStaticInfo->videoDelayFrame = pInfo->VideoDelayFrame;
    pSensorStaticInfo->video1DelayFrame = pInfo->HighSpeedVideoDelayFrame;
    pSensorStaticInfo->video2DelayFrame = pInfo->SlimVideoDelayFrame;
    pSensorStaticInfo->Custom1DelayFrame = pInfo->Custom1DelayFrame;
    pSensorStaticInfo->Custom2DelayFrame = pInfo->Custom2DelayFrame;
    pSensorStaticInfo->Custom3DelayFrame = pInfo->Custom3DelayFrame;
    pSensorStaticInfo->Custom4DelayFrame = pInfo->Custom4DelayFrame;
    pSensorStaticInfo->Custom5DelayFrame = pInfo->Custom5DelayFrame;
    pSensorStaticInfo->aeShutDelayFrame = pInfo->AEShutDelayFrame;
    pSensorStaticInfo->aeSensorGainDelayFrame = pInfo->AESensorGainDelayFrame;
    pSensorStaticInfo->aeISPGainDelayFrame = pInfo->AEISPGainDelayFrame;
    pSensorStaticInfo->FrameTimeDelayFrame = pInfo->FrameTimeDelayFrame;
    pSensorStaticInfo->SensorGrabStartX_PRV = pInfo->SensorGrabStartX_PRV;
    pSensorStaticInfo->SensorGrabStartY_PRV = pInfo->SensorGrabStartY_PRV;
    pSensorStaticInfo->SensorGrabStartX_CAP = pInfo->SensorGrabStartX_CAP;
    pSensorStaticInfo->SensorGrabStartY_CAP = pInfo->SensorGrabStartY_CAP;
    pSensorStaticInfo->SensorGrabStartX_VD = pInfo->SensorGrabStartX_VD;
    pSensorStaticInfo->SensorGrabStartY_VD = pInfo->SensorGrabStartY_VD;
    pSensorStaticInfo->SensorGrabStartX_VD1 = pInfo->SensorGrabStartX_VD1;
    pSensorStaticInfo->SensorGrabStartY_VD1 = pInfo->SensorGrabStartY_VD1;
    pSensorStaticInfo->SensorGrabStartX_VD2 = pInfo->SensorGrabStartX_VD2;
    pSensorStaticInfo->SensorGrabStartY_VD2 = pInfo->SensorGrabStartY_VD2;
    pSensorStaticInfo->SensorGrabStartX_CST1 = pInfo->SensorGrabStartX_CST1;
    pSensorStaticInfo->SensorGrabStartY_CST1 = pInfo->SensorGrabStartY_CST1;
    pSensorStaticInfo->SensorGrabStartX_CST2 = pInfo->SensorGrabStartX_CST2;
    pSensorStaticInfo->SensorGrabStartY_CST2 = pInfo->SensorGrabStartY_CST2;
    pSensorStaticInfo->SensorGrabStartX_CST3 = pInfo->SensorGrabStartX_CST3;
    pSensorStaticInfo->SensorGrabStartY_CST3 = pInfo->SensorGrabStartY_CST3;
    pSensorStaticInfo->SensorGrabStartX_CST4 = pInfo->SensorGrabStartX_CST4;
    pSensorStaticInfo->SensorGrabStartY_CST4 = pInfo->SensorGrabStartY_CST4;
    pSensorStaticInfo->SensorGrabStartX_CST5 = pInfo->SensorGrabStartX_CST5;
    pSensorStaticInfo->SensorGrabStartY_CST5 = pInfo->SensorGrabStartY_CST5;
    pSensorStaticInfo->iHDR_First_IS_LE = pInfo->IHDR_LE_FirstLine;
    pSensorStaticInfo->SensorModeNum = pInfo->SensorModeNum;
    pSensorStaticInfo->PerFrameCTL_Support = pInfo->PerFrameCTL_Support;
    pSensorStaticInfo->ZHDR_MODE = pInfo->ZHDR_Mode;

    if(pInfo->SensorHorFOV != 0) /*from sensor driver*/
		pSensorStaticInfo->horizontalViewAngle = pInfo->SensorHorFOV;
	if(pInfo->SensorVerFOV != 0)
		pSensorStaticInfo->verticalViewAngle = pInfo->SensorVerFOV;
    if(pInfo->SensorOrientation != 0)
		pSensorStaticInfo->SensorOrientation = pInfo->SensorOrientation;

    pSensorStaticInfo->sensorModuleID = pInfo->SensorModuleID;
    /*MY_LOGD("fov idx %d, hor %d, ver = %d orientation %d",
              idx,
              *pSensorStaticInfo->horizontalViewAngle,
              *pSensorStaticInfo->verticalViewAngle,
              pSensorStaticInfo->SensorOrientation);
    */

    pSensorStaticInfo->previewWidth = pResolutionInfo->SensorPreviewWidth;
    pSensorStaticInfo->previewHeight = pResolutionInfo->SensorPreviewHeight;
    pSensorStaticInfo->captureWidth = pResolutionInfo->SensorFullWidth;
    pSensorStaticInfo->captureHeight = pResolutionInfo->SensorFullHeight;
    pSensorStaticInfo->videoWidth = pResolutionInfo->SensorVideoWidth;
    pSensorStaticInfo->videoHeight = pResolutionInfo->SensorVideoHeight;
    pSensorStaticInfo->video1Width = pResolutionInfo->SensorHighSpeedVideoWidth;
    pSensorStaticInfo->video1Height = pResolutionInfo->SensorHighSpeedVideoHeight;
    pSensorStaticInfo->video2Width = pResolutionInfo->SensorSlimVideoWidth;
    pSensorStaticInfo->video2Height = pResolutionInfo->SensorSlimVideoHeight;
    pSensorStaticInfo->SensorCustom1Width = pResolutionInfo->SensorCustom1Width;
    pSensorStaticInfo->SensorCustom1Height = pResolutionInfo->SensorCustom1Height;
    pSensorStaticInfo->SensorCustom2Width = pResolutionInfo->SensorCustom2Width;
    pSensorStaticInfo->SensorCustom2Height = pResolutionInfo->SensorCustom2Height;
    pSensorStaticInfo->SensorCustom3Width = pResolutionInfo->SensorCustom3Width;
    pSensorStaticInfo->SensorCustom3Height = pResolutionInfo->SensorCustom3Height;
    pSensorStaticInfo->SensorCustom4Width = pResolutionInfo->SensorCustom4Width;
    pSensorStaticInfo->SensorCustom4Height = pResolutionInfo->SensorCustom4Height;
    pSensorStaticInfo->SensorCustom5Width = pResolutionInfo->SensorCustom5Width;
    pSensorStaticInfo->SensorCustom5Height = pResolutionInfo->SensorCustom5Height;
    pSensorStaticInfo->previewActiveWidth = pResolutionInfo->SensorEffectivePreviewWidth;//3d use onlyl?
    pSensorStaticInfo->previewActiveHeight = pResolutionInfo->SensorEffectivePreviewHeight;//3d use onlyl?
    pSensorStaticInfo->captureActiveWidth = pResolutionInfo->SensorEffectiveFullWidth;//3d use onlyl?
    pSensorStaticInfo->captureActiveHeight = pResolutionInfo->SensorEffectiveFullHeight;//3d use onlyl?
    pSensorStaticInfo->videoActiveWidth = pResolutionInfo->SensorEffectiveVideoWidth;//3d use onlyl?
    pSensorStaticInfo->videowActiveHeight = pResolutionInfo->SensorEffectiveVideoHeight;//3d use onlyl?
    pSensorStaticInfo->previewHoizontalOutputOffset = pResolutionInfo->SensorPreviewWidthOffset;//3d use onlyl?
    pSensorStaticInfo->previewVerticalOutputOffset = pResolutionInfo->SensorPreviewHeightOffset; //3d use onlyl?
    pSensorStaticInfo->captureHoizontalOutputOffset = pResolutionInfo->SensorFullWidthOffset;//3d use onlyl?
    pSensorStaticInfo->captureVerticalOutputOffset = pResolutionInfo->SensorFullHeightOffset; //3d use onlyl?
    pSensorStaticInfo->videoHoizontalOutputOffset = pResolutionInfo->SensorVideoWidthOffset;//3d use onlyl?
    pSensorStaticInfo->videoVerticalOutputOffset = pResolutionInfo->SensorVideoHeightOffset; //3d use onlyl?
    pSensorStaticInfo->virtualChannelSupport = MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID HalSensorList::buildSensorMetadata(IMGSENSOR_SENSOR_IDX idx)
{
    MINT64 exposureTime1 = 0x4000;
    MINT64 exposureTime2 = 0x4000;
    MUINT8 u8Para = 0;
    MINT32 s32Para = 0;

    MY_LOGD("impBuildSensorInfo start!");

    IMetadata metadataA;
    SensorStaticInfo *pSensorStaticInfo = &sensorStaticInfo[idx];

    {
        IMetadata::IEntry entryA(MTK_SENSOR_EXPOSURE_TIME);
        entryA.push_back(exposureTime1, Type2Type<MINT64>());
        entryA.push_back(exposureTime2, Type2Type<MINT64>());
        metadataA.update(MTK_SENSOR_EXPOSURE_TIME, entryA);
    }

    { //using full size?
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        MRect region1(MPoint(pSensorStaticInfo->captureHoizontalOutputOffset,pSensorStaticInfo->captureVerticalOutputOffset), MSize(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight));
        entryA.push_back(region1, Type2Type<MRect>());
        metadataA.update(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, entryA);
    }

    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT);
        switch(pSensorStaticInfo->sensorFormatOrder) {
            case SENSOR_FORMAT_ORDER_RAW_B:
                u8Para = 0x3;//BGGR
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gb:
                u8Para = 0x2;//GBRG
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gr:
                u8Para = 0x1;//GRBG
                break;
            case SENSOR_FORMAT_ORDER_RAW_R:
                u8Para = 0x0;//RGGB
                break;
            default:
                u8Para = 0x4;//BGR not bayer
                break;
        }
        entryA.push_back(u8Para, Type2Type<MUINT8>());
        metadataA.update(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE);
        entryA.push_back(3000, Type2Type<MINT64>());
        entryA.push_back(3000000000, Type2Type<MINT64>());
        metadataA.update(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_MAX_FRAME_DURATION);
        entryA.push_back(50000000000, Type2Type<MINT64>());
        metadataA.update(MTK_SENSOR_INFO_MAX_FRAME_DURATION, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PHYSICAL_SIZE);
        entryA.push_back(5.82, Type2Type<MFLOAT>());
        entryA.push_back(4.76, Type2Type<MFLOAT>());
        metadataA.update(MTK_SENSOR_INFO_PHYSICAL_SIZE, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE);
        MSize Size1(4000,3000);
        entryA.push_back(Size1, Type2Type<MSize>());
        metadataA.update(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, entryA);
    }

    {//need to add query from kernel
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_WHITE_LEVEL);
        switch (pSensorStaticInfo->rawSensorBit) {
            case RAW_SENSOR_8BIT:
                s32Para = 256;
                break;
            case RAW_SENSOR_10BIT:
                s32Para = 1024;
                break;
            case RAW_SENSOR_12BIT:
                s32Para = 4096;
                break;
            case RAW_SENSOR_14BIT:
                s32Para = 16384;
                break;
            default:
                s32Para = 256;
                break;
        }
        entryA.push_back(s32Para, Type2Type<MINT32>());
        metadataA.update(MTK_SENSOR_INFO_WHITE_LEVEL, entryA);
    }

    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ORIENTATION);
        entryA.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        metadataA.update(MTK_SENSOR_INFO_ORIENTATION, entryA);
    }

    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PACKAGE);
        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_PREVIEW,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->previewFrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->previewWidth,pSensorStaticInfo->previewHeight);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->previewWidth,pSensorStaticInfo->previewHeight));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_CAPTURE,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->captureFrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_VIDEO,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->videoFrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->videoWidth,pSensorStaticInfo->videoHeight);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->videoWidth,pSensorStaticInfo->videoHeight));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_SLIM_VIDEO1,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->video1FrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->video1Width,pSensorStaticInfo->video1Height);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->video1Width,pSensorStaticInfo->video1Height));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }

        {
            IMetadata metadataB;
            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                entryB.push_back((MINT32)SENSOR_SCENARIO_ID_SLIM_VIDEO2,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                entryB.push_back((MINT32)pSensorStaticInfo->video2FrameRate,Type2Type<MINT32>());
                metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                MSize size1(pSensorStaticInfo->video2Width,pSensorStaticInfo->video2Height);
                entryB.push_back(size1,Type2Type<MSize>());
                metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
            }

            {
                IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                MRect region1(MPoint(0,0), MSize(pSensorStaticInfo->video2Width,pSensorStaticInfo->video2Height));
                entryB.push_back(region1,Type2Type<MRect>());
                metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
            }

            entryA.push_back(metadataB,Type2Type<IMetadata>());
        }
        metadataA.update(MTK_SENSOR_INFO_PACKAGE, entryA);
    }
    metadataA.sort();

    MY_LOGD("impBuildSensorInfo end!");
}

/******************************************************************************
 *
 ******************************************************************************/
static
MBOOL
impConstructStaticMetadata_by_SymbolName(
    String8 const&  s8Symbol,
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
typedef MBOOL (*PFN_T)(
        IMetadata &         metadata,
        Info const&         info
    );

    PFN_T pfn;
    MBOOL ret = MTRUE;
    String8 const s8LibPath = String8::format("libcam.halsensor.so");
    void *handle = ::dlopen(s8LibPath.string(), RTLD_GLOBAL);
    if ( ! handle )
    {
        char const *err_str = ::dlerror();
        MY_LOGW("dlopen library=%s %s", s8LibPath.string(), err_str?err_str:"unknown");
        ret = MFALSE;
        goto lbExit;
    }
    pfn = (PFN_T)::dlsym(handle, s8Symbol.string());
    if  ( ! pfn ) {
        MY_LOGW("%s not found", s8Symbol.string());
        ret = MFALSE;
        goto lbExit;
    }

    ret = pfn(rMetadata, rInfo);
    MY_LOGW_IF(!ret, "%s fail", s8Symbol.string());

lbExit:
    if ( handle )
    {
        ::dlclose(handle);
        handle = NULL;
    }
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kStaticMetadataTypeNames[] = {
    "LENS",
    "SENSOR",
    "TUNING_3A",
    "FLASHLIGHT",
    "SCALER",
    "FEATURE",
    "CAMERA",
    "REQUEST",
    NULL
};

/******************************************************************************
 *
 ******************************************************************************/
static
MBOOL
impBuildStaticInfo(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];

        MBOOL status = MTRUE;

        String8 const s8Symbol_Sensor = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        String8 const s8Symbol_Common = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        MY_LOGE_IF(0, "Fail for both %s & %s", s8Symbol_Sensor.string(), s8Symbol_Common.string());
    }

    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];

        MBOOL status = MTRUE;

        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        MY_LOGE_IF(0, "Fail for both %s & %s", s8Symbol_Sensor.string(), s8Symbol_Common.string());
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kCommonStaticMetadataNames[] =
{
    // MODULE
    "CAMERA",
    "CONTROL_AE",
    "CONTROL_AF",
    "CONTROL_AWB",
    "TUNING",
    "FLASHLIGHT",
    "SENSOR",
    "LENS",
    // // PROJECT
    // "AVAILABLE_KEYS",
    // "FEATURE",
    // "MULTICAM",
    // "REQUEST",
    // "SCALER",
    // "VENDOR",
    NULL
};

/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kPlatformStaticMetadataNames[] =
{
    "MODULE",
    "LENS",
    // "MOD_OVERWRITE"
    // "PROJECT",
    // // "PRO_OVERWRITE"
    NULL
};


/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kPlatformOverwriteStaticMetadataNames[] =
{
    // "MODULE"
    // "LENS",
    "MOD_OVERWRITE",
    // // "PROJECT",
    // "PRO_OVERWRITE",
    NULL
};

enum {
    COMMON_COMMON       = 0x1,
    COMMON_SENSOR       = 0x1 << 1,
    PLATFORM_COMMON     = 0x1 << 2,
    PLATFORM_SENSOR     = 0x1 << 3,
    PROJECT_COMMON      = 0x1 << 4,
    PROJECT_SENSOR      = 0x1 << 5,
};

static
MBOOL
impBuildStaticInfo_v1(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    // step1. config common setting
    String8 strResult = String8::format("<load custom folder>\n\tSTATIC_COMMON: ");
    for (int i = 0; NULL != kCommonStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kCommonStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status==MTRUE ) loadResult|=COMMON_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status==MTRUE ) loadResult|=COMMON_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }

    //
    strResult += String8::format("\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
        if ( status_s!=MTRUE && status_d!=MTRUE ) {
            MY_LOGE("%s", strResult.string());
            return MFALSE;
        }
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    MY_LOGI("%s", strResult.string());
#if 1   // design for backward compatible
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];
        MBOOL status = MTRUE;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            MY_LOGW("project configuration exists!");
            return MFALSE;
        }
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            MY_LOGW("project configuration exists!");
            return MFALSE;
        }
    }
#endif
    return MTRUE;
}

static
MBOOL
impBuildStaticInfo_v1_overwrite(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    // step1. config common setting
    String8 strResult = String8::format("<load custom folder - overwrite>\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformOverwriteStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOverwriteStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformOverwriteStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOverwriteStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    MY_LOGI("%s", strResult.string());
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
static
void
updateAFData(IMetadata& rMetadata)
{
    auto eMinFocusDistance = rMetadata.entryFor(MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE);
    if ( eMinFocusDistance.isEmpty() ) {
        MY_LOGF("MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE must not empty");
    } else {
        MFLOAT value = eMinFocusDistance.itemAt(0, Type2Type<MFLOAT>() );
        if ( value != .0f ) {   // not fixed focus, update
            MY_LOGD("MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE: %f, add AF modes & regions", value);
            // MTK_CONTROL_AF_AVAILABLE_MODES
            IMetadata::IEntry afAvailableModes(MTK_CONTROL_AF_AVAILABLE_MODES);
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_OFF, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_AUTO, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_MACRO, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE, Type2Type<MUINT8>() );
            // afAvailableModes.push_back(MTK_CONTROL_AF_MODE_EDOF, Type2Type<MUINT8>() );
            rMetadata.update(afAvailableModes.tag(), afAvailableModes);

            // MTK_CONTROL_MAX_REGIONS
            IMetadata::IEntry maxRegions(MTK_CONTROL_MAX_REGIONS);
            maxRegions.push_back(1, Type2Type<MINT32>() );
            maxRegions.push_back(1, Type2Type<MINT32>() );
            maxRegions.push_back(1, Type2Type<MINT32>() );
            rMetadata.update(maxRegions.tag(), maxRegions);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensorList::
buildStaticInfo(Info const& rInfo, IMetadata& rMetadata) const
{
    const SensorStaticInfo *pSensorStaticInfo = &sensorStaticInfo[rInfo.getDeviceId()];

    MUINT8 u8Para = 0;

#if MTKCAM_CUSTOM_METADATA_COMMON
    bool v1 = true;
    if ( property_get_int32("vendor.debug.camera.static_meta.version", 1)!=0 ) {
        if ( !impBuildStaticInfo_v1(rInfo, rMetadata) ) {
            v1 = false;
            MY_LOGW( "V1: Fail to build static info for %s index:%d type:%d",
                     rInfo.getSensorDrvName().string(), rInfo.getDeviceId(), rInfo.getSensorType());
        } else {
            updateAFData(rMetadata);
            goto lbLoadDone;
        }
    }
#endif

    if (!impBuildStaticInfo(rInfo, rMetadata))
    {
        MY_LOGE(
            "Fail to build static info for %s index:%d",
            rInfo.getSensorDrvName().string(), rInfo.getDeviceId()
        );
        //return  MFALSE;
    }

lbLoadDone:
    /*METEDATA Ref  //system/media/camera/docs/docs.html*/
    //using full size
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        MRect region1(MPoint(pSensorStaticInfo->SensorGrabStartX_CAP,pSensorStaticInfo->SensorGrabStartY_CAP), MSize(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight));
        entryA.push_back(region1, Type2Type<MRect>());
        rMetadata.update(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, entryA);

        MY_LOGD("MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION(%d, %d, %d, %d)", pSensorStaticInfo->SensorGrabStartX_CAP, pSensorStaticInfo->SensorGrabStartY_CAP,
            pSensorStaticInfo->captureWidth, pSensorStaticInfo->captureHeight);
    }
    //using full size(No correction)
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE);
        entryA.push_back(pSensorStaticInfo->SensorGrabStartX_CAP, Type2Type<MINT32>());
        entryA.push_back(pSensorStaticInfo->SensorGrabStartY_CAP, Type2Type<MINT32>());
        entryA.push_back(pSensorStaticInfo->captureWidth, Type2Type<MINT32>());
        entryA.push_back(pSensorStaticInfo->captureHeight, Type2Type<MINT32>());
        rMetadata.update(MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE, entryA);

        MY_LOGD("MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE(%d, %d, %d, %d)", pSensorStaticInfo->SensorGrabStartX_CAP, pSensorStaticInfo->SensorGrabStartY_CAP,
            pSensorStaticInfo->captureWidth, pSensorStaticInfo->captureHeight);
    }
    //Pixel arry
    {
        ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance((IMGSENSOR_SENSOR_IDX) rInfo.getDeviceId());
        MUINT32 scenario = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG; /*capture mode*/
        SensorCropWinInfo rSensorCropInfo;

        ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));
        pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO,
                (MUINTPTR)&scenario,
                (MUINTPTR)&rSensorCropInfo,
                0);

        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE);
        MSize Size1(rSensorCropInfo.full_w, rSensorCropInfo.full_h);
        entryA.push_back(Size1, Type2Type<MSize>());
        rMetadata.update(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, entryA);
    }
    //Color filter
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT);
        switch(pSensorStaticInfo->sensorFormatOrder) {
            case SENSOR_FORMAT_ORDER_RAW_B:
                u8Para = 0x3;//BGGR
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gb:
                u8Para = 0x2;//GBRG
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gr:
                u8Para = 0x1;//GRBG
                break;
            case SENSOR_FORMAT_ORDER_RAW_R:
                u8Para = 0x0;//RGGB
                break;
            default:
                u8Para = 0x4;//BGR not bayer
                break;
        }
        entryA.push_back(u8Para, Type2Type<MUINT8>());
        rMetadata.update(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, entryA);
    }
    //Orientation
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ORIENTATION);
        entryA.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        rMetadata.update(MTK_SENSOR_INFO_ORIENTATION, entryA);

        // IMetadata::IEntry entryB(MTK_SENSOR_INFO_WANTED_ORIENTATION);
        // entryB.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        // rMetadata.update(MTK_SENSOR_INFO_WANTED_ORIENTATION, entryB);
    }

    //AOSP Orientation & Facing
    {
        IMetadata::IEntry entryA(MTK_SENSOR_ORIENTATION);
        entryA.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        rMetadata.update(MTK_SENSOR_ORIENTATION, entryA);

        IMetadata::IEntry entryB(MTK_LENS_FACING);
        if ( pSensorStaticInfo->facingDirection==0 )
            entryB.push_back(MTK_LENS_FACING_BACK, Type2Type<MUINT8>());
        else
            entryB.push_back(MTK_LENS_FACING_FRONT, Type2Type<MUINT8>());
        rMetadata.update(MTK_LENS_FACING, entryB);
    }

    //Sensor manual add tag list
    {
        IMetadata::IEntry entryA(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED);
        entryA.push_back((MINT64)MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_ORIENTATION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_WANTED_ORIENTATION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_ORIENTATION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_LENS_FACING, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_CONTROL_AF_AVAILABLE_MODES, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_CONTROL_MAX_REGIONS, Type2Type<MINT64>());
        rMetadata.update(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED, entryA);
    }

#if MTKCAM_CUSTOM_METADATA_COMMON
    if ( v1 )
        impBuildStaticInfo_v1_overwrite(rInfo, rMetadata);
#endif

    rMetadata.sort();

    return  MTRUE;
}

