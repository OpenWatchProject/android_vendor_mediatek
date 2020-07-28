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
#ifndef _CCU_CTRL_H_
#define _CCU_CTRL_H_

#include "ccuif_compat.h"
#include "ccuif_compat_conv.h"
#include "iccu_ctrl.h"
#include "ccu_mgr.h"
#include "ccu_udrv.h"

namespace NSCcuIf {

class AbsCcuCtrlBase: public virtual ICcuCtrl
{

public:
    AbsCcuCtrlBase(void);

    virtual int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);

    virtual int uninit();

    virtual int ccuControl(ccu_msg_id msgId, void *inDataPtr, void *outDataPtr);

private:
    virtual int ccu_control_handle_input(ccu_msg_id msgId, const struct CcuCtrlParamDescriptor *paramDescptr, void *inDataPtr);

    virtual int send_ccu_command(ccu_msg_id msgId);

protected:
    virtual enum ccu_tg_info getCcuTgInfo();

    virtual enum ccu_tg_info sensorDevToTgInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev);

    virtual bool ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr) = 0;

    virtual bool ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr) = 0;

    CcuDrvImp *m_pDrvCcu;

    CcuBufferList m_BufferList;

    MUINT32 m_sensorIdx;

    ESensorDev_T m_sensorDev;

    static android::Mutex   m_CcuccuControlMutex;

};

};  //namespace NSCcuIf

#endif