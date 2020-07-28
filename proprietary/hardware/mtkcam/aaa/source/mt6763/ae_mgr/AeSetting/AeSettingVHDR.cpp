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
{
    AE_SETTING_LOG("Enter AeSettingVHDR DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeSettingVHDR::
updateSensor(MVOID* input)
{
    AE_SETTING_CONTROL_INFO_T* pinput = (AE_SETTING_CONTROL_INFO_T*) input;

    if(m_bSetFrameRateValue) {     // update frame rate
        m_bSetFrameRateValue = MFALSE;
        AaaTimer localTimer("SetFrameRater", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, pinput->u4UpdateFrameRate_x10, m_eSensorMode);
        localTimer.End();
    }

    if(m_bSetShutterValue) {    // update shutter value
        // 0/0/2 and 0/1/2 sensor both use it
        AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, pinput->u4LEHDRshutter, pinput->u4SEHDRshutter);
        m_bSetShutterValue = MFALSE;
        AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] Exposure time LE/SE value = %d/%d\n", __FUNCTION__, pinput->u4LEHDRshutter, pinput->u4SEHDRshutter);
        // Update AWB gain to sensor
        if(m_bUpdateSensorAWBGain) {
            strSensorAWBGain rSensorAWBGain;
            AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
            rSensorAWBGain.u4R = rCurrentIspAWBGain.i4R;
            rSensorAWBGain.u4GR = rCurrentIspAWBGain.i4G;
            rSensorAWBGain.u4GB = rCurrentIspAWBGain.i4G;
            rSensorAWBGain.u4B = rCurrentIspAWBGain.i4B;
            AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)m_eSensorDev, &rSensorAWBGain);
        }
    }
    if(m_bSetGainValue) {    // update sensor gain value
        AE_SETTING_LOG_IF(m_3ALogEnable,"[%s()] Sensor gain LE/SE value = %d/%d\n", __FUNCTION__, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue2);
        if (!m_pSensorGainThread){  // 0/0/2 sensor
            if(pinput->eAETargetMode == AE_MODE_ZVHDR_TARGET) {
                AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue, pinput->u4UpdateGainValue2);
            }else{
                AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, pinput->u4UpdateGainValue);
            }
        } else {                    // 0/1/2 sensor
            if(pinput->eAETargetMode == AE_MODE_ZVHDR_TARGET) {
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->u4UpdateGainValue,pinput->u4UpdateGainValue2, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }else{
                ThreadSensorGain::Cmd_T rCmd(0, 0, pinput->u4UpdateGainValue, m_eSensorTG);
                m_pSensorGainThread->postCmd(&rCmd);
            }
        }
        m_bSetGainValue = MFALSE;
    }
}

AeSettingVHDR::
~AeSettingVHDR()
{
    
    AE_SETTING_LOG("Delete ~AeSettingVHDR - DeviceId:%d",(MUINT32)m_eSensorDev);

}


