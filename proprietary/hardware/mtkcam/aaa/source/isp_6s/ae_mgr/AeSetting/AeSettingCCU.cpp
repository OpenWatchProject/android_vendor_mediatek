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
#define LOG_TAG "AeSettingCCU"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeSettingCCU.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>

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

        
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCcuIf;


IAeSetting*
AeSettingCCU::
getInstance(ESensorDev_T sensor)
{
    AE_SETTING_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeSettingCCU singleton_main(ESensorDev_Main);
        AE_SETTING_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeSettingCCU singleton_main2(ESensorDev_MainSecond);
        AE_SETTING_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeSettingCCU singleton_sub(ESensorDev_Sub);
        AE_SETTING_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:        //  Sub Sensor
        static AeSettingCCU singleton_sub2(ESensorDev_SubSecond);
        AE_SETTING_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
        
    }
}

AeSettingCCU::
AeSettingCCU(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_eSensorMode(ESensorMode_Preview)
    , m_3ALogEnable(MFALSE)
    , m_eSensorTG(ESensorTG_1)
    , m_i4SensorIdx(0)
    , m_eAETargetMode(AE_MODE_NORMAL)
    , m_pICcuAe(NULL)
{
    
    AE_SETTING_LOG("Enter AeSettingCCU DeviceId:%d",(MUINT32)m_eSensorDev);
}

AeSettingCCU::
~AeSettingCCU()
{
    
    AE_SETTING_LOG("Delete ~AeSettingCCU - DeviceId:%d",(MUINT32)m_eSensorDev);

}

MVOID
AeSettingCCU::
init()
{
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
}

MVOID
AeSettingCCU::
uninit()
{
    AE_FLOW_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);

    if (m_pICcuAe != NULL)
    {
        m_pICcuAe = NULL;
    }
}

MVOID
AeSettingCCU::
start(MVOID* input)
{
    AE_SETTING_LOG("[%s()+]\n", __FUNCTION__);
    AE_SETTING_INPUT_INFO_T* pInput = (AE_SETTING_INPUT_INFO_T*) input;
    m_i4SensorIdx = pInput->i4SensorIdx;
    m_eSensorTG   = pInput->eSensorTG;
    m_eSensorMode = pInput->eSensorMode;
    m_eAETargetMode = pInput->eAETargetMode;
    m_pICcuAe = ICcuCtrlAe::getInstance(m_i4SensorIdx, m_eSensorDev);
    AE_SETTING_LOG("[%s()-]:%d/%d/%d/%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_i4SensorIdx, m_eSensorMode, m_eAETargetMode);
}

MBOOL
AeSettingCCU::
updateExpoSetting(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;
    ccu_manual_exp_info rCcuManualExpoInfo;
    ccu_manual_exp_info_output rCcuManualExpoInfoOut;

    if (pinput->bCcuPresetControl)
        rCcuManualExpoInfo.timing = CCU_MANUAL_EXP_TIMING_PRESET;
    else
        rCcuManualExpoInfo.timing = CCU_MANUAL_EXP_TIMING_SET;
    memcpy(&(rCcuManualExpoInfo.real_setting), &(pinput->rAERealSettingBeforeConv), sizeof(strAERealSetting)); // setting before convert
    memcpy(&(rCcuManualExpoInfo.binsum_conv_setting), &(pinput->rAERealSetting), sizeof(strAERealSetting));    // setting after  convert
    if (rCcuManualExpoInfo.real_setting.m_AETargetMode == AE_MODE_NORMAL)
        AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] m_eSensorDev:%d, Manual timing = %d, Exposure mode = %d, u4Index/F: %d/%d, bResetIndex: %d, algo_setting: %d/%d/%d (%d), conv_setting: %d/%d/%d (%d)\n",
                          __FUNCTION__, m_eSensorDev, rCcuManualExpoInfo.timing, rCcuManualExpoInfo.real_setting.u4ExposureMode, rCcuManualExpoInfo.real_setting.u4Index, rCcuManualExpoInfo.real_setting.u4IndexF, rCcuManualExpoInfo.real_setting.bResetIndex,
                          rCcuManualExpoInfo.real_setting.EvSetting.u4Eposuretime, rCcuManualExpoInfo.real_setting.EvSetting.u4AfeGain, rCcuManualExpoInfo.real_setting.EvSetting.u4IspGain, rCcuManualExpoInfo.real_setting.u4ISO,
                          rCcuManualExpoInfo.binsum_conv_setting.EvSetting.u4Eposuretime, rCcuManualExpoInfo.binsum_conv_setting.EvSetting.u4AfeGain, rCcuManualExpoInfo.binsum_conv_setting.EvSetting.u4IspGain, rCcuManualExpoInfo.binsum_conv_setting.u4ISO);
    else
        AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] m_eSensorDev: %d, Manual timing = %d, AETargetMode = %d, u4Index/F: %d/%d, bResetIndex: %d, Exposure time LE/SE/ME value = %d/%d/%d, Sensor gain LE/SE/ME value = %d/%d/%d, ISP gain value = %d, ISO = %d\n",
                          __FUNCTION__, m_eSensorDev, rCcuManualExpoInfo.timing, rCcuManualExpoInfo.real_setting.m_AETargetMode, rCcuManualExpoInfo.real_setting.u4Index, rCcuManualExpoInfo.real_setting.u4IndexF, rCcuManualExpoInfo.real_setting.bResetIndex,
                          rCcuManualExpoInfo.real_setting.HdrEvSetting.i4LEExpo, rCcuManualExpoInfo.real_setting.HdrEvSetting.i4SEExpo, rCcuManualExpoInfo.real_setting.HdrEvSetting.i4MEExpo,
                          rCcuManualExpoInfo.real_setting.HdrEvSetting.i4LEAfeGain, rCcuManualExpoInfo.real_setting.HdrEvSetting.i4SEAfeGain, rCcuManualExpoInfo.real_setting.HdrEvSetting.i4MEAfeGain,
                          rCcuManualExpoInfo.real_setting.HdrEvSetting.i4LEIspGain, rCcuManualExpoInfo.real_setting.u4ISO);
    m_pICcuAe->ccuControl(MSG_TO_CCU_MANUAL_EXP_CTRL, &rCcuManualExpoInfo, &rCcuManualExpoInfoOut);

    /* if CCU can set manual setting => return true;
       else if CCU can't set manual setting => return false */
    if (!rCcuManualExpoInfoOut.is_supported) {
        AE_SETTING_LOG("[%s()] CCU can't support manual setting\n", __FUNCTION__);
    }
    return rCcuManualExpoInfoOut.is_supported;
}

MVOID
AeSettingCCU::
updateSensorMaxFPS(MVOID* input)
{
    MUINT32* pinput = (MUINT32*) input;
    ccu_max_framerate_data max_framerate_data;
    max_framerate_data.framerate = (MUINT16)(*pinput);
    max_framerate_data.min_framelength_en = MTRUE;
    AE_FLOW_LOG("[%s()] control CCU max FPS change, m_eSensorDev: %d, m_eSensorMode: %d, m_i4AEMaxFps: 0x%x\n", __FUNCTION__, m_eSensorDev, m_eSensorMode, max_framerate_data.framerate);
    m_pICcuAe->ccuControl(MSG_TO_CCU_SET_MAX_FRAMERATE, &max_framerate_data, NULL);
}

