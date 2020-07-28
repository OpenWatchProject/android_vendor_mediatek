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
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include "camera_custom_msdk.h"
#include "ae_custom_transform.h"

using namespace NS3A;
using namespace NS3Av3;
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


#define AE_SET_ISP_INFO 0

MRESULT AeMgr::prepareSetStartParam(AE_SETTING_INPUT_INFO_T* pinput)
{
    pinput->i4SensorIdx   = m_i4SensorIdx;
    pinput->eSensorTG     = m_eSensorTG;
    pinput->eSensorMode   = m_eSensorMode;
    pinput->eAETargetMode = m_eAETargetMode;
    AE_LOG_IF(m_3ALogEnable,"[%s] m_i4SensorIdx: %d, m_eSensorTG: %d, m_eSensorMode: %d, m_eAETargetMode: %d\n",
              __FUNCTION__, pinput->i4SensorIdx, pinput->eSensorTG, pinput->eSensorMode, pinput->eAETargetMode);
    return S_AE_OK;
}

MRESULT AeMgr::startAESetting(E_AE_SETTING_TYPE_T eAESettingType)
{
    AE_LOG("[%s +] m_eSensorDev: %d, eAESettingType: %d\n", __FUNCTION__, m_eSensorDev, eAESettingType);
    AE_SETTING_INPUT_INFO_T rStartInfo;
    prepareSetStartParam(&rStartInfo);
    m_pIAeSettingCPU->start((MVOID*) &rStartInfo);
    if (eAESettingType != E_AE_SETTING_CPU) {
        m_pIAeSettingCCU->start((MVOID*) &rStartInfo);
    }
    AE_LOG("[%s -]\n", __FUNCTION__);
    return S_AE_OK;
}

MRESULT AeMgr::prepareSetControlParam(AE_SETTING_CONTROL_INFO_T* pinput, MBOOL bTriggerControlbyI2C)
{
    pinput->u4UpdateFrameRate_x10 = m_u4UpdateFrameRate_x10;
    pinput->rAERealSetting.m_AETargetMode = (CCU_AE_TargetMODE)m_eAETargetMode;
    pinput->rAERealSetting.u4ExposureMode = m_u4ExposureMode;
    if (m_eAETargetMode == AE_MODE_NORMAL) {
        if (!bTriggerControlbyI2C) { // force setting for capture
            pinput->rAERealSetting.EvSetting.u4Eposuretime = m_u4AESettingEXP[0];
            pinput->rAERealSetting.EvSetting.u4AfeGain     = m_u4AESettingAFE[0];
            pinput->rAERealSetting.EvSetting.u4IspGain     = m_u4AESettingISP[0];
        }
        else {
            pinput->rAERealSetting.EvSetting.u4Eposuretime = (m_u4AESettingEXP[m_i4ShutterDelayFrames]    != m_u4AESettingEXP[m_i4ShutterDelayFrames+1])    ? (m_u4AESettingEXP[m_i4ShutterDelayFrames])    : (0);
            pinput->rAERealSetting.EvSetting.u4AfeGain     = (m_u4AESettingAFE[m_i4SensorGainDelayFrames] != m_u4AESettingAFE[m_i4SensorGainDelayFrames+1]) ? (m_u4AESettingAFE[m_i4SensorGainDelayFrames]) : (0);
            pinput->rAERealSetting.EvSetting.u4IspGain     = (m_u4AESettingISP[m_i4IspGainDelayFrames]    != m_u4AESettingISP[m_i4IspGainDelayFrames+1])    ? (m_u4AESettingISP[m_i4IspGainDelayFrames])    : (0);
        }
    }
    else { // VHDR mode
        if (!bTriggerControlbyI2C) { // force setting for capture
            pinput->rAERealSetting.HdrEvSetting.i4LEExpo = m_u4AEVHDRSettingEXP_LE[0];
            pinput->rAERealSetting.HdrEvSetting.i4MEExpo = m_u4AEVHDRSettingEXP_ME[0];
            pinput->rAERealSetting.HdrEvSetting.i4SEExpo = m_u4AEVHDRSettingEXP_SE[0];
            pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain = m_u4AEVHDRSettingAFE_LE[0];
            pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain = m_u4AEVHDRSettingAFE_ME[0];
            pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain = m_u4AEVHDRSettingAFE_SE[0];
            pinput->rAERealSetting.HdrEvSetting.i4LEIspGain = m_u4AEVHDRSettingISP[0];
        }
        else {
            pinput->rAERealSetting.HdrEvSetting.i4LEExpo = m_u4AEVHDRSettingEXP_LE[m_i4ShutterDelayFrames];
            pinput->rAERealSetting.HdrEvSetting.i4MEExpo = m_u4AEVHDRSettingEXP_ME[m_i4ShutterDelayFrames];
            pinput->rAERealSetting.HdrEvSetting.i4SEExpo = m_u4AEVHDRSettingEXP_SE[m_i4ShutterDelayFrames];
            pinput->rAERealSetting.HdrEvSetting.i4LEAfeGain = m_u4AEVHDRSettingAFE_LE[m_i4SensorGainDelayFrames];
            pinput->rAERealSetting.HdrEvSetting.i4MEAfeGain = m_u4AEVHDRSettingAFE_ME[m_i4SensorGainDelayFrames];
            pinput->rAERealSetting.HdrEvSetting.i4SEAfeGain = m_u4AEVHDRSettingAFE_SE[m_i4SensorGainDelayFrames];
            pinput->rAERealSetting.HdrEvSetting.i4LEIspGain = m_u4AEVHDRSettingISP[m_i4IspGainDelayFrames];
        }
            pinput->bSensorAWBGain = bUpdateSensorAWBGain;
            pinput->i4Pass2SensorLVvalue = m_i4BVvalue + 50;
    }
    return S_AE_OK;
}

MRESULT AeMgr::updateAESetting(strAERealSetting &rAERealSetting)
{
    m_u4ExposureMode = rAERealSetting.u4ExposureMode;
    if (m_eAETargetMode == AE_MODE_NORMAL) {
        for (int i = MAX_AE_SETTING_NUM - 2 ; i >= 0; i--) {
            m_u4AESettingEXP[i+1] = m_u4AESettingEXP[i];
            m_u4AESettingAFE[i+1] = m_u4AESettingAFE[i];
            m_u4AESettingISP[i+1] = m_u4AESettingISP[i];
        }
        m_u4AESettingEXP[0] = rAERealSetting.EvSetting.u4Eposuretime;
        m_u4AESettingAFE[0] = rAERealSetting.EvSetting.u4AfeGain;
        m_u4AESettingISP[0] = rAERealSetting.EvSetting.u4IspGain;
        AE_LOG_IF( m_3ALogEnable,"[%s] AESetting[0]: %d/%d/%d, AESetting[1]: %d/%d/%d, AESetting[2]: %d/%d/%d\n", __FUNCTION__,
                   m_u4AESettingEXP[0], m_u4AESettingAFE[0], m_u4AESettingISP[0],
                   m_u4AESettingEXP[1], m_u4AESettingAFE[1], m_u4AESettingISP[1],
                   m_u4AESettingEXP[2], m_u4AESettingAFE[2], m_u4AESettingISP[2]);
    }
    else {
        for (int i = MAX_AE_SETTING_NUM - 2 ; i >= 0; i--) {
            m_u4AEVHDRSettingEXP_LE[i+1] = m_u4AEVHDRSettingEXP_LE[i];
            m_u4AEVHDRSettingEXP_ME[i+1] = m_u4AEVHDRSettingEXP_ME[i];
            m_u4AEVHDRSettingEXP_SE[i+1] = m_u4AEVHDRSettingEXP_SE[i];
            m_u4AEVHDRSettingAFE_LE[i+1] = m_u4AEVHDRSettingAFE_LE[i];
            m_u4AEVHDRSettingAFE_ME[i+1] = m_u4AEVHDRSettingAFE_ME[i];
            m_u4AEVHDRSettingAFE_SE[i+1] = m_u4AEVHDRSettingAFE_SE[i];
            m_u4AEVHDRSettingISP   [i+1] = m_u4AEVHDRSettingISP   [i];
        }
        m_u4AEVHDRSettingEXP_LE[0] = rAERealSetting.HdrEvSetting.i4LEExpo;
        m_u4AEVHDRSettingEXP_ME[0] = rAERealSetting.HdrEvSetting.i4MEExpo;
        m_u4AEVHDRSettingEXP_SE[0] = rAERealSetting.HdrEvSetting.i4SEExpo;
        m_u4AEVHDRSettingAFE_LE[0] = rAERealSetting.HdrEvSetting.i4LEAfeGain;
        m_u4AEVHDRSettingAFE_ME[0] = rAERealSetting.HdrEvSetting.i4MEAfeGain;
        m_u4AEVHDRSettingAFE_SE[0] = rAERealSetting.HdrEvSetting.i4SEAfeGain;
        m_u4AEVHDRSettingISP   [0] = rAERealSetting.HdrEvSetting.i4LEIspGain;
        AE_LOG_IF( m_3ALogEnable,"[%s] AEVHDRSetting[0]: LE(%d/%d/%d) ME(%d/%d/%d) SE(%d/%d/%d), AEVHDRSetting[1]: LE(%d/%d/%d) ME(%d/%d/%d) SE(%d/%d/%d), AEVHDRSetting[2]: LE(%d/%d/%d) ME(%d/%d/%d) SE(%d/%d/%d)\n", __FUNCTION__,
                   /* AEVHDRSetting[0] */
                   m_u4AEVHDRSettingEXP_LE[0], m_u4AEVHDRSettingAFE_LE[0], m_u4AEVHDRSettingISP[0],
                   m_u4AEVHDRSettingEXP_ME[0], m_u4AEVHDRSettingAFE_ME[0], m_u4AEVHDRSettingISP[0],
                   m_u4AEVHDRSettingEXP_SE[0], m_u4AEVHDRSettingAFE_SE[0], m_u4AEVHDRSettingISP[0],
                   /* AEVHDRSetting[1] */
                   m_u4AEVHDRSettingEXP_LE[1], m_u4AEVHDRSettingAFE_LE[1], m_u4AEVHDRSettingISP[1],
                   m_u4AEVHDRSettingEXP_ME[1], m_u4AEVHDRSettingAFE_ME[1], m_u4AEVHDRSettingISP[1],
                   m_u4AEVHDRSettingEXP_SE[1], m_u4AEVHDRSettingAFE_SE[1], m_u4AEVHDRSettingISP[1],
                   /* AEVHDRSetting[2] */
                   m_u4AEVHDRSettingEXP_LE[2], m_u4AEVHDRSettingAFE_LE[2], m_u4AEVHDRSettingISP[2],
                   m_u4AEVHDRSettingEXP_ME[2], m_u4AEVHDRSettingAFE_ME[2], m_u4AEVHDRSettingISP[2],
                   m_u4AEVHDRSettingEXP_SE[2], m_u4AEVHDRSettingAFE_SE[2], m_u4AEVHDRSettingISP[2]);
    }
    return S_AE_OK;
}

