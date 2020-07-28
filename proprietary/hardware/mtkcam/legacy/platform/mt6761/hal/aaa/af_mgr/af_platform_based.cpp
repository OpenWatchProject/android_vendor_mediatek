/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "af_mgr_v3"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
//#include <mtkcam/featureio/aaa_hal_if.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/acdk/cct_feature.h>
#include <af_feature.h>
#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <af_algo_if.h>
#include "af_mgr.h"
#include "nvbuf_util.h"

#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF v1.2
// g/gyro sensor listener handler and data
static MBOOL gListenEnAcce = MFALSE;
static MINT32 gAcceInfo[3];
static MUINT64 gAcceTS;
static MUINT64 gPreAcceTS;
static MBOOL gListenEnGyro = MFALSE;
static MINT32 gGyroInfo[3];
static MUINT64 gGyroTS;
static MUINT64 gPreGyroTS;
static MBOOL gListenLogOn = MFALSE;

static Mutex  gSensorCommonLock;
static Mutex  gSensorEventLock;
static sp<ISensorManager> mpSensorManager = NULL;
static sp<IEventQueue>    mpEventQueue = NULL;
static sp<AfSensorListener> mpAfSensorListener = NULL;
static MINT32             mGyroSensorHandle = 0;
static MINT32             mAcceSensorHandle = 0;

