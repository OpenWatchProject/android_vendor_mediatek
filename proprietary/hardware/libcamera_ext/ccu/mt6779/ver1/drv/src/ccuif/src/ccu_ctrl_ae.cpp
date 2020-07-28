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
#define LOG_TAG "ICcuCtrlAe"

#include "ccu_ctrl.h"
#include "ccu_ctrl_ae.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include "kd_camera_feature.h"/*for IMGSENSOR_SENSOR_IDX*/
#include "kd_ccu_i2c.h"
#include "n3d_sync2a_tuning_param.h"
#include "utilSystrace.h"
#include "ccu_drvutil.h"
#include "ccu_mem_mgr.h"
#ifndef min
#define min(a,b)  ((MINT32)(a) < (MINT32)(b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)  ((MINT32)(a) > (MINT32)(b) ? (a) : (b))
#endif

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
EXTERN_DBG_LOG_VARIABLE(ccu_drv);
static CcuCtrlAe _ccuCtrlAeSingleton[IMGSENSOR_SENSOR_IDX_MAX_NUM];

/*******************************************************************************
* Class Function
********************************************************************************/
ICcuCtrlAe *ICcuCtrlAe::getInstance(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
	return (ICcuCtrlAe *)&_ccuCtrlAeSingleton[sensorIdx];
}

