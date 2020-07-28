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

#define LOG_TAG "MtkCam/HalSensor"
//
#include "MyUtils.h"
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif
// For property_get().
#include <cutils/properties.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtkcam/utils/std/ULog.h>
//
//
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

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

//multiopen
MUINT32 gmeSensorDev;
pthread_mutex_t gpower_info_mutex = PTHREAD_MUTEX_INITIALIZER;



extern SensorDynamicInfo sensorDynamicInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];
extern MUINT32 mCreateSensorCount;
extern SENSORDRV_INFO_STRUCT sensorDrvInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];

//hwsync drv
#include <mtkcam/drv/IHwSyncDrv.h>
HWSyncDrv* mpHwSyncDrv=NULL;

//judge boot mode
static MUINT32 get_boot_mode(void)
{
    MINT32 fd;
    ssize_t s;
    char boot_mode[4] = {'0'};
    //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
    //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5
    fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
    if (fd < 0)
    {
        MY_LOGW("fail to open: %s", "/sys/class/BOOT/BOOT/boot/boot_mode");
        return 0;
    }

    s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
    close(fd);

    if(s <= 0)
    {
        MY_LOGW("could not read boot mode sys file");
        return 0;
    }

    boot_mode[s] = '\0';
    MY_LOGD("Boot Mode %d",atoi(boot_mode));
    return atoi(boot_mode);
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensor::PerDataPtr
HalSensor::
kpDefaultPerData = new PerData();

/******************************************************************************
 *
 ******************************************************************************/
HalSensor::
PerData::
PerData()
{
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensor::
~HalSensor()
{
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensor::
HalSensor()
    : IHalSensor()
    , mMutex()
    , mSensorDataMap()
    , mpSensorDrv(NULL)
    , mpSeninfDrv(NULL)
{

}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensor::
destroyInstance(
    char const* szCallerName
)
{
    mCreateSensorCount--;
    MY_LOGD("HalSensor destroyInstance count = %d, caller =%s",mCreateSensorCount,szCallerName);
    HalSensorList::singleton()->closeSensor(this, szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensor::
onDestroy()
{
    MY_LOGD("#Sensor:%zu gmeSensorDev %d", mSensorDataMap.size(),gmeSensorDev);

    Mutex::Autolock _l(mMutex);

#ifdef CONFIG_MTK_CAM_SECURE
    mpSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#endif

    pthread_mutex_lock(&gpower_info_mutex);
    if (gmeSensorDev == DUAL_CAMERA_NONE_SENSOR) {
        mpSeninfDrv->destroyInstance();//ToDo: necessary?
        mpSeninfDrv = NULL;
        mpSensorDrv = NULL;
        mSensorDataMap.clear();//ToDo:test
    }
    pthread_mutex_unlock(&gpower_info_mutex);
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
onCreate(
    SortedVector<MUINT>const& vSensorIndex
)
{
    MBOOL ret = MTRUE;

    MY_LOGD("+ #Sensor:%zu", vSensorIndex.size());

    Mutex::Autolock _l(mMutex);

    mSensorDataMap.clear();
    mSensorDataMap.setCapacity(vSensorIndex.size());
    for (MUINT i = 0; i < vSensorIndex.size(); i++)
    {
        MUINT const uSensorIndex = vSensorIndex[i];

        sp<PerData> pData = new PerData;
        mSensorDataMap.add(uSensorIndex, pData);

        mpSensorDrv = SensorDrv::get();
        if  (!mpSensorDrv) {
            MY_LOGE("SensorDrv::get() return NULL");
            ret = MFALSE;
            break;
        }

#ifdef CONFIG_MTK_CAM_SECURE
        mpSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#else
        mpSeninfDrv = SeninfDrv::createInstance();
#endif

        if  (!mpSeninfDrv) {
            MY_LOGE("SeninfDrv::createInstance() return NULL");
            ret = MFALSE;
            break;
        }
    }

    if(!ret)
    {
        mSensorDataMap.clear();
    }

    MY_LOGD("- ret:%d", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
isMatch(SortedVector<MUINT>const& vSensorIndex) const
{
    if  (vSensorIndex.size() != mSensorDataMap.size())
    {
        //MY_LOGD("isMatch vSensorIndex:%d, mSensorDataMap:%d", vSensorIndex.size(),mSensorDataMap.size());//ToDo: remove
        return  MFALSE;
    }

    for (MUINT i = 0; i < vSensorIndex.size(); i++)
    {
        if  ( vSensorIndex[i] != mSensorDataMap.keyAt(i) )
        {
            //MY_LOGD("isMatch vSensorIndex[i]:%d, mSensorDataMap[i]:%d", vSensorIndex[i],mSensorDataMap.keyAt(i));//ToDo:remove
            return  MFALSE;
        }
    }

    return  MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
powerOn(
    char const* szCallerName,
    MUINT const uCountOfIndex,
    MUINT const*pArrayOfIndex
)
{
    MBOOL ret = MTRUE;
    IMGSENSOR_SENSOR_IDX sensorIdx = IMGSENSOR_SENSOR_IDX_NONE;

    Mutex::Autolock _l(mMutex);

#ifdef CONFIG_MTK_CAM_SECURE
    mpSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#endif

    if ((ret = mpSeninfDrv->init()) < 0) {
        MY_LOGE("pSeninfDrv->init() fail ");
        return MFALSE;
    }
    if (uCountOfIndex > 1){
        MY_LOGE("power on multi-sensor at once");
        return MFALSE;
    }
    if (mpSensorDrv)
        mpSensorDrv->init();

    for (MUINT i = 0; i < uCountOfIndex; i++)
    {
        sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfIndex[i])->getDeviceId();

        MY_LOGD("sensorIdx : %d\n ",sensorIdx);
        pthread_mutex_lock(&gpower_info_mutex);
        if ((IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx) & gmeSensorDev) != 0) {
             pthread_mutex_unlock(&gpower_info_mutex);
             MY_LOGD("sensorIdx(0x%x) is already powerOn \n ",sensorIdx);
             ret = MFALSE;
             break;
        }
        pthread_mutex_unlock(&gpower_info_mutex);

        if (sensorIdx == IMGSENSOR_SENSOR_IDX_NONE) {
            MY_LOGD("gmeSensorDev is NONE ");
            ret = MFALSE;
            break;
        }

        if ((ret = setSensorMclk(sensorIdx, 1)) < 0) {
            MY_LOGE("setSensorMclk fail n");
            ret = MFALSE;
            break;
        }

        if ((ret = setSensorMclkDrivingCurrent(sensorIdx)) < 0) {
            MY_LOGE("initial IO driving current fail ");
            ret = MFALSE;
            break;
        }

        // Open sensor, try to open 3 time
        for (int i =0; i < 3; i++) {
            if ((ret = mpSensorDrv->open(sensorIdx)) != SENSOR_NO_ERROR) {
                MY_LOGE("pSensorDrv->open fail, retry = %d ", i);
            }
            else {
                break;
            }
        }
        if (ret != SENSOR_NO_ERROR) {
            MY_LOGE("pSensorDrv->open fail");
            setSensorMclk(sensorIdx, 0);
            mpSensorDrv->uninit();
            mpSeninfDrv->uninit();
            return MFALSE;
        }
        pthread_mutex_lock(&gpower_info_mutex);
        gmeSensorDev |= IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx);
        pthread_mutex_unlock(&gpower_info_mutex);

    }



    //hwsync driver, create instance and do init
    switch(get_boot_mode())
    {
        case 1:
        case 4:
            break;
        default:
            for (MUINT i = 0; i < uCountOfIndex; i++)
            {
                mpHwSyncDrv=HWSyncDrv::createInstance();
                mpHwSyncDrv->init(HW_SYNC_USER_HALSENSOR, pArrayOfIndex[i]);
            }
            break;
    }
    ret = MTRUE;
    MY_LOGD("- <%s> ret:%d, meSensorDev = 0x%x", szCallerName, ret, gmeSensorDev);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
powerOff(
    char const* szCallerName,
    MUINT const uCountOfIndex,
    MUINT const*pArrayOfIndex
)
{
    MBOOL ret = MFALSE;
    IMGSENSOR_SENSOR_IDX sensorIdx = IMGSENSOR_SENSOR_IDX_NONE;

    Mutex::Autolock _l(mMutex);

#ifdef CONFIG_MTK_CAM_SECURE
    mpSeninfDrv = SeninfDrv::createInstance(HalSensorList::singleton()->querySecureState());
#endif

    //hwsync driver, do uninit and destroy instance
    switch(get_boot_mode())
    {
        case 1:
        case 4:
            break;
        default:
            if(mpHwSyncDrv)
            {
                for (MUINT i = 0; i < uCountOfIndex; i++)
                {
                    mpHwSyncDrv->uninit(HW_SYNC_USER_HALSENSOR, pArrayOfIndex[i]);
                    mpHwSyncDrv->destroyInstance();
                }
            }
            break;
    }



    for (MUINT i = 0; i < uCountOfIndex; i++)
    {
        sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfIndex[i])->getDeviceId();
        MY_LOGD(" sensorIdx : %d",sensorIdx);

        pthread_mutex_lock(&gpower_info_mutex);
        if((gmeSensorDev & IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx))== 0)
        {
            pthread_mutex_unlock(&gpower_info_mutex);
            MY_LOGE("<%s> Cannot powerOff without powerOn sensorIdx %d gmeSensorDev %d", szCallerName, sensorIdx, gmeSensorDev);
            return MFALSE;
        }
        pthread_mutex_unlock(&gpower_info_mutex);

        sensorDynamicInfo[sensorIdx].TgInfo = CAM_TG_NONE;
        if (mpSensorDrv) {
            if((mpSensorDrv->close(sensorIdx)) != SENSOR_NO_ERROR) {
                MY_LOGE("mpSensorDrv->close fail");
                return MFALSE;
            }
        } else {
            MY_LOGE("mpSensorDrv is NULL");
        }

        /* Disable Sensor interface Clock divider*/
        if ((ret = setSensorMclk(sensorIdx, 0)) < 0) {
            MY_LOGE("setSensorMclk fail n");
            return MFALSE;
        }

        if (mpSeninfDrv) {
            mpSeninfDrv->sendCommand(CMD_SENINF_REFRESH_DFS, sensorIdx);
            mpSeninfDrv->resetConfiguration(sensorIdx);
        }

        pthread_mutex_lock(&gpower_info_mutex);
        gmeSensorDev &= (~(IMGSENSOR_SENSOR_IDX2DUAL(sensorIdx)));
        pthread_mutex_unlock(&gpower_info_mutex);
    }
    if (mpSensorDrv)
        mpSensorDrv->uninit();

    if (mpSeninfDrv) {
        if((ret = mpSeninfDrv->uninit()) < 0) {
            MY_LOGE("mpSeninfDrv->uninit fail");
            return MFALSE;
        }
    }

    MY_LOGD("- <%s> ret:%d, meSensorDev = 0x%x", szCallerName, ret ,gmeSensorDev);

    return MTRUE;
}
