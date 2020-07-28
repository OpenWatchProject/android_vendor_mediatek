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
#define LOG_TAG "AeFlowCCU"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdio.h>
#include <stdlib.h>
#include "AeFlowCCU.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
 #include <aaa_scheduler.h>
#include "n3d_sync2a_tuning_param.h"
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>


using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCcuIf;


IAeFlow*
AeFlowCCU::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    switch (sensor)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeFlowCCU singleton_main(ESensorDev_Main);
        AE_FLOW_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeFlowCCU singleton_main2(ESensorDev_MainSecond);
        AE_FLOW_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeFlowCCU singleton_sub(ESensorDev_Sub);
        AE_FLOW_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:        //  Sub Sensor
        static AeFlowCCU singleton_sub2(ESensorDev_SubSecond);
        AE_FLOW_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;

    }
}


AeFlowCCU::
AeFlowCCU(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_i4SensorIdx(0)
    , m_eSensorMode(ESensorMode_Preview)
    , m_pICcuAe(NULL)
    , m_bIsCCUStart(MFALSE)
    , m_bIsCCUWorking(MFALSE)
    , m_bIsCCUAEWorking(MFALSE)
    , m_bIsCCUPaused(MFALSE)
    , m_bIsCCUResultGet(MFALSE)
    , m_bCCUAEFlag(MTRUE)
    , m_bCCUIsSensorSupported(MFALSE)
    , m_bTriggerFrameInfoUpdate(MFALSE)
    , m_bTriggerOnchInfoUpdate(MFALSE)
    , m_bTriggerMaxFPSUpdate(MFALSE)
    , m_bTriggerFlickActiveUpdate(MFALSE)
    , m_bFlickActive(MFALSE)
    , m_3ALogEnable(MFALSE)
{
}

MVOID
AeFlowCCU::
init()
{
    AE_FLOW_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
}

MVOID
AeFlowCCU::
uninit()
{
    AE_FLOW_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);

    if (m_pICcuAe != NULL)
    {
        m_pICcuAe = NULL;
    }
}

MVOID getAdbProperty(AE_NVRAM_T* pNvram)
{
    char default_value[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
///////////////////////////in calFD40()///////////////////////////////////////////////////////////////////
    sprintf(default_value, "%d", pNvram->rAeParam.AEStableThd.u4FaceInStableThd);
    property_get("vendor.debug.face_ae.in_thd", value, default_value);
    pNvram->rAeParam.AEStableThd.u4FaceInStableThd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.AEStableThd.u4FaceOutB2TStableThd);
    property_get("vendor.debug.face_ae.out_thd_b2t", value, default_value);
    pNvram->rAeParam.AEStableThd.u4FaceOutB2TStableThd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.AEStableThd.u4FaceOutD2TStableThd);
    property_get("vendor.debug.face_ae.out_thd_d2t", value, default_value);
    pNvram->rAeParam.AEStableThd.u4FaceOutD2TStableThd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_InToOutThdMaxCnt);
    property_get("vendor.debug.face_ae.in_to_out_thd_cnt", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_InToOutThdMaxCnt = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBnd);
    property_get("vendor.debug.face_ae.thd_low_bnd", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBnd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBndNum);
    property_get("vendor.debug.face_ae.thd_low_bnd_num", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBndNum = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TEnd);
    property_get("vendor.debug.face_ae.b2t_end", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TEnd = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TStart);
    property_get("vendor.debug.face_ae.b2t_start", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TStart = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TEnd);
    property_get("vendor.debug.face_ae.d2t_end", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TEnd = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TStart);
    property_get("vendor.debug.face_ae.d2t_start", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TStart = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_FaceMotionLockRat);
    property_get("vendor.debug.face_ae.motion_lock_rat", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_FaceMotionLockRat = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_ContinueTrustCnt);
    property_get("vendor.debug.face_ae.cont_trust_cnt", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_ContinueTrustCnt = atoi(value);
    /* mark for P80
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4MaxFDYHighBound);
    property_get("vendor.debug.face_ae.y_high_bound", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4MaxFDYHighBound = atoi(value);
    */

}