bool ICcuCtrlAe::getHdrDataBuffer(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct CcuAeHdrBufferSet *bufferSets)
{    
    ICcuMgrPriv *ccuMgr = ICcuMgrPriv::createInstance();
    if(ccuMgr == NULL)
    {
        LOG_ERR("ccuMgr is NULL");
        return false;
    }
    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)ccuMgr->getCcuBufferList(CCU_FEATURE_AE, sensorIdx);;
    ccuMgr->destroyInstance();
    
    for(int i=0 ; i<CCU_HDR_DATA_BUF_CNT ; i++)
    {
        bufferSets[i].va = aeBufferList->hdrDataBuffer[i].getVa();
        bufferSets[i].mva = aeBufferList->hdrDataBuffer[i].getMva();
        LOG_DBG("fill HDR AE bufferSet[%d]: va(%p), mva(%x)", i, bufferSets[i].va, bufferSets[i].mva);
    }
    return true;
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int CcuCtrlAe::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
	LOG_DBG("+:%s\n",__FUNCTION__);
    uint8_t sensor_slot;
    enum ccu_tg_info tgInfo;

	int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
	if(ret != CCU_CTRL_SUCCEED)
	{
		return ret;
	}

    if (!m_pDrvCcu->allocSensorSlot(sensorIdx, &sensor_slot)){
        LOG_WRN("allocSensorSlot fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }

	if (!m_pDrvCcu->loadSensorBin(sensorIdx, sensor_slot, &m_isSpSensor))
    {
        LOG_WRN("loadSensorBin fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }
    if (!m_pDrvCcu->loadCustSensorBin(sensorIdx, sensor_slot))
    {
        LOG_WRN("loadCustSensorBin fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }
    if (!m_pDrvCcu->loadCustFuncBin())
    {
        LOG_WRN("loadCustFuncBin fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }

    //get instance index of corresponding CCU AE object, and bind it with this CcuCtrlAe instance
    struct ccu_ae_bind_instance_output bind_instance_info = {0};
    ccuControl(MSG_TO_CCU_AE_BIND_INSTANCE, NULL, &bind_instance_info);
    m_ccuInstanceIdx = bind_instance_info.instance_idx;
    LOG_DBG_MUST("Bind this instance of sensorIdx(%d) to CCU AE instanceIdx(%d)", m_sensorIdx, m_ccuInstanceIdx);

    //fill-up active output buffers
    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;
    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    //init ridx as -1, to prevent getting garbage data in first read

    LOG_DBG_MUST("bypass ae_output_ridx initialization");
    //sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_output_ridx = -1;
    sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_stat_ridx = -1;

    //fill-up ae output mva
    for(int i=0 ; i<AE_ACTOUT_RING_SIZE ; i++)
    {
        sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_output_ddr_mva[i] = aeBufferList->actoutAeOutput[i].getMva();
        LOG_DBG("fill-up actOut handle: aeOutputMva[%d](0x%x)", i, sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_output_ddr_mva[i]);
    }
    //fill-up ae algo data mva
    for(int i=0 ; i<AE_ACTOUT_RING_SIZE ; i++)
    {
        sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_algo_data_ddr_mva[i] = aeBufferList->actoutAeAlgoData[i].getMva();
        LOG_DBG("fill-up actOut handle: actoutAeAlgoData[%d](0x%x)", i, sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_algo_data_ddr_mva[i]);
    }
    //fill-up ae stat mva
    for(int i=0 ; i<AE_ACTOUT_RING_SIZE ; i++)
    {
        sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_stat_ddr_mva[i] = aeBufferList->actoutAeStat[i].getMva();
        LOG_DBG("fill-up actOut handle: actoutAeStat[%d](0x%x)", i, sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_stat_ddr_mva[i]);
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/
void CcuCtrlAe::getHdrDataBuffer(void **bufferAddr0, void **bufferAddr1)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe is not initialized yet, skip", __FUNCTION__);
        return;
    }

    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;
    *bufferAddr0 = (void *)aeBufferList->hdrDataBuffer[0].getVa();
    *bufferAddr1 = (void *)aeBufferList->hdrDataBuffer[1].getVa();
}

uint32_t CcuCtrlAe::getAeInfoForIsp(AE_INFO_T *a_rAEInfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return false;
    }

    LOG_DBG("RCDBG: m_pDrvCcu(%p), m_initialized(%d)", m_pDrvCcu, m_initialized);
    struct shared_buf_map * sharedBufMap = m_pDrvCcu->getSharedBufMap();
    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;
    uint32_t ridx = sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_algo_data_ridx;
    COMPAT_CCU_AeAlgo *aeAlgoDataPtr = NULL;
    AE_CORE_INIT *aeInitDataPtr = NULL;

    // if(sharedBufMap->is_ae_started[m_ccuInstanceIdx] == false)
    // {
    //     LOG_WRN("[%s] si(%d), ii(%d) AE not started, skip", __FUNCTION__, m_sensorIdx, m_ccuInstanceIdx);
    //     return false;
    // }

    if((ridx < 0) || (ridx >= AE_ACTOUT_RING_SIZE))
    {
        LOG_DBG_MUST("[%s] si(%d), ii(%d), ridx(%d) invalid, skip", __FUNCTION__, m_sensorIdx, m_ccuInstanceIdx, ridx);
        return false;
    }
    aeAlgoDataPtr = (COMPAT_CCU_AeAlgo *)aeBufferList->actoutAeAlgoData[ridx].getVa();
    aeInitDataPtr = (AE_CORE_INIT *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[m_ccuInstanceIdx].ae_init_data_addr);

    LOG_DBG("AE algo dataPtrs, aeAlgoDataPtr(%p, ridx=%d), aeInitDataPtr(%p)", aeAlgoDataPtr, ridx, aeInitDataPtr);

    if((aeAlgoDataPtr == NULL) || (aeInitDataPtr == NULL))
    {
        LOG_ERR("AE algo dataPtrs violation, skip fillUpAeInfo");
        return -1;
    }

    fillUpAeInfo(a_rAEInfo, aeAlgoDataPtr, aeInitDataPtr);
    UTIL_TRACE_END();

    LOG_DBG("%s, ReqNumber(%d)", __FUNCTION__, aeAlgoDataPtr->m_u4ReqNumberHead);
    return aeAlgoDataPtr->m_u4ReqNumberHead;
}

void CcuCtrlAe::fillUpAeInfo(AE_INFO_T *rAEISPInfo, COMPAT_CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData)
{
    rAEISPInfo->u4AETarget = aeAlgoData->m_u4AETarget;
    rAEISPInfo->u4AECurrentTarget = aeAlgoData->m_u4CWRecommend;
    rAEISPInfo->u4AECondition = aeAlgoData->m_u4AECondition;
    rAEISPInfo->eAEMeterMode = (LIB3A_AE_METERING_MODE_T)aeAlgoData->m_AECmdSet.eAEMeterMode;
    rAEISPInfo->u4Eposuretime = (MUINT64)aeAlgoData->m_CurAEOutput.EvSetting.u4Eposuretime * 1000; //multiply 1000 to convert us into ns

    LOG_DBG("[%s] u4Eposuretime(%d)->output_u4Eposuretime(%lld)",
        __FUNCTION__, aeAlgoData->m_CurAEOutput.EvSetting.u4Eposuretime, rAEISPInfo->u4Eposuretime);

    rAEISPInfo->u4AfeGain = aeAlgoData->m_CurAEOutput.EvSetting.u4AfeGain;
    rAEISPInfo->u4IspGain = aeAlgoData->m_i4IspGain; //aeAlgoData->m_CurAEOutput.EvSetting.u4IspGain; store DGN gain base
    rAEISPInfo->u4RealISOValue = aeAlgoData->m_CurAEOutput.u4ISO;
    rAEISPInfo->i4LightValue_x10 = aeAlgoData->m_i4BV + 50; //pAESetting->Bv + 50;
    rAEISPInfo->i4RealLightValue_x10 = aeAlgoData->m_i4RealBV + 50; //real LV
    rAEISPInfo->u4MaxISO = 800;
    rAEISPInfo->bAEStable = aeAlgoData->m_bAEStable;
    rAEISPInfo->bAETouchEnable = aeAlgoData->m_bAEMeteringEnable;
    rAEISPInfo->i4deltaIndex = aeAlgoData->m_i4DeltaIndex;
    rAEISPInfo->u4AEFinerEVIdxBase = aeAlgoData->m_u4AEFinerEVIdxBase;
    rAEISPInfo->i4EVRatio = aeAlgoData->m_LinearResp.i4EVRatio;
    rAEISPInfo->u4MgrCWValue = aeAlgoData->m_u4MgrCWValue;
    rAEISPInfo->u4AvgWValue = aeAlgoData->m_u4AvgWValue;
    rAEISPInfo->u4AEidxNext = aeAlgoData->m_u4Index;
    rAEISPInfo->u4AEidxNextF = aeAlgoData->m_u4IndexF;
    rAEISPInfo->bAEScenarioChange = (aeAlgoData->m_u4PlineChange > 0);
    rAEISPInfo->u4ReqNumber = aeAlgoData->m_u4ReqNumberHead;
    rAEISPInfo->i4AEComp = (aeAlgoData->m_AECmdSet.eAEComp > CCU_LIB3A_AE_EV_COMP_40) ? 
        ((aeAlgoData->m_AECmdSet.eAEComp - CCU_LIB3A_AE_EV_COMP_40) * -1) : (aeAlgoData->m_AECmdSet.eAEComp);
    memcpy(rAEISPInfo->u4Histogrm, aeAlgoData->m_pu4FullYHist, sizeof(MUINT32) * AE_SW_HISTOGRAM_BIN);

    rAEISPInfo->u4FaceAEStable = aeAlgoData->m_u4FaceAEStable;
    rAEISPInfo->i4Crnt_FDY = aeAlgoData->m_i4Crnt_FDY;
    rAEISPInfo->u4MeterFDTarget = aeAlgoData->m_u4CWRecommendAfterPureAE_Stable;
    rAEISPInfo->u4FaceFinalTarget = aeAlgoData->m_u4FaceFinalTarget;
    rAEISPInfo->u4FDProb = aeAlgoData->m_MeterFDSts.m_u4FaceSizeLocFinalProb;
    rAEISPInfo->u4FaceNum = aeAlgoData->m_MeterFDSts.m_u4FaceNum;
    memcpy(rAEISPInfo->i4FDY_ArrayOri, aeAlgoData->m_i4FDY_ArrayOri, sizeof(MINT32) * MAX_AE_METER_AREAS);
    memcpy(rAEISPInfo->i4FDY_Array, aeAlgoData->m_i4FDY_Array, sizeof(MINT32) * MAX_AE_METER_AREAS);
    memcpy(&(rAEISPInfo->LandMarkFDArea), &(aeAlgoData->m_LandMarkFDArea), sizeof(CCU_AEMeterArea_T));
    memcpy(&(rAEISPInfo->LandMarkFDSmoothArea), &(aeAlgoData->m_LandMarkFDSmoothArea), sizeof(CCU_AEMeterArea_T));
    memcpy(rAEISPInfo->FDArea, aeAlgoData->m_FDArea, sizeof(CCU_AEMeterArea_T) * MAX_AE_METER_AREAS);

    rAEISPInfo->bAETouchEnable = aeAlgoData->m_bAEMeteringEnable;
    rAEISPInfo->bFaceAELCELinkEnable = m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ContrastLinkParam.bEnableContrastLink;
    rAEISPInfo->u4MaxGain = aeAlgoData->m_u4MaxGain;
	rAEISPInfo->bIsMaxIndexStable = (aeAlgoData->m_u4Index == aeAlgoData->m_u4IndexMax);

    memcpy(rAEISPInfo->i4FaceBrightToneProtectValue_12bit, aeAlgoData->m_i4LTMFaceBrightToneValue_12bit, sizeof(MINT32) * MAX_AE_METER_AREAS);
    rAEISPInfo->bEnableFaceAE = aeAlgoData->m_MeterFDSts.bEnableFaceAE;
    rAEISPInfo->m_u4FaceState = aeAlgoData->m_MeterFDSts.m_u4FaceState;
    rAEISPInfo->m_u4FaceRobustCnt = aeAlgoData->m_MeterFDSts.m_u4FaceRobustCnt;
    rAEISPInfo->m_u4FaceRobustTrustCnt = aeAlgoData->m_MeterFDSts.m_u4FaceRobustTrustCnt;
    rAEISPInfo->m_u4FD_Lock_MaxCnt = aeAlgoData->m_MeterFDSts.m_u4FD_Lock_MaxCnt;
    rAEISPInfo->m_u4FDDropTempSmoothCnt = aeAlgoData->m_MeterFDSts.m_u4FDDropTempSmoothCnt;
    rAEISPInfo->m_u4OTFaceTrustCnt = aeAlgoData->m_MeterFDSts.m_u4OTFaceTrustCnt;
    rAEISPInfo->bOTFaceTimeOutLockAE = aeAlgoData->m_MeterFDSts.bOTFaceTimeOutLockAE;
    rAEISPInfo->u4MeterFDLinkTarget = aeAlgoData->m_u4CWRecommend_Stable;

    LOG_DBG("[%s] Exp:%lld / Afe:%d / Isp:%d / ISO:%d\n", __FUNCTION__, 
        rAEISPInfo->u4Eposuretime,rAEISPInfo->u4AfeGain, rAEISPInfo->u4IspGain, rAEISPInfo->u4RealISOValue);

    LOG_DBG("[%s] LCE fd link:%d/%d, fd target:%d/%d, fd en:%d/%d\n", __FUNCTION__, 
        aeAlgoData->m_u4CWRecommend_Stable, 
        rAEISPInfo->u4MeterFDLinkTarget, 
        aeAlgoData->m_u4CWRecommendAfterPureAE_Stable, 
        rAEISPInfo->u4MeterFDTarget, 
        aeAlgoData->m_MeterFDSts.bEnableFaceAE, 
        rAEISPInfo->bEnableFaceAE);

    /*
    rAEISPInfo->u4WeightingSum = aeAlgoData->m_u4WeightingSum;
    rAEISPInfo->i2FlareOffset = aeAlgoData->m_u4Prvflare;

    rAEISPInfo->u4AEidxCurrent = aeAlgoData->m_u4Index;
    rAEISPInfo->u4AEidxCurrentF = aeAlgoData->m_u4IndexF;
    //getAERealISOvalue(pAESetting);
        rAEISPInfo->u4OrgExposuretime = aeAlgoData->m_CurAEOutput.EvSetting.u4Eposuretime;
    rAEISPInfo->u4OrgRealISOValue = aeAlgoData->m_CurAEOutput.u4ISO;



    //updateHdrInfo(rAEISPInfo.rHdrAEInfo);
    //rAEISPInfo.i4HdrExpoDiffThr = m_pAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.rAutoTuning.i4RmmExpoDiffThr;
    // for shading smooth start
    rAEISPInfo->bEnableRAFastConverge = MTRUE;

    rAEISPInfo->TgCtrlRight = 0;

    rAEISPInfo->i4FrameCnt = aeAlgoData->m_i4FrameCnt;
    
    for(int i=0; i<AE_BLOCK_NO; i++)
    {
        for(int j=0;j<AE_BLOCK_NO;j++)
        {
            //rAEISPInfo->pu4AEBlock[i][j] = aeAlgoData->m_pu4AEBlock[i][j];
            //rAEISPInfo->pu4Weighting[i][j] = aeAlgoData->m_pWtTbl->W[i][j];
            rAEISPInfo->pu4AEBlock[i][j] = aeAlgoData->m_pu4AEBlock[i][j];
            rAEISPInfo->pu4Weighting[i][j] = 1;
        }
    }
    // for shading smooth end

    // for LCE smooth start

    if(m_CcuAeExifDataPtr.pAeNVRAM->rHistConfig.u4HistHighThres == 0)
    {
        rAEISPInfo->u4MeterFDTarget = aeAlgoData->m_u4MeterFDTarget;
    }
    else
    {
        //rAEISPInfo->u4MeterFDTarget = aeAlgoData->m_u4RealFaceCWR;
    }
    
    rAEISPInfo->bFaceAELCELinkEnable = m_CcuAeExifDataPtr.pAeNVRAM->rHistConfig.u4HistHighThres;
    rAEISPInfo->u4MaxGain = aeAlgoData->m_u4MaxGain;
    rAEISPInfo->u4FDProb = aeAlgoData->m_MeterFDSts.m_u4FaceSizeLocFinalProb;
    //rAEISPInfo->LandMarkFDArea = aeAlgoData->m_LandMarkFDArea;
    memcpy(&rAEISPInfo->LandMarkFDArea, &aeAlgoData->m_LandMarkFDArea, sizeof(CCU_AEMeterArea_T));
    rAEISPInfo->u4FaceNum = aeAlgoData->m_MeterFDSts.m_u4FaceNum;
    //rAEISPInfo->LandMarkFDSmoothArea = aeAlgoData->m_LandMarkFDSmoothArea;
    memcpy(&rAEISPInfo->LandMarkFDSmoothArea, &aeAlgoData->m_LandMarkFDSmoothArea, sizeof(CCU_AEMeterArea_T));
    for(int i=0;i<MAX_AE_METER_AREAS;i++)
    {
        rAEISPInfo->i4FDY_ArrayOri[i] = aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[i];
        rAEISPInfo->i4FDY_Array[i] = aeAlgoData->m_i4FDY_Array_CCUFrameDone[i];
        //rAEISPInfo->FDArea[i] = aeAlgoData->m_FDArea[i];
        memcpy(&rAEISPInfo->FDArea[i], &aeAlgoData->m_FDArea[i], sizeof(CCU_AEMeterArea_T));
    }
    rAEISPInfo->bAETouchEnable = aeAlgoData->m_bAEMeteringEnable;
    //rAEISPInfo->rHdrToneInfo = m_HdrAEInfo.rHdrToneInfo;
    // for LCE smooth end

    //============== [setAEOutput] moved from AE_Core_Main ===========================

    memcpy(&rAEISPInfo->EvSetting ,     &aeAlgoData->a_Output.EvSetting,    sizeof(CCU_strEvSetting));
//  memcpy(&rAEISPInfo->HdrEvSetting ,  &aeAlgoData->a_Output.HdrEvSetting, sizeof(CCU_strEvSetting)); to do
//  memcpy(&rAEISPInfo->HdrATRSetting , &aeAlgoData->a_Output.HdrATRSetting,sizeof(CCU_strEvSetting)); to do

    rAEISPInfo->Bv                              = aeAlgoData->a_Output.Bv;
    rAEISPInfo->AoeCompBv                       = aeAlgoData->a_Output.AoeCompBv;
    rAEISPInfo->i4EV                            = aeAlgoData->a_Output.i4EV;
    rAEISPInfo->u4ISO                           = aeAlgoData->a_Output.u4ISO;
    rAEISPInfo->i2FaceDiffIndex                 = aeAlgoData->a_Output.i2FaceDiffIndex;
    rAEISPInfo->i4Cycle0FullDeltaIdx            = aeAlgoData->a_Output.i4Cycle0FullDeltaIdx;
    rAEISPInfo->u4FracGain                      = aeAlgoData->a_Output.u4FracGain;
    rAEISPInfo->bAEStable                       = aeAlgoData->a_Output.bAEStable;
    rAEISPInfo->i4DeltaBV                       = aeAlgoData->a_Output.i4DeltaBV;
    rAEISPInfo->i4PreDeltaBV                    = aeAlgoData->a_Output.i4PreDeltaBV;
    rAEISPInfo->u2FrameRate                     = aeAlgoData->a_Output.u2FrameRate;
    rAEISPInfo->i2FlareGain                     = aeAlgoData->a_Output.i2FlareGain;
    rAEISPInfo->u4BayerY                        = aeAlgoData->a_Output.u4BayerY;
    rAEISPInfo->u4MonoY                         = aeAlgoData->a_Output.u4MonoY;
    rAEISPInfo->i4RealBVx1000                   = aeAlgoData->a_Output.i4RealBVx1000;
    rAEISPInfo->i4RealBV                        = aeAlgoData->a_Output.i4RealBV;
    rAEISPInfo->u4CWRecommendStable             = aeAlgoData->a_Output.u4CWRecommendStable;
    rAEISPInfo->u4FaceFailCnt                   = aeAlgoData->a_Output.u4FaceFailCnt;
    rAEISPInfo->u4FaceFoundCnt                  = aeAlgoData->a_Output.u4FaceFoundCnt;
    rAEISPInfo->u4FaceAEStable                  = aeAlgoData->a_Output.u4FaceAEStable;
    rAEISPInfo->i4gammaidx                      = aeAlgoData->a_Output.i4gammaidx;
    rAEISPInfo->i4cHdrSEDeltaEV                 = aeAlgoData->a_Output.i4cHdrSEDeltaEV;
    rAEISPInfo->i4AEidxNext                     = aeAlgoData->a_Output.i4AEidxNext;

    //=====================================================================
    LOG_DBG("u4AEidxCurrent: %d", aeAlgoData->m_u4Index);
    LOG_DBG("u4AEidxCurrentF: %d", aeAlgoData->m_u4IndexF);
    LOG_DBG("u4MgrCWValue: %d", aeAlgoData->m_u4MgrCWValue);
    LOG_DBG("u4AECurrentTarget: %d", aeAlgoData->m_u4CWRecommend);
    LOG_DBG("i4Crnt_FDY: %d", aeAlgoData->m_i4Crnt_FDY);
    LOG_DBG("ReqNumberHead: %d", aeAlgoData->ReqNumberHead);
    LOG_DBG("ReqNumberTail: %d", aeAlgoData->ReqNumberTail);
    //=============================================================================
    */
    return;
}

uint32_t CcuCtrlAe::getDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, AE_DEBUG_DATA_T *a_rAEDebugData)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return false;
    }

    struct shared_buf_map * sharedBufMap = m_pDrvCcu->getSharedBufMap();
    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;
    uint32_t ridx = sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_algo_data_ridx;
    COMPAT_CCU_AeAlgo *aeAlgoDataPtr = NULL;
    AE_CORE_INIT *aeInitDataPtr = NULL;

    // if(sharedBufMap->is_ae_started[m_ccuInstanceIdx] == false)
    // {
    //     LOG_WRN("[%s] si(%d), ii(%d) AE not started, skip", __FUNCTION__, m_sensorIdx, m_ccuInstanceIdx);
    //     return false;
    // }

    if((ridx < 0) || (ridx >= AE_ACTOUT_RING_SIZE))
    {
        LOG_DBG_MUST("[%s] si(%d), ii(%d), ridx(%d) invalid, skip", __FUNCTION__, m_sensorIdx, m_ccuInstanceIdx, ridx);
        return false;
    }

    aeAlgoDataPtr = (COMPAT_CCU_AeAlgo *)aeBufferList->actoutAeAlgoData[ridx].getVa();
    aeInitDataPtr = (AE_CORE_INIT *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[m_ccuInstanceIdx].ae_init_data_addr);

    LOG_DBG("AE algo dataPtrs, aeAlgoDataPtr(%p, ridx=%d), aeInitDataPtr(%p)", aeAlgoDataPtr, ridx, aeInitDataPtr);

    MUINT32 *dumpPtr = (MUINT32 *)aeAlgoDataPtr;

    if((aeAlgoDataPtr == NULL) || (aeInitDataPtr == NULL))
    {
        LOG_ERR("AE algo dataPtrs, skip fillUpAeDebugInfo");
        return -1;
    }

    fillUpAeDebugInfo(a_rAEDebugInfo, aeAlgoDataPtr, aeInitDataPtr);
    fillUpAeDebugData(a_rAEDebugData, aeAlgoDataPtr, aeInitDataPtr);
    UTIL_TRACE_END();

    LOG_DBG("%s, ReqNumber(%d)", __FUNCTION__, aeAlgoDataPtr->m_u4ReqNumberHead);
    return aeAlgoDataPtr->m_u4ReqNumberHead;
}

void CcuCtrlAe::fillUpAeDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, COMPAT_CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData)
{
    MINT32 i=0;
    MINT32 j=0;

    // AE Debug Info

    if (!a_rAEDebugInfo) 
    {
        LOG_WRN("[%s] Debug info buff=null, skip fillUpAeDebugInfo", __FUNCTION__);
        return;
    }
    memset(a_rAEDebugInfo, 0, sizeof(AE_DEBUG_INFO_T));

    //===========================================================================

#define SET_TAG(_tag, _val) setDebugTag(a_rAEDebugInfo, (_tag), (_val))

    // LOG_DBG("dbgeric aealgodata1 = %d\n", aeAlgoData->m_u4IndexF);
    // LOG_DBG("dbgeric aealgodata1 = %d\n", aeAlgoData->m_MeterFDSts.m_u4FaceNum);
    // LOG_DBG("dbgeric aeaInitdata1 = %d\n", m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Lock_MaxCnt);
    // LOG_DBG("dbgeric aeaInitdata2 = %d\n", m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.i4BVOffset);
    // LOG_DBG("dbgeric aeaVsyncdata1 = %d\n", aeVsyncInfo->m_AEWinBlock.u4XHi);
    // LOG_DBG("dbgeric AE_DEBUG_TAG_SUBVERSION = %d\n", AE_DEBUG_TAG_SUBVERSION);
    // LOG_DBG("dbgeric AE_DEBUG_TAG_VERSION = %d\n", AE_DEBUG_TAG_VERSION);

    // AE tag version
    SET_TAG(AE_TAG_DEBUG_VERSION, (((MUINT32)AE_DEBUG_TAG_SUBVERSION << 16) | AE_DEBUG_TAG_VERSION));
    SET_TAG(AE_TAG_AE_MODE, aeAlgoData->m_AECmdSet.eAEScene);
    // Output AE brightness information
    SET_TAG(AE_TAG_PRV_INDEX, aeAlgoData->m_u4Index);
    SET_TAG(AE_TAG_PRV_INDEXF,aeAlgoData->m_u4IndexF);

    SET_TAG(AE_TAG_PRV_SHUTTER_TIME, aeAlgoData->m_CurAEOutput.EvSetting.u4Eposuretime);
    SET_TAG(AE_TAG_PRV_SENSOR_GAIN, aeAlgoData->m_CurAEOutput.EvSetting.u4AfeGain);
    SET_TAG(AE_TAG_PRV_ISP_GAIN, aeAlgoData->m_CurAEOutput.EvSetting.u4IspGain);
    SET_TAG(AE_TAG_PRV_ISO_REAL, aeAlgoData->m_CurAEOutput.u4ISO);
    SET_TAG(AE_TAG_PRV_FRAME_RATE, aeAlgoData->m_CurAEOutput.u2FrameRate);
    SET_TAG(AE_TAG_FRAC_GAIN, aeAlgoData->m_u4FracGain);
    SET_TAG(AE_TAG_MAX_FPS, aeAlgoData->m_AECmdSet.i4AEMaxFps);
    SET_TAG(AE_TAG_MIN_FPS, aeAlgoData->m_AECmdSet.i4AEMinFps);
    SET_TAG(AE_TAG_STABLE, aeAlgoData->m_bAEStable);

    SET_TAG(AE_TAG_NVRAM_IDX, aeAlgoData->m_u4NvramIdx);
    SET_TAG(AE_TAG_ZOOM_XOFFSET, aeAlgoData->m_ZoomWinInfo.u4XOffset);
    SET_TAG(AE_TAG_ZOOM_XWIDTH, aeAlgoData->m_ZoomWinInfo.u4XWidth);
    SET_TAG(AE_TAG_ZOOM_YOFFSET, aeAlgoData->m_ZoomWinInfo.u4YOffset);
    SET_TAG(AE_TAG_ZOOM_YHEIGHT, aeAlgoData->m_ZoomWinInfo.u4YHeight);

    //Pline Info
    SET_TAG(AE_TAG_PRV_INDEXBASE, aeAlgoData->m_u4AEFinerEVIdxBase);
    SET_TAG(AE_TAG_MIN_INDEX, aeInitData->m_u4IndexMin);
    SET_TAG(AE_TAG_MAX_INDEX, aeInitData->m_u4IndexMax);
    SET_TAG(AE_TAG_MIN_INDEXF, aeAlgoData->m_u4IndexFMin);
    SET_TAG(AE_TAG_MAX_INDEXF, aeAlgoData->m_u4IndexFMax);
    SET_TAG(AE_TAG_PRV_PLINE_MAX_BV, aeInitData->i4MaxBV);
    SET_TAG(AE_TAG_EV_DIFF_FRAC, aeAlgoData->m_i4EvDiffFrac);
    
    SET_TAG(AE_TAG_PRV_DELTA_INDEX, aeAlgoData->m_i4DeltaIndex);
    SET_TAG(AE_TAG_PRV_EV_RATIO,aeAlgoData->m_LinearResp.i4EVRatio);
    SET_TAG(AE_TAG_PERFRAME_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bPerFrameAESmooth);
    
    // AE setting
    SET_TAG(AE_TAG_REALISO_EN, aeAlgoData->m_AECmdSet.bIsoSpeedReal);
    SET_TAG(AE_TAG_REALBV, aeAlgoData->m_i4RealBV);
    SET_TAG(AE_TAG_REALBVX1000, aeAlgoData->m_i4RealBVx1000);
    SET_TAG(AE_TAG_COMPBV, aeAlgoData->m_i4AoeCompBV);
    SET_TAG(AE_TAG_BV_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.i4BVOffset);

    SET_TAG(AE_TAG_AE_SCENE,aeAlgoData->m_AECmdSet.eAEScene);
    SET_TAG(AE_TAG_EV_COM, aeAlgoData->m_AECmdSet.eAEComp);
    SET_TAG(AE_TAG_FLICKER, aeAlgoData->m_AECmdSet.eAEFlickerMode);
    SET_TAG(AE_TAG_FLICKER_AUTO, aeAlgoData->m_AECmdSet.eAEAutoFlickerMode);
    SET_TAG(AE_TAG_AEMETER, aeAlgoData->m_AECmdSet.eAEMeterMode);
    SET_TAG(AE_TAG_ISO_SETTING, aeAlgoData->m_AECmdSet.u4AEISOSpeed);

    // device info
    SET_TAG(AE_TAG_MINI_GAIN, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4MinGain);
    SET_TAG(AE_TAG_MAX_GAIN, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4MaxGain);
    SET_TAG(AE_TAG_1XGAIN_ISO, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4MiniISOGain);
    SET_TAG(AE_TAG_PRV_EXP_UINT, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4PreExpUnit);
    SET_TAG(AE_TAG_VIDEO_EXP_UINT, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4VideoExpUnit);
    SET_TAG(AE_TAG_VIDEO_PRV_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4Video2PreRatio);
    SET_TAG(AE_TAG_CAP_EXP_UINT, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4CapExpUnit);
    SET_TAG(AE_TAG_CAP_PRV_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4Cap2PreRatio);

    // AE algorithm enable and condition check
    SET_TAG(AE_TAG_PRV_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4PreviewFlareOffset);
    SET_TAG(AE_TAG_CAP_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4CaptureFlareOffset);
    SET_TAG(AE_TAG_CAP_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4CaptureFlareThres);
    SET_TAG(AE_TAG_VIDEO_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoFlareOffset);
    SET_TAG(AE_TAG_VIDEO_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoFlareThres);
    SET_TAG(AE_TAG_STROBE_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4StrobeFlareOffset);
    SET_TAG(AE_TAG_STROBE_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4StrobeFlareThres);
    SET_TAG(AE_TAG_CAP_MAX_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4PrvMaxFlareThres);
    SET_TAG(AE_TAG_CAP_MIN_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4PrvMinFlareThres);
    SET_TAG(AE_TAG_VIDEO_MAX_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoMaxFlareThres);
    SET_TAG(AE_TAG_VIDEO_MIN_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoMinFlareThres);
    SET_TAG(AE_TAG_FLARE_STD_THR_HIGH, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareStdThrHigh);
    SET_TAG(AE_TAG_FLARE_STD_THR_LOW, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareStdThrLow);
    SET_TAG(AE_TAG_PRV_CAP_FLARE_DIFF, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4PrvCapFlareDiff);
    SET_TAG(AE_TAG_FLARE_MAX_STEP_GAP_FAST, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareMaxStepGap_Fast);
    SET_TAG(AE_TAG_FLARE_MAX_STEP_GAP_SLOW, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareMaxStepGap_Slow);
    SET_TAG(AE_TAG_FLARE_MAX_STEP_GAP_LIMIT_BV, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlarMaxStepGapLimitBV);
    SET_TAG(AE_TAG_FLARE_AE_STABLE_COUNT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareAEStableCount);

    // HS
    SET_TAG(AE_TAG_HISTSTRETCH_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_HIST_STRETCH)?1:0);
    SET_TAG(AE_TAG_HISTSTRETCH_WEIGHTING, aeAlgoData->m_TargetSts.Wet_HS);
    SET_TAG(AE_TAG_HISTSTRETCH_METERING, aeAlgoData->m_u4HistoStretchCWM );
    SET_TAG(AE_TAG_HS_NV_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.bEnableHistStretch);          //bEnableHistStretch
    SET_TAG(AE_TAG_HS_NV_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4HistStretchWeight);      //u4HistStretchWeight
    SET_TAG(AE_TAG_HS_NV_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4Pcent);      //u4Pcent
    SET_TAG(AE_TAG_HS_NV_THD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4Thd);      //u4Thd;                       // 0~255
    SET_TAG(AE_TAG_HS_NV_FLATTHD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatThd);      //u4FlatThd;                   // 0~255
    SET_TAG(AE_TAG_HS_NV_BRIGHTPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatBrightPcent);      //u4FlatBrightPcent;
    SET_TAG(AE_TAG_HS_NV_DARKPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatDarkPcent);      // u4FlatDarkPcent;
    SET_TAG(AE_TAG_HS_NV_FLATX1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4X1);       //sFlatRatio
    SET_TAG(AE_TAG_HS_NV_FLATY1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4Y1);
    SET_TAG(AE_TAG_HS_NV_FLATX2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4X2);
    SET_TAG(AE_TAG_HS_NV_FLATY2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4Y2);
    SET_TAG(AE_TAG_HS_EN_FLATSKY, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.bEnableFlatSkyEnhance);
    SET_TAG(AE_TAG_HS_FLATSKYEVD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4FlatSkyEVD);
    SET_TAG(AE_TAG_HS_FLATSKYTHD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4FlatSkyTHD);
    SET_TAG(AE_TAG_HS_BVRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_X[0]);
    SET_TAG(AE_TAG_HS_BVRATIO_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_Y[0]);
    SET_TAG(AE_TAG_HS_BVRATIO_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_X[1]);
    SET_TAG(AE_TAG_HS_BVRATIO_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_Y[1]);
    SET_TAG(AE_TAG_HS_SCENE_SUM_R, aeAlgoData->m_TargetSts.m_u4SceneSumR);
    SET_TAG(AE_TAG_HS_SCENE_SUM_G, aeAlgoData->m_TargetSts.m_u4SceneSumG);
    SET_TAG(AE_TAG_HS_SCENE_SUM_B, aeAlgoData->m_TargetSts.m_u4SceneSumB);

    SET_TAG(AE_TAG_HS_STS_Y, aeAlgoData->m_TargetSts.Hist_HS_LimitY);
    SET_TAG(AE_TAG_HS_STS_FLATHIGHY, aeAlgoData->m_TargetSts.HS_FlatHighY);
    SET_TAG(AE_TAG_HS_STS_FLATLOWY, aeAlgoData->m_TargetSts.HS_FlatLowY);
    SET_TAG(AE_TAG_HS_STS_FLATPROB, aeAlgoData->m_TargetSts.Prob_HS_FLAT);
    SET_TAG(AE_TAG_HS_STS_EVD, aeAlgoData->m_TargetSts.Orig_HS_FLAT);
    SET_TAG(AE_TAG_HS_STS_THD, aeAlgoData->m_TargetSts.Hist_HS_LimitTHD);
    SET_TAG(AE_TAG_HS_STS_TARGET, aeAlgoData->m_TargetSts.Hist_HS_Target);
    SET_TAG(AE_TAG_HS_STS_LIMITBY, aeAlgoData->m_TargetSts.HS_LIMITED_BY);
    SET_TAG(AE_TAG_HS_STS_COLORDIST, aeAlgoData->m_TargetSts.Orig_HS_COLOR);
    SET_TAG(AE_TAG_HS_STS_COLORPROB, aeAlgoData->m_TargetSts.Prob_HS_COLOR);
    SET_TAG(AE_TAG_HS_STS_FLATSKYDET, aeAlgoData->m_bIsBlueSkyDet);

    // AOE
    SET_TAG(AE_TAG_ANTIOVER_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_OVEREXPOSURE)?1:0);
    SET_TAG(AE_TAG_ANTIOVER_METERING, aeAlgoData->m_u4AntiOverExpCWM);

    SET_TAG(AE_TAG_AOE_NV_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.bEnableAntiOverExposure);  //bEnableAntiOverExposure;
    SET_TAG(AE_TAG_AOE_NV_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4AntiOverExpWeight);  //u4AntiOverExpWeight;
    SET_TAG(AE_TAG_AOE_NV_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4Pcent);  //u4Pcent;
    SET_TAG(AE_TAG_AOE_NV_THD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4Thd);  //u4Thd;
    SET_TAG(AE_TAG_AOE_NV_COEPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4COEPcent);
    SET_TAG(AE_TAG_AOE_NV_COETHD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4COEThd);
    SET_TAG(AE_TAG_AOE_NV_COEP_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.bEnableCOEP);
    SET_TAG(AE_TAG_AOE_NV_COEPY_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4X1);
    SET_TAG(AE_TAG_AOE_NV_COEPY_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4Y1);
    SET_TAG(AE_TAG_AOE_NV_COEPY_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4X2);
    SET_TAG(AE_TAG_AOE_NV_COEPY_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4Y2);
    SET_TAG(AE_TAG_AOE_NV_COEPDIFF_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4X1);
    SET_TAG(AE_TAG_AOE_NV_COEPDIFF_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4Y1);
    SET_TAG(AE_TAG_AOE_NV_COEPDIFF_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4X2);
    SET_TAG(AE_TAG_AOE_NV_COEPDIFF_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4Y2);
    SET_TAG(AE_TAG_AOE_EN_BV, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.bEnable);
    SET_TAG(AE_TAG_AOE_BV_THD_X0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.i4BV[0]);
    SET_TAG(AE_TAG_AOE_BV_THD_Y0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.u4THD[0]);
    SET_TAG(AE_TAG_AOE_BV_THD_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.i4BV[1]);
    SET_TAG(AE_TAG_AOE_BV_THD_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.u4THD[1]);
    SET_TAG(AE_TAG_AOE_BV_THD_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.i4BV[2]);
    SET_TAG(AE_TAG_AOE_BV_THD_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.u4THD[2]);
    SET_TAG(AE_TAG_AOE_EN_OERATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.bEnableOverExpoAOE);
    SET_TAG(AE_TAG_AOE_OE_LEVEL, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.u4OElevel);
    SET_TAG(AE_TAG_AOE_OE_LOWBND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.u4OERatio_LowBnd);
    SET_TAG(AE_TAG_AOE_OE_HIGHBND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.u4OERatio_HighBnd);
    SET_TAG(AE_TAG_AOE_OE_BVRATIO_X0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.i4OE_BVRatio_X[0]);
    SET_TAG(AE_TAG_AOE_OE_BVRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.i4OE_BVRatio_X[1]);

    SET_TAG(AE_TAG_AOE_STS_Y, aeAlgoData->m_TargetSts.AOE_Y);
    SET_TAG(AE_TAG_AOE_STS_WEIGHT, aeAlgoData->m_TargetSts.Wet_AOE);
    SET_TAG(AE_TAG_AOE_STS_THD, aeAlgoData->m_TargetSts.Hist_AOE_THD);
    SET_TAG(AE_TAG_AOE_STS_OERATIO, aeAlgoData->m_TargetSts.OE_Ratio);
    SET_TAG(AE_TAG_AOE_STS_OE_BV_P, aeAlgoData->m_TargetSts.Prob_OE_P);
    SET_TAG(AE_TAG_AOE_STS_OE_RATIO_P, aeAlgoData->m_TargetSts.Prob_OE_Ratio_P);    
    SET_TAG(AE_TAG_AOE_STS_OE_P, aeAlgoData->m_TargetSts.Final_Prob_OE_P);
    SET_TAG(AE_TAG_AOE_STS_OE_DELTAY, aeAlgoData->m_TargetSts.OE_DeltaY);
    SET_TAG(AE_TAG_AOE_STS_TARGET, aeAlgoData->m_TargetSts.Hist_AOE_Target);
    SET_TAG(AE_TAG_AOE_STS_COEP_P, aeAlgoData->m_TargetSts.Prob_COE_P);
    SET_TAG(AE_TAG_AOE_STS_COEP_ORIGY, aeAlgoData->m_TargetSts.Orig_COE_OuterY);
    SET_TAG(AE_TAG_AOE_STS_COEP_PROBY, aeAlgoData->m_TargetSts.Prob_COE_OuterY);
    SET_TAG(AE_TAG_AOE_STS_COEP_ORIGDIFF, aeAlgoData->m_TargetSts.Orig_COE_Diff);
    SET_TAG(AE_TAG_AOE_STS_COEP_PROBDIFF, aeAlgoData->m_TargetSts.Prob_COE_Diff);
    SET_TAG(AE_TAG_AOE_STS_COE_TARGET, aeAlgoData->m_TargetSts.AOE_COE_Target);
    SET_TAG(AE_TAG_AOE_STS_FINAL_TARGET, aeAlgoData->m_TargetSts.Target_AOE);
    SET_TAG(AE_TAG_AOE_STS_LIMITBY, aeAlgoData->m_TargetSts.AOE_LIMITED_BY);

    // ABL
    SET_TAG(AE_TAG_BACKLIGH_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_BACKLIGHT)?1:0);
    SET_TAG(AE_TAG_BACKLIGH_WEIGHTING, aeAlgoData->m_TargetSts.Wet_ABL);
    SET_TAG(AE_TAG_BACKLIGH_METERING, aeAlgoData->m_u4BacklightCWM);
    
    SET_TAG(AE_TAG_ABL_NV_ENABLE,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.bEnableBlackLight);      //bEnableBlackLight;
    SET_TAG(AE_TAG_ABL_NV_WEIGHT,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4BackLightWeight);      //u4BackLightWeight;
    SET_TAG(AE_TAG_ABL_NV_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4Pcent);      //u4Pcent;
    SET_TAG(AE_TAG_ABL_NV_THD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4Thd);      //u4Thd;
    SET_TAG(AE_TAG_ABL_NV_CENTERBND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4CenterHighBnd);   //u4CenterHighBnd;
    SET_TAG(AE_TAG_ABL_NV_TARGETSTRENGTH, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4TargetStrength);//u4TargetStrength;
    SET_TAG(AE_TAG_ABL_NV_FGBGRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4X1);//sFgBgEVRatio;
    SET_TAG(AE_TAG_ABL_NV_FGBGRATIO_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4Y1);
    SET_TAG(AE_TAG_ABL_NV_FGBGRATIO_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4X2);
    SET_TAG(AE_TAG_ABL_NV_FGBGRATIO_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4Y2);
    SET_TAG(AE_TAG_ABL_NV_BVRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4X1);//sBVRatio;
    SET_TAG(AE_TAG_ABL_NV_BVRATIO_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4Y1);
    SET_TAG(AE_TAG_ABL_NV_BVRATIO_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4X2);
    SET_TAG(AE_TAG_ABL_NV_BVRATIO_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4Y2);
    SET_TAG(AE_TAG_ABL_EN_ABS_DIFF, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.bEnableABLabsDiff);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_X[0]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_Y[0]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_X[1]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_Y[1]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_BVRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_X[0]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_BVRATIO_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_Y[0]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_BVRATIO_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_X[1]);
    SET_TAG(AE_TAG_ABL_ABS_DIFF_BVRATIO_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_Y[1]);

    SET_TAG(AE_TAG_ABL_STS_Y, aeAlgoData->m_TargetSts.ABL_Y);
    SET_TAG(AE_TAG_ABL_STS_TARGET, aeAlgoData->m_TargetSts.Hist_ABL_Target);
    SET_TAG(AE_TAG_ABL_STS_FGBGPROB, aeAlgoData->m_TargetSts.Prob_ABL_DIFF);
    SET_TAG(AE_TAG_ABL_STS_FGBGORIG, aeAlgoData->m_TargetSts.Orig_ABL_DIFF);
    SET_TAG(AE_TAG_ABL_STS_BVPROB, aeAlgoData->m_TargetSts.Prob_ABL_BV);
    SET_TAG(AE_TAG_ABL_STS_BVORIG, aeAlgoData->m_TargetSts.Orig_ABL_BV);
    SET_TAG(AE_TAG_ABL_STS_FINALPROB, aeAlgoData->m_TargetSts.Prob_ABL_P);
    SET_TAG(AE_TAG_ABL_STS_LIMITBY, aeAlgoData->m_TargetSts.ABL_LIMITED_BY);

    // NS
    SET_TAG(AE_TAG_NS_NV_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.bEnableNightScene);
    SET_TAG(AE_TAG_NS_NV_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4Pcent);//u4Pcent;
    SET_TAG(AE_TAG_NS_NV_THD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4Thd);//u4Thd;
    SET_TAG(AE_TAG_NS_NV_FLATTHD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4FlatThd);//u4FlatThd;
    SET_TAG(AE_TAG_NS_NV_BRIGHTTONE_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4BrightTonePcent);
    SET_TAG(AE_TAG_NS_NV_BRIGHTTONE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4BrightToneThd);
    SET_TAG(AE_TAG_NS_NV_LOWBNDPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4LowBndPcent);//u4LowBndPcent;
    SET_TAG(AE_TAG_NS_NV_LOWBNDTHD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4LowBndThd);//u4LowBndThd;
    SET_TAG(AE_TAG_NS_NV_LOWBNDTHDLIMIT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4LowBndThdLimit);//u4LowBndThdLimit;
    SET_TAG(AE_TAG_NS_NV_BRIGHTPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4FlatBrightPcent);//u4FlatBrightPcent;
    SET_TAG(AE_TAG_NS_NV_DARKPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4FlatDarkPcent);//u4FlatDarkPcent;
    SET_TAG(AE_TAG_NS_NV_FALT_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4X1);//sFlatRatio;
    SET_TAG(AE_TAG_NS_NV_FALT_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4Y1);
    SET_TAG(AE_TAG_NS_NV_FALT_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4X2);
    SET_TAG(AE_TAG_NS_NV_FALT_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4Y2);
    SET_TAG(AE_TAG_NS_NV_BV_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4X1);//sBVRatio;
    SET_TAG(AE_TAG_NS_NV_BV_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4Y1);
    SET_TAG(AE_TAG_NS_NV_BV_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4X2);
    SET_TAG(AE_TAG_NS_NV_BV_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4Y2);
    SET_TAG(AE_TAG_NS_NV_SKYENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.bEnableNightSkySuppresion);
    SET_TAG(AE_TAG_NS_NV_SKYBV_X1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4X1);//sBVRatio;
    SET_TAG(AE_TAG_NS_NV_SKYBV_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4Y1);
    SET_TAG(AE_TAG_NS_NV_SKYBV_X2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4X2);
    SET_TAG(AE_TAG_NS_NV_SKYBV_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4Y2);

    SET_TAG(AE_TAG_NS_STS_Y, aeAlgoData->m_TargetSts.NS_Y);
    SET_TAG(AE_TAG_NS_STS_WEIGHT, aeAlgoData->m_TargetSts.Prob_NS);
    SET_TAG(AE_TAG_NS_STS_HIGHBNDY, aeAlgoData->m_TargetSts.NS_BrightTone_Y);
    SET_TAG(AE_TAG_NS_STS_LOWBNDY, aeAlgoData->m_TargetSts.NS_BT_Y);
    SET_TAG(AE_TAG_NS_STS_FLATHIGHY, aeAlgoData->m_TargetSts.NS_FlatHighY);
    SET_TAG(AE_TAG_NS_STS_FLATLOWY, aeAlgoData->m_TargetSts.NS_FlatLowY);
    SET_TAG(AE_TAG_NS_STS_FLATPROB, aeAlgoData->m_TargetSts.Prob_NS_FLAT);
    SET_TAG(AE_TAG_NS_STS_EVD, aeAlgoData->m_TargetSts.Orig_NS_FLAT);
    SET_TAG(AE_TAG_NS_STS_THD, aeAlgoData->m_TargetSts.NS_OE_THD);
    SET_TAG(AE_TAG_NS_STS_TARGET, aeAlgoData->m_TargetSts.NS_Target);
    SET_TAG(AE_TAG_NS_STS_LOWBND_Y, aeAlgoData->m_TargetSts.NS_BT_Y);
    SET_TAG(AE_TAG_NS_STS_LOWBNDTARGET, aeAlgoData->m_TargetSts.NS_BT_Target);
    SET_TAG(AE_TAG_NS_STS_BRIGHTTONE_Y, aeAlgoData->m_TargetSts.NS_BrightTone_Y);
    SET_TAG(AE_TAG_NS_STS_BRIGHTTONE_THD, aeAlgoData->m_TargetSts.NS_BrightTone_THD);
    SET_TAG(AE_TAG_NS_STS_BRIGHTTONE_TARGET, aeAlgoData->m_TargetSts.NS_BrightTone_Target);
    SET_TAG(AE_TAG_NS_STS_BVORIG, aeAlgoData->m_TargetSts.Orig_NS_BV);
    SET_TAG(AE_TAG_NS_STS_BVPROB, aeAlgoData->m_TargetSts.Prob_NS_BV);
    SET_TAG(AE_TAG_NS_STS_CDF, aeAlgoData->m_TargetSts.NS_CDF_BV);
    SET_TAG(AE_TAG_NS_STS_CDFPROB, aeAlgoData->m_TargetSts.Prob_NS_CDF);
    SET_TAG(AE_TAG_NS_STS_LIMITBY, aeAlgoData->m_TargetSts.NS_LIMITED_BY);

    // v4p0
    SET_TAG(AE_TAG_METERV4P0_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bAEv4p0MeterEnable);
    // Main target
    SET_TAG(AE_TAG_MAINTARGET_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_MAINTARGET)?1:0);

    SET_TAG(AE_TAG_MT_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableAEMainTarget);
    SET_TAG(AE_TAG_MT_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.u4MainTargetWeight);
    SET_TAG(AE_TAG_MT_TARGETBV_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4X1);
    SET_TAG(AE_TAG_MT_TARGETBV_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4Y1);
    SET_TAG(AE_TAG_MT_TARGETBV_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4X2);
    SET_TAG(AE_TAG_MT_TARGETBV_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4Y2);
    SET_TAG(AE_TAG_MT_COLORWT_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableColorWTRatio);
    SET_TAG(AE_TAG_MT_COLORWT_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4X1);
    SET_TAG(AE_TAG_MT_COLORWT_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4Y1);
    SET_TAG(AE_TAG_MT_COLORWT_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4X2);
    SET_TAG(AE_TAG_MT_COLORWT_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4Y2);
    
    SET_TAG(AE_TAG_MT_STS_Y, aeAlgoData->m_u4GauCWValue);
    SET_TAG(AE_TAG_MT_STS_TARGET, aeAlgoData->m_TargetSts.Target_Main);
    SET_TAG(AE_TAG_MT_STS_PRETARGET, aeAlgoData->m_TargetSts.Target_PrevMain);
    SET_TAG(AE_TAG_MT_STS_WEIGHT, aeAlgoData->m_TargetSts.Wet_Main);

    // HSv4p0
    SET_TAG(AE_TAG_HSV4P0_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_HS_V4P0)?1:0);
    
    SET_TAG(AE_TAG_HSV4P0_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.bEnableHSv4p0);
    SET_TAG(AE_TAG_HSV4P0_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HSv4p0Weight);
    SET_TAG(AE_TAG_HSV4P0_BVRATIO_0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[0]);
    SET_TAG(AE_TAG_HSV4P0_BVRATIO_1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[1]);
    SET_TAG(AE_TAG_HSV4P0_BVRATIO_2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[2]);
    SET_TAG(AE_TAG_HSV4P0_BVRATIO_3, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[3]);
    SET_TAG(AE_TAG_HSV4P0_PCNT_0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[0]);
    SET_TAG(AE_TAG_HSV4P0_PCNT_1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[1]);
    SET_TAG(AE_TAG_HSV4P0_PCNT_2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[2]);
    SET_TAG(AE_TAG_HSV4P0_PCNT_3, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[3]);
    SET_TAG(AE_TAG_HSV4P0_EVD_0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[0]);
    SET_TAG(AE_TAG_HSV4P0_EVD_1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[1]);
    SET_TAG(AE_TAG_HSV4P0_EVD_2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[2]);
    SET_TAG(AE_TAG_HSV4P0_EVD_3, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[3]);
    SET_TAG(AE_TAG_HSV4P0_EVD_4, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[4]);
    for(i=0;i<4;i++)
    {
        for(j=0;j<5;j++)
        {
            SET_TAG(AE_TAG_HSV4P0_BV0THD_0+i*5+j, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HSTHDRatioTbl[i][j]);
        }
    }
    SET_TAG(AE_TAG_HSV4P0_DYNWT_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.bEnableDynWTRatio);
    SET_TAG(AE_TAG_HSV4P0_DYNWT_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4X1);
    SET_TAG(AE_TAG_HSV4P0_DYNWT_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4Y1);
    SET_TAG(AE_TAG_HSV4P0_DYNWT_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4X2);
    SET_TAG(AE_TAG_HSV4P0_DYNWT_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4Y2);
    
    SET_TAG(AE_TAG_HSV4P0_STS_Y, aeAlgoData->m_TargetSts.Hist_HS_LimitY);
    SET_TAG(AE_TAG_HSV4P0_STS_EVD, aeAlgoData->m_TargetSts.Orig_HS_FLAT);
    SET_TAG(AE_TAG_HSV4P0_STS_THD, aeAlgoData->m_TargetSts.Hist_HS_LimitTHD);
    SET_TAG(AE_TAG_HSV4P0_STS_WEIGHT, aeAlgoData->m_TargetSts.Wet_HSv4p0);
    SET_TAG(AE_TAG_HSV4P0_STS_TARGET, aeAlgoData->m_TargetSts.Target_HSv4p0);
    SET_TAG(AE_TAG_HSV4P0_STS_PRETARGET, aeAlgoData->m_TargetSts.Target_PrevHSv4p0);

    // Custom tuning setting
    SET_TAG(AE_TAG_EN_ROTATE_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableRotateWeighting);
    SET_TAG(AE_TAG_METERING_STABLE_MAX, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMax);
    SET_TAG(AE_TAG_METERING_STABLE_MIN, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMin);
    SET_TAG(AE_TAG_X_BLOCK_NO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.uBlockNumX);
    SET_TAG(AE_TAG_Y_BLOCK_NO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.uBlockNumY);
    SET_TAG(AE_TAG_FACE_HIGH_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.u4FdCwrHighBound);
    SET_TAG(AE_TAG_FACE_LOW_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.u4FdCwrLowBound);
    SET_TAG(AE_TAG_METERING_HIGH_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uMeteringYHighBound);
    SET_TAG(AE_TAG_METERING_LOW_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uMeteringYLowBound);

    // AE algorithm control setting
    SET_TAG(AE_TAG_CWV, aeAlgoData->m_u4CWValue);
    SET_TAG(AE_TAG_SE_CWV, aeAlgoData->m_u4CWSEValue);
    SET_TAG(AE_TAG_AVG, aeAlgoData->m_u4AvgWValue);
    SET_TAG(AE_TAG_SE_AVG, aeAlgoData->m_u4AvgWSEValue);
    SET_TAG(AE_TAG_CENTRAL_Y, aeAlgoData->m_u4CentralY);
    SET_TAG(AE_TAG_CWV_FINAL_TARGET, aeAlgoData->m_u4CWRecommend);
    SET_TAG(AE_TAG_AE_TARGET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4AETarget);
    
    SET_TAG(AE_TAG_MIN_CWV_RECMD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MinCWRecommend);
    SET_TAG(AE_TAG_MAX_CWV_RECMD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MaxCWRecommend);

    // FACE
    SET_TAG(AE_TAG_EN_FACE_AE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableFaceAE);
    SET_TAG(AE_TAG_METERFACE_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_FACEAE)?1:0);
    SET_TAG(AE_TAG_FACE_Y, aeAlgoData->m_MeterFDSts.m_u4FDY);    // for face AE value
    SET_TAG(AE_TAG_FACE_WEIGHTING, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.uFaceCentralWeight);
    SET_TAG(AE_TAG_FACE_EN_LOCSIZE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.bFaceLocSizeCheck);
    SET_TAG(AE_TAG_FACE_LOST_MAXCNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Lost_MaxCnt);
    SET_TAG(AE_TAG_FACE_TMPFDY_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4TmpFDY_LBound);
    SET_TAG(AE_TAG_FACE_TMPFDY_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4TmpFDY_HBound);
     
    SET_TAG(AE_TAG_FACE_Y_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.uFaceYLowBound);
    SET_TAG(AE_TAG_FACE_Y_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.uFaceYHighBound);
    SET_TAG(AE_TAG_FACE_BVRATIO_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.i4FaceBVLowBound);
    SET_TAG(AE_TAG_FACE_BVRATIO_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FdTarget.i4FaceBVHighBound);

    SET_TAG(AE_TAG_FACE_PROB, aeAlgoData->m_MeterFDSts.m_u4FDProb);
    SET_TAG(AE_TAG_FACE_STS_LOC, aeAlgoData->m_MeterFDSts.m_u4FaceProbLoc);
    SET_TAG(AE_TAG_FACE_STS_SIZE, aeAlgoData->m_MeterFDSts.m_u4FaceProbSize);
    SET_TAG(AE_TAG_FACE_STS_FINDCNT, aeAlgoData->m_MeterFDSts.u4FaceFoundCnt);
    SET_TAG(AE_TAG_FACE_STS_LOSTCNT, aeAlgoData->m_MeterFDSts.u4FaceFailCnt);

    SET_TAG(AE_TAG_FACE_20_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableFaceFastConverge);
    SET_TAG(AE_TAG_FACE_20_FACE_Y, aeAlgoData->m_MeterFDSts.m_u4FDY);
    SET_TAG(AE_TAG_FACE_20_CWV, aeAlgoData->m_u4CWValue);
    SET_TAG(AE_TAG_FACE_20_REAL_BV_X1000,aeAlgoData->m_i4RealBVx1000);    

    SET_TAG(AE_TAG_FACE_20_METER_FD_TARGET, aeAlgoData->m_u4MeterFDTarget );
    SET_TAG(AE_TAG_FACE_20_NORMAL_TARGET, aeAlgoData->m_u4NormalTarget );
    SET_TAG(AE_TAG_FACE_20_FINAL_TARGET, aeAlgoData->m_u4FaceFinalTarget );
    SET_TAG(AE_TAG_FACE_20_LCE_LINK, aeAlgoData->m_bFaceAELCELinkEnable);
    SET_TAG(AE_TAG_FACE_20_LCE_MAX_GAIN, aeAlgoData->m_u4MaxGain);
    SET_TAG(AE_TAG_FACE_20_IS_FRONT, aeAlgoData->m_MeterFDSts.m_u4IsRobustFace);
    SET_TAG(AE_TAG_FACE_20_IS_SIDE, aeAlgoData->m_MeterFDSts.m_u4IsSideFace);
    SET_TAG(AE_TAG_FACE_20_IS_OT, aeAlgoData->m_MeterFDSts.m_u4IsOTFace);
    SET_TAG(AE_TAG_FACE_20_CASE_NUMBER, aeAlgoData->m_u4FaceCaseNum);
    SET_TAG(AE_TAG_FACE_20_FOUND_CNT, aeAlgoData->m_MeterFDSts.u4FaceFoundCnt);
    SET_TAG(AE_TAG_FACE_20_FAIL_CNT, aeAlgoData->m_MeterFDSts.u4FaceFailCnt);
    SET_TAG(AE_TAG_FACE_20_STABLE_CNT, aeAlgoData->m_MeterFDSts.m_u4FaceStableCount);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE, aeAlgoData->m_u4FaceAEStable);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_INDEX, aeAlgoData->m_u4FaceAEStableIndex);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_CWV, aeAlgoData->m_u4FaceAEStableCWV);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_CWR, aeAlgoData->m_u4FaceAEStableCWR);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_INTHD, aeAlgoData->m_u4FaceAEStableInThd);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_OUTTHD, aeAlgoData->m_u4FaceAEStableOutThd);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_XLOW, aeAlgoData->m_FaceAEStableBlock.u4XLow);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_XHI, aeAlgoData->m_FaceAEStableBlock.u4XHi);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_YLOW, aeAlgoData->m_FaceAEStableBlock.u4YLow);
    SET_TAG(AE_TAG_FACE_20_FACE_AE_STABLE_YHI, aeAlgoData->m_FaceAEStableBlock.u4YHi);
    SET_TAG(AE_TAG_FACE_20_AE_COMP, aeAlgoData->m_AECmdSet.eAEComp);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACE_NUM, aeAlgoData->m_MeterFDSts.m_u4FaceNum);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_0, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[0]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_1, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[1]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_2, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[2]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_3, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[3]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_4, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[4]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_5, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[5]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_6, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[6]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_7, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[7]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FACEY_8, aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[8]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_0, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[0]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_1, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[1]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_2, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[2]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_3, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[3]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_4, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[4]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_5, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[5]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_6, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[6]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_7, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[7]);
    SET_TAG(AE_TAG_FACE_20_MULTI_LMY_8, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[8]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_0, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[0]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_1, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[1]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_2, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[2]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_3, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[3]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_4, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[4]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_5, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[5]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_6, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[6]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_7, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[7]);
    SET_TAG(AE_TAG_FACE_20_MULTI_FDY_8, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[8]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_0, aeAlgoData->m_i4FDY_Array_CCUFrameDone[0]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_1, aeAlgoData->m_i4FDY_Array_CCUFrameDone[1]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_2, aeAlgoData->m_i4FDY_Array_CCUFrameDone[2]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_3, aeAlgoData->m_i4FDY_Array_CCUFrameDone[3]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_4, aeAlgoData->m_i4FDY_Array_CCUFrameDone[4]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_5, aeAlgoData->m_i4FDY_Array_CCUFrameDone[5]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_6, aeAlgoData->m_i4FDY_Array_CCUFrameDone[6]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_7, aeAlgoData->m_i4FDY_Array_CCUFrameDone[7]);
    SET_TAG(AE_TAG_FACE_20_MULTI_SORT_FDY_8, aeAlgoData->m_i4FDY_Array_CCUFrameDone[8]);
    
    SET_TAG(AE_TAG_FACE_20_PARA_LOCK_MAX_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Lock_MaxCnt);
    SET_TAG(AE_TAG_FACE_20_PARA_TEMPORAL_SMOOTH, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_TemporalSmooth);
    SET_TAG(AE_TAG_FACE_20_PARA_OT_LOCK_UPPER, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_OTLockUpperBnd);
    SET_TAG(AE_TAG_FACE_20_PARA_OT_LOCK_LOWER, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_OTLockLowerBnd);
    SET_TAG(AE_TAG_FACE_20_PARA_CONT_TRUST_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ContinueTrustCnt);
    SET_TAG(AE_TAG_FACE_20_PARA_LOC_SIZE_CHECK, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.bFaceLocSizeCheck);
    SET_TAG(AE_TAG_FACE_20_PARA_IN_STALE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceInStableThd);
    SET_TAG(AE_TAG_FACE_20_PARA_OUT_B2T_STALE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceOutB2TStableThd);
    SET_TAG(AE_TAG_FACE_20_PARA_OUT_D2T_STALE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceOutD2TStableThd);
    SET_TAG(AE_TAG_FACE_20_PARA_IN_TO_OUT_THD_MAX_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_InToOutThdMaxCnt);
    SET_TAG(AE_TAG_FACE_20_PARA_SIDE_FACE_LOCK, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_SideFaceLock);
    SET_TAG(AE_TAG_FACE_20_PARA_LIMIT_STABLE_THD_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBnd);
    SET_TAG(AE_TAG_FACE_20_PARA_LIMIT_STABLE_THD_NUM, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBndNum);
    SET_TAG(AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_FACE_CHANGE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReConvergeWhenFaceChange);
    SET_TAG(AE_TAG_FACE_20_PARA_MOTION_LOCK_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_FaceMotionLockRat);
    SET_TAG(AE_TAG_FACE_20_PARA_IMPULSE_LOCK_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ImpulseLockCnt);
    SET_TAG(AE_TAG_FACE_20_PARA_IMPULSE_UPPER_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ImpulseUpperThd);
    SET_TAG(AE_TAG_FACE_20_PARA_IMPULSE_LOWER_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ImpulseLowerThd);
    SET_TAG(AE_TAG_FACE_20_PARA_ROP_TRUST_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ROPTrustCnt);
    SET_TAG(AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_SIZE_CHANGE_RAT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReCovergeWhenSizeChangeRat);
    SET_TAG(AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_POS_CHANGE_DIST, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReCovergeWhenPosChangeDist);
    SET_TAG(AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_FACE_AF_DONE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReCovergeWhenAFDone);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.bEnableMultiFaceWeight);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_PRIOR, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.Prior);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[0]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[1]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[2]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_3, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[3]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_4, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[4]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_5, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[5]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_6, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[6]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_7, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[7]);
    SET_TAG(AE_TAG_FACE_20_PARA_MULTI_WEIGHT_8, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[8]);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_EXT_RATIO_W, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkExtRatW);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_EXT_RATIO_H, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkExtRatH);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_TRUST_ROP, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkTrustRopDegree);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_SMOOTH_XY, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4RoundXYPercent);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_SMOOTH_SIZE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4RoundSZPercent);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_SMOOTH_OVERLAP, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4THOverlap);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_SMOOTH_BUFLEN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4BUFLEN);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_SMOOTH_LEVEL, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4SMOOTH_LEVEL);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_SMOOTH_MOMENTUM, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4MOMENTUM);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_MIX_WEIGHT_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkWeightPercent);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_CV_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkCV_Thd);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_CV_BV, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkCV_BV);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_ROP_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkROP_Thd);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_ROP_BV, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkROP_BV);

    SET_TAG(AE_TAG_FACE_20_PARA_LM_BK_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkBKMode);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_BK_CVthd, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkBK_CVthd);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_BK_FYthd, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkBK_FYthd);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_BK_Weight, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkBK_Weight);
    SET_TAG(AE_TAG_FACE_20_PARA_LM_BK_BV, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkBK_BV);

    // for Face AE
    SET_TAG(AE_TAG_FACE_CNT, aeAlgoData->m_MeterFDSts.m_u4FaceNum);
    for(i=0; i<9; i++)
    {
        SET_TAG(AE_TAG_FACE0_XLOW+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4XLow);
        SET_TAG(AE_TAG_FACE0_XHI+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4XHi);
        SET_TAG(AE_TAG_FACE0_YLOW+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4YLow);
        SET_TAG(AE_TAG_FACE0_YHI+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4YHi);
        SET_TAG(AE_TAG_FACE0_WEIGHT+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].u4Weight);
        SET_TAG(AE_TAG_FACE0_FULLY+7*i, aeAlgoData->m_i4FDY_Array_CCUFrameDone[i]);
        SET_TAG(AE_TAG_FACE0_CENTERY+7*i, aeAlgoData->m_MeterFDSts.m_u4FaceCentralYValue[i]);

        SET_TAG(AE_TAG_FACE_20_SCALE0_XLOW+4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4XLow);
        SET_TAG(AE_TAG_FACE_20_SCALE0_XHI +4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4XHi);
        SET_TAG(AE_TAG_FACE_20_SCALE0_YLOW+4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4YLow);
        SET_TAG(AE_TAG_FACE_20_SCALE0_YHI +4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4YHi);
		
        SET_TAG(AE_TAG_FACE0_WEIGHT2K+i, aeAlgoData->m_MeterFDSts.m_u4Weighting[i]);
    }
    SET_TAG(AE_TAG_FACE_BEFEXT_XLOW, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4BefExtXLow);
    SET_TAG(AE_TAG_FACE_BEFEXT_XHI, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4BefExtXHi);
    SET_TAG(AE_TAG_FACE_BEFEXT_YLOW, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4BefExtYLow);
    SET_TAG(AE_TAG_FACE_BEFEXT_YHI, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4BefExtYHi);
    
    SET_TAG(AE_TAG_FACE0_WEIGHT2K, aeAlgoData->m_MeterFDSts.m_u4Weighting[0]);
    SET_TAG(AE_TAG_FACE1_WEIGHT2K, aeAlgoData->m_MeterFDSts.m_u4Weighting[1]);
    SET_TAG(AE_TAG_FACE2_WEIGHT2K, aeAlgoData->m_MeterFDSts.m_u4Weighting[2]);

    SET_TAG(AE_TAG_FACE_20_LANDMARK0_XLOW, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkXLow);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_XHI, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkXHi);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_YLOW, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkYLow);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_YHI, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkYHi);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_RIP, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4LandMarkRip);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_ROP, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4LandMarkRop);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_CV, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4LandmarkCV);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_ICS_LEFT, aeAlgoData->m_LandMarkFDSmoothArea.i4Left);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_ICS_RIGHT, aeAlgoData->m_LandMarkFDSmoothArea.i4Right);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_ICS_TOP, aeAlgoData->m_LandMarkFDSmoothArea.i4Top);
    SET_TAG(AE_TAG_FACE_20_LANDMARK0_ICS_BOTTOM, aeAlgoData->m_LandMarkFDSmoothArea.i4Bottom);

    //for Metering temporal smooth
    SET_TAG(AE_TAG_FACE_ID0_ROP,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEHsTS);
    SET_TAG(AE_TAG_TEMP_SMOOTH_ABL,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEAblTS);
    SET_TAG(AE_TAG_TEMP_SMOOTH_AOE,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEAoeTS);
    SET_TAG(AE_TAG_TEMP_SMOOTH_NS,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAENsTS);
    SET_TAG(AE_TAG_TEMP_SMOOTH_VIDEO,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEVideoTS);
    SET_TAG(AE_TAG_EN_METER_AE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableMeterAE);
    SET_TAG(AE_TAG_METERING_EN, aeAlgoData->m_bAEMeteringEnable);
    SET_TAG(AE_TAG_METERING_Y, aeAlgoData->m_u4MeterY);
    

    SET_TAG(AE_TAG_METERING_Y_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4CwrLowBound);
    SET_TAG(AE_TAG_METERING_Y_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4CwrHighBound);
    SET_TAG(AE_TAG_METERING_Y0, aeAlgoData->m_pu4MeteringY[0]);    

    SET_TAG(AE_TAG_METERING_CNT, aeAlgoData->m_u4MeteringCnt);
    SET_TAG(AE_TAG_METERING0_XLOW, aeAlgoData->m_AETOUCHWinBlock[0].u4XLow);
    SET_TAG(AE_TAG_METERING0_XHI, aeAlgoData->m_AETOUCHWinBlock[0].u4XHi);
    SET_TAG(AE_TAG_METERING0_YLOW, aeAlgoData->m_AETOUCHWinBlock[0].u4YLow);
    SET_TAG(AE_TAG_METERING0_YHI, aeAlgoData->m_AETOUCHWinBlock[0].u4YHi);
    SET_TAG(AE_TAG_METERING0_WEIGHT, aeAlgoData->m_AETOUCHWinBlock[0].u4Weight);
    
    // AE window and histogram configure setting
    SET_TAG(AE_TAG_PRV_MAX_BLOCK_WIDTH, aeInitData->m_i4AEMaxBlockWidth);
    SET_TAG(AE_TAG_PRV_MAX_BLOCK_HEIGHT, aeInitData->m_i4AEMaxBlockHeight);
    SET_TAG(AE_TAG_STAT_XLOW, aeAlgoData->m_AEWinBlock.u4XLow); //vsync timing
    SET_TAG(AE_TAG_STAT_XHI, aeAlgoData->m_AEWinBlock.u4XHi); //vsync timing
    SET_TAG(AE_TAG_STAT_YLOW, aeAlgoData->m_AEWinBlock.u4YLow); //vsync timing
    SET_TAG(AE_TAG_STAT_YHI, aeAlgoData->m_AEWinBlock.u4YHi); //vsync timing
    
    SET_TAG(AE_TAG_STB_DEFAULT_IN_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4InStableThd);
    SET_TAG(AE_TAG_STB_DEFAULT_OUT_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4OutStableThd);
    SET_TAG(AE_TAG_STB_EN_MODE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.bEnableAEModeStableTHD);
    SET_TAG(AE_TAG_STB_Video_IN_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4VideoInStableThd);
    SET_TAG(AE_TAG_STB_Video_OUT_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4VideoOutStableThd);
    SET_TAG(AE_TAG_STB_Face_IN_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceInStableThd);
    SET_TAG(AE_TAG_STB_Face_OUT_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceOutStableThd);
    SET_TAG(AE_TAG_STB_Touch_IN_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4TouchInStableThd);
    SET_TAG(AE_TAG_STB_Touch_OUT_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4TouchOutStableThd);

    SET_TAG(AE_TAG_EN_TOUCH_SMOOTH, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableTouchSmoothRatio);
    SET_TAG(AE_TAG_EN_TOUCH_PERFRAME, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableTouchSmooth);
    SET_TAG(AE_TAG_EN_TOUCH_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeterWeight);

    // isp 6.0 flow
    SET_TAG(AE_TAG_FLOW_AE_STATE, aeAlgoData->m_AECtrl.eAEState);
    SET_TAG(AE_TAG_FLOW_IS_LOCK, (aeAlgoData->m_AECtrl.bIsAFLock || aeAlgoData->m_AECtrl.bIsAPLock));
    SET_TAG(AE_TAG_FLOW_IS_CALCULATE, aeAlgoData->m_AECtrl.bIsCalculate);
    SET_TAG(AE_TAG_FLOW_CYCLE_CNT, aeAlgoData->m_CycleCtrl.u4CycleCnt);
    SET_TAG(AE_TAG_FLOW_MAGIC_NO, aeAlgoData->m_CycleCtrl.u4MagicNo);
    SET_TAG(AE_TAG_FLOW_DELAY_CNT, aeAlgoData->m_CycleCtrl.u4DelayCnt);
    SET_TAG(AE_TAG_FLOW_IS_SLOW_MOTION, aeAlgoData->m_FeatureCtrl.bIsSlowMotion);
    SET_TAG(AE_TAG_FLOW_IS_LIMIT, aeAlgoData->m_FeatureCtrl.bIsLimitMode);
    SET_TAG(AE_TAG_FLOW_MAIN_STATE, aeAlgoData->m_AlgoCtrl.eMainState);
    SET_TAG(AE_TAG_FLOW_SUB_STATE, aeAlgoData->m_AlgoCtrl.eSubState);
    SET_TAG(AE_TAG_FLOW_CURR_FRM_CNT, aeAlgoData->m_AlgoCtrl.u4CurrFrameCnt);
    SET_TAG(AE_TAG_FLOW_REST_FRM_CNT, aeAlgoData->m_AlgoCtrl.u4WaitFrameCnt);

    // isp 6.0 metering
    SET_TAG(AE_TAG_ABL_STRENGTH_BYBV_EN, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.bEnableTargetStrengthByBV);
    SET_TAG(AE_TAG_CS_GS_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.bEnableGreenSupress);
    SET_TAG(AE_TAG_CS_GS_GRTH, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenDetectGRRatio);
    SET_TAG(AE_TAG_CS_GS_GBTH, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenDetectGBRatio);
    SET_TAG(AE_TAG_CS_GS_GCNTRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenRatio[0]);
    SET_TAG(AE_TAG_CS_GS_GCNTRATIO_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenRatio[1]);
    SET_TAG(AE_TAG_CS_GS_GCNTRATIO_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenProb[0]);
    SET_TAG(AE_TAG_CS_GS_GCNTRATIO_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenProb[1]);
    SET_TAG(AE_TAG_CS_GS_DETMIN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ColorSup.GreenSup.u4GreenDetectMin);
    SET_TAG(AE_TAG_CS_GS_STS_GREENCNT, aeAlgoData->m_TargetSts.u4Orig_Cs_Gs_G_Ratio);
    SET_TAG(AE_TAG_CS_GS_STS_GREENRATIO, aeAlgoData->m_TargetSts.u4Prob_Cs_Gs_GC);
    SET_TAG(AE_TAG_CS_GS_PROB, aeAlgoData->m_TargetSts.u4Prob_Cs_Gs);
    SET_TAG(AE_TAG_CS_GS_PROB_BV, aeAlgoData->m_TargetSts.u4Prob_Cs_Gs_BV);
    SET_TAG(AE_TAG_CS_GS_PROB_GC, aeAlgoData->m_TargetSts.u4Prob_Cs_Gs_GC);
    SET_TAG(AE_TAG_CS_GS_HS4P0_ORG_Y, aeAlgoData->m_TargetSts.u4Orig_Cs_Gs_HS_LimtY);
    SET_TAG(AE_TAG_CS_GS_SUP_Y, aeAlgoData->m_TargetSts.u4Pure_Cs_Gs_HS_LimtY);
    SET_TAG(AE_TAG_CS_GS_HS4P0_GS_Y, aeAlgoData->m_TargetSts.u4Cs_Gs_HS_LImitY);
    SET_TAG(AE_TAG_CS_GS_MT_ORG_Y, aeAlgoData->m_TargetSts.u4Orig_Cs_Gs_MT_CWV);
    SET_TAG(AE_TAG_CS_GS_MT_GS_Y, aeAlgoData->m_TargetSts.u4Cs_Gs_MT_CWV);

    SET_TAG(AE_TAG_FACE_SIZELOC_BYDR_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.bFaceLocSizeByBv);
    SET_TAG(AE_TAG_FACE_SIZELOC_DR, aeAlgoData->m_MeterFDSts.m_i4FaceXDR);

    SET_TAG(AE_TAG_FBT_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FBTParam.bEnableFBT);
    SET_TAG(AE_TAG_FBT_SKP, aeAlgoData->m_FBTRep.m_u4SkipPcent);
    SET_TAG(AE_TAG_FBT_OEP, aeAlgoData->m_FBTRep.m_u4OEPcent);
    SET_TAG(AE_TAG_FBT_XP_SYS, aeAlgoData->m_FBTRep.m_u410PcentY);
    SET_TAG(AE_TAG_FBT_OE_SYS, aeAlgoData->m_FBTRep.m_u4OEPcentY);
    SET_TAG(AE_TAG_FBT_DR, aeAlgoData->m_FBTRep.m_i4DR);
    
    SET_TAG(AE_TAG_FBT_OETH, aeAlgoData->m_FBTRep.m_u4OeTh);
    SET_TAG(AE_TAG_FBT_FDTH, aeAlgoData->m_FBTRep.m_u4FdTh);
    SET_TAG(AE_TAG_FBT_FDMINTH, aeAlgoData->m_FBTRep.m_u4FdMinTh);
    SET_TAG(AE_TAG_FBT_FDY, aeAlgoData->m_FBTRep.m_u4FDY);
    SET_TAG(AE_TAG_FBT_TARGET, aeAlgoData->m_FBTRep.u4FaceBalancedTarget);
    SET_TAG(AE_TAG_PURE_AE_CWR, aeAlgoData->m_u4CWRecommendPureAE);
    SET_TAG(AE_TAG_LINK_AE_CWR, aeAlgoData->m_u4CWRecommend);
    SET_TAG(AE_TAG_PURE_AE_CWR_STABLE, aeAlgoData->m_u4CWRecommendPureAE_Stable);
    SET_TAG(AE_TAG_PURE_AE_CWR_AF_STABLE, aeAlgoData->m_u4CWRecommendAfterPureAE_Stable);
    SET_TAG(AE_TAG_LINK_AE_CWR_STABLE, aeAlgoData->m_u4CWRecommend_Stable);

    SET_TAG(AE_TAG_EN_LTM_LINK, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.bEnableLTMOE_Link);
    SET_TAG(AE_TAG_LTM_WEIGHT, (MUINT32)(m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.fLtm_Cap_Weight * 1000));
    SET_TAG(AE_TAG_LTM_FINAL_WEIGHT, (MUINT32)(aeAlgoData->m_TargetSts.Final_Ltm_Weight* 1000));
    
    
    SET_TAG(AE_TAG_LTM_OFFSET, (MUINT32)(aeAlgoData->m_ltm_to_ae_info.ltm_cap * 1000));
    SET_TAG(AE_TAG_LTM_FINAL_OFFSET, (MUINT32)(aeAlgoData->m_TargetSts.Final_LTM_CAP * 1000));
    SET_TAG(AE_TAG_AOE_STS_OE_HBOUND, aeAlgoData->m_TargetSts.Prob_OE_Value_Hbound);
    SET_TAG(AE_TAG_AOE_STS_OE_ORIY, aeAlgoData->m_TargetSts.OE_DeltaY_Ori_SE);
    SET_TAG(AE_TAG_AOE_STS_OE_SEY, aeAlgoData->m_TargetSts.OE_DeltaY_SE);
    SET_TAG(AE_TAG_AOE_STS_DELTA_SEY, aeAlgoData->m_TargetSts.OE_DeltaY_Ori_SE);
    SET_TAG(AE_TAG_LTM_CT, aeAlgoData->m_ltm_to_ae_info.u4ClipThd);

    // isp 6.0 face
    for(i=0; i<9; i++)
    {
        SET_TAG(AE_TAG_FACE_ID0_ROP+i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4Rop);
        SET_TAG(AE_TAG_FACE_MOTION_X0+2*i, aeAlgoData->m_MeterFDSts.m_i4FDMotionHist[i][0]);
        SET_TAG(AE_TAG_FACE_MOTION_Y0+2*i, aeAlgoData->m_MeterFDSts.m_i4FDMotionHist[i][1]);
    }

    SET_TAG(AE_TAG_FACE_LCE_LINK_EN,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ContrastLinkParam.bEnableContrastLink);
    SET_TAG(AE_TAG_FACE_LOCK, aeAlgoData->m_bFaceAELock);
    SET_TAG(AE_TAG_FACE_LCETARGET, aeAlgoData->m_MeterFDSts.m_u4LCETarget);
    SET_TAG(AE_TAG_FACE_LCEHALFTARGET, aeAlgoData->m_MeterFDSts.m_u4LCEHalfTarget);
    
    for(i=0; i<9; i++)
    {
        SET_TAG(AE_TAG_FACE_20_MULTI_AVGFDY_0+i, aeAlgoData->m_i4FDY_Array[i]);
    }
    for(i=0; i<4; i++)
    {
        SET_TAG(AE_TAG_FACE_SIZE_XLOW_0+4*i, aeAlgoData->m_MeterFDSts.m_i4FDSizeHist[0][0]);
        SET_TAG(AE_TAG_FACE_SIZE_XHIGH_0+4*i, aeAlgoData->m_MeterFDSts.m_i4FDSizeHist[0][1]);
        SET_TAG(AE_TAG_FACE_SIZE_YLOW_0+4*i, aeAlgoData->m_MeterFDSts.m_i4FDSizeHist[0][2]);
        SET_TAG(AE_TAG_FACE_SIZE_YHIGH_0+4*i, aeAlgoData->m_MeterFDSts.m_i4FDSizeHist[0][3]);
    }

    SET_TAG(AE_TAG_FACE_LTM_0_XLOW, aeAlgoData->m_MeterFDSts.m_LTMFDWinBlock_Scale[0].u4XLow);
    SET_TAG(AE_TAG_FACE_LTM_0_XLOW, aeAlgoData->m_MeterFDSts.m_LTMFDWinBlock_Scale[0].u4XLow);
    SET_TAG(AE_TAG_FACE_LTM_0_XLOW, aeAlgoData->m_MeterFDSts.m_LTMFDWinBlock_Scale[0].u4XLow);
    SET_TAG(AE_TAG_FACE_LTM_0_XLOW, aeAlgoData->m_MeterFDSts.m_LTMFDWinBlock_Scale[0].u4XLow);
    SET_TAG(AE_TAG_FACE_LTM_BRIGHT_AVG_0, aeAlgoData->m_i4LTMFaceBrightToneValue_12bit[0]);
    SET_TAG(AE_TAG_FACE_LTM_PROTECT_PERCENT, aeAlgoData->m_i4LTMFaceProtectPercent);
    SET_TAG(AE_TAG_FACE_LTM_BRIGHT_AVG_PERCENT, aeAlgoData->m_i4LTMFaceProtectBrightTonePercent);
    
    // isp 6.0 smooth
    SET_TAG(AE_TAG_STABLE_CASE, aeAlgoData->m_StableCase);
    SET_TAG(AE_TAG_STABLE_CASE_C, aeAlgoData->m_StableCaseCur);
    SET_TAG(AE_TAG_SMOOTH_STATUS, aeAlgoData->e_prev_smooth_status);
    SET_TAG(AE_TAG_PREV_DELTA_IDX, aeAlgoData->prev_delta_idx);
    SET_TAG(AE_TAG_PREV2_DELTA_IDX, aeAlgoData->prev2_delta_idx);
    SET_TAG(AE_TAG_PREV3_DELTA_IDX, aeAlgoData->prev3_delta_idx);
    SET_TAG(AE_TAG_DELTA_INDEX_FULL, aeAlgoData->delta_index_full);
    SET_TAG(AE_TAG_NON_LINEAR_CWV_RATIO, aeAlgoData->non_linear_cwv_ratio);
    SET_TAG(AE_TAG_NON_LINEAR_AVG_RATIO, aeAlgoData->non_linear_avg_ratio);
    SET_TAG(AE_TAG_OVER_PROB, aeAlgoData->over_prob);
    SET_TAG(AE_TAG_OVER_PL_PROB, aeAlgoData->over_point_light_prob);

    SET_TAG(AE_TAG_FDY, aeAlgoData->m_i4Crnt_FDY);
    SET_TAG(AE_TAG_ONE_SHOT, aeAlgoData->m_bAEOneShot);


    // Folivora effect
    SET_TAG(AE_TAG_FOLIVORA_ENABLE, aeAlgoData->m_bFolivoraEffectEnable);
    SET_TAG(AE_TAG_FOLIVORA_IS_IN_EFFECT, aeAlgoData->m_bInFolivoraEffect);
    SET_TAG(AE_TAG_FOLIVORA_IS_MOVING, aeAlgoData->m_bIsFolivoraMoving);
    SET_TAG(AE_TAG_FOLIVORA_IS_B_MOVING, aeAlgoData->m_bIsFolivoraBMoving);
    SET_TAG(AE_TAG_FOLIVORA_IS_F_MOVING, aeAlgoData->m_bIsFolivoraFMoving);
    SET_TAG(AE_TAG_FOLIVORA_IS_IN_TRACKING, aeAlgoData->m_bIsFolivoraInTracking);
    SET_TAG(AE_TAG_FOLIVORA_HESITATE, aeAlgoData->m_u4FolivoraHesitate);
    SET_TAG(AE_TAG_FOLIVORA_B_TICKET, aeAlgoData->m_u4FolivoraBTicket);
    SET_TAG(AE_TAG_FOLIVORA_F_TICKET, aeAlgoData->m_u4FolivoraFTicket);
    SET_TAG(AE_TAG_FOLIVORA_C_TICKET, aeAlgoData->m_u4FolivoraCurTicket);
    SET_TAG(AE_TAG_FOLIVORA_B_VALUE, aeAlgoData->m_u4FolivoraBValue);
    SET_TAG(AE_TAG_FOLIVORA_C_VALUE, aeAlgoData->m_u4FolivoraCurValue);
    SET_TAG(AE_TAG_FOLIVORA_MOVING_QUANTITY, aeAlgoData->m_u4FolivoraFMovingQuantity);
    SET_TAG(AE_TAG_FOLIVORA_B_MOVING_TH, aeAlgoData->m_u4FolivoraBMovingTH);
    SET_TAG(AE_TAG_FOLIVORA_F_MOVING_TH, aeAlgoData->m_u4FolivoraFMovingTH);
    SET_TAG(AE_TAG_FOLIVORA_MOVING_QUANTITY_TH, aeAlgoData->m_u4FolivoraFMovingQuantityTH);
    SET_TAG(AE_TAG_FOLIVORA_DARK_TH, aeAlgoData->m_u4FolivoraDarkMotionTH);
    SET_TAG(AE_TAG_FOLIVORA_DARK_MIN_MOTION, aeAlgoData->m_u4FolivoraDarkMinMotion);
    SET_TAG(AE_TAG_FOLIVORA_TRACKING_TH, aeAlgoData->m_u4FolivoraTrackingTH);
    SET_TAG(AE_TAG_FOLIVORA_HESITATE_TH, aeAlgoData->m_u4FolivoraHesitateTH);

    //Butter fly effect
    SET_TAG(AE_TAG_BF_ENABLE, aeAlgoData->m_bBtEffectEnable);
    SET_TAG(AE_TAG_BF_IN_EFFECT, aeAlgoData->m_bBtInEffect);
    SET_TAG(AE_TAG_BF_OVER, aeAlgoData->m_bBtOver);
    SET_TAG(AE_TAG_BF_ORG_STABLE, aeAlgoData->m_bBtOrgStable);
    SET_TAG(AE_TAG_BF_STABLE, aeAlgoData->m_bBtStable);
    SET_TAG(AE_TAG_BF_PRE_DEV_STABLE, aeAlgoData->m_bBtPreDevStable);
    SET_TAG(AE_TAG_BF_PRE_DEV_INDEX, aeAlgoData->m_u4BtPreDevIndex);
    SET_TAG(AE_TAG_BF_B_V, aeAlgoData->m_u4BtBValue);
    SET_TAG(AE_TAG_BF_C_V, aeAlgoData->m_u4BtCurValue);
    SET_TAG(AE_TAG_BF_B_TICKET, aeAlgoData->m_u4BtBTicket);
    SET_TAG(AE_TAG_BF_C_TICKET, aeAlgoData->m_u4BtCTicket);
    SET_TAG(AE_TAG_BF_RightAfterWT, aeAlgoData->m_bBtRightAfterSync);
    SET_TAG(AE_TAG_BF_SYNC, aeAlgoData->m_bBtSync);
    SET_TAG(AE_TAG_BF_MAPPING, aeAlgoData->m_bBtMapping);
    SET_TAG(AE_TAG_BF_TH, aeAlgoData->m_u4BtTh);
    SET_TAG(AE_TAG_BF_SPEED, aeAlgoData->m_u4BtSpeed);
    setDebugTag(a_rAEDebugInfo,AE_TAG_BF_CHANG_CNT, aeAlgoData->m_u4BtChangeCnt);
    setDebugTag(a_rAEDebugInfo,AE_TAG_BF_UPBOUND, aeAlgoData->m_u4BtUpBound);
    setDebugTag(a_rAEDebugInfo,AE_TAG_BF_LOWBOUND, aeAlgoData->m_u4BtLowBound);


    SET_TAG(AE_TAG_EN_BVACCRATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.u4BVAccEnable);
    SET_TAG(AE_TAG_BVACC_DELTA_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.i4DeltaBVRatio);
    SET_TAG(AE_TAG_BVACC_B2TEND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.pAEBVAccRatio.u4Bright2TargetEnd);
    SET_TAG(AE_TAG_BVACC_B2TEND_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.pAEBVAccRatio.u4B2TEnd);
    SET_TAG(AE_TAG_BVACC_B2TSTART_RATIO,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.pAEBVAccRatio.u4B2TStart);

    SET_TAG(AE_TAG_CAM_MODE, aeAlgoData->m_AECmdSet.eAECamMode);
    SET_TAG(AE_TAG_VIDEO_DFS_EN, aeAlgoData->m_AECmdSet.bVideoDynamic);
    SET_TAG(AE_TAG_SENSOR_ID, aeAlgoData->m_AECmdSet.eSensorDev);
    SET_TAG(AE_TAG_AE_LOCKED, aeAlgoData->m_AECmdSet.bAELock);
    SET_TAG(AE_TAG_VIDEO_RECORD, aeAlgoData->m_AECmdSet.bVideoRecording);
    SET_TAG(AE_TAG_AE_LIMITER, aeAlgoData->m_AECmdSet.bAElimitor);
    SET_TAG(AE_TAG_STROBE_ON, aeAlgoData->m_AECmdSet.bStrobeOn);
    SET_TAG(AE_TAG_ROTATE_WEIGHTING, aeAlgoData->m_AECmdSet.bRotateWeighting);
    SET_TAG(AE_TAG_OBJECT_TRACKING, aeAlgoData->m_AECmdSet.bAEObjectTracking);
    SET_TAG(AE_TAG_INFO_0, AE_ALGO_REVISION);
    SET_TAG(AE_TAG_INFO_1, AE_ALGO_IF_REVISION);
    SET_TAG(AE_TAG_INFO_2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.u4TuningVersion);
    SET_TAG(AE_TAG_INFO_3, DBG_AE_PARAM_REVISION);
    SET_TAG(AE_TAG_INFO_4, AE_ALGO_TOOL_REVISION);

    // 3hdr
    SET_TAG(AE_TAG_LE_SHUTTER_TIME, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4LEExpo);
    SET_TAG(AE_TAG_LE_SENSOR_GAIN, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4LEAfeGain);
    SET_TAG(AE_TAG_LE_ISP_GAIN, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4LEIspGain);
    SET_TAG(AE_TAG_ME_SHUTTER_TIME, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4MEExpo);
    SET_TAG(AE_TAG_ME_SENSOR_GAIN, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4MEAfeGain);
    SET_TAG(AE_TAG_ME_ISP_GAIN, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4MEIspGain);
    SET_TAG(AE_TAG_SE_SHUTTER_TIME, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4SEExpo);
    SET_TAG(AE_TAG_SE_SENSOR_GAIN, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4SEAfeGain);
    SET_TAG(AE_TAG_SE_ISP_GAIN, aeAlgoData->m_CurAEOutput.HdrEvSetting.i4SEIspGain);

    SET_TAG(AE_TAG_3HDR_REAL_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.u4realStatHDRRatio);
    SET_TAG(AE_TAG_3HDR_PRE_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.u4preHDRRatio);
    SET_TAG(AE_TAG_3HDR_CUR_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.u4curHDRRatio);
    SET_TAG(AE_TAG_3HDR_TARGET_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.u4targetHDRRatio);
    SET_TAG(AE_TAG_3HDR_LE_CNT, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.u4MHDRLEYHistTotalCount);
    SET_TAG(AE_TAG_3HDR_SE_CNT, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.u4MHDRSEYHistTotalCount);
    
    SET_TAG(AE_TAG_3HDR_BVX1000, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.i4MHDRBV);
    SET_TAG(AE_TAG_3HDR_CWV, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.u4MHDRCWValue);
    SET_TAG(AE_TAG_3HDR_FDY_AAO, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.u4MHDRFDY);
    SET_TAG(AE_TAG_3HDR_FDY_SENSOR, 0);
    SET_TAG(AE_TAG_3HDR_TARGET, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.sHdrTargetInfo.u4MHDRTarget);
    SET_TAG(AE_TAG_3HDR_BVTARGET, aeAlgoData->m_HdrAEInfo.sMHDRAEInfo.sHdrTargetInfo.u4MHDRBVTarget);
    SET_TAG(AE_TAG_3HDR_HSTARGET, aeAlgoData->m_TargetSts.MVHDR_Hist_HS_Target);

    SET_TAG(AE_TAG_3HDR_FACE_MULTI_WEIGHT_PRIOR, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4MultiFacePrior);
    SET_TAG(AE_TAG_HDR_FACE_Y_LOWBOUND, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.uFaceYLowBound);
    SET_TAG(AE_TAG_HDR_FACE_Y_HIGHBOUND, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.uFaceYHighBound);
    SET_TAG(AE_TAG_HDR_FACE_BVRATIO_LOWBOUND, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4FaceBVLowBound);
    SET_TAG(AE_TAG_HDR_FACE_BVRATIO_HIGHBOUND, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4FaceBVHighBound);
    
    SET_TAG(AE_TAG_3HDR_BV4FDTARGET_0, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4BV4HDRFDTarget[0]);
    SET_TAG(AE_TAG_3HDR_BV4FDTARGET_1, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4BV4HDRFDTarget[1]);
    SET_TAG(AE_TAG_3HDR_BV4FDTARGET_2, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4BV4HDRFDTarget[2]);
    SET_TAG(AE_TAG_3HDR_BV4FDTARGET_3, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4BV4HDRFDTarget[3]);
    SET_TAG(AE_TAG_3HDR_FDTARGET_0, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.u4HDRFDTarget[0]);
    SET_TAG(AE_TAG_3HDR_FDTARGET_1, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.u4HDRFDTarget[1]);
    SET_TAG(AE_TAG_3HDR_FDTARGET_2, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.u4HDRFDTarget[2]);
    SET_TAG(AE_TAG_3HDR_FDTARGET_3, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.u4HDRFDTarget[3]);

    SET_TAG(AE_TAG_3HDR_MAX_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.u4maxHDRRatio);
    SET_TAG(AE_TAG_3HDR_OE1_BIN_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.i4OE1BinRatio);
    SET_TAG(AE_TAG_3HDR_OE2_BIN_RATIO, aeAlgoData->m_HdrAEInfo.sHDRRatioInfo.i4OE2BinRatio);

    SET_TAG(AE_TAG_3HDR_OE1_PCNT_0, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE1Pct[0]);
    SET_TAG(AE_TAG_3HDR_OE1_PCNT_1, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE1Pct[1]);
    SET_TAG(AE_TAG_3HDR_OE1_PCNT_2, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE1Pct[2]);
    SET_TAG(AE_TAG_3HDR_OE1_PCNT_3, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE1Pct[3]);
    SET_TAG(AE_TAG_3HDR_OE1_PCNT_4, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE1Pct[4]);
    SET_TAG(AE_TAG_3HDR_OE1_RATIO_MOVE_0, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE1[0]);
    SET_TAG(AE_TAG_3HDR_OE1_RATIO_MOVE_1, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE1[1]);
    SET_TAG(AE_TAG_3HDR_OE1_RATIO_MOVE_2, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE1[2]);
    SET_TAG(AE_TAG_3HDR_OE1_RATIO_MOVE_3, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE1[3]);
    SET_TAG(AE_TAG_3HDR_OE1_RATIO_MOVE_4, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE1[4]);

    SET_TAG(AE_TAG_3HDR_OE2_PCNT_0, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE2Pct[0]);
    SET_TAG(AE_TAG_3HDR_OE2_PCNT_1, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE2Pct[1]);
    SET_TAG(AE_TAG_3HDR_OE2_PCNT_2, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE2Pct[2]);
    SET_TAG(AE_TAG_3HDR_OE2_PCNT_3, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE2Pct[3]);
    SET_TAG(AE_TAG_3HDR_OE2_PCNT_4, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4OE2Pct[4]);
    SET_TAG(AE_TAG_3HDR_OE2_RATIO_MOVE_0, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE2[0]);
    SET_TAG(AE_TAG_3HDR_OE2_RATIO_MOVE_1, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE2[1]);
    SET_TAG(AE_TAG_3HDR_OE2_RATIO_MOVE_2, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE2[2]);
    SET_TAG(AE_TAG_3HDR_OE2_RATIO_MOVE_3, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE2[3]);
    SET_TAG(AE_TAG_3HDR_OE2_RATIO_MOVE_4, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4RatioMovOE2[4]);

    SET_TAG(AE_TAG_3HDR_RATIO_STABLE, aeAlgoData->m_HdrAEInfo.sHDRRatioSmoothInfo.eHDRState);
    SET_TAG(AE_TAG_3HDR_RATIO_STABLE_CNT, aeAlgoData->m_HdrAEInfo.sHDRRatioSmoothInfo.i4HdrRatioStableCount);

    SET_TAG(AE_TAG_3HDR_RATIO_IN_THD, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4HDRRatioInTolerance);
    SET_TAG(AE_TAG_3HDR_RATIO_OUT_THD, aeAlgoData->m_HdrAEInfo.sHDRParaInfo.i4HDRRatioOutTolerance);

    //method for exposure transition from LE to SE/ME
    SET_TAG(AE_TAG_3HDR_3EXP_TRANS_METHOD, 0);
        
    for(i=0;i<64;i++)
    {
        SET_TAG(AE_TAG_3HDR_RSV_0+i, 0);
    }
    //3hdr end
    
    //===========================================================================
    LOG_DBG("[%s] AE_TAG_PRV_INDEX: %d", __FUNCTION__, aeAlgoData->m_u4Index); 
    LOG_DBG("[%s] AE_TAG_PRV_INDEXF: %d", __FUNCTION__, aeAlgoData->m_u4IndexF);
    LOG_DBG("[%s] AE_TAG_CWV: %d", __FUNCTION__, aeAlgoData->m_u4CWValue);
    LOG_DBG("[%s] AE_TAG_CWV_FINAL_TARGET: %d", __FUNCTION__, aeAlgoData->m_u4CWRecommend);
    LOG_DBG("[%s] AE_TAG_FACE_Y: %d", __FUNCTION__, aeAlgoData->m_MeterFDSts.m_u4FDY);
    //=============================================================================
    
