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

#include "MyUtils.h"
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif
#include <mtkcam/def/common.h>

#include <string.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtkcam/utils/std/ULog.h>

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

//hwsync drv
#include <mtkcam/drv/IHwSyncDrv.h>
HWSyncDrv* mpHwSyncDrv=NULL;

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

//judge boot mode
static MUINT32 get_boot_mode(void)
{
    MINT32 fd;
    size_t s;
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
    : IHalSensor(),
    mMutex(),
    mSensorDataMap(),
    mSensorIdx(IMGSENSOR_SENSOR_IDX_NONE),
    mPowerOnCount(0),
    mScenarioId(0),
    mHdrMode(0),
    mPdafMode(0),
    mFramerate(0)
{
    memset(&mSensorDynamicInfo, 0, sizeof(SensorDynamicInfo));
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
    HalSensorList::singleton()->closeSensor(this, szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensor::
onDestroy()
{
    MY_LOGD("#Sensor:%zu", mSensorDataMap.size());

    Mutex::Autolock _l(mMutex);

    if (mSensorIdx == IMGSENSOR_SENSOR_IDX_NONE) {
        mSensorDataMap.clear();
    } else {
        MY_LOGI("Forget to powerOff before destroying. mSensorIdx:%d", mSensorIdx);
    }
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
    MY_LOGD("+ #Sensor:%zu", vSensorIndex.size());

    Mutex::Autolock _l(mMutex);

    mSensorDataMap.clear();
    mSensorDataMap.setCapacity(vSensorIndex.size());
    for (MUINT i = 0; i < vSensorIndex.size(); i++)
    {
        MUINT const uSensorIndex = vSensorIndex[i];

        sp<PerData> pData = new PerData;
        mSensorDataMap.add(uSensorIndex, pData);
    }

    return MTRUE;
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

Mutex HalSensor::sSensorPowerOnMutex[IMGSENSOR_SENSOR_IDX_MAX_NUM];
volatile int HalSensor::sSensorPowerOnCount[IMGSENSOR_SENSOR_IDX_MAX_NUM] = { 0 };

int
HalSensor::
sensorPowerOnCountInc(
        char  const *szCallerName,
        IMGSENSOR_SENSOR_IDX sensorIdx)
{
    sSensorPowerOnCount[sensorIdx]++;
    MY_LOGD("caller(%s) sensorIdx(0x%x) powerOn count: %d", szCallerName, sensorIdx, sSensorPowerOnCount[sensorIdx]);
    return sSensorPowerOnCount[sensorIdx];
}

int
HalSensor::
sensorPowerOnCountDec(
        char  const *szCallerName,
        IMGSENSOR_SENSOR_IDX sensorIdx)
{
    if (sSensorPowerOnCount[sensorIdx] > 0) {
        sSensorPowerOnCount[sensorIdx]--;
    }
    MY_LOGD("caller(%s) sensorIdx(0x%x) powerOn count: %d", szCallerName, sensorIdx, sSensorPowerOnCount[sensorIdx]);
    return sSensorPowerOnCount[sensorIdx];
}

int
HalSensor::
getSensorPowerOnCount(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    return sSensorPowerOnCount[sensorIdx];
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
powerOn(
    char  const *szCallerName,
    MUINT const  uCountOfIndex,
    MUINT const *pArrayOfIndex
)
{
    IMGSENSOR_SENSOR_IDX sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(*pArrayOfIndex)->getDeviceId();
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif
    ImgSensorDrv *pSensorDrv = ImgSensorDrv::getInstance(sensorIdx);

#ifdef CONFIG_MTK_CAM_SECURE_I2C
    MINT32 u32Enable = 0;
#endif

    (void)uCountOfIndex;

    Mutex::Autolock _l(mMutex);
    Mutex::Autolock _s(sSensorPowerOnMutex[sensorIdx]);

    MY_LOGD("sensorIdx : %d\n ", sensorIdx);

    if (mSensorIdx != IMGSENSOR_SENSOR_IDX_NONE && mSensorIdx != sensorIdx) {
        MY_LOGE("cannot powerOn more than one, sensorIdx(0x%x) powered\n ", mSensorIdx);
        return MFALSE;
    }

    if (getSensorPowerOnCount(sensorIdx) > 0) {
         mSensorIdx = sensorIdx;
         mPowerOnCount++;
         MY_LOGD("sensorIdx(0x%x) is already powerOn %x(%d)\n ", sensorIdx, mSensorIdx, mPowerOnCount);
         sensorPowerOnCountInc(szCallerName, sensorIdx);
         return MTRUE;
    }

    if (pSeninfDrv->init() < 0) {
        MY_LOGE("pSeninfDrv->init() fail ");
        return MFALSE;
    }

    if (pSensorDrv->init(sensorIdx) < 0) {
        MY_LOGE("pSensorDrv->init() fail ");
        return MFALSE;
    }

    if (setSensorMclk(sensorIdx, 1) < 0) {
        MY_LOGE("setSensorMclk fail n");
        return MFALSE;
    }

    if (setSensorMclkDrivingCurrent(sensorIdx) < 0) {
        MY_LOGE("initial IO driving current fail ");
        return MFALSE;
    }

#ifdef CONFIG_MTK_CAM_SECURE_I2C
      if(HalSensorList::singleton()->querySecureState()) {
        u32Enable = 1;
        pSensorDrv->sendCommand(CMD_SENSOR_OPEN_SECURE_SESSION);
        pSensorDrv->sendCommand(CMD_SENSOR_SET_AS_SECURE_DRIVER, (MUINTPTR)&u32Enable);
    } else {
        u32Enable = 0;
        //pSensorDrv->sendCommand(CMD_SENSOR_CLOSE_SECURE_SESSION);
        pSensorDrv->sendCommand(CMD_SENSOR_SET_AS_SECURE_DRIVER, (MUINTPTR)&u32Enable);
    }
#endif

    MBOOL ret = MTRUE;
    // Open sensor, try to open 3 time
    for (int j =0; j < 3; j++) {
        if ((ret = pSensorDrv->open()) < 0) {
            MY_LOGE("pSensorDrv->open fail, retry = %d ", j);
        }
        else {
            break;
        }
    }

    if (ret < 0) {
        MY_LOGE("pSensorDrv->open fail");

#ifdef CONFIG_MTK_CAM_SECURE_I2C
        if(HalSensorList::singleton()->querySecureState()) {
            MINT32 u32Enable = 0;
            pSensorDrv->sendCommand(CMD_SENSOR_CLOSE_SECURE_SESSION);
            pSensorDrv->sendCommand(CMD_SENSOR_SET_AS_SECURE_DRIVER, (MUINTPTR)&u32Enable);
        }
#endif

        return MFALSE;
    }

    mSensorIdx = sensorIdx;
    mPowerOnCount++;

    //hwsync driver, create instance and do init
    switch(get_boot_mode())
    {
        case 1:
        case 4:
            break;
        default:
            mpHwSyncDrv=HWSyncDrv::createInstance();
            mpHwSyncDrv->init(HW_SYNC_USER_HALSENSOR, *pArrayOfIndex);
            break;
    }

    MY_LOGD("- <%s> mSensorIdx = 0x%x", szCallerName, mSensorIdx);
    sensorPowerOnCountInc(szCallerName, sensorIdx);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
powerOff(
    char  const *szCallerName,
    MUINT const  uCountOfIndex,
    MUINT const *pArrayOfIndex
)
{
    CUSTOM_CFG_CSI_PORT port = CUSTOM_CFG_CSI_PORT_NONE;
    IMGSENSOR_SENSOR_IDX sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(*pArrayOfIndex)->getDeviceId();
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif
    ImgSensorDrv *pSensorDrv;
    SensorDynamicInfo *pSensorDynamicInfo = &mSensorDynamicInfo;

    (void)uCountOfIndex;

    Mutex::Autolock _l(mMutex);
    Mutex::Autolock _s(sSensorPowerOnMutex[sensorIdx]);

    if (mSensorIdx == IMGSENSOR_SENSOR_IDX_NONE || mSensorIdx != sensorIdx) {
        MY_LOGE("<%s> power off fail. mSensorIdx = %d, sensorIdx = %d", szCallerName, mSensorIdx, sensorIdx);
        return MFALSE;
    }

    if (sensorPowerOnCountDec(szCallerName, sensorIdx) > 0) {
        mPowerOnCount--;
        MY_LOGD("mSensorIdx : %x (%d)\n ", mSensorIdx, mPowerOnCount);
        if (mPowerOnCount == 0) {
            mSensorIdx = IMGSENSOR_SENSOR_IDX_NONE;
        }
        return MTRUE;
    }

    MY_LOGD("last user of sensorIdx : %d\n ", sensorIdx);

    //hwsync driver, do uninit and destroy instance
    switch(get_boot_mode()) {
    case 1:
    case 4:
        break;
    default:
        if (mpHwSyncDrv)
        {
            mpHwSyncDrv->uninit(HW_SYNC_USER_HALSENSOR, *pArrayOfIndex);
            mpHwSyncDrv->destroyInstance();
        }
        break;
    }

    pSensorDrv = ImgSensorDrv::getInstance(mSensorIdx);

    if (pSensorDrv) {
        if ((pSensorDrv->close()) < 0) {
            MY_LOGE("pSensorDrv->close fail");
            return MFALSE;
        }

#ifdef CONFIG_MTK_CAM_SECURE_I2C
        if(HalSensorList::singleton()->querySecureState()) {
            pSensorDrv->sendCommand(CMD_SENSOR_CLOSE_SECURE_SESSION);
            int u32Enable = 0;
            pSensorDrv->sendCommand(CMD_SENSOR_SET_AS_SECURE_DRIVER, (MUINTPTR)&u32Enable);
        }
#endif

        pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&port);

#ifdef CONFIG_MTK_CAM_SECURE
/*prevent powerOff without streaming_suspend -> devapc issue on seninfMuxReleaseAll*/
        pSeninfDrv->sendCommand(CMD_SENINF_RESUME_FREE);
#endif

        seninfMUXReleaseAll(port);

        /* Disable Sensor interface Clock divider*/
        if(setSensorMclk(mSensorIdx, false) < 0) {
            MY_LOGE("setSensorMclk fail n");
            return MFALSE;
        }

        if(pSensorDrv->uninit() < 0) {
            MY_LOGE("pSensorDrv->uninit fail");
            return MFALSE;
        }
    } else {
        MY_LOGE("pSensorDrv is NULL");
        return MFALSE;
    }

    /*seninf mux disable*/
    pSensorDynamicInfo->config_infos.clear();
    pSensorDynamicInfo->TgInfo   =
    pSensorDynamicInfo->HDRInfo  =
    pSensorDynamicInfo->PDAFInfo = CAM_TG_NONE;

    MY_LOGD("- <%s> mSensorIdx = 0x%x", szCallerName, mSensorIdx);
    pSeninfDrv->resetConfiguration(mSensorIdx);
    mPowerOnCount = 0;
    mSensorIdx = IMGSENSOR_SENSOR_IDX_NONE;

    SENINF_CSI_MIPI csiMipi;
    csiMipi.pCsiInfo = pSeninfDrv->getCSIInfo(port);

    pSeninfDrv->powerOff((void *)&csiMipi);

    if (pSeninfDrv->uninit() < 0) {
        MY_LOGE("pSeninfDrv->uninit fail");
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL HalSensor::querySensorDynamicInfo(
   MUINT32 indexDual,
   SensorDynamicInfo *pSensorDynamicInfo
)
{
    (void)indexDual;
    SensorDynamicInfo *pmSensorDynamicInfo = &mSensorDynamicInfo;
    //memcpy(pSensorDynamicInfo, &mSensorDynamicInfo, sizeof(SensorDynamicInfo));
    //vector must be assined one by one
    //MY_LOGE("sizeof(SensorDynamicInfo) %d",sizeof(SensorDynamicInfo));
    pSensorDynamicInfo->TgInfo = pmSensorDynamicInfo->TgInfo;
    pSensorDynamicInfo->pixelMode = pmSensorDynamicInfo->pixelMode;
    pSensorDynamicInfo->TgVR1Info = pmSensorDynamicInfo->TgVR1Info;
    pSensorDynamicInfo->TgVR2Info = pmSensorDynamicInfo->TgVR2Info;
    pSensorDynamicInfo->TgCLKInfo = pmSensorDynamicInfo->TgCLKInfo;
    pSensorDynamicInfo->HDRInfo = pmSensorDynamicInfo->HDRInfo;
    pSensorDynamicInfo->PDAFInfo = pmSensorDynamicInfo->PDAFInfo;
    pSensorDynamicInfo->HDRPixelMode = pmSensorDynamicInfo->HDRPixelMode;
    pSensorDynamicInfo->PDAFPixelMode = pmSensorDynamicInfo->PDAFPixelMode;
    for(int i = 0; i < HDR_DATA_MAX_NUM; i++) {
        pSensorDynamicInfo->CamInfo[i] = pmSensorDynamicInfo->CamInfo[i];
        pSensorDynamicInfo->PixelMode[i] = pmSensorDynamicInfo->PixelMode[i];
    }
    pSensorDynamicInfo->config_infos.clear();
    for(unsigned int i = 0; i < pmSensorDynamicInfo->config_infos.size(); i++)
        pSensorDynamicInfo->config_infos.push_back(pmSensorDynamicInfo->config_infos[i]);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL HalSensor::configure(
    MUINT const         uCountOfParam,
    ConfigParam const  *pConfigParam
)
{
    MINT32 ret = MFALSE;
    HALSENSOR_SENINF_CSI csi;
    IMGSENSOR_SENSOR_IDX sensorIdx = (IMGSENSOR_SENSOR_IDX)HalSensorList::singleton()->queryEnumInfoByIndex(pConfigParam->index)->getDeviceId();

    (void)uCountOfParam;

    Mutex::Autolock _l(mMutex);

    if (mSensorIdx == IMGSENSOR_SENSOR_IDX_NONE || mSensorIdx != sensorIdx) {
        MY_LOGE("configure fail. mSensorIdx = %d, sensorIdx = %d", mSensorIdx, sensorIdx);
        return MFALSE;
    }

    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(mSensorIdx);

#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *const pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());

    /*prevent without streaming_suspend -> devapc issue on seninfMuxReleaseAll*/
    pSeninfDrv->sendCommand(CMD_SENINF_RESUME_FREE);
#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::getInstance();
#endif

    pSensorDrv->sendCommand(CMD_SENSOR_SET_STREAMING_SUSPEND);

    CUSTOM_CFG_CSI_PORT port;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&port);
    csi.pCsiInfo = pSeninfDrv->getCSIInfo(port);
    MY_LOGD("CSI2_IP = %d seninfSrc = %d srcType = %d", csi.pCsiInfo->port, csi.pCsiInfo->seninf, csi.pCsiInfo->srcType);

    csi.pConfigParam = pConfigParam;
    csi.pInfo = &(pSensorDrv->getDrvInfo()->info);
    csi.sensorIdx = mSensorIdx;

    switch (csi.pCsiInfo->srcType) {
    case MIPI_SENSOR:
        ret = configureMipi(csi);
        break;
    case SERIAL_SENSOR:
        ret = configureSerial(csi);
        break;
    case PARALLEL_SENSOR:
        ret = configureParallel(csi);
        break;
    default:
        break;
    }

    if (pConfigParam->HDRMode) { // vHDR mode
        MUINT hdr_lv = 0xffffffff;
        if (pConfigParam->exposureTime > 0 && pConfigParam->exposureTime_se > 0 && pConfigParam->gain > 0) {
            if (pConfigParam->HDRMode == SENSOR_VHDR_MODE_ZVHDR && pConfigParam->gain_se > 0) { // ZVHDR mode: needs dual gain
                ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_HDR_SHUTTER, (MUINTPTR)&(pConfigParam->exposureTime), (MUINTPTR)&(pConfigParam->exposureTime_se), (MUINTPTR)&(hdr_lv));
                ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_SENSOR_DUAL_GAIN, (MUINTPTR)&(pConfigParam->gain), (MUINTPTR)&(pConfigParam->gain_se));
            }  else if(csi.pInfo->HDR_Support == MVHDR_SUPPORT_MultiCAMSV) { //3HDR mode: needs triple shutter/gain
                ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_HDR_TRI_SHUTTER, (MUINTPTR)&(pConfigParam->exposureTime), (MUINTPTR)&(pConfigParam->exposureTime_me), (MUINTPTR)&(pConfigParam->exposureTime_se));
                ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_HDR_TRI_GAIN, (MUINTPTR)&(pConfigParam->gain), (MUINTPTR)&(pConfigParam->gain_me), (MUINTPTR)&(pConfigParam->gain_se));
            } else {
                ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_HDR_SHUTTER, (MUINTPTR)&(pConfigParam->exposureTime), (MUINTPTR)&(pConfigParam->exposureTime_se), (MUINTPTR)&(hdr_lv));
                ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_SENSOR_GAIN, (MUINTPTR)&(pConfigParam->gain));
            }
        }
    } else { // normal mode
        if (pConfigParam->exposureTime > 0 && pConfigParam->gain > 0) {
            ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_SENSOR_EXP_TIME, (MUINTPTR)&(pConfigParam->exposureTime));
            ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_SENSOR_GAIN, (MUINTPTR)&(pConfigParam->gain));
        }
    }

    return (ret == 0);
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::seninfMUXReleaseAll(CUSTOM_CFG_CSI_PORT port)
{
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif

    for(int i = SENINF_MUX1; i < SENINF_MUX_NUM; i++)
        if(pSeninfDrv->getSeninfTopMuxCtrl((SENINF_MUX_ENUM)i) == pSeninfDrv->getCSIInfo(port)->seninf && pSeninfDrv->isMUXUsed((SENINF_MUX_ENUM)i))
            pSeninfDrv->disableMUX((SENINF_MUX_ENUM)i);

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/

SENINF_MUX_ENUM HalSensor::seninfMUXArbitration(SENINF_MUX_ENUM searchStart, SENINF_MUX_ENUM searchEnd)
{
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif

    MINT i = searchStart;
    MY_LOGD("searchStart %d searchEnd %d", searchStart, searchEnd);

    while(i < searchEnd) {
        if(!pSeninfDrv->isMUXUsed((SENINF_MUX_ENUM)i))
            break;
        i++;
    }

    if(i == searchEnd) {
        MY_LOGE("No free MUX for CAM");
        i = SENINF_MUX_ERROR;
    }

    return (SENINF_MUX_ENUM)i;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::configureMipi(HALSENSOR_SENINF_CSI &csi)
{
    int i;
    MINT32 ret = 0;
    MUINT32 frame_line = 0;
    MUINT32 pclk = 0;
    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(csi.sensorIdx);
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif

    ACDK_SENSOR_INFO_STRUCT *pInfo        = csi.pInfo;
    const ConfigParam       *pConfigParam = csi.pConfigParam;

    SENINF_CSI_MIPI csiMipi;
    MUINT32 hdrMode = 0, pdafMode = 0, framerate = 0;

    if (pInfo->SensroInterfaceType != SENSOR_INTERFACE_TYPE_MIPI)
        MY_LOGW("Sensor type doesn't match CSI port SensroInterfaceType = %d, srcType = %d", pInfo->SensroInterfaceType, csi.pCsiInfo->srcType);

    /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
    pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY, (MUINTPTR)&(pConfigParam->scenarioId), (MUINTPTR)&pdafMode);
    MY_LOGD("PDAF on/off = %d, PDAF support mode = %d", pdafMode, pInfo->PDAF_Support);

    hdrMode = pConfigParam->HDRMode;

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //Test Mode use property parameter
    {
        property_get("vendor.debug.senif.hdrmode", value, "0");
        int hdrModeTest = atoi(value);
        if(hdrModeTest == 1 || hdrModeTest == 2 || hdrModeTest == 9)
        {
            hdrMode = hdrModeTest;
        }
    }
    //PDAFTest Mode use property parameter
    {
        property_get("vendor.debug.senif.pdafmode", value, "0");
        int PDAFModeTest = atoi(value);
        if(PDAFModeTest == 1)
        {
            pdafMode = 1;
        }
    }

    MY_LOGD("SenDev=%d, scenario=%d, HDR=%d",
        csi.sensorIdx,
        pConfigParam->scenarioId,
        hdrMode);





    csiMipi.enable          = 1;
    csiMipi.dataheaderOrder = pInfo->SensorPacketECCOrder;
    csiMipi.dlaneNum        = pInfo->SensorMIPILaneNumber;
    csiMipi.csi_type        = (pInfo->MIPIsensorType == MIPI_CPHY) ? CSI2_2_5G_CPHY : CSI2_2_5G;
    csiMipi.pCsiInfo        = csi.pCsiInfo;
#ifdef DFS_SUPPORT
    pSensorDrv->sendCommand(CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO, (MUINTPTR)&pConfigParam->scenarioId,(MUINTPTR)&frame_line);
    csiMipi.line_length     = frame_line & 0xFFFF;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO, (MUINTPTR)&pConfigParam->scenarioId, (MUINTPTR)&csiMipi.pclk);
    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_PIXEL_RATE,(MUINTPTR)&pConfigParam->scenarioId, (MUINTPTR)&csiMipi.mipi_pixel_rate);