MRESULT AeMgr::controlCPUExpoSetting(strAERealSetting &rAERealSetting, MBOOL bTriggerControlbyI2C)
{
    updateAESetting(rAERealSetting);
    if (!bTriggerControlbyI2C) { // trigger control setting immediately
        AE_SETTING_CONTROL_INFO_T rAESetControlInfo;
        prepareSetControlParam(&rAESetControlInfo);
        m_pIAeSettingCPU->updateExpoSetting((MVOID*) &rAESetControlInfo);
    }
    else { // trigger control setting by updateSensorbyI2C()
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
    }

    return S_AE_OK;
}

MBOOL AeMgr::controlCCUExpoSetting(strAERealSetting &rSettingAfterConv, strAERealSetting &rSettingBeforeConv, MBOOL bCcuPresetControl)
{
    updateAESetting(rSettingAfterConv);
    AE_SETTING_CONTROL_INFO_T rAESetControlInfo;
    prepareSetControlParam(&rAESetControlInfo);
    rAESetControlInfo.bCcuPresetControl = bCcuPresetControl;
    rAESetControlInfo.rAERealSetting.u4ISO = rSettingAfterConv.u4ISO;
    rAESetControlInfo.rAERealSetting.u4Index = rSettingAfterConv.u4Index;
    rAESetControlInfo.rAERealSetting.u4IndexF = rSettingAfterConv.u4IndexF;
    rAESetControlInfo.rAERealSetting.u4AEFinerEVIdxBase = rSettingAfterConv.u4AEFinerEVIdxBase;
    rAESetControlInfo.rAERealSetting.bResetIndex = rSettingAfterConv.bResetIndex;
    memcpy(&(rAESetControlInfo.rAERealSettingBeforeConv), &rSettingBeforeConv, sizeof(strAERealSetting));
    return m_pIAeSettingCCU->updateExpoSetting((MVOID*) &rAESetControlInfo);
}

MRESULT AeMgr::controlCPUSensorMaxFPS(MUINT32 u4SensorMaxFPS, MBOOL bTriggerControlbyI2C)
{
    if (u4SensorMaxFPS == 0 || m_u4UpdateFrameRate_x10 == u4SensorMaxFPS) {
        return S_AE_OK;
    }

    m_u4UpdateFrameRate_x10 = u4SensorMaxFPS;
    if (!bTriggerControlbyI2C) { // trigger control setting immediately
        MUINT32 u4MaxFPS = m_u4UpdateFrameRate_x10;
        m_pIAeSettingCPU->updateSensorMaxFPS((MVOID*) &u4MaxFPS);
    }
    else { // trigger control setting by updateSensorbyI2C()
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_FRAMERATE, MTRUE);
    }

    return S_AE_OK;
}

MRESULT AeMgr::controlCCUSensorMaxFPS(MUINT32 u4SensorMaxFPS)
{
    if (u4SensorMaxFPS == 0 || m_u4UpdateFrameRate_x10 == u4SensorMaxFPS) {
        return S_AE_OK;
    }

    m_u4UpdateFrameRate_x10 = u4SensorMaxFPS;
    MUINT32 u4MaxFPS = m_u4UpdateFrameRate_x10;
    m_pIAeSettingCCU->updateSensorMaxFPS((MVOID*) &u4MaxFPS);

    return S_AE_OK;
}

