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
#define LOG_TAG "AeFlowDefault"

#include <stdlib.h>
#include "AeFlowDefault.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace NS3Av3;
using namespace NSIspTuning;

IAeFlow*
AeFlowDefault::
getInstance(ESensorDev_T eSensorDev)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)eSensorDev);

    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeFlowDefault singleton_main(ESensorDev_Main, CPU_AE_CORE_CAM_ID_MAIN);
        AE_FLOW_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeFlowDefault singleton_main2(ESensorDev_MainSecond, CPU_AE_CORE_CAM_ID_MAIN2);
        AE_FLOW_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeFlowDefault singleton_sub(ESensorDev_Sub, CPU_AE_CORE_CAM_ID_SUB);
        AE_FLOW_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        static AeFlowDefault singleton_sub2(ESensorDev_SubSecond, CPU_AE_CORE_CAM_ID_SUB2); // temp: currently without sub2 enum
        AE_FLOW_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AeFlowDefault::
AeFlowDefault(ESensorDev_T eSensorDev, CPU_AE_CORE_CAM_ID_ENUM eCamID)
    : m_eSensorDev(eSensorDev)
    , m_eCamID(eCamID)
    , m_3ALogEnable(0)
    , m_bTriggerFrameInfoUpdate(MFALSE)
    , m_bTriggerOnchInfoUpdate(MFALSE)
{
    AE_FLOW_LOG("Enter AeFlowDefault - m_eSensorDev:%d, m_eCamID:%d", (MUINT32)m_eSensorDev, (MUINT32)m_eCamID);
}

AeFlowDefault::
~AeFlowDefault()
{
    AE_FLOW_LOG("Delete ~AeFlowDefault - m_eSensorDev:%d, m_eCamID:%d", (MUINT32)m_eSensorDev, (MUINT32)m_eCamID);
}

MVOID
AeFlowDefault::
start(MVOID* input)
{
    AE_FLOW_LOG("[%s +] m_eSensorDev:%d, m_eCamID:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, (MUINT32)m_eCamID);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);

    AE_FLOW_CPU_INIT_INFO_T* pCPUInitInfo = (AE_FLOW_CPU_INIT_INFO_T*) input;
    CPU_AE_Core_Init(&(pCPUInitInfo->mInitData), m_eCamID);
    AE_FLOW_LOG("[%s -]\n", __FUNCTION__);
}

MBOOL
AeFlowDefault::
queryStatus(MUINT32 index)
{
    E_AE_FLOW_CPU_STATUS_T eStatus = (E_AE_FLOW_CPU_STATUS_T) index;
    switch (eStatus)
    {
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
            return MFALSE;
    }
}

MVOID
AeFlowDefault::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_FLOW_CPU_TRIGER_T etrigger = (E_AE_FLOW_CPU_TRIGER_T) index;
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] m_eSensorDev: %d, m_eCamID: %d, index: %d, istrigger: %d \n", __FUNCTION__, m_eSensorDev, m_eCamID, index, btrigger);
    switch (etrigger)
    {
        case E_AE_FLOW_CPU_TRIGGER_FRAME_INFO_UPDATE:
            m_bTriggerFrameInfoUpdate = MTRUE;
            break;
        case E_AE_FLOW_CPU_TRIGGER_ONCH_INFO_UPDATE:
            m_bTriggerOnchInfoUpdate = MTRUE;
            break;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
    }
}

MBOOL
AeFlowDefault::
calculateAE(MVOID* input, MVOID* output)
{
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s +] m_eSensorDev:%d, m_eCamID:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, (MUINT32)m_eCamID);
    AE_FLOW_CPU_CALCULATE_INFO_T* pInput = (AE_FLOW_CPU_CALCULATE_INFO_T*)input;
    AE_CORE_MAIN_OUT* pOutput = (AE_CORE_MAIN_OUT*) output;
    if (m_bTriggerFrameInfoUpdate) {
        CPU_AE_Core_Ctrl(AE_CTRL_SET_VSYNC_INFO, (void*)&(pInput->mVsyncInfo), 0, m_eCamID);
        m_bTriggerFrameInfoUpdate = MFALSE;
    }
    if (m_bTriggerOnchInfoUpdate) {
        CPU_AE_Core_Ctrl(AE_CTRL_SET_RUN_TIME_INFO, (void*)&(pInput->mRunTimeInfo), 0, m_eCamID);
        m_bTriggerOnchInfoUpdate = MFALSE;
    }
    CPU_AE_Core_Stat(&(pInput->mAEMainIn), pOutput, m_eCamID);
    CPU_AE_Core_Main(&(pInput->mAEMainIn), pOutput, m_eCamID);
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s -]\n", __FUNCTION__);

/*
    // temp
    pOutput->a_Output.u4CWValue = 102;
    pOutput->a_Output.EvSetting.u4Eposuretime = 30000;
    pOutput->a_Output.EvSetting.u4AfeGain = 2048;
    pOutput->a_Output.EvSetting.u4IspGain = 4096;
    pOutput->a_Output.u4ISO = 200;
    pOutput->a_Output.u4Index = 100;
    pOutput->a_Output.u4IndexF = 300;
    pOutput->a_Output.bAEStable = MTRUE;
*/
    return MTRUE;
}

MVOID
AeFlowDefault::
getAEInfoForISP(AE_INFO_T &ae_info)
{
    // to do
}

MVOID
AeFlowDefault::
getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info)
{
    // to do
}

