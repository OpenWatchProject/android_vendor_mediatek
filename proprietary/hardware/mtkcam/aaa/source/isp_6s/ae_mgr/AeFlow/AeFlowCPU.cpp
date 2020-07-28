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
#define LOG_TAG "AeFlowCPU"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeFlowCPU.h"
#include "AeFlowDefault.h"
#include "AeFlowCustom.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
 #include <aaa_scheduler.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace NS3Av3;

IAeFlow*
AeFlowCPU::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeFlowCPU singleton_main(ESensorDev_Main);
        AE_FLOW_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeFlowCPU singleton_main2(ESensorDev_MainSecond);
        AE_FLOW_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeFlowCPU singleton_sub(ESensorDev_Sub);
        AE_FLOW_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:        //  Sub Sensor
        static AeFlowCPU singleton_sub2(ESensorDev_SubSecond);
        AE_FLOW_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;

    }
}

AeFlowCPU::
AeFlowCPU(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_eCPUAlgoType(E_AE_ALGO_CPU_DEFAULT)
    , m_pAeFlowDefault(NULL)
    , m_pAeFlowCustom(NULL)
{
    m_pAeFlowDefault = AeFlowDefault::getInstance(m_eSensorDev);
    m_pAeFlowCustom  = AeFlowCustom::getInstance(m_eSensorDev);
    AE_FLOW_LOG("Enter AeFlowCPU DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeFlowCPU::
start(MVOID* input)
{
    AE_FLOW_CPU_INIT_INFO_T* pinput = (AE_FLOW_CPU_INIT_INFO_T*) input;
    m_eCPUAlgoType  = pinput->mCPUAlgoType;
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        m_pAeFlowCustom->start(input);
    else
        m_pAeFlowDefault->start(input);
}

MVOID
AeFlowCPU::
stop()
{
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        m_pAeFlowCustom->stop();
    else
        m_pAeFlowDefault->stop();
}

MBOOL
AeFlowCPU::
queryStatus(MUINT32 index)
{
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        return m_pAeFlowCustom->queryStatus(index);
    else
        return m_pAeFlowDefault->queryStatus(index);
}

MVOID
AeFlowCPU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        m_pAeFlowCustom->controltrigger(index, btrigger);
    else
        m_pAeFlowDefault->controltrigger(index, btrigger);
}

MBOOL
AeFlowCPU::
calculateAE(MVOID* input, MVOID* output)
{
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        return m_pAeFlowCustom->calculateAE(input, output);
    else
        return m_pAeFlowDefault->calculateAE(input, output);
}

MVOID
AeFlowCPU::
getAEInfoForISP(AE_INFO_T &ae_info)
{
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        m_pAeFlowCustom->getAEInfoForISP(ae_info);
    else
        m_pAeFlowDefault->getAEInfoForISP(ae_info);
}

MVOID
AeFlowCPU::
getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info)
{
    if (m_eCPUAlgoType == E_AE_ALGO_CPU_CUSTOM)
        m_pAeFlowCustom->getDebugInfo(exif_info, dbg_data_info);
    else
        m_pAeFlowDefault->getDebugInfo(exif_info, dbg_data_info);
}

AeFlowCPU::
~AeFlowCPU()
{
    AE_FLOW_LOG("Delete ~AeFlowCPU - DeviceId:%d",(MUINT32)m_eSensorDev);
}


