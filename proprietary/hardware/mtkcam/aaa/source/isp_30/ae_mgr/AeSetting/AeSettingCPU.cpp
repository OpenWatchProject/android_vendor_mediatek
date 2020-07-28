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
#define LOG_TAG "AeSettingCPU"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeSettingCPU.h"
#include "AeSettingDefault.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include <aaa_scheduler.h>

#if defined(VHDR)
#include "AeSettingVHDR.h"
#endif

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
AeSettingCPU::
getInstance(ESensorDev_T sensor)
{
    AE_SETTING_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeSettingCPU singleton_main(ESensorDev_Main);
        AE_SETTING_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeSettingCPU singleton_main2(ESensorDev_MainSecond);
        AE_SETTING_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_MainThird:  //  Main Third Sensor
        static AeSettingCPU singleton_main3(ESensorDev_MainThird);
        AE_SETTING_LOG("ESensorDev_MainThird(%p)", &singleton_main3);
        return &singleton_main3;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeSettingCPU singleton_sub(ESensorDev_Sub);
        AE_SETTING_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        static AeSettingCPU singleton_sub2(ESensorDev_SubSecond);
        AE_SETTING_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AeSettingCPU::
AeSettingCPU(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_eAETargetMode(AE_MODE_NORMAL)
    #if defined(VHDR)
    , m_pAeSettingVHDR(NULL)
    #endif
    , m_pAeSettingDefault(NULL)
{
    #if defined(VHDR)
    m_pAeSettingVHDR = AeSettingVHDR::getInstance(m_eSensorDev);
    #endif
    m_pAeSettingDefault = AeSettingDefault::getInstance(m_eSensorDev);
    AE_SETTING_LOG("Enter AeSettingCPU DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeSettingCPU::
init()
{
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    #if defined(VHDR)
    m_pAeSettingVHDR->init();
    #endif
    m_pAeSettingDefault->init();
}

MVOID
AeSettingCPU::
uninit()
{
    #if defined(VHDR)
    m_pAeSettingVHDR->uninit();
    #endif
    m_pAeSettingDefault->uninit();
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
}

MVOID
AeSettingCPU::
start(MVOID* pinputinfo)
{
    AE_SETTING_INPUT_INFO_T* pInput = (AE_SETTING_INPUT_INFO_T*) pinputinfo;
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    m_eAETargetMode = pInput->eAETargetMode;
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->start(pinputinfo);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->start(pinputinfo);
    #endif
}

MVOID
AeSettingCPU::
stop()
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->stop();
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->stop();
    #endif
    AE_SETTING_LOG( "[%s():%d]\n", __FUNCTION__, m_eSensorDev);
}

MBOOL
AeSettingCPU::
queryStatus(MUINT32 index)
{
    #if defined(VHDR)
    if (m_eAETargetMode == AE_MODE_NORMAL)
    #endif
        return m_pAeSettingDefault->queryStatus(index);
    #if defined(VHDR)
    else
        return m_pAeSettingVHDR->queryStatus(index);
    #endif
}

MVOID
AeSettingCPU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    if( index == E_AE_SETTING_CPU_STORE){
      m_pAeSettingDefault->controltrigger(index, btrigger);
      #if defined(VHDR)
      m_pAeSettingVHDR->controltrigger(index, btrigger);
      #endif
      return;
    }

    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->controltrigger(index, btrigger);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->controltrigger(index, btrigger);
    #endif
}


MVOID
AeSettingCPU::
updateSensor(MVOID* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->updateSensor(input);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->updateSensor(input);
    #endif
}
MVOID
AeSettingCPU::
updateSensorFast(MVOID* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->updateSensorFast(input);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->updateSensorFast(input);
    #endif

}
MVOID
AeSettingCPU::
updateSensorGain(MVOID* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->updateSensorGain(input);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->updateSensorGain(input);
    #endif
}

MVOID
AeSettingCPU::
updateSensorExp(MVOID* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->updateSensorExp(input);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->updateSensorExp(input);
    #endif
}


MVOID
AeSettingCPU::
updateISP(MVOID* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeSettingDefault->updateISP(input);
    #if defined(VHDR)
    else
        m_pAeSettingVHDR->updateISP(input);
    #endif
    AE_SETTING_LOG( "[%s()]\n", __FUNCTION__);
}


AeSettingCPU::
~AeSettingCPU()
{
    AE_SETTING_LOG("Delete ~AeSettingCPU - DeviceId:%d",(MUINT32)m_eSensorDev);
}


