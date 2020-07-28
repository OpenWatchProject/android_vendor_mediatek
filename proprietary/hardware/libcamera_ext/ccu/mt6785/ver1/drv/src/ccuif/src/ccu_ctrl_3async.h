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
#ifndef _CCU_CTRL_3A_SYNC_H_
#define _CCU_CTRL_3A_SYNC_H_

#include "iccu_ctrl_3async.h"

namespace NSCcuIf {

class CcuCtrl3ASync : AbsCcuCtrlBase, public ICcuCtrl3ASync
{
public:
	int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);
	void destroyInstance(void);
	MBOOL isSupportFrameSync(void);
    MBOOL isSupport3ASync(void);
    void getDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo);

private:
    enum ccu_feature_type _getFeatureType();
    enum ccu_tg_info getCcuTgInfo();
    void fillUpN3AedDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo, CCU_N3D_AE_INIT_STRUCT *gN3d_ae_init_info, CCU_N3DAE_STRUCT *m_n3dAE, CCU_strAEOutput *m_AE_Output);
    bool ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr, void *inDataBuf) { return true; };
    bool ccuCtrlPreprocess_SetSyncState(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_SyncAeSetteings(void *inDataPtr, void *inDataBuf);
};

};  //namespace NSCcuIf

#endif