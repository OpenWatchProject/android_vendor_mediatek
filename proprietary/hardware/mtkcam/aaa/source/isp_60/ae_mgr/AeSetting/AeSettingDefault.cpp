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
#define LOG_TAG "AeSettingDefault"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeSettingDefault.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
 #include <aaa_sensor_mgr.h>
#include <aaa_scheduler.h>
#include <ae_mgr.h>


#define AE_SETTING_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_SETTING_LOG_IF(cond, fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE_IF(cond, fmt, ##arg); \
        } else { \
            CAM_LOGD_IF(cond, fmt, ##arg); \
        } \
    }while(0)

#define EN_3A_SCHEDULE_LOG    2

using namespace NS3Av3; 
static std::array<MY_INST_AESETTING, SENSOR_IDX_MAX> gMultiton;

IAeSetting*
AeSettingDefault::
getInstance(ESensorDev_T sensor)
{
    AE_SETTING_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_AESETTING& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeSettingDefault>(sensor);
    } );

    return rSingleton.instance.get();
}

AeSettingDefault::
AeSettingDefault(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_eSensorMode(ESensorMode_Preview)
    , m_3ALogEnable(MFALSE)
    , m_bSetShutterValue(MFALSE)
    , m_bSetGainValue(MFALSE)
    , m_bSetFrameRateValue(MFALSE)
    , m_pSensorGainThread(NULL)
    , m_bSGainThdFlg(MFALSE)
    , m_eSensorTG(ESensorTG_1)
    , m_i4SensorIdx(0)
    , m_eAETargetMode(AE_MODE_NORMAL)
{
    AE_SETTING_LOG("Enter AeSettingDefault DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeSettingDefault::
init()
{
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
    m_bSetShutterValue = MFALSE;
    m_bSetGainValue = MFALSE;
    m_bSetFrameRateValue = MFALSE;
}

MVOID
AeSettingDefault::
uninit()
{
    m_bSGainThdFlg = MFALSE;
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
}

MVOID
AeSettingDefault::
start(MVOID* pinputinfo)
{
    m_bSetShutterValue = MFALSE;
    m_bSetGainValue = MFALSE;
    m_bSetFrameRateValue = MFALSE;

    AE_SETTING_INPUT_INFO_T* pInput = (AE_SETTING_INPUT_INFO_T*) pinputinfo;
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    m_i4SensorIdx = pInput->i4SensorIdx;
    m_eSensorTG   = pInput->eSensorTG;
    m_eSensorMode = pInput->eSensorMode;
    m_eAETargetMode = pInput->eAETargetMode;
    // Create Sensor Gain thread for Type-B sensor
    if ((m_bSGainThdFlg == MTRUE)&&(m_pSensorGainThread == NULL)) {
        m_pSensorGainThread = ThreadSensorGain::createInstance(m_eSensorDev, m_i4SensorIdx);
    } else {
        AE_SETTING_LOG( "[%s()] Sensor gain thread status Flg/Thread : %d/%d \n", __FUNCTION__, m_bSGainThdFlg,(m_pSensorGainThread == NULL));
    }
}

MVOID
AeSettingDefault::
stop()
{
    if (m_pSensorGainThread){
        m_pSensorGainThread->destroyInstance();
        m_pSensorGainThread = NULL;
    }
    AE_SETTING_LOG( "[%s()]\n", __FUNCTION__);
}

MBOOL
AeSettingDefault::
queryStatus(MUINT32 index)
{
    E_AE_SETTING_CPU_STATUS_T eStatus = (E_AE_SETTING_CPU_STATUS_T) index;
    switch (eStatus)
    {
        case E_AE_SETTING_CPU_STATUS_NEEDUPDATED:
            return (m_bSetFrameRateValue||m_bSetShutterValue||m_bSetGainValue);
        case E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD:
            return m_bSGainThdFlg;

        default:
            AE_SETTING_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
            return MFALSE;
    }
}

MVOID
AeSettingDefault::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_SETTING_CPU_TRIGER_T etrigger = (E_AE_SETTING_CPU_TRIGER_T) index;
    switch(etrigger){
        case E_AE_SETTING_CPU_SENSORGAINTHREAD:
            if (m_pSensorGainThread == NULL)
                m_bSGainThdFlg = MTRUE;
            break;
        case E_AE_SETTING_CPU_FRAMERATE:
                m_bSetFrameRateValue = btrigger;
            break;
        case E_AE_SETTING_CPU_SHUTTER:
                m_bSetShutterValue= btrigger;
            break;
        case E_AE_SETTING_CPU_SENSORGAIN:
                m_bSetGainValue= btrigger;
            break;
        default:
            AE_SETTING_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
    }
}

MVOID
AeSettingDefault::
updateExpoSettingbyI2C(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;

    /* update frame rate */
    if (m_bSetFrameRateValue) {
        AE_SETTING_LOG("[%s()] Max frame rate = %d\n", __FUNCTION__, pinput->u4UpdateFrameRate_x10);
        AaaTimer localTimer("SetFrameRater", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, pinput->u4UpdateFrameRate_x10, m_eSensorMode);
        localTimer.End();
        m_bSetFrameRateValue = MFALSE;
    }
    /* update shutter value */
    if (m_bSetShutterValue) {
        AaaTimer localTimer("SetSensorShutter", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        if(pinput->rAERealSetting.EvSetting.u4Eposuretime != 0) {
            AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] Exposure time value = %d, Exposure mode = %d\n", __FUNCTION__, pinput->rAERealSetting.EvSetting.u4Eposuretime, pinput->rAERealSetting.u4ExposureMode);
            if(pinput->rAERealSetting.u4ExposureMode == eAE_EXPO_TIME)
                AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.EvSetting.u4Eposuretime);
            else
                AAASensorMgr::getInstance().setSensorExpLine((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.EvSetting.u4Eposuretime);
        }
        localTimer.End();
        m_bSetShutterValue = MFALSE;
    }
    /* update sensor gain value */
    if (m_bSetGainValue) {
        AaaTimer localTimer("SetSensorGain", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        if(pinput->rAERealSetting.EvSetting.u4AfeGain != 0) {
            AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] Sensor gain value = %d\n", __FUNCTION__, pinput->rAERealSetting.EvSetting.u4AfeGain);
            if (m_pSensorGainThread){
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->rAERealSetting.EvSetting.u4AfeGain, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            } else {
                AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.EvSetting.u4AfeGain);
            }
        }
        localTimer.End();
        m_bSetGainValue = MFALSE;
    }
}

MBOOL
AeSettingDefault::
updateExpoSetting(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;
    AE_SETTING_LOG("[%s()] m_eSensorDev:%d, Exposure time value = %d, Sensor gain value = %d, Exposure mode = %d\n",
                   __FUNCTION__, m_eSensorDev, pinput->rAERealSetting.EvSetting.u4Eposuretime, pinput->rAERealSetting.EvSetting.u4AfeGain, pinput->rAERealSetting.u4ExposureMode);
    if(pinput->rAERealSetting.EvSetting.u4Eposuretime != 0) {
        if(pinput->rAERealSetting.u4ExposureMode == eAE_EXPO_TIME)
            AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.EvSetting.u4Eposuretime);
        else
            AAASensorMgr::getInstance().setSensorExpLine((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.EvSetting.u4Eposuretime);
    }
    if(pinput->rAERealSetting.EvSetting.u4AfeGain != 0)
        AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.EvSetting.u4AfeGain);
    m_bSetShutterValue = MFALSE;
    m_bSetGainValue = MFALSE;
    return MFALSE;
}

MVOID
AeSettingDefault::
updateSensorMaxFPS(MVOID* input)
{
    MUINT32* pinput = (MUINT32*) input;
    AE_SETTING_LOG("[%s()] m_eSensorDev: %d, m_eSensorMode: %d, Sensor max FPS = %d\n", __FUNCTION__, m_eSensorDev, m_eSensorMode, *pinput);
    AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, *pinput, m_eSensorMode);
    m_bSetFrameRateValue = MFALSE;
}

AeSettingDefault::
~AeSettingDefault()
{
    AE_SETTING_LOG("Delete ~AeFlowDefault - DeviceId:%d",(MUINT32)m_eSensorDev);
}