MRESULT AeMgr::UpdateSensorISPParams(AE_FLOW_STATE_T eAEFlowState, strAERealSetting &rAERealSetting)
{
    /* check setting convert */
    strAERealSetting rSettingBeforeConv; // setting before convert
    strAERealSetting rSettingAfterConv;  // setting after  convert
    memset(&rSettingBeforeConv, 0, sizeof(strAERealSetting));
    memset(&rSettingAfterConv,  0, sizeof(strAERealSetting));
    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT && eAEFlowState == AE_FLOW_STATE_PREVIEW && (!m_bAdbAEPreviewUpdate)) {
        /* since CCU has already do convert, in this case we take algo output as m_rSensorCurrentInfo */
        memcpy(&rSettingBeforeConv, &m_rAEAlgoOutput, sizeof(strAERealSetting));
        memcpy(&rSettingAfterConv,  &rAERealSetting,  sizeof(strAERealSetting));
    }
    else {
        memcpy(&rSettingBeforeConv, &rAERealSetting,  sizeof(strAERealSetting));
        memcpy(&rSettingAfterConv,  &rAERealSetting,  sizeof(strAERealSetting));
        /* customized transform function: binning sum transforamtion */
        if (m_u4BinSumRatio > 1 && m_eAETargetMode == AE_MODE_NORMAL)
            getBinSumTransSetting(&rSettingAfterConv, m_u4BinSumRatio);
        /* customized transform function: super night shot transforamtion */
        if (m_bSuperNightShotMode && eAEFlowState == AE_FLOW_STATE_PREVIEW && m_eAETargetMode == AE_MODE_NORMAL) {
            m_AeCustParam.u4FrameIndex += 1;
            AE_LOG( "[%s] super night shot transforamtion, m_eSensorDev: %d, u4FrameIndex: %d, input rSettingBeforeConv: %d/%d/%d (%d), input rSettingAfterConv: %d/%d/%d (%d)\n",
                     __FUNCTION__, m_eSensorDev, m_AeCustParam.u4FrameIndex,
                     rSettingBeforeConv.EvSetting.u4Eposuretime, rSettingBeforeConv.EvSetting.u4AfeGain, rSettingBeforeConv.EvSetting.u4IspGain, rSettingBeforeConv.u4ISO,
                     rSettingAfterConv.EvSetting.u4Eposuretime,  rSettingAfterConv.EvSetting.u4AfeGain,  rSettingAfterConv.EvSetting.u4IspGain,  rSettingAfterConv.u4ISO);
            transSuperNightShot(&m_AeCustParam, m_rAEInitInput.rAENVRAM, &rSettingBeforeConv);
            transSuperNightShot(&m_AeCustParam, m_rAEInitInput.rAENVRAM, &rSettingAfterConv );
        }
    }
    {   /* fill in m_rSensorCurrentInfo: AE setting before convert */
        m_rSensorCurrentInfo.u8FrameDuration = getFrameDuration(100*rSettingBeforeConv.u2FrameRate);
        m_rSensorCurrentInfo.u8ExposureTime  = 1000L*rSettingBeforeConv.EvSetting.u4Eposuretime;
        if(m_rSensorCurrentInfo.u8FrameDuration < m_rSensorCurrentInfo.u8ExposureTime)
            m_rSensorCurrentInfo.u8FrameDuration = m_rSensorCurrentInfo.u8ExposureTime;
        m_rSensorCurrentInfo.u4Sensitivity   = rSettingBeforeConv.u4ISO;
    }
    AE_LOG_IF( m_3ALogEnable, "[%s] m_eSensorDev: %d, m_rSensorCurrentInfo.u8FrameDuration/u8ExposureTime/u4Sensitivity: %lld/%lld/%d, rSettingBeforeConv: %d/%d/%d (%d), rSettingAfterConv: %d/%d/%d (%d)\n",
               __FUNCTION__, m_eSensorDev, (long long)m_rSensorCurrentInfo.u8FrameDuration, (long long)m_rSensorCurrentInfo.u8ExposureTime, m_rSensorCurrentInfo.u4Sensitivity,
               rSettingBeforeConv.EvSetting.u4Eposuretime, rSettingBeforeConv.EvSetting.u4AfeGain, rSettingBeforeConv.EvSetting.u4IspGain, rSettingBeforeConv.u4ISO,
               rSettingAfterConv.EvSetting.u4Eposuretime,  rSettingAfterConv.EvSetting.u4AfeGain,  rSettingAfterConv.EvSetting.u4IspGain,  rSettingAfterConv.u4ISO);

    /* remapping VHDR state */
    AE_FLOW_STATE_T _eAEFlowState = eAEFlowState;
    if (m_eAETargetMode != AE_MODE_NORMAL) {
        if      (_eAEFlowState == AE_FLOW_STATE_INIT   ) _eAEFlowState = AE_FLOW_STATE_VHDR_INIT;
        else if (_eAEFlowState == AE_FLOW_STATE_PREVIEW) _eAEFlowState = AE_FLOW_STATE_VHDR_PREVIEW;
        else if (_eAEFlowState == AE_FLOW_STATE_CAPTURE) _eAEFlowState = AE_FLOW_STATE_VHDR_CAPTURE;
        else if (_eAEFlowState == AE_FLOW_STATE_MANUAL ) _eAEFlowState = AE_FLOW_STATE_VHDR_MANUAL;
        else                                             _eAEFlowState = AE_FLOW_STATE_VHDR_PREVIEW;
    }

    MBOOL isSupportedSettingByCCU = MTRUE;
    switch(_eAEFlowState)
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                     Normal Mode AE Exposure Control                        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        case AE_FLOW_STATE_INIT:

            AE_LOG( "[%s: AE_FLOW_STATE_INIT] m_eSensorDev: %d, m_eAEState: %d, RealSetting: %d/%d/%d\n",
                     __FUNCTION__, m_eSensorDev, m_eAEState,
                     rSettingAfterConv.EvSetting.u4Eposuretime, rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain);

            /* Exposure setting control */
            for (int i = 0; i < MAX_AE_SETTING_NUM; i++)
                updateAESetting(rSettingAfterConv);
            m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MFALSE);
            m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MFALSE);
            updateAEInfo2ISP(AE_FLOW_STATE_INIT, rSettingBeforeConv, rSettingAfterConv);

            /* Frame rate control */
            if (m_i4SMSubSamples > 1) {
                m_i4AEMaxFps = m_rSensorMaxFrmRate[m_eSensorMode];
            }
            m_u4UpdateFrameRate_x10 = 0; // to forceupdate max fps
            if (m_eSettingType == E_AE_SETTING_CPU) {
                controlCPUSensorMaxFPS(m_i4AEMaxFps);
            }
            else {
                if (m_bAdbAEPreviewUpdate)
                    controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
                controlCCUSensorMaxFPS(m_i4AEMaxFps);
            }
            AE_LOG("[%s(): AE_FLOW_STATE_INIT] Forced set max frame rate, m_i4SMSubSamples :%d, m_i4AEMaxFps: %d ", __FUNCTION__, m_i4SMSubSamples, m_i4AEMaxFps);

            m_u4StableCnt = 0 ;
            break;

        case AE_FLOW_STATE_PREVIEW:

            AE_LOG_IF( m_3ALogEnable,"[%s: AE_FLOW_STATE_PREVIEW] m_eSensorDev: %d, m_eAEState: %d, m_u4StableCnt: %d, RealSetting: %d/%d/%d \n",
                       __FUNCTION__, m_eSensorDev, m_eAEState, m_u4StableCnt,
                       rSettingAfterConv.EvSetting.u4Eposuretime, rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU) {
                controlCPUExpoSetting(rSettingAfterConv, MTRUE);
                controlCPUSensorMaxFPS(m_i4AEMaxFps, MTRUE);
            }
            else {
                if (m_eSettingType == E_AE_SETTING_CCU_FULL || m_bAdbAEPreviewUpdate)
                    controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
                controlCCUSensorMaxFPS(m_i4AEMaxFps);
            }
            updateAEInfo2ISP(AE_FLOW_STATE_PREVIEW, rSettingBeforeConv, rSettingAfterConv);

            if (m_bAEStable == MFALSE) {
                m_u4StableCnt = 0;
            } else {
                if (m_u4StableCnt < 0xFFFF) {
                    m_u4StableCnt++;
                }
            }
            break;

        case AE_FLOW_STATE_AE2AF:

            AE_LOG( "[%s: AE_FLOW_STATE_AE2AF] m_eSensorDev: %d, m_eAEState: %d, RealSetting: %d/%d/%d, m_i4WaitVDNum: %d, delay: %d/%d/%d \n",
                     __FUNCTION__, m_eSensorDev, m_eAEState,
                     rSettingAfterConv.EvSetting.u4Eposuretime, rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain,
                     m_i4WaitVDNum, m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU)
                controlCPUExpoSetting(rSettingAfterConv);
            else
                controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
            updateAEInfo2ISP(AE_FLOW_STATE_AE2AF, rSettingBeforeConv, rSettingAfterConv);

            if ( (m_i4WaitVDNum >= m_i4ShutterDelayFrames) &&
                 (m_i4WaitVDNum >= m_i4SensorGainDelayFrames) &&
                 (m_i4WaitVDNum >= m_i4IspGainDelayFrames)) {
                m_i4WaitVDNum = 0;
                m_bAEStable = MTRUE;
                AE_LOG( "[%s: AE_FLOW_STATE_AE2AF] m_eSensorDev:%d, wait m_i4WaitVDNum stable\n", __FUNCTION__, m_eSensorDev);
            }
            else {
                m_bAEStable = MFALSE;
                m_i4WaitVDNum ++;
            }
            break;

        case AE_FLOW_STATE_CAPTURE:

            AE_LOG( "[%s: AE_FLOW_STATE_CAPTURE] SensorDev:%d Exp. Mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d\n",
                     __FUNCTION__, m_eSensorDev, rSettingAfterConv.u4ExposureMode,
                     rSettingAfterConv.EvSetting.u4Eposuretime, rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain, rSettingAfterConv.u4ISO);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU)
                controlCPUExpoSetting(rSettingAfterConv);
            else
                controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
            updateAEInfo2ISP(AE_FLOW_STATE_CAPTURE, rSettingBeforeConv, rSettingAfterConv);

            m_u4VsyncCnt = m_i4SensorCaptureDelayFrame;
            break;

        case AE_FLOW_STATE_POSTCAP:

            AE_LOG( "[%s: AE_FLOW_STATE_POSTCAP] SensorDev:%d Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d ISO:%d\n",
                     __FUNCTION__, m_eSensorDev, rSettingAfterConv.EvSetting.u4Eposuretime,
                     rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain, rSettingAfterConv.u2FrameRate, rSettingAfterConv.u4ISO);
            updateAEInfo2ISP(AE_FLOW_STATE_POSTCAP, rSettingBeforeConv, rSettingAfterConv);
            break;

        case AE_FLOW_STATE_MANUAL:

            AE_LOG("[%s: AE_FLOW_STATE_MANUAL] m_eSensorDev: %d, m_eAEState: %d, manual setting: %d/%d/%d\n",
                     __FUNCTION__, m_eSensorDev, m_eAEState,
                     rSettingAfterConv.EvSetting.u4Eposuretime, rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain);

            /* Exposure setting control */
            if (m_u4DynamicHFPS <= 30) {
                if (m_eSettingType == E_AE_SETTING_CPU)
                    controlCPUExpoSetting(rSettingAfterConv, MTRUE);
                else {
                    isSupportedSettingByCCU = controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
                    if (!isSupportedSettingByCCU) { // CCU can't set manual setting, reset exposure control by CPU
                        AE_LOG("[%s: AE_FLOW_STATE_MANUAL] CCU can't set manual setting, reset exposure control by CPU\n", __FUNCTION__);
                        controlCPUExpoSetting(rSettingAfterConv);
                    }
                }
                updateAEInfo2ISP(AE_FLOW_STATE_MANUAL, rSettingBeforeConv, rSettingAfterConv, !isSupportedSettingByCCU);
            }
            break;

        case AE_FLOW_STATE_RESTORE:

            AE_LOG("[%s: AE_FLOW_STATE_RESTORE] m_eSensorDev: %d, m_eAEState: %d,, RealSetting: %d/%d/%d \n",
                   __FUNCTION__, m_eSensorDev, m_eAEState, rSettingAfterConv.EvSetting.u4Eposuretime, rSettingAfterConv.EvSetting.u4AfeGain, rSettingAfterConv.EvSetting.u4IspGain);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU)
                controlCPUExpoSetting(rSettingAfterConv);
            else
                controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
            updateAEInfo2ISP(AE_FLOW_STATE_RESTORE, rSettingBeforeConv, rSettingAfterConv);
            break;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                      VHDR Mode AE Exposure Control                         //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        case AE_FLOW_STATE_VHDR_INIT:

            AE_LOG( "[%s: AE_FLOW_STATE_VHDR_INIT] m_eSensorDev: %d, m_eAETargetMode: %d, m_eAEState: %d, RealSetting: LE(%d,%d,%d) ME(%d,%d,%d) SE(%d,%d,%d)\n",
                     __FUNCTION__, m_eSensorDev, m_eAETargetMode, m_eAEState,
                     rSettingAfterConv.HdrEvSetting.i4LEExpo, rSettingAfterConv.HdrEvSetting.i4LEAfeGain, rSettingAfterConv.HdrEvSetting.i4LEIspGain,
                     rSettingAfterConv.HdrEvSetting.i4MEExpo, rSettingAfterConv.HdrEvSetting.i4MEAfeGain, rSettingAfterConv.HdrEvSetting.i4MEIspGain,
                     rSettingAfterConv.HdrEvSetting.i4SEExpo, rSettingAfterConv.HdrEvSetting.i4SEAfeGain, rSettingAfterConv.HdrEvSetting.i4SEIspGain);

            /* Exposure setting control */
            for (int i = 0; i < MAX_AE_SETTING_NUM; i++)
                updateAESetting(rSettingAfterConv);
            m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MFALSE);
            m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MFALSE);
            updateAEInfo2ISP(AE_FLOW_STATE_VHDR_INIT, rSettingBeforeConv, rSettingAfterConv);

            /* Frame rate control */
            m_u4UpdateFrameRate_x10 = 0; // to forceupdate max fps
            if (m_eSettingType == E_AE_SETTING_CPU) {
                controlCPUSensorMaxFPS(m_i4AEMaxFps);
            }
            else {
                controlCCUSensorMaxFPS(m_i4AEMaxFps);
            }
            AE_LOG("[%s(): AE_FLOW_STATE_VHDR_INIT] Forced set max frame rate, m_i4AEMaxFps: %d ", __FUNCTION__, m_i4AEMaxFps);

            m_u4StableCnt = 0 ;
            break;

        case AE_FLOW_STATE_VHDR_PREVIEW:

            AE_LOG("[%s: AE_FLOW_STATE_VHDR_PREVIEW] m_eSensorDev: %d, m_eAETargetMode: %d, m_eAEState: %d, m_u4StableCnt: %d, RealSetting: LE(%d,%d,%d) ME(%d,%d,%d) SE(%d,%d,%d) \n",
                    __FUNCTION__, m_eSensorDev, m_eAETargetMode, m_eAEState, m_u4StableCnt,
                    rSettingAfterConv.HdrEvSetting.i4LEExpo, rSettingAfterConv.HdrEvSetting.i4LEAfeGain, rSettingAfterConv.HdrEvSetting.i4LEIspGain,
                    rSettingAfterConv.HdrEvSetting.i4MEExpo, rSettingAfterConv.HdrEvSetting.i4MEAfeGain, rSettingAfterConv.HdrEvSetting.i4MEIspGain,
                    rSettingAfterConv.HdrEvSetting.i4SEExpo, rSettingAfterConv.HdrEvSetting.i4SEAfeGain, rSettingAfterConv.HdrEvSetting.i4SEIspGain);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU) {
                controlCPUExpoSetting(rSettingAfterConv, MTRUE);
                controlCPUSensorMaxFPS(m_i4AEMaxFps, MTRUE);
            }
            else {
                if (m_eSettingType == E_AE_SETTING_CCU_FULL || m_bAdbAEPreviewUpdate)
                    controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
                controlCCUSensorMaxFPS(m_i4AEMaxFps);
            }
            updateAEInfo2ISP(AE_FLOW_STATE_VHDR_PREVIEW, rSettingBeforeConv, rSettingAfterConv);

            /* Sensor ATR control */
            if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) { // only MVHDR 3-expo needs to control ATR
                AAASensorMgr::getInstance().setSensorATR((ESensorDev_T)m_eSensorDev, m_u4veLimitGain, m_u4veLTCRate);
            }

            if (m_bAEStable == MFALSE) {
                m_u4StableCnt = 0;
            } else {
                if (m_u4StableCnt < 0xFFFF) {
                    m_u4StableCnt++;
                }
            }
            break;

        case AE_FLOW_STATE_VHDR_CAPTURE:

            AE_LOG( "[%s: AE_FLOW_STATE_VHDR_CAPTURE] m_eSensorDev: %d, m_eAETargetMode: %d, m_eAEState: %d, RealSetting: LE(%d,%d,%d) ME(%d,%d,%d) SE(%d,%d,%d) \n",
                     __FUNCTION__, m_eSensorDev, m_eAETargetMode, m_eAEState,
                     rSettingAfterConv.HdrEvSetting.i4LEExpo, rSettingAfterConv.HdrEvSetting.i4LEAfeGain, rSettingAfterConv.HdrEvSetting.i4LEIspGain,
                     rSettingAfterConv.HdrEvSetting.i4MEExpo, rSettingAfterConv.HdrEvSetting.i4MEAfeGain, rSettingAfterConv.HdrEvSetting.i4MEIspGain,
                     rSettingAfterConv.HdrEvSetting.i4SEExpo, rSettingAfterConv.HdrEvSetting.i4SEAfeGain, rSettingAfterConv.HdrEvSetting.i4SEIspGain);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU)
                controlCPUExpoSetting(rSettingAfterConv);
            else
                controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
            updateAEInfo2ISP(AE_FLOW_STATE_VHDR_CAPTURE, rSettingBeforeConv, rSettingAfterConv);

            m_u4StableCnt = 0 ;
            break;

        case AE_FLOW_STATE_VHDR_MANUAL:

            AE_LOG("[%s: AE_FLOW_STATE_VHDR_MANUAL] m_eSensorDev: %d, m_eAEState: %d, VHDR manual setting: LE: %d/%d/%d ME: %d/%d/%d SE: %d/%d/%d\n",
                     __FUNCTION__, m_eSensorDev, m_eAEState,
                     rSettingAfterConv.HdrEvSetting.i4LEExpo, rSettingAfterConv.HdrEvSetting.i4LEAfeGain, rSettingAfterConv.HdrEvSetting.i4LEIspGain,
                     rSettingAfterConv.HdrEvSetting.i4MEExpo, rSettingAfterConv.HdrEvSetting.i4MEAfeGain, rSettingAfterConv.HdrEvSetting.i4MEIspGain,
                     rSettingAfterConv.HdrEvSetting.i4SEExpo, rSettingAfterConv.HdrEvSetting.i4SEAfeGain, rSettingAfterConv.HdrEvSetting.i4SEIspGain);

            /* Exposure setting control */
            if (m_eSettingType == E_AE_SETTING_CPU)
                controlCPUExpoSetting(rSettingAfterConv, MTRUE);
            else
                controlCCUExpoSetting(rSettingAfterConv, rSettingBeforeConv);
            updateAEInfo2ISP(AE_FLOW_STATE_VHDR_MANUAL, rSettingBeforeConv, rSettingAfterConv);
            break;

        default:
            break;
    }
    return S_AE_OK;
}