MVOID
AeFlowCCU::
start(MVOID* pAEInitInput)
{
    if(!m_bIsCCUStart){
        AE_FLOW_CCU_INIT_INFO_T* pinitInfo = ((AE_FLOW_CCU_INIT_INFO_T*) pAEInitInput);
        m_i4SensorIdx = pinitInfo->i4SensorIdx;
        m_eSensorMode = pinitInfo->eSensorMode;
        m_pICcuAe = ICcuCtrlAe::getInstance(m_i4SensorIdx, m_eSensorDev);
        AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]:%d/%d/%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_i4SensorIdx, m_eSensorMode);

        // init algo
        CCU_AE_ALGO_INITI_PARAM_T rAEAlgoInitparam;
        memset(&rAEAlgoInitparam, 0, sizeof(CCU_AE_ALGO_INITI_PARAM_T));

        rAEAlgoInitparam.algo_init_param = pinitInfo->mInitData;

        //get AE sync init info
        //const strSyncAEInitInfo* syncAeInfo = getSyncAEInitInfo();

        //AE_FLOW_LOG("[%s()] syncInfo.EVOffset_main[0]: %x RGB2YCoef_main[0]: %x pDeltaBVtoRatioArray[last][last]: %x", __FUNCTION__,
        //    syncAeInfo->EVOffset_main[0],syncAeInfo->RGB2YCoef_main[0],syncAeInfo->pDeltaBVtoRatioArray[SYNC_DUAL_CAM_DENOISE_MAX-1][MAX_MAPPING_DELTABV_ISPRATIO-1]);

        //rAEAlgoInitparam.sync_algo_init_param = (CCU_strSyncAEInitInfo *)syncAeInfo;

        //AE_FLOW_LOG("[%s()] u2TotalNum=%d i4BvOffset = %d", __FUNCTION__,
        //rAEAlgoInitparam.algo_init_param.pCurrentGainList->u4TotalNum,
        //rAEAlgoInitparam.algo_init_param.prAeSyncNvram->i4EvOffset[0]);

        if(pinitInfo->bIsCCUAEInit){
          //init CCU AE
          if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_ALGO_INIT, &rAEAlgoInitparam, NULL))
          {
            AE_FLOW_LOG("ccu_ae_algo_initialize fail\n");
            m_bCCUIsSensorSupported = MFALSE;
            return;
          }
          AE_FLOW_LOG("ccu_ae_algo_initialize success\n");
        }else{
          AE_FLOW_LOG("No Need to initialize ccu_ae_algo\n");
        }
        /*int debugInfo;
        while(1)
        {
            debugInfo = m_pICcu->readInfoReg(20);
            AE_FLOW_LOG( "[%s()] bankdone debug: %x\n", __FUNCTION__, debugInfo);
            usleep(1000);
        }*/
        m_bTriggerFrameInfoUpdate   = MFALSE;
        m_bTriggerOnchInfoUpdate    = MFALSE;
        m_bTriggerMaxFPSUpdate      = MTRUE;
        m_bTriggerFlickActiveUpdate = MTRUE;
        m_bIsCCUStart = MTRUE;
        m_bIsCCUPaused = MFALSE;
        m_bIsCCUResultGet = MFALSE;
        m_bCCUIsSensorSupported = MTRUE;


    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEParamCFG cpu:%x ccu:%x\n", sizeof(strAEParamCFG), sizeof(CCU_strAEParamCFG) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strWeightTable cpu:%x ccu:%x\n", sizeof(strWeightTable), sizeof(CCU_strWeightTable) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%x ccu:%x\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%x ccu:%x\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%x ccu:%x\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%x ccu:%x\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strEVC cpu:%x ccu:%x\n", sizeof(strEVC), sizeof(CCU_strEVC) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%x ccu:%x\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%x ccu:%x\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%x ccu:%x\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%x ccu:%x\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAELimiterTable cpu:%x ccu:%x\n", sizeof(strAELimiterTable), sizeof(CCU_strAELimiterTable) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww VdoDynamicFrameRate_T cpu:%x ccu:%x\n", sizeof(VdoDynamicFrameRate_T), sizeof(CCU_VdoDynamicFrameRate_T) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww AE_HIST_WIN_CFG_T cpu:%x ccu:%x\n", sizeof(AE_HIST_WIN_CFG_T), sizeof(CCU_AE_HIST_WIN_CFG_T) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww AE_HIST_WIN_CFG_T cpu:%x ccu:%x\n", sizeof(AE_HIST_WIN_CFG_T), sizeof(CCU_AE_HIST_WIN_CFG_T) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww PS_HIST_WIN_CFG_T cpu:%x ccu:%x\n", sizeof(PS_HIST_WIN_CFG_T), sizeof(CCU_PS_HIST_WIN_CFG_T) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceLocSize cpu:%x ccu:%x\n", sizeof(strFaceLocSize), sizeof(CCU_strFaceLocSize) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strPerframeCFG cpu:%x ccu:%x\n", sizeof(strPerframeCFG), sizeof(CCU_strPerframeCFG) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAETgTuningPara cpu:%x ccu:%x\n", sizeof(strAETgTuningPara), sizeof(CCU_strAETgTuningPara) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strEVC cpu:%x ccu:%x\n", sizeof(strEVC), sizeof(CCU_strEVC) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strEVC cpu:%x ccu:%x\n", sizeof(strEVC), sizeof(CCU_strEVC) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEStableThd cpu:%x ccu:%x\n", sizeof(strAEStableThd), sizeof(CCU_strAEStableThd) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strBVAccRatio cpu:%x ccu:%x\n", sizeof(strBVAccRatio), sizeof(CCU_strBVAccRatio)   );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strPSOConverge cpu:%x ccu:%x\n", sizeof(strPSOConverge), sizeof(CCU_strPSOConverge) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strNonCWRAcc cpu:%x ccu:%x\n", sizeof(strNonCWRAcc), sizeof(CCU_strNonCWRAcc)    );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceSmooth cpu:%x ccu:%x\n", sizeof(strFaceSmooth), sizeof(CCU_strFaceSmooth)  );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceWeight cpu:%x ccu:%x\n", sizeof(strFaceWeight), sizeof(CCU_strFaceWeight)  );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAECWRTempSmooth cpu:%x ccu:%x\n", sizeof(strAECWRTempSmooth), sizeof(CCU_strAECWRTempSmooth)  );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceLandMarkCtrl cpu:%x ccu:%x\n", sizeof(strFaceLandMarkCtrl), sizeof(CCU_strFaceLandMarkCtrl) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAOEBVRef cpu:%x ccu:%x\n", sizeof(strAOEBVRef), sizeof(CCU_strAOEBVRef)     ); //16->8?

    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strNS_CDF cpu:%x ccu:%x\n", sizeof(strNS_CDF), sizeof(CCU_strNS_CDF)       );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMovingRatio cpu:%x ccu:%x\n", sizeof(strAEMovingRatio), sizeof(CCU_strAEMovingRatio) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHSFlatSkyCFG cpu:%x ccu:%x\n", sizeof(strHSFlatSkyCFG), sizeof(CCU_strHSFlatSkyCFG)     );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strABL_absDiff cpu:%x ccu:%x\n", sizeof(strABL_absDiff), sizeof(CCU_strABL_absDiff)      );
    //mark fo P80
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHSMultiStep cpu:%x ccu:%x\n", sizeof(strHSMultiStep), sizeof(CCU_strHSMultiStep)    );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHistStableCFG cpu:%x ccu:%x\n", sizeof(strHistStableCFG), sizeof(CCU_strHistStableCFG)  );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strOverExpoAOECFG cpu:%x ccu:%x\n", sizeof(strOverExpoAOECFG), sizeof(CCU_strOverExpoAOECFG) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strMainTargetCFG cpu:%x ccu:%x\n", sizeof(strMainTargetCFG), sizeof(CCU_strMainTargetCFG) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHSv4p0CFG cpu:%x ccu:%x\n", sizeof(strHSv4p0CFG), sizeof(CCU_strHSv4p0CFG) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strNSBVCFG cpu:%x ccu:%x\n", sizeof(strNSBVCFG), sizeof(CCU_strNSBVCFG) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMovingRatio cpu:%x ccu:%x\n", sizeof(strAEMovingRatio), sizeof(CCU_strAEMovingRatio) );
    //mark for P80
	//AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww HDR_AE_PARAM_T cpu:%x ccu:%x\n", sizeof(HDR_AE_PARAM_T), sizeof(CCU_HDR_AE_PARAM_T)    );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAESceneMapping cpu:%x ccu:%x\n", sizeof(strAESceneMapping), sizeof(CCU_strAESceneMapping) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAESceneMapping cpu:%x ccu:%x\n", sizeof(strAESceneMapping), sizeof(CCU_strAESceneMapping) );
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strDynamicFlare cpu:%x ccu:%x\n", sizeof(strDynamicFlare), sizeof(CCU_strDynamicFlare) );

    AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww NVRAM cpu:%x ccu:%x\n", sizeof(AE_NVRAM_T), sizeof(CCU_AE_NVRAM_T) );
    //mark for P80
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_eHDRPARAM_ID %x\n", sizeof(CCU_eHDRPARAM_ID));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_DETECTOR_T %x\n", sizeof(CCU_HDR_DETECTOR_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_AUTO_ENHANCE_T %x\n", sizeof(CCU_HDR_AUTO_ENHANCE_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_AE_TARGET_T %x\n", sizeof(CCU_HDR_AE_TARGET_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_MISC_PROP_T %x\n", sizeof(CCU_HDR_MISC_PROP_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_CHDR_T %x\n", sizeof(CCU_HDR_CHDR_T));
    AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AE_TARGET_PROB_T %x\n ", sizeof(CCU_AE_TARGET_PROB_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_BRIGHT_ENHANCE_T %x\n ", sizeof(CCU_AUTO_BRIGHT_ENHANCE_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_FLICKER_RATIO_T %x\n ", sizeof(CCU_AUTO_FLICKER_RATIO_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_SMOOTH_T %x\n ", sizeof(CCU_AUTO_SMOOTH_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_TUNING_T %x\n ", sizeof(CCU_AUTO_TUNING_T));
    //AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_FACE_T %x\n ", sizeof(CCU_HDR_FACE_T));
    }
}

MVOID
AeFlowCCU::
stop()
{
    //if CCU is under pause state, shutdown & uninit flow should also take place
    if (m_bIsCCUStart /*|| m_bIsCCUPaused*/){
        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);

        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_STOP, NULL, NULL))
        {
            AE_FLOW_LOG("MSG_TO_CCU_AE_STOP fail\n");
            return;
        }
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        m_bIsCCUResultGet = MFALSE;
        //m_bIsCCUPaused = MFALSE;
    }
}

