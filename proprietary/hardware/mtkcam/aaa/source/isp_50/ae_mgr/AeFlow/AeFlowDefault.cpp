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
#define LOG_TAG "AeFlowDefault"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG (1)
#endif

#include <stdlib.h>
#include "AeFlowDefault.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_tuning_mgr.h>
#include <aaa_scheduler.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <ae_flow_custom.h>

#define HDR 1

#define AE_FLOW_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)
#define AE_FLOW_LOG_IF(cond, fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE_IF(cond, fmt, ##arg); \
        } else { \
            CAM_LOGD_IF(cond, fmt, ##arg); \
        } \
    }while(0)
#define EN_3A_SCHEDULE_LOG 2

using namespace NS3Av3;
using namespace NSIspTuning;

static std::array<MY_INST_AEFLOW, SENSOR_IDX_MAX> gMultiton;

IAeFlow *
AeFlowDefault::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_AEFLOW& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeFlowDefault>(sensor);
    } );

    return rSingleton.instance.get();
}

AeFlowDefault::
AeFlowDefault(ESensorDev_T sensor)
: m_pIAeAlgo(NULL)
, m_eSensorDev(sensor)
, m_3ALogEnable(0)
, m_pAEParam(NULL)
, m_pAENVRAM(NULL)
, m_u4AEExitStableCnt(0)
, m_u4AEScheduleCnt(0)
, m_i4WaitVDNum(0)
, m_bAEMonitorStable(MFALSE)
, m_bManualOverSpec(MFALSE)
, m_u4HwMagicNum(0)
, m_bAdbAEEnable(MFALSE)
, m_bAdbAELock(MFALSE)
, m_bAdbAELog(MFALSE)
, m_bPerframeAEFlag(MFALSE)
, m_bTouchAEAreaChange(MFALSE)
, m_eAETargetMode(AE_MODE_NORMAL)
, m_bAAASchedule(MFALSE)
, m_bAElimitor(MFALSE)
, m_bAdbAEPreviewUpdate(MFALSE)
, m_i4AdbAEISPDisable(0)
, m_u4AdbAEShutterTime(0)
, m_u4AdbAESensorGain(0)
, m_u4AdbAEISPGain(0)
, m_bTGValid(MFALSE)
, m_bTGReturn(MFALSE)
, m_bAFTouchROISet(MFALSE)
, m_u2VCModeSelect(0)
, m_bPerFrameAEWorking(MFALSE)
{
    memset(&m_strAECycleInfo, 0, sizeof(strAECycleInfo));
    memset(&m_PreEvSettingQueue[0], 0, MAX_AE_PRE_EVSETTING *sizeof(strEvSetting));
    memset(&m_BackupEvSetting, 0, sizeof(strEvSetting));
    memset(&m_u4PreAEidxQueue[0], 0, MAX_AE_PRE_EVSETTING *sizeof(MUINT32));
    memset(&m_u4PreTgIntAEidxQueue[0], 0, MAX_AE_PRE_EVSETTING *sizeof(MUINT32));
    AE_FLOW_LOG("[AeFlowDefault:Construct] SensorDev:%d\n", (MUINT32)m_eSensorDev);
}

