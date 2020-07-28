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
#define LOG_TAG "AeFlowVHDR"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG (1)
#endif

#include <stdlib.h>
#include "AeFlowVHDR.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_tuning_mgr.h>
#include <aaa_scheduler.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include "camera_custom_zvhdr.h"
#include "camera_custom_mvhdr3expo.h"

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

#define MY_INST_FLOW_VHDR INST_T<AeFlowVHDR>
static std::array<MY_INST_FLOW_VHDR, SENSOR_IDX_MAX> gMultiton;

IAeFlow *
AeFlowVHDR::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        // modify i4SensorIdx so it will be inside the range
        i4SensorIdx = (i4SensorIdx >= SENSOR_IDX_MAX) ? SENSOR_IDX_MAX-1 : 0;
    }

    MY_INST_FLOW_VHDR& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeFlowVHDR>(sensor);
    } );

    return rSingleton.instance.get();
}

AeFlowVHDR::
AeFlowVHDR(ESensorDev_T sensor)
: AeFlowDefault(sensor)
, m_u4AEISOSpeed(0)
, m_bRealISOSpeed(MFALSE)
{
    // Adb command input
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae_mgr.enable", value, "0");
    m_bAdbAEEnable = atoi(value);
    AE_FLOW_LOG("[AeFlowVHDR:Construct] SensorDev:%d", (MUINT32)m_eSensorDev);
}

