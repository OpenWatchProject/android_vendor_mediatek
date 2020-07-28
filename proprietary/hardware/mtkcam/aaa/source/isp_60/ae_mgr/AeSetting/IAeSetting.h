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
#ifndef _I_AE_SETTING_H_
#define _I_AE_SETTING_H_


#include <aaa_types.h>
#include <isp_tuning.h>
#include <ae_feature.h>
#include <aaa/ae_param.h>
#include <custom/aaa/AEPlinetable.h>

#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

typedef enum
{
    E_AE_SETTING_CPU = 0,    // [CPU AE algo => CPU setting]
    E_AE_SETTING_CCU_FULL,   // [CPU AE algo => CCU setting]
    E_AE_SETTING_CCU_MANUAL  // [CCU AE algo => CCU setting] only for manual cases and capture/restore
} E_AE_SETTING_TYPE_T;

typedef enum
{
    E_AE_SETTING_CPU_STATUS_NEEDUPDATED = 0,
    E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD
} E_AE_SETTING_CPU_STATUS_T;

typedef enum
{
    E_AE_SETTING_CPU_SENSORGAINTHREAD = 0,
    E_AE_SETTING_CPU_FRAMERATE,
    E_AE_SETTING_CPU_SHUTTER,
    E_AE_SETTING_CPU_SENSORGAIN
} E_AE_SETTING_CPU_TRIGER_T;

typedef struct
{
    MINT32 i4SensorIdx;
    ESensorTG_T eSensorTG;
    ESensorMode_T eSensorMode;
    eAETargetMODE eAETargetMode;

}AE_SETTING_INPUT_INFO_T;

typedef struct
{
    MUINT32 u4UpdateFrameRate_x10;
    strAERealSetting rAERealSetting;
    strAERealSetting rAERealSettingBeforeConv; // setting before convert (for CCU AE only)
    MBOOL bCcuPresetControl;
    MBOOL bSensorAWBGain;
    MINT32 i4Pass2SensorLVvalue;

}AE_SETTING_CONTROL_INFO_T;

namespace NS3Av3
{
class IAeSetting
{
public:

    typedef enum
    {
        E_AE_SETTING_TYPE_DFT = 0,
        E_AE_SETTING_TYPE_CCU
    } E_AE_SETTING_TYPE_T;

    static IAeSetting*    getInstance(E_AE_SETTING_TYPE_T etype, ESensorDev_T sensor);
    virtual MVOID      init() = 0;
    virtual MVOID      uninit() = 0;
    virtual MVOID      start(MVOID* pAInput) = 0;
    virtual MVOID      stop() = 0;

    virtual MBOOL      queryStatus(MUINT32 index) = 0;
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger = MFALSE) = 0;
    virtual MVOID      updateExpoSettingbyI2C(MVOID* input) = 0;
    virtual MBOOL      updateExpoSetting(MVOID* input) = 0;
    virtual MVOID      updateSensorMaxFPS(MVOID* input) = 0;

//protected:
    virtual            ~IAeSetting(){}
};
#define MY_INST_AESETTING INST_T<IAeSetting>
};
#endif //_I_AE_SETTING_H_