#undef SET_TAG
    return;
}

void CcuCtrlAe::fillUpAeDebugData(AE_DEBUG_DATA_T *a_rAEDebugData, COMPAT_CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData)
{
    if (!a_rAEDebugData)
    {
        LOG_WRN("[%s] Debug data buff=null, skip fillUpAeDebugData", __FUNCTION__);
        return;
    }
    memset(a_rAEDebugData, 0, sizeof(AE_DEBUG_DATA_T));
    
    //===========================================================================
    a_rAEDebugData->u4AETableVer = AE_DEBUG_DATA_VERSION_DP;
    a_rAEDebugData->u4AETableSize = sizeof(AE_DEBUG_DATA_T);
    /*
    a_rAEDebugData->u4AAO_AWBValueWinSize8 = aeAlgoData->m_u4AAO_AWBValueWinSize8;
    a_rAEDebugData->u4AAO_AWBValueSepSize8 = aeAlgoData->m_u4AAO_AWBValueSepSize8;
    a_rAEDebugData->u4AAO_AWBSumWinSize8 = aeAlgoData->m_u4AAO_AWBSumWinSize8;
    a_rAEDebugData->u4AAO_AWBSumSepSize8 = aeAlgoData->m_u4AAO_AWBSumSepSize8;
    a_rAEDebugData->u4AAO_AEYWinSize8 = aeAlgoData->m_u4AAO_AEYWinSize8;
    a_rAEDebugData->u4AAO_AEYSepSize8 = aeAlgoData->m_u4AAO_AEYSepSize8;
    a_rAEDebugData->u4AAO_AEYWinSizeSE8 = aeAlgoData->m_u4AAO_AEYWinSizeSE8;
    a_rAEDebugData->u4AAO_AEYSepSizeSE8 = aeAlgoData->m_u4AAO_AEYSepSizeSE8;
    a_rAEDebugData->u4AAO_AEOverWinSize8 = aeAlgoData->m_u4AAO_AEOverWinSize8;
    a_rAEDebugData->u4AAO_AEOverSepSize8 = aeAlgoData->m_u4AAO_AEOverSepSize8;

    a_rAEDebugData->u4HIST_LEWinSize8 = aeAlgoData->m_u4HIST_LEWinSize8;
    a_rAEDebugData->u4HIST_LESepSize8 = aeAlgoData->m_u4HIST_LESepSize8;
    a_rAEDebugData->u4HIST_SEWinSize8 = aeAlgoData->m_u4HIST_SEWinSize8;
    a_rAEDebugData->u4HIST_SESepSize8 = aeAlgoData->m_u4HIST_SESepSize8;
    a_rAEDebugData->u4AAO_LineSize8 = aeAlgoData->m_u4AAO_LineSize8;

    a_rAEDebugData->u4AWB_Rounding = aeAlgoData->m_u4AWB_Rounding;
    a_rAEDebugData->u4AWB_ShiftBits = aeAlgoData->m_u4AWB_ShiftBits;
    a_rAEDebugData->u4Y_Rounding = aeAlgoData->m_u4Y_Rounding;
    a_rAEDebugData->u4Y_ShiftBits = aeAlgoData->m_u4Y_ShiftBits;
    //AE Block
    a_rAEDebugData->u4AEOverExpRatio = aeAlgoData->m_u4AEOverExpRatio;
    a_rAEDebugData->u4AEOverExpRatioR = aeAlgoData->m_u4AEOverExpRatioR;
    a_rAEDebugData->u4AEOverExpRatioG = aeAlgoData->m_u4AEOverExpRatioG;
    a_rAEDebugData->u4AEOverExpRatioB = aeAlgoData->m_u4AEOverExpRatioB;

    memcpy(a_rAEDebugData->pu4AEOverExpBlockCnt, aeAlgoData->m_pu4AEOverExpBlockCnt, sizeof(aeAlgoData->m_pu4AEOverExpBlockCnt));
    memcpy(a_rAEDebugData->pu4AEOverExpBlockCntR, aeAlgoData->m_pu4AEOverExpBlockCntR, sizeof(aeAlgoData->m_pu4AEOverExpBlockCntR));
    memcpy(a_rAEDebugData->pu4AEOverExpBlockCntG, aeAlgoData->m_pu4AEOverExpBlockCntG, sizeof(aeAlgoData->m_pu4AEOverExpBlockCntG));
    memcpy(a_rAEDebugData->pu4AEOverExpBlockCntB, aeAlgoData->m_pu4AEOverExpBlockCntB, sizeof(aeAlgoData->m_pu4AEOverExpBlockCntB));
    */
    memcpy(a_rAEDebugData->pu4AEBlock, aeAlgoData->m_pu4AEBlock, sizeof(aeAlgoData->m_pu4AEBlock));
    memcpy(a_rAEDebugData->pu4AEBlockSE, aeAlgoData->m_pu4AEBlockSE, sizeof(aeAlgoData->m_pu4AEBlockSE));
    memcpy(a_rAEDebugData->pu4AEv4p0Block, aeAlgoData->m_pu4AEv4p0Block, sizeof(aeAlgoData->m_pu4AEv4p0Block));
    memcpy(a_rAEDebugData->pu4AEv4p0BlockSE, aeAlgoData->m_pu4AEv4p0BlockSE, sizeof(aeAlgoData->m_pu4AEv4p0BlockSE));
    memcpy(a_rAEDebugData->pu4AEBlockR, aeAlgoData->m_pu4AEBlockR, sizeof(aeAlgoData->m_pu4AEBlockR));
    memcpy(a_rAEDebugData->pu4AEBlockG, aeAlgoData->m_pu4AEBlockG, sizeof(aeAlgoData->m_pu4AEBlockG));
    memcpy(a_rAEDebugData->pu4AEBlockB, aeAlgoData->m_pu4AEBlockB, sizeof(aeAlgoData->m_pu4AEBlockB));
    /*
    a_rAEDebugData->u4AEOverExpSE = aeAlgoData->m_u4AEOverExpSE;
    a_rAEDebugData->u4AEOverExpCntSE = aeAlgoData->m_u4AEOverExpCntSE;
    a_rAEDebugData->u4AETotalCntSE = aeAlgoData->m_u4AETotalCntSE;
    */
    memcpy(a_rAEDebugData->pu4AELieanrSortedY, aeAlgoData->m_pu4AELieanrSortedY, sizeof(aeAlgoData->m_pu4AELieanrSortedY));
    memcpy(a_rAEDebugData->pu4AELieanrSortedSEY, aeAlgoData->m_pu4AELieanrSortedSEY, sizeof(aeAlgoData->m_pu4AELieanrSortedSEY));
    //Folivora Matrix
    memcpy(a_rAEDebugData->m_u4FolivoraFMatrix, aeAlgoData->m_u4FolivoraFMatrix, sizeof(aeAlgoData->m_u4FolivoraFMatrix));
    
    memcpy(a_rAEDebugData->pu4CentralYHist, aeAlgoData->m_pu4CentralYHist, sizeof(aeAlgoData->m_pu4CentralYHist));
    memcpy(a_rAEDebugData->pu4FullYHist, aeAlgoData->m_pu4FullYHist, sizeof(aeAlgoData->m_pu4FullYHist));
    memcpy(a_rAEDebugData->pu4FullRGBHist, aeAlgoData->m_pu4FullRGBHist, sizeof(aeAlgoData->m_pu4FullRGBHist));
    /*
    memcpy(a_rAEDebugData->pu4FullRHist, aeAlgoData->m_pu4FullRHist, sizeof(aeAlgoData->m_pu4FullRHist));
    memcpy(a_rAEDebugData->pu4FullGHist, aeAlgoData->m_pu4FullGHist, sizeof(aeAlgoData->m_pu4FullGHist));
    memcpy(a_rAEDebugData->pu4FullBHist, aeAlgoData->m_pu4FullBHist, sizeof(aeAlgoData->m_pu4FullBHist));
    */
    memcpy(a_rAEDebugData->pu4CentralYHistSE, aeAlgoData->m_pu4CentralYHistSE, sizeof(aeAlgoData->m_pu4CentralYHistSE));
    memcpy(a_rAEDebugData->pu4FullYHistSE, aeAlgoData->m_pu4FullYHistSE, sizeof(aeAlgoData->m_pu4FullYHistSE));
    memcpy(a_rAEDebugData->pu4FullRGBHistSE, aeAlgoData->m_pu4FullRGBHistSE, sizeof(aeAlgoData->m_pu4FullRGBHistSE));
    /*
    memcpy(a_rAEDebugData->pu4FullRHistSE, aeAlgoData->m_pu4FullRHistSE, sizeof(aeAlgoData->m_pu4FullRHistSE));
    memcpy(a_rAEDebugData->pu4FullGHistSE, aeAlgoData->m_pu4FullGHistSE, sizeof(aeAlgoData->m_pu4FullGHistSE));
    memcpy(a_rAEDebugData->pu4FullBHistSE, aeAlgoData->m_pu4FullBHistSE, sizeof(aeAlgoData->m_pu4FullBHistSE));
    */
    //===========================================================================
    LOG_DBG("[%s] ae table ver:%d, size:%d", __FUNCTION__, a_rAEDebugData->u4AETableVer, a_rAEDebugData->u4AETableSize); 
    //=============================================================================
}


