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
#define LOG_TAG "TemperatureMonitor"

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include "TemperatureMonitor.h"
#include <cutils/properties.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace NSCam::v3;
//
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
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END MY_LOGD("-")

#define MAX_OBSERVE_NUM 4
#define TICK_TIMER_PERIODIC 2000 //ms
#define FILENAME "//proc//driver//thermal//imgsensor_temp"
//#define FILENAME "//sdcard//ckh_temp"
/******************************************************************************
 *
 ******************************************************************************/
sp<ITemperatureMonitor>
ITemperatureMonitor::
create()
{
    return new TemperatureMonitor();
}
/******************************************************************************
 *
 ******************************************************************************/
TemperatureMonitor::
TemperatureMonitor()
{
    sem_init(&semTicker, 0, 0);
	MY_LOGD("ctor (%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
TemperatureMonitor::
~TemperatureMonitor()
{
    MY_LOGD("dctor (%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TemperatureMonitor::
init()
{
    IHalSensorList* sensorList = MAKE_HalSensorList();
    if (sensorList == NULL)
    {
        MY_LOGE("get sensor obj failed");
        return MFALSE;
    }
    miSensorNum = sensorList->queryNumberOfSensors();
    MY_LOGD("Sensor num: %d", miSensorNum);
    //
    MUINT32 size = mvObserveList.size();
    if(size == 0)
    {
        MY_LOGE("Init fail. ObserveList.size is 0");
        return MFALSE;
    }
    if(size != MAX_OBSERVE_NUM)
    {
        for(MINT32 i = size; i<MAX_OBSERVE_NUM ; ++i)
        {
            mvObserveList.add(i+0xF0, new SensorTemperature(0xffff, 0xffff, MFALSE));
        }
    }
    // init sensor
    for(size_t i=0;i<mvObserveList.size();++i)
    {
        mvObserveList.valueAt(i)->init();
    }
    std::string sTemperature = getTemperature(MTRUE);
    MY_LOGD("init value: %s", sTemperature.c_str());
    //
    {
        miTick_Timer_Periodic = TICK_TIMER_PERIODIC;
        char cProperty[PROPERTY_VALUE_MAX];
        ::property_get("vendor.debug.camera.temperature.timer", cProperty, "0");
        MUINT32 temperature = ::atoi(cProperty);
        if(0 != temperature)
        {
            miTick_Timer_Periodic = temperature;
            MY_LOGD("Manually set timer: %d ms", miTick_Timer_Periodic);
        }
    }
    {
        char cProperty[PROPERTY_VALUE_MAX];
        ::property_get("vendor.debug.camera.log.temperature", cProperty, "0");
        MUINT32 value = ::atoi(cProperty);
        if(0 != value)
        {
            miLogLevel = value;
        }
    }
    mbTimerStop = MFALSE;
    sem_init(&semTicker, 0, 0);
    // open file
    mOutFile.open(FILENAME, std::fstream::out|std::fstream::trunc);
    if(!mOutFile)
    {
        MY_LOGE("Open file error");
    }
    writeToFile(sTemperature);
lbExit:
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TemperatureMonitor::
uninit()
{
    FUNC_START;
    MUINT32 size = mvObserveList.size();
    if(size>0)
    {
        for(MUINT32 i=0;i<size;++i)
        {
            SensorTemperature* value = mvObserveList.editValueAt(i);
            value->uninit();
        }
    }
    std::string value = getTemperature(MFALSE);
    writeToFile(value);
    mvObserveList.clear();
    if(mOutFile)
    {
        mOutFile.close();
    }
    FUNC_END;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TemperatureMonitor::
addToObserve(MUINT32 iOpenId, MUINT32 iSensorDev)
{
    if(iOpenId > miSensorNum - 1)
    {
        MY_LOGE("Invalid openid (%d) sensorNum (%d)", iOpenId, miSensorNum);
        return MFALSE;
    }
    if(mvObserveList.size() > MAX_OBSERVE_NUM)
    {
        MY_LOGE("ObserveList is full mvObserveList.size(%zu) max size(%d)", mvObserveList.size(), MAX_OBSERVE_NUM);
        return MFALSE;
    }
    ssize_t index = mvObserveList.indexOfKey(iOpenId);
    if(index < 0)
    {
        mvObserveList.add(iOpenId, new SensorTemperature(iOpenId, iSensorDev));
        MY_LOGD("add value, key(%d) value(%d)", iOpenId, iSensorDev);
    }
    else
    {
        MY_LOGE("OpenId is exist");
        return MFALSE;
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
TemperatureMonitor::
threadLoop()
{
    sem_timedwait_millsecs(semTicker, miTick_Timer_Periodic);
    if(!mbTimerStop)
    {
        std::string sTemperature = getTemperature(MFALSE);
        MY_LOGD_IF(miLogLevel >= 1, "value: %s", sTemperature.c_str());
        writeToFile(sTemperature);
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
void
TemperatureMonitor::
requestExit()
{
    ITemperatureMonitor::requestExit();
    mbTimerStop = MTRUE;
    sem_post(&semTicker);
    sem_destroy(&semTicker);
}
/******************************************************************************
 *
 ******************************************************************************/
int
TemperatureMonitor::
sem_timedwait_millsecs(
    sem_t &sem,
    long msecs
)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long secs = msecs/1000;
    msecs = msecs%1000;

    long add = 0;
    msecs = msecs*1000*1000 + ts.tv_nsec;
    add = msecs / (1000*1000*1000);
    ts.tv_sec += (add + secs);
    ts.tv_nsec = msecs%(1000*1000*1000);

    return sem_timedwait(&sem, &ts);
}
/******************************************************************************
 *
 ******************************************************************************/
std::string
TemperatureMonitor::
getTemperature(
    MBOOL bNeedSensorName
)
{
    std::string sTemperature = std::to_string(mvObserveList.valueAt(0)->getTemperature());
    for(size_t i = 1;i < mvObserveList.size(); ++i)
    {
        sTemperature += std::string(",") + std::to_string(mvObserveList.valueAt(i)->getTemperature());
    }
    std::string sSensorName = "";
    if(bNeedSensorName)
    {
        sSensorName = mvObserveList.valueAt(0)->getSensorName();
        for(size_t i = 1;i < mvObserveList.size(); ++i)
        {
            sSensorName += std::string(",") + mvObserveList.valueAt(i)->getSensorName();
        }
        return (sTemperature + std::string(",") + sSensorName);
    }
    return (sTemperature);
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TemperatureMonitor::
writeToFile(
    std::string content
)
{
    if(mOutFile)
    {
        mOutFile<<content.c_str()<<std::endl;
        mOutFile.flush();
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
TemperatureMonitor::
SensorTemperature::
SensorTemperature(
    MUINT32 iOpenId,
    MUINT32 iSensorDevId,
    MBOOL valid
) : miOpenId(iOpenId),
    miSensorDevId(iSensorDevId),
    mbValid(valid)
{
    MY_LOGD("ctor sensor id (%d) dev (%d) valid (%d)", miOpenId, miSensorDevId, mbValid);
}
/******************************************************************************
 *
 ******************************************************************************/
TemperatureMonitor::
SensorTemperature::
~SensorTemperature()
{
    MY_LOGD("dctor sensor id (%d) dev (%d)", miOpenId, miSensorDevId);
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TemperatureMonitor::
SensorTemperature::
init()
{
    if(!mbValid)
    {
        return MTRUE;
    }
    IHalSensorList* sensorList = MAKE_HalSensorList();
    if (sensorList == nullptr)
    {
        return MFALSE;
    }
    mpIHalSensor = sensorList->createSensor(LOG_TAG, miOpenId);
    if(mpIHalSensor != nullptr)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TemperatureMonitor::
SensorTemperature::
uninit()
{
    if(!mbValid)
    {
        return MTRUE;
    }
    if(mpIHalSensor!=nullptr)
    {
        mpIHalSensor->destroyInstance(LOG_TAG);
    }
    mbValid = MFALSE;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
TemperatureMonitor::
SensorTemperature::
getTemperature()
{
    if(!mbValid)
    {
        return -275;
    }
    MINT32 temperature = 0;
    if(mpIHalSensor!=nullptr)
    {
        mpIHalSensor->sendCommand(miSensorDevId, SENSOR_CMD_GET_TEMPERATURE_VALUE,
                            (MINTPTR)& temperature, 0, 0);
    }
    return temperature;
}
/******************************************************************************
 *
 ******************************************************************************/
std::string
TemperatureMonitor::
SensorTemperature::
getSensorName()
{
    if(!mbValid)
    {
        return "null";
    }
    msSensorName = MAKE_HalSensorList()->queryDriverName(miOpenId);
    return msSensorName;
}
