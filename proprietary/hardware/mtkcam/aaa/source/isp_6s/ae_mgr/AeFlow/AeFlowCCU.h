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
#ifndef _AE_FLOW_CCU_H_
#define _AE_FLOW_CCU_H_


#include <aaa_types.h>
#include <isp_tuning.h>
#include <AeFlow/IAeFlow.h>
#include <mtkcam/drv/IHalSensor.h>

using namespace NSIspTuning;
using namespace NSCam;
using namespace NSCcuIf;

namespace NS3Av3
{
class AeFlowCCU : public IAeFlow
{
public:

    static IAeFlow*    getInstance(ESensorDev_T sensor);
    virtual MVOID      init();
    virtual MVOID      uninit();
    virtual MVOID      start(MVOID* pAEInitInput);
    virtual MVOID      pause();
    virtual MVOID      stop();
    virtual MBOOL      queryStatus(MUINT32 index);
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger);
    virtual MBOOL      calculateAE(MVOID* input, MVOID* output){(void)input;(void)output;return MTRUE;}
    virtual MVOID      getCCUResult(MVOID* poutput);
    virtual MVOID      getAEInfoForISP(AE_INFO_T &ae_info);
    virtual MVOID      getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info);
    virtual MVOID      controlCCU(AE_FLOW_CCU_CONTROL_INFO_T* pcontrolInfo);

protected:
                         AeFlowCCU(ESensorDev_T sensor);
    virtual            ~AeFlowCCU();

private:

    //MVOID getCCUResult(AE_CORE_MAIN_OUT* poutput);
    MUINT getSensorId(MUINT m_i4SensorIdx, MUINT m_eSensorDev);
    MBOOL IsCCUWorking();
    MBOOL IsCCUAEWorking();
    MVOID startCCUAE();

private:
    ESensorDev_T m_eSensorDev;
    MINT32 m_i4SensorIdx;
    ESensorMode_T m_eSensorMode;
    NSCcuIf::ICcuCtrlAe* m_pICcuAe;
    MBOOL m_bIsCCUStart;
    MBOOL m_bIsCCUWorking;
    MBOOL m_bIsCCUAEWorking;
    MBOOL m_bIsCCUPaused;
    MBOOL m_bIsCCUResultGet;
    MBOOL m_bCCUAEFlag;
    MBOOL m_bCCUIsSensorSupported;
    MBOOL m_bTriggerFrameInfoUpdate;
    MBOOL m_bTriggerOnchInfoUpdate;
    MBOOL m_bTriggerMaxFPSUpdate;
    MBOOL m_bTriggerFlickActiveUpdate;
    MBOOL m_bFlickActive;
    MBOOL m_3ALogEnable;
    //MBOOL m_bIsDoPvAEControlCCU;
};


};
#endif //_AE_FLOW_CCU_H_
