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
#ifndef _CCU_CTRL_AE_H_
#define _CCU_CTRL_AE_H_

#include "iccu_ctrl_ae.h"
#include "dbg_ae_param.h"
#include <debug_exif/cam/dbg_cam_n3d_param.h>
#include "aaa/ae_param.h"
#include "algorithm/ccu_ae_algo_data.h"
#include "algorithm/ccu_n3d_ae_algo_data.h"

namespace NSCcuIf {

enum CcuCtrlAeErrorCode
{
    CCU_CTRL_AE_SUCCEED = 0,
    CCU_CTRL_AE_ERR_GENERAL,
};

struct CcuAeExifDataPtr
{
    //from AE_CORE_INIT
    MINT32 *m_LumLog2x1000;
    CCU_strFinerEvPline *pCurrentTableF;
    CCU_strEvPline *pCurrentTable;
    MUINT32 *pEVValueArray;
    CCU_strAEMovingRatio *pAETouchMovingRatio;
    CCU_AE_NVRAM_T* pAeNVRAM;
    //from ccu_ae_onchange_data
    CCU_strEvPline *pCurrentTableOnch;
    CCU_strFinerEvPline *pCurrentTableFOnch;
};

class CcuCtrlAe : AbsCcuCtrlBase, public ICcuCtrlAe
{
public:
	int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);
    void getDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo);
    void getAeInfoForIsp(AE_INFO_T *a_rAEInfo);
	void destroyInstance(void);

private:
    inline void setDebugTag(AE_DEBUG_INFO_T *a_rAEDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
    {
        a_rAEDebugInfo->Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_AE_MODULE_ID, a_i4ID, 0);
        a_rAEDebugInfo->Tag[a_i4ID].u4FieldValue = a_i4Value;
    }

    void fillUpN3AedDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo, CCU_N3D_AE_INIT_STRUCT *gN3d_ae_init_info, CCU_N3DAE_STRUCT *m_n3dAE, CCU_strAEOutput **m_AE_Output);
    void fillUpAeDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData, AE_CORE_CTRL_CCU_VSYNC_INFO *aeVsyncInfo);
    void fillUpAeInfo(AE_INFO_T *rAEISPInfo, CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData);
    enum ccu_feature_type _getFeatureType();
    bool ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr, void *inDataBuf);
	bool ccuCtrlPreprocess_AeInit(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_OnchangeData(void *inDataPtr, void *inDataBuf);
	bool ccuCtrlPreprocess_SensorInit(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess_SensorInit(void *outDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess_GetCcuOutput(void *outDataPtr);

    struct CcuAeExifDataPtr m_CcuAeExifDataPtr = {0};
    bool m_isSpSensor = 0;
};

};  //namespace NSCcuIf

#endif