MVOID
AeFlowVHDR::
schedulePvAE(MVOID * input, MVOID * output)
{
    AE_FLOW_SCHEDULE_INFO_T* pinputinfo = (AE_FLOW_SCHEDULE_INFO_T*) input;
    AE_FLOW_SCHEDULE_OUTPUT_T* poutputinfo = (AE_FLOW_SCHEDULE_OUTPUT_T*) output;

    //MBOOL bIsHDRflow = (pinputinfo->eAETargetMode != AE_MODE_NORMAL); /* fix build warning */
    //MINT32 i4ActiveItem = 0; /* fix build warning */
    MBOOL bCalculateAE = MFALSE;
    MBOOL bApplyAE = MFALSE;
    m_u4HwMagicNum = pinputinfo->u4HwMagicNum;
    m_i4WaitVDNum = pinputinfo->i4WaitVDNum;
    m_bPerframeAEFlag = pinputinfo->bPerframeAEFlag;
    m_bTouchAEAreaChange = pinputinfo->bTouchAEAreaChange;
    m_eAETargetMode = pinputinfo->eAETargetMode;
    m_bAAASchedule = pinputinfo->bAAASchedule;
    m_bAElimitor = pinputinfo->bAElimitor;

    if (m_bPerframeAEFlag)
    {
        switch (pinputinfo->eAEFlowType)
        {
            case E_AE_FLOW_CPU_DOPVAE:
                if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
                {
                    m_bPerframeAEFlag = MFALSE;
                    monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is non-Perframe mVHDR AE flow \n", __FUNCTION__);
                }
                else if (m_bTouchAEAreaChange)
                {
                    if (m_pAEParam->strAEParasetting.bEnableTouchSmooth)
                    {
                        bCalculateAE = MTRUE;
                        bApplyAE = MTRUE;
                        m_bPerFrameAEWorking = MTRUE;
                        AE_FLOW_LOG("[monitorAndReschedule:Perframe touch HDR AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvYcur:%d u4CwvYpre:%d add flag\n",
                            bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                    }
                    else
                    {
                        m_bPerframeAEFlag = MFALSE;
                        monitorAndReschedule(pinputinfo->bFaceAEAreaChange, m_bTouchAEAreaChange, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable, pinputinfo->u4WOFDcnt, &bCalculateAE, &bApplyAE);
                    }
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Touch HDR AE flow, bTouchPerframe:%d \n", __FUNCTION__, m_pAEParam->strAEParasetting.bEnableTouchSmooth);
                }
                else
                {
                    bCalculateAE = MTRUE;
                    bApplyAE = MTRUE;
                    m_bPerFrameAEWorking = MTRUE;
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] This is Perframe HDR AE flow \n", __FUNCTION__);
                    AE_FLOW_LOG("[monitorAndReschedule:Perframe HDR AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvYcur:%d u4CwvYpre:%d add flag\n",
                        bCalculateAE, bApplyAE, m_u4HwMagicNum, m_bAEMonitorStable, m_i4WaitVDNum, pinputinfo->i4CwvY, pinputinfo->u4CwvYStable);
                }
                break;
            case E_AE_FLOW_CPU_DOAFAE:
                if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET || (m_bAFTouchROISet && m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag && (pinputinfo->bAEReadyCapture == MFALSE)))
                {
                    bCalculateAE = MTRUE;
                    bApplyAE = MTRUE;
                    m_bPerFrameAEWorking = MTRUE;
                    AE_FLOW_LOG("[monitorAndReschedule:Perframe Touch HDR AE] Calc:%d Apply:%d Magic:%d AEMonitorStable:%d VdCnt:%d u4CwvYcur:%d u4CwvYpre:%d add flag\n",
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
    poutputinfo->pModifiedApply = &bApplyAE;
    poutputinfo->pModifiedCalc = &bCalculateAE;
    poutputinfo->pModifiedPerframeFlag = &m_bPerframeAEFlag;
    poutputinfo->pWaitVDNum = &m_i4WaitVDNum;
}

MVOID
AeFlowVHDR::
calculateAE(MVOID * input, MVOID * output)
{
    AE_FLOW_CALCULATE_INFO_T* pinputinfo = (AE_FLOW_CALCULATE_INFO_T*) input;
    strAEOutput* poutputinfo = (strAEOutput*) output;
    strAEInput rAEInput;
    memset(&rAEInput , 0, sizeof(strAEInput));
    m_u4AEISOSpeed = pinputinfo->u4AEISOSpeed;
    m_bRealISOSpeed = pinputinfo->bRealISOSpeed;
    // Debug adb command
    if (m_bAdbAEEnable)
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.ae_mgr.lock", value, "0");
        m_bAdbAELock = atoi(value);
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
                AE_FLOW_LOG("[%s()] AE_STATE_ONE_SHOT", __FUNCTION__);
            }
            else if (m_bTouchAEAreaChange)
            {
                if (m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag)
                    rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                else
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Enable per-frame AE for touch:%d\n", __FUNCTION__, m_pAEParam->strAEParasetting.bEnableTouchSmooth);
            }
            else if (m_bAElimitor)
            {
                rAEInput.eAeState = AE_STATE_PANORAMA_MODE;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] PANORAMA \n", __FUNCTION__);
            }
            else
            {
                if ((m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (!m_bPerframeAEFlag))
                {
                    rAEInput.eAeState = AE_STATE_SLOW_MOTION;
                    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] VHDR non-perframe flow\n", __FUNCTION__);
                }
                else
                {
                    rAEInput.eAeState = AE_STATE_NORMAL_PREVIEW;
                }
            }
            break;
        case E_AE_FLOW_CPU_DOAFAE:
            if (pinputinfo->bAElock || m_bAdbAELock)
            {
                rAEInput.eAeState = AE_STATE_AELOCK;
            }
            else if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                AE_FLOW_LOG("[doAFAE] MVHDR3 AE_STATE_TOUCH_PERFRAME :%d\n", m_bPerframeAEFlag);
            }
            else if (m_bAFTouchROISet && m_pAEParam->strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag && (!pinputinfo->bAEReadyCapture))
            {
                rAEInput.eAeState = AE_STATE_TOUCH_PERFRAME;
                AE_FLOW_LOG("[doAFAE] AE_STATE_TOUCH_PERFRAME :%d\n", m_bPerframeAEFlag);
            }
            else
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
                AE_FLOW_LOG("[doAFAE] AE_STATE_ONE_SHOT\n");
            }
            break;
        case E_AE_FLOW_CPU_DOPRECAPAE:
            if (pinputinfo->bAElock || m_bAdbAELock)
            {
                rAEInput.eAeState = AE_STATE_AELOCK;
            }
            else
            {
                rAEInput.eAeState = AE_STATE_ONE_SHOT;
            }
            break;
        case E_AE_FLOW_CPU_DOCAPAE:
            rAEInput.eAeState = AE_STATE_POST_CAPTURE;
            break;
        case E_AE_FLOW_CPU_DOAFASSIST:
            rAEInput.eAeState = AE_STATE_AFASSIST;
            break;
        case E_AE_FLOW_CPU_DOPVAETG:
            m_bTGReturn = MTRUE;
            break;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: FlowType = %d \n", __FUNCTION__, m_eSensorDev, pinputinfo->eAEFlowType);
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
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s():PassToAlgo] EvSetting/AEidx/Tgidx [N -1]:%d/%d/%d/%d/%d [N -2]:%d/%d/%d/%d/%d [N -3]:%d/%d/%d/%d/%d FrameCnt/Tgcnt/Magic(%d/%d/%d)\n", __FUNCTION__,
            rAEInput.PreEvSetting[0].u4Eposuretime, rAEInput.PreEvSetting[0].u4AfeGain, rAEInput.PreEvSetting[0].u4IspGain, rAEInput.u4PreAEidx[0], rAEInput.u4PreTgIntAEidx[0],
            rAEInput.PreEvSetting[1].u4Eposuretime, rAEInput.PreEvSetting[1].u4AfeGain, rAEInput.PreEvSetting[1].u4IspGain, rAEInput.u4PreAEidx[1], rAEInput.u4PreTgIntAEidx[1],
            rAEInput.PreEvSetting[2].u4Eposuretime, rAEInput.PreEvSetting[2].u4AfeGain, rAEInput.PreEvSetting[2].u4IspGain, rAEInput.u4PreAEidx[2], rAEInput.u4PreTgIntAEidx[2],
            rAEInput.CycleInfo.m_i4FrameCnt, rAEInput.CycleInfo.m_i4FrameTgCnt, rAEInput.CycleInfo.m_u4HwMagicNum);

    }

    // Handle Full -frame AE
    AaaTimer localTimer("handleAE", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
    AAA_TRACE_ALG(handleAE);
    m_pIAeAlgo->handleAE(&rAEInput, poutputinfo);
    AAA_TRACE_END_ALG;
    localTimer.End();

    // i / m / zVHDR params modification
    if (m_eAETargetMode == AE_MODE_IVHDR_TARGET)
    {
        updatePreviewParamsByiVHDR(poutputinfo);
    }
    else if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
    {
        updatePreviewParamsBymVHDR(poutputinfo);
    }
    else if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
    {
        updatePreviewParamsBymVHDR3Expo(poutputinfo);
    }
    else if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
    {
        updatePreviewParamsByzVHDR(poutputinfo, pinputinfo->u4VHDRratio);
    }

    //////// ADB COMMAND ////////////
    if (m_eAETargetMode != AE_MODE_NORMAL && m_bAdbAEPreviewUpdate)
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.ae_mgr.shutter_le", value, "0"); // shutter
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4LEExpo = atoi(value);
        property_get("vendor.debug.ae_mgr.shutter_se", value, "0");
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4SEExpo = atoi(value);
        property_get("vendor.debug.ae_mgr.shutter_me", value, "0");
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4MEExpo = atoi(value);
        property_get("vendor.debug.ae_mgr.shutter_vse", value, "0");
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4VSEExpo = atoi(value);

        property_get("vendor.debug.ae_mgr.sensorgain_le", value, "0"); // sensor gain
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain = atoi(value);
        property_get("vendor.debug.ae_mgr.sensorgain_se", value, "0");
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4SEAfeGain = atoi(value);
        property_get("vendor.debug.ae_mgr.sensorgain_me", value, "0");
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4MEAfeGain = atoi(value);
        property_get("vendor.debug.ae_mgr.sensorgain_vse", value, "0");
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4VSEAfeGain = atoi(value);

        property_get("vendor.debug.ae_mgr.ispgain", value, "0"); // isp gain
        poutputinfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain = poutputinfo->HdrEvSetting.rHdrEVInfo.i4SEIspGain = poutputinfo->HdrEvSetting.rHdrEVInfo.i4MEIspGain = poutputinfo->HdrEvSetting.rHdrEVInfo.i4VSEIspGain = atoi(value);
        AE_FLOW_LOG("[%s()] i4SensorDev:%d Apply LE:%d/%d/%d SE:%d/%d/%d ME:%d/%d/%d VSE:%d/%d/%d\n", __FUNCTION__, m_eSensorDev,
            poutputinfo->HdrEvSetting.rHdrEVInfo.i4LEExpo, poutputinfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain, poutputinfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain,
            poutputinfo->HdrEvSetting.rHdrEVInfo.i4SEExpo, poutputinfo->HdrEvSetting.rHdrEVInfo.i4SEAfeGain, poutputinfo->HdrEvSetting.rHdrEVInfo.i4SEIspGain,
            poutputinfo->HdrEvSetting.rHdrEVInfo.i4MEExpo, poutputinfo->HdrEvSetting.rHdrEVInfo.i4MEAfeGain, poutputinfo->HdrEvSetting.rHdrEVInfo.i4MEIspGain,
            poutputinfo->HdrEvSetting.rHdrEVInfo.i4VSEExpo, poutputinfo->HdrEvSetting.rHdrEVInfo.i4VSEAfeGain, poutputinfo->HdrEvSetting.rHdrEVInfo.i4VSEIspGain);
    }

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
}