MINT32 AeMgr::updateRTVString(AE_ISP_INFO_T &rAEISPInfo, char *stringBuffer)
{
    MINT32 RTVStingTotalLength = 0;
    if (!stringBuffer)
    {
        return RTVStingTotalLength;
    }
    else
    {
        char tmpString[40] = {"\0"};
        sprintf(tmpString, "AE[%d]ReqNum[%d]:", m_eSensorDev, rAEISPInfo.u4RequestNum);
        strncpy(stringBuffer, tmpString, PROPERTY_STRING_LENGTH);
        for (int i = 0; i < RTVMAXCOUNT; i++)
        {
            MINT32 tmpEnum = m_rtvId[i];
            MINT32 tmpValue = 0;
            memset(tmpString, 0, sizeof(char)*40);

            switch(tmpEnum)
            {
                case E_RTV_AE_SHUTTER:
                    tmpValue = rAEISPInfo.u8P1Exposuretime_ns/1000L;
                    break;
                case E_RTV_AE_ISO:
                    tmpValue = rAEISPInfo.u4P1RealISOValue;
                    break;
                case E_RTV_AE_LV:
                    tmpValue = rAEISPInfo.i4RealLightValue_x10;
                    break;
                case E_RTV_AE_STATE:
                    tmpValue = m_eAEState;
                    break;
                case E_RTV_AE_SCENARIO_MODE:
                    tmpValue = m_u4AENVRAMIdx;
                    break;
                case E_RTV_AE_CWV:
                    tmpValue = m_AEPerFrameInfo.rAEUpdateInfo.u4CWValue;
                    break;
                case E_RTV_AE_INDEXF:
                    tmpValue = m_AEPerFrameInfo.rAEUpdateInfo.u4AEidxCurrentF;
                    break;
                case E_RTV_AE_STABLE:
                    tmpValue = m_bAEStable;
                    break;
                case E_RTV_AE_FACE_NUM:
                    tmpValue = rAEISPInfo.u4FaceNum;
                    break;
                case E_RTV_AE_FACE_TARGET:
                    tmpValue = rAEISPInfo.u4FaceFinalTarget;
                    break;
                case E_RTV_AE_FINAL_TARGET:
                    tmpValue = 0; // TODO
                    break;
                case E_RTV_AE_AVGY:
                    tmpValue = m_AEPerFrameInfo.rAEUpdateInfo.u4AvgY;
                    break;
                case E_RTV_AE_PRE_PLINE:
                    tmpValue = m_pPreviewTableCurrent->eID;
                    break;
                case E_RTV_AE_CAP_PLINE:
                    tmpValue = m_pCaptureTable->eID;
                    break;
                case E_RTV_AE_HDR_RATIO:
                    tmpValue = m_rAEAlgoOutput.HdrEvSetting.u4AECHdrRatio;
                    break;
                case E_RTV_AE_MAGIC_NUM:
                    tmpValue = m_u4HwMagicNum;
                    break;
                default:
                    tmpEnum = 0;
                    tmpValue = 0;
                    break;
            }
            if (tmpEnum != 0)
            {
                sprintf(tmpString, "%d,%d;", tmpEnum, tmpValue);
                if (int(PROPERTY_STRING_LENGTH - strlen(stringBuffer) - strlen(tmpString) - 1) >= 0) // need enough space to attach next string
                    strncat(stringBuffer, tmpString, PROPERTY_STRING_LENGTH - strlen(stringBuffer) - 1);
            }
        }
        RTVStingTotalLength = strlen(stringBuffer);
        AE_LOG_IF(m_i4DgbRtvEnable, "%s total(%d): %s", __FUNCTION__, RTVStingTotalLength, stringBuffer);
    }
    return RTVStingTotalLength;
}