MVOID
AeFlowDefault::
start(MVOID * input)
{
    // Adb command input
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
    property_get("vendor.debug.ae_mgr.enable", value, "0");
    m_bAdbAEEnable = atoi(value);

    AE_FLOW_CPUSTART_INFO_T* pinput = (AE_FLOW_CPUSTART_INFO_T*) input;
    m_pIAeAlgo = (NS3A::IAeAlgo*)pinput->pIAeAlgo;
    if (m_pIAeAlgo == NULL)
        AE_FLOW_LOG("[%s()] SensorDev:%d Null AeAlgo instance:%p/%p\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_pIAeAlgo, (NS3A::IAeAlgo*)pinput->pIAeAlgo);
    m_pAEParam = pinput->pAEInitInput;
    m_pAENVRAM = pinput->pAEInitInputNVRAM;
    m_eAETargetMode = pinput->eAETargetMode;
    m_u2VCModeSelect = pinput->eAETargetMode;
    updateAECycleInfo(pinput->i4CycleVDNum, pinput->i4ShutterDelayFrames, pinput->i4GainDelayFrames, pinput->i4IspGainDelayFrames);
    for (int i = 0; i < MAX_AE_PRE_EVSETTING; i++)
    {
        m_u4PreTgIntAEidxQueue[i] = pinput->u4Index;
        updatePreEvSettingQueue(pinput->u4Exp, pinput->u4Afe, pinput->u4Isp, pinput->u4Index);
    }
    m_u4AEScheduleCnt = 0;
    AE_FLOW_LOG("[%s()] SensorDev:%d LongCaptureThres:%d MiniISOGain:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_pAEParam->strAEParasetting.u4LongCaptureThres, m_pAENVRAM->rDevicesInfo.u4MiniISOGain);
}

MBOOL
AeFlowDefault::
queryStatus(MUINT32 index)
{
    E_AE_FLOW_CPU_STATUS_T eStatus = (E_AE_FLOW_CPU_STATUS_T) index;
    switch (eStatus)
    {
        case E_AE_FLOW_CPU_RESTORE_WAIT:
            return (m_bPerframeAEFlag && (m_strAECycleInfo.m_i4FrameCnt <= m_strAECycleInfo.m_i4CycleVDNum));
        case E_AE_FLOW_CPU_TG_RETURN:
            return m_bTGReturn;
        case E_AE_FLOW_CPU_TG_VALID:
            return m_bTGValid;
        case E_AE_FLOW_CPU_ADB_UPDATE:
            return m_bAdbAEPreviewUpdate;
        case E_AE_FLOW_CPU_ADB_LOCK:
            return m_bAdbAELock;
        case E_AE_FLOW_CPU_CYCLECOUNT_VALID:
            return (m_strAECycleInfo.m_i4FrameCnt > 10);
        case E_AE_FLOW_CPU_MANUAL_OVERSPEC:
            return m_bManualOverSpec;
        default:
            AE_FLOW_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
            return MFALSE;
    }
}

MVOID
AeFlowDefault::
getParam(MUINT32 index, MUINT32& i4value)
{
    E_AE_FLOW_CPU_STATUS_T eStatus = (E_AE_FLOW_CPU_STATUS_T) index;
    switch (eStatus)
    {
        case E_AE_FLOW_CPU_ADB_EXP:
            i4value = m_u4AdbAEShutterTime;
            break;
        case E_AE_FLOW_CPU_ADB_AFE:
            i4value = m_u4AdbAESensorGain;
            break;
        case E_AE_FLOW_CPU_ADB_ISP:
            i4value = m_u4AdbAEISPGain;
            break;
        case E_AE_FLOW_CPU_PREVQUEUE_EXP:
            i4value = m_PreEvSettingQueue[1].u4Eposuretime;
            break;
        case E_AE_FLOW_CPU_PREVQUEUE_AFE:
            i4value = m_PreEvSettingQueue[1].u4AfeGain;
            break;
        default:
            AE_FLOW_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
            return;
    }
}

MVOID
AeFlowDefault::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_FLOW_CPU_TRIGER_T etrigger = (E_AE_FLOW_CPU_TRIGER_T) index;
    switch (etrigger)
    {
        case E_AE_FLOW_CPU_SCHEDULER_RESET:
            if (btrigger == MTRUE)
            {
                m_bAEMonitorStable = MFALSE;
                m_u4AEScheduleCnt = 0;
            }
            else
            {
                if (m_bAEMonitorStable == MTRUE)
                {
                    m_bAEMonitorStable = MFALSE;
                    m_u4AEScheduleCnt = 0;
                }
            }
            break;
        case E_AE_FLOW_CPU_SCHEDULE_COUNT:
            m_u4AEScheduleCnt = 0;
            break;
        case E_AE_FLOW_CPU_SCHEDULE_STABLE:
            m_bAEMonitorStable = btrigger;
            break;
        case E_AE_FLOW_CPU_CYCLEINFO_ADDCOUNT:
            m_strAECycleInfo.m_i4FrameCnt++;
            break;
        case E_AE_FLOW_CPU_CYCLEINFO_RESET:
            m_strAECycleInfo.m_i4FrameCnt = 0;
            m_strAECycleInfo.m_i4FrameTgCnt = 0;
            m_strAECycleInfo.m_u4HwMagicNum = 0;
            break;
        case E_AE_FLOW_CPU_CYCLEINFO_AERESET:
            if (m_strAECycleInfo.m_i4FrameCnt >= 1)
            {
                m_strAECycleInfo.m_i4FrameCnt = 6;
                MY_LOG("[AeMgr::SetAFAELock] AE reschedule \n");
            }
            break;
        case E_AE_FLOW_CPU_CYCLEINFO_COUNTWAIT:
            if (btrigger)
                m_strAECycleInfo.m_i4FrameCnt = 1;
            else
                m_strAECycleInfo.m_i4FrameCnt = 0;
            break;
        default:
            AE_FLOW_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
    }
}

MVOID
AeFlowDefault::
schedulePvAE(MVOID * input, MVOID * output)
{
    AE_FLOW_SCHEDULE_INFO_T* pinputinfo = (AE_FLOW_SCHEDULE_INFO_T*) input;
    AE_FLOW_SCHEDULE_OUTPUT_T* poutputinfo = (AE_FLOW_SCHEDULE_OUTPUT_T*) output;

    //MBOOL bIsHDRflow = (pinputinfo->eAETargetMode != AE_MODE_NORMAL); /* fix build warning */
    //MBOOL bPerframeResult = (pinputinfo->bPerframeAEFlag); /* fix build warning */
    MINT32 i4ActiveItem = 0;
    MBOOL bCalculateAE = MFALSE;
    MBOOL bApplyAE = MFALSE;
    m_u4HwMagicNum = pinputinfo->u4HwMagicNum;
    m_i4WaitVDNum = pinputinfo->i4WaitVDNum;
    m_bPerframeAEFlag = pinputinfo->bPerframeAEFlag;
    m_bTouchAEAreaChange = pinputinfo->bTouchAEAreaChange;
    m_eAETargetMode = pinputinfo->eAETargetMode;
    m_bAAASchedule = pinputinfo->bAAASchedule;
    m_bAElimitor = pinputinfo->bAElimitor;
    m_bAFTouchROISet = pinputinfo->AFTouchROISet;

    if (m_bAAASchedule)
    {
        // Stereo case
        m_u4AEScheduleCnt = 0;
        i4ActiveItem = pinputinfo->i4ActiveItem;
        bCalculateAE = i4ActiveItem & E_AE_AE_CALC;
        bApplyAE = i4ActiveItem & E_AE_AE_APPLY;
        m_bPerframeAEFlag = MFALSE;
        m_i4WaitVDNum = m_i4WaitVDNum%3;
        AE_FLOW_LOG("[monitorAndReschedule:Stereo-3A-Schedule] i4SensorDev:%d ActiveItem:%d Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvY(cur/pre):%d/%d\n",
            m_eSensorDev, i4ActiveItem, bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
    }
    else
    {
        // Single cam
        if (m_bPerframeAEFlag)
        {
            switch (pinputinfo->eAEFlowType)
            {
                case E_AE_FLOW_CPU_DOPVAE:
                    if (pinputinfo->bAEOneShotControl)
                    {
                        monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                        m_bPerframeAEFlag = MFALSE;
                    }
                    else if (m_bTouchAEAreaChange)
                    {
                        if (m_pAEParam->strAEParasetting.bEnableTouchSmooth)
                        {
                            bCalculateAE = MTRUE;
                            bApplyAE = MTRUE;
                            m_bPerFrameAEWorking = MTRUE;
                            AE_FLOW_LOG("[monitorAndReschedule:Perframe Touch AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvY(cur/pre):%d/%d\n",
                                bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                        }
                        else
                        {
                            monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                            m_bPerframeAEFlag = MFALSE;
                        }
                        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Touch AE flow, bTouchPerframe:%d \n", __FUNCTION__, m_pAEParam->strAEParasetting.bEnableTouchSmooth);
                    }
                    else if (m_bAElimitor)
                    {
                        monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                        m_bPerframeAEFlag = MFALSE;
                        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Limiter AE flow \n", __FUNCTION__);
                    }
                    else
                    {
                        bCalculateAE = MTRUE;
                        bApplyAE = MTRUE;
                        m_bPerFrameAEWorking = MTRUE;
                        AE_FLOW_LOG("[monitorAndReschedule:Perframe AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvY(cur/pre):%d/%d\n",
                            bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Perframe AE flow \n", __FUNCTION__);
                    }
                    break;
                case E_AE_FLOW_CPU_DOAFAE:
                    if (m_bAFTouchROISet && m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag && (pinputinfo->bAEReadyCapture == MFALSE))
                    {
                        bCalculateAE = MTRUE;
                        bApplyAE = MTRUE;
                        m_bPerFrameAEWorking = MTRUE;
                        AE_FLOW_LOG("[monitorAndReschedule:Perframe Touch AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvY(cur/pre):%d/%d\n",
                            bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                    }
                    else
                    {
                        monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                    }
                    break;
                case E_AE_FLOW_CPU_DOPRECAPAE:
                    monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                    break;
                case E_AE_FLOW_CPU_DOCAPAE:
                case E_AE_FLOW_CPU_DOAFASSIST:
                case E_AE_FLOW_CPU_DOPVAETG:
                    break;
            }
        }
        else
        {
            monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
        }
    }
    poutputinfo->pModifiedApply = &bApplyAE;
    poutputinfo->pModifiedCalc = &bCalculateAE;
    poutputinfo->pModifiedPerframeFlag = &m_bPerframeAEFlag;
    poutputinfo->pWaitVDNum = &m_i4WaitVDNum;
}

MVOID
AeFlowDefault::
calculateAE(MVOID * input, MVOID * output)
{
    AE_FLOW_CALCULATE_INFO_T* pinputinfo = (AE_FLOW_CALCULATE_INFO_T*) input;
    strAEOutput* poutputinfo = (strAEOutput*) output;

    strAEInput rAEInput;
    memset(&rAEInput , 0, sizeof(strAEInput));
    MBOOL bIsHDRflow = (m_eAETargetMode != AE_MODE_NORMAL);
    // Debug adb command
    if (m_bAdbAEEnable)
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.ae_mgr.lock", value, "0");
        m_bAdbAELock = atoi(value);
        property_get("vendor.debug.ae_mgr.log", value, "0");
        m_bAdbAELog = atoi(value);
        property_get("vendor.debug.ae_mgr.disableISP", value, "0");
        m_i4AdbAEISPDisable = atoi(value); // 0 : No disable, 1 : Disable ISP gain to shutter, 2 : Disable ISP gain to sensor gain
        property_get("vendor.debug.ae_mgr.preview.update", value, "0");
        m_bAdbAEPreviewUpdate = atoi(value);
    }
    switch (pinputinfo->eAEFlowType)
    {
        case E_AE_FLOW_CPU_DOPVAE:
            if (pinputinfo->bAElock || m_bAdbAELock)
            {
                rAEInput.eAeState = AE_STATE_AELOCK;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] AE lock state \n", __FUNCTION__);
            }
            else if (pinputinfo->bAEOneShotControl)
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG("[%s()] One shot state\n", __FUNCTION__);
            }
            else if (m_bTouchAEAreaChange)
            {
                if (m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag && (!m_bAAASchedule))
                {
                    rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Perframe touch AE state\n", __FUNCTION__);
                }
                else
                {
                    rAEInput.eAeState = AE_STATE_ONE_SHOT;
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Non-Perframe touch AE state (one shot)\n", __FUNCTION__);
                }
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Enable per-frame AE for touch:%d\n", __FUNCTION__, m_pAEParam->strAEParasetting.bEnableTouchSmooth);
            }
            else if (m_bAAASchedule)
            {
                if (m_bPerframeAEFlag)
                {
                    rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
                }
                else
                {
                    rAEInput.eAeState = AE_STATE_SLOW_MOTION;
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Stereo mode\n", __FUNCTION__);
                }
            }
            else if (pinputinfo->bEnSWBuffMode)
            {
                //rAEInput.eAeState = AE_STATE_SLOW_MOTION;
                rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Slow motion mode\n", __FUNCTION__);
            }
            else if (m_bAElimitor)
            {
                rAEInput.eAeState = AE_STATE_PANORAMA_MODE;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Panorama mode\n", __FUNCTION__);
            }
            else
            {
                rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
            }
            break;
        case E_AE_FLOW_CPU_DOAFAE:
            if (pinputinfo->bAElock || m_bAdbAELock)
            {
                rAEInput.eAeState = AE_STATE_AELOCK;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[doAFAE] AE lock state\n");
            }
            else if (m_bAFTouchROISet && m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag && (!m_bAAASchedule) && (!pinputinfo->bAEReadyCapture))
            {
                rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                AE_FLOW_LOG("[doAFAE] PerFrame touch AE state:%d\n", m_bPerframeAEFlag);
            }
            else
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG("[doAFAE] One shot state, m_bAFTouchROISet:%d\n", m_bAFTouchROISet);
            }
            break;
        case E_AE_FLOW_CPU_DOPRECAPAE:
            if (pinputinfo->bAElock || m_bAdbAELock)
            {
                rAEInput.eAeState = AE_STATE_AELOCK;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[doPreCapAE] AE lock state\n");
            }
            else
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[doPreCapAE] One shot state\n");
            }
            break;
        case E_AE_FLOW_CPU_DOCAPAE:
            rAEInput.eAeState = AE_STATE_POST_CAPTURE;
            break;
        case E_AE_FLOW_CPU_DOAFASSIST:
            rAEInput.eAeState = AE_STATE_AFASSIST;
            break;
        case E_AE_FLOW_CPU_DOPVAETG:
            m_bTGReturn = MFALSE;
            if (pinputinfo->bAElock || m_bAdbAELock)
            {
                rAEInput.eAeState = AE_STATE_AELOCK;
                AE_FLOW_LOG("[%s()] AE_STATE_AELOCK", __FUNCTION__);
            }
            else if (pinputinfo->bAEOneShotControl)
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG("[%s()] AE_STATE_ONE_SHOT", __FUNCTION__);
            }
            else if (m_bTouchAEAreaChange)
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG("[%s()] Enable one shot for touch AE enable:%d\n", __FUNCTION__, m_bTouchAEAreaChange);
            }
            else if (bIsHDRflow || (pinputinfo->bEnSWBuffMode))
            {
                rAEInput.eAeState = AE_STATE_SLOW_MOTION;
                AE_FLOW_LOG("[%s()] Stereo or VHDR or Slow Motion flow\n", __FUNCTION__);
            }
            else if (m_bAElimitor == MTRUE)
            {
                rAEInput.eAeState = AE_STATE_PANORAMA_MODE;
                AE_FLOW_LOG("[%s()] PANORAMA \n", __FUNCTION__);
            }
            else
            {
                rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
            }
            // not normal preview, skip calculation
            if (rAEInput.eAeState != AE_STATE_NORMAL_PREVIEW)
            {
                AE_FLOW_LOG("[%s()] NOT Normal preview, skip TG Calculation\n", __FUNCTION__);
                m_bTGReturn = MTRUE;
                return;
            }
            break;
        default:
            AE_FLOW_LOG("[%s():Warning] SensorDev:%d FlowType:%d\n", __FUNCTION__, m_eSensorDev, pinputinfo->eAEFlowType);
    }
    // Prepare param for algorithm
    rAEInput.pAESatisticBuffer = pinputinfo->pAEStatisticBuf;
    rAEInput.eAeTargetMode = m_eAETargetMode;
    rAEInput.i8TimeStamp = pinputinfo->i8TimeStamp;
    m_strAECycleInfo.m_u4HwMagicNum = m_u4HwMagicNum;
    // Prepare perframe AE info to algorithm
    if (m_bPerframeAEFlag)
    {
        rAEInput.CycleInfo = m_strAECycleInfo;
        for (int i = 0; i < MAX_AE_PRE_EVSETTING; i++)
        {
            rAEInput.PreEvSetting[i] = m_PreEvSettingQueue[i];
            rAEInput.u4PreAEidx[i] = m_u4PreAEidxQueue[i];
            rAEInput.u4PreTgIntAEidx[i] = m_u4PreTgIntAEidxQueue[i];
            rAEInput.rHdrGyroInfo.i4Acce[i] = *(pinputinfo->i4AcceInfo + i);
            rAEInput.rHdrGyroInfo.i4Gyro[i] = *(pinputinfo->i4GyroInfo + i);
        }
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s():PassToAlgo] EvSetting/AEidx/Tgidx [N-1]:%d/%d/%d/%d/%d [N-2]:%d/%d/%d/%d/%d [N-3]:%d/%d/%d/%d/%d FrameCnt/Tgcnt/Magic(%d/%d/%d)\n", __FUNCTION__,
            rAEInput.PreEvSetting[0].u4Eposuretime, rAEInput.PreEvSetting[0].u4AfeGain, rAEInput.PreEvSetting[0].u4IspGain, rAEInput.u4PreAEidx[0], rAEInput.u4PreTgIntAEidx[0],
            rAEInput.PreEvSetting[1].u4Eposuretime, rAEInput.PreEvSetting[1].u4AfeGain, rAEInput.PreEvSetting[1].u4IspGain, rAEInput.u4PreAEidx[1], rAEInput.u4PreTgIntAEidx[1],
            rAEInput.PreEvSetting[2].u4Eposuretime, rAEInput.PreEvSetting[2].u4AfeGain, rAEInput.PreEvSetting[2].u4IspGain, rAEInput.u4PreAEidx[2], rAEInput.u4PreTgIntAEidx[2],
            rAEInput.CycleInfo.m_i4FrameCnt, rAEInput.CycleInfo.m_i4FrameTgCnt, rAEInput.CycleInfo.m_u4HwMagicNum);

    }
    if (m_pIAeAlgo != NULL)
    {
        if (pinputinfo->eAEFlowType == E_AE_FLOW_CPU_DOPVAETG)
        {
            // Handle Half -frame AE
            m_bTGValid = m_pIAeAlgo->handleInterAE(&rAEInput, poutputinfo);
            AE_FLOW_LOG("[%s()] EPIPECmd_AE_SMOOTH bValid(%d) \n", __FUNCTION__, m_bTGValid);
            if (m_bTGValid && (m_strAECycleInfo.m_i4FrameCnt > 10))
            {
                updatePreEvSettingQueue(poutputinfo->EvSetting.u4Eposuretime, poutputinfo->EvSetting.u4AfeGain, poutputinfo->EvSetting.u4IspGain, poutputinfo->i4AEidxNext);
            }
            output = poutputinfo;
            m_strAECycleInfo.m_i4FrameTgCnt++;
            return;
        }
        else
        {
            // Handle Full -frame AE
            AaaTimer localTimer("handleAE", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            AAA_TRACE_ALG(handleAE);
            m_pIAeAlgo->handleAE(&rAEInput, poutputinfo);
            AAA_TRACE_END_ALG;
            localTimer.End();
        }
    }

    ////////Modify Exposure for Super Night Shot/////////////
    if (pinputinfo->i4ShotMode != 0x00)
    {
        AE_EXP_SETTING_INPUT_T strExpInput;
        AE_EXP_SETTING_OUTPUT_T strExpOutput;
        AE_Cust_Param_T* pAeCustParam = (AE_Cust_Param_T*) pinputinfo->pAeCustParam;

        memset(&strExpOutput, 0, sizeof(AE_EXP_SETTING_OUTPUT_T));

        strExpInput.bIs60Hz = pinputinfo->bIs60Hz;
        strExpInput.i4SensorId = pinputinfo->i4SensorId;
        strExpInput.u4AlgoExposureTime_us = poutputinfo->EvSetting.u4Eposuretime;
        strExpInput.u4AlgoSensorGain_x1024 = poutputinfo->EvSetting.u4AfeGain;
        strExpInput.u4AlgoISPGain_x1024 = poutputinfo->EvSetting.u4IspGain;
        strExpInput.u4AlgoISO = poutputinfo->u4ISO;
        strExpInput.u41xGainISOvalue = pinputinfo->u41xGainISOvalue;
        strExpInput.u4MinShutter = MIN_EXP_VALUE;
        strExpInput.u4MaxShutter = MAX_EXP_VALUE;
        strExpInput.u4MinSensorGain = pinputinfo->u4MinSensorGain;
        strExpInput.u4MaxSensorGain = pinputinfo->u4MaxSensorGain;
        strExpInput.u4MinISPGain = MIN_ISP_GAIN;
        strExpInput.u4MaxISPGain = MAX_ISP_VALUE;

        calculateNewExposureSetting(pAeCustParam, &strExpInput, &strExpOutput);

        poutputinfo->EvSetting.u4Eposuretime = strExpOutput.u4NewExposureTime_us;
        poutputinfo->EvSetting.u4AfeGain = strExpOutput.u4NewSensorGain_x1024;
        poutputinfo->EvSetting.u4IspGain = strExpOutput.u4NewISPGain_x1024;

        AE_FLOW_LOG("[%s()] Exposure Time:%d SensorGain:%d ISPGain:%d\n", __FUNCTION__, poutputinfo->EvSetting.u4Eposuretime, poutputinfo->EvSetting.u4AfeGain, poutputinfo->EvSetting.u4IspGain);
    }

    ////////Modify Exposure for manual control/////////////
    m_bManualOverSpec = MFALSE;
    if ((pinputinfo->u4AEMode == LIB3A_AE_MODE_OFF) && ((pinputinfo->u4TargetExp == 0) || (pinputinfo->u4TargetISO == 0)))
    {
        AE_EXP_MANUAL_INPUT_T strExpInput;
        AE_EXP_SETTING_OUTPUT_T strExpOutput;

        memset(&strExpOutput, 0, sizeof(AE_EXP_SETTING_OUTPUT_T));

        strExpInput.u4TargetExposureTime = pinputinfo->u4TargetExp;
        strExpInput.u4TargetSensitivity = pinputinfo->u4TargetISO;
        strExpInput.u4OriExposureTime = poutputinfo->EvSetting.u4Eposuretime;
        strExpInput.u4OriSensorGain = poutputinfo->EvSetting.u4AfeGain;
        strExpInput.u4OriISPGain = poutputinfo->EvSetting.u4IspGain;
        strExpInput.u41xGainISOvalue = pinputinfo->u41xGainISOvalue;
        strExpInput.u4MinSensorGain = pinputinfo->u4MinSensorGain;
        strExpInput.u4MaxShutter = pinputinfo->u4MaxShutter;
        strExpInput.u4MaxSensorGain = pinputinfo->u4MaxSensorGain;
        strExpInput.u4MaxISPGain = pinputinfo->u4MaxISPGain;

        calculateExpSettingByShutterISOpriority(&strExpInput, &strExpOutput);

        poutputinfo->EvSetting.u4Eposuretime = strExpOutput.u4NewExposureTime_us;
        poutputinfo->EvSetting.u4AfeGain = strExpOutput.u4NewSensorGain_x1024;
        poutputinfo->EvSetting.u4IspGain = strExpOutput.u4NewISPGain_x1024;

        if ((strExpOutput.bEquivalent == MTRUE) && (strExpInput.u4TargetExposureTime <= MAX_MANUAL_EXP_VALUE) && (strExpInput.u4TargetSensitivity <= MAX_MANUAL_ISO_VALUE))
        {
            MUINT32 u4IndexMax = 0, u4IndexMin = 0, u4Index = 0, u4IndexFMax = 0, u4IndexFMin = 0, u4IndexF = 0;

            if (m_pIAeAlgo != NULL)
            {
                m_pIAeAlgo->getEVIdxInfo_v4p0(u4IndexMax , u4IndexMin , u4Index, u4IndexFMax, u4IndexFMin, u4IndexF);
            }
            u4Index = u4Index + strExpOutput.i4DeltaIndex;

            if (u4Index > u4IndexMax)
            {
                AE_FLOW_LOG("[%s()] New Max u4Index:%d->%d\n", __FUNCTION__, u4Index, u4IndexMax);
                u4Index = u4IndexMax;
            }
            else if (u4Index < u4IndexMin)
            {
                AE_FLOW_LOG("[%s()] New Min u4Index:%d->%d\n", __FUNCTION__, u4Index, u4IndexMin);
                u4Index = u4IndexMin;
            }

            u4IndexF = u4Index * pinputinfo->u4FinerEVIdxBase;
            AE_FLOW_LOG("[%s()] u4Index:%d u4IndexMin:%d u4IndexMax:%d u4IndexF:%d u4IndexFMin:%d u4IndexFMax:%d i4DeltaIndex:%d u4FinerEVIdxBase:%d\n", __FUNCTION__, u4Index, u4IndexMin, u4IndexMax, u4IndexF, u4IndexFMin, u4IndexFMax, strExpOutput.i4DeltaIndex, pinputinfo->u4FinerEVIdxBase);

            if (m_pIAeAlgo != NULL)
            {
                m_pIAeAlgo->setEVIdxInfo_v4p0(u4IndexMax , u4IndexMin , u4Index, u4IndexFMax, u4IndexFMin, u4IndexF);
            }
            m_bManualOverSpec = MTRUE;
        }
    }

    ////////ADB COMMAND////////////
    if (m_eAETargetMode == AE_MODE_NORMAL)
    {
        MUINT32 u4Shutter;
        MUINT32 u4SensorGain;
        MUINT32 u4ISPGain;
        u4Shutter = poutputinfo->EvSetting.u4Eposuretime;
        u4SensorGain = poutputinfo->EvSetting.u4AfeGain;
        u4ISPGain = poutputinfo->EvSetting.u4IspGain;

        if (m_bAdbAEPreviewUpdate)
        {
            char value[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("vendor.debug.ae_mgr.shutter", value, "0");
            m_u4AdbAEShutterTime = atoi(value);
            property_get("vendor.debug.ae_mgr.sensorgain", value, "0");
            m_u4AdbAESensorGain = atoi(value);
            property_get("vendor.debug.ae_mgr.ispgain", value, "0");
            m_u4AdbAEISPGain = atoi(value);
            u4Shutter = m_u4AdbAEShutterTime;
            u4SensorGain = m_u4AdbAESensorGain;
            u4ISPGain = m_u4AdbAEISPGain;
            AE_FLOW_LOG("[%s()] i4SensorDev:%d Apply New Shutter:%d Sensor Gain:%d ISP Gain:%d m_bAdbAEPreviewUpdate:%d\n", __FUNCTION__, m_eSensorDev, m_u4AdbAEShutterTime, m_u4AdbAESensorGain, m_u4AdbAEISPGain, m_bAdbAEPreviewUpdate);
        }
        // Copy Sensor information to output structure
        if (m_i4AdbAEISPDisable)
        {
            AE_FLOW_LOG("[%s()] i4SensorDev:%d Old Shutter:%d Sensor Gain:%d ISP Gain:%d m_i4AdbAEISPDisable:%d\n", __FUNCTION__, m_eSensorDev, u4Shutter, u4SensorGain, u4ISPGain, m_i4AdbAEISPDisable);
            if (m_i4AdbAEISPDisable == 1)
            {
                // Disable ISP gain to shutter
                poutputinfo->EvSetting.u4Eposuretime = (u4Shutter *u4ISPGain) >> 10;
                poutputinfo->EvSetting.u4AfeGain = u4SensorGain;
            }
            else
            {
                // Disable ISP gain to sensor gain
                poutputinfo->EvSetting.u4Eposuretime = u4Shutter;
                poutputinfo->EvSetting.u4AfeGain = (u4SensorGain *u4ISPGain) >> 10;
            }
            poutputinfo->EvSetting.u4IspGain = 1024;
            AE_FLOW_LOG("[%s()] i4SensorDev:%d Modify Shutter:%d Sensor Gain:%d ISP Gain:%d\n", __FUNCTION__, m_eSensorDev, u4Shutter, u4SensorGain, u4ISPGain);
        }
        else
        {
            poutputinfo->EvSetting.u4Eposuretime = u4Shutter;
            poutputinfo->EvSetting.u4AfeGain = u4SensorGain;
            poutputinfo->EvSetting.u4IspGain = u4ISPGain;
        }
    }
    ////////ADB COMMAND////////////

    //Update AE algorihm cycle info
    updatePreEvSettingQueue(poutputinfo->EvSetting.u4Eposuretime, poutputinfo->EvSetting.u4AfeGain, poutputinfo->EvSetting.u4IspGain, poutputinfo->i4AEidxNext);
    output = poutputinfo;
    // AF assist avoid frame count maintain
    if (pinputinfo->eAEFlowType == E_AE_FLOW_CPU_DOAFASSIST)
        return;
    // Frame count maintain
    if ((rAEInput.CycleInfo.m_i4FrameCnt != m_strAECycleInfo.m_i4FrameCnt) && (m_bPerframeAEFlag))
    {
        if (rAEInput.CycleInfo.m_i4FrameCnt > 0)
            m_strAECycleInfo.m_i4FrameCnt = rAEInput.CycleInfo.m_i4FrameCnt;
        else
        AE_FLOW_LOG("Wrong frameCnt:%d %d\n", m_strAECycleInfo.m_i4FrameCnt, rAEInput.CycleInfo.m_i4FrameCnt);
    }

    m_strAECycleInfo.m_i4FrameTgCnt++;
    // Debug
    if (m_bAdbAELog)
    {
        // enable adb log
        MUINT8 uYvalue[AE_BLOCK_NO][AE_BLOCK_NO];
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->getAEBlockYvalues(&uYvalue[0][0], AE_BLOCK_NO *AE_BLOCK_NO);
            AE_FLOW_LOG("i4SensorDev:%d block0:%d %d %d %d %d\n", m_eSensorDev, uYvalue[0][0], uYvalue[0][1], uYvalue[0][2], uYvalue[0][3], uYvalue[0][4]);
            AE_FLOW_LOG("i4SensorDev:%d block1:%d %d %d %d %d\n", m_eSensorDev, uYvalue[1][0], uYvalue[1][1], uYvalue[1][2], uYvalue[1][3], uYvalue[1][4]);
            AE_FLOW_LOG("i4SensorDev:%d block2:%d %d %d %d %d\n", m_eSensorDev, uYvalue[2][0], uYvalue[2][1], uYvalue[2][2], uYvalue[2][3], uYvalue[2][4]);
            AE_FLOW_LOG("i4SensorDev:%d block3:%d %d %d %d %d\n", m_eSensorDev, uYvalue[3][0], uYvalue[3][1], uYvalue[3][2], uYvalue[3][3], uYvalue[3][4]);
            AE_FLOW_LOG("i4SensorDev:%d block4:%d %d %d %d %d\n", m_eSensorDev, uYvalue[4][0], uYvalue[4][1], uYvalue[4][2], uYvalue[4][3], uYvalue[4][4]);
        }
        else
        {
            AE_FLOW_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }
}

MVOID
AeFlowDefault::
update(AE_CORE_CTRL_CPU_EXP_INFO * input)
{
    updatePreEvSettingQueue(input->u4Eposuretime, input->u4AfeGain, input->u4IspGain, input->m_u4EffectiveIndex);
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]\n", __FUNCTION__);
}