#endif
    MUINT8 *pDPCMType = &pInfo->IMGSENSOR_DPCM_TYPE_PRE;
    csiMipi.dpcm = (pConfigParam->scenarioId > SENSOR_SCENARIO_ID_SLIM_VIDEO2) ?
                    pInfo->IMGSENSOR_DPCM_TYPE_PRE :
                    pDPCMType[pConfigParam->scenarioId];

    MUINT32           inDataFmt = 0;

    ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

    // Source is from sensor
    switch(pSensorDrv->getType()) {
        case IMAGE_SENSOR_TYPE_RAW: {
            // RAW
            csiMipi.padSel = PAD_10BIT;//pad2cam_data_sel
            csiMipi.inDataType = RAW_10BIT_FMT;//cam_tg_input_fmt
            break;
        }

        case IMAGE_SENSOR_TYPE_RAW8: {
            // RAW
            csiMipi.padSel = (inDataFmt) ? PAD_8BIT_7_0 : PAD_8BIT_9_2;
            csiMipi.inDataType = RAW_8BIT_FMT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RAW12: {
            // RAW
            csiMipi.padSel = PAD_10BIT;
            csiMipi.inDataType = RAW_12BIT_FMT;
            break;
        }

        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR: {
            // Yuv422 or YCbCr
            csiMipi.padSel = (inDataFmt) ? PAD_8BIT_7_0 : PAD_8BIT_9_2;
            csiMipi.inDataType = YUV422_FMT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RGB565: {
            // RGB565
            csiMipi.padSel = (inDataFmt) ? PAD_8BIT_7_0 : PAD_8BIT_9_2;
            csiMipi.inDataType = RGB565_MIPI_FMT;
            break;
        }

        case IMAGE_SENSOR_TYPE_RGB888: {
            // RGB888
            csiMipi.padSel = (inDataFmt) ? PAD_8BIT_7_0 : PAD_8BIT_9_2;
            csiMipi.inDataType = RGB888_MIPI_FMT;
            break;
        }

        case IMAGE_SENSOR_TYPE_JPEG: {
            csiMipi.padSel = (inDataFmt) ? PAD_8BIT_7_0 : PAD_8BIT_9_2;
            csiMipi.inDataType = JPEG_FMT;
            break;
        }

        default:
            break;
    }
    pSeninfDrv->mutexLock();
    SensorVCInfo2 vcInfo2;

    pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_VC_INFO2, (MUINTPTR)&vcInfo2, (MUINTPTR)&(pConfigParam->scenarioId));
    MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d),VC4_ID(%d),VC4_DataType(%d),VC5_ID(%d),VC5_DataType(%d)",
            vcInfo2.vcInfo2s.size()> 0 ? vcInfo2.vcInfo2s[0].VC_ID : 0,
            vcInfo2.vcInfo2s.size()> 0 ? vcInfo2.vcInfo2s[0].VC_DataType :0,
            vcInfo2.vcInfo2s.size()> 1 ? vcInfo2.vcInfo2s[1].VC_ID :0,
            vcInfo2.vcInfo2s.size()> 1 ? vcInfo2.vcInfo2s[1].VC_DataType :0,
            vcInfo2.vcInfo2s.size()> 2 ? vcInfo2.vcInfo2s[2].VC_ID :0,
            vcInfo2.vcInfo2s.size()> 2 ? vcInfo2.vcInfo2s[2].VC_DataType :0,
            vcInfo2.vcInfo2s.size()> 3 ? vcInfo2.vcInfo2s[3].VC_ID :0,
            vcInfo2.vcInfo2s.size()> 3 ? vcInfo2.vcInfo2s[3].VC_DataType :0,
            vcInfo2.vcInfo2s.size()> 4 ? vcInfo2.vcInfo2s[4].VC_ID :0,
            vcInfo2.vcInfo2s.size()> 4 ? vcInfo2.vcInfo2s[4].VC_DataType :0,
            vcInfo2.vcInfo2s.size()> 5 ? vcInfo2.vcInfo2s[5].VC_ID :0,
            vcInfo2.vcInfo2s.size()> 5 ? vcInfo2.vcInfo2s[5].VC_DataType :0);
    ret = pSeninfDrv->setSeninfVC(
            csi.pCsiInfo->seninf,
            vcInfo2.vcInfo2s.size()> 0 ? ((vcInfo2.vcInfo2s[0].VC_DataType<<2)|(vcInfo2.vcInfo2s[0].VC_ID&0x03)) :0,
            vcInfo2.vcInfo2s.size()> 1 ? ((vcInfo2.vcInfo2s[1].VC_DataType<<2)|(vcInfo2.vcInfo2s[1].VC_ID&0x03)) :0,
            vcInfo2.vcInfo2s.size()> 2 ? ((vcInfo2.vcInfo2s[2].VC_DataType<<2)|(vcInfo2.vcInfo2s[2].VC_ID&0x03)) :0,
            vcInfo2.vcInfo2s.size()> 3 ? ((vcInfo2.vcInfo2s[3].VC_DataType<<2)|(vcInfo2.vcInfo2s[3].VC_ID&0x03)) :0,
            vcInfo2.vcInfo2s.size()> 4 ? ((vcInfo2.vcInfo2s[4].VC_DataType<<2)|(vcInfo2.vcInfo2s[4].VC_ID&0x03)) :0,
            vcInfo2.vcInfo2s.size()> 5 ? ((vcInfo2.vcInfo2s[5].VC_DataType<<2)|(vcInfo2.vcInfo2s[5].VC_ID&0x03)) :0);

    pSeninfDrv->setSeninfCsi((void *)&csiMipi, MIPI_SENSOR);

    SensorDynamicInfo *pSensorDynamicInfo = &mSensorDynamicInfo;
    SENINF_MUX_ENUM    muxSelect = SENINF_MUX_ERROR;
    SENINF_MUX_ENUM    vc_muxSelect = SENINF_MUX_ERROR;
    int pixel_mode = 0;
    int tg_select = 0;
    pSensorDynamicInfo->config_infos.clear();