MRESULT AeMgr::updateAEInfo2ISP(AE_FLOW_STATE_T eAEFlowState, strAERealSetting &rSettingBeforeConv, strAERealSetting &rSettingAfterConv, MBOOL bForceSetByCPU)
{
    AE_INFO_T* pAEInfo2ISP;
    if (eAEFlowState == AE_FLOW_STATE_CAPTURE) {
        AE_LOG( "[updateAEInfo2ISP] get AE info from preparing capture parameters \n");
        pAEInfo2ISP = &m_rAEInfoCapture;
    } else {
        pAEInfo2ISP = &m_rAEInfoPreview;
    }

    MUINT32 u4FinalGain;
    MUINT32 u4ISOBase_1xGain = 100;
    MUINT32 u4FrameRate = 30000;   // 30fps

    // Update to isp tuning
    if(m_bStereoManualPline){ //For update both master & slave cams' LV
        m_i4BVvalue = pAEInfo2ISP->i4LightValue_x10-50;
        m_i4AOECompBVvalue = pAEInfo2ISP->i4RealLightValue_x10-50;   //real BV value
        AE_LOG_IF(m_3ALogEnable,"[updateAEInfo2ISP] m_i4AOECompBVvalue = %d \n",m_i4AOECompBVvalue);
    }

    // Update from NVram
    if(m_p3ANVRAM != NULL) {
        u4ISOBase_1xGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain * m_u4BinSumRatio;
    } else {
        AE_LOG( "[updateAEInfo2ISP] NVRAM data is NULL\n");
    }

    AE_ISP_INFO_T rAEISPInfo;
    memset(&rAEISPInfo, 0, sizeof(AE_ISP_INFO_T));
    rAEISPInfo.bAEStable = m_bAEStable;
    rAEISPInfo.bGammaEnable = pAEInfo2ISP->bGammaEnable;
    rAEISPInfo.bAETouchEnable = pAEInfo2ISP->bAETouchEnable;
    rAEISPInfo.bFaceAELCELinkEnable = pAEInfo2ISP->bFaceAELCELinkEnable;
    rAEISPInfo.bIsPlineMaxIndex = pAEInfo2ISP->bIsMaxIndexStable;
    rAEISPInfo.bDisableDGN = (m_eSettingType == E_AE_SETTING_CPU || bForceSetByCPU)? MFALSE : MTRUE;
    rAEISPInfo.bEnableFaceAE = pAEInfo2ISP->bEnableFaceAE;
    rAEISPInfo.bOTFaceTimeOutLockAE = pAEInfo2ISP->bOTFaceTimeOutLockAE;
    rAEISPInfo.u4RequestNum = pAEInfo2ISP->u4ReqNumber;
    rAEISPInfo.u4MagicNumber = m_u4HwMagicNum;
    rAEISPInfo.u4AETarget = pAEInfo2ISP->u4AETarget;
    rAEISPInfo.u4AECurrentTarget = pAEInfo2ISP->u4AECurrentTarget;
    rAEISPInfo.i4LightValue_x10 = pAEInfo2ISP->i4LightValue_x10;
    rAEISPInfo.u4AECondition = pAEInfo2ISP->u4AECondition;
    rAEISPInfo.i4GammaIdx = m_i4GammaIdx;
    rAEISPInfo.i4LESE_Ratio = m_i4LESE_Ratio;
    rAEISPInfo.u4MaxISO = m_u4MaxISO*m_rAEPLineLimitation.u4IncreaseISO_x100/100;
    rAEISPInfo.u4AEStableCnt = m_u4StableCnt;
    rAEISPInfo.u4AlgoExposuretime_us = rSettingBeforeConv.EvSetting.u4Eposuretime;
    rAEISPInfo.u4AlgoSensorGain = rSettingBeforeConv.EvSetting.u4AfeGain;
    rAEISPInfo.u4AlgoDGNGain = rSettingBeforeConv.EvSetting.u4IspGain;
    rAEISPInfo.u4AlgoRealISOValue = rSettingBeforeConv.u4ISO;
    memcpy(&rAEISPInfo.u4Histogrm[0], &(pAEInfo2ISP->u4Histogrm[0]), sizeof(MUINT32)*AE_Y_HISTOGRAM_BIN);
    rAEISPInfo.i4deltaIndex = pAEInfo2ISP->i4deltaIndex;
    rAEISPInfo.u4EVRatio = m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp];
    rAEISPInfo.u4AEFinerEVIdxBase = pAEInfo2ISP->u4AEFinerEVIdxBase;
    rAEISPInfo.u4FaceAEStable = pAEInfo2ISP->u4FaceAEStable;
    rAEISPInfo.i4Crnt_FDY = pAEInfo2ISP->i4Crnt_FDY;
    rAEISPInfo.u4MeterFDTarget = pAEInfo2ISP->u4MeterFDTarget;
    rAEISPInfo.u4FaceFinalTarget = pAEInfo2ISP->u4FaceFinalTarget;
    rAEISPInfo.u4FDProb = pAEInfo2ISP->u4FDProb;
    rAEISPInfo.u4FaceNum = pAEInfo2ISP->u4FaceNum;
    memcpy(&rAEISPInfo.i4FDY_ArrayOri[0], &(pAEInfo2ISP->i4FDY_ArrayOri[0]), sizeof(MINT32)*MAX_AE_METER_AREAS);
    memcpy(&rAEISPInfo.i4FDY_Array[0], &(pAEInfo2ISP->i4FDY_Array[0]), sizeof(MINT32)*MAX_AE_METER_AREAS);
    memcpy(&rAEISPInfo.LandMarkFDArea, &(pAEInfo2ISP->LandMarkFDArea), sizeof(CCU_AEMeterArea_T));
    memcpy(&rAEISPInfo.LandMarkFDSmoothArea, &(pAEInfo2ISP->LandMarkFDSmoothArea), sizeof(CCU_AEMeterArea_T));
    memcpy(&rAEISPInfo.FDArea[0], &(pAEInfo2ISP->FDArea[0]), sizeof(CCU_AEMeterArea_T)*MAX_AE_METER_AREAS);
    rAEISPInfo.u4MaxGain = pAEInfo2ISP->u4MaxGain;
    memcpy(&rAEISPInfo.i4FaceBrightToneProtectValue_12bit[0], &(pAEInfo2ISP->i4FaceBrightToneProtectValue_12bit[0]), sizeof(MINT32)*MAX_AE_METER_AREAS);
    rAEISPInfo.uFaceState = pAEInfo2ISP->m_u4FaceState;
    rAEISPInfo.u4FaceRobustCnt = pAEInfo2ISP->m_u4FaceRobustCnt;
    rAEISPInfo.u4FaceRobustTrustCnt = pAEInfo2ISP->m_u4FaceRobustTrustCnt;
    rAEISPInfo.u4FD_Lock_MaxCnt = pAEInfo2ISP->m_u4FD_Lock_MaxCnt;
    rAEISPInfo.u4FDDropTempSmoothCnt = pAEInfo2ISP->m_u4FDDropTempSmoothCnt;
    rAEISPInfo.u4OTFaceTrustCnt = pAEInfo2ISP->m_u4OTFaceTrustCnt;
    rAEISPInfo.u4MeterFDLinkTarget = pAEInfo2ISP->u4MeterFDLinkTarget;

    //mark for p80. please mgr owner check how to modify
    //memcpy(&rAEISPInfo.rHdrToneInfo, &(pAEInfo2ISP->rHdrToneInfo), sizeof(HDR_TONE_INFO_T));

    //rAEISPInfo.rHdrAEInfo.rHdrEVInfo.i4HdrRatio = pAEInfo2ISP->HdrEvSetting.u4AECHdrRatio;
    //rAEISPInfo.rHdrAEInfo.u4mVHdrRealRatio = pAEInfo2ISP->HdrEvSetting.u4CurHdrRatio;

    rAEISPInfo.i4RealLightValue_x10 = pAEInfo2ISP->i4RealLightValue_x10;
    rAEISPInfo.u4EISExpRatio = m_u4EISExpRatio;
    rAEISPInfo.i4AEComp = pAEInfo2ISP->i4AEComp;

    switch(eAEFlowState) {
        case AE_FLOW_STATE_INIT    :
        case AE_FLOW_STATE_PREVIEW :
        case AE_FLOW_STATE_AE2AF   :
        case AE_FLOW_STATE_CAPTURE :
        case AE_FLOW_STATE_POSTCAP :
        case AE_FLOW_STATE_RESTORE :
            rAEISPInfo.u8P1Exposuretime_ns = 1000L*rSettingAfterConv.EvSetting.u4Eposuretime;
            rAEISPInfo.u4P1SensorGain      = rSettingAfterConv.EvSetting.u4AfeGain;
            rAEISPInfo.u4P1DGNGain         = rSettingAfterConv.EvSetting.u4IspGain;
            u4FinalGain = (rAEISPInfo.u4P1SensorGain*rAEISPInfo.u4P1DGNGain)/AE_GAIN_BASE_ISP;
            rAEISPInfo.u4P1RealISOValue    = (u4ISOBase_1xGain*u4FinalGain)/AE_GAIN_BASE_AFE;
            u4FrameRate = 100*rSettingAfterConv.u2FrameRate;
            break;
        case AE_FLOW_STATE_MANUAL  :
            rAEISPInfo.u8P1Exposuretime_ns = 1000L*rSettingAfterConv.EvSetting.u4Eposuretime;
            rAEISPInfo.u4P1SensorGain      = rSettingAfterConv.EvSetting.u4AfeGain;
            rAEISPInfo.u4P1DGNGain         = rSettingAfterConv.EvSetting.u4IspGain;
            u4FinalGain = (rAEISPInfo.u4P1SensorGain*rAEISPInfo.u4P1DGNGain)/AE_GAIN_BASE_ISP;
            rAEISPInfo.u4P1RealISOValue    = (u4ISOBase_1xGain*u4FinalGain)/AE_GAIN_BASE_AFE;
            u4FrameRate = 100*rSettingAfterConv.u2FrameRate;
            break;
        case AE_FLOW_STATE_VHDR_INIT    :
        case AE_FLOW_STATE_VHDR_PREVIEW :
        case AE_FLOW_STATE_VHDR_CAPTURE :
            rAEISPInfo.u8P1Exposuretime_ns = 1000L*rSettingAfterConv.HdrEvSetting.i4LEExpo;
            rAEISPInfo.u4P1SensorGain      = rSettingAfterConv.HdrEvSetting.i4LEAfeGain;
            rAEISPInfo.u4P1DGNGain         = rSettingAfterConv.HdrEvSetting.i4LEIspGain;
            rAEISPInfo.i4LESE_Ratio      = rSettingAfterConv.HdrEvSetting.u4AECHdrRatio;
            u4FinalGain = (rAEISPInfo.u4P1SensorGain*rAEISPInfo.u4P1DGNGain)/AE_GAIN_BASE_ISP;
            rAEISPInfo.u4P1RealISOValue    = (u4ISOBase_1xGain*u4FinalGain)/AE_GAIN_BASE_AFE;
            u4FrameRate = 100*rSettingAfterConv.u2FrameRate;
            break;
        case AE_FLOW_STATE_VHDR_MANUAL  :
            rAEISPInfo.u8P1Exposuretime_ns = 1000L*rSettingAfterConv.HdrEvSetting.i4LEExpo;
            rAEISPInfo.u4P1SensorGain      = rSettingAfterConv.HdrEvSetting.i4LEAfeGain;
            rAEISPInfo.u4P1DGNGain         = rSettingAfterConv.HdrEvSetting.i4LEIspGain;
            u4FinalGain = (rAEISPInfo.u4P1SensorGain*rAEISPInfo.u4P1DGNGain)/AE_GAIN_BASE_ISP;
            rAEISPInfo.u4P1RealISOValue    = (u4ISOBase_1xGain*u4FinalGain)/AE_GAIN_BASE_AFE;
            u4FrameRate = 100*rSettingAfterConv.u2FrameRate;
            break;
        default:
            AE_LOG( "[updateAEInfo2ISP] Nothing to do State:%d\n", eAEFlowState);
            return S_AE_OK;
    }

    if(u4FrameRate > (100*m_i4AEMaxFps)) {
        AE_LOG("[updateAEInfo2ISP] Frame rate too large:%d %d\n", u4FrameRate, m_i4AEMaxFps);
        u4FrameRate = 100*m_i4AEMaxFps;
    }
    m_AEPerFrameInfo.rAEUpdateInfo.u2FrameRate_x10 = u4FrameRate / 100;
    m_AEPerFrameInfo.rAEUpdateInfo.u8FrameDuration_ns = getFrameDuration(u4FrameRate);
	m_i4AEAutoFlickerMode = (MINT32)(m_eAEAutoFlickerMode);

    if(rAEISPInfo.u4P1RealISOValue >= m_u4ISOIdx1Value) {
        if(m_u4ISOIdx1StableCount < 2*m_u4Idx1StableThres) {
            m_u4ISOIdx1StableCount++;
        }
    } else {
        if(m_u4ISOIdx1StableCount > 0) {
            m_u4ISOIdx1StableCount--;
        }
    }

    if(rAEISPInfo.u4P1RealISOValue >= m_u4ISOIdx2Value) {
        if(m_u4ISOIdx2StableCount < 2*m_u4Idx2StableThres) {
            m_u4ISOIdx2StableCount++;
        }
    } else {
        if(m_u4ISOIdx2StableCount > 0) {
            m_u4ISOIdx2StableCount--;
        }
    }

    // Stereo denoise OB2
    if (m_bStereoManualPline && m_u4StereoDenoiseMode){
        MINT32 i4StereoDenoiserto = 1000;
        if (m_u4StereoDenoiseMode == 1)
            i4StereoDenoiserto = m_i4StereoDenoiserto[0];
        else if (m_u4StereoDenoiseMode == 2)
            i4StereoDenoiserto = m_i4StereoDenoiserto[1];

        rAEISPInfo.u4P1DGNGain = rAEISPInfo.u4P1DGNGain*i4StereoDenoiserto/1000;
        rAEISPInfo.u4P1RealISOValue = rAEISPInfo.u4P1RealISOValue*i4StereoDenoiserto/1000;
        AE_LOG("[updateAEInfo2ISP][More OB2] Denoise mode :%d Dev:%d, Stereo ZSD capture - Ratio = %d \n", m_u4StereoDenoiseMode, m_eSensorDev, i4StereoDenoiserto);
    }

    rAEISPInfo.u4P1Gain_x100 = rAEISPInfo.u4P1RealISOValue; //100*rAEISPInfo.u4P1SensorGain/AE_GAIN_BASE_AFE*rAEISPInfo.u4P1DGNGain/AE_GAIN_BASE_ISP;

    if(eAEFlowState == AE_FLOW_STATE_POSTCAP){ // keep ISP P1 info as previous
        m_AEPerFrameInfo.rAEISPInfo.u8P2Exposuretime_ns = rAEISPInfo.u8P1Exposuretime_ns;
        m_AEPerFrameInfo.rAEISPInfo.u4P2SensorGain = rAEISPInfo.u4P1SensorGain;
        m_AEPerFrameInfo.rAEISPInfo.u4P2DGNGain = rAEISPInfo.u4P1DGNGain;
        u4FinalGain = (m_AEPerFrameInfo.rAEISPInfo.u4P2SensorGain*m_AEPerFrameInfo.rAEISPInfo.u4P2DGNGain)/AE_GAIN_BASE_ISP;
        m_AEPerFrameInfo.rAEISPInfo.u4P2RealISOValue = (u4ISOBase_1xGain*u4FinalGain)/AE_GAIN_BASE_AFE;
        m_AEPerFrameInfo.rAEISPInfo.u4P2Gain_x100 = m_AEPerFrameInfo.rAEISPInfo.u4P2RealISOValue; //100*m_AEPerFrameInfo.rAEISPInfo.u4P2SensorGain*m_AEPerFrameInfo.rAEISPInfo.u4P2DGNGain/AE_GAIN_BASE_AFE/AE_GAIN_BASE_ISP;
        /* [LCE request: update FD info] special case for force update FD info at postcap state */
        m_AEPerFrameInfo.rAEISPInfo.i4Crnt_FDY = pAEInfo2ISP->i4Crnt_FDY;
        m_AEPerFrameInfo.rAEISPInfo.u4MeterFDTarget = pAEInfo2ISP->u4MeterFDTarget;
        AE_LOG( "[%s] force update FD info at postcap state, FDY/FDT: %d/%d\n", __FUNCTION__, m_AEPerFrameInfo.rAEISPInfo.i4Crnt_FDY, m_AEPerFrameInfo.rAEISPInfo.u4MeterFDTarget);        
    } else if(eAEFlowState == AE_FLOW_STATE_RESTORE){ // keep ISP P2 info as previous
        rAEISPInfo.u8P2Exposuretime_ns = m_AEPerFrameInfo.rAEISPInfo.u8P2Exposuretime_ns;
        rAEISPInfo.u4P2SensorGain = m_AEPerFrameInfo.rAEISPInfo.u4P2SensorGain;
        rAEISPInfo.u4P2DGNGain = m_AEPerFrameInfo.rAEISPInfo.u4P2DGNGain;
        rAEISPInfo.u4P2Gain_x100 = m_AEPerFrameInfo.rAEISPInfo.u4P2Gain_x100;
        rAEISPInfo.u4P2RealISOValue =  m_AEPerFrameInfo.rAEISPInfo.u4P2RealISOValue;
        rAEISPInfo.i2FlareOffset = 0;
        memcpy(&(m_AEPerFrameInfo.rAEISPInfo), &rAEISPInfo, sizeof(AE_ISP_INFO_T)); // fill in ISP P1 info    
    } else {
        rAEISPInfo.u8P2Exposuretime_ns = rAEISPInfo.u8P1Exposuretime_ns;
        rAEISPInfo.u4P2SensorGain = rAEISPInfo.u4P1SensorGain;
        rAEISPInfo.u4P2DGNGain = rAEISPInfo.u4P1DGNGain;
        rAEISPInfo.u4P2Gain_x100 = rAEISPInfo.u4P1Gain_x100;
        rAEISPInfo.u4P2RealISOValue = rAEISPInfo.u4P1RealISOValue;
        rAEISPInfo.i2FlareOffset = 0;
        memcpy(&(m_AEPerFrameInfo.rAEISPInfo), &rAEISPInfo, sizeof(AE_ISP_INFO_T)); // fill in ISP P1 info
    }

    m_AEPerFrameInfo.rAEUpdateInfo.bAELock = m_bAELock;
    m_AEPerFrameInfo.rAEUpdateInfo.TgCtrlRight = pAEInfo2ISP->TgCtrlRight;
    m_AEPerFrameInfo.rAEUpdateInfo.bAELock = m_bAELock;
    m_AEPerFrameInfo.rAEUpdateInfo.bAELimiter = m_bAElimitor;
    m_AEPerFrameInfo.rAEUpdateInfo.bAEScenarioChange = pAEInfo2ISP->bAEScenarioChange;
    m_AEPerFrameInfo.rAEUpdateInfo.bEnableRAFastConverge = pAEInfo2ISP->bEnableRAFastConverge;
    m_AEPerFrameInfo.rAEUpdateInfo.bAPAELock = m_bAPAELock;
    m_AEPerFrameInfo.rAEUpdateInfo.bLVChangeTooMuch = isLVChangeTooMuch();
    m_AEPerFrameInfo.rAEUpdateInfo.bIsNeedUpdateCPUstatus = IsNeedUpdateSensor();
    m_AEPerFrameInfo.rAEUpdateInfo.bStrobeBVTrigger = IsStrobeBVTrigger();

    memcpy(&m_AEPerFrameInfo.rAEUpdateInfo.pu4AEBlock[0][0], &(pAEInfo2ISP->pu4AEBlock[0][0]), sizeof(MUINT32)*AE_BLOCK_NO*AE_BLOCK_NO);
    m_AEPerFrameInfo.rAEUpdateInfo.u4CWValue = m_u4CWVY;
    m_AEPerFrameInfo.rAEUpdateInfo.u4AvgY = m_u4CWVY;
    m_AEPerFrameInfo.rAEUpdateInfo.i4AEidxCur = m_rAESettingPreview.u4Index;
    m_AEPerFrameInfo.rAEUpdateInfo.u4AEidxCurrentF = pAEInfo2ISP->u4AEidxNextF;
    m_AEPerFrameInfo.rAEUpdateInfo.i4AEidxNext = m_rAESettingPreview.u4Index;
    m_AEPerFrameInfo.rAEUpdateInfo.i4AEidxNextF = m_rAESettingPreview.u4IndexF;
    m_AEPerFrameInfo.rAEUpdateInfo.i4DeltaBV = m_i4DeltaBV;
    m_AEPerFrameInfo.rAEUpdateInfo.u4AECondition = m_u4AECondition;
    m_AEPerFrameInfo.rAEUpdateInfo.i2AEFaceDiffIndex = 0; //m_i2AEFaceDiffIndex;
    m_AEPerFrameInfo.rAEUpdateInfo.i4HdrOnOff = m_i4HdrOnOff;
    m_AEPerFrameInfo.rAEUpdateInfo.i4AEMode = static_cast<MINT32>(m_eAEMode);
    m_AEPerFrameInfo.rAEUpdateInfo.i4AEState = static_cast<MINT32>(m_eAEState);
    m_AEPerFrameInfo.rAEUpdateInfo.i4BVvalue_x10 = m_i4BVvalue;
    m_AEPerFrameInfo.rAEUpdateInfo.i4AOECompLVvalue = m_i4AOECompBVvalue + 50;
    m_AEPerFrameInfo.rAEUpdateInfo.u4LuxValue_x10000 = 10000* pow(2, (rAEISPInfo.i4LightValue_x10/10-3));
