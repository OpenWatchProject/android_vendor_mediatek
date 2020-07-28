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
static MUINT32 _lastAEIndexF[CCU_CAM_TG_CNT] = {0};

/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrlAe *ICcuCtrlAe::createInstance()
{
	return new CcuCtrlAe();
}

/*******************************************************************************
* Public Functions
********************************************************************************/
//this variable indicates if current senosr is s5k3p9 used as main camera
int CcuCtrlAe::init(MUINT32 sensorIdx, ESensorDev_T sensorDev)
{
	LOG_DBG("+:%s\n",__FUNCTION__);
    uint8_t sensor_slot;

	int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
	if(ret != CCU_CTRL_SUCCEED)
	{
		return ret;
	}
	   
    //sensor_slot: 0 => 60~62 / 30~31, 1 => 62~64 / 31~32
    enum ccu_tg_info tg_info = getCcuTgInfo();

    if (tg_info == CCU_CAM_TG_1)
        sensor_slot = 1;
    else 
        sensor_slot = 2;

	if (!m_pDrvCcu->loadSensorBin(sensorIdx, sensor_slot, &m_isSpSensor))
    {
        LOG_WRN("loadSensorBin fail \n");
        return -CCU_CTRL_ERR_GENERAL;
    }

    LOG_DBG("-:0x%x:%d:%s\n", m_isSpSensor, sensor_slot,__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

void CcuCtrlAe::destroyInstance(void)
{
	LOG_DBG("+:%s\n",__FUNCTION__);
	delete this;
	LOG_DBG("-:%s\n",__FUNCTION__);
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/

void CcuCtrlAe::getAeInfoForIsp(AE_INFO_T *a_rAEInfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    if(!m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe not initialized\n", __FUNCTION__);
        return;
    }
    LOG_DBG("[%s] check initialized ok\n", __FUNCTION__);

    enum ccu_tg_info tg_info = getCcuTgInfo();
    struct shared_buf_map * sharedBufMap = m_pDrvCcu->getSharedBufMap();
    CCU_AeAlgo *aeAlgoDataPtr = NULL;
    AE_CORE_INIT *aeInitDataPtr = NULL;

    aeAlgoDataPtr = (CCU_AeAlgo *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].ae_algo_data_addr);
    aeInitDataPtr = (AE_CORE_INIT *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].ae_init_data_addr);

    if((aeAlgoDataPtr == NULL) || (aeInitDataPtr == NULL))
    {
        LOG_ERR("AE algo dataPtrs, skip fillUpAeInfo");
        return;
    }

    fillUpAeInfo(a_rAEInfo, aeAlgoDataPtr, aeInitDataPtr);
    UTIL_TRACE_END();
}

void CcuCtrlAe::fillUpAeInfo(AE_INFO_T *rAEISPInfo, CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData)
{
    MUINT32 i;
    //strAEOutput *pAESetting;

    //switch (eSensorMode)
    //{
    //    default:
    //    case LIB3A_SENSOR_MODE_PRVIEW:
    //        pAESetting = &m_PrvStateAESetting;
    //        break;
    //    case LIB3A_SENSOR_MODE_CAPTURE:
    //    case LIB3A_SENSOR_MODE_CAPTURE_ZSD:
    //        pAESetting = &m_CaptureAESetting;
    //        break;
    //}

    rAEISPInfo->u4AETarget = aeAlgoData->m_u4AETarget;
    rAEISPInfo->u4AECurrentTarget = aeAlgoData->m_u4CWRecommend;
    rAEISPInfo->u4AECondition = aeAlgoData->m_u4AECondition;
    rAEISPInfo->eAEMeterMode = (LIB3A_AE_METERING_MODE_T)aeAlgoData->m_AECmdSet.eAEMeterMode;
    rAEISPInfo->i2FlareOffset = aeAlgoData->m_u4Prvflare;
    rAEISPInfo->u4MaxISO = 800;
    rAEISPInfo->i4LightValue_x10 = aeAlgoData->m_i4BV+50;//pAESetting->Bv + 50;
    rAEISPInfo->i4RealLightValue_x10 = aeAlgoData->m_i4RealBV+50;   //real LV
    rAEISPInfo->u4Eposuretime = aeAlgoData->m_PrvStateAESetting.EvSetting.u4Eposuretime;
    rAEISPInfo->u4AfeGain = aeAlgoData->m_PrvStateAESetting.EvSetting.u4AfeGain;
    rAEISPInfo->u4IspGain = aeAlgoData->m_PrvStateAESetting.EvSetting.u4IspGain;
    rAEISPInfo->u4RealISOValue = aeAlgoData->m_PrvStateAESetting.u4ISO;
    rAEISPInfo->u4OrgExposuretime = aeAlgoData->m_PrvStateAESetting.EvSetting.u4Eposuretime;
    rAEISPInfo->u4AEidxNext = aeAlgoData->m_u4Index;
    rAEISPInfo->u4AEidxNextF = aeAlgoData->m_u4IndexF;
    rAEISPInfo->u4AEidxCurrent = aeAlgoData->m_u4Index;
    rAEISPInfo->u4AEidxCurrentF = aeAlgoData->m_u4IndexF;
    //getAERealISOvalue(pAESetting);
    rAEISPInfo->u4OrgRealISOValue = aeAlgoData->m_PrvStateAESetting.u4ISO;//m_PrvStateAESetting.EvSetting.u4AfeGain;

    for(i=0; i<AE_HISTOGRAM_BIN; i++)
    {
        rAEISPInfo->u4Histogrm[i] = aeAlgoData->m_pu4FullYHist[i];
    }

    //updateHdrInfo(rAEISPInfo.rHdrAEInfo);
    //rAEISPInfo.i4HdrExpoDiffThr = m_pAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.rAutoTuning.i4RmmExpoDiffThr;
    // for shading smooth start
    rAEISPInfo->bEnableRAFastConverge = MTRUE;
    rAEISPInfo->bAEStable = aeAlgoData->m_bAEStable;
    rAEISPInfo->i4deltaIndex = aeAlgoData->m_i4deltaIndex;
    rAEISPInfo->u4MgrCWValue = aeAlgoData->m_u4MgrCWValue;
    rAEISPInfo->u4AvgWValue = aeAlgoData->m_u4AvgWValue;
    rAEISPInfo->u4WeightingSum = aeAlgoData->m_u4WeightingSum;
    rAEISPInfo->TgCtrlRight = 0;
    rAEISPInfo->i4EVRatio = aeAlgoData->m_LinearResp.i4EVRatio;
    rAEISPInfo->i4FrameCnt = aeAlgoData->m_i4FrameCnt;
    rAEISPInfo->u4AEFinerEVIdxBase = aeAlgoData->m_u4AEFinerEVIdxBase;

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
    rAEISPInfo->u4FaceAEStable = aeAlgoData->m_u4FaceAEStable;
    rAEISPInfo->i4Crnt_FDY = aeAlgoData->m_i4Crnt_FDY;
    if(m_CcuAeExifDataPtr.pAeNVRAM->rHistConfig.u4HistHighThres == 0)
    {
        rAEISPInfo->u4MeterFDTarget = aeAlgoData->m_u4MeterFDTarget;
    }
    else
    {
        rAEISPInfo->u4MeterFDTarget = aeAlgoData->m_u4RealFaceCWR;
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

    rAEISPInfo->i4AEComp = (aeAlgoData->m_AECmdSet.eAEComp > LIB3A_AE_EV_COMP_40) ?  
                            ((aeAlgoData->m_AECmdSet.eAEComp - LIB3A_AE_EV_COMP_40) * -1) : (aeAlgoData->m_AECmdSet.eAEComp);

    return;

}

void CcuCtrlAe::getDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo)
{
    UTIL_TRACE_BEGIN(__FUNCTION__);

    if(!m_initialized)
    {
        LOG_WRN("[%s] CcuCtrlAe not initialized\n", __FUNCTION__);
        return;
    }

    enum ccu_tg_info tg_info = getCcuTgInfo();
    struct shared_buf_map * sharedBufMap = m_pDrvCcu->getSharedBufMap();
    CCU_AeAlgo *aeAlgoDataPtr = NULL;
    AE_CORE_INIT *aeInitDataPtr = NULL;
    AE_CORE_CTRL_CCU_VSYNC_INFO *aeVsyncInfoPtr = NULL;

    aeAlgoDataPtr = (CCU_AeAlgo *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].ae_algo_data_addr);
    aeInitDataPtr = (AE_CORE_INIT *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].ae_init_data_addr);
    aeVsyncInfoPtr = (AE_CORE_CTRL_CCU_VSYNC_INFO *)m_pDrvCcu->ccuAddrToVa(sharedBufMap->exif_data_addrs[CCU_TG2IDX(tg_info)].ae_vsync_info_addr);

    LOG_DBG("AE algo dataPtrs, aeAlgoDataPtr(%p), aeInitDataPtr(%p), aeVsyncInfoPtr(%p)", aeAlgoDataPtr, aeInitDataPtr, aeVsyncInfoPtr);

    MUINT32 *dumpPtr = (MUINT32 *)aeAlgoDataPtr;
    //for(int i=0 ; i<sizeof(CCU_AeAlgo) ; i += 16)
    //{
    //    LOG_WRN("kfd_%08x: 0x%08x ,0x%08x ,0x%08x ,0x%08x\n", i, *(dumpPtr), *(dumpPtr+1), *(dumpPtr+2), *(dumpPtr+3));
    //    dumpPtr += 4;
    //}

    if((aeAlgoDataPtr == NULL) || (aeInitDataPtr == NULL) || (aeVsyncInfoPtr == NULL))
    {
        LOG_ERR("AE algo dataPtrs, skip fillUpAeDebugInfo");
        return;
    }

    fillUpAeDebugInfo(a_rAEDebugInfo, aeAlgoDataPtr, aeInitDataPtr, aeVsyncInfoPtr);
    UTIL_TRACE_END();
    return;
}