#ifdef DFS_SUPPORT
    pSensorDynamicInfo->pixelMode =
            (pConfigParam->isp_pixel_mode != 0 && pConfigParam->isp_pixel_mode != UNKNOWN_PIXEL_MODE)
            ?pConfigParam->isp_pixel_mode :SENINF_PIXEL_MODE_CAM;
#else
    seninfLowPowerConfigure((MSDK_SCENARIO_ID_ENUM)pConfigParam->scenarioId);
#endif

    //Fixed pixel mode
    pSensorDynamicInfo->HDRPixelMode  =
    pSensorDynamicInfo->PDAFPixelMode = SENINF_PIXEL_MODE_CAMSV;

    {
        property_get("vendor.debug.seninf.pixelmode", value, "-1");
        int pixelMode_debug = atoi(value);
        if (pixelMode_debug >= ONE_PIXEL_MODE && pixelMode_debug <= FOUR_PIXEL_MODE) {
            pSensorDynamicInfo->pixelMode = pixelMode_debug;
        }
    }

    for(i = 0; i < HDR_DATA_MAX_NUM; i++) {
        pSensorDynamicInfo->PixelMode[i] = SENINF_PIXEL_MODE_CAMSV;
        pSensorDynamicInfo->CamInfo[i] = CAM_TG_NONE;
    }

    pSensorDynamicInfo->TgInfo   =
    pSensorDynamicInfo->HDRInfo  =
    pSensorDynamicInfo->PDAFInfo = CAM_TG_NONE;

    CUSTOM_CFG_CSI_PORT port;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&port);

    int isp_tg_num = pConfigParam->isp_tg_num > 1 ? pConfigParam->isp_tg_num : 1;
    int grab_offset_dcif = 0;
    int isp_grab_alignment = pConfigParam->isp_grab_alignment ? pConfigParam->isp_grab_alignment : 1;
    SensorCropWinInfo cropInfo;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&pConfigParam->scenarioId, (MUINTPTR)&cropInfo);
    int size_increament = (cropInfo.w2_tg_size/isp_tg_num)%isp_grab_alignment
                            ? (((cropInfo.w2_tg_size/isp_tg_num)/isp_grab_alignment) + 1) * isp_grab_alignment
                            : (cropInfo.w2_tg_size/isp_tg_num)/isp_grab_alignment * isp_grab_alignment;

    seninfMUXReleaseAll(port);
    MY_LOGD("vcInfo2.vcInfo2s.size() %lu size_increament %d, isp_tg_num %d isp_grab_alignment %d",
        (unsigned long) vcInfo2.vcInfo2s.size(), size_increament, isp_tg_num, isp_grab_alignment );
    if(vcInfo2.vcInfo2s.size() == 0) {
        for (int j = 0; j < isp_tg_num; j++) {
            muxSelect = seninfMUXArbitration(SENINF_CAM_MUX_MIN, SENINF_CAM_MUX_MAX);
            if (muxSelect != SENINF_MUX_ERROR) {
                pSeninfDrv->enableMUX(muxSelect);
                pSeninfDrv->setSeninfMuxCtrl(muxSelect,
                                             pInfo->SensorHsyncPolarity ? 0 : 1,
                                             pInfo->SensorHsyncPolarity ? 0 : 1,
                                             MIPI_SENSOR,
                                             csiMipi.inDataType,
                                             pSensorDynamicInfo->pixelMode);
                pSeninfDrv->setSeninfTopMuxCtrl(muxSelect, csi.pCsiInfo->seninf);
                pSeninfDrv->setIntegration(csi.sensorIdx, muxSelect, pConfigParam->crop, pConfigParam->framerate);
#ifdef CAM_MUX_CONFIGURABLE
                pSeninfDrv->sendCommand(CMD_SENINF_GET_FREE_CAM_MUX,(unsigned long) &pSensorDynamicInfo->TgInfo, SENINF_CAM_MUX_CAM_START, SENINF_CAM_MUX_CAM_END);
#else
                pSensorDynamicInfo->TgInfo = muxSelect - SENINF_MUX1 + CAM_TG_1;
#endif
                SeninfConfigInfo configInfo;
                configInfo.mux_pixel_mode = pSensorDynamicInfo->pixelMode;
                configInfo.feature_type = VC_RAW_DATA;
                configInfo.tg_info = pSensorDynamicInfo->TgInfo;
                configInfo.seninf_mux_info = muxSelect;
#if 0
                configInfo.grab_start_offset = 0;
                configInfo.grab_size_x = cropInfo.w2_tg_size;
#else

                configInfo.grab_start_offset = grab_offset_dcif;
                configInfo.grab_size_x = configInfo.grab_start_offset + size_increament > cropInfo.w2_tg_size
                                        ? (cropInfo.w2_tg_size - configInfo.grab_start_offset) : size_increament;
                grab_offset_dcif += configInfo.grab_size_x;

                MY_LOGD("grab_start_offset %d grab_size_x %d", configInfo.grab_start_offset, configInfo.grab_size_x);
#endif

                pSensorDynamicInfo->config_infos.push_back(configInfo);
#ifdef CAM_MUX_CONFIGURABLE
                pSeninfDrv->sendCommand(CMD_SENINF_SET_MUX_CROP_PIX_CTRL, (unsigned long)&configInfo, isp_tg_num > 1);
                pSeninfDrv->sendCommand(CMD_SENINF_SET_CAM_MUX, (unsigned long)&configInfo, (unsigned long)NULL);
#endif
#ifdef DFS_SUPPORT
                 pSeninfDrv->sendCommand(CMD_SENINF_SET_DFS_FOR_MUX, (unsigned long)&configInfo, (unsigned long)&csiMipi);
#endif
            }
        }
    } else {
        int tg_used_dcif = 0;
        for(unsigned int i = 0; i < vcInfo2.vcInfo2s.size(); i++){
#if 0
             MY_LOGD("i %d VC_FEATURE %d, VC_ID %d, VC_DataType 0x%x, VC_SIZEH_PIXEL 0x%x, VC_SIZEV 0x%x, VC_SIZEH_BYTE 0x%x",
                    i,
                    vcInfo2.vcInfo2s[i].VC_FEATURE,
                    vcInfo2.vcInfo2s[i].VC_ID,
                    vcInfo2.vcInfo2s[i].VC_DataType,
                    vcInfo2.vcInfo2s[i].VC_SIZEH_PIXEL,
                    vcInfo2.vcInfo2s[i].VC_SIZEV,
                    vcInfo2.vcInfo2s[i].VC_SIZEH_BYTE);
#endif
            if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_RAW_DATA) {
                if(tg_used_dcif == isp_tg_num)
                    continue;
                muxSelect = seninfMUXArbitration(SENINF_CAM_MUX_MIN, SENINF_CAM_MUX_MAX);
                pixel_mode = pSensorDynamicInfo->pixelMode;
#ifdef CAM_MUX_CONFIGURABLE
                pSeninfDrv->sendCommand(CMD_SENINF_GET_FREE_CAM_MUX,(unsigned long ) &tg_select, SENINF_CAM_MUX_CAM_START, SENINF_CAM_MUX_CAM_END);
#else
                tg_select = muxSelect - SENINF_MUX1 + CAM_TG_1;
#endif

                pSensorDynamicInfo->TgInfo = tg_select;
                tg_used_dcif += 1;

            } else if (vcInfo2.vcInfo2s[i].VC_FEATURE >= VC_PDAF_MIN_NUM &&
                vcInfo2.vcInfo2s[i].VC_FEATURE < VC_PDAF_MAX_NUM &&
                !(pdafMode &&
                    (pInfo->PDAF_Support == PDAF_SUPPORT_CAMSV ||
                    pInfo->PDAF_Support == PDAF_SUPPORT_CAMSV_LEGACY ||
                    pInfo->PDAF_Support == PDAF_SUPPORT_CAMSV_DUALPD))) {
                MY_LOGD("skip i %d, pInfo->PDAF_Support %d, VC_FEATURE %d, VC_ID %d, VC_DataType 0x%x, VC_SIZEH_PIXEL 0x%x, VC_SIZEV 0x%x, VC_SIZEH_BYTE 0x%x",
                        i,
                        pInfo->PDAF_Support,
                        vcInfo2.vcInfo2s[i].VC_FEATURE,
                        vcInfo2.vcInfo2s[i].VC_ID,
                        vcInfo2.vcInfo2s[i].VC_DataType,
                        vcInfo2.vcInfo2s[i].VC_SIZEH_PIXEL,
                        vcInfo2.vcInfo2s[i].VC_SIZEV,
                        vcInfo2.vcInfo2s[i].VC_SIZEH_BYTE);
                continue;
            } else if (vcInfo2.vcInfo2s[i].VC_FEATURE >= VC_HDR_MIN_NUM &&
                vcInfo2.vcInfo2s[i].VC_FEATURE < VC_3HDR_MAX_NUM &&
                hdrMode != SENSOR_VHDR_MODE_MVHDR) {
                MY_LOGD("skip i %d, hdrMode %d, VC_FEATURE %d, VC_ID %d, VC_DataType 0x%x, VC_SIZEH_PIXEL 0x%x, VC_SIZEV 0x%x, VC_SIZEH_BYTE 0x%x",
                        i,
                        hdrMode,
                        vcInfo2.vcInfo2s[i].VC_FEATURE,
                        vcInfo2.vcInfo2s[i].VC_ID,
                        vcInfo2.vcInfo2s[i].VC_DataType,
                        vcInfo2.vcInfo2s[i].VC_SIZEH_PIXEL,
                        vcInfo2.vcInfo2s[i].VC_SIZEV,
                        vcInfo2.vcInfo2s[i].VC_SIZEH_BYTE);
                continue;
            } else {

#ifdef CAM_MUX_CONFIGURABLE
                if(vc_muxSelect == SENINF_MUX_ERROR)
                    vc_muxSelect = seninfMUXArbitration(SENINF_CAMSV_MUX_MIN, SENINF_CAMSV_MUX_MAX);
                pixel_mode = ((vcInfo2.vcInfo2s[i].VC_SIZEH_PIXEL) % 4) ? TWO_PIXEL_MODE : FOUR_PIXEL_MODE;
                pSeninfDrv->sendCommand(CMD_SENINF_GET_FREE_CAM_MUX, (unsigned long )&tg_select, SENINF_CAM_MUX_SV_START, SENINF_CAM_MUX_SV_END);
                muxSelect = vc_muxSelect;
                if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_PDAF_STATS)
                    pSensorDynamicInfo->PDAFPixelMode = pixel_mode;
                else if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_HDR_MVHDR)
                    pSensorDynamicInfo->HDRPixelMode = pixel_mode;
                else if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_3HDR_EMBEDDED)
                    pSensorDynamicInfo->PixelMode[EmbInfo] = pixel_mode;
                else if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_3HDR_FLICKER)
                    pSensorDynamicInfo->PixelMode[FlickerInfo] = pixel_mode;
                else if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_3HDR_Y)
                    pSensorDynamicInfo->PixelMode[YInfo] = pixel_mode;
                else if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_3HDR_AE)
                    pSensorDynamicInfo->PixelMode[AEInfo] = pixel_mode;