//    m_AEPerFrameInfo.rAEUpdateInfo.u4ExposureMode = m_rAESettingPreview.EvSetting.u4Eposuretime;

    AE_LOG("[updateAEInfo2ISP] State:%d eAEstate:%d CCU ON:%d Algo:%d/%d/%d(%d) P1RealSetting:%d/%d/%d(%d) P2RealSetting:%d/%d/%d(%d) P1/P2GainRatioX100:%d/%d FrameDuration:%lld Index:%d(%d/%d) m_u4ISOIdx1StableCount:%d m_u4ISOIdx2StableCount:%d IsNeedUpdateSensor:%d i4LightValue_x10(Real):%d(%d) Lux:%d FaceNum:%d RequestNum:%d MagicNum:%d u4ISOBase_1xGain:%d m_u4BinSumRatio:%d i4AEComp:%d\n",
            eAEFlowState, m_eAEState, m_AEPerFrameInfo.rAEISPInfo.bDisableDGN,
            m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
            m_AEPerFrameInfo.rAEISPInfo.u8P1Exposuretime_ns/1000L, m_AEPerFrameInfo.rAEISPInfo.u4P1SensorGain,m_AEPerFrameInfo.rAEISPInfo.u4P1DGNGain, m_AEPerFrameInfo.rAEISPInfo.u4P1RealISOValue,
            m_AEPerFrameInfo.rAEISPInfo.u8P2Exposuretime_ns/1000L, m_AEPerFrameInfo.rAEISPInfo.u4P2SensorGain,m_AEPerFrameInfo.rAEISPInfo.u4P2DGNGain, m_AEPerFrameInfo.rAEISPInfo.u4P2RealISOValue,
            m_AEPerFrameInfo.rAEISPInfo.u4P1Gain_x100, m_AEPerFrameInfo.rAEISPInfo.u4P2Gain_x100, (long long)m_rSensorCurrentInfo.u8FrameDuration,
            m_AEPerFrameInfo.rAEISPInfo.bIsPlineMaxIndex, m_u4Index, m_u4IndexMax, m_u4ISOIdx1StableCount, m_u4ISOIdx2StableCount,
            m_AEPerFrameInfo.rAEUpdateInfo.bIsNeedUpdateCPUstatus, m_AEPerFrameInfo.rAEISPInfo.i4LightValue_x10, m_AEPerFrameInfo.rAEISPInfo.i4RealLightValue_x10, m_AEPerFrameInfo.rAEUpdateInfo.u4LuxValue_x10000, m_AEPerFrameInfo.rAEISPInfo.u4FaceNum, m_AEPerFrameInfo.rAEISPInfo.u4RequestNum, m_u4HwMagicNum,
            u4ISOBase_1xGain, m_u4BinSumRatio, m_AEPerFrameInfo.rAEISPInfo.i4AEComp);

    if (m_i4DgbRtvEnable)
    {
        // prepare the ae_rtv property
        char value[PROPERTY_STRING_LENGTH] = {'\0'}; // strlen > 92 (include '\0') can't show
        MINT32 count = updateRTVString(rAEISPInfo, value);
        if (count > 0)
        {
            AE_LOG("AERTV : %s", value);
            property_set("vendor.debug.ae_rtv.data", value);
        }
    }

    return S_AE_OK;
}