#define GET_CCU_OUTPUT_POLLING_SPEC 10000
bool CcuCtrlAe::getCcuAeOutput(struct ccu_ae_output *output)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return false;
    }

    if(output == NULL)
        LOG_WRN("[%s] output buffer form AeMgr is NULL, skip copying.", __FUNCTION__);

    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;

    volatile struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();

    uint32_t ridx;
    volatile uint32_t *is_ae_output_ready = &(sharedBufMap->is_ae_output_ready[m_ccuInstanceIdx]);

    if(sharedBufMap->is_ae_started[m_ccuInstanceIdx] == false)
    {
        LOG_WRN("[%s] si(%d), ii(%d) AE not started, skip", __FUNCTION__, m_sensorIdx, m_ccuInstanceIdx);
        return false;
    }

    double ts_start = CcuDrvUtil::getTimeUs();
    double ts_end = ts_start;
    double duration = 0;
    bool timeout = false;
    
    //while(*is_ae_output_ready == false)
    //{
    //    ts_end = CcuDrvUtil::getTimeUs();
    //    duration = ts_end - ts_start;
    //    if(duration > GET_CCU_OUTPUT_POLLING_SPEC)
    //    {
    //        timeout = true;
    //        break;
    //    }
    //}

    if(timeout)
    {
        LOG_ERR("%s timeout, duration_us(%lf)", __FUNCTION__, duration);
        return false;
    }

    ridx = sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_output_ridx;
    if((ridx < 0) || (ridx >= AE_ACTOUT_RING_SIZE))
    {
        LOG_DBG_MUST("[%s] si(%d), ii(%d), ridx(%d) invalid, skip", __FUNCTION__, m_sensorIdx, m_ccuInstanceIdx, ridx);
        return false;
    }

    //if(sharedBufMap->is_ae_instant_mode[m_ccuInstanceIdx] == false)
    //{
    //    *is_ae_output_ready = false;
    //}
    
    LOG_DBG("getCcuAeOutput si(%d), ridx(%d), aeBufferList(%p), instant(%d), started(%d), ready(%d)", m_sensorIdx, ridx, aeBufferList, 
        sharedBufMap->is_ae_instant_mode[m_ccuInstanceIdx],
        sharedBufMap->is_ae_started[m_ccuInstanceIdx],
        *is_ae_output_ready);

    LOG_DBG("getCcuAeOutput (%p)<-(%p)", output, aeBufferList->actoutAeOutput[ridx].getVa());

    memcpy(output, aeBufferList->actoutAeOutput[ridx].getVa(), sizeof(struct ccu_ae_output));
    LOG_DBG("%s done, duration_us(%lf), aeOutputVa(%p), aeOutputMVa(%p), ridx(%d), idxf(%d), stable(%d)", 
        __FUNCTION__, duration, aeBufferList->actoutAeOutput[ridx].getVa(), aeBufferList->actoutAeOutput[ridx].getMva(),
        ridx, output->algo_output.RealOutput.u4IndexF, output->algo_output.bAEStable);
    return true;
}

