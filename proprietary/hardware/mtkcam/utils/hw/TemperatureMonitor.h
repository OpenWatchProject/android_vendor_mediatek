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
/**
 * @file TemperatureMonitor.h
 * @brief Image sensor temperature monitor.
*/
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_TEMPERATUREMONITOR_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_TEMPERATUREMONITOR_H_
//
// Standard C header file
#include <string>
#include <semaphore.h>
#include <fstream>
// Android system/core header file
#include <utils/KeyedVector.h>
// mtkcam custom header file
// mtkcam global header file
// Module header file
#include <mtkcam/utils/hw/ITemperatureMonitor.h>
// Local header file
/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace android;
namespace NSCam{
namespace v3{
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class TemperatureMonitor
 * @brief use to monitor image sensor temperature, and write to specific file.
 */
class TemperatureMonitor : public ITemperatureMonitor
{
public:
    /**
     * @brief construct
     */
    TemperatureMonitor();
    /**
     * @brief destruct
     */
    virtual ~TemperatureMonitor();
    /**
     * @brief init temperature monitor and create SensorTemperator instance.
     * @return if init success return MTRUE.
     */
    MBOOL init() override;
    /**
     * @brief uninit temperature monitor and release SensorTemperator instance.
     * @return if uninit success return MTRUE.
     */
    MBOOL uninit() override;
    /**
     * @brief add sensor to observe list.
     * @param [in] iOpenId open id.
     * @param [in] iSensorDev sensor dev id.
     * @return If add success return MTURE.
     */
    MBOOL addToObserve(MUINT32 iOpenId, MUINT32 iSensorDev) override;
    /**
     * @brief update logic. It will query to sensor driver to get temperature.
     * @return always return true.
     */
    bool threadLoop() override;
    /**
     * @brief request to exit thread loop.
     */
    void requestExit() override;
protected:
    /**
     * @brief wait specific time.
     * @param [in] sem semaphore use to wait specific time
     * @param [in] msecs ms
     * @return If set semaphore succuss return status.
     */
    int sem_timedwait_millsecs(sem_t &sem, long msecs);
    /**
     * @brief get image sensor temperature
     * @param [in] bNeedSensorName set true to add sensor name.
     * @return return temperature string
     */
    std::string getTemperature(MBOOL bNeedSensorName);
    /**
     * @brief write string to specific file
     * @param [in] content string
     * @return return true
     */
    MBOOL writeToFile(std::string content);
protected:
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class SensorTemperature
 * @brief use to query sensor information, such as SensorTemperature and sensor name.
 */
    class SensorTemperature : public virtual RefBase
    {
    public:
        /**
         * @brief default construct is not exist.
         */
        SensorTemperature() = delete;
        /**
         * @brief construct
         * @param [in] iOpenId open id.
         * @param [in] iSensorDevId sensor dev id.
         * @param [in] valid if sensor id is valid set ture.
         */
        SensorTemperature(MUINT32 iOpenId, MUINT32 iSensorDevId, MBOOL valid = MTRUE);
        /**
         * @brief destruct
         */
        ~SensorTemperature();
        /**
         * @brief init SensorTemperature and create sensor instance.
         * @return If init success return MTRUE
         */
        MBOOL init();
        /**
         * @brief unint SensorTemperature and sensor instance.
         * @return If uninit success return MTRUE
         */
        MBOOL uninit();
        /**
         * @brief get SensorTemperature
         * @return return sensor SensorTemperature
         */
        MINT32 getTemperature();
        /**
         * @brief get sensor name
         * @return return sensor name
         */
        std::string getSensorName();
    private:
        MUINT32 miOpenId = -1;
        MUINT32 miSensorDevId = -1;
        std::string msSensorName = "";
        MBOOL mbValid = MFALSE;
        IHalSensor* mpIHalSensor = nullptr;
    };
private:
    MUINT32 miSensorNum = 0;
    DefaultKeyedVector<MUINT32, SensorTemperature*> mvObserveList;
    MUINT32 miTick_Timer_Periodic = 0;
    sem_t   semTicker;
    MBOOL   mbTimerStop = MFALSE;
    std::fstream mOutFile;
    MINT32 miLogLevel = 0;
};
};
};
#endif