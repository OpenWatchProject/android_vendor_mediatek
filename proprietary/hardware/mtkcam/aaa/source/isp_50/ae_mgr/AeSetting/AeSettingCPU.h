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
#ifndef _AE_SETTING_CPU_H_
#define _AE_SETTING_CPU_H_


#include <aaa_types.h>
#include <isp_tuning.h>
#include <AeSetting/IAeSetting.h>
#include <IThread.h>

using namespace NSIspTuning;
namespace NS3Av3
{
class AeSettingCPU : public IAeSetting
{
public:

    static IAeSetting*    getInstance(ESensorDev_T sensor);

    virtual MVOID      init();
    virtual MVOID      uninit();
    virtual MVOID      start(MVOID* pAInput);
    virtual MVOID      stop();

    virtual MBOOL      queryStatus(MUINT32 index);
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger);
    virtual MVOID      updateSensor(MVOID* input);
    virtual MVOID      updateSensorFast(MVOID* input);
    virtual MVOID      updateSensorGain(MVOID* input);
    virtual MVOID      updateSensorExp(MVOID* input);
    virtual MVOID      updateISP(MVOID* input);

//protected:
                         AeSettingCPU(ESensorDev_T sensor);
    virtual            ~AeSettingCPU();

private:
    ESensorDev_T m_eSensorDev;
    eAETargetMODE m_eAETargetMode;
    IAeSetting* m_pAeSettingVHDR;
    IAeSetting* m_pAeSettingDefault;
};
};
#endif //_AE_SETTING_CPU_H_