AeFlowVHDR::
~AeFlowVHDR()
{
    AE_FLOW_LOG("[AeFlowVHDR:Destruct] SensorDev:%d\n", (MUINT32)m_eSensorDev);

}

MVOID
AeFlowVHDR::
updatePreviewParamsByiVHDR(strAEOutput *sAEInfo)
{
    IVHDRExpSettingInputParam_T strIVHDRInputSetting;
    IVHDRExpSettingOutputParam_T strIVHDROutputSetting;
    AE_EXP_GAIN_MODIFY_T rSensorInputData, rSensorOutputData;
    strIVHDRInputSetting.bIs60HZ = 0;
    strIVHDRInputSetting.u4ShutterTime = sAEInfo->EvSetting.u4Eposuretime;
    strIVHDRInputSetting.u4SensorGain = sAEInfo->EvSetting.u4AfeGain;
    strIVHDRInputSetting.u4IspGain = sAEInfo->EvSetting.u4IspGain;
    strIVHDRInputSetting.u41xGainISO = m_pAENVRAM->rDevicesInfo.u4MiniISOGain;
    strIVHDRInputSetting.u4SaturationGain = m_pAENVRAM->rDevicesInfo.u4MinGain;
    // Initialize strIVHDROutputSetting & rSensorOutputData to fix coverity check
    memset(&strIVHDROutputSetting, 0, sizeof(IVHDRExpSettingOutputParam_T));
    memset(&rSensorOutputData, 0, sizeof(AE_EXP_GAIN_MODIFY_T));

    if ((m_bRealISOSpeed == MTRUE) && (m_u4AEISOSpeed != 0))
    {
        //Eng mode
        strIVHDROutputSetting.u4SEExpTimeInUS = sAEInfo->EvSetting.u4Eposuretime;
        strIVHDROutputSetting.u4LEExpTimeInUS = sAEInfo->EvSetting.u4Eposuretime;
        strIVHDROutputSetting.u4SESensorGain = sAEInfo->EvSetting.u4AfeGain;
        strIVHDROutputSetting.u4LESensorGain = sAEInfo->EvSetting.u4AfeGain;
        strIVHDROutputSetting.u4SEISPGain = sAEInfo->EvSetting.u4IspGain;
        strIVHDROutputSetting.u4LEISPGain = sAEInfo->EvSetting.u4IspGain;
    }
    else
    {
        getIVHDRExpSetting(strIVHDRInputSetting, strIVHDROutputSetting);
        strIVHDROutputSetting.u4LE_SERatio_x100 = strIVHDROutputSetting.u4LEExpTimeInUS *100 / strIVHDROutputSetting.u4SEExpTimeInUS;
    }

    AE_FLOW_LOG("[%s] i4SensorDev:%d iVHDR 60Hz:%d Shutter:%d Gain:%d %d %d 1xGainISO:%d SatGain:%d Workaround:%d SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
        strIVHDRInputSetting.bIs60HZ, strIVHDRInputSetting.u4ShutterTime, strIVHDRInputSetting.u4SensorGain,
        sAEInfo->EvSetting.u4AfeGain, sAEInfo->EvSetting.u4IspGain, strIVHDRInputSetting.u41xGainISO, strIVHDRInputSetting.u4SaturationGain,
        strIVHDROutputSetting.bEnableWorkaround, strIVHDROutputSetting.u4SEExpTimeInUS, strIVHDROutputSetting.u4SESensorGain, strIVHDROutputSetting.u4SEISPGain,
        strIVHDROutputSetting.u4LEExpTimeInUS, strIVHDROutputSetting.u4LESensorGain, strIVHDROutputSetting.u4LEISPGain, strIVHDROutputSetting.u4LE_SERatio_x100);
    // LE time
    rSensorInputData.u4SensorExpTime = strIVHDROutputSetting.u4LEExpTimeInUS *101 / 100;
    rSensorInputData.u4SensorGain = strIVHDROutputSetting.u4LESensorGain *100 / 101;
    rSensorInputData.u4IspGain = strIVHDROutputSetting.u4LEISPGain;

    if (m_bRealISOSpeed == MTRUE)
    {
        //Eng mode
        rSensorOutputData.u4SensorExpTime = strIVHDROutputSetting.u4LEExpTimeInUS;
        rSensorOutputData.u4SensorGain = strIVHDROutputSetting.u4LESensorGain;
        rSensorOutputData.u4IspGain = strIVHDROutputSetting.u4LEISPGain;
    }
    else
    {
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData); // send to 3A to calculate the exposure time and gain
        }
        else
        {
            AE_FLOW_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        // check if gain is less than min saturation gain
        if (rSensorOutputData.u4IspGain < 1024)
        {
            rSensorOutputData.u4SensorExpTime = rSensorInputData.u4SensorExpTime;
            rSensorOutputData.u4SensorGain = rSensorInputData.u4SensorGain;
            rSensorOutputData.u4IspGain = rSensorInputData.u4IspGain;
            AE_FLOW_LOG("[updatePreviewParamsByiVHDR] Isp gain from switchSensorExposureGain should be larger than 1x , revert before setting \n");
        }
    }
    strIVHDROutputSetting.u4LEExpTimeInUS = rSensorOutputData.u4SensorExpTime;
    strIVHDROutputSetting.u4LESensorGain = rSensorOutputData.u4SensorGain;
    strIVHDROutputSetting.u4LEISPGain = rSensorOutputData.u4IspGain;

    // SE time
    strIVHDROutputSetting.u4SEExpTimeInUS = strIVHDROutputSetting.u4SEExpTimeInUS * strIVHDROutputSetting.u4SESensorGain / strIVHDROutputSetting.u4LESensorGain * strIVHDROutputSetting.u4SEISPGain / strIVHDROutputSetting.u4LEISPGain;
    if (strIVHDROutputSetting.u4SEExpTimeInUS > strIVHDROutputSetting.u4LEExpTimeInUS)
    {
        strIVHDROutputSetting.u4SEExpTimeInUS = strIVHDROutputSetting.u4LEExpTimeInUS;
    }
    strIVHDROutputSetting.u4SESensorGain = strIVHDROutputSetting.u4LESensorGain;
    strIVHDROutputSetting.u4SEISPGain = strIVHDROutputSetting.u4LEISPGain;

    strIVHDROutputSetting.u4LE_SERatio_x100 = 100 *strIVHDROutputSetting.u4LEExpTimeInUS / strIVHDROutputSetting.u4SEExpTimeInUS;

    if (strIVHDROutputSetting.u4LE_SERatio_x100 > 800)
    {
        AE_FLOW_LOG("[%s] i4SensorDev:%d Ratio:%d\n", __FUNCTION__, m_eSensorDev, strIVHDROutputSetting.u4LE_SERatio_x100);
        strIVHDROutputSetting.u4LE_SERatio_x100 = 800;
    }

    AE_FLOW_LOG("[%s] i4SensorDev:%d iVHDR workound:%d New SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
        strIVHDROutputSetting.bEnableWorkaround, strIVHDROutputSetting.u4SEExpTimeInUS, strIVHDROutputSetting.u4SESensorGain, strIVHDROutputSetting.u4SEISPGain,
        strIVHDROutputSetting.u4LEExpTimeInUS, strIVHDROutputSetting.u4LESensorGain, strIVHDROutputSetting.u4LEISPGain,
        strIVHDROutputSetting.u4LE_SERatio_x100);
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo = strIVHDROutputSetting.u4LEExpTimeInUS;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain = strIVHDROutputSetting.u4LESensorGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain = strIVHDROutputSetting.u4LEISPGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEExpo = strIVHDROutputSetting.u4SEExpTimeInUS;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEAfeGain = strIVHDROutputSetting.u4SESensorGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEIspGain = strIVHDROutputSetting.u4SEISPGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4HdrRatio = strIVHDROutputSetting.u4LE_SERatio_x100;

}