void CcuCtrlAe::fillUpAeDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, CCU_AeAlgo *aeAlgoData, AE_CORE_INIT *aeInitData, AE_CORE_CTRL_CCU_VSYNC_INFO *aeVsyncInfo)
{
    int start;
    int end;
    int diffCnt;

    MINT32 i=0,j=0,i4flareidx=0;
    MUINT32 u4packedFlareWT = 0, u4packedRGBY=0;

    // AE Debug Info
    memset(a_rAEDebugInfo, 0, sizeof(AE_DEBUG_INFO_T));


    // LOG_DBG("dbgeric aealgodata1 = %d\n", aeAlgoData->m_u4IndexF);
    // LOG_DBG("dbgeric aealgodata1 = %d\n", aeAlgoData->m_MeterFDSts.m_u4FaceNum);
    // LOG_DBG("dbgeric aeaInitdata1 = %d\n", m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Lock_MaxCnt);
    // LOG_DBG("dbgeric aeaInitdata2 = %d\n", m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.i4BVOffset);
    // LOG_DBG("dbgeric aeaVsyncdata1 = %d\n", aeVsyncInfo->m_AEWinBlock.u4XHi);
    // LOG_DBG("dbgeric AE_DEBUG_TAG_SUBVERSION = %d\n", AE_DEBUG_TAG_SUBVERSION);
    // LOG_DBG("dbgeric AE_DEBUG_TAG_VERSION = %d\n", AE_DEBUG_TAG_VERSION);
/*
    setDebugTag(a_rAEDebugInfo, AE_TAG_DEBUG_VERSION, (((MUINT32)AE_DEBUG_TAG_SUBVERSION << 16) | AE_DEBUG_TAG_VERSION));
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_0, AE_ALGO_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_1, AE_PARAM_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_2, AE_ALGO_IF_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_3, DBG_AE_PARAM_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_4, AE_ALGO_TOOL_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_REALBVX1000, 5000);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_INDEX,  123);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_INDEXF, 456);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BV_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.i4BVOffset);
*/
    

    // AE tag version
    setDebugTag(a_rAEDebugInfo, AE_TAG_DEBUG_VERSION, (((MUINT32)AE_DEBUG_TAG_SUBVERSION << 16) | AE_DEBUG_TAG_VERSION));
    setDebugTag(a_rAEDebugInfo, AE_TAG_AE_MODE, aeAlgoData->m_AECmdSet.eAEScene);
    // Output AE brightness information
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_INDEX,  aeAlgoData->m_u4Index);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_INDEXF, aeAlgoData->m_u4IndexF);

    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_SHUTTER_TIME, aeAlgoData->m_PrvStateAESetting.EvSetting.u4Eposuretime);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_SENSOR_GAIN, aeAlgoData->m_PrvStateAESetting.EvSetting.u4AfeGain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_ISP_GAIN, aeAlgoData->m_PrvStateAESetting.EvSetting.u4IspGain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_ISO_REAL, aeAlgoData->m_PrvStateAESetting.u4ISO);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_FRAME_RATE, aeAlgoData->m_PrvStateAESetting.u2FrameRate);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_FLARE, aeAlgoData->m_PrvStateAESetting.i2FlareOffset);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_FLARE_GAIN, aeAlgoData->m_PrvStateAESetting.i2FlareGain);


    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_PLINE_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAFPLine.bAFPlineEnable);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_ZSD_PLINE_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAFZSDPLine.bAFPlineEnable);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_SHUTTER_TIME, m_AFStateAESetting.EvSetting.u4Eposuretime);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_SENSOR_GAIN, m_AFStateAESetting.EvSetting.u4AfeGain);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_ISP_GAIN, m_AFStateAESetting.EvSetting.u4IspGain);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_ISO_REAL, m_AFStateAESetting.u4ISO);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_FRAME_RATE, m_AFStateAESetting.u2FrameRate);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_FLARE, m_AFStateAESetting.i2FlareOffset);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_AF_FLARE_GAIN, m_AFStateAESetting.i2FlareGain);

    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_INDEX,  m_u4CaptureIdx);

    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_SHUTTER_TIME, m_CaptureAESetting.EvSetting.u4Eposuretime);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_SENSOR_GAIN, m_CaptureAESetting.EvSetting.u4AfeGain);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_ISP_GAIN, m_CaptureAESetting.EvSetting.u4IspGain);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_ISO_REAL, m_CaptureAESetting.u4ISO);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_FRAME_RATE, m_CaptureAESetting.u2FrameRate);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_FLARE, m_CaptureAESetting.i2FlareOffset);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_FLARE_GAIN, m_CaptureAESetting.i2FlareGain);

    // setDebugTag(a_rAEDebugInfo, AE_TAG_STROBE_PLINE_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strStrobePLine.bAFPlineEnable);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_STROBE_ZSD_PLINE_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strStrobeZSDPLine.bAFPlineEnable);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FRAC_GAIN, aeAlgoData->m_u4FracGain);

    //Pline Info

    //setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_PLINE_ID,(m_pPreviewTableCurrent != NULL)?m_pPreviewTableCurrent->eID:AETABLE_SCENE_MAX);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_PLINE_ID,(m_pCaptureTable != NULL)?m_pCaptureTable->eID:AETABLE_SCENE_MAX);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_INDEXBASE, aeAlgoData->m_u4AEFinerEVIdxBase);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MIN_INDEX, aeInitData->m_u4IndexMin);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MAX_INDEX, aeInitData->m_u4IndexFMax);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MIN_INDEXF, aeAlgoData->m_u4IndexFMin);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MAX_INDEXF, aeAlgoData->m_u4IndexFMax);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_PLINE_MAX_BV, aeInitData->i4MaxBV);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_PLINE_MIN_BV,(aeAlgoData->m_pCurrentTableF->sPlineTable != NULL)?aeInitData->i4MinBV:0); //???
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_PLINE_MAX_BV,(m_pCaptureTable != NULL)?m_pCaptureTable->i4MaxBV:0);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_PLINE_MIN_BV,(m_pCaptureTable != NULL)?m_pCaptureTable->i4MinBV:0);

    setDebugTag(a_rAEDebugInfo, AE_TAG_EV_DIFF_FRAC, aeAlgoData->m_i4EvDiffFrac);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_DELTA_INDEX, aeAlgoData->m_i4deltaIndex);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_EV_RATIO,aeAlgoData->m_LinearResp.i4EVRatio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PERFRAME_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bPerFrameAESmooth);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NOT_PERFRAME_EN, aeAlgoData->m_bAENotPerFrameMode);

    // AE setting
    // setDebugTag(a_rAEDebugInfo, AE_TAG_ZSD_EN, (m_pCaptureTable != NULL && m_pCaptureTable->eID == AETABLE_SCENE_INDEX5)?1:0);
    setDebugTag(a_rAEDebugInfo, AE_TAG_REALISO_EN, aeAlgoData->m_AECmdSet.bIsoSpeedReal);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_APERTURE, m_AeDevicesInfo.u4LensFno);


    // if(m_pPreviewTableCurrent != NULL)
    // {
    //     setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_AETABLE_ID, m_pPreviewTableCurrent->eID);
    // }

    // if((m_u4CaptureIdx == 0xFFFFFFFF) || (m_pCaptureTable == NULL))
    // {
    //     setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_AETABLE_ID, 0);
    // }
    // else
    // {
    //     setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_AETABLE_ID, m_pCaptureTable->eID);
    // }

    //setDebugTag(a_rAEDebugInfo, AE_TAG_LV, (m_CaptureAESetting.Bv + 50));
    setDebugTag(a_rAEDebugInfo, AE_TAG_REALBV, aeAlgoData->m_i4RealBV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_REALBVX1000, aeAlgoData->m_i4RealBVx1000);
    setDebugTag(a_rAEDebugInfo, AE_TAG_COMPBV, aeAlgoData->m_i4AoeCompBV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BV_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.i4BVOffset);

    setDebugTag(a_rAEDebugInfo, AE_TAG_AE_SCENE,aeAlgoData->m_AECmdSet.eAEScene);
    setDebugTag(a_rAEDebugInfo, AE_TAG_EV_COM, aeAlgoData->m_AECmdSet.eAEComp);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_EV_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.EVValueArray[aeAlgoData->m_AECmdSet.eAEComp]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLICKER, aeAlgoData->m_AECmdSet.eAEFlickerMode);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLICKER_AUTO, aeAlgoData->m_AECmdSet.eAEAutoFlickerMode);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AEMETER, aeAlgoData->m_AECmdSet.eAEMeterMode);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ISO_SETTING, aeAlgoData->m_AECmdSet.u4AEISOSpeed);

    // device info
    setDebugTag(a_rAEDebugInfo, AE_TAG_MINI_GAIN, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4MinGain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MAX_GAIN, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4MaxGain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_1XGAIN_ISO, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4MiniISOGain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_EXP_UINT, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4PreExpUnit);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_EXP_UINT, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4VideoExpUnit);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_PRV_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4Video2PreRatio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_EXP_UINT, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4CapExpUnit);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_PRV_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rDevicesInfo.u4Cap2PreRatio);

    // AE algorithm enable and condition check
    //FLARE
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_FALRE_CAPTURE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.bEnableCaptureThres);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_FLARE_VIDEO_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.bEnableVideoThres);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_FLARE_STROBE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.bEnableStrobeThres);

    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4PreviewFlareOffset);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4CaptureFlareOffset);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4CaptureFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoFlareOffset);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STROBE_FLARE_OFFSET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4StrobeFlareOffset);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STROBE_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4StrobeFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_MAX_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4PrvMaxFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CAP_MIN_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4PrvMinFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_MAX_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoMaxFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_MIN_FLARE_THRES, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4VideoMinFlareThres);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLARE_STD_THR_HIGH, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareStdThrHigh);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLARE_STD_THR_LOW, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareStdThrLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_CAP_FLARE_DIFF, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4PrvCapFlareDiff);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLARE_MAX_STEP_GAP_FAST, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareMaxStepGap_Fast);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLARE_MAX_STEP_GAP_SLOW, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareMaxStepGap_Slow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLARE_MAX_STEP_GAP_LIMIT_BV, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlarMaxStepGapLimitBV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FLARE_AE_STABLE_COUNT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.u4FlareAEStableCount);
    //#ifdef FLARE
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_STD, aeAlgoData->m_i4FlareSTD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_COUNT_R, aeAlgoData->m_i4FlareCountR);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_COUNT_G, aeAlgoData->m_i4FlareCountG);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_COUNT_B, aeAlgoData->m_i4FlareCountB);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_PIXEL_VALUE, aeAlgoData->m_i4FlareFlarePixelValue);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_FAST_CONVERGE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableFlareFastConverge);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_DST_VALUE, aeAlgoData->m_u4SW_FlareValue);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SW_FLARE_APPLY_VALUE, aeAlgoData->m_u4Prvflare);

    u4packedFlareWT = 0;
    for(i=0; i<8; i++)
    {
        u4packedFlareWT = u4packedFlareWT |(m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.uPrvFlareWeightArr[i]<<(i*4));
    }
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_FALRE_WEIGHT0_7,u4packedFlareWT);

    u4packedFlareWT = 0;
    for(i=8; i<16; i++)
    {
        u4packedFlareWT = u4packedFlareWT |(m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.uPrvFlareWeightArr[i]<<((i-8)*4));
    }
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_FALRE_WEIGHT8_15,u4packedFlareWT);

    u4packedFlareWT = 0;
    for(i=0; i<8; i++)
    {
        u4packedFlareWT = u4packedFlareWT |(m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.uVideoFlareWeightArr[i]<<(i*4));
    }
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_FALRE_WEIGHT0_7,u4packedFlareWT);

    u4packedFlareWT = 0;
    for(i=8; i<16; i++)
    {
        u4packedFlareWT = u4packedFlareWT |(m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rFlareSpec.uVideoFlareWeightArr[i]<<((i-8)*4));
    }
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_FALRE_WEIGHT8_15,u4packedFlareWT);
    //#endif
    // setDebugTag(a_rAEDebugInfo, AE_TAG_HIST_INFO_FULL_BRIGHTEST, m_sHistInfo.u4FullBrightest);

    // LSC
    // setDebugTag(a_rAEDebugInfo, AE_TAG_LSC_FAST_CONVERGE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableRAFastConverge);

    // HS
    setDebugTag(a_rAEDebugInfo, AE_TAG_HISTSTRETCH_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_HIST_STRETCH)?1:0);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_HISTSTRETCH_EN,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.bEnableHistStretch);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HISTSTRETCH_WEIGHTING, aeAlgoData->m_TargetSts.Wet_HS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HISTSTRETCH_METERING, aeAlgoData->m_u4HistoStretchCWM );
    //#ifdef INIT_PARAM
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_ENABLE,         m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.bEnableHistStretch);          //bEnableHistStretch
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_WEIGHT,         m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4HistStretchWeight);      //u4HistStretchWeight
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_PCENT,          m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4Pcent);      //u4Pcent
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_THD,            m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4Thd);      //u4Thd;                       // 0~255
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_FLATTHD,        m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatThd);      //u4FlatThd;                   // 0~255
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_BRIGHTPCENT,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatBrightPcent);      //u4FlatBrightPcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_DARKPCENT,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatDarkPcent);      // u4FlatDarkPcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_FLATX1,         m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4X1);       //sFlatRatio
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_FLATY1,         m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_FLATX2,         m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_FLATY2,         m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.sFlatRatio.u4Y2);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_HS_NV_ENABLEGREYTEXT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.bEnableGreyTextEnhance);


    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_EN_FLATSKY,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.bEnableFlatSkyEnhance);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_FLATSKYEVD,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4FlatSkyEVD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_FLATSKYTHD,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4FlatSkyTHD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_BVRATIO_X1,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_X[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_BVRATIO_Y1,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_Y[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_BVRATIO_X2,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_X[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_BVRATIO_Y2,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSFlatSkyEnhance.u4BVRatio_Y[1]);
    //#endif

    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_SCENE_SUM_R,         aeAlgoData->m_TargetSts.m_u4SceneSumR);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_SCENE_SUM_G,         aeAlgoData->m_TargetSts.m_u4SceneSumG);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_SCENE_SUM_B,         aeAlgoData->m_TargetSts.m_u4SceneSumB);

    //#ifdef INIT_PARAM
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_EN_MULTISTEP,         m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.bEnableMultiStepHS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_EVDIFF_0,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_EVDiff[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_EVDIFF_1,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_EVDiff[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_EVDIFF_2,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_EVDiff[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_EVDIFF_3,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_EVDiff[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_ODTHD_0,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_OutdoorTHD[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_ODTHD_1,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_OutdoorTHD[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_ODTHD_2,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_OutdoorTHD[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_ODTHD_3,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_OutdoorTHD[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_IDTHD_0,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_IndoorTHD[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_IDTHD_1,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_IndoorTHD[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_IDTHD_2,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_IndoorTHD[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_IDTHD_3,           m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.u4MS_IndoorTHD[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_BVRATIO_Y0,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.i4MS_BVRatio[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_MS_BVRATIO_Y1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSMultiStep.i4MS_BVRatio[1]);
    //#endif
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_Y,         aeAlgoData->m_TargetSts.Hist_HS_LimitY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_FLATHIGHY, aeAlgoData->m_TargetSts.HS_FlatHighY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_FLATLOWY,  aeAlgoData->m_TargetSts.HS_FlatLowY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_FLATPROB,  aeAlgoData->m_TargetSts.Prob_HS_FLAT);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_EVD,       aeAlgoData->m_TargetSts.Orig_HS_FLAT);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_THD,       aeAlgoData->m_TargetSts.Hist_HS_LimitTHD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_TARGET,    aeAlgoData->m_TargetSts.Hist_HS_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_LIMITBY,   aeAlgoData->m_TargetSts.HS_LIMITED_BY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_COLORDIST,     aeAlgoData->m_TargetSts.Orig_HS_COLOR);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_COLORPROB,     aeAlgoData->m_TargetSts.Prob_HS_COLOR);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HS_STS_FLATSKYDET,     aeAlgoData->m_bIsBlueSkyDet);

    // AOE
    setDebugTag(a_rAEDebugInfo, AE_TAG_ANTIOVER_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_OVEREXPOSURE)?1:0);
    //#if INIT_PARAM
    // setDebugTag(a_rAEDebugInfo, AE_TAG_ANTIOVER_EN, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.bEnableAntiOverExposure);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ANTIOVER_METERING, aeAlgoData->m_u4AntiOverExpCWM);

    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.bEnableAntiOverExposure);  //bEnableAntiOverExposure;
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4AntiOverExpWeight);  //u4AntiOverExpWeight;
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_PCENT,  m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4Pcent);  //u4Pcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_THD,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4Thd);  //u4Thd;
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4COEPcent);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COETHD, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.u4COEThd);

    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEP_ENABLE,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.bEnableCOEP);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPY_X1,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4X1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPY_Y1,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPY_X2,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPY_Y2,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEYRatio.u4Y2);

    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPDIFF_X1,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4X1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPDIFF_Y1,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPDIFF_X2,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_NV_COEPDIFF_Y2,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rAOE_Spec.sCOEDiffRatio.u4Y2);

    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_EN_BV, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.bEnable);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_BV_THD_X0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.i4BV[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_BV_THD_Y0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.u4THD[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_BV_THD_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.i4BV[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_BV_THD_Y1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.u4THD[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_BV_THD_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.i4BV[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_BV_THD_Y2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AOERefBV.u4THD[2]);


    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_EN_OERATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.bEnableOverExpoAOE);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_OE_LEVEL, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.u4OElevel);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_OE_LOWBND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.u4OERatio_LowBnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_OE_HIGHBND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.u4OERatio_HighBnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_OE_BVRATIO_X0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.i4OE_BVRatio_X[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_OE_BVRATIO_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.OverExpoAOECFG.i4OE_BVRatio_X[1]);
    //#endif

    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_Y,     aeAlgoData->m_TargetSts.AOE_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_WEIGHT, aeAlgoData->m_TargetSts.Wet_AOE);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_THD, aeAlgoData->m_TargetSts.Hist_AOE_THD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_OERATIO, aeAlgoData->m_TargetSts.OE_Ratio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_OE_P, aeAlgoData->m_TargetSts.Prob_OE_P);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_OE_DELTAY, aeAlgoData->m_TargetSts.OE_DeltaY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_TARGET, aeAlgoData->m_TargetSts.Hist_AOE_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_COEP_P, aeAlgoData->m_TargetSts.Prob_COE_P);

    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_COEP_ORIGY,  aeAlgoData->m_TargetSts.Orig_COE_OuterY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_COEP_PROBY,  aeAlgoData->m_TargetSts.Prob_COE_OuterY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_COEP_ORIGDIFF, aeAlgoData->m_TargetSts.Orig_COE_Diff);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_COEP_PROBDIFF, aeAlgoData->m_TargetSts.Prob_COE_Diff);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_COE_TARGET, aeAlgoData->m_TargetSts.AOE_COE_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_FINAL_TARGET, aeAlgoData->m_TargetSts.Target_AOE);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AOE_STS_LIMITBY,   aeAlgoData->m_TargetSts.AOE_LIMITED_BY);

    // ABL
    setDebugTag(a_rAEDebugInfo, AE_TAG_BACKLIGH_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_BACKLIGHT)?1:0);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_BACKLIGH_EN,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.bEnableBlackLight);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BACKLIGH_WEIGHTING, aeAlgoData->m_TargetSts.Wet_ABL);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BACKLIGH_METERING, aeAlgoData->m_u4BacklightCWM);
    //#ifdef INIT_PARAM
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_ENABLE,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.bEnableBlackLight);      //bEnableBlackLight;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_WEIGHT,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4BackLightWeight);      //u4BackLightWeight;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4Pcent);      //u4Pcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_THD,   m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4Thd);      //u4Thd;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_CENTERBND,       m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4CenterHighBnd);   //u4CenterHighBnd;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_TARGETSTRENGTH,  m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.u4TargetStrength);//u4TargetStrength;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_FGBGRATIO_X1,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4X1);//sFgBgEVRatio;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_FGBGRATIO_Y1,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_FGBGRATIO_X2,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_FGBGRATIO_Y2,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sFgBgEVRatio.u4Y2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_BVRATIO_X1,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4X1);//sBVRatio;
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_BVRATIO_Y1,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_BVRATIO_X2,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_NV_BVRATIO_Y2,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rABL_Spec.sBVRatio.u4Y2);

    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_EN_ABS_DIFF,     m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.bEnableABLabsDiff);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_X1,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_X[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_Y1,       m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_Y[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_X2,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_X[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_Y2,       m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4EVDiffRatio_Y[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_BVRATIO_X1,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_X[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_BVRATIO_Y1,       m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_Y[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_BVRATIO_X2,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_X[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_ABS_DIFF_BVRATIO_Y2,       m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.ABLabsDiff.u4BVRatio_Y[1]);
    //#endif
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_Y,               aeAlgoData->m_TargetSts.ABL_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_TARGET,          aeAlgoData->m_TargetSts.Hist_ABL_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_FGBGPROB,        aeAlgoData->m_TargetSts.Prob_ABL_DIFF);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_FGBGORIG,        aeAlgoData->m_TargetSts.Orig_ABL_DIFF);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_BVPROB,          aeAlgoData->m_TargetSts.Prob_ABL_BV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_BVORIG,          aeAlgoData->m_TargetSts.Orig_ABL_BV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_FINALPROB,       aeAlgoData->m_TargetSts.Prob_ABL_P);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ABL_STS_LIMITBY,         aeAlgoData->m_TargetSts.ABL_LIMITED_BY);
    //#ifdef INIT_PARAM
    // NS
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_ENABLE,       m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.bEnableNightScene);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_PCENT,        m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4Pcent);//u4Pcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_THD,          m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4Thd);//u4Thd;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_FLATTHD,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4FlatThd);//u4FlatThd;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BRIGHTTONE_PCENT,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4BrightTonePcent);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BRIGHTTONE_THD,m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4BrightToneThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_LOWBNDPCENT,  m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4LowBndPcent);//u4LowBndPcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_LOWBNDTHD,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4LowBndThd);//u4LowBndThd;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_LOWBNDTHDLIMIT, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4LowBndThdLimit);//u4LowBndThdLimit;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BRIGHTPCENT,  m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4FlatBrightPcent);//u4FlatBrightPcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_DARKPCENT,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.u4FlatDarkPcent);//u4FlatDarkPcent;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_FALT_X1,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4X1);//sFlatRatio;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_FALT_Y1,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_FALT_X2,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_FALT_Y2,      m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sFlatRatio.u4Y2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BV_X1,        m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4X1);//sBVRatio;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BV_Y1,        m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BV_X2,        m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_BV_Y2,        m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sBVRatio.u4Y2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_SKYENABLE,    m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.bEnableNightSkySuppresion);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_SKYBV_X1,     m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4X1);//sBVRatio;
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_SKYBV_Y1,     m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_SKYBV_X2,     m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_NV_SKYBV_Y2,     m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rNS_Spec.sSkyBVRatio.u4Y2);
    //#endif
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_Y,            aeAlgoData->m_TargetSts.NS_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_WEIGHT,       aeAlgoData->m_TargetSts.Prob_NS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_HIGHBNDY,     aeAlgoData->m_TargetSts.NS_BrightTone_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_LOWBNDY,      aeAlgoData->m_TargetSts.NS_BT_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_FLATHIGHY,    aeAlgoData->m_TargetSts.NS_FlatHighY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_FLATLOWY,     aeAlgoData->m_TargetSts.NS_FlatLowY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_FLATPROB,     aeAlgoData->m_TargetSts.Prob_NS_FLAT);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_EVD,          aeAlgoData->m_TargetSts.Orig_NS_FLAT);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_THD,          aeAlgoData->m_TargetSts.NS_OE_THD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_TARGET,       aeAlgoData->m_TargetSts.NS_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_LOWBND_Y,     aeAlgoData->m_TargetSts.NS_BT_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_LOWBNDTARGET, aeAlgoData->m_TargetSts.NS_BT_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_BRIGHTTONE_Y, aeAlgoData->m_TargetSts.NS_BrightTone_Y);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_BRIGHTTONE_THD, aeAlgoData->m_TargetSts.NS_BrightTone_THD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_BRIGHTTONE_TARGET, aeAlgoData->m_TargetSts.NS_BrightTone_Target);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_BVORIG,       aeAlgoData->m_TargetSts.Orig_NS_BV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_BVPROB,       aeAlgoData->m_TargetSts.Prob_NS_BV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_CDF,       aeAlgoData->m_TargetSts.NS_CDF_BV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_CDFPROB,       aeAlgoData->m_TargetSts.Prob_NS_CDF);
    setDebugTag(a_rAEDebugInfo, AE_TAG_NS_STS_LIMITBY, aeAlgoData->m_TargetSts.NS_LIMITED_BY);

    //#ifdef INIT_PARAM
    //v4p0
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERV4P0_EN,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bAEv4p0MeterEnable);
    //Main target
    setDebugTag(a_rAEDebugInfo, AE_TAG_MAINTARGET_HIT,  (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_MAINTARGET)?1:0);

    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_ENABLE,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableAEMainTarget);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_WEIGHT,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.u4MainTargetWeight);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_TARGETBV_X1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4X1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_TARGETBV_Y1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_TARGETBV_X2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_TARGETBV_Y2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.TargetBVRatioTbl.u4Y2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLORWT_EN,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableColorWTRatio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLORWT_X1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4X1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLORWT_Y1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4Y1);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLORWT_X2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4X2);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLORWT_Y2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.ColorWTRatioTbl.u4Y2);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLOSUP_R,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableColorSuppressR);?????????????????????????????????????????
    //setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLOSUP_G,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableColorSuppressG);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_MT_COLOSUP_B,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.MainTargetCFG.bEnableColorSuppressB);
    //#endif
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_STS_Y,        aeAlgoData->m_u4GauCWValue);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_STS_TARGET,        aeAlgoData->m_TargetSts.Target_Main);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_STS_PRETARGET,        aeAlgoData->m_TargetSts.Target_PrevMain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MT_STS_WEIGHT,        aeAlgoData->m_TargetSts.Wet_Main);

    //HSv4p0
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_HIT,        (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_HS_V4P0)?1:0);
    //#ifdef INIT_PARAM
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_ENABLE,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.bEnableHSv4p0);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_WEIGHT,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HSv4p0Weight);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_BVRATIO_0,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_BVRATIO_1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_BVRATIO_2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_BVRATIO_3,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.i4HS_BVRatio[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_PCNT_0,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_PCNT_1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_PCNT_2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_PCNT_3,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_PcntRatio[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_EVD_0,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_EVD_1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_EVD_2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_EVD_3,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_EVD_4,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HS_EVDRatio[4]);
    //#endif
    for(i=0;i<4;i++)
        for(j=0;j<5;j++)
        {
            setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_BV0THD_0+i*5+j,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.u4HSTHDRatioTbl[i][j]);
        }
        //#ifdef INIT_PARAM
        setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_DYNWT_EN,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.bEnableDynWTRatio);
        setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_DYNWT_X1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4X1);
        setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_DYNWT_Y1,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4Y1);
        setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_DYNWT_X2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4X2);
        setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_DYNWT_Y2,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.HSv4p0CFG.DynWTRatioTbl.u4Y2);
        //#endif
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_STS_Y,        aeAlgoData->m_TargetSts.Hist_HS_LimitY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_STS_EVD,        aeAlgoData->m_TargetSts.Orig_HS_FLAT);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_STS_THD,        aeAlgoData->m_TargetSts.Hist_HS_LimitTHD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_STS_WEIGHT,        aeAlgoData->m_TargetSts.Wet_HSv4p0);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_STS_TARGET,        aeAlgoData->m_TargetSts.Target_HSv4p0);
    setDebugTag(a_rAEDebugInfo, AE_TAG_HSV4P0_STS_PRETARGET,        aeAlgoData->m_TargetSts.Target_PrevHSv4p0);

   // #ifdef INIT_PARAM
    // Custom tuning setting
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_PRE_INDEX, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnablePreIndex);
    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_ROTATE_WEIGHT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableRotateWeighting);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_EV0_STROBE_TRIGGER, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEV0TriggerStrobe);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_LOCK_CAM_PRE_METERING_WIN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bLockCamPreMeteringWin);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_LOCK_VDO_PRE_METERING_WIN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bLockVideoPreMeteringWin);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_LOCK_VDO_REC_METERING_WIN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bLockVideoRecMeteringWin);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_EN_PREAF_LOCK_AE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bPreAFLockAE);

    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_STABLE_MAX, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMax);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_STABLE_MIN, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMin);
    setDebugTag(a_rAEDebugInfo, AE_TAG_X_BLOCK_NO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.uBlockNumX);
    setDebugTag(a_rAEDebugInfo, AE_TAG_Y_BLOCK_NO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.uBlockNumY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_HIGH_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uFaceYHighBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_LOW_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uFaceYLowBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_HIGH_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uMeteringYHighBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_LOW_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uMeteringYLowBound);
    //#endif
    // AE algorithm control setting
    setDebugTag(a_rAEDebugInfo, AE_TAG_CWV, aeAlgoData->m_u4CWValue);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AVG, aeAlgoData->m_u4AvgWValue);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CENTRAL_Y, aeAlgoData->m_u4CentralY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_CWV_FINAL_TARGET, aeAlgoData->m_u4CWRecommend);

    //#ifdef INIT_PARAM
    setDebugTag(a_rAEDebugInfo, AE_TAG_AE_TARGET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4AETarget);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_STROBE_TARGET, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4StrobeAETarget);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_INIT_INDEX, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.u4InitIndex);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MIN_CWV_RECMD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MinCWRecommend);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MAX_CWV_RECMD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MaxCWRecommend);

    //FACE
    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_FACE_AE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableFaceAE);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERFACE_HIT, (aeAlgoData->m_u4AECondition&CCU_AE_CONDITION_FACEAE)?1:0);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_Y, aeAlgoData->m_MeterFDSts.m_u4FDY);    // for face AE value
    // setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_CENTRAL_Y, aeAlgoData->m_MeterFDSts.m_u4FDCentralY);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_FULL_Y, aeAlgoData->m_MeterFDSts.m_u4FDFullY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_WEIGHTING, m_CcuAeExifDataPtr.pAeNVRAM->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec.uFaceCentralWeight);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Scale_Ratio);

    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_Y_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4MinFDYLowBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_Y_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4MaxFDYHighBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_BVRATIO_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.i4FaceBVLowBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_BVRATIO_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.i4FaceBVHighBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_TMPFDY_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4TmpFDY_LBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_TMPFDY_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4TmpFDY_HBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_NS_LOW_BOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FaceNSLowBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_LOST_MAXCNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Lost_MaxCnt);


   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_EN_LOCSIZE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.bFaceLocSizeCheck);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_SIZE_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Size_X[0]);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_SIZE_Y1,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Size_Y[0] );
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_SIZE_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Size_X[1]);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_SIZE_Y2,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Size_Y[1]);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_LOC_X1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Loc_X[0]);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_LOC_Y1,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Loc_Y[0]);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_LOC_X2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Loc_X[1]);
   setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_LOC_Y2,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.u4Loc_Y[1]);
   //#endif
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_PROB,    aeAlgoData->m_MeterFDSts.m_u4FDProb);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_STS_LOC,    aeAlgoData->m_MeterFDSts.m_u4FaceProbLoc);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_STS_SIZE,    aeAlgoData->m_MeterFDSts.m_u4FaceProbSize);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_STS_FINDCNT,    aeAlgoData->m_MeterFDSts.u4FaceFoundCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_STS_LOSTCNT,    aeAlgoData->m_MeterFDSts.u4FaceFailCnt);




    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableFaceFastConverge);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_Y, aeAlgoData->m_MeterFDSts.m_u4FDY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_CWV, aeAlgoData->m_u4CWValue);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_BV_L, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.i4FaceBVLowBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_BV_H, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.i4FaceBVHighBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_REAL_BV_X1000,aeAlgoData-> m_i4RealBVx1000);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_METER_FD_TARGET, aeAlgoData->m_u4MeterFDTarget );
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_NORMAL_TARGET, aeAlgoData->m_u4NormalTarget );
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FINAL_TARGET, aeAlgoData->m_u4FaceFinalTarget );
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_TARGET, aeAlgoData->m_u4RealFaceCWR);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LCE_LINK, aeAlgoData->m_bFaceAELCELinkEnable);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LCE_MAX_GAIN, aeAlgoData->m_u4MaxGain);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_SE_RATIO, aeAlgoData->m_u4SERatio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_IS_FRONT, aeAlgoData->m_MeterFDSts.m_u4IsRobustFace);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_IS_SIDE, aeAlgoData->m_MeterFDSts.m_u4IsSideFace);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_IS_OT, aeAlgoData->m_MeterFDSts.m_u4IsOTFace);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_CASE_NUMBER, aeAlgoData->m_u4FaceCaseNum);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FOUND_CNT, aeAlgoData->m_MeterFDSts.u4FaceFoundCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FAIL_CNT, aeAlgoData->m_MeterFDSts.u4FaceFailCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_STABLE_CNT, aeAlgoData->m_MeterFDSts.m_u4FaceStableCount);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE, aeAlgoData->m_u4FaceAEStable);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_INDEX, aeAlgoData->m_u4FaceAEStableIndex);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_CWV, aeAlgoData->m_u4FaceAEStableCWV);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_CWR, aeAlgoData->m_u4FaceAEStableCWR);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_INTHD, aeAlgoData->m_u4FaceAEStableInThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_OUTTHD, aeAlgoData->m_u4FaceAEStableOutThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_XLOW, aeAlgoData->m_FaceAEStableBlock.u4XLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_XHI, aeAlgoData->m_FaceAEStableBlock.u4XHi);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_YLOW, aeAlgoData->m_FaceAEStableBlock.u4YLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_FACE_AE_STABLE_YHI, aeAlgoData->m_FaceAEStableBlock.u4YHi);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_AE_COMP, aeAlgoData->m_AECmdSet.eAEComp);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACE_NUM, aeAlgoData->m_MeterFDSts.m_u4FaceNum);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_0,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_1,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_2,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_3,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_4,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[4]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_5,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[5]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_6,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[6]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_7,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[7]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FACEY_8,  aeAlgoData->m_i4FDY_ArrayFace_CCUFrameDone[8]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_0, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_1, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_2, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_3, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_4, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[4]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_5, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[5]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_6, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[6]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_7, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[7]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_LMY_8, aeAlgoData->m_i4FDY_ArrayLM_CCUFrameDone[8]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_0, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_1, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_2, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_3, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_4, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[4]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_5, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[5]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_6, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[6]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_7, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[7]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_FDY_8, aeAlgoData->m_i4FDY_ArrayOri_CCUFrameDone[8]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_0, aeAlgoData->m_i4FDY_Array_CCUFrameDone[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_1, aeAlgoData->m_i4FDY_Array_CCUFrameDone[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_2, aeAlgoData->m_i4FDY_Array_CCUFrameDone[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_3, aeAlgoData->m_i4FDY_Array_CCUFrameDone[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_4, aeAlgoData->m_i4FDY_Array_CCUFrameDone[4]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_5, aeAlgoData->m_i4FDY_Array_CCUFrameDone[5]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_6, aeAlgoData->m_i4FDY_Array_CCUFrameDone[6]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_7, aeAlgoData->m_i4FDY_Array_CCUFrameDone[7]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_MULTI_SORT_FDY_8, aeAlgoData->m_i4FDY_Array_CCUFrameDone[8]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LOCK_MAX_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_Lock_MaxCnt);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MIX_MAX_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_MixCWR_MaxCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_TEMPORAL_SMOOTH, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_TemporalSmooth);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_OT_LOCK_UPPER, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_OTLockUpperBnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_OT_LOCK_LOWER, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_OTLockLowerBnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_CONT_TRUST_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ContinueTrustCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_PERFRAME_ANTI_OVER, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_PerframeAntiOverFlag);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LOC_SIZE_CHECK, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLocSizecheck.bFaceLocSizeCheck);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_IN_STALE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceInStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_OUT_B2T_STALE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceOutB2TStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_OUT_D2T_STALE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceOutD2TStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_IN_TO_OUT_THD_MAX_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_InToOutThdMaxCnt);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_W_CROP_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_FaceWidthCropRat);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_H_CROP_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_FaceHeightCropRat);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_SIDE_FACE_LOCK, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_SideFaceLock);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LIMIT_STABLE_THD_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LIMIT_STABLE_THD_NUM, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBndNum);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_FACE_CHANGE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReConvergeWhenFaceChange);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MOTION_LOCK_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_FaceMotionLockRat);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_IMPULSE_LOCK_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ImpulseLockCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_IMPULSE_UPPER_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ImpulseUpperThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_IMPULSE_LOWER_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ImpulseLowerThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_ROP_TRUST_CNT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ROPTrustCnt);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_SIZE_CHANGE_RAT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReCovergeWhenSizeChangeRat);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_POS_CHANGE_DIST, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReCovergeWhenPosChangeDist);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_RECONVERGE_WHEN_FACE_AF_DONE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceSmooth.u4FD_ReCovergeWhenAFDone);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.bEnableMultiFaceWeight);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_PRIOR, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.Prior);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_0, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_1, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_2, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[2]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_3, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[3]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_4, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[4]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_5, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[5]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_6, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[6]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_7, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[7]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_MULTI_WEIGHT_8, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceWeight.u4FaceWeight[8]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_EXT_RATIO_W,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkExtRatW);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_EXT_RATIO_H,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkExtRatH);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_TRUST_ROP,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4LandmarkTrustRopDegree);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_SMOOTH_XY,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4RoundXYPercent);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_SMOOTH_SIZE,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4RoundSZPercent);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_SMOOTH_OVERLAP,   m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4THOverlap);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_SMOOTH_BUFLEN,    m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4BUFLEN);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_SMOOTH_LEVEL,     m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4SMOOTH_LEVEL);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_SMOOTH_MOMENTUM,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.i4MOMENTUM);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_PARA_LM_MIX_WEIGHT_PCENT, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.FaceLandmark.u4LandmarkWeightPercent);

    // for Face AE
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_CNT, aeAlgoData->m_MeterFDSts.m_u4FaceNum);
    for(i=0; i<3; i++)
    {
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_XLOW+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4XLow);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_XHI+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4XHi);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_YLOW+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4YLow);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_YHI+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].i4YHi);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_WEIGHT+7*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[i].u4Weight);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_FULLY+7*i, aeAlgoData->m_i4FDY_Array_CCUFrameDone[i]);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_CENTERY+7*i, aeAlgoData->m_MeterFDSts.m_u4FaceCentralYValue[i]);

        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_SCALE0_XLOW+4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4XLow);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_SCALE0_XHI +4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4XHi);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_SCALE0_YLOW+4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4YLow);
        setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_SCALE0_YHI +4*i, aeAlgoData->m_MeterFDSts.m_AEFDWinBlock_Scale[i].u4YHi);
    }

    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE0_WEIGHT2K, aeAlgoData->m_MeterFDSts.m_u4Weighting[0]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE1_WEIGHT2K, aeAlgoData->m_MeterFDSts.m_u4Weighting[1]);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE2_WEIGHT2K, aeAlgoData->m_MeterFDSts.m_u4Weighting[2]);

    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_XLOW,       aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkXLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_XHI,        aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkXHi);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_YLOW,       aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkYLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_YHI,        aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].u4LandMarkYHi);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_RIP,        aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4LandMarkRip);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_ROP,        aeAlgoData->m_MeterFDSts.m_AEFDWinBlock[0].i4LandMarkRop);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_ICS_LEFT,   aeAlgoData->m_LandMarkFDSmoothArea.i4Left);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_ICS_RIGHT,  aeAlgoData->m_LandMarkFDSmoothArea.i4Right);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_ICS_TOP,    aeAlgoData->m_LandMarkFDSmoothArea.i4Top);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_LANDMARK0_ICS_BOTTOM, aeAlgoData->m_LandMarkFDSmoothArea.i4Bottom);

    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_DIGITAL_ZOOM_TGDOMAIN_XOFFSET, aeAlgoData->m_i4DigitalZoomTGDomain_XOffset);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_DIGITAL_ZOOM_TGDOMAIN_YOFFSET, aeAlgoData->m_i4DigitalZoomTGDomain_YOffset);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_DIGITAL_ZOOM_TGDOMAIN_WIDTH, aeAlgoData->m_i4DigitalZoomTGDomain_Width);
    setDebugTag(a_rAEDebugInfo, AE_TAG_FACE_20_DIGITAL_ZOOM_TGDOMAIN_HEIGHT, aeAlgoData->m_i4DigitalZoomTGDomain_Height);


    // //TG
    // setDebugTag(a_rAEDebugInfo, AE_TAG_INT_AE_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bTgIntAEEn);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_TG_ESC_OVEREXP_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bSpeedupEscOverExp);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_TG_ESC_UNDEREXP_EN, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.bSpeedupEscUnderExp);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_TG_ACC_DELTA_EV_OVEREXP, m_accTGDeltaEV_OverExp);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_TG_ACC_DELTA_EV_UNDEREXP, m_accTGDeltaEV_UnderExp);

    // //PSO
    // setDebugTag(a_rAEDebugInfo, AE_TAG_PSO_LINEAR_CWV, m_u4LinearCWValue);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_PSO_TUNING_ACC_ENABLE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.PSOConverge->u4PsoAccEnable);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_PSO_TUNING_ACC_ESC_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.PSOConverge->i4PsoEscRatioWO);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_PSO_TUNING_CONV_DELTAEV_POSDOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.PSOConverge->ConvergeDeltaPosBound);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_PSO_TUNING_CONV_DELTAEV_NEGDOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.PSOConverge->ConvergeDeltaNegBound);

    //for Metering temporal smooth
    //#if INIT_PARAM
    setDebugTag(a_rAEDebugInfo, AE_TAG_TEMP_SMOOTH_HS,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEHsTS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_TEMP_SMOOTH_ABL,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEAblTS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_TEMP_SMOOTH_AOE,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEAoeTS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_TEMP_SMOOTH_NS,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAENsTS);
    // setDebugTag(a_rAEDebugInfo, AE_TAG_TEMP_SMOOTH_FACE,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEFaceTS);
    setDebugTag(a_rAEDebugInfo, AE_TAG_TEMP_SMOOTH_VIDEO,m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AETempSmooth.bEnableAEVideoTS);
    //#endif

    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_METER_AE, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableMeterAE);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_EN, aeAlgoData->m_bAEMeteringEnable);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_Y, aeAlgoData->m_u4MeterY);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_Y_LOWBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MinYLowBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_Y_HIGHBOUND, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MaxYHighBound);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_Y0, aeAlgoData->m_u4MeteringYValue[0]);

    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING_CNT, aeAlgoData->m_u4MeteringCnt);

    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING0_XLOW, aeAlgoData->m_AETOUCHWinBlock[0].u4XLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING0_XHI, aeAlgoData->m_AETOUCHWinBlock[0].u4XHi);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING0_YLOW, aeAlgoData->m_AETOUCHWinBlock[0].u4YLow);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING0_YHI, aeAlgoData->m_AETOUCHWinBlock[0].u4YHi);
    setDebugTag(a_rAEDebugInfo, AE_TAG_METERING0_WEIGHT, aeAlgoData->m_AETOUCHWinBlock[0].u4Weight);

    // if ((m_i4AEDebugCtrl == AE_DBG_OPTION_ALL_ON) || (m_i4AEDebugCtrl == AE_DBG_OPTION_AE_WIN_INFO))
    //     MY_LOG_IF(m_i4AEDebugCtrl,"[getDebugInfo] XLow:%d XHi:%d YLow:%d YHi:%d Weight:%d\n", m_MeterFDSts.m_AEFDWinBlock[0].u4XLow, m_MeterFDSts.m_AEFDWinBlock[0].u4XHi,
    //     m_MeterFDSts.m_AEFDWinBlock[0].u4YLow, m_MeterFDSts.m_AEFDWinBlock[0].u4YHi, m_MeterFDSts.m_AEFDWinBlock[0].u4Weight);

    // AE window and histogram configure setting
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_MAX_BLOCK_WIDTH, aeInitData->m_i4AEMaxBlockWidth);
    setDebugTag(a_rAEDebugInfo, AE_TAG_PRV_MAX_BLOCK_HEIGHT, aeInitData->m_i4AEMaxBlockHeight);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STAT_XLOW, aeVsyncInfo->m_AEWinBlock.u4XLow); //vsync timing
    setDebugTag(a_rAEDebugInfo, AE_TAG_STAT_XHI, aeVsyncInfo->m_AEWinBlock.u4XHi); //vsync timing
    setDebugTag(a_rAEDebugInfo, AE_TAG_STAT_YLOW, aeVsyncInfo->m_AEWinBlock.u4YLow); //vsync timing
    setDebugTag(a_rAEDebugInfo, AE_TAG_STAT_YHI, aeVsyncInfo->m_AEWinBlock.u4YHi); //vsync timing

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_DEFAULT_IN_THD,      m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4InStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_DEFAULT_OUT_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4OutStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_EN_MODE_THD, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.bEnableAEModeStableTHD);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_Video_IN_THD,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4VideoInStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_Video_OUT_THD,    m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4VideoOutStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_Face_IN_THD,          m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceInStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_Face_OUT_THD,     m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4FaceOutStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_Touch_IN_THD,        m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4TouchInStableThd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STB_Touch_OUT_THD,   m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.AEStableThd.u4TouchOutStableThd);

    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_TOUCH_SMOOTH,     m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableTouchSmoothRatio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_TOUCH_PERFRAME,     m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.bEnableTouchSmooth);
    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_TOUCH_WEIGHT,     m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.strAEParasetting.u4MeterWeight);


    setDebugTag(a_rAEDebugInfo, AE_TAG_EN_BVACCRATIO,             m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.u4BVAccEnable);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BVACC_DELTA_RATIO,    m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.i4DeltaBVRatio);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BVACC_B2TEND,               m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.pAEBVAccRatio.u4Bright2TargetEnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BVACC_B2TEND_RATIO, m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.pAEBVAccRatio.u4B2TEnd);
    setDebugTag(a_rAEDebugInfo, AE_TAG_BVACC_B2TSTART_RATIO,  m_CcuAeExifDataPtr.pAeNVRAM->rAeParam.BVAccRatio.pAEBVAccRatio.u4B2TStart);

    // AE statistic window output value
    for(i=0; i<25; i++)
    {
        setDebugTag(a_rAEDebugInfo, AE_TAG_STAT_WIN00+i, aeAlgoData->m_pu4AEBlock_Exif[i/5][i%5]);
    }

    for(i=0; i<225; i++)
    {
        u4packedRGBY = aeAlgoData->m_pu4AEv4p0Block_Exif[i/15][i%15];
        setDebugTag(a_rAEDebugInfo, AE_TAG_STATRGBY15_WIN0000+i, (MINT32)u4packedRGBY);
    }

    for(i=0; i<225; i++)
    {
        setDebugTag(a_rAEDebugInfo, AE_TAG_OVERCNT_WIN0000+i, aeAlgoData->m_pu4AEOverExpoBlockCnt[i/15][i%15]);
    }

    //m_pu4FlareRGBHist
    for(i=0;i<(FLARE_HISTOGRAM_BIN/2) * 3;i++)
    {
        i4flareidx = (i/(FLARE_HISTOGRAM_BIN/2))*(FLARE_HISTOGRAM_BIN/2) + i;
        setDebugTag(a_rAEDebugInfo, AE_TAG_HIST0_INFO_BIN_0+i, aeAlgoData->m_pu4FlareRGBHist[i4flareidx]);
    }

    // AE histogram output value
    for(i=0; i<AE_HISTOGRAM_BIN; i++)
    {
        setDebugTag(a_rAEDebugInfo, AE_TAG_HIST1_INFO_BIN_0+i, aeAlgoData->m_pu4FullRGBHist[i]);
        setDebugTag(a_rAEDebugInfo, AE_TAG_HIST2_INFO_BIN_0+i, aeAlgoData->m_pu4FullYHist[i]);
        setDebugTag(a_rAEDebugInfo, AE_TAG_HIST3_INFO_BIN_0+i, aeAlgoData->m_pu4CentralYHist[i]);
    }

    //PSO Statistic data
    //setDebugTag(a_rAEDebugInfo, AE_TAG_LINEARSORTED_Y_224, m_pu4AELieanrSortedY[224]);

    setDebugTag(a_rAEDebugInfo, AE_TAG_CAM_MODE, aeAlgoData->m_AECmdSet.eAECamMode);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_OLD_AE_SMOOTH, m_CcuAeExifDataPtr.pAeNVRAM->bOldAESmooth);
    //setDebugTag(a_rAEDebugInfo, AE_TAG_SUB_PRE_INDEX_EN, m_CcuAeExifDataPtr.pAeNVRAM->bEnableSubPreIndex);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_DFS_EN, aeAlgoData->m_AECmdSet.bVideoDynamic);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MAX_FPS, aeAlgoData->m_AECmdSet.i4AEMaxFps);
    setDebugTag(a_rAEDebugInfo, AE_TAG_MIN_FPS, aeAlgoData->m_AECmdSet.i4AEMinFps);
    setDebugTag(a_rAEDebugInfo, AE_TAG_SENSOR_ID, aeAlgoData->m_AECmdSet.eSensorDev);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AE_LOCKED, aeAlgoData->m_AECmdSet.bAELock);
    setDebugTag(a_rAEDebugInfo, AE_TAG_VIDEO_RECORD, aeAlgoData->m_AECmdSet.bVideoRecording);
    setDebugTag(a_rAEDebugInfo, AE_TAG_AE_LIMITER, aeAlgoData->m_AECmdSet.bAElimitor);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STROBE_ON, aeAlgoData->m_AECmdSet.bStrobeOn);
    setDebugTag(a_rAEDebugInfo, AE_TAG_ROTATE_WEIGHTING, aeAlgoData->m_AECmdSet.bRotateWeighting);
    setDebugTag(a_rAEDebugInfo, AE_TAG_OBJECT_TRACKING, aeAlgoData->m_AECmdSet.bAEObjectTracking);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_0, AE_ALGO_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_1, AE_PARAM_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_2, AE_ALGO_IF_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_3, DBG_AE_PARAM_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_INFO_4, AE_ALGO_TOOL_REVISION);
    setDebugTag(a_rAEDebugInfo, AE_TAG_STABLE, aeAlgoData->m_bAEStable);
    // log_value("EXIFTIME", diffCnt);
    return;
}

