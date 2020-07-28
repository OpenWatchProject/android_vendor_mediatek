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
//
#include "MyUtils.h"
#include "sensor_drv.h"
#include "seninf_drv.h"
//#include "HalSensorList.h"
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
HalSensorList::EnumInfo const*
HalSensorList::
queryEnumInfoByIndex(MUINT index) const
{
    Mutex::Autolock _l(mEnumSensorMutex);

    if(index >= mEnumSensorList.size())
    {
        MY_LOGE("bad sensorIdx:%d >= size:%zu", index, mEnumSensorList.size());
        return  NULL;
    }

    return  &mEnumSensorList[index];
}

/******************************************************************************
 *
 ******************************************************************************/
static
NSSensorType::Type
mapToSensorType(MUINT const sensor_type)
{
    NSSensorType::Type eSensorType;
    switch  (sensor_type)
    {
    case NSFeature::SensorInfoBase::EType_YUV:
        eSensorType = NSSensorType::eYUV;
        break;
    case NSFeature::SensorInfoBase::EType_RAW:
        eSensorType = NSSensorType::eRAW;
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
enumerateSensor_Locked()
{
    int ret = 0;
    NSFeature::SensorInfoBase* pSensorInfo ;

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    if(!pSeninfDrv) {
        MY_LOGE("pSeninfDrv == NULL");
        return 0;
    }

    if((ret = pSeninfDrv->init()) < 0) {
        MY_LOGE("pSeninfDrv->init() fail");
        return 0;
    }

    /*search sensor using 8mA driving current*/
    pSeninfDrv->setAllMclkOnOff(ISP_DRIVING_8MA, TRUE);

    pSensorDrv->init();

    MUINT max_index_of_camera = IMGSENSOR_SENSOR_IDX_SUB;
#ifdef MTK_CAM_MAX_NUMBER_OF_CAMERA
    max_index_of_camera = MTK_CAM_MAX_NUMBER_OF_CAMERA - 1;
#endif

    MY_LOGD("impSearchSensor search to %d\n", max_index_of_camera);
    for (MUINT i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i <= max_index_of_camera; i++) {
        if((ret = pSensorDrv->searchSensor((IMGSENSOR_SENSOR_IDX)i)) == SENSOR_NO_ERROR) {
            //query sensorinfo
            querySensorDrvInfo((IMGSENSOR_SENSOR_IDX)i);
            //fill in metadata
            buildSensorMetadata((IMGSENSOR_SENSOR_IDX)i);

            pSensorInfo = pSensorDrv->getSensorInfo((IMGSENSOR_SENSOR_IDX)i);
            addAndInitSensorEnumInfo_Locked(
                (IMGSENSOR_SENSOR_IDX)i,
                mapToSensorType(pSensorInfo->GetType()),
                pSensorInfo->getDrvMacroName());
        }
    }

    pSeninfDrv->setAllMclkOnOff(0, FALSE);

    ret = pSeninfDrv->uninit();
    if(ret < 0) {
        MY_LOGE("pSeninfDrv->uninit() fail");
        return 0;
    }
    pSeninfDrv->destroyInstance();
    pSensorDrv->uninit();

    return  mEnumSensorList.size();
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::EnumInfo const*
HalSensorList::
addAndInitSensorEnumInfo_Locked(
    IMGSENSOR_SENSOR_IDX eSensorIdx,
    MUINT           eSensorType,
    char const*     szSensorDrvName
)
{
    mEnumSensorList.push_back(EnumInfo());

    EnumInfo& rEnumInfo = mEnumSensorList.editTop();

    SensorStaticInfo     *pSensorStaticInfo = &sensorStaticInfo[eSensorIdx];
    bool bBackSide = (pSensorStaticInfo->facingDirection==0)? true : false;

    rEnumInfo.setDeviceId(eSensorIdx);
    rEnumInfo.setSensorType(eSensorType);
    rEnumInfo.setBackSide(bBackSide);
    rEnumInfo.setSensorDrvName(android::String8(szSensorDrvName));

    buildStaticInfo(Info(eSensorIdx, eSensorType, bBackSide, szSensorDrvName), rEnumInfo.mMetadata);

    return  &rEnumInfo;
}

