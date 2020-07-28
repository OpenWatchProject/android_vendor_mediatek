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
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include <aaa_scheduler.h>



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
using namespace NSIspTuning;
using namespace NSIspTuningv3;


IAeSetting*
AeSettingDefault::
getInstance(ESensorDev_T sensor)
{
    AE_SETTING_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeSettingDefault singleton_main(ESensorDev_Main);
        AE_SETTING_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeSettingDefault singleton_main2(ESensorDev_MainSecond);
        AE_SETTING_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_MainThird:  //  Main Third Sensor
        static AeSettingDefault singleton_main3(ESensorDev_MainThird);
        AE_SETTING_LOG("ESensorDev_MainThird(%p)", &singleton_main3);
        return &singleton_main3;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeSettingDefault singleton_sub(ESensorDev_Sub);
        AE_SETTING_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        static AeSettingDefault singleton_sub2(ESensorDev_SubSecond);
        AE_SETTING_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AeSettingDefault::
AeSettingDefault(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_eSensorMode(ESensorMode_Preview)
    , m_3ALogEnable(MFALSE)
    , m_bSetShutterValue(MFALSE)
    , m_bSetGainValue(MFALSE)
    , m_bSetFrameRateValue(MFALSE)
    , m_bUpdateSensorAWBGain(MFALSE)
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
            AE_SETTING_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
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
        case E_AE_SETTING_CPU_STORE:
            break;
        default:
            AE_SETTING_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
    }
}

MVOID
AeSettingDefault::
updateSensor(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;

    // update frame rate
    if (m_bSetFrameRateValue)
    {
        m_bSetFrameRateValue = MFALSE;
        AaaTimer localTimer("SetFrameRater", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, pinput->u4UpdateFrameRate_x10, m_eSensorMode);
        AE_SETTING_LOG_IF(m_3ALogEnable, "[%s()] PreviewMaxFrameRate:%d\n", __FUNCTION__, pinput->u4UpdateFrameRate_x10);
        localTimer.End();
    }

    // update shutter value
    if (m_bSetShutterValue)
    {
        if (pinput->bPerframeAndExpChanged)
        {
            AaaTimer localTimer("SetSensorShutter", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, pinput->u4UpdateShutterValue);
            AE_SETTING_LOG_IF(m_3ALogEnable, "[%s()] Exposure time value = %d\n", __FUNCTION__, pinput->u4UpdateShutterValue);
            localTimer.End();
        }
        m_bSetShutterValue = MFALSE;
    }

    // update sensor gain value
    if (m_bSetGainValue)
    {
        if (pinput->bPerframeAndExpChanged)
        {
            AaaTimer localTimer("SetSensorGain", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            if (m_pSensorGainThread)
            {
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->u4UpdateGainValue, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
            else
            {
                AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue);
            }
            AE_SETTING_LOG_IF(m_3ALogEnable, "[%s()] Sensor gain value = %d\n", __FUNCTION__, pinput->u4UpdateGainValue);
            localTimer.End();
        }
        m_bSetGainValue = MFALSE;
    }
}

MVOID
AeSettingDefault::
updateSensorFast(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;
    if((m_bSetShutterValue)&&(pinput->u4UpdateShutterValue != pinput->u4PreExposureTime)) {    // update shutter value
        AaaTimer localTimer("SetSensorShutterBufferMode", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, pinput->u4UpdateShutterValue);
        localTimer.End();
        AE_SETTING_LOG( "[%s()]  Exp = %d -> %d \n", __FUNCTION__,pinput->u4PreExposureTime, pinput->u4UpdateShutterValue);
        m_bSetShutterValue = MFALSE;
        return;
    }

    if((m_bSetGainValue)&&(pinput->u4UpdateGainValue!= pinput->u4PreSensorGain)) {    // update sensor gain value
        AaaTimer localTimer("SetSensorGainBufferMode", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue);
        localTimer.End();
        AE_SETTING_LOG( "[%s()]  Afe = %d -> %d \n", __FUNCTION__, pinput->u4PreSensorGain, pinput->u4UpdateGainValue);
        m_bSetGainValue = MFALSE;
    }

}

MVOID
AeSettingDefault::
updateSensorGain(MVOID* input)
{
    MUINT32* psensorgain = (MUINT32*) input;
    AE_SETTING_LOG("[%s] setAfe:%d\n", __FUNCTION__, *psensorgain);
    if (!m_pSensorGainThread)
    {
        // 0/0/2 sensor
        AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, *psensorgain);
    }
    else
    {
        // 0/1/2 sensor
        ThreadSensorGain::Cmd_T rCmd(0, 0, *psensorgain, m_eSensorTG);
        m_pSensorGainThread->postCmd(&rCmd);
    }
}

MVOID
AeSettingDefault::
updateSensorExp(MVOID* input)
{
    MUINT32* psensorExp = (MUINT32*) input;
    AE_SETTING_LOG("[%s] setExp:%d\n",__FUNCTION__,*psensorExp);
    AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, *psensorExp);
}

MVOID
AeSettingDefault::
updateISP(MVOID* input)
{
    (void) input;
    AE_SETTING_LOG("[%s()]\n", __FUNCTION__);
}


AeSettingDefault::
~AeSettingDefault()
{
    AE_SETTING_LOG("Delete ~AeFlowDefault - DeviceId:%d",(MUINT32)m_eSensorDev);
}


