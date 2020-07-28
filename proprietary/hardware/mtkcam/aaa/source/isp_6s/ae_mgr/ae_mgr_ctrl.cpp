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

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <mtkcam/def/common.h>
using namespace NSCam;
#include <faces.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <ae_tuning_custom.h>
//#include <isp_mgr.h>
#include <isp_tuning.h>
#include <camera_feature.h>
#include <isp_tuning_cam_info.h>
//#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <camera_feature.h>
#include <private/aaa_hal_private.h>
#include <ae_mgr.h>
#include <nvbuf_util.h>
//#include "aaa_state_flow_custom.h"

using namespace NS3Av3;
//using namespace NSIspTuning;
//using namespace NSIspTuningv3;
using namespace NSFeature;

#define MIN_MANUAL_EXP_VALUE 100
#define MAX_MANUAL_EXP_VALUE 40000   // 40 ms
#define MIN_MANUAL_ISO_VALUE 100    // ISO100
#define MAX_MANUAL_ISO_VALUE 3200   // ISO3200

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateSensorParams(AE_SENSOR_PARAM_T strSensorParams, MBOOL bForceUpdate)
{
    if(m_bShutterISOPriorityMode && !bForceUpdate) {
        AE_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev = %d, skip manual setting due to ShutterISOPriorityMode\n", __FUNCTION__, m_eSensorDev);
        return S_AE_OK;
    }

    if(strSensorParams.u8FrameDuration < strSensorParams.u8ExposureTime) {
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration = (MUINT32) (strSensorParams.u8ExposureTime / 1000);
        AE_LOG("[%s()] i4SensorDev = %d line:%d Frame Duration less than EXposure time: %lld %lld\n", __FUNCTION__, m_eSensorDev, __LINE__,
        (long long)strSensorParams.u8FrameDuration, (long long)strSensorParams.u8ExposureTime);
    } else {
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration = (MUINT32) (strSensorParams.u8FrameDuration / 1000);
    }
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4ExposureTime = (MUINT32) (strSensorParams.u8ExposureTime / 1000);
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4Sensitivity = (MUINT32) (strSensorParams.u4Sensitivity);

    AE_LOG("[%s()] i4SensorDev = %d line:%d Idx:%d %d FrameDuration:%d %lld Exposure Time:%d %lld ISO:%d %d\n", __FUNCTION__, m_eSensorDev, __LINE__,
        m_SensorQueueCtrl.uInputIndex, m_SensorQueueCtrl.uOutputIndex,
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration, (long long)strSensorParams.u8FrameDuration,
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4ExposureTime, (long long)strSensorParams.u8ExposureTime,
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4Sensitivity, strSensorParams.u4Sensitivity);

    m_SensorQueueCtrl.uInputIndex++;

    if(m_SensorQueueCtrl.uInputIndex >= AE_SENSOR_MAX_QUEUE) {
        m_SensorQueueCtrl.uInputIndex = 0;
    }


    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::bBlackLevelLock(MBOOL bLockBlackLevel)
{
    if(m_bLockBlackLevel != bLockBlackLevel) {
        AE_LOG("[%s()] i4SensorDev = %d line:%d BlackLevel:%d %d \n", __FUNCTION__, m_eSensorDev, __LINE__, m_bLockBlackLevel, bLockBlackLevel);
        m_bLockBlackLevel = bLockBlackLevel;
        AAASensorMgr::getInstance().setSensorOBLock((ESensorDev_T)m_eSensorDev, m_bLockBlackLevel);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorParams(AE_SENSOR_PARAM_T &a_rSensorInfo)
{
    a_rSensorInfo = m_rSensorCurrentInfo;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AeMgr::getSensorRollingShutter() const
{
    MUINT32 tline = 0, vsize = 0;
    AAASensorMgr::getInstance().getRollingShutter((ESensorDev_T)m_eSensorDev, tline, vsize);
    return (MINT64) tline * vsize;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::CCUManualControl(MUINT32 u4ExpTime,MUINT32 u4AfeGain,MUINT32 u4IspGain,MBOOL EnableManual)
{
 if(EnableManual){
 m_u4CCUManualShutter = u4ExpTime;
 m_u4CCUManualAfeGain = u4AfeGain;
 m_u4CCUManualIspGain = u4IspGain;
 }
 else{
 m_u4CCUManualShutter =0;
 m_u4CCUManualAfeGain =0;
 m_u4CCUManualIspGain =0;
 }
 m_u4CCUManualEnable = EnableManual;
 return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setExpoSetting(MUINT32 u4ExpTime, MUINT32 u4AfeGain, MUINT32 u4IspGain)
{
    AE_LOG("[%s()] i4SensorDev: %d, u4ExpTime/u4AfeGain/u4IspGain: %d/%d/%d \n", __FUNCTION__, m_eSensorDev, u4ExpTime, u4AfeGain, u4IspGain);
    strAERealSetting rAERealSetting;

    memset(&rAERealSetting, 0, sizeof(strAERealSetting));

    rAERealSetting.u4ExposureMode = eAE_EXPO_TIME;
    rAERealSetting.EvSetting.u4Eposuretime = u4ExpTime;
    rAERealSetting.EvSetting.u4AfeGain     = u4AfeGain;
    rAERealSetting.EvSetting.u4IspGain     = u4IspGain;
    if (m_eSettingType == E_AE_SETTING_CPU)
        controlCPUExpoSetting(rAERealSetting);
    else
        controlCCUExpoSetting(rAERealSetting, rAERealSetting);
    updateAEInfo2ISP(AE_FLOW_STATE_PREVIEW, rAERealSetting, rAERealSetting);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::sendAECtrl(EAECtrl_T eAECtrl, MINTPTR iArg1, MINTPTR iArg2, MINTPTR iArg3, MINTPTR iArg4)
{
    MINT32 i4Ret = 0;
    MBOOL bEnable = 0;

    switch (eAECtrl)
    {
        // --------------------------------- Get AE Info ---------------------------------
        case EAECtrl_GetCapDelayFrame:
            *(reinterpret_cast<MINT32*>(iArg1)) = get3ACaptureDelayFrame();
            break;
        case EAECtrl_GetAAOLineByteSize:
            AE_LOG( "[%s:EAECtrl_GetAAOLineByteSize] i4SensorDev:%d i4AAOLineByte:%d\n", __FUNCTION__, m_eSensorDev, m_rAEInitInput.i4AAOLineByte);
            *(reinterpret_cast<MINT32*>(iArg1)) = m_rAEInitInput.i4AAOLineByte;
            break;
        case EAECtrl_GetCapPlineTable:
            i4Ret = getAECapPlineTable(reinterpret_cast<MINT32*>(iArg1), *reinterpret_cast<strAETable*>(iArg2));
            break;
        case EAECtrl_GetSensorDelayInfo:
            i4Ret = getAEdelayInfo(reinterpret_cast<MINT32*>(iArg1), reinterpret_cast<MINT32*>(iArg2), reinterpret_cast<MINT32*>(iArg3));
            break;
        case EAECtrl_GetFDMeteringAreaInfo:
            i4Ret = getAEFDMeteringAreaInfo(*reinterpret_cast<std::vector<int>*>(iArg1), (MINT32)iArg2, (MINT32)iArg3);
            break;
        case EAECtrl_GetAEInitExpSetting:
            i4Ret = getAEInitExpoSetting(*reinterpret_cast<AEInitExpoSetting_T*>(iArg1));
            break;
        case EAECtrl_GetISOSpeedMode:
            *(reinterpret_cast<MINT32*>(iArg1)) = (m_u4AEISOSpeed == LIB3A_AE_ISO_SPEED_AUTO) ? m_rSensorCurrentInfo.u4Sensitivity : m_u4AEISOSpeed;
            break;
        case EAECtrl_GetAEMaxMeterAreaNum:
            *(reinterpret_cast<MINT32*>(iArg1)) = MAX_METERING_AREAS;
            break;
        case EAECtrl_GetAEMeterMode:
            *(reinterpret_cast<MINT32*>(iArg1)) = m_eAEMeterMode;
            break;
        case EAECtrl_GetCapParams:
            i4Ret = getCaptureParams(*reinterpret_cast<AE_MODE_CFG_T*>(iArg1));
            break;
        case EAECtrl_GetCCUResultCBActive:
            return getCCUresultCBActive(reinterpret_cast<MVOID*>(iArg1));
        case EAECtrl_GetCurrentPlineTable:
            i4Ret = getCurrentPlineTable(*reinterpret_cast<strAETable*>(iArg1), *reinterpret_cast<strAETable*>(iArg2), *reinterpret_cast<strAETable*>(iArg3), *reinterpret_cast<strAFPlineInfo*>(iArg4));
            break;
        case EAECtrl_GetCurrentPlineTableF:
            i4Ret = getCurrentPlineTableF(*reinterpret_cast<strFinerEvPline*>(iArg1));
            break;
        case EAECtrl_GetDebugInfo:
            AE_LOG_IF(m_3ALogEnable,"[%s:EAECtrl_GetDebugInfo] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
            i4Ret = getAlgoDebugInfo(reinterpret_cast<MVOID*>(iArg1), reinterpret_cast<MVOID*>(iArg2));
            break;
        case EAECtrl_GetEVCompensateIndex:
            *(reinterpret_cast<MINT32*>(iArg1)) = getEVCompensateIndex();
            break;
        case EAECtrl_GetExposureInfo:
            i4Ret = getExposureInfo(*reinterpret_cast<ExpSettingParam_T*>(iArg1));
            break;
        case EAECtrl_GetNVRAParam:
            i4Ret = getNVRAMParam(reinterpret_cast<MVOID *>(iArg1), reinterpret_cast<MUINT32 *>(iArg2));
            break;
        case EAECtrl_GetSensorDeviceInfo:
            i4Ret = getNvramData(m_eSensorDev);
            getSensorDeviceInfo(*reinterpret_cast<AE_DEVICES_INFO_T*>(iArg1));
            break;
        case EAECtrl_GetSensorRollingShutter:
            *(reinterpret_cast<MINT64*>(iArg1)) = getSensorRollingShutter();
            break;
        case EAECtrl_GetIsAELockSupported:
            *(reinterpret_cast<MINT32*>(iArg1)) = MTRUE;//isAELockSupported();
            break;
        case EAECtrl_GetCapDiffEVState:
            switchCapureDiffEVState((MINT32)iArg1,  *reinterpret_cast<CCU_strAEOutput*>(iArg2));
            break;
        case EAECtrl_GetExpSettingByShutterISOPriority:
            switchExpSettingByShutterISOpriority(*reinterpret_cast<AE_EXP_SETTING_T*>(iArg1), *reinterpret_cast<AE_EXP_SETTING_T*>(iArg2));
            break;
        case EAECtrl_GetNeedPresetControlCCU:
            return (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT ? MTRUE : MFALSE);

        // --------------------------------- Set AE Info ---------------------------------
        case EAECtrl_SetContinueShot:
            i4Ret = IsAEContinueShot((MBOOL)iArg1);
            break;
        case EAECtrl_SetPlineTableLimitation:
            i4Ret = modifyAEPlineTableLimitation((MBOOL)iArg1, (MBOOL)iArg2, (MUINT32)iArg3, (MUINT32)iArg4);
            break;
        case EAECtrl_SetPresetControlCCU:
            i4Ret = controlCCUAE();
            if (m_u4DynamicHFPS > 30) presetCtrlDynamicHFPS();
            break;
        case EAECtrl_SetAAOMode:
            AE_LOG( "[setAAOMode] m_rAEConfigInfo.u4AAOmode: %d -> %d (0:8/12bits, 1:14bits)\n", m_rAEConfigInfo.u4AAOmode, (MUINT32)iArg1);
            break;
        case EAECtrl_SetAAOProcInfo:
            i4Ret = setAAOProcInfo(reinterpret_cast<CCU_AAO_PROC_INFO_T const *>(iArg2));
            break;
        case EAECtrl_SetAutoFlickerMode:
            i4Ret = setAEAutoFlickerMode((MUINT32)iArg1);
            break;
        case EAECtrl_SetLimiterMode:
            if(m_bAElimitor != (MBOOL)iArg1) {
                AE_LOG( "[%s:EAECtrl_SetLimiterMode] AE limitor: %d -> %d\n", __FUNCTION__, m_bAElimitor, (MBOOL)iArg1);
                m_bAElimitor = (MBOOL)iArg1;
            }
            break;
        case EAECtrl_SetSMBuffermode:
            //if (m_bEnSWBuffMode != (MBOOL)iArg1){
                AE_LOG( "[%s:EAECtrl_SetSMBuffermode] Slow Motion AE Buffer Mode Enable %d -> %d , Sample = %d -> %d \n", __FUNCTION__, m_bEnSWBuffMode, (MBOOL)iArg1, m_i4SMSubSamples, (MINT32)iArg2);
                m_bEnSWBuffMode  = (MBOOL)iArg1;
                m_i4SMSubSamples = (MINT32)iArg2;
                if (m_i4SMSubSamples == 4)
                    i4Ret = enableAEManualPline(EAEManualPline_SM120FPS, m_bEnSWBuffMode);
                else if (m_i4SMSubSamples == 8)
                    i4Ret = enableAEManualPline(EAEManualPline_SM240FPS, m_bEnSWBuffMode);
                else if (m_i4SMSubSamples == 16)
                    i4Ret = enableAEManualPline(EAEManualPline_SM480FPS, m_bEnSWBuffMode);
                else
                    i4Ret = enableAEManualPline(EAEManualPline_SM120FPS, MFALSE);
            //}
            break;
        case EAECtrl_SetState2Converge:
            AE_LOG( "[%s:EAECtrl_SetState2Converge] Chage to converge state, Old state:%d\n", __FUNCTION__, m_eAEState);
            m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
            m_AEPerFrameInfo.rAEUpdateInfo.i4AEState = static_cast<MINT32>(m_eAEState);
            break;
        case EAECtrl_SetTargetMode:
            i4Ret = SetAETargetMode((eAETargetMODE) iArg1);
            break;
        case EAECtrl_SetCamScenarioMode:
            i4Ret = setCamScenarioMode((MUINT32)iArg1, (MBOOL)iArg2);
            break;
        case EAECtrl_SetEMVHDRratio:
            i4Ret = setEMVHDRratio((MUINT32)iArg1);
            break;
        case EAECtrl_SetFDInfo:
            i4Ret = setFDInfo(reinterpret_cast<MVOID *>(iArg1), (MINT32)iArg2, (MINT32)iArg3);
            break;
        case EAECtrl_SetMVHDR3ExpoProcInfo:
            i4Ret = setMVHDR3ExpoProcInfo(reinterpret_cast<MVOID *>(iArg1), (MUINT32)iArg2);
            break;
        case EAECtrl_SetNVRAMIndex:
            i4Ret = setNVRAMIndex((MUINT32)iArg1, (MBOOL)iArg2);
            break;
        case EAECtrl_SetOTInfo:
            i4Ret = setOTInfo(reinterpret_cast<MVOID *>(iArg1));
            break;
        case EAECtrl_SetSensorMode:
            AE_LOG( "sensorMode(%d) %d, %d",(MINT32)(iArg1), (MUINT32)(iArg2), (MUINT32)(iArg3));
            i4Ret = setSensorMode((MINT32)iArg1, (MUINT32)iArg2, (MUINT32)iArg3);
            break;
        case EAECtrl_SetStrobeMode:
            i4Ret = setStrobeMode((MBOOL)iArg1);
            break;
        case EAECtrl_SetzCHDRShot:
            i4Ret = setzCHDRShot((MBOOL)iArg1);
            break;
        case EAECtrl_SetUpdateAEBV:
            i4Ret = updateAEBV(reinterpret_cast<MVOID *>(iArg1));
            break;
        case EAECtrl_SetUnderExpdeltaBVIdx:
            i4Ret = updateAEUnderExpdeltaBVIdx((MINT32)iArg1);
            break;
        case EAECtrl_SetCaptureParams:
            i4Ret = updateCaptureParams(*reinterpret_cast<AE_MODE_CFG_T*>(iArg1));
            break;
        case EAECtrl_SetISPNvramLTMIndex:
            i4Ret = updateISPNvramLTMIndex((MINT32)(iArg1));
            break;
        case EAECtrl_SetPreviewParams:
            i4Ret = updatePreviewParams(*reinterpret_cast<AE_MODE_CFG_T*>(iArg1), (MINT32)iArg2, (MINT32)iArg3);
            break;
        case EAECtrl_SetSensorListenerParams:
            i4Ret = updateSensorListenerParams((MINT32 *)(iArg1));
            break;
        case EAECtrl_SetSensorParams:
            i4Ret = UpdateSensorParams(*reinterpret_cast<AE_SENSOR_PARAM_T*>(iArg1));
            break;
        case EAECtrl_SetStereoDenoiseRatio:
            i4Ret = updateStereoDenoiseRatio((MINT32 *)(iArg1));
            break;
        case EAECtrl_EnableAEOneShotControl:
            i4Ret = enableAEOneShotControl((MBOOL)iArg1);
            break;
        case EAECtrl_EnableAEStereoManualPline:
            i4Ret = enableAEStereoManualPline((MBOOL)iArg1);
            break;
        case EAECtrl_EnableAISManualPline:
            EAEManualPline_T eAEManualCapPline;

            switch(iArg1) {
                case 0x01:
                    eAEManualCapPline = EAEManualPline_AIS1Capture;
                    bEnable = MTRUE;
                    i4Ret = enableAEManualCapPline(eAEManualCapPline, bEnable);
                    break;
                case 0x02:
                    eAEManualCapPline = EAEManualPline_AIS2Capture;
                    bEnable = MTRUE;
                    i4Ret = enableAEManualPline(eAEManualCapPline, bEnable);
                    break;
                case 0x00:
                default:
                    eAEManualCapPline = EAEManualPline_Default;
                    bEnable = MFALSE;
                    if (m_eAEManualPline == EAEManualPline_AIS2Capture){
                        i4Ret = enableAEManualPline(eAEManualCapPline, bEnable);
                    } else {
                        i4Ret = enableAEManualCapPline(eAEManualCapPline, bEnable);
                    }
                    break;
            }
            AE_LOG( "[%s():EAECtrl_EnableAISManualPline] AISPline:%d(%d) Enable:%d\n", __FUNCTION__, iArg1, eAEManualCapPline, bEnable);
            break;
        case EAECtrl_EnableBMDNManualPline:
            i4Ret = enableAEManualCapPline(EAEManualPline_BMDNCapture, (MBOOL)iArg1);
            break;
        case EAECtrl_EnableEISRecording:
            EAEManualPline_T eAEManualPline;

            switch(iArg1) {
                case 0x01:
                    eAEManualPline = EAEManualPline_EISRecord1;
                    bEnable = MTRUE;
                    break;
                case 0x02:
                    eAEManualPline = EAEManualPline_EISRecord2;
                    bEnable = MTRUE;
                    break;
                case 0x00:
                default:
                    eAEManualPline = EAEManualPline_Default;
                    bEnable = MFALSE;
                    break;
            }

            AE_LOG( "[%s():EAECtrl_EnableEISRecording] EISPline:%d(%d) Enable:%d\n", __FUNCTION__, iArg1, eAEManualPline, bEnable);
            i4Ret = enableAEManualPline(eAEManualPline, bEnable);
            break;
        case EAECtrl_EnableFlareInManualControl:
            break;
        case EAECtrl_EnableHDRShot:
            i4Ret = enableHDRShot((MBOOL)iArg1);
            break;
        case EAECtrl_EnableMFHRManualPline:
            i4Ret = enableAEManualCapPline(EAEManualPline_MFHRCapture, (MBOOL)iArg1);
            break;
        case EAECtrl_EnableAE:
            m_bEnableAE = MTRUE;
            AE_LOG( "enableAE()\n");
            m_pICcuAe->ccuControl(MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE, NULL, NULL);
            CAM_LOGD( "[%s():EAECtrl_SetAEMgrPreState] reset CCU manual control \n", __FUNCTION__);
            break;
        case EAECtrl_DisableAE:
            m_bEnableAE = MFALSE;
            AE_LOG( "disableAE()\n");
            break;
        case EAECtrl_SetCCUManualControl:
            i4Ret = CCUManualControl((MUINT32)iArg1, (MUINT32)iArg2,(MUINT32)iArg3, (MBOOL)iArg4);
            break;
        case EAECtrl_SetDoBackAEInfo:
            i4Ret = doBackupAE();
            break;
        case EAECtrl_SetDoRestoreAEInfo:
            i4Ret = doRestoreAE();
            break;
        case EAECtrl_SetAFAELock:
            i4Ret = setAFAELock((MBOOL)iArg1);
            break;
        case EAECtrl_SetFDenable:
            i4Ret = setFDenable((MBOOL)iArg1);
            break;
        case EAECtrl_SetSensorbyI2C:
            i4Ret = updateSensorbyI2C();
            break;
        case EAECtrl_SetSensorbyI2CBufferMode:
            i4Ret = updateSensorbyI2CBufferMode();
            break;
        case EAECtrl_SetCCUOnOff:
            i4Ret = setCCUOnOff((MBOOL)iArg1);
            break;
        case EAECtrl_SetExposureSetting:
            i4Ret = setExpoSetting((MUINT32)iArg1, (MUINT32)iArg2,(MUINT32)iArg3);
            break;
        case EAECtrl_SetAEMgrPreState:
            if(m_u4AEMgrStateCCU != (MUINT32)iArg1) {
                AE_LOG( "[%s:EAECtrl_SetAEMgrPreState] m_u4AEMgrStateCCU: %d -> %d \n", __FUNCTION__, m_u4AEMgrStateCCU, (MBOOL)iArg1);
                m_u4AEMgrStateCCU = (MUINT32)iArg1;
                resetCCUStable();
                if((getAEAlgoState() != AE_STATE_POSTCAP) && (m_eAEMode!=LIB3A_AE_MODE_OFF)) disableManualCCU(MTRUE); //no need to disable manual in profession mode
            }
            break;
        case EAECtrl_SetAEMode:
            setAEMode((MUINT32)iArg1, MTRUE);
            m_bCCTEnable = (MBOOL)iArg2;
            AE_LOG( "[%s:EAECtrl_SetAEMode] bCCTEnable:%d Aemode:%d \n", __FUNCTION__, m_bCCTEnable, (MUINT32)iArg1);
            break;
        case EAECtrl_SetDynamicHFPS:
            if (m_u4DynamicHFPS != (MUINT32)iArg1) {
                AE_LOG("[%s:EAECtrl_SetDynamicHFPS] i4SensorDev: %d, m_u4DynamicHFPS: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_u4DynamicHFPS, (MUINT32)iArg1);
                m_u4DynamicHFPS = (MUINT32)iArg1;
                i4Ret = setDynamicHFPS();
            }
            break;
        case EAECtrl_NotifyMainFlashON:
            if (m_bMainFlashON != (MBOOL)iArg1) {
                AE_LOG("[%s:EAECtrl_NotifyMainFlashON] i4SensorDev: %d, m_bMainFlashON: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_bMainFlashON, (MBOOL)iArg1);
                m_bMainFlashON = (MBOOL)iArg1;
            }
            break;
        case EAECtrl_SetSkipCalAE:
            if (m_bSkipCalAE != (MBOOL)iArg1) {
                AE_LOG("[%s:EAECtrl_SetSkipCalAE] i4SensorDev: %d, m_bSkipCalAE: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_bSkipCalAE, (MBOOL)iArg1);
                m_bSkipCalAE = (MBOOL)iArg1;
                if (m_bSkipCalAE == MFALSE) disableManualCCU(MTRUE);
            }
            break;
        case EAECtrl_SetFDSkipCalAE:
            if (m_bFDSkipCalAE != (MBOOL)iArg1) {
                AE_LOG("[%s:EAECtrl_SetFDSkipCalAE] i4SensorDev: %d, m_bFDSkipCalAE: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_bFDSkipCalAE, (MBOOL)iArg1);
                m_bFDSkipCalAE = (MBOOL)iArg1;
            }
            break;
      case EAECtrl_EnableShutterISOPriority: // iArg1: enable; iArg2: specified shutter; iArg3: specified ISO
            AE_LOG_IF(m_3ALogEnable,"[%s:EAECtrl_EnableShutterISOPriority] i4SensorDev: %d, mode: %d, shutter: %lld, iso: %d\n", __FUNCTION__, m_eSensorDev, (MBOOL)iArg1, (MINT64)iArg2, (MINT32)iArg3);
            if(m_bManualByShutterISOPriorityMode) {
                disableManualCCU(MFALSE);
                m_bManualByShutterISOPriorityMode = MFALSE;
                AE_LOG("[%s:EAECtrl_EnableShutterISOPriority] resume after long-exposure capture\n", __FUNCTION__);
            }
            if ((MBOOL)iArg1) {
                if ((MINT64)iArg2 <= 0 && m_AePriorityParam.u4FixISO != (MINT32)iArg3 && (MINT32)iArg3 > 0) { // ISO priority
                    m_bShutterISOPriorityMode = MTRUE;
                    m_AePriorityParam.eMode = CCU_AE_PRIORITY_ISO;
                    m_AePriorityParam.u4FixShutter = 0;
                    m_AePriorityParam.u4FixISO = (MINT32)iArg3;
                    m_AePriorityParam.u4MinShutter = MIN_MANUAL_EXP_VALUE;
                    m_AePriorityParam.u4MaxShutter = MAX_MANUAL_EXP_VALUE;
                    m_AePriorityParam.u4MinISO = MIN_MANUAL_ISO_VALUE;
                    m_AePriorityParam.u4MaxISO = (m_u4AdbISOspeed>0)?(MAX_ADB_ISO_VALUE):(MAX_MANUAL_ISO_VALUE); //(m_u4AdbISOspeed>0) for AF calibration
                    AE_LOG("[%s:EAECtrl_EnableShutterISOPriority] i4SensorDev: %d, ISO priority: %d, min/max shutter: %d/%d, min/max ISO: %d/%d\n", __FUNCTION__, m_eSensorDev,
                            m_AePriorityParam.u4FixISO, m_AePriorityParam.u4MinShutter, m_AePriorityParam.u4MaxShutter, m_AePriorityParam.u4MinISO, m_AePriorityParam.u4MaxISO);
                    enableAEManualPline(EAEManualPline_ShtterISOPriority, MTRUE);
                    m_bNeedCheckAEPline = MTRUE;
                }
                else if ((MINT32)iArg3 <= 0 && m_AePriorityParam.u4FixShutter != (MINT64)iArg2/1000 && (MINT64)iArg2 > 0) { // shutter priority
                    m_bShutterISOPriorityMode = MTRUE;
                    if ((MINT64)iArg2/1000 <= MAX_MANUAL_EXP_VALUE) {
                        m_AePriorityParam.eMode = CCU_AE_PRIORITY_SHUTTER;
                        m_AePriorityParam.u4FixShutter = (MINT64)iArg2/1000;
                        m_AePriorityParam.u4FixISO = 0;
                        m_AePriorityParam.u4MinShutter = MIN_MANUAL_EXP_VALUE;
                        m_AePriorityParam.u4MaxShutter = MAX_MANUAL_EXP_VALUE;
                        m_AePriorityParam.u4MinISO = MIN_MANUAL_ISO_VALUE;
                        m_AePriorityParam.u4MaxISO = (m_u4AdbISOspeed>0)?(MAX_ADB_ISO_VALUE):(MAX_MANUAL_ISO_VALUE); //(m_u4AdbISOspeed>0) for AF calibration
                        AE_LOG("[%s:EAECtrl_EnableShutterISOPriority] i4SensorDev: %d, shutter priority: %d, min/max shutter: %d/%d, min/max ISO: %d/%d\n", __FUNCTION__, m_eSensorDev,
                                m_AePriorityParam.u4FixShutter, m_AePriorityParam.u4MinShutter, m_AePriorityParam.u4MaxShutter, m_AePriorityParam.u4MinISO, m_AePriorityParam.u4MaxISO);
                        enableAEManualPline(EAEManualPline_ShtterISOPriority, MTRUE);
                        m_bNeedCheckAEPline = MTRUE;
                    }
                    else {
                        AE_SENSOR_PARAM_T strSensorParams;
                        strSensorParams.u8ExposureTime  = (MINT64)iArg2;
                        strSensorParams.u4Sensitivity   =  (m_rAEAlgoOutput.EvSetting.u4Eposuretime * m_rAEAlgoOutput.u4ISO) / (strSensorParams.u8ExposureTime/1000);
                        if (strSensorParams.u4Sensitivity < MIN_MANUAL_ISO_VALUE) strSensorParams.u4Sensitivity = MIN_MANUAL_ISO_VALUE;
                        strSensorParams.u8FrameDuration = strSensorParams.u8ExposureTime;
                        UpdateSensorParams(strSensorParams, MTRUE);
                        AE_LOG("[%s:EAECtrl_EnableShutterISOPriority] i4SensorDev: %d, manual exposure with shutter priority: %lld, shutter/ISO: %lld/%d\n",
                                __FUNCTION__, m_eSensorDev, (MINT64)iArg2/1000, strSensorParams.u8ExposureTime, strSensorParams.u4Sensitivity);
                        m_bManualByShutterISOPriorityMode = MTRUE;
                    }
                }
            }
            else if ((MBOOL)iArg1 == MFALSE && m_bShutterISOPriorityMode != (MBOOL)iArg1){
                m_bShutterISOPriorityMode = MFALSE;
                memset(&m_AePriorityParam, 0, sizeof(CCU_AE_PRIORITY_INFO));
                m_AePriorityParam.eMode = CCU_AE_PRIORITY_OFF;
                AE_LOG("[%s:EAECtrl_EnableShutterISOPriority] i4SensorDev: %d, priority off\n", __FUNCTION__, m_eSensorDev);
                enableAEManualPline(EAEManualPline_ShtterISOPriority, MFALSE);
            }
            break;
      case EAECtrl_EnableSuperNightShot: // iArg1: enable; iArg2: AeCustParam
            if (m_bSuperNightShotMode != (MBOOL)iArg1) {
                AE_LOG("[%s:EAECtrl_EnableSuperNightShot] i4SensorDev: %d, m_bSuperNightShotMode: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_bSuperNightShotMode, (MBOOL)iArg1);
                m_bSuperNightShotMode = (MBOOL)iArg1;
                if (m_bSuperNightShotMode && reinterpret_cast<CCU_AE_CUST_Super_Night_Param_T*>(iArg2) != NULL)
                    memcpy(&m_AeCustParam, reinterpret_cast<CCU_AE_CUST_Super_Night_Param_T*>(iArg2), sizeof(CCU_AE_CUST_Super_Night_Param_T));
                if (m_bIsCCUStart) { // need to notify CCU
                    ccu_super_night_mode_info super_night_mode_info;
                    super_night_mode_info.enable = m_bSuperNightShotMode;
                    memcpy(&(super_night_mode_info.ae_cust_param), &m_AeCustParam, sizeof(CCU_AE_CUST_Super_Night_Param_T));
                    m_pICcuAe->ccuControl(MSG_TO_CCU_SET_SUPER_NIGHT_MODE, &super_night_mode_info, NULL);
                    CAM_LOGD( "[%s():EAECtrl_EnableSuperNightShot] MSG_TO_CCU_SET_SUPER_NIGHT_MODE, enable: %d\n", __FUNCTION__, super_night_mode_info.enable);
                }
            }
            break;
    }
    return i4Ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEParams(AE_PARAM_SET_INFO const &rNewParam)
{
    MINT32 i4Ret = 0;

    AE_LOG( "[%s()] i4SensorDev:%d i4MinFps:%d i4MaxFps:%d u4AeMeterMode:%d i4RotateDegree:%d i4IsoSpeedMode:%d rMeteringAreas Cnt:%d Left:%d Top:%d Right:%d Bottom:%d Weight:%d bIsAELock:%d i4ExpIndex: %d fExpCompStep:%f u4AeMode:%d i4DenoiseMode:%d u4AntiBandingMode:%d u4CamMode:%d u4ShotMode:%d u4SceneMode:%d bBlackLvlLock:%d u4ZoomXOffset:%d u4ZoomYOffset:%d u4ZoomWidth:%d u4ZoomHeight:%d i4ZoomRatio:%d u1HdrMode:%d \n",
              __FUNCTION__, m_eSensorDev, rNewParam.i4MinFps, rNewParam.i4MaxFps, rNewParam.u4AeMeterMode, rNewParam.i4RotateDegree,
              rNewParam.i4IsoSpeedMode, rNewParam.rMeteringAreas.u4Count, rNewParam.rMeteringAreas.rAreas[0].i4Left, rNewParam.rMeteringAreas.rAreas[0].i4Top,
              rNewParam.rMeteringAreas.rAreas[0].i4Right, rNewParam.rMeteringAreas.rAreas[0].i4Bottom, rNewParam.rMeteringAreas.rAreas[0].i4Weight,
              rNewParam.bIsAELock, rNewParam.i4ExpIndex, rNewParam.fExpCompStep, rNewParam.u4AeMode, rNewParam.i4DenoiseMode, rNewParam.u4AntiBandingMode,
              rNewParam.u4CamMode, rNewParam.u4ShotMode, rNewParam.u4SceneMode, rNewParam.bBlackLvlLock, rNewParam.u4ZoomXOffset, rNewParam.u4ZoomYOffset,
              rNewParam.u4ZoomWidth, rNewParam.u4ZoomHeight, rNewParam.i4ZoomRatio, rNewParam.u1HdrMode);

    i4Ret = setAEMinMaxFrameRate(rNewParam.i4MinFps, rNewParam.i4MaxFps)
          | setAEMeteringMode(rNewParam.u4AeMeterMode)
          | setAERotateDegree(rNewParam.i4RotateDegree)
          | setAEISOSpeed(rNewParam.i4IsoSpeedMode)
          | setAEMeteringArea(&rNewParam.rMeteringAreas)
          | setAPAELock(rNewParam.bIsAELock, MTRUE)
          | setAEEVCompIndex(rNewParam.i4ExpIndex, rNewParam.fExpCompStep)
          | setAEMode(rNewParam.u4AeMode)
          | enableStereoDenoiseRatio(rNewParam.i4DenoiseMode)
          | setAEFlickerMode(rNewParam.u4AntiBandingMode)
          | setAECamMode(rNewParam.u4CamMode)
          | setAEShotMode(rNewParam.u4ShotMode)
          | setSceneMode(rNewParam.u4SceneMode)
          | bBlackLevelLock(rNewParam.bBlackLvlLock)
          | setZoomWinInfo(rNewParam.u4ZoomXOffset, rNewParam.u4ZoomYOffset, rNewParam.u4ZoomWidth, rNewParam.u4ZoomHeight)
          | setDigZoomRatio(rNewParam.i4ZoomRatio)
          | setAEHDRMode(rNewParam.u1HdrMode);

    return i4Ret;
}

