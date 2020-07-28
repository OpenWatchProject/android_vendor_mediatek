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
#ifndef _AE_FLOW_DFT_H_
#define _AE_FLOW_DFT_H_

#include <aaa_types.h>
#include <isp_tuning.h>
#include <AeFlow/IAeFlow.h>
#include <dbg_aaa_param.h>
#include <dbg_ae_param.h>
#include "algorithm/cpu_ae_algo_ctrl_if.h"

using namespace NSIspTuning;
namespace NS3Av3
{
class AeFlowDefault : public IAeFlow
{
public:

    static IAeFlow*    getInstance(ESensorDev_T eSensorDev);
    virtual MVOID      init(){};
    virtual MVOID      uninit() {}
    virtual MVOID      start(MVOID* input);
    virtual MVOID      pause() {}
    virtual MVOID      stop() {}
    virtual MBOOL      queryStatus(MUINT32 index);
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger);
    virtual MBOOL      calculateAE(MVOID* input, MVOID* output);
    virtual MVOID      getCCUResult(MVOID* poutput){(void)poutput;}
    virtual MVOID      getAEInfoForISP(AE_INFO_T &ae_info);
    virtual MVOID      getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info);
    virtual MVOID      controlCCU(AE_FLOW_CCU_CONTROL_INFO_T* pcontrolInfo){(void)pcontrolInfo;}

protected:
                       AeFlowDefault(ESensorDev_T eSensorDev, CPU_AE_CORE_CAM_ID_ENUM eCamID);
    virtual            ~AeFlowDefault();

    ESensorDev_T m_eSensorDev;
    CPU_AE_CORE_CAM_ID_ENUM m_eCamID;
    MUINT32 m_3ALogEnable;
    MBOOL m_bTriggerFrameInfoUpdate;
    MBOOL m_bTriggerOnchInfoUpdate;
};
};
#endif //_AE_FLOW_DFT_H_