#else
                muxSelect = seninfMUXArbitration(SENINF_CAMSV_MUX_MIN, SENINF_CAMSV_MUX_MAX);
                pixel_mode = SENINF_PIXEL_MODE_CAMSV;
                tg_select = muxSelect - SENINF_CAMSV_MUX_MIN + CAM_SV_1;
#endif
            }
            MY_LOGD("i %d VC_FEATURE %d, VC_ID %d, VC_DataType 0x%x, VC_SIZEH_PIXEL 0x%x, VC_SIZEV 0x%x, VC_SIZEH_BYTE 0x%x, muxSelect %d, pixel_mode %d, tg_select %d",
                    i,
                    vcInfo2.vcInfo2s[i].VC_FEATURE,
                    vcInfo2.vcInfo2s[i].VC_ID,
                    vcInfo2.vcInfo2s[i].VC_DataType,
                    vcInfo2.vcInfo2s[i].VC_SIZEH_PIXEL,
                    vcInfo2.vcInfo2s[i].VC_SIZEV,
                    vcInfo2.vcInfo2s[i].VC_SIZEH_BYTE,
                    muxSelect,
                    pixel_mode,
                    tg_select);
            if (muxSelect != SENINF_MUX_ERROR) {
                pSeninfDrv->enableMUX(muxSelect);
                pSeninfDrv->setSeninfMuxCtrl(muxSelect,
                                         pInfo->SensorHsyncPolarity ? 0 : 1,
                                         pInfo->SensorHsyncPolarity ? 0 : 1,
                                          (SENINF_SOURCE_ENUM)(i + MIPI_SENSOR),
                                         csiMipi.inDataType,
                                         pixel_mode);
                pSeninfDrv->setSeninfTopMuxCtrl(muxSelect, csi.pCsiInfo->seninf);
                SeninfConfigInfo configInfo;
                configInfo.mux_pixel_mode = pixel_mode;
                configInfo.feature_type = vcInfo2.vcInfo2s[i].VC_FEATURE;
                configInfo.tg_info = tg_select;
                configInfo.seninf_mux_info = muxSelect;
                if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_RAW_DATA) {
                        pSeninfDrv->setIntegration(csi.sensorIdx, muxSelect, pConfigParam->crop, pConfigParam->framerate);
                        configInfo.grab_start_offset = grab_offset_dcif;
                        configInfo.grab_size_x = configInfo.grab_start_offset + size_increament > cropInfo.w2_tg_size
                            ? (cropInfo.w2_tg_size - configInfo.grab_start_offset) : size_increament;
                        grab_offset_dcif += configInfo.grab_size_x;
                        MY_LOGD("grab_start_offset %d grab_size_x %d", configInfo.grab_start_offset, configInfo.grab_size_x);
                } else {
                    configInfo.grab_start_offset = 0;
                    configInfo.grab_size_x = vcInfo2.vcInfo2s[i].VC_SIZEH_PIXEL;
                }
                pSensorDynamicInfo->config_infos.push_back(configInfo);
