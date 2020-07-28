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
#define LOG_TAG "af_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/acdk/cct_feature.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include "af_mgr.h"
#include "nvbuf_util.h"

#define AAA_TRACE_D(fmt, arg...) do{ CAM_TRACE_FMT_BEGIN(fmt, ##arg); } while(0)
#define AAA_TRACE_END_D do{ CAM_TRACE_FMT_END(); } while(0)

#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

using namespace NS3A;
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

static MVOID* threadGGyroSensor(MVOID* arg);

static MVOID* threadGGyroSensor(MVOID* arg)
{
    // thread auto detach after this function finish
    ::pthread_detach(pthread_self());

    ::prctl(PR_SET_NAME, "threadGGyroSensor", 0, 0, 0);
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
//    sched_p.sched_priority = NICE_CAMERA_ENABLE_GYROSENSOR;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, SCHED_OTHER, &sched_p);
    //::setpriority(PRIO_PROCESS, 0, priority);

    AfMgr *_this = static_cast<AfMgr*>(arg);
    _this->EnableGGyro();

    ::pthread_exit((MVOID*)0);
}

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

MVOID AfMgr::EnableGGyro()
{
    if(m_bGGyroEn)
    {
        Mutex::Autolock lock(gSensorCommonLock);
        //------------------------------------------- init g/gyro sensor listener -----------------------------------------
        // i4Coef[5] == 1:enable SensorListener, else: disable
        {

            if(/*(m_ptrNVRam->rAFNVRAM.i4SensorEnable > 0) && */(mpSensorManager == NULL) && (gListenEnAcce==MFALSE) && (gListenEnGyro==MFALSE))
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
                    if (mpEventQueue->enableSensor(mGyroSensorHandle, SENSOR_GYRO_POLLING_MS*1000 /* sample period */, 0 /* latency */) != Result::OK)
                    {
                        MY_ERR("enable Gyro Sensor FAIL!");
                        goto create_fail_exit;
                    }
                    else
                        gListenEnGyro = MTRUE;
                }
                if (mAcceSensorHandle != -1)
                {
                    if (mpEventQueue->enableSensor(mAcceSensorHandle, SENSOR_ACCE_POLLING_MS*1000 /* sample period */, 0 /* latency */) != Result::OK)
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
    else
    {
        Mutex::Autolock lock(gSensorCommonLock);
        // AF v1.2
        // --- uninit g/gyro sensor listener ---
        {
            if(/*(m_ptrNVRam->rAFNVRAM.i4SensorEnable > 0) &&*/ (mpSensorManager != NULL))
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
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SensorProvider_Init()
{
    AAA_TRACE_D("init gyro sensor");
    // GyroSensor init
    m_bGGyroEn = MTRUE;
    ::pthread_create(&m_ThreadGGyro, NULL, threadGGyroSensor, this);
    AAA_TRACE_END_D;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SensorProvider_Uninit()
{
    AAA_TRACE_D("uninit gyro sensor");
    // GyroSensor uninit
    m_bGGyroEn = MFALSE;
    ::pthread_create(&m_ThreadGGyro, NULL, threadGGyroSensor, this);
    AAA_TRACE_END_D;
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
            if(m_pIAfAlgo)
                m_pIAfAlgo->setAcceSensorInfo(gAcceInfo, SENSOR_ACCE_SCALE);
        }
        else
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF AcceInValid");
            if(m_pIAfAlgo)
                m_pIAfAlgo->setAcceSensorInfo(gAcceInfo, 0);    // set scale 0 means invalid to algo
        }

        if( gListenEnGyro && (gGyroTS!=gPreGyroTS) )
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF GyroValid");
            if(m_pIAfAlgo)
                m_pIAfAlgo->setGyroSensorInfo(gGyroInfo, SENSOR_GYRO_SCALE);
        }
        else
        {
            CAM_LOGD_IF((m_i4DgbLogLv&1),"SensorEventAF GyroInValid");
            if(m_pIAfAlgo)
                m_pIAfAlgo->setGyroSensorInfo(gGyroInfo, 0);    // set scale 0 means invalid to algo
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CmdSetAFMode( MINT32 eAFMode, MUINT32 u4Caller)
{
    if( m_i4EnableAF==0)
    {
        return S_AF_OK;
    }

    m_bIsFullScan = FALSE;
    MINT32 mtkAFMode;
    switch(eAFMode)
    {
    case AF_MODE_AFS: // AF-Single Shot Mode
    case AF_MODE_INFINITY: // Focus is set at infinity //[todo] Mapping infinity to new afmode
    case AF_MODE_FULLSCAN: // AF Full Scan Mode
        mtkAFMode = MTK_CONTROL_AF_MODE_AUTO;
        if(eAFMode==AF_MODE_FULLSCAN)
            m_bIsFullScan = TRUE;
        break;
    case AF_MODE_AFC: // AF-Continuous Mode
        mtkAFMode = MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        break;
    case AF_MODE_AFC_VIDEO: // AF-Continuous Mode (Video)
        mtkAFMode = MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO;
        break;
    case AF_MODE_MACRO: // AF Macro Mode
        mtkAFMode = MTK_CONTROL_AF_MODE_MACRO;
        break;
    case AF_MODE_MF: // Manual Focus Mode
        mtkAFMode = MTK_CONTROL_AF_MODE_OFF;
        break;
    default:
        mtkAFMode = MTK_CONTROL_AF_MODE_AUTO;
        break;
    }

    if( m_eCurAFMode==mtkAFMode)
    {
        return S_AF_OK;
    }

    /**
     *  Before new af mode is set, setting af area command is sent.
     */
    if( u4Caller==AF_MGR_CALLER)
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  mtkAFMode);
    }
    else
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d):ctl_afmode(%d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  mtkAFMode);
    }



    m_sHostCmd.u4AfMode = mtkAFMode;
    sendAFRequest(AF_CMD_SETAFMODE, (MINTPTR)&m_sHostCmd);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CmdSetAFArea( CameraFocusArea_T &sInAFArea)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): [Cnt]%d [L]%d [R]%d [U]%d [B]%d",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              sInAFArea.u4Count,
              sInAFArea.rAreas[0].i4Left,
              sInAFArea.rAreas[0].i4Right,
              sInAFArea.rAreas[0].i4Top,
              sInAFArea.rAreas[0].i4Bottom);

    memcpy( &m_sHostCmd.rFocusAreas, &sInAFArea, sizeof(CameraFocusArea_T));
    sendAFRequest(AF_CMD_SETAFAREA, (MINTPTR)&m_sHostCmd);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CmdSetMFPos( MINT32 a_i4Pos, MUINT32 u4Caller)
{
    if( u4Caller==AF_MGR_CALLER)
    {
        CAM_LOGD( "%s Dev %d : %d",
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  a_i4Pos);
    }
    else
    {
        CAM_LOGD_IF( m_i4DgbLogLv,
                     "cmd-%s Dev %d : %d",
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     a_i4Pos);
    }

    m_sHostCmd.i4MFPos = a_i4Pos;
    sendAFRequest(AF_CMD_SETMFPOS, (MINTPTR)&m_sHostCmd);

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::CmdAutoFocus()
{
    if(m_i4EnableAF == 0)
    {
        CAM_LOGD("autofocus : disableAF or dummy lens");
        CallbackNotify_Timeout();
        return;
    }
    m_sHostCmd.bIsAutoFocusFromHost = MTRUE;
    sendAFRequest(AF_CMD_AUTOFOCUS, (MINTPTR)&m_sHostCmd);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::CmdCancelAutoFocus()
{
    m_sHostCmd.bIsCancelAutoFocusFromHost=MTRUE;
    sendAFRequest(AF_CMD_CANCELAUTOFOCUS, (MINTPTR)&m_sHostCmd);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::CmdSetFocusDistance( MFLOAT lens_focusDistance)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): lens_focusDistance = %f",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              lens_focusDistance);

    if( lens_focusDistance<0 || !m_pAFParam)
    {
        return;
    }


    m_sHostCmd.fFocusDistance = lens_focusDistance;
    sendAFRequest(AF_CMD_SETFOCUSDISTANCE, (MINTPTR)&m_sHostCmd);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CmdTriggerAF( MUINT32 u4Caller)
{
    if( u4Caller==AF_MGR_CALLER)
        CAM_LOGD( "%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);
    else
        CAM_LOGD( "cmd-%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    sendAFRequest(AF_CMD_TRIGGERAF, (MINTPTR)&m_sHostCmd);
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CmdSetFullScanstep( MINT32 a_i4Step)
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): %d",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              a_i4Step);

    m_sHostCmd.i4FullScanStep = a_i4Step;
    sendAFRequest(AF_CMD_SETFULLSCANSTEP, (MINTPTR)&m_sHostCmd);
    return S_AF_OK;
}


/************** AF Request related API *****************************************/
MBOOL AfMgr::sendAFRequest(AF_CMD_T const r3ACmd, MUINTPTR const i4Arg)
{
    HostCmdQ_T rReq;
    rReq.afCmd = r3ACmd;
    if (i4Arg != 0)
        rReq.rParam = *reinterpret_cast<HostCmdParam_t*>(i4Arg);
    CAM_LOGD_IF(m_i4DgbLogLv, "[%s] rAFHostCmd(%d)\n", __FUNCTION__, r3ACmd);

    addAFRequestQ(rReq);
    return MTRUE;
}
// add rReq to mHostReqQ
MVOID AfMgr::addAFRequestQ(HostCmdQ_T &rReq)
{
    Mutex::Autolock autoLock(mReqQMtx);

    mHostReqQ.push_back(rReq);

    CAM_LOGD_IF(m_i4DgbLogLv, "[%s] mHostReqQ size(%d) + cmd(%d)", __FUNCTION__, (MINT32)mHostReqQ.size(), rReq.afCmd);
}

MVOID AfMgr::clearAFRequestQ()
{
    FUNC_BEGIN;
    Mutex::Autolock autoLock(mReqQMtx);

    MINT32 Qsize = mHostReqQ.size();

    for (List<HostCmdQ_t>::iterator it = mHostReqQ.begin(); it != mHostReqQ.end();)
    {
        AF_CMD_T afCmd = it->afCmd;
        CAM_LOGD("[%s] cmd size(%d), clear(%d)", __FUNCTION__, (MINT32)mHostReqQ.size(), afCmd);
        it = mHostReqQ.erase(it);
    }
    CAM_LOGD("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
}

MBOOL AfMgr::getAFRequest(HostCmdQ_T &rReq, MINT32 i4Count)
{
    if(m_i4DgbLogLv)
    {
        FUNC_BEGIN;
    }

    Mutex::Autolock autoLock(mReqQMtx);
    CAM_LOGD_IF(m_i4DgbLogLv, "[%s] mCmdQ.size()=%d ", __FUNCTION__, (MINT32)mHostReqQ.size());

    if (mHostReqQ.size() == 0) //this is only for en_timeout == 1 & timeout case
    {
        CAM_LOGD_IF(m_i4DgbLogLv, "[%s] mCmdQ.size() = %d after mCmdQCond.wait/waitRelative\n", __FUNCTION__, (MINT32)mHostReqQ.size());
        return MFALSE;
    }

    rReq = *mHostReqQ.begin();
    CAM_LOGD_IF(m_i4DgbLogLv, "mReqQ-size(%d), eCmd(%d)", (MINT32)mHostReqQ.size(), rReq.afCmd);

    mHostReqQ.erase(mHostReqQ.begin());

    if(m_i4DgbLogLv)
    {
        FUNC_END;
    }

    return MTRUE;
}

MVOID AfMgr::QuerryAFRequest()
{
    // ISP3.0/ISP4.0 compatible
    // SetCropRegion SHOULD be called before ExeSetAFArea to get the correct m_sCropRegionInfo.
    HostCmdQ_T rReq;
    MINT32 i4RequestCount = 0;
    while(getAFRequest(rReq, i4RequestCount))
    {
        i4RequestCount++;
        CAM_LOGD_IF(m_i4DgbLogLv, "[%s] getAFRequest \n", __FUNCTION__);
        switch(rReq.afCmd)
        {
        case AF_CMD_AUTOFOCUS:
            autoFocus(m_sHostCmd.bIsAutoFocusFromHost);
            break;
        case AF_CMD_CANCELAUTOFOCUS:
            cancelAutoFocus(m_sHostCmd.bIsCancelAutoFocusFromHost);
            break;
        case AF_CMD_SETAFAREA:
            setAFArea(m_sHostCmd.rFocusAreas);
            break;
        case AF_CMD_SETAFMODE:
            setAFMode(m_sHostCmd.u4AfMode, AF_CMD_CALLER);
            break;
        case AF_CMD_SETFULLSCANSTEP:
            setFullScanstep(m_sHostCmd.i4FullScanStep);
            break;
        case AF_CMD_SETMFPOS:
            setMFPos(m_sHostCmd.i4MFPos, AF_CMD_CALLER);
            break;
        case AF_CMD_SETZOOMWININFO:
            SetCropRegionInfo(m_sHostCmd.rCropRegion.i4X, m_sHostCmd.rCropRegion.i4Y, m_sHostCmd.rCropRegion.i4W, m_sHostCmd.rCropRegion.i4H, AF_CMD_CALLER);
            break;
        case AF_CMD_SETSENSORMODE:
            setCamScenarioMode( m_sHostCmd.a_eCamScenarioMode);
            break;
        case AF_CMD_SETFOCUSDISTANCE:
            setFocusDistance(m_sHostCmd.fFocusDistance);
            break;
        case AF_CMD_TRIGGERAF:
            triggerAF(AF_CMD_CALLER);
            break;
        default:
            CAM_LOGD("%s getAFRequest invalid cmd");
            break;
        }
    }

    if(m_bIsPause_Last!=MTRUE)
    {
        SetPauseAF(m_bIsPause_Last);
    }
}
