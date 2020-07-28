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
#include <isp_tuning_mgr.h>




#define AE_SETTING_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

        
        
using namespace NS3Av3;
using namespace NSIspTuning;


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
{
    
    AE_SETTING_LOG("Enter AeSettingCCU DeviceId:%d",(MUINT32)m_eSensorDev);
}

AeSettingCCU::
~AeSettingCCU()
{
    
    AE_SETTING_LOG("Delete ~AeSettingCCU - DeviceId:%d",(MUINT32)m_eSensorDev);

}