MVOID
AeFlowDefault::
monitorAndReschedule(MBOOL bFaceAEAreaChage, MBOOL bTouchAEAreaChage, MUINT32 u4AvgYcur, MUINT32 u4AvgYStable, MUINT32 u4WOFDCnt, MBOOL *bAECalc, MBOOL *bAEApply)
{
    MBOOL bAEExit = MFALSE;
    MBOOL bAESkip = MFALSE;
    MINT32 i4framePerAECycle = 3;
    MUINT8 AEScheduleFrame;
    //MUINT8 NumCycleAESkipAfterExit = m_pAEParam->strAEParasetting.uCycleNumAESkipAfterExit; /* fix build warning */

    // Detect AE statistic change
    MBOOL bStatChange = AEStatisticChange(u4AvgYcur, u4AvgYStable, m_pAEParam->strAEParasetting.u2AEStatThrs);
    // Reset and Follow Schedule
    MBOOL bReschedule = (m_bAEMonitorStable && (bStatChange || (bFaceAEAreaChage && (u4WOFDCnt > m_pAEParam->FaceSmooth.u4FD_Lost_MaxCnt)) || bTouchAEAreaChage));

    //AE Exit condition
    if (m_bAEMonitorStable)
    {
        if ((bReschedule) && (m_i4WaitVDNum > (i4framePerAECycle -1)))
        {
            // (touchAE || Face || statchange) when stable
            m_u4AEScheduleCnt = 0;
        }
        else if ((bReschedule) && (m_i4WaitVDNum <= (i4framePerAECycle -1)))
        {
            // (touchAE || Face || statchange) when stable
            m_u4AEScheduleCnt = m_i4WaitVDNum;
        }
        else
        {
            // without (touchAE || Face || statchange) when stable
            bAEExit = MTRUE;
        }
    }
    //Skip after AEexit
    if (bAEExit == MTRUE)
    {
        (m_u4AEExitStableCnt < (MUINT32)i4framePerAECycle *(m_pAEParam->strAEParasetting.uCycleNumAESkipAfterExit))?(bAESkip = MFALSE):(bAESkip = MTRUE);
        m_u4AEExitStableCnt++;
    }
    else
    {
        m_u4AEExitStableCnt = 0;
    }
    // Skip 1st frame after per-frame --> non per-frame
    MBOOL PreEvSettingQueueChanged = (m_PreEvSettingQueue[0].u4Eposuretime != m_PreEvSettingQueue[1].u4Eposuretime) || (m_PreEvSettingQueue[0].u4AfeGain != m_PreEvSettingQueue[1].u4AfeGain) ||
    (m_PreEvSettingQueue[0].u4IspGain != m_PreEvSettingQueue[1].u4IspGain) || (m_PreEvSettingQueue[1].u4Eposuretime != m_PreEvSettingQueue[2].u4Eposuretime) ||
    (m_PreEvSettingQueue[1].u4AfeGain != m_PreEvSettingQueue[2].u4AfeGain) || (m_PreEvSettingQueue[1].u4IspGain != m_PreEvSettingQueue[2].u4IspGain);
    if (m_bPerFrameAEWorking == MTRUE && PreEvSettingQueueChanged)
    {
        bAESkip = MTRUE;
        AE_FLOW_LOG("[%s()] PerFrame --> non-PerFrame AE Skip\n", __FUNCTION__);
    }
    m_bPerFrameAEWorking = MFALSE;
    // Set AECalc & AEApply
    AEScheduleFrame = (m_u4AEScheduleCnt) % (i4framePerAECycle);
    if (bAESkip == MTRUE)
    {
        *bAECalc = MFALSE;
        *bAEApply = MFALSE;
        m_i4WaitVDNum = 0;
    }
    else if (AEScheduleFrame == 0)
    {
        *bAECalc = MTRUE;
        *bAEApply = MTRUE;
    }
    else
    {
        *bAECalc = MFALSE;
        *bAEApply = MTRUE;
    }

    AE_FLOW_LOG("[%s()] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvY(cur/pre):%d/%d Cnt:%d Frame:%d Skip:%d Exit:%d ReSchedule:%d Change(Stat/FaceArea/TouchArea):%d/%d/%d FaceWOCnt:%d\n",
        __FUNCTION__, *bAECalc, *bAEApply, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, u4AvgYcur, u4AvgYStable, m_u4AEScheduleCnt, AEScheduleFrame,
        bAESkip, bAEExit, bReschedule, bStatChange, bFaceAEAreaChage, bTouchAEAreaChage, u4WOFDCnt);

    // Update next AE Schedule Count
    m_u4AEScheduleCnt ++;
    return;
}

