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
#define LOG_TAG "AeFlowCustom"

#include <stdlib.h>
#include "AeFlowCustom.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace NS3Av3;
using namespace NSCcuIf;

IAeFlow*
AeFlowCustom::
getInstance(ESensorDev_T eSensorDev)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)eSensorDev);

    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static AeFlowCustom singleton_main(ESensorDev_Main, AE_CORE_CAM_ID_MAIN);
        AE_FLOW_LOG("ESensorDev_Main(%p)", &singleton_main);
        return &singleton_main;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static AeFlowCustom singleton_main2(ESensorDev_MainSecond, AE_CORE_CAM_ID_MAIN2);
        AE_FLOW_LOG("ESensorDev_MainSecond(%p)", &singleton_main2);
        return &singleton_main2;
    case ESensorDev_Sub:        //  Sub Sensor
        static AeFlowCustom singleton_sub(ESensorDev_Sub, AE_CORE_CAM_ID_MAIN_SUB);
        AE_FLOW_LOG("ESensorDev_Sub(%p)", &singleton_sub);
        return &singleton_sub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        static AeFlowCustom singleton_sub2(ESensorDev_SubSecond, AE_CORE_CAM_ID_MAIN_SUB); // temp: currently without sub2 enum
        AE_FLOW_LOG("ESensorDev_Sub2(%p)", &singleton_sub2);
        return &singleton_sub2;
    }
}

AeFlowCustom::
AeFlowCustom(ESensorDev_T eSensorDev, AE_CORE_CAM_ID_ENUM eCamID)
    : m_eSensorDev(eSensorDev)
    , m_i4SensorIdx(0)
    , m_pIAeAlgo(NULL)
    , m_pICcuAe(NULL)
    , m_bIsCCUStart(MFALSE)
    , m_bIsCCUWorking(MFALSE)
    , m_bIsCCUAEWorking(MFALSE)
    , m_bCCUIsSensorSupported(MFALSE)
    , m_bIsCCUStatReady(MFALSE)
    , m_3ALogEnable(0)
    , m_bTriggerFrameInfoUpdate(MFALSE)
    , m_bTriggerOnchInfoUpdate(MFALSE)
{
    m_pIAeAlgo = ICustomAeAlgo::getInstance(m_eSensorDev);
    AE_FLOW_LOG("Enter AeFlowCustom - m_eSensorDev:%d", (MUINT32)m_eSensorDev);
}

AeFlowCustom::
~AeFlowCustom()
{
    AE_FLOW_LOG("Delete ~AeFlowCustom - m_eSensorDev:%d", (MUINT32)m_eSensorDev);
}

MVOID
AeFlowCustom::
start(MVOID* input)
{
    AE_FLOW_LOG("[%s +] m_eSensorDev:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);

    AE_FLOW_CPU_INIT_INFO_T* pCPUInitInfo = (AE_FLOW_CPU_INIT_INFO_T*) input;

    if(!m_bIsCCUStart)
    {
        // init algo
        CCU_AE_ALGO_INITI_PARAM_T rAEAlgoInitparam;
        memset(&rAEAlgoInitparam, 0, sizeof(CCU_AE_ALGO_INITI_PARAM_T));
        rAEAlgoInitparam.algo_init_param = pCPUInitInfo->mInitData;

        m_i4SensorIdx = pCPUInitInfo->i4SensorIdx;
        m_pICcuAe = ICcuCtrlAe::getInstance(m_i4SensorIdx, m_eSensorDev);
        if(pCPUInitInfo->bIsCCUAEInit)
        {
            //init CCU AE
            if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_ALGO_INIT, &rAEAlgoInitparam, NULL))
            {
                AE_FLOW_LOG("ccu_ae_algo_initialize fail\n");
                m_bCCUIsSensorSupported = MFALSE;
                return;
            }
            AE_FLOW_LOG("ccu_ae_algo_initialize success\n");
        }
        else
        {
            AE_FLOW_LOG("No Need to initialize ccu_ae_algo\n");
        }

        m_bTriggerFrameInfoUpdate   = MFALSE;
        m_bTriggerOnchInfoUpdate    = MFALSE;
        m_bIsCCUStart = MTRUE;
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
        //AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww HDR_AE_PARAM_T cpu:%x ccu:%x\n", sizeof(HDR_AE_PARAM_T), sizeof(CCU_HDR_AE_PARAM_T)    );
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAESceneMapping cpu:%x ccu:%x\n", sizeof(strAESceneMapping), sizeof(CCU_strAESceneMapping) );
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAESceneMapping cpu:%x ccu:%x\n", sizeof(strAESceneMapping), sizeof(CCU_strAESceneMapping) );
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strDynamicFlare cpu:%x ccu:%x\n", sizeof(strDynamicFlare), sizeof(CCU_strDynamicFlare) );

        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww NVRAM cpu:%x ccu:%x\n", sizeof(AE_NVRAM_T), sizeof(CCU_AE_NVRAM_T) );

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
    m_pIAeAlgo->CUS_AE_Core_Init(&(pCPUInitInfo->mInitData));
    AE_FLOW_LOG("[%s -]\n", __FUNCTION__);
}

MVOID
AeFlowCustom::
stop()
{
    //if CCU is under pause state, shutdown & uninit flow should also take place
    if (m_bIsCCUStart)
    {
        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);

        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_STAT_STOP, NULL, NULL))
        {
            AE_FLOW_LOG("MSG_TO_CCU_AE_STAT_STOP fail\n");
        }
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        m_bIsCCUStatReady = MFALSE;
    }
    
    if(m_pIAeAlgo != NULL){
        AE_FLOW_LOG( "[%s() Cust AE algo uninit]\n", __FUNCTION__);
        m_pIAeAlgo->CUS_AE_Core_Uninit();
    } else {
        AE_FLOW_LOG( "%s() Cust AE Algo NULL\n", __FUNCTION__);    
    }
}