Return<void> AfSensorListener::onEvent(const Event &e)
{
    sensors_event_t sensorEvent;
    android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);

    Mutex::Autolock lock(gSensorEventLock);

    switch(e.sensorType)
    {
    case SensorType::ACCELEROMETER:
        if(gListenLogOn)
        {
            MY_LOG("Acc(%f,%f,%f,%lld)",
                   sensorEvent.acceleration.x,
                   sensorEvent.acceleration.y,
                   sensorEvent.acceleration.z,
                   sensorEvent.timestamp);
        }
        gPreAcceTS = gAcceTS;
        gAcceInfo[0] = sensorEvent.acceleration.x * SENSOR_ACCE_SCALE;
        gAcceInfo[1] = sensorEvent.acceleration.y * SENSOR_ACCE_SCALE;
        gAcceInfo[2] = sensorEvent.acceleration.z * SENSOR_ACCE_SCALE;
        gAcceTS = sensorEvent.timestamp;
        break;

    case SensorType::GYROSCOPE:
        if(gListenLogOn)
        {
            MY_LOG("Gyro(%f,%f,%f,%lld)",
                   sensorEvent.gyro.x,
                   sensorEvent.gyro.y,
                   sensorEvent.gyro.z,
                   sensorEvent.timestamp);
        }
        gPreGyroTS = gGyroTS;
        gGyroInfo[0] = sensorEvent.gyro.x * SENSOR_GYRO_SCALE;
        gGyroInfo[1] = sensorEvent.gyro.y * SENSOR_GYRO_SCALE;
        gGyroInfo[2] = sensorEvent.gyro.z * SENSOR_GYRO_SCALE;
        gGyroTS = sensorEvent.timestamp;
        break;

    default:
        MY_LOG("unknown type(%d)",sensorEvent.type);
        break;
    }

    return android::hardware::Void();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SensorProvider_Init()
{
    //------------------------------------------- init g/gyro sensor listener -----------------------------------------
    // i4Coef[5] == 1:enable SensorListener, else: disable
    {
        Mutex::Autolock lock(gSensorCommonLock);

        if((m_ptrNVRam->rAFNVRAM.i4SensorEnable > 0) && (mpSensorManager == NULL) && (gListenEnAcce==MFALSE) && (gListenEnGyro==MFALSE))
        {
            //============ enableSensor ============
            mpSensorManager = ISensorManager::getService();

            if (mpSensorManager == NULL)
            {
                MY_ERR("get SensorManager FAIL!");
                goto create_fail_exit;
            }

            mpSensorManager->getDefaultSensor(SensorType::GYROSCOPE,
                                              [&](const SensorInfo& sensor, Result ret)
            {
                ret == Result::OK ? mGyroSensorHandle = sensor.sensorHandle : mGyroSensorHandle = -1;
            });
            mpSensorManager->getDefaultSensor(SensorType::ACCELEROMETER,
                                              [&](const SensorInfo& sensor, Result ret)
            {
                ret == Result::OK ? mAcceSensorHandle = sensor.sensorHandle : mAcceSensorHandle = -1;
            });

            if (mGyroSensorHandle == -1 || mAcceSensorHandle == -1)
            {
                MY_ERR("get DefaultSensor FAIL! %d %d", mGyroSensorHandle, mAcceSensorHandle);
            }
            //create SensorEventQueue and register callback
            mpAfSensorListener = new AfSensorListener();
            mpSensorManager->createEventQueue(mpAfSensorListener,
                                              [&](const sp<IEventQueue>& queue, Result ret)
            {
                ret == Result::OK ? mpEventQueue = queue : mpEventQueue = NULL;
            });

            if (mpEventQueue == NULL)
            {
                MY_ERR("createEventQueue FAIL!");
                goto create_fail_exit;
            }
            //enable sensor
            if (mGyroSensorHandle != -1)
            {
                if (mpEventQueue->enableSensor(mGyroSensorHandle, SENSOR_GYRO_POLLING_MS /* sample period */, 0 /* latency */) != Result::OK)
                {
                    MY_ERR("enable Gyro Sensor FAIL!");
                    goto create_fail_exit;
                }
                else
                    gListenEnGyro = MTRUE;
            }
            if (mAcceSensorHandle != -1)
            {
                if (mpEventQueue->enableSensor(mAcceSensorHandle, SENSOR_ACCE_POLLING_MS /* sample period */, 0 /* latency */) != Result::OK)
                {
                    MY_ERR("enable Acce Sensor FAIL!");
                    goto create_fail_exit;
                }
                else
                    gListenEnAcce = MTRUE;
            }

create_fail_exit:
            if( (gListenEnAcce!=MTRUE) || (gListenEnGyro!=MTRUE))
            {
                MY_ERR("sensor fail : Acc(%d) Gyro(%d)", gListenEnAcce, gListenEnGyro);
            }
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SensorProvider_Uninit()
{
    // AF v1.2
    // --- uninit g/gyro sensor listener ---
    {
        Mutex::Autolock lock(gSensorCommonLock);

        if((m_ptrNVRam->rAFNVRAM.i4SensorEnable > 0) && (mpSensorManager != NULL))
        {
            if (mpSensorManager != NULL)
            {
                mpSensorManager = NULL;
            }
            if (mpEventQueue != NULL)
            {
                if(gListenEnGyro)
                {
                    MY_LOG("[%s] mpEventQueue->disableSensor(Gyro) + \n", __FUNCTION__);
                    ::android::hardware::Return<Result> _ret = mpEventQueue->disableSensor(mGyroSensorHandle);
                    if (!_ret.isOk())
                    {
                        MY_ERR("disableSensor fail");
                    }
                    gListenEnGyro = MFALSE;
                    MY_LOG("[%s] mpEventQueue->disableSensor(Gyro) - \n", __FUNCTION__);
                }

                if(gListenEnAcce)
                {
                    MY_LOG("[%s] mpEventQueue->disableSensor(Acce) + \n", __FUNCTION__);
                    ::android::hardware::Return<Result> _ret = mpEventQueue->disableSensor(mAcceSensorHandle);
                    if (!_ret.isOk())
                    {
                        MY_ERR("disableSensor fail");
                    }
                    gListenEnAcce = MFALSE;
                    MY_LOG("[%s] mpEventQueue->disableSensor(Acce) - \n", __FUNCTION__);
                }
                mpEventQueue = NULL;
            }
            if (mpAfSensorListener != NULL)
            {
                mpAfSensorListener = NULL;
            }
        }
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SensorProvider_getData()
{
    // i4Coef[5] == 1:enable SensorListener, 0: disable
    if(m_ptrNVRam->rAFNVRAM.i4SensorEnable > 0)
    {
        CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF gAcceValid=%d gPreAcceTS=%lld gAcceTS=%lld gAcceInfo[]=(%d %d %d)\n"
                    , gListenEnAcce, gPreAcceTS, gAcceTS, gAcceInfo[0], gAcceInfo[1], gAcceInfo[2]);
        CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF gGyroValid=%d gPreGyroTS=%lld gGyroTS=%lld gGyroInfo[]=(%d %d %d)\n"
                    , gListenEnGyro, gPreGyroTS, gGyroTS, gGyroInfo[0], gGyroInfo[1], gGyroInfo[2]);
        if( gListenEnAcce && (gAcceTS!=gPreAcceTS) )
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF AcceValid");
            m_pIAfAlgo->setAcceSensorInfo(gAcceInfo, SENSOR_ACCE_SCALE);
        }
        else
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF AcceInValid");
            m_pIAfAlgo->setAcceSensorInfo(gAcceInfo, 0);    // set scale 0 means invalid to algo
        }

        if( gListenEnGyro && (gGyroTS!=gPreGyroTS) )
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF GyroValid");
            m_pIAfAlgo->setGyroSensorInfo(gGyroInfo, SENSOR_GYRO_SCALE);
        }
        else
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF GyroInValid");
            m_pIAfAlgo->setGyroSensorInfo(gGyroInfo, 0);    // set scale 0 means invalid to algo
        }
    }
}