bool CcuCtrlAe::getCcuAeStat(CCU_AE_STAT *output)
{
    if(!this->m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe is not initialized yet, skip", __FUNCTION__);
        return false;
    }

    if(!ICcuMgrPriv::isCcuBooted())
    {
        LOG_WRN("[%s] CCU is not booted yet, skip", __FUNCTION__);
        return false;
    }

    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;
    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();

    // add polling for custom non-instant AE getting statistics
    volatile uint32_t *is_ae_stat_ready = &(sharedBufMap->is_ae_stat_ready[m_ccuInstanceIdx]);

    double ts_start = CcuDrvUtil::getTimeUs();
    double ts_end = ts_start;
    double duration = 0;

    while(*is_ae_stat_ready == false)
    {
        ts_end = CcuDrvUtil::getTimeUs();
        duration = ts_end - ts_start;
        if(duration > GET_CCU_OUTPUT_POLLING_SPEC) // timeout
        {
            //if timeout occur, may get previous statisics result, not return false
            LOG_ERR("[%s] timeout, duration_us(%lf)", __FUNCTION__, duration);
            break;
        }
    }

    uint32_t ridx = sharedBufMap->ae_actout_handle[m_ccuInstanceIdx].ae_stat_ridx;

    if((ridx < 0) || (ridx >= AE_ACTOUT_RING_SIZE))
    {
        LOG_DBG_MUST("[%s] ridx:%d invalid, skip", __FUNCTION__, ridx);
        return false;
    }

    memcpy(output, aeBufferList->actoutAeStat[ridx].getVa(), sizeof(CCU_AE_STAT));
    LOG_DBG("aeOutputVa(%p), ridx(%d)", aeBufferList->actoutAeStat[ridx].getVa(), ridx);
    return true;
}