MVOID
AeFlowCCU::
pause()
{
    if (m_bIsCCUStart){
        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);
        this->stop();
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        //m_bIsCCUPaused = MTRUE;
    }

}

MBOOL
AeFlowCCU::
queryStatus(MUINT32 index)
{
    E_AE_FLOW_CCU_STATUS_T eStatus = (E_AE_FLOW_CCU_STATUS_T) index;
    switch (eStatus)
    {
        case E_AE_FLOW_CCU_WORKING:
            return IsCCUWorking();
        case E_AE_FLOW_CCU_AE_WORKING:
            return IsCCUAEWorking();
        case E_AE_FLOW_CCU_SENSOR_SUPPORTED:
            return m_bCCUIsSensorSupported;
        case E_AE_FLOW_CCU_AE_RESULT_GET:
            return m_bIsCCUResultGet;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
            return MFALSE;
    }
}


MVOID
AeFlowCCU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_FLOW_CCU_TRIGER_T etrigger = (E_AE_FLOW_CCU_TRIGER_T) index;
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] m_eSensorDev: %d, index: %d, istrigger: %d \n", __FUNCTION__, m_eSensorDev, index, btrigger);
    switch (etrigger)
    {
        case E_AE_FLOW_CCU_TRIGGER_FRAME_INFO_UPDATE:
            m_bTriggerFrameInfoUpdate = MTRUE;
            break;
        case E_AE_FLOW_CCU_TRIGGER_ONCH_INFO_UPDATE:
            m_bTriggerOnchInfoUpdate = MTRUE;
            break;
        case E_AE_FLOW_CCU_TRIGGER_MAX_FPS_UPDATE:
            m_bTriggerMaxFPSUpdate = MTRUE;
            break;
        case E_AE_FLOW_CCU_TRIGGER_FLICKER_ACTIVE_UPDATE:
            if(m_bFlickActive != btrigger) {
                AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] FlickerState: %d -> %d \n", __FUNCTION__, m_bFlickActive, btrigger);
                m_bFlickActive = btrigger;
                m_bTriggerFlickActiveUpdate = MTRUE;
            }
            break;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
    }
}
MVOID
AeFlowCCU::
startCCUAE()
{
    if(!m_bIsCCUAEWorking && IsCCUWorking()){

        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);
        m_pICcuAe->ccuControl(MSG_TO_CCU_AE_START, NULL, NULL);

        m_bIsCCUAEWorking = MTRUE;
    }
}

