/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "ae_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"


using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;

#define AE_LOG(fmt, arg...) \
    do { \
        if (m_i4DgbLogE) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_LOG_IF(cond, ...) \
    do { \
        if (m_i4DgbLogE) { \
            CAM_LOGE_IF(cond, __VA_ARGS__); \
        } else { \
            CAM_LOGD_IF(cond, __VA_ARGS__); \
        } \
    }while(0)

#define AE_REFACTORING 0


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                           CCU flow implement                               //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MRESULT AeMgr::prepareCCUPvParam(AE_FLOW_CONTROL_INFO_T* pinput)
{
    pinput->pPreviewTableF = &m_PreviewTableF;
    pinput->pPreviewTableCurrent = m_pPreviewTableCurrent;
    pinput->eAEEVcomp = m_eAEEVcomp;
    pinput->bZoomChange = m_eZoomWinInfo.bZoomChange;
    pinput->bskipCCUAlgo = ((m_eAEMode == LIB3A_AE_MODE_OFF)||m_bAELock||m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_ADB_LOCK));
    pinput->u4IndexFMax = m_u4IndexFMax;
    pinput->u4IndexFMin = m_u4IndexFMin;
    pinput->i4AEMaxFps = m_i4AEMaxFps;
    pinput->u4HwMagicNum = m_u4HwMagicNum;
    pinput->i4ZoomRatio = m_i4ZoomRatio;
    return S_AE_OK;
}

MRESULT AeMgr::prepareCCUStartParam(AE_FLOW_INPUT_INFO_T* pinput)
{
    pinput->pAEInitInput = &m_rAEInitInput;
    pinput->pPreviewTableF = &m_PreviewTableF;
    pinput->pPreviewTableCurrent = m_pPreviewTableCurrent;
    pinput->pISPNvramISOInterval = &(m_rISPNvramISOInterval);
    pinput->pISPNvramOBC_Table = &(m_rISPNvramOBC_Table[0]);
    pinput->i4SensorIdx = m_i4SensorIdx;
    pinput->eSensorMode = m_eSensorMode;
    pinput->u4AAOmode = m_i4AAOmode;
    pinput->u4FinerEVIdxBase = m_u4FinerEVIdxBase;
    pinput->u4IndexFMax = m_u4IndexFMax;
    pinput->u4IndexFMin = m_u4IndexFMin;
    return S_AE_OK;
}


MRESULT AeMgr::copyCCUAEInfo2mgr(AE_CORE_MAIN_OUT* pCCUoutput)
{
    if ((m_eAEMode != LIB3A_AE_MODE_OFF) && (pCCUoutput->enumCpuAction == AE_CPU_ACTION_DO_FULL_AE)){

        AE_LOG( "[%s() CCU Nextidx %d->%d]\n", __FUNCTION__, m_i4AEidxNext, pCCUoutput->a_Output.i4AEidxNext);

        //Fixme: check strAEOutput is differ from strCcuAeOutput
        m_rAEOutput.rPreviewMode.u4Eposuretime = pCCUoutput->a_Output.EvSetting.u4Eposuretime;
        m_rAEOutput.rPreviewMode.u4AfeGain = pCCUoutput->a_Output.EvSetting.u4AfeGain;
        m_rAEOutput.rPreviewMode.u4IspGain = pCCUoutput->a_Output.EvSetting.u4IspGain;
        m_i4AEidxNext = pCCUoutput->a_Output.i4AEidxNext;
        m_i4AEidxCurrent = pCCUoutput->a_Output.i4AEidxCurrent;
        m_i4EVvalue = pCCUoutput->a_Output.i4EV;
        m_BVvalue = pCCUoutput->a_Output.Bv;
        m_rAEOutput.rPreviewMode.u4RealISO = pCCUoutput->a_Output.u4ISO;
        m_AOECompBVvalue = pCCUoutput->a_Output.AoeCompBv;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_COUNTWAIT, MFALSE);
        m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax ,m_u4IndexMin ,m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_i4AEidxNext);
        updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
        // Update to CPU AE algorithm
        AE_CORE_CTRL_CPU_EXP_INFO rAECoreCtrlCPUInfo;
        memset(&rAECoreCtrlCPUInfo, 0, sizeof(AE_CORE_CTRL_CPU_EXP_INFO));
        rAECoreCtrlCPUInfo.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
        rAECoreCtrlCPUInfo.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
        rAECoreCtrlCPUInfo.u4IspGain = m_rAEOutput.rPreviewMode.u4IspGain;
        rAECoreCtrlCPUInfo.m_u4EffectiveIndex = m_i4AEidxNext;
        m_pIAeFlowCPU->update(&rAECoreCtrlCPUInfo);
    }else {

        if (m_eAEMode != LIB3A_AE_MODE_OFF && (pCCUoutput->a_Output.EvSetting.u4Eposuretime!=0) &&
            (pCCUoutput->a_Output.EvSetting.u4AfeGain!=0)&&(pCCUoutput->a_Output.EvSetting.u4IspGain!=0)){
            m_rAEOutput.rPreviewMode.u4Eposuretime = pCCUoutput->a_Output.EvSetting.u4Eposuretime;
            m_rAEOutput.rPreviewMode.u4AfeGain = pCCUoutput->a_Output.EvSetting.u4AfeGain;
            m_rAEOutput.rPreviewMode.u4IspGain = pCCUoutput->a_Output.EvSetting.u4IspGain;

            updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
        }
    }
    m_pCCUresultCB->doNotifyCb(0,NULL,NULL,NULL);
    return S_AE_OK;
}