#ifdef CAM_MUX_CONFIGURABLE
                pSeninfDrv->sendCommand(CMD_SENINF_SET_MUX_CROP_PIX_CTRL, (unsigned long)&configInfo, isp_tg_num > 1);
                pSeninfDrv->sendCommand(CMD_SENINF_SET_CAM_MUX, (unsigned long)&configInfo, (unsigned long)&vcInfo2.vcInfo2s[i]);

                if(vcInfo2.vcInfo2s[i].VC_FEATURE == VC_RAW_DATA)
                    i--;//repeat VC_RAW_DATA for tg_num times if DCIF
#endif
#ifdef DFS_SUPPORT
                pSeninfDrv->sendCommand(CMD_SENINF_SET_DFS_FOR_MUX, (unsigned long)&configInfo, (unsigned long)&csiMipi);
#endif

            }
        }
    }
    pSeninfDrv->mutexUnlock();

    for(unsigned int j = 0; j < pSensorDynamicInfo->config_infos.size(); j++) {
        MY_LOGD("j %d, feature_type %d, mux_pixel_mode %d, tg_info %d",
            j,
            pSensorDynamicInfo->config_infos[j].feature_type,
            pSensorDynamicInfo->config_infos[j].mux_pixel_mode,
            pSensorDynamicInfo->config_infos[j].tg_info);

        if(pSensorDynamicInfo->config_infos[j].feature_type == VC_PDAF_STATS)
            pSensorDynamicInfo->PDAFInfo = pSensorDynamicInfo->config_infos[j].tg_info;
        else if(pSensorDynamicInfo->config_infos[j].feature_type == VC_HDR_MVHDR)
            pSensorDynamicInfo->HDRInfo = pSensorDynamicInfo->config_infos[j].tg_info;
        else if(pSensorDynamicInfo->config_infos[j].feature_type == VC_3HDR_EMBEDDED)
            pSensorDynamicInfo->CamInfo[EmbInfo] = pSensorDynamicInfo->config_infos[j].tg_info;
        else if(pSensorDynamicInfo->config_infos[j].feature_type == VC_3HDR_FLICKER)
            pSensorDynamicInfo->CamInfo[FlickerInfo] = pSensorDynamicInfo->config_infos[j].tg_info;
        else if(pSensorDynamicInfo->config_infos[j].feature_type == VC_3HDR_Y)
            pSensorDynamicInfo->CamInfo[YInfo] = pSensorDynamicInfo->config_infos[j].tg_info;
        else if(pSensorDynamicInfo->config_infos[j].feature_type == VC_3HDR_AE)
            pSensorDynamicInfo->CamInfo[AEInfo] = pSensorDynamicInfo->config_infos[j].tg_info;
    }
    pSensorDynamicInfo->TgCLKInfo = SENINF_TIMESTAMP_CLK;

    MY_LOGD("Tg usage infomation[%d] pixel mode %d: HDR = %d pixel mode %d, PDAF = %d pixel mode %d",
        pSensorDynamicInfo->TgInfo, pSensorDynamicInfo->pixelMode,
        pSensorDynamicInfo->HDRInfo, pSensorDynamicInfo->HDRPixelMode,
        pSensorDynamicInfo->PDAFInfo, pSensorDynamicInfo->PDAFPixelMode);
#if 0
    for(i = 0; i < HDR_DATA_MAX_NUM; i++)
        MY_LOGD("CamInfo[%d] tg %d pixel mode %d", i, pSensorDynamicInfo->CamInfo[i], pSensorDynamicInfo->PixelMode[i]);
#endif
    if (pConfigParam->framerate)
        // Unit : FPS , Driver Unit : 10*FPS
        framerate = pConfigParam->framerate * 10;
    else
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&(pConfigParam->scenarioId), (MUINTPTR)&framerate);

    MY_LOGD("framerate = %d", framerate);

    mScenarioId = pConfigParam->scenarioId;
    mHdrMode    = hdrMode;
    mPdafMode   = pdafMode;
    mFramerate  = framerate;

    pSensorDrv->setScenario((MSDK_SCENARIO_ID_ENUM)pConfigParam->scenarioId, framerate, hdrMode, pdafMode);