MVOID
AeFlowCCU::
controlCCU(AE_FLOW_CCU_CONTROL_INFO_T* pcontrolInfo)
{
    if(!m_bIsCCUAEWorking && IsCCUWorking()){
        AE_FLOW_LOG( "[%s()] MSG_TO_CCU_AE_START\n", __FUNCTION__);
        m_pICcuAe->ccuControl(MSG_TO_CCU_AE_START, NULL, NULL);
        m_bIsCCUAEWorking = MTRUE;
    }

    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()+] m_bTriggerFrameInfoUpdate: %d, m_bTriggerOnchInfoUpdate: %d, m_bTriggerMaxFPSUpdate: %d, m_bTriggerFlickActiveUpdate: %d\n",
        __FUNCTION__, m_bTriggerFrameInfoUpdate, m_bTriggerOnchInfoUpdate, m_bTriggerMaxFPSUpdate, m_bTriggerFlickActiveUpdate);
    if(!IsCCUWorking() || !IsCCUAEWorking())
        return;

    /* control AE algo frame sync data */
    if(m_bTriggerFrameInfoUpdate) {
        // Fill frame sync data
        ccu_ae_ctrldata_perframe rCcuFrameSyncData;
        memset(&rCcuFrameSyncData, 0, sizeof(ccu_ae_ctrldata_perframe));
        rCcuFrameSyncData.force_reset_ae_status = pcontrolInfo->bForceResetCCUStable;
        rCcuFrameSyncData.algo_frame_data = pcontrolInfo->mFrameData;
        rCcuFrameSyncData.magic_num = pcontrolInfo->u4HwMagicNum;
        rCcuFrameSyncData.req_num = pcontrolInfo->u4RequestNum;
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] control AE algo frame sync data, magic_num:0x%x, req_num:0x%x, bForceResetCCUStable: %d\n", __FUNCTION__, rCcuFrameSyncData.magic_num, rCcuFrameSyncData.req_num, rCcuFrameSyncData.force_reset_ae_status);