MRESULT AeMgr::setAEInfo2CCU(MUINT32 exp, MUINT32 afe, MUINT32 isp, MUINT32 iso, MUINT32 effective_idx, MUINT32 idx, MUINT32 delata_idx)
{
    (void)idx;
    if(m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)){

    //per-frame

    AE_CORE_CTRL_CPU_EXP_INFO rAECoreCtrlCPUInfo;
    memset(&rAECoreCtrlCPUInfo, 0, sizeof(AE_CORE_CTRL_CPU_EXP_INFO));

    rAECoreCtrlCPUInfo.u4CpuReady = 1;
    rAECoreCtrlCPUInfo.m_u4EffectiveIndex = effective_idx;
    rAECoreCtrlCPUInfo.m_i4deltaIndex = delata_idx;
    rAECoreCtrlCPUInfo.u4Eposuretime = exp;
    rAECoreCtrlCPUInfo.u4AfeGain = afe;
    rAECoreCtrlCPUInfo.u4IspGain = isp;
    rAECoreCtrlCPUInfo.u4ISO = iso;

    AE_LOG_IF(m_3ALogEnable, "[%s()] u4CpuReady: 0x%x m_u4EffectiveIndex: 0x%x m_i4deltaIndex: 0x%x u4Eposuretime: 0x%x u4AfeGain: 0x%x u4IspGain: 0x%x u4ISO: 0x%x\n", __FUNCTION__,
              rAECoreCtrlCPUInfo.u4CpuReady, rAECoreCtrlCPUInfo.m_u4EffectiveIndex, rAECoreCtrlCPUInfo.m_i4deltaIndex,
              rAECoreCtrlCPUInfo.u4Eposuretime, rAECoreCtrlCPUInfo.u4AfeGain, rAECoreCtrlCPUInfo.u4IspGain, rAECoreCtrlCPUInfo.u4ISO);

    m_pIAeFlowCCU->update(&rAECoreCtrlCPUInfo);

    }
    return S_AE_OK;
}