enum ccu_feature_type CcuCtrlAe::_getFeatureType()
{
    return CCU_FEATURE_AE;
}

bool CcuCtrlAe::ccuCtrlPreprocess(ccu_msg_id msgId, void *inDataPtr, void *inDataBuf)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_SENSOR_INIT:
        {
            result = ccuCtrlPreprocess_SensorInit(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_AE_INIT:
        {
            result = ccuCtrlPreprocess_AeInit(inDataPtr, inDataBuf);
            break;
        }
        default : break;
    }
    return result;
}

bool CcuCtrlAe::ccuCtrlPostprocess(ccu_msg_id msgId, void *outDataPtr, void *inDataBuf)
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
bool CcuCtrlAe::ccuCtrlPreprocess_AeInit(void *inDataPtr, void *inDataBuf)
{
    enum ccu_tg_info tgInfo = getCcuTgInfo();

    LOG_DBG_MUST("param1 size: %d", LumLog2x1000_TABLE_SIZE * sizeof(MUINT32));
    LOG_DBG_MUST("param2 size: %d", CCU_LIB3A_AE_EV_COMP_MAX * sizeof(MUINT32));
    LOG_DBG_MUST("param3 size: %d", sizeof(CCU_strEvPline));
    LOG_DBG_MUST("param4 size: %d", sizeof(CCU_AE_NVRAM_T));
    

    CcuCtrlAeInitParam *input = (CcuCtrlAeInitParam *)inDataPtr;
    CcuCtrlAeInitParam *initParam = (CcuCtrlAeInitParam *)inDataBuf;

    //Pick CCU AE Algo. required data from AE_PARAM_T, fill into CcuCtrlAeInitParam.ccuAeInitParam.m_AeTuningParam
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableTouchSmoothRatio = input->nvramData->rAeParam.strAEParasetting.bEnableTouchSmoothRatio;      
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableTouchSmooth = input->nvramData->rAeParam.strAEParasetting.bEnableTouchSmooth;           
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bLockCamPreMeteringWin = input->nvramData->rAeParam.strAEParasetting.bLockCamPreMeteringWin;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bLockVideoPreMeteringWin = input->nvramData->rAeParam.strAEParasetting.bLockVideoPreMeteringWin;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bLockVideoRecMeteringWin = input->nvramData->rAeParam.strAEParasetting.bLockVideoRecMeteringWin;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableFlareFastConverge = input->nvramData->rAeParam.strAEParasetting.bEnableFlareFastConverge;     
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableRotateWeighting = input->nvramData->rAeParam.strAEParasetting.bEnableRotateWeighting;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableFaceAE = input->nvramData->rAeParam.strAEParasetting.bEnableFaceAE;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableMeterAE = input->nvramData->rAeParam.strAEParasetting.bEnableMeterAE;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableHDRLSB = input->nvramData->rAeParam.strAEParasetting.bEnableHDRLSB;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumX = input->nvramData->rAeParam.strAEParasetting.uBlockNumX;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumY = input->nvramData->rAeParam.strAEParasetting.uBlockNumY;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MinYLowBound = input->nvramData->rAeParam.strAEParasetting.u4MinYLowBound;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MaxYHighBound = input->nvramData->rAeParam.strAEParasetting.u4MaxYHighBound;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MeterWeight = input->nvramData->rAeParam.strAEParasetting.u4MeterWeight;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MinCWRecommend = input->nvramData->rAeParam.strAEParasetting.u4MinCWRecommend;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MaxCWRecommend = input->nvramData->rAeParam.strAEParasetting.u4MaxCWRecommend;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.m_TgValidBlkHeight = 84; //Hard-code at first phase
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.bAEv4p0MeterEnable = input->nvramData->rAeParam.bAEv4p0MeterEnable;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4FinerEvIndexBase = input->nvramData->rAeParam.strAEParasetting.u4FinerEvIndexBase;
    // //Flare
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bFlarMaxStepGapLimitEnable = input->nvramData->rAeParam.strAEParasetting.bFlarMaxStepGapLimitEnable;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableFlareFastConverge = input->nvramData->rAeParam.strAEParasetting.bEnableFlareFastConverge;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4HSSmoothTHD = input->nvramData->rAeParam.strAEParasetting.u4HSSmoothTHD;
    // //Flare
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bFlarMaxStepGapLimitEnable = input->nvramData->rAeParam.strAEParasetting.bFlarMaxStepGapLimitEnable;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableFlareFastConverge = input->nvramData->rAeParam.strAEParasetting.bEnableFlareFastConverge;

    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pDynamicFlareCFG.u4FlareSmallDeltaIdxStep = input->nvramData->rAeParam.DynamicFlareCFG.u4FlareSmallDeltaIdxStep;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pDynamicFlareCFG.u4CWVChangeStep          = input->nvramData->rAeParam.DynamicFlareCFG.u4CWVChangeStep         ;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pDynamicFlareCFG.u4AllStableStep          = input->nvramData->rAeParam.DynamicFlareCFG.u4AllStableStep         ; 

    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pABLabsDiff, &input->nvramData->rAeParam.ABLabsDiff, sizeof(CCU_strABL_absDiff));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHSFlatSkyEnhance, &input->nvramData->rAeParam.HSFlatSkyEnhance, sizeof(CCU_strHSFlatSkyCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHSMultiStep, &input->nvramData->rAeParam.HSMultiStep, sizeof(CCU_strHSMultiStep));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pNonCWRAcc, &input->nvramData->rAeParam.NonCWRAcc, sizeof(CCU_strNonCWRAcc));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAOERefBV, &input->nvramData->rAeParam.AOERefBV, sizeof(CCU_strAOEBVRef));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pOverExpoAOECFG, &input->nvramData->rAeParam.OverExpoAOECFG, sizeof(CCU_strOverExpoAOECFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pMainTargetCFG, &input->nvramData->rAeParam.MainTargetCFG, sizeof(CCU_strMainTargetCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHSv4p0CFG, &input->nvramData->rAeParam.HSv4p0CFG, sizeof(CCU_strHSv4p0CFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pNSBVCFG, &input->nvramData->rAeParam.NSBVCFG, sizeof(CCU_strNSBVCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAEACCMovingRatio, &input->nvramData->rAeParam.AEACCMovingRatio, sizeof(CCU_strAEMovingRatio));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAETempSmooth, &input->nvramData->rAeParam.AETempSmooth, sizeof(CCU_strAECWRTempSmooth));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pFaceSmooth, &input->nvramData->rAeParam.FaceSmooth, sizeof(CCU_strFaceSmooth));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pFaceLandmark, &input->nvramData->rAeParam.FaceLandmark, sizeof(CCU_strFaceLandMarkCtrl));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pNsCdfRatio, &input->nvramData->rAeParam.NsCdfRatio, sizeof(CCU_strNS_CDF));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pFaceLocSizecheck, &input->nvramData->rAeParam.FaceLocSizecheck, sizeof(CCU_strFaceLocSize));
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.bPerFrameAESmooth = input->nvramData->rAeParam.bPerFrameAESmooth;
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pAEStableThd, &input->nvramData->rAeParam.AEStableThd, sizeof(CCU_strAEStableThd));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pPerframeCFG, &input->nvramData->rAeParam.PerframeCFG, sizeof(CCU_strPerframeCFG));
    // {//Customize weighting table but the last table now is null. Solved with new nvram structure
    //     int i = 0;
    //     for(i = 0; i<3; ++i)
    //         memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pWeighting[i], &input->nvramData->rAeParam.Weighting[i], sizeof(CCU_strWeightTable));
    // }
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pHistStableCFG, &input->nvramData->rAeParam.HistStableCFG, sizeof(CCU_strHistStableCFG));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pPSOConverge, &input->nvramData->rAeParam.PSOConverge, sizeof(CCU_strPSOConverge));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pBVAccRatio, &input->nvramData->rAeParam.BVAccRatio, sizeof(CCU_strBVAccRatio));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pDynamicFlareCFG, &input->nvramData->rAeParam.DynamicFlareCFG, sizeof(CCU_strDynamicFlare));
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pFaceWeight, &input->nvramData->rAeParam.FaceWeight, sizeof(CCU_strFaceWeight));
    
    
        // int i, j, k ;
        // for(k = 0; k<3; ++k)
        // {
        //  LOG_DBG_MUST("dbgweight table %x \n ", k);
        //  for(i = 0; i<5; ++i)
        //  {
        //      for(j = 0; j<5; ++j)
        //      {
        //          LOG_DBG_MUST("dbgweight2 %x %x %x, ", i, j, initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pWeighting[k].W[i][j]);
        //      }
        //      //LOG_DBG_MUST("\n");
        //  }            
        // }
        

    
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u2AEStatThrs = input->nvramData->rAeParam.strAEParasetting.u2AEStatThrs;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uCycleNumAESkipAfterExit = input->nvramData->rAeParam.strAEParasetting.uCycleNumAESkipAfterExit;
    // initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.iMiniBVValue = input->nvramData->rAeParam.strAEParasetting.iMiniBVValue;
    
    LOG_DBG_MUST("XDBG_inDataPtr: %p", inDataPtr);
    LOG_DBG_MUST("XDBG_O_&uBlockNumX: %p", &input->nvramData->rAeParam.strAEParasetting.uBlockNumX);
    LOG_DBG_MUST("XDBG_O_bEnableHDRLSB: %x", input->nvramData->rAeParam.strAEParasetting.bEnableHDRLSB);
    LOG_DBG_MUST("XDBG_O_uBlockNumX: %x", input->nvramData->rAeParam.strAEParasetting.uBlockNumX);
    LOG_DBG_MUST("XDBG_O_uBlockNumY: %x", input->nvramData->rAeParam.strAEParasetting.uBlockNumY);
    LOG_DBG_MUST("XDBG_O_u4MinYLowBound: %x", input->nvramData->rAeParam.strAEParasetting.u4MinYLowBound);
    
    LOG_DBG_MUST("XDBG_inDataBuf: %p", inDataBuf);
    // LOG_DBG_MUST("XDBG_&uBlockNumX: %p", &initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumX);
    // LOG_DBG_MUST("XDBG_bEnableHDRLSB: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableHDRLSB);
    // LOG_DBG_MUST("XDBG_uBlockNumX: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumX);
    // LOG_DBG_MUST("XDBG_uBlockNumY: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.uBlockNumY);
    // LOG_DBG_MUST("XDBG_u4MinYLowBound: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4MinYLowBound);

    //m_pDrvCcu->PrintReg();
    //Pick CCU AE Algo. required data from AE_NVRAM_T, fill into CcuCtrlAeInitParam.m_AeCCTConfig
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4AETarget = input->nvramData->rCCTConfig.u4AETarget;
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.i4BVOffset = input->nvramData->rCCTConfig.i4BVOffset;
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec, &input->nvramData->rCCTConfig.rMeteringSpec, sizeof(CCU_AE_METER_SPEC_T));
    // //Flare
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4PrvMinFlareThres = input->nvramData->rCCTConfig.u4PrvMinFlareThres;
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4PrvMaxFlareThres = input->nvramData->rCCTConfig.u4PrvMaxFlareThres;
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoMinFlareThres = input->nvramData->rCCTConfig.u4VideoMinFlareThres;
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoMaxFlareThres = input->nvramData->rCCTConfig.u4VideoMaxFlareThres;

    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4PreviewFlareOffset  =     input->nvramData->rCCTConfig.u4PreviewFlareOffset; 
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4CaptureFlareOffset  =     input->nvramData->rCCTConfig.u4CaptureFlareOffset;    
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4CaptureFlareThres   =     input->nvramData->rCCTConfig.u4CaptureFlareThres;    
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoFlareOffset    =     input->nvramData->rCCTConfig.u4VideoFlareOffset;   
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoFlareThres     =     input->nvramData->rCCTConfig.u4VideoFlareThres;   
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4CustomFlareOffset   =     input->nvramData->rCCTConfig.u4CustomFlareOffset;   
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4CustomFlareThres    =     input->nvramData->rCCTConfig.u4CustomFlareThres;   
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4StrobeFlareOffset   =     input->nvramData->rCCTConfig.u4StrobeFlareOffset;   
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4StrobeFlareThres    =     input->nvramData->rCCTConfig.u4StrobeFlareThres;   
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4PrvMaxFlareThres    =     input->nvramData->rCCTConfig.u4PrvMaxFlareThres;    
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4PrvMinFlareThres    =     input->nvramData->rCCTConfig.u4PrvMinFlareThres;    
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoMaxFlareThres  =     input->nvramData->rCCTConfig.u4VideoMaxFlareThres;          
    // initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoMinFlareThres  =     input->nvramData->rCCTConfig.u4VideoMinFlareThres;   
 
    // memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rFlareSpec, &input->nvramData->rCCTConfig.rFlareSpec, sizeof(CCU_AE_FLARE_T));
    
    // {
    //     int i = 0;
    //     for(i = 0; i < 16; ++i)
    //     {
    //         LOG_DBG_MUST("uPrvFlareWeightArr : %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rFlareSpec.uPrvFlareWeightArr[i]);
    //     }
    //     LOG_DBG_MUST("u4VideoMinFlareThres : %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.u4VideoMinFlareThres);
        
    // }
    //memcpy(&initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec.rTOUCHFD_Spec, &input->nvramData->rCCTConfig.rMeteringSpec.rTOUCHFD_Spec, sizeof(CCU_AE_TOUCH_FD_SPEC_T));
    //LOG_DBG_MUST("AEIDBG m_AeCCTConfig.rMeteringSpec.rABL_Spec.bEnableBlackLight: %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec.rABL_Spec.bEnableBlackLight);
    // LOG_DBG_MUST("AEIDBG m_AeTuningParam.bEnableRotateWeighting: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.bEnableRotateWeighting);
    // LOG_DBG_MUST("AEIDBG m_AeTuningParam.pBVAccRatio: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pBVAccRatio.i4DeltaBVRatio);
    // LOG_DBG_MUST("AEIDBG m_AeTuningParam.pBVAccRatio: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.pBVAccRatio.u4B2T_Target);

    // LOG_DBG_MUST("AEIDBG m_AeCCTConfig.u4MeteringStableMax: %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMax);
    // LOG_DBG_MUST("AEIDBG m_AeCCTConfig.u4MeteringStableMin: %x", initParam->ccuAeInitParam.algo_init_param.m_AeCCTConfig.rMeteringSpec.rTOUCHFD_Spec.u4MeteringStableMin);
    // LOG_DBG_MUST("AEIDBG strAEParasetting.u4FinerEvIndexBase: %x", initParam->ccuAeInitParam.algo_init_param.m_AeTuningParam.strAEParasetting.u4FinerEvIndexBase);
    
    //save data ptr for exif dump using
    m_CcuAeExifDataPtr.m_LumLog2x1000 = input->ccuAeInitParam.algo_init_param.m_LumLog2x1000;
    m_CcuAeExifDataPtr.pCurrentTableF = input->ccuAeInitParam.algo_init_param.pCurrentTableF;
    m_CcuAeExifDataPtr.pCurrentTable = input->ccuAeInitParam.algo_init_param.pCurrentTable;
    m_CcuAeExifDataPtr.pEVValueArray = input->ccuAeInitParam.algo_init_param.pEVValueArray;
    m_CcuAeExifDataPtr.pAETouchMovingRatio = input->ccuAeInitParam.algo_init_param.pAETouchMovingRatio;
    m_CcuAeExifDataPtr.pAeNVRAM = input->ccuAeInitParam.algo_init_param.pAeNVRAM;

    LOG_DBG("+:%s\n",__FUNCTION__);

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_OnchangeData(void *inDataPtr, void *inDataBuf)
{
    ccu_ae_onchange_data *onchangeDataIn = (ccu_ae_onchange_data *)inDataPtr;

    //save data ptr for exif dump using
    m_CcuAeExifDataPtr.pCurrentTable = onchangeDataIn->pCurrentTable;
    m_CcuAeExifDataPtr.pCurrentTableF = onchangeDataIn->pCurrentTableF;

    return true;
}