#ifdef HALSENSOR_AUTO_DESKEW
    if (csiMipi.csi_type != CSI2_2_5G_CPHY) {
        ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_STREAMING_RESUME); //sensor deskew pattern
        pSeninfDrv->setAutoDeskew(&csiMipi);
        ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_STREAMING_SUSPEND);
}
#endif

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::configureSerial(HALSENSOR_SENINF_CSI &csi)
{
    SENINF_CSI_SCAM csiScam;
    SENINF_MUX_ENUM muxSelect;
    SensorDynamicInfo *pSensorDynamicInfo = &mSensorDynamicInfo;
    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(csi.sensorIdx);
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif
    ACDK_SENSOR_INFO_STRUCT *pInfo = csi.pInfo;
    MUINT32 framerate;

    const ConfigParam *pConfigParam = csi.pConfigParam;

    MINT32 ret = 0;

    if (csi.pInfo->SensroInterfaceType != SENSOR_INTERFACE_TYPE_SERIAL)
        MY_LOGW("Sensor type doesn't match CSI port SensroInterfaceType = %d, srcType = %d", csi.pInfo->SensroInterfaceType, csi.pCsiInfo->srcType);

    csiScam.enable             = 1;
    csiScam.SCAM_DataNumber    = (MUINT)pInfo->SCAM_DataNumber;
    csiScam.SCAM_DDR_En        = (MUINT)pInfo->SCAM_DDR_En;
    csiScam.SCAM_CLK_INV       = (MUINT)pInfo->SCAM_CLK_INV;
    csiScam.SCAM_DEFAULT_DELAY = (MUINT)pInfo->SCAM_DEFAULT_DELAY;
    csiScam.SCAM_CRC_En        = (MUINT)pInfo->SCAM_CRC_En;
    csiScam.SCAM_SOF_src       = (MUINT)pInfo->SCAM_SOF_src;
    csiScam.SCAM_Timout_Cali   = (MUINT)pInfo->SCAM_Timout_Cali;

    //Fixed pixel mode
    pSensorDynamicInfo->pixelMode     = SENINF_PIXEL_MODE_CAM;
    pSensorDynamicInfo->HDRPixelMode  =
    pSensorDynamicInfo->PDAFPixelMode = SENINF_PIXEL_MODE_CAMSV;

    pSensorDynamicInfo->TgInfo   =
    pSensorDynamicInfo->HDRInfo  =
    pSensorDynamicInfo->PDAFInfo = CAM_TG_NONE;

    CUSTOM_CFG_CSI_PORT port;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&port);
    seninfMUXReleaseAll(port);

    pSeninfDrv->mutexLock();
    muxSelect = seninfMUXArbitration(SENINF_CAM_MUX_MIN, SENINF_CAM_MUX_MAX);
    if(muxSelect != SENINF_MUX_ERROR) {
        pSeninfDrv->enableMUX(muxSelect);
        pSeninfDrv->setSeninfMuxCtrl(muxSelect,
                                     pInfo->SensorHsyncPolarity ? 0 : 1,
                                     pInfo->SensorHsyncPolarity ? 0 : 1,
                                     SERIAL_SENSOR,
                                     YUV422_FMT,
                                     pSensorDynamicInfo->pixelMode);
        pSeninfDrv->setSeninfTopMuxCtrl(muxSelect, csi.pCsiInfo->seninf);
        pSeninfDrv->setIntegration(csi.sensorIdx, muxSelect, pConfigParam->crop, pConfigParam->framerate);
        pSensorDynamicInfo->TgInfo = muxSelect - SENINF_MUX1 + CAM_TG_1;
    }
    pSeninfDrv->mutexUnlock();

    pSeninfDrv->setSeninfCsi((void *)&csiScam, SERIAL_SENSOR);

    pSensorDynamicInfo->TgCLKInfo = SENINF_TIMESTAMP_CLK;

    if (pConfigParam->framerate)
        // Unit : FPS , Driver Unit : 10*FPS
        framerate = pConfigParam->framerate * 10;
    else
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&(pConfigParam->scenarioId), (MUINTPTR)&framerate);

    MY_LOGD("framerate = %d", framerate);

    mScenarioId = pConfigParam->scenarioId;
    mHdrMode    = pConfigParam->HDRMode;
    mPdafMode   = 0;
    mFramerate  = framerate;

    pSensorDrv->setScenario((MSDK_SCENARIO_ID_ENUM)pConfigParam->scenarioId, framerate, pConfigParam->HDRMode, 0);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::configureParallel(HALSENSOR_SENINF_CSI &csi)
{
    SENINF_CSI_PARALLEL csiParallel;
    SENINF_MUX_ENUM muxSelect;
    SensorDynamicInfo *pSensorDynamicInfo = &mSensorDynamicInfo;
    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(csi.sensorIdx);
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif
    ACDK_SENSOR_INFO_STRUCT *pInfo = csi.pInfo;
    MUINT32 framerate;

    const ConfigParam *pConfigParam = csi.pConfigParam;

    MINT32 ret = 0;

    if (pInfo->SensroInterfaceType != SENSOR_INTERFACE_TYPE_PARALLEL)
        MY_LOGW("Sensor type doesn't match CSI port SensroInterfaceType = %d, srcType = %d", csi.pInfo->SensroInterfaceType, csi.pCsiInfo->srcType);

    //Fixed pixel mode
    pSensorDynamicInfo->pixelMode     = SENINF_PIXEL_MODE_CAM;
    pSensorDynamicInfo->HDRPixelMode  =
    pSensorDynamicInfo->PDAFPixelMode = SENINF_PIXEL_MODE_CAMSV;

    pSensorDynamicInfo->TgInfo   =
    pSensorDynamicInfo->HDRInfo  =
    pSensorDynamicInfo->PDAFInfo = CAM_TG_NONE;

    CUSTOM_CFG_CSI_PORT port;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&port);
    seninfMUXReleaseAll(port);

    pSeninfDrv->mutexLock();
    muxSelect = seninfMUXArbitration(SENINF_CAM_MUX_MIN, SENINF_CAM_MUX_MAX);
    if(muxSelect != SENINF_MUX_ERROR) {
        pSeninfDrv->enableMUX(muxSelect);
        pSeninfDrv->setSeninfMuxCtrl(muxSelect,
                                     pInfo->SensorHsyncPolarity ? 0 : 1,
                                     pInfo->SensorHsyncPolarity ? 0 : 1,
                                     PARALLEL_SENSOR,
                                     YUV422_FMT,
                                     pSensorDynamicInfo->pixelMode);
        pSeninfDrv->setSeninfTopMuxCtrl(muxSelect, csi.pCsiInfo->seninf);
        pSeninfDrv->setIntegration(csi.sensorIdx, muxSelect, pConfigParam->crop, pConfigParam->framerate);
        pSensorDynamicInfo->TgInfo = muxSelect - SENINF_MUX1 + CAM_TG_1;
    }
    pSeninfDrv->mutexUnlock();

    csiParallel.enable = 1;
    pSeninfDrv->setSeninfCsi((void *)&csiParallel, PARALLEL_SENSOR);

    pSensorDynamicInfo->TgCLKInfo = SENINF_TIMESTAMP_CLK;

    if (pConfigParam->framerate)
        // Unit : FPS , Driver Unit : 10*FPS
        framerate = pConfigParam->framerate * 10;
    else
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&(pConfigParam->scenarioId), (MUINTPTR)&framerate);

    MY_LOGD("framerate = %d", framerate);

    mScenarioId = pConfigParam->scenarioId;
    mHdrMode    = pConfigParam->HDRMode;
    mPdafMode   = 0;
    mFramerate  = framerate;

    pSensorDrv->setScenario((MSDK_SCENARIO_ID_ENUM)pConfigParam->scenarioId, framerate, pConfigParam->HDRMode, 0);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::setSensorMclk(IMGSENSOR_SENSOR_IDX sensorIdx, MBOOL pcEn)
{
    SENINF_MCLK_PARA mclk_para;
    MINT32  ret = 0;

    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(sensorIdx);
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif
    ACDK_SENSOR_INFO_STRUCT *pInfo = &(pSensorDrv->getDrvInfo()->info);

    mclk_para.sensorIdx       = sensorIdx;
    mclk_para.mclkFreq        = pInfo->SensorClockFreq;
    mclk_para.mclkPolarityLow = pInfo->SensorClockPolarity;
    mclk_para.mclkFallingCnt  = pInfo->SensorClockFallingCount;
    mclk_para.mclkRisingCnt   = pInfo->SensorClockRisingCount;

    ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&mclk_para.mclkIdx);

    ret = pSeninfDrv->configMclk(&mclk_para, pcEn);
    if (ret < 0) {
        MY_LOGE("configMclk fail");
        return ret;
    }
    MY_LOGD("sensorIdx %d, mclk_src %d, SensorMCLKPLL %d",
        sensorIdx, mclk_para.mclkIdx, mclk_para.mclkPLL);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::sendCommand(
    MUINT    indexDual,
    MUINTPTR cmd,
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3)
{
    MINT32 ret = 0;
    IMGSENSOR_SENSOR_IDX sensorDevId = IMGSENSOR_SENSOR_IDX_MAP(indexDual);

    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(sensorDevId);
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif

#ifdef CONFIG_MTK_CAM_SECURE
    MUINT32 isSecure = 0;
#endif

    if(sensorDevId == IMGSENSOR_SENSOR_IDX_NONE) {
        MY_LOGE("Wrong sensor index: %d", indexDual);
        return SENSOR_UNKNOWN_ERROR;
    }

    switch (cmd) {
    case SENSOR_CMD_GET_HORIZONTAL_BLANKING:
    {

        SensorDynamicInfo *psensorDynamicInfo = &mSensorDynamicInfo;
        MUINT32            pixelNum = 0;
        MUINT32            pixelClock = 0;
        MUINT32            hSize = 0;
        int mux = 0;

        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO, (MUINTPTR)&mScenarioId,(MUINTPTR)&pixelNum);
        std::vector<SeninfConfigInfo>::iterator it = psensorDynamicInfo->config_infos.begin();
        for (; it != psensorDynamicInfo->config_infos.end(); ++it) {
            if(it->tg_info != *(MUINT32 *)arg1)
                continue;
            if(it->feature_type == VC_RAW_DATA) {
                mux = it->seninf_mux_info;
                    //pSeninfDrv->getSeninfCamTGMuxCtrl(*(MUINT32 *)arg1 - CAM_TG_1);
                hSize = IS_MUX_HW_BUFFERED(mux) ? (pixelNum & 0xFFFF) : it->grab_size_x;
                break;
            } else {
                mux = it->seninf_mux_info;
                //mux = pSeninfDrv->getSeninfCamTGMuxCtrl(*(MUINT32 *)arg1 - CAM_SV_1 + CAM_TG_2);
                if(IS_MUX_HW_BUFFERED(mux)) {
                    hSize = pixelNum & 0xFFFF;
                        break;
                } else {
                    if(it->grab_size_x > hSize)
                        hSize = it->grab_size_x;//the largest one as peak
                    else
                        continue;
                }
            }
        }

        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO, (MUINTPTR)&mScenarioId, (MUINTPTR)&pixelClock);

        /* Unit: 10ns */
        *((MUINT32*) arg2) = (MUINT32)((double)100000000 * ((pixelNum & 0xFFFF) - hSize) / pixelClock);
        *((MUINT32*) arg3) = (MUINT32)((double)100000000 * (pixelNum & 0xFFFF) / pixelClock);

        break;
    }
    case SENSOR_CMD_GET_START_OF_EXPOSURE:
    {
        //arg1 scenario id, arg2 tSof, arg3 = EXP/tSoE;
        if (!arg2 || !arg3) {
            MY_LOGE("SENSOR_CMD_GET_START_OF_EXPOSURE fail arg2 %p arg3 %p", (MINT64*)arg2, (MINT64*)arg3);
            break;
        }

        MINT32 offset = 0;
        MINT64 tSoF = (*((MINT64*) arg2));
        MINT64 tExp = (*((MINT64*) arg3));
        MINT64 tSoE = 0;

        int offset_predefined = property_get_int32("vendor.debug.seninf.offset_start_exposure", 0);
        if (offset_predefined != 0)
            tSoE = tSoF - tExp + offset_predefined;
        else {
            ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_OFFSET_TO_START_OF_EXPOSURE, arg1 == NULL ?(MUINTPTR)&mScenarioId :arg1, (MUINTPTR)&offset);
#ifdef OFFSET_START_EXPOSURE
            if (offset == 0)
                offset = OFFSET_START_EXPOSURE;
#endif
            tSoE = tSoF - tExp + offset;
        }
        //MY_LOGD("111SENSOR_CMD_GET_START_OF_EXPOSURE offset %d tSoF %lld, tExp %lld tSoE %lld offset_predefined %d",
          // offset, tSoF, tExp, tSoE, offset_predefined);
        (*((MINT64*) arg3)) = tSoE;
    }
        break;
    case SENSOR_CMD_GET_SENINF_PIXEL_RATE:
    {
        if (arg3 != NULL && IS_MUX_HW_BUFFERED(*((MUINT32*) arg3))){
            unsigned int     line_length = 0;
            unsigned int     pclk = 0;
            MUINT32 frame_line = 0;
            pSensorDrv->sendCommand(CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO, arg1 == NULL ?(MUINTPTR)&mScenarioId :arg1, (MUINTPTR)&frame_line);
            line_length = frame_line & 0xFFFF;
            pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO, arg1 == NULL ?(MUINTPTR)&mScenarioId :arg1, (MUINTPTR)&pclk);
            SensorCropWinInfo cropInfo;
            pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO, arg1 == NULL ?(MUINTPTR)&mScenarioId :arg1, (MUINTPTR)&cropInfo);
            *((MUINT32*) arg2) = cropInfo.w2_tg_size * (pclk / (line_length - HW_BUF_EFFECT));
        } else {
             pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_PIXEL_RATE, arg1 == NULL ?(MUINTPTR)&mScenarioId :arg1, arg2);
        }
    }
        break;
    case SENSOR_CMD_GET_SENSOR_PIXELMODE:
        *(MUINT32 *)arg3 = mSensorDynamicInfo.pixelMode;
        break;

    case SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT:
        ret = pSeninfDrv->getN3DDiffCnt((MUINT32 *)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail!");
        }
        break;

    case SENSOR_CMD_GET_SENSOR_POWER_ON_STETE: /*LSC funciton need open after sensor Power On*/
        *((MUINT32*) arg1) = (mSensorIdx != IMGSENSOR_SENSOR_IDX_NONE) ? 1<<mSensorIdx : 0;
        break;

    case SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS: /*Pass1 deque fail would call this command*/
        ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_DEBUG_DUMP);
        ret = pSeninfDrv->sendCommand(CMD_SENINF_DEBUG_TASK);

        break;
    case SENSOR_CMD_DEBUG_GET_SENINF_METER:
        ret =  pSeninfDrv->sendCommand(CMD_SENINF_DEBUG_PIXEL_METER, arg1);
        break;
    case SENSOR_CMD_SET_SENINF_CAM_TG_MUX:
    {
        MUINT32 initMux = CAM_TG_ERR;
        SensorDynamicInfo *psensorDynamicInfo = &mSensorDynamicInfo;
        std::vector<SeninfConfigInfo>::iterator it = psensorDynamicInfo->config_infos.begin();
        for (; it != psensorDynamicInfo->config_infos.end(); ++it) {
            if(it->tg_info != *(MUINT32 *)arg1 || it->finalized)
                continue;
            it->tg_info = *(MUINT32 *)arg2;
            it->finalized = TRUE;
            initMux = CAM_SV_1;
            switch(it->feature_type){
                case VC_RAW_DATA:
                    psensorDynamicInfo->TgInfo = it->tg_info;
                    initMux = CAM_TG_1;
                    break;
                case VC_HDR_MVHDR:
                    psensorDynamicInfo->HDRInfo = it->tg_info;
                    break;
                case VC_PDAF_STATS:
                    psensorDynamicInfo->PDAFInfo = it->tg_info;
                    break;
                case VC_3HDR_EMBEDDED:
                    psensorDynamicInfo->CamInfo[EmbInfo] = it->tg_info;
                    break;
                case VC_3HDR_FLICKER:
                    psensorDynamicInfo->CamInfo[FlickerInfo] = it->tg_info;
                    break;
                case VC_3HDR_AE:
                    psensorDynamicInfo->CamInfo[AEInfo] = it->tg_info;
                    break;
                case VC_3HDR_Y:
                    psensorDynamicInfo->CamInfo[YInfo] = it->tg_info;
                    break;
                default:
                    break;
            }
            break;
        }
        if (initMux == CAM_TG_ERR)
            MY_LOGE("Error Cam MUX Setting originalCamTG=%d targetCamTG=%d", *(MUINT32 *)arg1, *(MUINT32 *)arg2);
        else
            MY_LOGD("SENSOR_CMD_SET_SENINF_CAM_TG_MUX: original = %d, target = %d", *(MUINT32 *)arg1, *(MUINT32 *)arg2);
#ifdef  CAM_MUX_CONFIGURABLE
        ret = pSeninfDrv->sendCommand(CMD_SENINF_FINALIZE_CAM_MUX, *(MUINT32 *)arg1, *(MUINT32 *)arg2);
#else
        ret = pSeninfDrv->setSeninfCamTGMuxCtrl(*(MUINT32 *)arg2 - initMux, (SENINF_MUX_ENUM)(*(MUINT32 *)arg1 - initMux));
        //ret = pSeninfDrv->setSeninfCamTGMuxCtrl(*(MUINT32 *)arg2 - initMux, (SENINF_MUX_ENUM)it->seninf_mux_info);
#endif

        break;
    }

    case SENSOR_CMD_SET_TEST_MODEL:
    {
#define HALSENSOR_TEST_MODEL_ISP_CLOCK 6000000
#define HALSENSOR_TEST_MODEL_VSYNC_TH  16
        /* FPS = ISP clock / ((TM_DUMMYPXL + TM_PXL) * (TM_VSYNC + TM_LINE))*/

        MUINT32 ispClock   = HALSENSOR_TEST_MODEL_ISP_CLOCK;
        MINT    dummyPixel = *(MUINT32 *)arg1 >> 7;
        MUINT32 vsync      = ispClock / *(MUINT32 *)arg3 - (*(MUINT32 *)arg1 + dummyPixel) - *(MUINT32 *)arg2;

        ret = pSeninfDrv->init();
        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_MUX1, SENINF_1);
        ret = pSeninfDrv->enableMUX(SENINF_MUX1);
        ret = pSeninfDrv->setSeninfMuxCtrl(SENINF_MUX1, 0, 0, TEST_MODEL, RAW_10BIT_FMT, 0);

        if(vsync < ((*(MUINT32 *)arg3) * (*(MUINT32 *)arg2))>>10) {
            MY_LOGW("[sendCommand] SENSOR_CMD_SET_TEST_MODEL fail, set to lower framerate! %d",
                    ispClock / ((dummyPixel + *(MUINT32 *)arg1) * (vsync + *(MUINT32 *)arg2)));
            ret = pSeninfDrv->setTestModel(true, dummyPixel, vsync, *(MUINT32 *)arg2, *(MUINT32 *)arg1);
        } else {
            ret = pSeninfDrv->setTestModel(true, dummyPixel, vsync, *(MUINT32 *)arg2, *(MUINT32 *)arg1);
        }
        break;
    }
    case SENSOR_CMD_GET_VERTICAL_BLANKING:
    {
        SensorCropWinInfo cropInfo;
        MUINT32 lineNum = 0;
        MUINT32 pixelClock = 0;
        MUINT32 frameRate = 0;

        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO, arg2 == NULL ?(MUINTPTR)&mScenarioId :arg2, (MUINTPTR)&lineNum);
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO, arg2 == NULL ?(MUINTPTR)&mScenarioId :arg2, (MUINTPTR)&cropInfo);
#ifdef GET_PCLK_BY_SCENARIO
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO, arg2 == NULL ?(MUINTPTR)&mScenarioId :arg2, (MUINTPTR)&pixelClock);
#else
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ, (MUINTPTR)&pixelClock);
#endif
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, arg2 == NULL ?(MUINTPTR)&mScenarioId :arg2, (MUINTPTR)&frameRate);

        if(pixelClock != 0 && frameRate != 0) {
            *((MUINT32*) arg1) = (MUINT32)((double)10000000 / frameRate - (double)cropInfo.h2_tg_size * (lineNum & 0xFFFF) * 1000000 / pixelClock);
        } else {
            *((MUINT32*) arg1) = 0;
            MY_LOGW("Wrong pixel clock or framerate %d %d", pixelClock, frameRate);
        }
        //MY_LOGD("SENSOR_CMD_GET_VERTICAL_BLANKING: vb_time = %d, lineNum = %d, pixelClock = %d, frameRate = %d\n",
        //    (*(MUINT32 *)arg1), lineNum, pixelClock, frameRate);
        break;
    }

    case SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER:
    {
        SensorCropWinInfo cropInfo;
        MUINT32 pixelClock = 0;
        MUINT32 lineLength = 0;

        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MUINTPTR)&lineLength);
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&mScenarioId, (MUINTPTR)&cropInfo);
        ret = pSensorDrv->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ, (MUINTPTR)&pixelClock);

        if (pixelClock != 0) {
            *((MUINT32*) arg1) = (MUINT32)((double)(lineLength & 0xFFFF) * cropInfo.h2_tg_size * 1000000000 / pixelClock); /* unit: ns */
        } else {
            *((MUINT32*) arg1) = 0;
            MY_LOGW("Wrong pixel clock");
        }
        break;
    }

