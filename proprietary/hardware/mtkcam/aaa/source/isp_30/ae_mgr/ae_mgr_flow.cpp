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
    pinput->u4VHDRratio = m_u4VHDRratio;
    pinput->i4AF2AE_IsAFDone = m_i4AF2AE_IsAFDone;
    pinput->i4AF2AE_AfDac = m_i4AF2AE_AfDac;
    pinput->i4AF2AE_MagicNum = m_i4AF2AE_MagicNum;
    pinput->i4AF2AE_IsSceneStable = m_i4AF2AE_IsSceneStable;
    pinput->u4AEMode = m_eAEMode;
    pinput->u4TargetExp = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime;
    pinput->u4TargetISO = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity;
    pinput->u41xGainISO = m_p3ANVRAM->rAENVRAM[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    pinput->u4MinGain = m_p3ANVRAM->rAENVRAM[m_u4AENVRAMIdx].rDevicesInfo.u4MinGain;
    pinput->u4MaxSensorGain = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4IndexMax].u4AfeGain;
    pinput->u4MaxISPGain = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4IndexMax].u4IspGain;
    pinput->u4MaxShutter = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4IndexMax].u4Eposuretime;
    pinput->u4FinerEVIdxBase = m_u4FinerEVIdxBase;
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

