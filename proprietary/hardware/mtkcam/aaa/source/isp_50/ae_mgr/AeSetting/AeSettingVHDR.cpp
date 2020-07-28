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
#define ENABLE_MY_LOG (1)
#endif

#include <stdlib.h>
#include "AeSettingVHDR.h"
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
#define EN_3A_SCHEDULE_LOG 2

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

#define MY_INST_SETTING_VHDR INST_T<AeSettingVHDR>
static std::array<MY_INST_SETTING_VHDR, SENSOR_IDX_MAX> gMultiton;

IAeSetting *
AeSettingVHDR::
getInstance(ESensorDev_T sensor)
{
    AE_SETTING_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_SETTING_VHDR& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeSettingVHDR>(sensor);
    } );

    return rSingleton.instance.get();
}

AeSettingVHDR::
AeSettingVHDR(ESensorDev_T sensor)
: AeSettingDefault(sensor)
, u4ExpLEperframe(0) // updatesensor updated
, u4ExpSEperframe(0)
, u4ExpMEperframe(0)
, u4AfeLEperframe(0)
, u4AfeSEperframe(0)
, u4AfeMEperframe(0)
, u4ExpLEstore(0) // control trigger copied from
, u4ExpSEstore(0)
, u4ExpMEstore(0)
, u4AfeLEstore(0)
, u4AfeSEstore(0)
, u4AfeMEstore(0)
, m_eAETargetMode(AE_MODE_NORMAL)
{
    AE_SETTING_LOG("[AeSettingVHDR:Construct] SensorDev:%d\n", (MUINT32)m_eSensorDev);
}

MVOID
AeSettingVHDR::
updateSensor(MVOID * input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;

    m_eAETargetMode = pinput->eAETargetMode;
    m_bUpdateSensorAWBGain = pinput->bSensorAWBGain;

    if (m_bSetFrameRateValue)
    {
        // update frame rate
        m_bSetFrameRateValue = MFALSE;
        AaaTimer localTimer("SetFrameRater", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, pinput->u4UpdateFrameRate_x10, m_eSensorMode);
        localTimer.End();
    }

    if (m_bSetShutterValue)
    {
        // update shutter value
        // 0 / 0 / 2 and 0 / 1 / 2 sensor both use it
        if (pinput->eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) // MVHDR 3 -expo
        AAASensorMgr::getInstance().set3ShutterControl((ESensorDev_T)m_eSensorDev, pinput->u4LEHDRshutter, pinput->u4MEHDRshutter, pinput->u4SEHDRshutter);
        else
        AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, pinput->u4LEHDRshutter, pinput->u4SEHDRshutter, pinput->i4Pass2SensorLVvalue);
        u4ExpLEperframe = pinput->u4LEHDRshutter;
        u4ExpSEperframe = pinput->u4SEHDRshutter;
        u4ExpMEperframe = pinput->u4MEHDRshutter;
        m_bSetShutterValue = MFALSE;
        AE_SETTING_LOG_IF(m_3ALogEnable, "[%s()] Exposure time LE/SE/ME value = %d/%d/%d, LV = %d\n", __FUNCTION__, pinput->u4LEHDRshutter, pinput->u4SEHDRshutter, pinput->u4MEHDRshutter, pinput->i4Pass2SensorLVvalue);
        // Update AWB gain to sensor
        if (m_bUpdateSensorAWBGain)
        {
            strSensorAWBGain rSensorAWBGain;
            AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
            rSensorAWBGain.u4R = rCurrentIspAWBGain.i4R;
            rSensorAWBGain.u4GR = rCurrentIspAWBGain.i4G;
            rSensorAWBGain.u4GB = rCurrentIspAWBGain.i4G;
            rSensorAWBGain.u4B = rCurrentIspAWBGain.i4B;
            AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)m_eSensorDev, &rSensorAWBGain);
        }
    }
    if (m_bSetGainValue)
    {
        // update sensor gain value
        u4AfeLEperframe = pinput->u4UpdateGainValue;
        u4AfeSEperframe = pinput->u4UpdateGainValue2;
        u4AfeMEperframe = pinput->u4UpdateGainValue3;
        AE_SETTING_LOG_IF(m_3ALogEnable, "[%s()] Sensor gain LE/SE/ME value = %d/%d/%d\n", __FUNCTION__, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue2, pinput->u4UpdateGainValue3);
        if (!m_pSensorGainThread)
        {
            // 0 / 0 / 2 sensor
            if (pinput->eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                // MVHDR 3 -expo
                AAASensorMgr::getInstance().setSensorHDRTriGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue3, pinput->u4UpdateGainValue2);
            }
            else if (pinput->eAETargetMode == AE_MODE_ZVHDR_TARGET)
            {
                AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue2);
            }
            else
            {
                AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue);
            }
        }
        else
        {
            // 0 / 1 / 2 sensor
            if (pinput->eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                // MVHDR 3 -expo
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue2, pinput->u4UpdateGainValue3, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
            if (pinput->eAETargetMode == AE_MODE_ZVHDR_TARGET)
            {
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue2, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
            else
            {
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->u4UpdateGainValue, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
        }
        m_bSetGainValue = MFALSE;
    }
}

MVOID
AeSettingVHDR::
updateSensorGain(MVOID * input)
{
    MUINT32* psensorgain = (MUINT32*) input; //LE
    if (!m_pSensorGainThread)
    {
        // 0 / 0 / 2 sensor
        if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
        {
            // MVHDR 3 -expo
            AAASensorMgr::getInstance().setSensorHDRTriGain((ESensorDev_T)m_eSensorDev, u4AfeLEstore, u4AfeMEstore, u4AfeSEstore);
            AE_SETTING_LOG("[%s()] input = %d , update Sensor gain LE = %d /SE = %d /ME = %d", __FUNCTION__, *psensorgain, u4AfeLEstore, u4AfeSEstore, u4AfeMEstore);
        }
        else if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
        {
            AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, u4AfeLEstore, u4AfeSEstore);
            AE_SETTING_LOG("[%s()] input = %d , update Sensor gain LE = %d /SE = %d ", __FUNCTION__, *psensorgain, u4AfeLEstore, u4AfeSEstore);
        }
        else
        {
            AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, *psensorgain);
        }
    }
    else
    {
        // 0 / 1 / 2 sensor
        if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
        {
            // MVHDR 3 -expo
            ThreadSensorGain::Cmd_T rCmd(0, 0, u4AfeLEstore, u4AfeSEstore, u4AfeMEstore, m_eSensorTG);
            m_pSensorGainThread->postCmd(&rCmd);
        }
        else if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
        {
            ThreadSensorGain::Cmd_T rCmd(0, 0, u4AfeLEstore, u4AfeSEstore, m_eSensorTG);
            m_pSensorGainThread->postCmd(&rCmd);
        }
        else
        {
            ThreadSensorGain::Cmd_T rCmd(0, 0, *psensorgain, m_eSensorTG);
            m_pSensorGainThread->postCmd(&rCmd);
        }
    }
}