#ifdef CONFIG_MTK_CAM_SECURE
    case SENSOR_CMD_SET_STREAMING_RESUME:
    {
        pSeninfDrv->sendCommand(CMD_SENINF_CHECK_PIPE);
        ret = pSensorDrv->sendCommand(cmd, arg1, arg2, arg3);
        if(ret < 0) {
            MY_LOGE("[sendCommand] sendCommand fail! %u", (MUINT32)cmd);
		}
		break;
    }
#endif
#ifdef CONFIG_MTK_CAM_SECURE
    case SENSOR_CMD_SET_STREAMING_SUSPEND:
    {
        pSeninfDrv->sendCommand(CMD_SENINF_RESUME_FREE);
        ret = pSensorDrv->sendCommand(cmd, arg1, arg2, arg3);
        if(ret < 0) {
            MY_LOGE("[sendCommand] sendCommand suspend fail! %d\n", cmd);
        }
        break;
    }
#endif

    default:
        ret = pSensorDrv->sendCommand(cmd, arg1, arg2, arg3);
        if(ret < 0) {
            MY_LOGE("[sendCommand] sendCommand fail! %u", (MUINT32)cmd);
        }
        break;
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT HalSensor::setSensorMclkDrivingCurrent(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MINT32 ret = 0;
    MUINT32 mclkSrc;
    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(sensorIdx);

    pSensorDrv->sendCommand(CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&mclkSrc);
#ifdef    MCLK_DRIVE_CURRENT_BY_PINCTRL
    ret = pSensorDrv->sendCommand(CMD_SENSOR_SET_DRIVE_CURRENT,(MUINTPTR)&pSensorDrv->getDrvInfo()->info.SensorDrivingCurrent);

#else
    SeninfDrv *const pSeninfDrv = SeninfDrv::getInstance();
    ret = pSeninfDrv->setMclkIODrivingCurrent(mclkSrc, pSensorDrv->getDrvInfo()->info.SensorDrivingCurrent);
#endif
    if (ret < 0) {
        MY_LOGE("The driving current for cam%d is wrong", sensorIdx);
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HalSensor::setDebugInfo(IBaseCamExif *pIBaseCamExif)
{
    (void)pIBaseCamExif;
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HalSensor::reset()
{
    MINT32 ret = 0;
    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(mSensorIdx);
#ifdef CONFIG_MTK_CAM_SECURE
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance(HalSensorList::singleton()->querySecureState());
#else
    SeninfDrv *pSeninfDrv = SeninfDrv::getInstance();
#endif

    Mutex::Autolock _l(mMutex);
#ifdef CONFIG_MTK_CAM_SECURE
    pSeninfDrv->sendCommand(CMD_SENINF_RESUME_FREE);
    pSensorDrv->sendCommand(CMD_SENSOR_SET_STREAMING_SUSPEND);
#endif
    if ((pSensorDrv->close()) < 0) {
        MY_LOGE("pSensorDrv->close fail");
        return MFALSE;
    }

    for (int j =0; j < 3; j++) {
        if ((ret = pSensorDrv->open()) < 0) {
            MY_LOGE("pSensorDrv->open fail, retry = %d ", j);
        }
        else {
            break;
        }
    }

    CUSTOM_CFG_CSI_PORT port;
    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&port);

    pSeninfDrv->reset(port);
    pSensorDrv->setScenario((MSDK_SCENARIO_ID_ENUM)mScenarioId, mFramerate, mHdrMode, mPdafMode);

#ifdef CONFIG_MTK_CAM_SECURE
    pSeninfDrv->sendCommand(CMD_SENINF_CHECK_PIPE);
#endif

    pSensorDrv->sendCommand(CMD_SENSOR_SET_STREAMING_RESUME);

    return ret;
}

MINT HalSensor::seninfLowPowerConfigure(MSDK_SCENARIO_ID_ENUM scenarioId)
{

    MUINT32 mipiPixelRate = 0;
    MUINT32 frameRate = 0;
    SensorCropWinInfo cropInfo;

    ImgSensorDrv *const pSensorDrv = ImgSensorDrv::getInstance(mSensorIdx);
    //SeninfDrv    *const pSeninfDrv = SeninfDrv::getInstance();

    SensorDynamicInfo *pSensorDynamicInfo = &mSensorDynamicInfo;
    SENSORDRV_INFO_STRUCT              *pSensorDrvInfo  = pSensorDrv->getDrvInfo();
    //ACDK_SENSOR_INFO_STRUCT            *pInfo           = &pSensorDrvInfo->info;
    //ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pResolutionInfo = &pSensorDrvInfo->resolutionInfo;

    pSensorDrv->sendCommand(CMD_SENSOR_GET_MIPI_PIXEL_RATE, (MUINTPTR)&scenarioId, (MUINTPTR)&mipiPixelRate);

    if (!mipiPixelRate) {
        pSensorDrv->sendCommand(CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&scenarioId, (MUINTPTR)&cropInfo);
        pSensorDrv->sendCommand(CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&scenarioId, (MUINTPTR)&frameRate);
        mipiPixelRate = cropInfo.w2_tg_size * cropInfo.h2_tg_size * frameRate * 0.125;
    }

    MY_LOGD("mipiPixelRate = %d", mipiPixelRate);

    //TODO: cam clock query from kernel
    pSensorDynamicInfo->pixelMode = (mipiPixelRate > 364000000 * 2) ? FOUR_PIXEL_MODE : TWO_PIXEL_MODE;

    //TODO: Dynamically adjust seninf clock

    return 0;
}