MVOID
AeFlowVHDR::
updatePreviewParamsBymVHDR(strAEOutput *sAEInfo)
{
    IVHDRExpSettingInputParam_T strIVHDRInputSetting;
    mVHDR_TRANSFER_Param_T strInputTransferParam;
    IVHDRExpSettingOutputParam_T strIVHDROutputSetting;
    AE_EXP_GAIN_MODIFY_T rSensorInputData, rSensorOutputData;

    memset(&strIVHDRInputSetting, 0, sizeof(IVHDRExpSettingInputParam_T));
    memset(&strInputTransferParam, 0, sizeof(mVHDR_TRANSFER_Param_T));
    memset(&strIVHDROutputSetting, 0, sizeof(IVHDRExpSettingOutputParam_T));
    memset(&rSensorOutputData, 0, sizeof(AE_EXP_GAIN_MODIFY_T));

    strIVHDRInputSetting.bIs60HZ = 0;
    strIVHDRInputSetting.u4ShutterTime = sAEInfo->EvSetting.u4Eposuretime;
    strIVHDRInputSetting.u4SensorGain = sAEInfo->EvSetting.u4AfeGain;
    strIVHDRInputSetting.u4IspGain = sAEInfo->EvSetting.u4IspGain;
    strIVHDRInputSetting.u41xGainISO = m_pAENVRAM->rDevicesInfo.u4MiniISOGain;
    strIVHDRInputSetting.u4SaturationGain = m_pAENVRAM->rDevicesInfo.u4MinGain;

    if ((m_bRealISOSpeed == MTRUE) && (m_u4AEISOSpeed != 0))
    {
        //Eng mode
        strIVHDROutputSetting.u4SEExpTimeInUS = sAEInfo->EvSetting.u4Eposuretime;
        strIVHDROutputSetting.u4LEExpTimeInUS = sAEInfo->EvSetting.u4Eposuretime;
        strIVHDROutputSetting.u4SESensorGain = sAEInfo->EvSetting.u4AfeGain;
        strIVHDROutputSetting.u4LESensorGain = sAEInfo->EvSetting.u4AfeGain;
        strIVHDROutputSetting.u4SEISPGain = sAEInfo->EvSetting.u4IspGain;
        strIVHDROutputSetting.u4LEISPGain = sAEInfo->EvSetting.u4IspGain;
    }
    else
    {
        getIVHDRExpSetting(strIVHDRInputSetting, strIVHDROutputSetting);
        strIVHDROutputSetting.u4LE_SERatio_x100 = strIVHDROutputSetting.u4LEExpTimeInUS *100 / strIVHDROutputSetting.u4SEExpTimeInUS;
    }

    AE_FLOW_LOG("[%s] i4SensorDev:%d iVHDR 60Hz:%d Shutter:%d Gain:%d %d %d 1xGainISO:%d SatGain:%d Workaround:%d SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
        strIVHDRInputSetting.bIs60HZ, strIVHDRInputSetting.u4ShutterTime, strIVHDRInputSetting.u4SensorGain,
        sAEInfo->EvSetting.u4AfeGain, sAEInfo->EvSetting.u4IspGain, strIVHDRInputSetting.u41xGainISO, strIVHDRInputSetting.u4SaturationGain,
        strIVHDROutputSetting.bEnableWorkaround, strIVHDROutputSetting.u4SEExpTimeInUS, strIVHDROutputSetting.u4SESensorGain, strIVHDROutputSetting.u4SEISPGain,
        strIVHDROutputSetting.u4LEExpTimeInUS, strIVHDROutputSetting.u4LESensorGain, strIVHDROutputSetting.u4LEISPGain, strIVHDROutputSetting.u4LE_SERatio_x100);

    //strInputTransferParam.bIs60HZ = bIs60Hz;
    strInputTransferParam.u2SelectMode = m_u2VCModeSelect;
    strInputTransferParam.i4LV = sAEInfo->Bv + 50;
    getmVHDRExpSetting(strInputTransferParam, strIVHDROutputSetting);
    strIVHDROutputSetting.u4LE_SERatio_x100 = strIVHDROutputSetting.u4LEExpTimeInUS *100 / strIVHDROutputSetting.u4SEExpTimeInUS;
    AE_FLOW_LOG("[%s] i4SensorDev:%d mVHDR SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
        strIVHDROutputSetting.u4SEExpTimeInUS,
        strIVHDROutputSetting.u4SESensorGain,
        strIVHDROutputSetting.u4SEISPGain,
        strIVHDROutputSetting.u4LEExpTimeInUS,
        strIVHDROutputSetting.u4LESensorGain,
        strIVHDROutputSetting.u4LEISPGain,
        strIVHDROutputSetting.u4LE_SERatio_x100);
    // LE time
    rSensorInputData.u4SensorExpTime = strIVHDROutputSetting.u4LEExpTimeInUS *101 / 100;
    rSensorInputData.u4SensorGain = strIVHDROutputSetting.u4LESensorGain *100 / 101;
    rSensorInputData.u4IspGain = strIVHDROutputSetting.u4LEISPGain;

    if (m_bRealISOSpeed == MTRUE)
    {
        //Eng mode
        rSensorOutputData.u4SensorExpTime = strIVHDROutputSetting.u4LEExpTimeInUS;
        rSensorOutputData.u4SensorGain = strIVHDROutputSetting.u4LESensorGain;
        rSensorOutputData.u4IspGain = strIVHDROutputSetting.u4LEISPGain;
    }
    else
    {
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData); // send to 3A to calculate the exposure time and gain
        }
        else
        {
            AE_FLOW_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        // check if gain is less than min saturation gain
        if (rSensorOutputData.u4IspGain < 1024)
        {
            rSensorOutputData.u4SensorExpTime = rSensorInputData.u4SensorExpTime;
            rSensorOutputData.u4SensorGain = rSensorInputData.u4SensorGain;
            rSensorOutputData.u4IspGain = rSensorInputData.u4IspGain;
            AE_FLOW_LOG("[updatePreviewParamsByiVHDR] Isp gain from switchSensorExposureGain should be larger than 1x , revert before setting \n");
        }
    }
    strIVHDROutputSetting.u4LEExpTimeInUS = rSensorOutputData.u4SensorExpTime;
    strIVHDROutputSetting.u4LESensorGain = rSensorOutputData.u4SensorGain;
    strIVHDROutputSetting.u4LEISPGain = rSensorOutputData.u4IspGain;

    // SE time
    strIVHDROutputSetting.u4SEExpTimeInUS = strIVHDROutputSetting.u4SEExpTimeInUS *strIVHDROutputSetting.u4SESensorGain / strIVHDROutputSetting.u4LESensorGain * strIVHDROutputSetting.u4SEISPGain / strIVHDROutputSetting.u4LEISPGain;
    if (strIVHDROutputSetting.u4SEExpTimeInUS > strIVHDROutputSetting.u4LEExpTimeInUS)
    {
        strIVHDROutputSetting.u4SEExpTimeInUS = strIVHDROutputSetting.u4LEExpTimeInUS;
    }
    strIVHDROutputSetting.u4SESensorGain = strIVHDROutputSetting.u4LESensorGain;
    strIVHDROutputSetting.u4SEISPGain = strIVHDROutputSetting.u4LEISPGain;

    strIVHDROutputSetting.u4LE_SERatio_x100 = 100 *strIVHDROutputSetting.u4LEExpTimeInUS / strIVHDROutputSetting.u4SEExpTimeInUS;

    if (strIVHDROutputSetting.u4LE_SERatio_x100 > 800)
    {
        AE_FLOW_LOG("[%s] i4SensorDev:%d Ratio:%d\n", __FUNCTION__, m_eSensorDev, strIVHDROutputSetting.u4LE_SERatio_x100);
        strIVHDROutputSetting.u4LE_SERatio_x100 = 800;
    }

    AE_FLOW_LOG("[%s] i4SensorDev:%d iVHDR workound:%d New SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
        strIVHDROutputSetting.bEnableWorkaround, strIVHDROutputSetting.u4SEExpTimeInUS, strIVHDROutputSetting.u4SESensorGain, strIVHDROutputSetting.u4SEISPGain,
        strIVHDROutputSetting.u4LEExpTimeInUS, strIVHDROutputSetting.u4LESensorGain, strIVHDROutputSetting.u4LEISPGain,
        strIVHDROutputSetting.u4LE_SERatio_x100);
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo = strIVHDROutputSetting.u4LEExpTimeInUS;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain = strIVHDROutputSetting.u4LESensorGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain = strIVHDROutputSetting.u4LEISPGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEExpo = strIVHDROutputSetting.u4SEExpTimeInUS;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEAfeGain = strIVHDROutputSetting.u4SESensorGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEIspGain = strIVHDROutputSetting.u4SEISPGain;
    sAEInfo->HdrEvSetting.rHdrEVInfo.i4HdrRatio = strIVHDROutputSetting.u4LE_SERatio_x100;
}