bool CcuCtrlAe::ccuCtrlPreprocess_SensorInit(void *inDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    SENSOR_INFO_IN_T *infoIn = (SENSOR_INFO_IN_T *)inDataPtr;
    struct ccu_i2c_buf_mva_ioarg i2cBufIoArg;
    int32_t currentFpsList[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    struct ccu_i2c_info sensorI2CInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM] = {0};
    MUINT32 sensor_nm_idx = CcuDrvUtil::sensorIdxToNonmapIdx(m_sensorIdx);

    //======== Set i2c i2cControllerId of cam type to make kernel driver select correct i2c driver =========
    LOG_INF_MUST("Sensor Type: %d map %d\n", m_sensorIdx, sensor_nm_idx);

    //======== Get Sensor I2c Slave Address =========
    if (!m_pDrvCcu->getSensorI2CInfo(&sensorI2CInfo[0]))
    {
        LOG_ERR("getSensorI2cSlaveAddr fail \n");
        m_pDrvCcu->TriggerAee("getSensorI2cSlaveAddr failed");
        return false;
    }
    
    for (int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; ++i)
    {
        LOG_DBG("[CCU If] Sensor %d slave addr : %d, i2c_id : %d\n",i, sensorI2CInfo[i].slave_addr ,sensorI2CInfo[i].i2c_id);
    }

    bool slaveAddrValid = true;

    if(sensorI2CInfo[sensor_nm_idx].slave_addr == 0)
        slaveAddrValid = false;


    if(slaveAddrValid == false)
    {
        for (int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM ; i++)
        {
            LOG_ERR("[CCU If]Sensor %d slave addr : %d\n", i, currentFpsList[i]);
        }
        m_pDrvCcu->TriggerAee("CCU Sensor slave addr error");
    }
    
    //======== Get i2c dma buffer mva =========
    i2cBufIoArg.sensor_idx = sensor_nm_idx;
    i2cBufIoArg.i2c_id = sensorI2CInfo[sensor_nm_idx].i2c_id;

    LOG_DBG_MUST("skip get i2c bufaddr\n");
    if (!m_pDrvCcu->getI2CDmaBufferAddr(&i2cBufIoArg))
    {
        LOG_ERR("getI2CDmaBufferAddr fail \n");
        m_pDrvCcu->TriggerAee("getI2CDmaBufferAddr failed");
        return false;
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

    //======== Call CCU to initial sensor drv =========
    //Convert to compatible structure
    COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)inDataBuf;

    //compatInfoIn->u32SensorId = infoIn->u32SensorId;
    compatInfoIn->u16FPS = currentFpsList[sensor_nm_idx];
    compatInfoIn->sensorI2cSlaveAddr = sensorI2CInfo[sensor_nm_idx].slave_addr;
    compatInfoIn->eScenario    = infoIn->eScenario;
    compatInfoIn->dma_buf_mva  = i2cBufIoArg.mva;
    //put 3p9 indicator in i2c dma address high part (since high part is not used)
    compatInfoIn->dma_buf_va_h = i2cBufIoArg.va_h;
    compatInfoIn->dma_buf_va_l = i2cBufIoArg.va_l;
    compatInfoIn->u32I2CId     = i2cBufIoArg.i2c_id;
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

    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}

bool CcuCtrlAe::ccuCtrlPostprocess_SensorInit(void *outDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    COMPAT_SENSOR_INFO_IN_T *compatInfoIn = (COMPAT_SENSOR_INFO_IN_T *)inDataBuf;
    SENSOR_INFO_OUT_T *infoOut = (SENSOR_INFO_OUT_T *)outDataPtr;

    LOG_DBG_MUST("ccu sensor init cmd done, Sensor Type: %x\n", m_sensorIdx);
    LOG_DBG_MUST("ccu sensor out, u8SupportedByCCU: %x\n", infoOut->u8SupportedByCCU);

    if(infoOut->u8SupportedByCCU == 0)
    {
        LOG_ERR("ccu not support sensor\n");
        m_pDrvCcu->TriggerAee("ccu not support sensor");
        return false;
    }  

    //if current sensor is 3p9, mark it in u8SupportedByCCU variable to let AeMgr know that
    if(m_isSpSensor == true)
    {
        infoOut->u8SupportedByCCU |= 0x10;
    }

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

};  //namespace NSCcuIf