//        rCcuFrameSyncData.do_manual = pcontrolInfo->bManualAE; // move to AeSettingCCU
//        rCcuFrameSyncData.manual_exp.u4Eposuretime = pcontrolInfo->u4ManualExp;
//        rCcuFrameSyncData.manual_exp.u4AfeGain = pcontrolInfo->u4ManualAfeGain;
//        rCcuFrameSyncData.manual_exp.u4IspGain = pcontrolInfo->u4ManualISPGain;
//        rCcuFrameSyncData.manual_exp.u4ISO = pcontrolInfo->u4ManualISO;
//        rCcuFrameSyncData.manual_exp.m_u4Index = pcontrolInfo->u4CurrentIndex;
//        rCcuFrameSyncData.manual_exp.m_u4IndexF = pcontrolInfo->u4CurrentIndexF;
//        if(rCcuFrameSyncData.do_manual) {
//            AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] manual exposure frame sync data, exposure setting: %d/%d/%d/%d, index/F: %d/%d\n", __FUNCTION__,
//                rCcuFrameSyncData.manual_exp.u4Eposuretime, rCcuFrameSyncData.manual_exp.u4AfeGain, rCcuFrameSyncData.manual_exp.u4IspGain, rCcuFrameSyncData.manual_exp.u4ISO,
//                rCcuFrameSyncData.manual_exp.m_u4Index, rCcuFrameSyncData.manual_exp.m_u4IndexF);
//        }

        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME, &rCcuFrameSyncData, NULL);
        m_bTriggerFrameInfoUpdate = MFALSE;
    }

    /* control AE algo on-change data */
    if(m_bTriggerOnchInfoUpdate) {
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] control AE algo on-change data, CurrentTableF: 0x%x, IdxBase: %d\n", __FUNCTION__,
            pcontrolInfo->mOnchData.pCurrentTableF, pcontrolInfo->mOnchData.m_u4FinerEVIdxBase);
        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE, &(pcontrolInfo->mOnchData), NULL);
        m_bTriggerOnchInfoUpdate = MFALSE;
    }

    /* control CCU max FPS change */
