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
#include "AeFlowVHDR.h"
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

static std::array<MY_INST_AEFLOW, SENSOR_IDX_MAX> gMultiton;

IAeFlow*
AeFlowCPU::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_AEFLOW& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeFlowCPU>(sensor);
    } );

    return rSingleton.instance.get();
}

AeFlowCPU::
AeFlowCPU(ESensorDev_T sensor)
: m_eSensorDev(sensor)
, m_eAETargetMode(AE_MODE_NORMAL)
, m_pAeFlowVHDR(NULL)
, m_pAeFlowDefault(NULL)
{
    m_pAeFlowVHDR = AeFlowVHDR::getInstance(m_eSensorDev);
    m_pAeFlowDefault = AeFlowDefault::getInstance(m_eSensorDev);
    AE_FLOW_LOG("[AeFlowCPU:Construct] SensorDev:%d\n", (MUINT32)m_eSensorDev);
}

MVOID
AeFlowCPU::
start(MVOID* input)
{
    AE_FLOW_CPUSTART_INFO_T* pinput = (AE_FLOW_CPUSTART_INFO_T*) input;
    m_eAETargetMode = pinput->eAETargetMode;
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->start(input);
    else
        m_pAeFlowVHDR->start(input);
}

MBOOL
AeFlowCPU::
queryStatus(MUINT32 index)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        return m_pAeFlowDefault->queryStatus(index);
    else
        return m_pAeFlowVHDR->queryStatus(index);
}

MVOID
AeFlowCPU::
getParam(MUINT32 index, MUINT32& i4value)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->getParam(index, i4value);
    else
        m_pAeFlowVHDR->getParam(index, i4value);
}


MVOID
AeFlowCPU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->controltrigger(index, btrigger);
    else
        m_pAeFlowVHDR->controltrigger(index, btrigger);
}


MVOID
AeFlowCPU::
schedulePvAE(MVOID* input, MVOID* output)
{
    AE_FLOW_SCHEDULE_INFO_T* pinputinfo = (AE_FLOW_SCHEDULE_INFO_T*) input;
    m_eAETargetMode = pinputinfo->eAETargetMode;
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->schedulePvAE(input, output);
    else
        m_pAeFlowVHDR->schedulePvAE(input, output);

}

MVOID
AeFlowCPU::
calculateAE(MVOID* input, MVOID* output)
{
    //AE_FLOW_SCHEDULE_INFO_T* pinputinfo = (AE_FLOW_SCHEDULE_INFO_T*) input;  /* fix build warning */
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->calculateAE(input, output);
    else
        m_pAeFlowVHDR->calculateAE(input, output);
}


MVOID
AeFlowCPU::
update(AE_CORE_CTRL_CPU_EXP_INFO* input)
{
    if (m_eAETargetMode == AE_MODE_NORMAL)
        m_pAeFlowDefault->update(input);
    else
        m_pAeFlowVHDR->update(input);
}

AeFlowCPU::
~AeFlowCPU()
{
    AE_FLOW_LOG("[AeFlowCPU:Destruct] SensorDev:%d\n",(MUINT32)m_eSensorDev);
}

MVOID
AeFlowCPU::
setMVHDR3ExpoProcInfo(MVOID* pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100)
{
    if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
        m_pAeFlowVHDR->setMVHDR3ExpoProcInfo(pMVHDR3ExpoStatBuf, u4MVHDRRatio_x100);
}