MVOID
AeFlowDefault::
updateAECycleInfo(MINT32 i4cyclenum, MINT32 i4Shutter, MINT32 i4AfeGain, MINT32 i4ISPgain)
{
    m_strAECycleInfo.m_i4CycleVDNum = i4cyclenum;
    m_strAECycleInfo.m_i4ShutterDelayFrames = i4Shutter;
    m_strAECycleInfo.m_i4GainDelayFrames = i4AfeGain;
    m_strAECycleInfo.m_i4IspGainDelayFrames = i4ISPgain;
    m_strAECycleInfo.m_i4FrameCnt = 0;
    m_strAECycleInfo.m_i4FrameTgCnt = 0;
    m_strAECycleInfo.m_u4HwMagicNum = 0;

    AE_FLOW_LOG("[%s()] m_i4CycleVDNum:%d m_i4ShutterDelayFrames:%d m_i4GainDelayFrames:%d m_i4IspGainDelayFrames:%d m_i4FrameCnt:%d\n", __FUNCTION__,
        m_strAECycleInfo.m_i4CycleVDNum,
        m_strAECycleInfo.m_i4ShutterDelayFrames,
        m_strAECycleInfo.m_i4GainDelayFrames,
        m_strAECycleInfo.m_i4IspGainDelayFrames,
        m_strAECycleInfo.m_i4FrameCnt);
}