MBOOL AeMgr::ShouldCCUStart()
{
    MBOOL res = MFALSE;
    if((m_eAETargetMode==AE_MODE_NORMAL)&& \
        (!m_bStereoManualPline)&& \
        (m_i4SMSubSamples == 1))
    {
        res = MTRUE;
    }
    else
    {
        AE_LOG_IF(m_3ALogEnable, "[CCU start condition not reached]\n");

    }
    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                           CPU flow implement                               //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MRESULT AeMgr::prepareCPUStartParam(AE_FLOW_CPUSTART_INFO_T* pinput)
{
    pinput->pIAeAlgo = (MVOID* )m_pIAeAlgo;
    pinput->eAETargetMode = m_eAETargetMode;
    pinput->pAEInitInput = &m_rAEInitInput.rAEPARAM;
    pinput->pAEInitInputNVRAM = &(m_p3ANVRAM->rAENVRAM[m_u4AENVRAMIdx]);
    pinput->i4CycleVDNum = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1;
    pinput->i4ShutterDelayFrames = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET - m_i4ShutterDelayFrames;
    pinput->i4GainDelayFrames = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET - m_i4SensorGainDelayFrames;
    pinput->i4IspGainDelayFrames = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET;
    pinput->u4Index = m_u4Index;
    pinput->u4Exp = m_rAEOutput.rPreviewMode.u4Eposuretime;
    pinput->u4Afe = m_rAEOutput.rPreviewMode.u4AfeGain;
    pinput->u4Isp = m_rAEOutput.rPreviewMode.u4IspGain;
    pinput->u2VCModeSelect = m_rSensorVCInfo.u2VCModeSelect;

    return S_AE_OK;
}

MRESULT AeMgr::prepareSchedulerParam(AE_FLOW_SCHEDULE_INFO_T* pinput, E_AE_FLOW_CPU_T etype, MBOOL bAAASchedule, MINT32 i4ActiveAEItem)
{
    pinput->eAEFlowType = etype;
    pinput->bAAASchedule = bAAASchedule;
    pinput->i4ActiveItem = i4ActiveAEItem;
    pinput->eAETargetMode = m_eAETargetMode;
    pinput->bPerframeAEFlag = m_bPerframeAEFlag;
    pinput->bTouchAEAreaChange = m_bTouchAEAreaChage;
    pinput->bAElimitor = m_bAElimitor;
    pinput->bAEReadyCapture = m_bAEReadyCapture;
    pinput->bAEOneShotControl = m_bAEOneShotControl;
    pinput->AFTouchROISet = m_bAFTouchROISet;
    AE_LOG_IF(m_3ALogEnable," AFTouchROISet : %d", m_bAFTouchROISet);
    if(m_pIAeAlgo != NULL){
     pinput->i4CwvY = m_pIAeAlgo->getBrightnessCenterWeightedValue();
    }
    else{
     pinput->i4CwvY = 0;
     AE_LOG_IF(m_3ALogEnable, "[%s()] m_pIAeAlgo is NULL ", __FUNCTION__);
    }
    pinput->u4CwvYStable = m_u4StableYValue;
    pinput->bFaceAEAreaChange = m_bFaceAEAreaChage;
    pinput->i4WaitVDNum = m_i4WaitVDNum;
    pinput->u4WOFDcnt = m_u4WOFDCnt;
    pinput->u4HwMagicNum = m_u4HwMagicNum;
    return S_AE_OK;
}

MRESULT AeMgr::prepareCalculateParam(AE_FLOW_CALCULATE_INFO_T* pinput, E_AE_FLOW_CPU_T etype, MVOID* pStatistic, MINT64 i8timestamp)
{
    pinput->eAEFlowType = etype;
    pinput->pAEStatisticBuf = pStatistic;
    pinput->i4AcceInfo = &m_i4AcceInfo[0];
    pinput->i4GyroInfo = &m_i4GyroInfo[0];
    pinput->bAEReadyCapture = m_bAEReadyCapture;
    pinput->bEnSWBuffMode = m_bEnSWBuffMode;
    pinput->i8TimeStamp = i8timestamp;
    pinput->bAElock = m_bAELock;
    pinput->bAEOneShotControl = m_bAEOneShotControl;
    pinput->bIs60Hz = (m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_60HZ) || ((m_eAEFlickerMode == LIB3A_AE_FLICKER_MODE_AUTO) && (m_eAEAutoFlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_60HZ));
    pinput->bRealISOSpeed = m_bRealISOSpeed;
    pinput->u4AEISOSpeed = m_u4AEISOSpeed;
    pinput->i4AF2AE_IsAFDone = m_i4AF2AE_IsAFDone;
    pinput->i4AF2AE_AfDac = m_i4AF2AE_AfDac;
    pinput->i4AF2AE_MagicNum = m_i4AF2AE_MagicNum;
    pinput->i4AF2AE_IsSceneStable = m_i4AF2AE_IsSceneStable;
    return S_AE_OK;
}


MRESULT AeMgr::copyScheduleInfo2mgr(AE_FLOW_SCHEDULE_OUTPUT_T* poutput, MBOOL& bCalc, MBOOL& bApply)
{
    bCalc        = *(poutput->pModifiedCalc);
    bApply       = *(poutput->pModifiedApply);
    m_bPerframeAEFlag = *(poutput->pModifiedPerframeFlag);
    m_i4WaitVDNum = *(poutput->pWaitVDNum);
    return S_AE_OK;
}