MVOID
AeFlowCustom::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_FLOW_CPU_TRIGER_T etrigger = (E_AE_FLOW_CPU_TRIGER_T) index;
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] m_eSensorDev: %d, index: %d, istrigger: %d \n", __FUNCTION__, m_eSensorDev, index, btrigger);
    switch (etrigger)
    {
        case E_AE_FLOW_CPU_TRIGGER_FRAME_INFO_UPDATE:
            m_bTriggerFrameInfoUpdate = MTRUE;
            break;
        case E_AE_FLOW_CPU_TRIGGER_ONCH_INFO_UPDATE:
            m_bTriggerOnchInfoUpdate = MTRUE;
            break;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
    }
}

MBOOL
AeFlowCustom::
calculateAE(MVOID* input, MVOID* output)
{
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s +] m_eSensorDev:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    AE_FLOW_CPU_CALCULATE_INFO_T* pInput = (AE_FLOW_CPU_CALCULATE_INFO_T*)input;
    AE_CORE_MAIN_OUT* pOutput = (AE_CORE_MAIN_OUT*) output;

    if (m_bTriggerFrameInfoUpdate) {
        m_pIAeAlgo->CUS_AE_Core_Ctrl(AE_CTRL_SET_VSYNC_INFO, (void*)&(pInput->mVsyncInfo), 0);
        m_bTriggerFrameInfoUpdate = MFALSE;
    }
    if (m_bTriggerOnchInfoUpdate) {
        m_pIAeAlgo->CUS_AE_Core_Ctrl(AE_CTRL_SET_RUN_TIME_INFO, (void*)&(pInput->mRunTimeInfo), 0);
        m_bTriggerOnchInfoUpdate = MFALSE;
    }

    // start statistics processing
    if(!m_bIsCCUAEWorking && IsCCUWorking())
    {
        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_STAT_START, NULL, NULL))
        {
            AE_FLOW_LOG("MSG_TO_CCU_AE_STAT_START fail\n");
            return MFALSE;
        }
        AE_FLOW_LOG_IF(m_3ALogEnable,  "[%s()] MSG_TO_CCU_AE_STAT_START success\n", __FUNCTION__);
        m_bIsCCUAEWorking = MTRUE;
        return MFALSE;
    }

    if (m_bIsCCUStatReady)
    {
        // get statistics from CCU
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] statistic is ready (0x%x)\n", __FUNCTION__, pInput->mAEMainIn.pCustStatData);
        CCU_AE_STAT rCustStatData;
        pInput->mAEMainIn.pCustStatData = (MVOID*)(&rCustStatData);
        if(!getCCUStat((CCU_AE_STAT*)pInput->mAEMainIn.pCustStatData)) {
            AE_FLOW_LOG("[%s] can't get CCU AE stat data, skip AE calculation\n", __FUNCTION__);
            return MFALSE;
        }

        // algo main
        m_pIAeAlgo->CUS_AE_Core_Main(&(pInput->mAEMainIn), pOutput);
    }
    else
    {
        // skip first frame until statistic is ready
        m_bIsCCUStatReady = MTRUE;
        return MFALSE;
    }

    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s -]\n", __FUNCTION__);
    return MTRUE;
}

MVOID
AeFlowCustom::
getAEInfoForISP(AE_INFO_T &ae_info)
{
    if(m_pIAeAlgo != NULL){
        AE_FLOW_LOG( "[%s() Get Cust AE Info  for ISP]\n", __FUNCTION__);
        m_pIAeAlgo->CUS_AE_Core_ISPInfo(ae_info);
    } else {
        AE_FLOW_LOG( "%s() Cust AE Algo NULL\n", __FUNCTION__);    
    }
}

MVOID
AeFlowCustom::
getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info)
{
    if(m_pIAeAlgo != NULL){
        AE_FLOW_LOG( "[%s() Cust AE Debug Info]\n", __FUNCTION__);
        m_pIAeAlgo->CUS_AE_Core_DebugInfo(exif_info);
    } else {
        AE_FLOW_LOG( "%s() Cust AE Algo NULL\n", __FUNCTION__);    
    }
}

MBOOL
AeFlowCustom::
getCCUStat(CCU_AE_STAT* poutput)
{
    AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] IsCCUWorking(%d), IsCCUAEWorking(%d)\n", __FUNCTION__, IsCCUWorking(), IsCCUAEWorking());
    if(IsCCUWorking() && IsCCUAEWorking())
    {
        CCU_AE_STAT ae_stat;

        if(!m_pICcuAe->getCcuAeStat(&ae_stat)) {
            AE_FLOW_LOG("[%s()] can't get CCU AE stat data\n", __FUNCTION__);
            return MFALSE;
        }
        memcpy(poutput, &ae_stat, sizeof(CCU_AE_STAT));

        AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] stat_width(%d), stat_height(%d), stat_sample(%d/%d/%d/%d/%d)\n", __FUNCTION__, ae_stat.stat_width, ae_stat.stat_height,
                       ae_stat.y[1], ae_stat.y[3], ae_stat.y[5], ae_stat.y[7], ae_stat.y[9]);
    }
    return MTRUE;
}

MBOOL
AeFlowCustom::
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
AeFlowCustom::
IsCCUAEWorking()
{
    return m_bIsCCUAEWorking;
}

