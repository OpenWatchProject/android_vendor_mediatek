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
#if defined ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeFlowCPU.h"
#include "AeFlowDefault.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_tuning_mgr.h>
#include <aaa_scheduler.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#define HDR 1

#if defined(VHDR)
#include "AeFlowVHDR.h"
#endif


#define AE_FLOW_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_FLOW_LOG_IF(cond, fmt, arg...) \
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
    case ESensorDev_MainThird:  //  Main Third Sensor
        static AeFlowCPU singleton_main3(ESensorDev_MainThird);
        AE_FLOW_LOG("ESensorDev_MainThird(%p)", &singleton_main3);
        return &singleton_main3;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeFlowCPU singleton_sub(ESensorDev_Sub);
        AE_FLOW_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        static AeFlowCPU singleton_sub2(ESensorDev_SubSecond);
        AE_FLOW_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AeFlowCPU::
AeFlowCPU(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_eAETargetMode(AE_MODE_NORMAL)
    #if defined(VHDR)
    , m_pAeFlowVHDR(NULL)
    #endif
    , m_pAeFlowDefault(NULL)
{
    #if defined(VHDR)
    m_pAeFlowVHDR = AeFlowVHDR::getInstance(m_eSensorDev);
    #endif
    m_pAeFlowDefault = AeFlowDefault::getInstance(m_eSensorDev);
    AE_FLOW_LOG("Enter AeFlowCPU DeviceId:%d",(MUINT32)m_eSensorDev);
    m_3ALogEnable = 0;
}

MVOID
AeFlowCPU::
start(MVOID* input)
{
    AE_FLOW_CPUSTART_INFO_T* pinput = (AE_FLOW_CPUSTART_INFO_T*) input;
    m_eAETargetMode = pinput->eAETargetMode;
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->start(input);
    #if defined(VHDR)
    else
        m_pAeFlowVHDR->start(input);
    #endif
}

MBOOL
AeFlowCPU::
queryStatus(MUINT32 index)
{
    #if defined(VHDR)
    if (m_eAETargetMode == AE_MODE_NORMAL)
    #endif
        return m_pAeFlowDefault->queryStatus(index);
    #if defined(VHDR)
    else
        return m_pAeFlowVHDR->queryStatus(index);
    #endif
}

MVOID
AeFlowCPU::
getParam(MUINT32 index, MUINT32& i4value)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->getParam(index, i4value);
    #if defined(VHDR)
    else
        m_pAeFlowVHDR->getParam(index, i4value);
    #endif
}


MVOID
AeFlowCPU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->controltrigger(index, btrigger);
    #if defined(VHDR)
    else
        m_pAeFlowVHDR->controltrigger(index, btrigger);
    #endif
}


MVOID
AeFlowCPU::
schedulePvAE(MVOID* input, MVOID* output)
{
    AE_FLOW_SCHEDULE_INFO_T* pinputinfo = (AE_FLOW_SCHEDULE_INFO_T*) input;
    m_eAETargetMode = pinputinfo->eAETargetMode;
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->schedulePvAE(input, output);
    #if defined(VHDR)
    else
        m_pAeFlowVHDR->schedulePvAE(input, output);
    #endif
}

MVOID
AeFlowCPU::
calculateAE(MVOID* input, MVOID* output)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
    AE_FLOW_LOG_IF(m_3ALogEnable,  "[%s()] Enter AeFlowCPU calculateAE\n", __FUNCTION__);
    if (m_eAETargetMode == AE_MODE_NORMAL)
    {
        m_pAeFlowDefault->calculateAE(input, output);
        AE_FLOW_LOG_IF(m_3ALogEnable,  "[%s()] Enter AeFlowDefault calculateAE\n", __FUNCTION__);
    }
    #if defined(VHDR)
    else
        m_pAeFlowVHDR->calculateAE(input, output);
    #endif
}


MVOID
AeFlowCPU::
update(AE_EXP_INFO* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->update(input);
    #if defined(VHDR)
    else
        m_pAeFlowVHDR->update(input);
    #endif
}

AeFlowCPU::
~AeFlowCPU()
{
    AE_FLOW_LOG("Delete ~AeFlowCPU - DeviceId:%d",(MUINT32)m_eSensorDev);
}