MVOID
AeFlowVHDR::
updatePreviewParamsBymVHDR3Expo(strAEOutput * /* sAEInfo */ ) /* fix build warning */
{
}

MRESULT
AeFlowVHDR::
updatePreviewParamsByzVHDR(strAEOutput *sAEInfo, MUINT32 u4VHDRratio)
{
    IVHDRExpSettingOutputParam_T strIVHDROutputSetting;

    if (u4VHDRratio >= 1000 && (u4VHDRratio < 9999))
    {
        // Ratio range: 1000~9998: shutter only HDR
        strIVHDROutputSetting.u4SEExpTimeInUS = (sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo *1000 + (u4VHDRratio >> 1)) / u4VHDRratio;
        strIVHDROutputSetting.u4SESensorGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain;
        strIVHDROutputSetting.u4SEISPGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain;
        strIVHDROutputSetting.u4LE_SERatio_x100 = (u4VHDRratio + 5) / 10;
    }
    else
    {
        //<1000 or >= 9999
        if (m_pIAeAlgo != NULL)
        {
            // Ratio range: 100~999: set short exposure by setting shutter & gain
            strEvSetting rEvSetting;
            strEvHdrSetting HdrEvSetting;
            rEvSetting.u4Eposuretime = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo;
            rEvSetting.u4AfeGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain;
            rEvSetting.u4IspGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain;
            rEvSetting.uIris = 0;
            rEvSetting.uSensorMode = 0;
            rEvSetting.uFlag = 0;
            if ((u4VHDRratio < 100) || (u4VHDRratio > 9999))
            {
                // ~(100~999 or 9999)
                AE_FLOW_LOG("[%s()] HDR ratio is out of range = %d ", __FUNCTION__, u4VHDRratio);
                return E_AE_PARAMETER_ERROR;
            }
            else if (u4VHDRratio != 9999)
            {
                //100~999
                //If EM ratio = 9999 => auto ratio; else => over -write with specific ratio
                m_pIAeAlgo->setHdrRatio(u4VHDRratio);
            }   //else: 9999

            m_pIAeAlgo->calRatio2AEInfoIf(rEvSetting);
            m_pIAeAlgo->updateHdrInfo(HdrEvSetting);

            strIVHDROutputSetting.u4LEExpTimeInUS = HdrEvSetting.rHdrEVInfo.i4LEExpo;
            strIVHDROutputSetting.u4LESensorGain = HdrEvSetting.rHdrEVInfo.i4LEAfeGain;
            strIVHDROutputSetting.u4LEISPGain = HdrEvSetting.rHdrEVInfo.i4LEIspGain;
            strIVHDROutputSetting.u4SEExpTimeInUS = HdrEvSetting.rHdrEVInfo.i4SEExpo;
            strIVHDROutputSetting.u4SESensorGain = HdrEvSetting.rHdrEVInfo.i4SEAfeGain;
            strIVHDROutputSetting.u4SEISPGain = HdrEvSetting.rHdrEVInfo.i4SEIspGain;
            strIVHDROutputSetting.u4LE_SERatio_x100 = HdrEvSetting.rHdrEVInfo.i4HdrRatio;

            AE_FLOW_LOG("[%s()] Engineer Mode HDR AE(In Ratio:%d) = LE(%d, %d, %d), SE(%d, %d, %d), R(%d) ", __FUNCTION__,
                u4VHDRratio,
                sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo, sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain, sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain,
                strIVHDROutputSetting.u4SEExpTimeInUS, strIVHDROutputSetting.u4SESensorGain, strIVHDROutputSetting.u4SEISPGain,
                strIVHDROutputSetting.u4LE_SERatio_x100);

            AE_FLOW_LOG("[%s] i4SensorDev:%d New SE:%d %d %d LE:%d %d %d Ratio:%d\n", __FUNCTION__, m_eSensorDev,
                strIVHDROutputSetting.u4SEExpTimeInUS, strIVHDROutputSetting.u4SESensorGain, strIVHDROutputSetting.u4SEISPGain,
                strIVHDROutputSetting.u4LEExpTimeInUS, strIVHDROutputSetting.u4LESensorGain, strIVHDROutputSetting.u4LEISPGain,
                strIVHDROutputSetting.u4LE_SERatio_x100);
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo = strIVHDROutputSetting.u4LEExpTimeInUS;
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain = strIVHDROutputSetting.u4LESensorGain;
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain = strIVHDROutputSetting.u4LEISPGain;
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEExpo = strIVHDROutputSetting.u4SEExpTimeInUS;
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEAfeGain = strIVHDROutputSetting.u4SESensorGain;
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEIspGain = strIVHDROutputSetting.u4SEISPGain;
            sAEInfo->HdrEvSetting.rHdrEVInfo.i4HdrRatio = strIVHDROutputSetting.u4LE_SERatio_x100;
        }
        else
        {
            AE_FLOW_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

MVOID
AeFlowVHDR::
setMVHDR3ExpoProcInfo(MVOID * pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100)
{
    AE_3EXPO_PROC_INFO_T a_rMVHDR3ExpoProcInfo;
    a_rMVHDR3ExpoProcInfo.u4sizewidth = MVHDR3EXPO_WIDTH;
    a_rMVHDR3ExpoProcInfo.u4sizeheight = MVHDR3EXPO_HEIGHT;
    if (m_bPerframeAEFlag)
        a_rMVHDR3ExpoProcInfo.u4ispgain = m_PreEvSettingQueue[2].u4IspGain;
    else
    a_rMVHDR3ExpoProcInfo.u4ispgain = m_PreEvSettingQueue[0].u4IspGain;
    a_rMVHDR3ExpoProcInfo.u4HDRratio = u4MVHDRRatio_x100;
    if (m_pIAeAlgo != NULL)
        m_pIAeAlgo->setmVHDRAESatisticBufferAddr(pMVHDR3ExpoStatBuf, &a_rMVHDR3ExpoProcInfo);
    else
    AE_FLOW_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
}
