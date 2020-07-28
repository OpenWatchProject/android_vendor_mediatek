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
#ifdef CCU_AF_ENABLE
#ifndef _CCU_CTRL_AF_H_
#define _CCU_CTRL_AF_H_

#include "iccu_ctrl_af.h"
#include "ccu_ext_interface/ccu_lens_extif.h"

//using namespace NS3Av3;

namespace NSCcuIf {

enum CcuCtrlAfErrorCode
{
    CCU_CTRL_AF_SUCCEED = 0,
    CCU_CTRL_AF_ERR_GENERAL,
};

class CcuCtrlAf : AbsCcuCtrlBase, public ICcuCtrlAf
{
public:
    int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);
	void destroyInstance(void);
    bool ccu_af_start(MUINT32 initialpos);
    bool ccu_af_initialize();
    void ccu_af_deque_afo_buffer(AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout);
    bool ccu_af_stop();
    bool ccu_af_enque_afo_buffer(void *AfoBufferPtr);
    //AfAlgoOutput type not defined yet, use void* for build pass
    bool ccu_af_set_hw_regs(void *AfAlgoOutput);
    bool ccu_af_resume();
    bool ccu_af_suspend();
    bool ccu_af_abort();

    //init ccu af control
    bool ccu_af_control_initialize(char* mcuName);
    //init AFalgo and enable afo interrupt
    bool ccu_af_init_hwconfig(ConfigAFInput_T *firstAFhwconfig, AF_HW_INIT_OUTPUT_T *configOutput);
    //init AFalgo and enable afo interrupt
    bool ccu_af_start_and_init_algo(CCUStartAFInput_T *InitAFInput, StartAFOutput_T *InitAFOutput);
    //init AFalgo to update_nvram
    bool ccu_af_update_nvram(StartAFInput_t InitAFInput, StartAFOutput_t *InitAFOutput);
    //for intput of command and data from AFmgr to algo and wait output for per frame state update
    void ccu_af_doAF(DoAFInput_T *doAFinput);
    //for capture or query result
    bool ccu_af_update_doAFoutput(DoAFOutput_T *doAFoutput);

    bool ccu_af_get_PD_block(GetPdBlockInput_T pdBlockInut, GetPdBlockOutput_T* pdBlockOutput);

    bool ccu_af_get_DebugInfo(EXT_AF_DEBUG_INFO_T *a_DebugInfo);

    //for PDAF
    bool ccu_af_get_curposition(MINT32 *curposition,MINT32 *curtime_H,MINT32 *curtime_L,MINT32 *lastposition,MINT32 *lasttime_H,MINT32 *lasttime_L);
    bool ccu_af_get_AFOinfo(AFAcquireBuf_Reg_INFO_OUT_AP_T AFOinfo);

    //for SYNCAF
    bool ccu_af_syncAf_process(MINT32 masterDev, MINT32 masterIdx, MINT32 slaveDev, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T *syncInfo);

private:
    CcuAfBufferList *m_afBufferList = NULL;
    CcuBuffer AFO_BUF[Half_AFO_BUFFER_COUNT];
    MUINT32 sensorDEVin;
    I8 mcuName_in[32];
    enum ccu_feature_type _getFeatureType();
    void afCtrlInitBuf(MUINT32 sensorIdx);
    void vaTomva(int device,char * bufinfo);
    //void mvaTova(int device,AFAcquireBuf_Reg_INFO_OUT_T AFAcquireBufout,AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout);
    bool ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess(uint32_t msgId, void *outDataPtr, void *inDataBuf);
};

};  //namespace NSCcuIf

/*
mosesq 'echo setenv; source build/envsetup.sh 1>0; lunch full_k71v1_64_bsp 1>0; mmma vendor/mediatek/proprietary/hardware/libcamera_3a/libaf_core 2>&1 -j48 | tee ccu_build.log'
*/
#endif
#endif //CCU_AF_ENABLE