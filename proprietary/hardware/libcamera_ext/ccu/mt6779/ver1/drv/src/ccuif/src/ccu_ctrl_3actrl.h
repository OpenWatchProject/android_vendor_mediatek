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
#ifndef _CCU_CTRL_3A_CTRL_H_
#define _CCU_CTRL_3A_CTRL_H_

#include "iccu_ctrl_3actrl.h"

namespace NSCcuIf {

class CcuCtrl3ACtrl : AbsCcuCtrlBase, public ICcuCtrl3ACtrl
{
public:
	int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);
	void destroyInstance(void);
	MBOOL isSupportFrameSync(void);
    MBOOL isSupport3ASync(void);
    void getDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo);
    //getAeDebugInfo: returns request number
    uint32_t getAeDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, AE_DEBUG_INFO_T *a_rAEDebugInfo, AE_DEBUG_DATA_T *a_rAEDebugData);
    //getAeInfo: returns request number
    uint32_t getAeInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, AE_INFO_T *a_rAEInfo, MUINT64 *u8RealEposuretime = NULL);
    //returns request number
    uint32_t getLtmDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info);
    //returns request number
    uint32_t getLtmInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct ccu_ltm_info_isp *ltm_info);
    //returns request number
    uint32_t getHlrDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info);
    //returns request number
    uint32_t getHlrInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct ccu_hlr_info_isp *hlr_info);

private:
    enum ccu_feature_type _getFeatureType();
    enum ccu_tg_info getCcuTgInfo();
    
    void fillUpN3AedDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo, COMPAT_CCU_SYNC_AE_INPUT_T *a_rAesyncInData, CCU_SYNC_AE_OUTPUT_T *a_rAesyncOutData);
    
    bool ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess(uint32_t msgId, void *outDataPtr, void *inDataBuf) { return true; };
    bool ccuCtrlPreprocess_SetSyncState(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_SyncAeSetteings(void *inDataPtr, void *inDataBuf);
};

};  //namespace NSCcuIf

#endif
