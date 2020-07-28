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
#define HDR_SENSOR_DATA_SIZE (1024*100) //100k

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
    //from ccu_ae_ctrldata_onchange
    CCU_strEvPline *pCurrentTableOnch;
    CCU_strFinerEvPline *pCurrentTableFOnch;
};

class CcuCtrlAe : AbsCcuCtrlBase, public ICcuCtrlAe
{
public:
	int init(MUINT32 sensorIdx, ESensorDev_T sensorDev);
    uint32_t getDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, AE_DEBUG_DATA_T *a_rAEDebugData);
    uint32_t getAeInfoForIsp(AE_INFO_T *a_rAEInfo);
	void destroyInstance(void);
    void getHdrDataBuffer(void **bufferAddr0, void **bufferAddr1);
    bool getCcuAeOutput(struct ccu_ae_output *output);
    bool getCcuAeStat(CCU_AE_STAT *output);

private:
    inline void setDebugTag(AE_DEBUG_INFO_T *a_rAEDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
    {
        a_rAEDebugInfo->Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_AE_MODULE_ID, a_i4ID, 0);
        a_rAEDebugInfo->Tag[a_i4ID].u4FieldValue = a_i4Value;
    }

    
    void fillUpN3AedDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo, CCU_SYNC_AE_INPUT_T *a_rAesyncInData, CCU_SYNC_AE_OUTPUT_T *a_rAesyncOutData);
    void fillUpAeDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, COMPAT_CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData);
    void fillUpAeDebugData(AE_DEBUG_DATA_T *a_rAEDebugData, COMPAT_CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData);
    void fillUpAeInfo(AE_INFO_T *rAEISPInfo, COMPAT_CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData);
    enum ccu_feature_type _getFeatureType();
    bool ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess(uint32_t msgId, void *outDataPtr, void *inDataBuf);
	bool ccuCtrlPreprocess_AeAlgoInit(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_PerframeData(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_OnchangeData(void *inDataPtr, void *inDataBuf);
	bool ccuCtrlPreprocess_SensorInit(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPreprocess_HdrInputDataReady(void *inDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess_SensorInit(void *outDataPtr, void *inDataBuf);
    bool ccuCtrlPostprocess_GetCcuOutput(void *outDataPtr);
    bool ccuCtrlPreprocess_CcuAeStop(void *inDataPtr, void *inDataBuf);

    struct CcuAeExifDataPtr m_CcuAeExifDataPtr = {0};
    bool m_isSpSensor = 0;
};

};  //namespace NSCcuIf

#endif
