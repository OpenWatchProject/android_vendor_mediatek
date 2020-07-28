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
#ifndef _CCU_CTRL_LTM_H_
#define _CCU_CTRL_LTM_H_

#include "ccu_ctrl.h"
#include "iccu_ctrl_ltm.h"
#include "ccu_ext_interface/ccu_control_extif.h"
#include "algorithm/ccu_ltm_data.h"
#include "algorithm/ccu_ltm_nvram.h"
#include "algorithm/ccu_hlr_data.h"
#include "algorithm/ccu_hlr_nvram.h"
#include <debug_exif/aaa/dbg_isp_param.h>


namespace NSCcuIf {

class CcuCtrlLtm : AbsCcuCtrlBase, public ICcuCtrlLtm
{
public:
    int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);
    int uninit();
    void destroyInstance(void);
    bool getLtmInitStatus();
    uint32_t getLtmInfoExif(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info);
    uint32_t getLtmInfoIsp(struct ccu_ltm_info_isp *ccu_ltm_isp);
    uint32_t getHlrInfoExif(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info);
    uint32_t getHlrInfoIsp(struct ccu_hlr_info_isp *ccu_hlr_isp);

private:
    enum ccu_feature_type _getFeatureType();
    bool ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess(uint32_t msgId, void *outDataPtr, void *inDataBuf) { return true; };
    bool ccuCtrlPreprocess_LtmInit(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_LtmPerframeCtrl(void *inDataPtr, void *inDataBuf);

    void fillUpLtmDebugInfo(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info, ccu_ltm_algo_data *p_ltm_algo_data);
    void setDebugTag(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info, unsigned int index, int value);
	void fillUpHlrDebugInfo(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info, ccu_hlr_algo_data *p_hlr_algo_data);
    void setHlrDebugTag(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info, unsigned int index, unsigned int value);
};

};  //namespace NSCcuIf

#endif