//    if(m_bTriggerMaxFPSUpdate) { // move to AeSettingCCU
//        ccu_max_framerate_data max_framerate_data;
//        max_framerate_data.framerate = (MUINT16)pcontrolInfo->i4AEMaxFps;
//        max_framerate_data.min_framelength_en = MTRUE;
//        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] control CCU max FPS change, m_i4AEMaxFps: 0x%x\n", __FUNCTION__, max_framerate_data.framerate);
//        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_MAX_FRAMERATE, &max_framerate_data, NULL);
//        m_bTriggerMaxFPSUpdate = MFALSE;
//    }

    /* control flicker active change */
    if(m_bTriggerFlickActiveUpdate){
        ccu_ae_auto_flk_data ae_auto_flk_data;
        ae_auto_flk_data.auto_flicker_en = m_bFlickActive;
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] control flicker active change, auto_flicker_en: %d\n", __FUNCTION__, ae_auto_flk_data.auto_flicker_en);
        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AUTO_FLK, &ae_auto_flk_data, NULL);
        m_bTriggerFlickActiveUpdate = MFALSE;
    }

    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()-]\n", __FUNCTION__);
}

MVOID
AeFlowCCU::
getCCUResult(MVOID* poutput)
{
    if(IsCCUWorking() && IsCCUAEWorking()){
        struct ccu_ae_output output;
        memset(&output, 0, sizeof(ccu_ae_output));

        //m_pICcuAe->ccuControl(MSG_TO_CCU_GET_CCU_OUTPUT, NULL, &output);

        m_pICcuAe->getCcuAeOutput(&output);
        memcpy(poutput, &output, sizeof(ccu_ae_output));

        AAA_TRACE_D("getCCUResult (%d)", output.is_ae_output_valid);
        m_bIsCCUResultGet = (output.is_ae_output_valid==1)?MTRUE:MFALSE;
        AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] is_ae_output_valid: %d IndexF: %d\n", __FUNCTION__,m_bIsCCUResultGet, output.algo_output.RealOutput.u4IndexF);
        AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] Index/RealBV/BVx1000/BV/AoeBV/EV: %d/%d/%d/%d/%d/%d\n", __FUNCTION__,
                       output.algo_output.RealOutput.u4Index, output.algo_output.i4RealBV, output.algo_output.i4RealBVx1000,
                       output.algo_output.i4Bv, output.algo_output.i4AoeCompBv, output.algo_output.i4EV);//, poutput->enumCpuAction, poutput->enumCcuAction);
        AAA_TRACE_END_D;
    }
}

MVOID
AeFlowCCU::getAEInfoForISP(AE_INFO_T &ae_info)
{
    m_pICcuAe->getAeInfoForIsp(&ae_info);
}

MVOID
AeFlowCCU::getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info)
{
    m_pICcuAe->getDebugInfo((AE_DEBUG_INFO_T*)exif_info, (AE_DEBUG_DATA_T*)dbg_data_info);
}

MUINT
AeFlowCCU::
getSensorId(MUINT m_i4SensorIdx, MUINT m_eSensorDev)
{
    (void)m_i4SensorIdx;

    MUINT sensorId;

    // Query TG info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    if(!pIHalSensorList) {
        CAM_LOGE("[%s] Can not create SensorHal list\n", __FUNCTION__);
        return E_AAA_SENSOR_NULL;
    }

    SensorStaticInfo sInfo;

    switch(m_eSensorDev) {
        case ESensorDev_Main:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &sInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &sInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &sInfo);
            break;
        case ESensorDev_SubSecond:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &sInfo);
            break;
        default:    //  Shouldn't happen.
            AE_FLOW_LOG("Invalid sensor device: %d", m_eSensorDev);
    }

    sensorId = sInfo.sensorDevID;

    return sensorId;
}

MBOOL
AeFlowCCU::
IsCCUWorking()
{
    MBOOL res = MFALSE;
    if (m_bIsCCUStart){
        res = MTRUE;
    }
    m_bIsCCUWorking = res;
    return res;
}

MBOOL
AeFlowCCU::
IsCCUAEWorking()
{
    return m_bIsCCUAEWorking;
}

AeFlowCCU::
~AeFlowCCU()
{

    AE_FLOW_LOG("Delete ~AeFlowCCU - DeviceId:%d",(MUINT32)m_eSensorDev);

}


