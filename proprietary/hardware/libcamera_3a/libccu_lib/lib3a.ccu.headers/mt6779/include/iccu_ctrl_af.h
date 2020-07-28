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
#ifndef _ICCU_CTRL_AF_H_
#define _ICCU_CTRL_AF_H_
#include "iccu_ctrl.h"
#include "ccu_ext_interface/ccu_af_extif.h"
#include "ccu_ext_interface/af_cxu_param.h"
//using namespace NS3Av3;

namespace NSCcuIf {
#define AFO_BUF_FH_OFFSET 16
#define CCU_SUPPORT_VBLANKRANGE 800

#define AFO_WIN_NUM (128)
#define AFO_WIN_DATA (32) // byte
#define AFO_STRIDE_NUM (1)
#define AFO_BUF_SIZE (660480)//((AFO_WIN_NUM + AFO_STRIDE_NUM) * AFO_WIN_NUM * AFO_WIN_DATA)static volatile int tempcount;

//ICcuCtrlAf extends ICcuCtrl, add AF specific operations
class ICcuCtrlAf : public virtual ICcuCtrl
{
public:

    static ICcuCtrlAf* createInstance(uint8_t const szUsrName[32],ESensorDev_T sensorDev);

    virtual void  destroyInstance(void) = 0;

    ICcuCtrlAf(void){}

    virtual ~ICcuCtrlAf(void){}

    //virtual bool init(ESensorDev_T sensorDev) = 0;

    //virtual bool uninit() = 0;

    /*AfAlgoOutput type not defined yet, use void* for build pass*/
    virtual bool ccu_af_initialize() = 0;

    virtual bool ccu_af_start(MUINT32 initialpos) = 0;

    virtual bool ccu_af_stop() = 0;

    virtual bool ccu_af_resume() = 0;

    virtual bool ccu_af_suspend() = 0;

    virtual void ccu_af_deque_afo_buffer(AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout) = 0;

    virtual bool ccu_af_enque_afo_buffer(void *AfoBufferPtr) = 0;

    /*AfAlgoOutput type not defined yet, use void* for build pass*/
    virtual bool ccu_af_set_hw_regs(void *AfAlgoOutput) = 0;
    virtual bool ccu_af_abort() = 0;

    bool afo_done_valid = true;

    //init ccu af control
    virtual bool ccu_af_control_initialize(char* mcuName) = 0;
    //init AFalgo and enable afo interrupt
    virtual bool ccu_af_init_hwconfig(ConfigAFInput_T *firstAFhwconfig, AF_HW_INIT_OUTPUT_T *configOutput) = 0;
    //init AFalgo and enable afo interrupt
    virtual bool ccu_af_start_and_init_algo(CCUStartAFInput_T *InitAFInput, StartAFOutput_T *InitAFOutput) = 0;
    //init AFalgo to update_nvram
    virtual bool ccu_af_update_nvram(StartAFInput_t InitAFInput, StartAFOutput_t *InitAFOutput) = 0;
    //for intput of command and data from AFmgr to algo and wait output for per frame state update
    virtual void ccu_af_doAF(DoAFInput_T *doAFinput) = 0;
    //for capture or query result
    virtual bool ccu_af_update_doAFoutput(DoAFOutput_T *doAFoutput) = 0;

    virtual bool ccu_af_get_PD_block(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T* pdBlockOutput) = 0;

    virtual bool ccu_af_get_DebugInfo(EXT_AF_DEBUG_INFO_T *a_DebugInfo) = 0;

    //for PDAF
    virtual bool ccu_af_get_curposition(MINT32 *curposition,MINT32 *curtime_H,MINT32 *curtime_L,MINT32 *lastposition,MINT32 *lasttime_H,MINT32 *lasttime_L) = 0;
    virtual bool ccu_af_get_AFOinfo(AFAcquireBuf_Reg_INFO_OUT_AP_T AFOinfo) = 0;

    //for SYNCAF
    virtual bool ccu_af_syncAf_process(MINT32 masterDev, MINT32 masterIdx, MINT32 slaveDev, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T *syncInfo) = 0;
};
};  //namespace NSCcuAFIf
#endif
