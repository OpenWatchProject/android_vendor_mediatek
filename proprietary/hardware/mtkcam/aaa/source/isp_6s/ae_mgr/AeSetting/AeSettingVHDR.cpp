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
#define LOG_TAG "AeSettingVHDR"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeSettingVHDR.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
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

IAeSetting*
AeSettingVHDR::
getInstance(ESensorDev_T sensor)
{
    AE_SETTING_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeSettingVHDR singleton_main(ESensorDev_Main);
        AE_SETTING_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeSettingVHDR singleton_main2(ESensorDev_MainSecond);
        AE_SETTING_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeSettingVHDR singleton_sub(ESensorDev_Sub);
        AE_SETTING_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:        //  Sub Sensor
        static AeSettingVHDR singleton_sub2(ESensorDev_SubSecond);
        AE_SETTING_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;

    }
}

AeSettingVHDR::
AeSettingVHDR(ESensorDev_T sensor)
    : AeSettingDefault(sensor)
    , m_eAETargetMode(AE_MODE_NORMAL)
{
    AE_SETTING_LOG("Enter AeSettingVHDR DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeSettingVHDR::
updateExpoSettingbyI2C(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;

    /* update frame rate */
    if(m_bSetFrameRateValue) {
        AE_SETTING_LOG("[%s()] Max frame rate = %d\n", __FUNCTION__, pinput->u4UpdateFrameRate_x10);
        AaaTimer localTimer("SetFrameRater", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, pinput->u4UpdateFrameRate_x10, m_eSensorMode);
        localTimer.End();
        m_bSetFrameRateValue = MFALSE;
    }
    /* update shutter value */
    if(m_bSetShutterValue) {
        AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] Exposure time LE/SE/ME value = %d/%d/%d, LV = %d\n",
                          __FUNCTION__, pinput->rAERealSetting.HdrEvSetting.i4LEExpo, pinput->rAERealSetting.HdrEvSetting.i4SEExpo, pinput->rAERealSetting.HdrEvSetting.i4MEExpo, pinput->i4Pass2SensorLVvalue);
        if(m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) // MVHDR 3-expo
            AAASensorMgr::getInstance().set3ShutterControl((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEExpo, pinput->rAERealSetting.HdrEvSetting.i4MEExpo, pinput->rAERealSetting.HdrEvSetting.i4SEExpo);
        else
            AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEExpo, pinput->rAERealSetting.HdrEvSetting.i4SEExpo,pinput->i4Pass2SensorLVvalue);
        m_bSetShutterValue = MFALSE;
    }
    /* update sensor gain value */
    if(m_bSetGainValue) {
        AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] Sensor gain LE/SE/ME value = %d/%d/%d\n", __FUNCTION__, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain);
        if (!m_pSensorGainThread){  // 0/0/2 sensor
            if(m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) {// MVHDR 3-expo
                AAASensorMgr::getInstance().setSensorHDRTriGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain);
            } else if(m_eAETargetMode == AE_MODE_ZVHDR_TARGET) {
                AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain);
            } else{
                AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain);
            }
        } else {                    // 0/1/2 sensor
            if(m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) {// MVHDR 3-expo
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
            if(m_eAETargetMode == AE_MODE_ZVHDR_TARGET) {
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain,pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }else{
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
        }
        m_bSetGainValue = MFALSE;
    }
}

MBOOL
AeSettingVHDR::
updateExpoSetting(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;
    AE_SETTING_LOG("[%s()] m_eSensorDev: %d, Exposure time LE/SE/ME value = %d/%d/%d, Sensor gain LE/SE/ME value = %d/%d/%d, AETargetMode = %d, LV = %d\n", __FUNCTION__, m_eSensorDev,
                    pinput->rAERealSetting.HdrEvSetting.i4LEExpo, pinput->rAERealSetting.HdrEvSetting.i4SEExpo, pinput->rAERealSetting.HdrEvSetting.i4MEExpo,
                    pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain,
                    pinput->rAERealSetting.m_AETargetMode, pinput->i4Pass2SensorLVvalue);
    if(pinput->rAERealSetting.HdrEvSetting.i4LEExpo != 0 && pinput->rAERealSetting.HdrEvSetting.i4SEExpo != 0) {
        if(m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) // MVHDR 3-expo
            AAASensorMgr::getInstance().set3ShutterControl((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEExpo, pinput->rAERealSetting.HdrEvSetting.i4MEExpo, pinput->rAERealSetting.HdrEvSetting.i4SEExpo);
        else
            AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEExpo, pinput->rAERealSetting.HdrEvSetting.i4SEExpo,pinput->i4Pass2SensorLVvalue);
    }
    if(pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain != 0 && pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain != 0) {
        if(m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) {// MVHDR 3-expo
            AAASensorMgr::getInstance().setSensorHDRTriGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain);
        } else if(m_eAETargetMode == AE_MODE_ZVHDR_TARGET) {
            AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain, pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain);
        } else{
            AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain);
        }
    }
    m_bSetShutterValue = MFALSE;
    m_bSetGainValue = MFALSE;
    return MFALSE;
}

MVOID
AeSettingVHDR::
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

AeSettingVHDR::
~AeSettingVHDR()
{
    AE_SETTING_LOG("Delete ~AeSettingVHDR - DeviceId:%d",(MUINT32)m_eSensorDev);
}