MBOOL AeMgr::IsNeedUpdateSensor()
{
    if (m_eSettingType == E_AE_SETTING_CPU) {
        return (m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_NEEDUPDATED));
    }
    else
        return MFALSE;
}

MRESULT AeMgr::updateSensorbyI2CBufferMode()
{
    if (m_eSettingType == E_AE_SETTING_CPU) {
        AE_LOG_IF( m_3ALogEnable,"[%s] \n", __FUNCTION__);
        AE_SETTING_CONTROL_INFO_T rAESetControlInfo;
        prepareSetControlParam(&rAESetControlInfo);
        m_pIAeSettingCPU->updateExpoSetting((MVOID*) &rAESetControlInfo);
    }
    return S_AE_OK;
}

MRESULT AeMgr::updateSensorbyI2C()
{
    if (m_eSettingType == E_AE_SETTING_CPU) {
        AE_LOG_IF( m_3ALogEnable,"[%s] \n", __FUNCTION__);
        AE_SETTING_CONTROL_INFO_T rAESetControlInfo;
        prepareSetControlParam(&rAESetControlInfo, MTRUE);
        AAA_TRACE_D("updateSensorbyI2C");
        m_pIAeSettingCPU->updateExpoSettingbyI2C((MVOID *) &rAESetControlInfo);
        AAA_TRACE_END_D;
    }
    return S_AE_OK;
}