MVOID
AeFlowDefault::
updatePreEvSettingQueue(MUINT32 newExpsuretime, MUINT32 newAfegain, MUINT32 newIspGain, MINT32 i4AEidxNext)
{
    m_BackupEvSetting = m_PreEvSettingQueue[2];
    for (int i = MAX_AE_PRE_EVSETTING - 2; i >= 0; i--)
    {
        m_PreEvSettingQueue[i + 1] = m_PreEvSettingQueue[i];
        m_u4PreAEidxQueue[i + 1] = m_u4PreAEidxQueue[i];
        m_u4PreTgIntAEidxQueue[i + 1] = m_u4PreTgIntAEidxQueue[i];
    }
    m_PreEvSettingQueue[0].u4Eposuretime = newExpsuretime;
    m_PreEvSettingQueue[0].u4AfeGain = newAfegain;
    m_PreEvSettingQueue[0].u4IspGain = newIspGain;
    m_PreEvSettingQueue[0].uFlag = 1;
    m_u4PreAEidxQueue[0] = i4AEidxNext;
    m_u4PreTgIntAEidxQueue[0] = i4AEidxNext;
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] m_PreEvSettingQueue[N-1]:%d/%d/%d m_PreEvSettingQueue[N-2]:%d/%d/%d m_PreEvSettingQueue[N-3]:%d/%d/%d FrameCnt(%d) FrameTgCnt(%d) HwMagicNum(%d)\n", __FUNCTION__,
        m_PreEvSettingQueue[0].u4Eposuretime, m_PreEvSettingQueue[0].u4AfeGain, m_PreEvSettingQueue[0].u4IspGain,
        m_PreEvSettingQueue[1].u4Eposuretime, m_PreEvSettingQueue[1].u4AfeGain, m_PreEvSettingQueue[1].u4IspGain,
        m_PreEvSettingQueue[2].u4Eposuretime, m_PreEvSettingQueue[2].u4AfeGain, m_PreEvSettingQueue[2].u4IspGain,
        m_strAECycleInfo.m_i4FrameCnt, m_strAECycleInfo.m_i4FrameTgCnt, m_strAECycleInfo.m_u4HwMagicNum);
}

MBOOL
AeFlowDefault::
AEStatisticChange(MUINT32 u4AvgYcur, MUINT32 u4AvgYpre, MUINT32 u4thres)
{
    MBOOL bStatChange = MFALSE;
    if ((u4AvgYcur < (u4AvgYpre - u4AvgYpre * u4thres / 100)) || (u4AvgYcur > (u4AvgYpre + u4AvgYpre * u4thres / 100)))
    {
        bStatChange = MTRUE;
    }
    return bStatChange;
}

AeFlowDefault::
~AeFlowDefault()
{
    AE_FLOW_LOG("[AeFlowDefault:Destruct] SensorDev:%d\n", (MUINT32)m_eSensorDev);
}

MVOID
AeFlowDefault::
setMVHDR3ExpoProcInfo(MVOID * /* pMVHDR3ExpoStatBuf */ , MUINT32 /* u4MVHDRRatio_x100 */ ) /* fix build warning */
{
}