MVOID
AeSettingVHDR::
updateSensorExp(MVOID * input)
{
    MUINT32* psensorExp = (MUINT32*) input;
    AE_SETTING_LOG("[%s] restore sensorExp input(%d) LE(%d) SE(%d) ME(%d)", __FUNCTION__, *psensorExp, u4ExpLEstore, u4ExpSEstore, u4ExpMEstore);
    if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) // MVHDR 3 -expo
        AAASensorMgr::getInstance().set3ShutterControl((ESensorDev_T)m_eSensorDev, u4ExpLEstore, u4ExpMEstore, u4ExpSEstore);
    else
        AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, u4ExpLEstore, u4ExpSEstore, 0);
}

MVOID
AeSettingVHDR::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_SETTING_CPU_TRIGER_T etrigger = (E_AE_SETTING_CPU_TRIGER_T) index;
    switch (etrigger)
    {
        case E_AE_SETTING_CPU_SENSORGAINTHREAD:
            if (m_pSensorGainThread == NULL)
                m_bSGainThdFlg = MTRUE;
            break;
        case E_AE_SETTING_CPU_FRAMERATE:
            m_bSetFrameRateValue = btrigger;
            break;
        case E_AE_SETTING_CPU_SHUTTER:
            m_bSetShutterValue = btrigger;
            break;
        case E_AE_SETTING_CPU_SENSORGAIN:
            m_bSetGainValue = btrigger;
            break;
        case E_AE_SETTING_CPU_STORE:
            u4ExpLEstore = u4ExpLEperframe;
            u4ExpSEstore = u4ExpSEperframe;
            u4ExpMEstore = u4ExpMEperframe;
            u4AfeLEstore = u4AfeLEperframe;
            u4AfeSEstore = u4AfeSEperframe;
            u4AfeMEstore = u4AfeMEperframe;
            AE_SETTING_LOG("[%s()] Exp:%d/%d/%d Afe:%d/%d/%d", __FUNCTION__, u4ExpLEperframe, u4ExpSEperframe, u4ExpMEperframe, u4AfeLEperframe, u4AfeSEperframe, u4AfeMEperframe);
            break;
        default:
            AE_SETTING_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
    }
}

AeSettingVHDR::
~AeSettingVHDR()
{
    AE_SETTING_LOG("[AeSettingVHDR:Destruct] SensorDev:%d\n", (MUINT32)m_eSensorDev);
}

