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
    virtual MVOID      pause(MBOOL IsNeedCCUSensorInit);
    virtual MVOID      stop();
    virtual MBOOL      queryStatus(MUINT32 index);
    virtual MVOID      getParam(MUINT32 index, MUINT32& i4value){(void)index;(void)i4value;}
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger);
    virtual MVOID      schedulePvAE(MVOID* input, MVOID* output){(void)input;(void)output;}
    virtual MVOID      calculateAE(MVOID* input, MVOID* output){(void)input;(void)output;}
    virtual MVOID      setupParamPvAE(MVOID* input, MVOID* output);
    virtual MVOID      update(AE_CORE_CTRL_CPU_EXP_INFO* input);
    virtual MVOID      getdebuginfo(AE_DEBUG_INFO_T &exif_info);
    virtual MVOID      getAEInfoForISP(AE_INFO_T &ae_info);
    virtual MVOID      PresetControlCCU(AE_FLOW_CONTROL_INFO_T* pcontrolInfo);
    virtual MVOID      setMVHDR3ExpoProcInfo(MVOID* pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100){(void)pMVHDR3ExpoStatBuf;(void)u4MVHDRRatio_x100;}
    virtual MVOID      getCCUResult(AE_CORE_MAIN_OUT* poutput,MBOOL isCCUResultGet);

//protected:
                         AeFlowCCU(ESensorDev_T sensor);
    virtual            ~AeFlowCCU();

private:

    MVOID controlCCU(AE_FLOW_CONTROL_INFO_T* pcontrolInfo,MBOOL bIsDoPvAEControlCCU);
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
    //MBOOL m_bCCUAEFlag;  /* fix build warning */
    MBOOL m_bCCUIsSensorSupported;
    MBOOL m_bEnableFlickerFPSstate;
    MBOOL m_bControlCCUInfochange;
    MBOOL m_bControlOBCInfochange;
    MINT32 m_i4OBCTableidx;
    MBOOL m_bSetCCUFrameRate;
    MBOOL m_3ALogEnable;
    ccu_ae_onchange_data rOnchangeData;
    //MBOOL m_bIsDoPvAEControlCCU;
    MBOOL m_bIsLongExpControlbyCPU;
    MBOOL m_bIsFirstcontrolCCU;
    MBOOL m_bIsNeedCCUSensorInit;
};


};
#endif //_AE_FLOW_CCU_H_