MBOOL AeMgr::getAEManualSetting(strAERealSetting* pAERealSetting)
{
    MUINT32 u4MinIsoGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    MUINT32 u4MaxGain    = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain;
    if(m_SensorQueueCtrl.uOutputIndex >= AE_SENSOR_MAX_QUEUE) {
        m_SensorQueueCtrl.uOutputIndex = 0;
    }

    if(m_SensorQueueCtrl.uInputIndex != m_SensorQueueCtrl.uOutputIndex) {
        // frame rate control
        pAERealSetting->u2FrameRate = 10000000 / m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4FrameDuration;

        pAERealSetting->EvSetting.u4Eposuretime = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime;
        pAERealSetting->u4ISO = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity;

        // check if input iso is lower than sensor min iso
        if (pAERealSetting->u4ISO<u4MinIsoGain) pAERealSetting->u4ISO= u4MinIsoGain;
        pAERealSetting->EvSetting.u4IspGain = AE_GAIN_BASE_ISP;
        pAERealSetting->EvSetting.u4AfeGain= (pAERealSetting->u4ISO * AE_GAIN_BASE_AFE) / u4MinIsoGain;
        if (pAERealSetting->EvSetting.u4AfeGain > u4MaxGain) {
            pAERealSetting->EvSetting.u4IspGain = AE_GAIN_BASE_ISP * pAERealSetting->EvSetting.u4AfeGain / u4MaxGain;
            pAERealSetting->EvSetting.u4AfeGain = u4MaxGain;
        }
        if(pAERealSetting->EvSetting.u4AfeGain < AE_GAIN_BASE_AFE) pAERealSetting->EvSetting.u4AfeGain = AE_GAIN_BASE_AFE;
        pAERealSetting->u4Index  = m_u4Index;
        pAERealSetting->u4IndexF = m_u4IndexF;
        pAERealSetting->u4AEFinerEVIdxBase = m_u4FinerEVIdxBase;
        AE_LOG( "[%s: normal manual] m_eSensorDev: %d, manual shutter/ISO: %d/%d, manual setting: %d/%d/%d, index/F/base: %d/%d/%d FrameRate(%d)\n",
                 __FUNCTION__, m_eSensorDev,
                 m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime, m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity,
                 pAERealSetting->EvSetting.u4Eposuretime, pAERealSetting->EvSetting.u4AfeGain, pAERealSetting->EvSetting.u4IspGain, pAERealSetting->u4Index, pAERealSetting->u4IndexF, pAERealSetting->u4AEFinerEVIdxBase, pAERealSetting->u2FrameRate);

        if (m_eAETargetMode != AE_MODE_NORMAL) {
            /* VHDR exposure setting manually */            
            if(m_pIAeCalc != NULL) {
                m_pIAeCalc->transHDREVSetting(pAERealSetting,  m_u4EMVHDRratio);
            } else {
                AE_LOG( "[%s()] The AE calculate library pointer is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
            }
            AE_LOG( "[%s: VHDR manual] m_eSensorDev: %d, m_u4EMVHDRratio: %d, manual shutter/ISO: %d/%d, VHDR manual setting: LE: %d/%d/%d ME: %d/%d/%d SE: %d/%d/%d\n",
                     __FUNCTION__, m_eSensorDev, m_u4EMVHDRratio,
                     m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime, m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity,
                     pAERealSetting->HdrEvSetting.i4LEExpo, pAERealSetting->HdrEvSetting.i4LEAfeGain, pAERealSetting->HdrEvSetting.i4LEIspGain,
                     pAERealSetting->HdrEvSetting.i4MEExpo, pAERealSetting->HdrEvSetting.i4MEAfeGain, pAERealSetting->HdrEvSetting.i4MEIspGain,
                     pAERealSetting->HdrEvSetting.i4SEExpo, pAERealSetting->HdrEvSetting.i4SEAfeGain, pAERealSetting->HdrEvSetting.i4SEIspGain);
        }

        m_SensorQueueCtrl.uOutputIndex++;
    } else {
        AE_LOG( "[%s] No data input: %d %d\n", __FUNCTION__, m_SensorQueueCtrl.uInputIndex, m_SensorQueueCtrl.uOutputIndex);
        return MFALSE;
    }
    return MTRUE;
}

MRESULT AeMgr::getBinSumTransSetting(strAERealSetting* pAERealSetting, MUINT32 u4BinSumRatio)
{
    strBinningSumTrans rBinningSumTrans;
    rBinningSumTrans.u4BinningSumRatio = u4BinSumRatio;
    rBinningSumTrans.pAeNVRAM = (CCU_AE_NVRAM_T *)(m_rAEInitInput.rAENVRAM);
    rBinningSumTrans.pRealSetting = pAERealSetting;
    GetAECustomTransform(AE_CUSTOM_TRANSFORM_BINSUM, getsensorDevID(m_eSensorDev), &rBinningSumTrans);
    AE_LOG( "[%s] m_eSensorDev: %d, u4BinSumRatio: %d, after binning sum transform: %d/%d/%d (%d)\n", __FUNCTION__, m_eSensorDev, u4BinSumRatio,
            rBinningSumTrans.pRealSetting->EvSetting.u4Eposuretime, rBinningSumTrans.pRealSetting->EvSetting.u4AfeGain, rBinningSumTrans.pRealSetting->EvSetting.u4IspGain, rBinningSumTrans.pRealSetting->u4ISO);
    return S_AE_OK;
}

/* dynamic high framerate */
MRESULT AeMgr::setDynamicHFPS()
{
    if (m_eSettingType == E_AE_SETTING_CPU || m_eAEMode != LIB3A_AE_MODE_OFF || m_eAETargetMode != AE_MODE_NORMAL) {
        AE_LOG("[%s()] skip setting, m_eSettingType: %d, m_eAEMode: %d, m_eAETargetMode: %d\n", __FUNCTION__, m_eSettingType, m_eAEMode, m_eAETargetMode);
        return S_AE_OK;
    }

    ccu_dynamic_high_fps_info dynamic_high_fps_info;
    if (m_u4DynamicHFPS > 30) {
        doBackupAE();
        dynamic_high_fps_info.target_fps = m_u4DynamicHFPS;
        MBOOL isValidMunual = getAEManualSetting(&(dynamic_high_fps_info.manual_exp));
        if (isValidMunual) {
            AE_LOG( "[%s] m_eSensorDev: %d, target_fps: %d, manual setting: %d/%d/%d\n",
                    __FUNCTION__, m_eSensorDev, dynamic_high_fps_info.target_fps,
                    dynamic_high_fps_info.manual_exp.EvSetting.u4Eposuretime, dynamic_high_fps_info.manual_exp.EvSetting.u4AfeGain, dynamic_high_fps_info.manual_exp.EvSetting.u4IspGain);
            m_pICcuAe->ccuControl(MSG_TO_CCU_SWITCH_HIGH_FPS, &dynamic_high_fps_info, NULL);
        }
    }
    else {
        dynamic_high_fps_info.target_fps = m_u4DynamicHFPS;
        memcpy(&(dynamic_high_fps_info.manual_exp), &m_rAESettingBackup, sizeof(strAERealSetting));
        AE_LOG( "[%s] m_eSensorDev: %d, target_fps: %d, manual setting: %d/%d/%d\n",
                    __FUNCTION__, m_eSensorDev, dynamic_high_fps_info.target_fps,
                    dynamic_high_fps_info.manual_exp.EvSetting.u4Eposuretime, dynamic_high_fps_info.manual_exp.EvSetting.u4AfeGain, dynamic_high_fps_info.manual_exp.EvSetting.u4IspGain);
        m_pICcuAe->ccuControl(MSG_TO_CCU_SWITCH_HIGH_FPS, &dynamic_high_fps_info, NULL);
    }
    return S_AE_OK;
}

MRESULT AeMgr::presetCtrlDynamicHFPS()
{
    if (m_eSettingType == E_AE_SETTING_CPU || m_eAEMode != LIB3A_AE_MODE_OFF || m_eAETargetMode != AE_MODE_NORMAL) {
        AE_LOG("[%s()] skip setting, m_eSettingType: %d, m_eAEMode: %d, m_eAETargetMode: %d\n", __FUNCTION__, m_eSettingType, m_eAEMode, m_eAETargetMode);
        return S_AE_OK;
    }

    strAERealSetting rTempSetting;
    memset(&rTempSetting,0,sizeof(strAERealSetting));
    MBOOL isValidMunual = getAEManualSetting(&rTempSetting);
    if (isValidMunual) {
        AE_LOG( "[%s] m_eSensorDev: %d, manual setting: %d/%d/%d\n",
                __FUNCTION__, m_eSensorDev, rTempSetting.EvSetting.u4Eposuretime, rTempSetting.EvSetting.u4AfeGain, rTempSetting.EvSetting.u4IspGain);
        controlCCUExpoSetting(rTempSetting, rTempSetting, MTRUE);
        updateAEInfo2ISP(AE_FLOW_STATE_MANUAL, rTempSetting, rTempSetting);
    }
    return S_AE_OK;
}

MINT64 AeMgr::getFrameDuration(MUINT32 u4FrameRateX100)
{
    MINT64 u8FrameDuration;
    if(u4FrameRateX100 > (100*m_i4AEMaxFps)) {
        u4FrameRateX100 = 100*m_i4AEMaxFps;
    }
    if(u4FrameRateX100 != 0) {
        u8FrameDuration = 1000500000000L / u4FrameRateX100;
    } else {
        u8FrameDuration = 33000000L;
        AE_LOG( "No Frame rate value, using default frame rate value (33ms)\n");
    }
    return u8FrameDuration;
}