enum ccu_feature_type CcuCtrlAe::_getFeatureType()
{
    return CCU_FEATURE_AE;
}

bool CcuCtrlAe::ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_SENSOR_INIT:
        {
            result = ccuCtrlPreprocess_SensorInit(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_AE_ALGO_INIT:
        {
            result = ccuCtrlPreprocess_AeAlgoInit(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME:
        {
            result = ccuCtrlPreprocess_PerframeData(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE:
        {
            result = ccuCtrlPreprocess_OnchangeData(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_HDR_AE_INPUT_READY:
        {
            result = ccuCtrlPreprocess_HdrInputDataReady(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_AE_STOP:
        {
            result = ccuCtrlPreprocess_CcuAeStop(inDataPtr, inDataBuf);
            break;
        }
        default : break;
    }
    return result;
}

bool CcuCtrlAe::ccuCtrlPostprocess(uint32_t msgId, void *outDataPtr, void *inDataBuf)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_SENSOR_INIT:
        {
            result = ccuCtrlPostprocess_SensorInit(outDataPtr, inDataBuf);
            break;
        }
        default : break;
    }
    return result;
}

/*******************************************************************************
* Private Functions
********************************************************************************/
bool CcuCtrlAe::ccuCtrlPreprocess_AeAlgoInit(void *inDataPtr, void *inDataBuf)
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();

    LOG_DBG_MUST("param1 size: %d", LumLog2x1000_TABLE_SIZE * sizeof(MUINT32));
    LOG_DBG_MUST("param2 size: %d", CCU_LIB3A_AE_EV_COMP_MAX * sizeof(MUINT32));
    LOG_DBG_MUST("param3 size: %d", sizeof(CCU_strEvPline));
    LOG_DBG_MUST("param4 size: %d", sizeof(CCU_AE_NVRAM_T));


    CCU_AE_ALGO_INITI_PARAM_T *input = (CCU_AE_ALGO_INITI_PARAM_T *)inDataPtr;
    CCU_AE_ALGO_INITI_PARAM_T *initParam = (CCU_AE_ALGO_INITI_PARAM_T *)inDataBuf;

    LOG_DBG_MUST("XDBG_inDataBuf: %p", inDataBuf);    //save data ptr for exif dump using
    m_CcuAeExifDataPtr.m_LumLog2x1000 = input->algo_init_param.m_LumLog2x1000;
    m_CcuAeExifDataPtr.pCurrentTableF = input->algo_init_param.pCurrentTableF;
    m_CcuAeExifDataPtr.pCurrentTable = input->algo_init_param.pCurrentTable;
    m_CcuAeExifDataPtr.pEVValueArray = input->algo_init_param.pEVValueArray;
    m_CcuAeExifDataPtr.pAETouchMovingRatio = input->algo_init_param.pAETouchMovingRatio;
    m_CcuAeExifDataPtr.pAeNVRAM = input->algo_init_param.pAeNVRAM;

    LOG_DBG("+:%s\n",__FUNCTION__);

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_PerframeData(void *inDataPtr, void *inDataBuf)
{
    ccu_ae_ctrldata_perframe *perframeDataIn = (ccu_ae_ctrldata_perframe *)inDataPtr;
    LOG_DBG("CCU perframeData: magic(%d), req(%d)", perframeDataIn->magic_num, perframeDataIn->req_num);

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_OnchangeData(void *inDataPtr, void *inDataBuf)
{
    AE_CORE_CTRL_RUN_TIME_INFO *onchangeDataIn = (AE_CORE_CTRL_RUN_TIME_INFO *)inDataPtr;
    COMPAT_AE_CORE_CTRL_RUN_TIME_INFO *onchangeDataInCompat = (COMPAT_AE_CORE_CTRL_RUN_TIME_INFO *)inDataBuf;

    //save data ptr for exif dump using
    m_CcuAeExifDataPtr.pCurrentTable = onchangeDataIn->pCurrentTable;
    m_CcuAeExifDataPtr.pCurrentTableF = onchangeDataIn->pCurrentTableF;
    m_CcuAeExifDataPtr.pAeNVRAM = onchangeDataIn->pAeNVRAM;

    LOG_DBG("pCurrentTable: va(%p), mva(%x)", onchangeDataIn->pCurrentTable, onchangeDataInCompat->pCurrentTable);
    LOG_DBG("pCurrentTableF: va(%p), mva(%x)", onchangeDataIn->pCurrentTableF, onchangeDataInCompat->pCurrentTableF);
    LOG_DBG("pAeNVRAM: va(%p), mva(%x)", onchangeDataIn->pAeNVRAM, onchangeDataInCompat->pAeNVRAM);

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_SensorInit(void *inDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    uint8_t sensor_slot;
    SENSOR_INFO_IN_T *infoIn = (SENSOR_INFO_IN_T *)inDataPtr;
    struct ccu_i2c_buf_mva_ioarg i2cBufIoArg;
    int32_t currentFpsList[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    int32_t sensorI2cSlaveAddr[IMGSENSOR_SENSOR_IDX_MAX_NUM] = {-1};
    MUINT32 sensor_nm_idx = CcuDrvUtil::sensorIdxToNonmapIdx(m_sensorIdx);

    //======== Set i2c i2cControllerId of cam type to make kernel driver select correct i2c driver =========
    LOG_INF_MUST("Sensor Type: %d map %d\n", m_sensorIdx, sensor_nm_idx);
    i2cBufIoArg.sensor_idx = sensor_nm_idx;
    //======== Get i2c dma buffer mva =========
    LOG_DBG_MUST("skip get i2c bufaddr\n");
    if (!m_pDrvCcu->getI2CDmaBufferAddr(&i2cBufIoArg))
    {
        LOG_ERR("getI2CDmaBufferAddr fail \n");
        m_pDrvCcu->TriggerAee("getI2CDmaBufferAddr failed");
        return false;
    }

    if ((m_sensorIdx == IMGSENSOR_SENSOR_IDX_MAIN) || (m_sensorIdx == IMGSENSOR_SENSOR_IDX_SUB))
    {
        i2cBufIoArg.mva+=PAGE_SIZE;
        i2cBufIoArg.va_l+=PAGE_SIZE;
    }

    LOG_DBG("i2c_get_dma_buffer_addr: %x\n" , i2cBufIoArg.mva);
    LOG_DBG("i2c_get_dma_buffer_va_h: %x\n" , i2cBufIoArg.va_h);
    LOG_DBG("i2c_get_dma_buffer_va_l: %x\n" , i2cBufIoArg.va_l);
    LOG_DBG("i2c_get_dma_buffer_i2c_id:%x\n", i2cBufIoArg.i2c_id);

    //======== Get current fps =========
    if (!m_pDrvCcu->getCurrentFps(currentFpsList))
    {
        LOG_ERR("getCurrentFps fail \n");
        m_pDrvCcu->TriggerAee("getCurrentFps failed");
        return false;
    }
    for (int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM ; i++)
    {
        LOG_DBG("currentFpsList[%d]: %d\n", i, currentFpsList[i]);
    }

    //======== Get Sensor I2c Slave Address =========
    if (!m_pDrvCcu->getSensorI2cSlaveAddr(&sensorI2cSlaveAddr[0]))
    {
        LOG_ERR("getSensorI2cSlaveAddr fail \n");
        m_pDrvCcu->TriggerAee("getSensorI2cSlaveAddr failed");
        return false;
    }

    LOG_DBG("[CCU If]Main  Sensor slave addr : %d\n", sensorI2cSlaveAddr[0]);
    LOG_DBG("[CCU If]Sub   Sensor slave addr : %d\n", sensorI2cSlaveAddr[1]);
    LOG_DBG("[CCU If]Main2 Sensor slave addr : %d\n", sensorI2cSlaveAddr[2]);
    LOG_DBG("[CCU If]Main3 Sensor slave addr : %d\n", sensorI2cSlaveAddr[3]);
    LOG_DBG("[CCU If]Sub2  Sensor slave addr : %d\n", sensorI2cSlaveAddr[4]);

    bool slaveAddrValid = true;

    if(sensorI2cSlaveAddr[sensor_nm_idx] == -1)
        slaveAddrValid = false;

    if(slaveAddrValid == false)
    {
        for (int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM ; i++)
        {
            LOG_ERR("[CCU If]Sensor %d slave addr : %d\n", i, currentFpsList[i]);
        }
        m_pDrvCcu->TriggerAee("CCU Sensor slave addr error");
    }

    if (!m_pDrvCcu->allocSensorSlot(m_sensorIdx, &sensor_slot)){
        LOG_WRN("allocSensorSlot fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }
    //======== Call CCU to initial sensor drv =========
    //Convert to compatible structure
    COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)inDataBuf;

    compatInfoIn->u16FPS = currentFpsList[sensor_nm_idx];
    compatInfoIn->sensorI2cSlaveAddr = sensorI2cSlaveAddr[sensor_nm_idx];
    compatInfoIn->eScenario    = infoIn->eScenario;
    compatInfoIn->dma_buf_mva  = i2cBufIoArg.mva;
    compatInfoIn->dma_buf_va_h = i2cBufIoArg.va_h;
    compatInfoIn->dma_buf_va_l = i2cBufIoArg.va_l;
    compatInfoIn->u32I2CId     = i2cBufIoArg.i2c_id;
    compatInfoIn->sensor_slot  = sensor_slot;
    compatInfoIn->isSpSensor   = m_isSpSensor;

    //LOG_DBG("ccu sensor in, compatInfoIn->u32SensorId: %x\n", compatInfoIn->u32SensorId);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->u16FPS: %x\n", compatInfoIn->u16FPS);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->eScenario: %x\n", compatInfoIn->eScenario);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->dma_buf_mva: %x\n", compatInfoIn->dma_buf_mva);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->dma_buf_va_h: %x\n", compatInfoIn->dma_buf_va_h);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->dma_buf_va_l: %x\n", compatInfoIn->dma_buf_va_l);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->u32I2CId: %x\n", compatInfoIn->u32I2CId);
    LOG_DBG_MUST("ccu sensor in, compatInfoIn->isSpSensor: %x\n", compatInfoIn->isSpSensor);
    LOG_DBG("ccu sensor in, compatInfoIn->sensorI2cSlaveAddr : %x\n", compatInfoIn->sensorI2cSlaveAddr);

    //check if need to request frequncey change
    if(compatInfoIn->u16FPS == 600)
        m_pDrvCcu->RequestCamFreqUpdate(CCU_REQ_CAM_FREQ_MID);

    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}

bool CcuCtrlAe::ccuCtrlPostprocess_SensorInit(void *outDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)inDataBuf;
    SENSOR_INFO_OUT_T *infoOut = (SENSOR_INFO_OUT_T *)outDataPtr;

    LOG_DBG_MUST("ccu sensor out, u8SupportedByCCU: %x\n", infoOut->u8SupportedByCCU);

    if(infoOut->u8SupportedByCCU == 0)
    {
        LOG_ERR("ccu not support sensor\n");
        m_pDrvCcu->TriggerAee("ccu not support sensor");
        return false;
    }

    //tell AeMgr if current sensor is special sensor to bypass longexp on CCU
    infoOut->isSpSensor = m_isSpSensor;

    //Init i2c controller, call i2c driver to do initialization

    LOG_DBG_MUST("skip i2c init\n");
    if (!m_pDrvCcu->initI2cController(compatInfoIn->u32I2CId))
    {
        LOG_ERR("initI2cController fail \n");
        m_pDrvCcu->TriggerAee("initI2cController failed");
        return false;
    }
    LOG_DBG("-:%s\n",__FUNCTION__);

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_HdrInputDataReady(void *inDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    struct ccu_hdr_ae_input_data *inputData = (struct ccu_hdr_ae_input_data *)inDataBuf;
    int ionDevFd = 0;
    CcuMemMgr *memMgr = CcuMemMgr::getInstance();
    if (memMgr != NULL)
    {
        ionDevFd = memMgr->getIonDevFd();
    }

    CcuAeBufferList *aeBufferList = (CcuAeBufferList *)m_pBufferList;
    for(int i=0 ; i<CCU_HDR_DATA_BUF_CNT ; i++)
    {
        if (inputData->hdr_data_buffer_mva == aeBufferList->hdrDataBuffer[i].getMva())
        {
            ion_cache_sync_flush_range_va(ionDevFd, (char*)aeBufferList->hdrDataBuffer[i].getVa(), HDR_SENSOR_DATA_SIZE);
        }
    }

    LOG_DBG_MUST("CCU HdrInputDataReady, bufferMva(%x)\n",inputData->hdr_data_buffer_mva);

    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_CcuAeStop(void *inDataPtr, void *inDataBuf)
{
    m_pDrvCcu->RequestCamFreqUpdate(CCU_REQ_CAM_FREQ_NONE);
    return true;
}

};  //namespace NSCcuIf
