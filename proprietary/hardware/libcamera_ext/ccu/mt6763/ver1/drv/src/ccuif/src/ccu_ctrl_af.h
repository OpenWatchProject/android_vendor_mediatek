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
#ifndef _CCU_CTRL_AF_H_
#define _CCU_CTRL_AF_H_

#include "iccu_ctrl_af.h"

namespace NSCcuIf {

enum CcuCtrlAfErrorCode
{
    CCU_CTRL_AF_SUCCEED = 0,
    CCU_CTRL_AF_ERR_GENERAL,
};

class CcuCtrlAf : AbsCcuCtrlBase, public ICcuCtrlAf
{
	void destroyInstance(void);
/*
    bool init();

    bool uninit();
    */
    bool ccu_af_start();
    bool ccu_af_initialize();
    void ccu_af_deque_afo_buffer(AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout);
    bool ccu_af_stop();

    bool ccu_af_enque_afo_buffer(void *AfoBufferPtr);

    //AfAlgoOutput type not defined yet, use void* for build pass
    bool ccu_af_set_hw_regs(void *AfAlgoOutput);
    bool ccu_af_abort();
private:

    void afCtrlInitBuf(int device, U32 *AFORegAddr);

    void vaTomva(int device,char * bufinfo);

    //void mvaTova(int device,AFAcquireBuf_Reg_INFO_OUT_T AFAcquireBufout,AFAcquireBuf_Reg_INFO_OUT_AP_T* rAFAcquireBufout);

    bool ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr);

    bool ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr);

};

};  //namespace NSCcuIf

#endif