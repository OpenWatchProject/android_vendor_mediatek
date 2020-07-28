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
#define ENABLE_MY_LOG (1)
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
// CCU flow implement //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MRESULT AeMgr::prepareCCUPvParam(AE_FLOW_CONTROL_INFO_T * pinput)
{
    Mutex::Autolock lock(m_Lock);
    AE_LOG_IF(m_3ALogEnable, "[%s()] + add protection \n", __FUNCTION__);
    if (m_SensorQueueCtrl.uOutputIndex >= AE_SENSOR_MAX_QUEUE)
    {
        m_SensorQueueCtrl.uOutputIndex = 0;
    }

    AE_LOG_IF(m_3ALogEnable, "[%s()] CCUDBG index=%d\n", __FUNCTION__, m_SensorQueueCtrl.uOutputIndex);

    pinput->pPreviewTableF = &m_PreviewTableF;
    AE_LOG_IF(m_3ALogEnable, "[%s()] AEIDBG CPU prepare curTable m_u4IndexMax: %x\n", __FUNCTION__, m_u4IndexFMax);
    /*
    AE_LOG_IF(m_3ALogEnable,"AEIDBG CPU prepare curTable 0/1/2/100/200/300/400/MAX-1/MAX: %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
    pinput->pPreviewTableF->sPlineTable[0].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[1].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[2].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[100].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[200].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[300].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[400].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[m_u4IndexFMax-1].u4Eposuretime,
    pinput->pPreviewTableF->sPlineTable[m_u4IndexFMax].u4Eposuretime);*/
    pinput->pPreviewTableCurrent = m_pPreviewTableCurrent;
    pinput->eAEEVcomp = m_eAEEVcompPreset;
    pinput->bZoomChange = m_eZoomWinInfo.bZoomChange;
    pinput->bskipCCUAlgo = ((m_eAEMode == LIB3A_AE_MODE_OFF) || m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_ADB_LOCK) || m_bForceSkipCCU);
    AE_LOG_IF(m_3ALogEnable, "[%s()] dbgevc %x\n", __FUNCTION__, pinput->bskipCCUAlgo);
    pinput->bManualAE = (m_eAEMode == LIB3A_AE_MODE_OFF);
    pinput->u4IndexFMax = m_u4IndexFMax;
    pinput->u4IndexFMin = m_u4IndexFMin;
    if (m_i4AEMaxFps > 0)
    {
        pinput->i4AEMaxFps = m_i4AEMaxFps;
    }
    else
    {
        pinput->i4AEMaxFps = LIB3A_AE_FRAMERATE_MODE_30FPS;
    }
    pinput->u4HwMagicNum = m_u4HwMagicNum;
    pinput->pISPNvramOBC_Table = &(m_rISP5NvramOBC_Table[0]); /*&(m_rISPNvramOBC_Table[0]); */
    pinput->pISP5NvramOBC_Table = &(m_rISP5NvramOBC_Table[0]);
    AE_LOG_IF(m_3ALogEnable, "[%s()] ISP5NvramOBC %d\n", __FUNCTION__, pinput->pISP5NvramOBC_Table[0].offst0.val);
    pinput->OBCTableidx = m_i4OBCTableidx;
    pinput->u4CurrentIndex = m_u4Index | mbIsPlineChange << 16;
    pinput->u4CurrentIndexF = m_u4IndexF;

    if (m_eAEMode == LIB3A_AE_MODE_OFF)
    {
        if (m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime > 0 &&
            m_SensorQueueCtrl.uInputIndex != m_SensorQueueCtrl.uOutputIndex)
        {
            pinput->u4ManualExp = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime;
            m_u4CCUManShutter = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime;
            pinput->u4ManualISO = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity;
            m_u4CCUManISO = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity;
        }
        else
        {
            pinput->u4ManualExp = m_u4CCUManShutter;
            pinput->u4ManualISO = m_u4CCUManISO;
        }
        if (m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime > 600000 &&
            m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_LONG_EXP_CONTROL))
        {
            AE_LOG("[%s()] longexp_work m_bIsLongExpControlbyCCU = MFALSE\n", __FUNCTION__);
            m_bIsLongExpControlbyCCU = MFALSE;
        }
    }
    else
    {
        pinput->u4ManualExp = 0;
        pinput->u4ManualISO = 0;
    }
    pinput->u4AENVRAMIdx = m_u4AENVRAMIdx;
    pinput->u4FinerEVBase = m_u4FinerEVIdxBase;
    pinput->pAEOnchNVRAM = (CCU_AE_NVRAM_T *)m_rAEInitInput.rAENVRAM;
    pinput->u4AEMeteringMode = m_eAEMeterMode;
    pinput->u4Prvflare = m_u4Prvflare;
    pinput->u4FDProb = m_u4FDProbForCCU;
    pinput->u4FaceFoundCnt = m_u4FaceFoundCntForCCU;
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET) || (m_u4FaceFailCntForCCU >= 0))
    {
        pinput->u4FaceFailCnt = m_u4FaceFailCntForCCU;
        AE_LOG_IF(m_3ALogEnable, "[%s()] backup FaceFailCnt", __FUNCTION__);
    }
    else
    {
        pinput->u4FaceFailCnt = 1024;
        AE_LOG_IF(m_3ALogEnable, "[%s()] reset FaceFailCnt", __FUNCTION__);
    }
    pinput->u4FaceFoundCnt = m_u4FaceFoundCntForCCU;
    pinput->bUpdateAELockIdx = m_bUpdateAELockIdx;
    m_bUpdateAELockIdx = MFALSE; //reset

    eAESTATE AeState;
    if (m_bAELock)
    {
        AeState = AE_STATE_AELOCK;
        AE_LOG_IF(m_3ALogEnable, "[%s()] AE lock state\n", __FUNCTION__);
    }
    else if (m_bAEOneShotControl)
    {
        AeState = AE_STATE_ONE_SHOT;
        AE_LOG_IF(m_3ALogEnable, "[%s()] One shot state\n", __FUNCTION__);
    }
    else if (m_bTouchAEAreaChage)
    {
        if (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableTouchSmooth && m_bPerframeAEFlag /*&& (!m_bAAASchedule) */ )//For stereo case
        {
            AeState = AE_STATE_TOUCH_PERFRAME;
            AE_LOG_IF(m_3ALogEnable, "[%s()] Perframe touch AE state\n", __FUNCTION__);
        }
        else
        {
            AeState = AE_STATE_ONE_SHOT;
            AE_LOG_IF(m_3ALogEnable, "[%s()] Non-Perframe touch AE state (one shot)\n", __FUNCTION__);
        }
        AE_LOG_IF(m_3ALogEnable, "[%s()] Enable per-frame AE for touch:%d\n", __FUNCTION__, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableTouchSmooth);
    }
    /*else if (m_bAAASchedule)
    {   //For stereo case
        if (m_bPerframeAEFlag)
        {
            AeState = AE_STATE_NORMAL_PREVIEW;
        }
        else
        {
            AeState =AE_STATE_SLOW_MOTION;
            AE_LOG_IF(m_3ALogEnable, "[%s()] Stereo flow\n", __FUNCTION__);
        }
    }
    else if (m_bEnSWBuffMode)
    {
        //rAEInput.eAeState =AE_STATE_SLOW_MOTION;
        AeState = AE_STATE_NORMAL_PREVIEW;
        AE_LOG_IF(m_3ALogEnable, "[%s()] Slow motion flow\n", __FUNCTION__);
    }
    else if (m_bAElimitor)
    {
        AeState = AE_STATE_PANORAMA_MODE;
        AE_LOG_IF(m_3ALogEnable, "[%s()] PANORAMA \n", __FUNCTION__);
    } */
    else
    {
        AeState = AE_STATE_NORMAL_PREVIEW;
    }
    pinput->mFrameData.AeState = (CCU_eAESTATE)AeState; //2222222444444, normalae->lockae->normalae???? !!!!!!!!!!!!!!!!!!!!
    AE_LOG_IF(m_3ALogEnable, "[%s()] AeState:%d", __FUNCTION__, pinput->mFrameData.AeState);
    pinput->mFrameData.i4DeltaSensitivityIdx = m_i4DeltaSensitivityIdx;
    pinput->mFrameData.i4DeltaBVIdx = m_i4DeltaBVIdx;
    // calculate manual afe & isp gain
    MUINT32 u4AfeGain = 1024;
    MUINT32 u4ISPGain = 1024;

    if (m_p3ANVRAM != NULL)
    {
        MUINT32 u4ISOValue = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        if (pinput->u4ManualISO > 0)
            u4ISOValue = pinput->u4ManualISO;
        MUINT32 u4MinIsoGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 u4MaxGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain;
        u4AfeGain = (u4ISOValue * 1024) / u4MinIsoGain;
        if (u4AfeGain > u4MaxGain)
        {
            u4ISPGain = 1024 * u4AfeGain / u4MaxGain;
            u4AfeGain = u4MaxGain;
        }
    }
    else
    {
        MY_LOG("[%s] NVRAM is NULL\n", __FUNCTION__);
        //m_u4UpdateGainValue = 1024;
    }
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae.manual.exposure", value, "0");
    MUINT32 ManualExp = atoi(value);

    if (m_u4CCUManualEnable)
    {
        pinput->u4ManualExp = m_u4CCUManualShutter;
        pinput->u4ManualAfeGain = m_u4CCUManualAfeGain;
        pinput->u4ManualISPGain = m_u4CCUManualIspGain;
        pinput->bManualAE = MTRUE;
    }
    else if (ManualExp > 0)
    {
        pinput->u4ManualExp = ManualExp;
        property_get("vendor.debug.ae.manual.afe", value, "1024");
        pinput->u4ManualAfeGain = atoi(value);
        property_get("vendor.debug.ae.manual.isp", value, "1024");
        pinput->u4ManualISPGain = atoi(value);
        pinput->bManualAE = MTRUE;
    }
    else
    {
        pinput->u4ManualAfeGain = u4AfeGain;
        pinput->u4ManualISPGain = u4ISPGain;
    }
    AE_LOG_IF(m_3ALogEnable, "[%s()]ManDBG.Exp = 0x%x, ManDBG.Gain = 0x%x, 0x%x, ManDBG.ISO = 0x%x\n", __FUNCTION__, pinput->u4ManualExp, pinput->u4ManualAfeGain, pinput->u4ManualISPGain , pinput->u4ManualISO);
    //Prepare Frame data here
    // pinput->mFrameData.m_AEWinBlock.u4XLow = m_eZoomWinInfo.u4XOffset;
    // pinput->mFrameData.m_AEWinBlock.u4XHi = m_eZoomWinInfo.u4XOffset + m_eZoomWinInfo.u4XWidth;
    // pinput->mFrameData.m_AEWinBlock.u4YLow = m_eZoomWinInfo.u4YOffset;
    // pinput->mFrameData.m_AEWinBlock.u4YHi = m_eZoomWinInfo.u4YOffset + m_eZoomWinInfo.u4YHeight;
    //pinput->mFrameData.m_AEWinBlock.u4Weight = m_eZoomWinInfo.

    memset(&pinput->mFrameData.m_eAEFDArea, 0, sizeof(CCU_AEMeteringArea_T));
    pinput->mFrameData.m_eAEFDArea.u4Count = 0;

    if (m_eAEFDArea.u4Count != 0)
    {
        pinput->mFrameData.m_eAEFDArea.u4Count = m_eAEFDArea.u4Count;

        for (MUINT8 i = 0; i < m_eAEFDArea.u4Count; i++) //copy FD info
        {
            /**
             * Structure need to be fixed, low->left...
             */
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Left   = m_eAEFDArea.rAreas[i].i4Left;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Right  = m_eAEFDArea.rAreas[i].i4Right;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Top    = m_eAEFDArea.rAreas[i].i4Top;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Bottom = m_eAEFDArea.rAreas[i].i4Bottom;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Weight = (MUINT16)m_eAEFDArea.rAreas[i].i4Weight;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Id     = (MUINT16)m_eAEFDArea.rAreas[i].i4Id;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Type   = (MUINT8)m_eAEFDArea.rAreas[i].i4Type; // 0:GFD, 1:LFD, 2:OT //change type
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4ROP    = (MUINT8)m_eAEFDArea.rAreas[i].i4ROP;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Motion[0] = (MINT8)m_eAEFDArea.rAreas[i].i4Motion[0];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Motion[1] = (MINT8)m_eAEFDArea.rAreas[i].i4Motion[1];

            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[0][0] = m_eAEFDArea.rAreas[i].i4Landmark[0][0];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[1][0] = m_eAEFDArea.rAreas[i].i4Landmark[1][0];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[2][0] = m_eAEFDArea.rAreas[i].i4Landmark[2][0];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[0][1] = m_eAEFDArea.rAreas[i].i4Landmark[0][1];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[1][1] = m_eAEFDArea.rAreas[i].i4Landmark[1][1];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[2][1] = m_eAEFDArea.rAreas[i].i4Landmark[2][1];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[0][2] = m_eAEFDArea.rAreas[i].i4Landmark[0][2];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[1][2] = m_eAEFDArea.rAreas[i].i4Landmark[1][2];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[2][2] = m_eAEFDArea.rAreas[i].i4Landmark[2][2];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[0][3] = m_eAEFDArea.rAreas[i].i4Landmark[0][3];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[1][3] = m_eAEFDArea.rAreas[i].i4Landmark[1][3];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4Landmark[2][3] = m_eAEFDArea.rAreas[i].i4Landmark[2][3];
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4LandmarkCV = m_eAEFDArea.rAreas[i].i4LandmarkCV;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4LandMarkRip = m_eAEFDArea.rAreas[i].i4LandMarkRip;
            pinput->mFrameData.m_eAEFDArea.rAreas[i].i4LandMarkRop = m_eAEFDArea.rAreas[i].i4LandMarkRop;
            //pinput->mFrameData.m_AEFDWinBlock[i].bLandMarkBullsEye = ;
        }
    }

    for (MINT8 j = 0; j < 3; j++)
    {
        pinput->mFrameData.m_eAEFDArea.GyroRot[j] = m_i4GyroInfo[j];
        pinput->mFrameData.m_eAEFDArea.GyroAcc[j] = m_i4AcceInfo[j];
        AE_LOG_IF(m_3ALogEnable, "[%s()] m_eAEFDArea.GyroRot[%d] =%d", __FUNCTION__, j, pinput->mFrameData.m_eAEFDArea.GyroRot[j]);
    }
    memset(&pinput->mFrameData.m_AETOUCHWinBlock, 0, CCU_MAX_AE_METER_AREAS * sizeof(CCU_AE_TOUCH_BLOCK_WINDOW_T));
    pinput->mFrameData.m_u4MeteringCnt = 0;

    if (m_eAEMeterArea.u4Count != 0)
    {
        pinput->mFrameData.m_u4MeteringCnt = m_eAEMeterArea.u4Count;

        for (MUINT8 i = 0; i < m_eAEMeterArea.u4Count; i++) //copy touch info
        {
            pinput->mFrameData.m_AETOUCHWinBlock[i].u4XLow = m_eAEMeterArea.rAreas[i].i4Left;
            pinput->mFrameData.m_AETOUCHWinBlock[i].u4XHi = m_eAEMeterArea.rAreas[i].i4Right;
            pinput->mFrameData.m_AETOUCHWinBlock[i].u4YLow = m_eAEMeterArea.rAreas[i].i4Top;
            pinput->mFrameData.m_AETOUCHWinBlock[i].u4YHi = m_eAEMeterArea.rAreas[i].i4Bottom;
            pinput->mFrameData.m_AETOUCHWinBlock[i].u4Weight = m_eAEMeterArea.rAreas[i].i4Weight;
        }

    }
    pinput->mFrameData.u4IsCalAE = (!pinput->bskipCCUAlgo);
    AE_LOG_IF(m_3ALogEnable, "[%s()] u4IsCalAE = %d ROP=%d", __FUNCTION__, pinput->mFrameData.u4IsCalAE, pinput->mFrameData.m_eAEFDArea.rAreas[0].i4ROP);
    pinput->mFrameData.pAAOProcInfo.LSC_Gain_Grid_G[0] = m_eZoomWinInfo.u4XOffset;
    pinput->mFrameData.pAAOProcInfo.LSC_Gain_Grid_G[1] = m_eZoomWinInfo.u4XWidth;
    pinput->mFrameData.pAAOProcInfo.LSC_Gain_Grid_G[2] = m_eZoomWinInfo.u4YOffset;
    pinput->mFrameData.pAAOProcInfo.LSC_Gain_Grid_G[3] = m_eZoomWinInfo.u4YHeight;

    AE_LOG_IF(m_3ALogEnable, "[%s()] TouchArea i4Left:%x i4Right:%x i4Top:%x i4Bottom:%x i4Weight:%x\n", __FUNCTION__,
        m_eAEMeterArea.rAreas[0].i4Left, m_eAEMeterArea.rAreas[0].i4Right,
        m_eAEMeterArea.rAreas[0].i4Top, m_eAEMeterArea.rAreas[0].i4Bottom, m_eAEMeterArea.rAreas[0].i4Weight);
    /**
     *  For flare dbg
     */
    int flare_disable = 0;
    //char value[128] = {0};
    pinput->bForceResetCCUStable = m_bForceResetCCUStable;
    /**
     *  For DualCam Calib
     */

    AE_ADB_CFG ae_adb_cfg;

    property_get("vendor.debug.ae.manual.target_en", value, "0");
    ae_adb_cfg.bEableManualTarget = atoi(value);
    property_get("vendor.debug.ae.manual.target", value, "47");
    ae_adb_cfg.u4ManualTarget = atoi(value);
    property_get("vendor.debug.ae.calib_en", value, "0");
    ae_adb_cfg.bEableCalib = atoi(value);

    pinput->mFrameData.ae_adb_cfg = ae_adb_cfg;
    pinput->mFrameData.flare_disable = flare_disable;
    AE_LOG_IF(m_3ALogEnable, "[%s()] FaceArea i4Left:%x i4Right:%x i4Top:%x i4Bottom:%x i4Weight:%x", __FUNCTION__,
        pinput->mFrameData.m_eAEFDArea.rAreas[0].i4Left, pinput->mFrameData.m_eAEFDArea.rAreas[0].i4Right,
        pinput->mFrameData.m_eAEFDArea.rAreas[0].i4Top, pinput->mFrameData.m_eAEFDArea.rAreas[0].i4Bottom,
        pinput->mFrameData.m_eAEFDArea.rAreas[0].i4Weight);

    AE_LOG_IF(m_3ALogEnable, "[%s()] FaceLandmark %x %x %x %x %x %x %x %x %x %x %x %x\n", __FUNCTION__,
        m_eAEFDArea.rAreas[0].i4Landmark[0][0], m_eAEFDArea.rAreas[0].i4Landmark[1][0],
        m_eAEFDArea.rAreas[0].i4Landmark[2][0], m_eAEFDArea.rAreas[0].i4Landmark[0][1],
        m_eAEFDArea.rAreas[0].i4Landmark[1][1], m_eAEFDArea.rAreas[0].i4Landmark[2][1],
        m_eAEFDArea.rAreas[0].i4Landmark[0][2], m_eAEFDArea.rAreas[0].i4Landmark[1][2],
        m_eAEFDArea.rAreas[0].i4Landmark[2][2], m_eAEFDArea.rAreas[0].i4Landmark[0][3],
        m_eAEFDArea.rAreas[0].i4Landmark[1][3], m_eAEFDArea.rAreas[0].i4Landmark[2][3]);

    pinput->mFrameData.eAEComp = (CCU_LIB3A_AE_EVCOMP_T)m_eAEEVcompPreset;
    AE_LOG_IF(m_3ALogEnable, "[%s()] EVComp:%x\n", __FUNCTION__, pinput->mFrameData.eAEComp);
    pinput->mFrameData.eAEFlickerMode = (CCU_LIB3A_AE_FLICKER_MODE_T)m_eAEFlickerMode;
    pinput->mFrameData.eAEAutoFlickerMode = (CCU_LIB3A_AE_FLICKER_AUTO_MODE_T)m_eAEAutoFlickerMode;
    pinput->mFrameData.u4AEISOSpeed = m_u4AEISOSpeed;
    if (m_i4AEMaxFps > 0)
    {
        pinput->mFrameData.i4AEMaxFps = m_i4AEMaxFps;
    }
    else
    {
        pinput->i4AEMaxFps = LIB3A_AE_FRAMERATE_MODE_30FPS;
    }
    AE_LOG_IF(m_3ALogEnable, "[%s()] m_i4AEMinFps:%d m_i4AEMaxFps:%d\n", __FUNCTION__, m_i4AEMinFps, m_i4AEMaxFps);
    pinput->mFrameData.i4AEMinFps = m_i4AEMinFps;
    pinput->mFrameData.i4SensorMode = m_eSensorMode;
    pinput->mFrameData.eAEScene = (CCU_LIB3A_AE_SCENE_T)m_eAEScene;
    pinput->mFrameData.eAECamMode = (CCU_LIB3A_AECAM_MODE_T)m_eAECamMode;
    pinput->mFrameData.pAAOProcInfo.Pso_Output_Path = (CCU_PSO_PATH_SEL_ENUM)m_strAAOProcInfo.Pso_Output_Path;
    memcpy(&pinput->mFrameData.pAAOProcInfo.awb_gain, &m_strAAOProcInfo.awb_gain, sizeof(CCU_AWB_GAIN_T));
    pinput->mFrameData.pAAOProcInfo.awb_gain_Unit = m_strAAOProcInfo.awb_gain_Unit;

    pinput->mFrameData.pAAOProcInfo.OB_Gain_Unit = m_strAAOProcInfo.OB_Gain_Unit;
    memcpy(pinput->mFrameData.pAAOProcInfo.OB_Gain, m_strAAOProcInfo.OB_Gain, 4 * sizeof(MUINT32));
    memcpy(pinput->mFrameData.pAAOProcInfo.OB_Offset, m_strAAOProcInfo.OB_Offset, 4 * sizeof(MUINT32));;
    //pinput->mFrameData.pAAOProcInfo.LSC_Gain_Unit = m_strAAOProcInfo.LSC_Gain_Unit;
    //memcpy(pinput->mFrameData.pAAOProcInfo.LSC_Gain_Grid_G, m_strAAOProcInfo.LSC_Gain_Grid_G, AE_V4P0_BLOCK_NO *AE_V4P0_BLOCK_NO *sizeof(MUINT16));
    {
        //int i, j; /* fix build warning */
        // AE_LOG("[%s()] dbglsc1 \n", __FUNCTION__);
        // for (i = 0; i < AE_V4P0_BLOCK_NO; ++i)
        // {
        // for (j = 0; j < AE_V4P0_BLOCK_NO; ++j)
        // {
        // AE_LOG("[%s()] %x, ", __FUNCTION__, m_strAAOProcInfo.LSC_Gain_Grid_G[i *AE_V4P0_BLOCK_NO + j]);
        // }
        // AE_LOG("[%s()] \n", __FUNCTION__);
        // }
        // AE_LOG("[%s()] dbglsc1 \n", __FUNCTION__);
        // for (i = 0; i < AE_V4P0_BLOCK_NO; ++i)
        // {
        // for (j = 0; j < AE_V4P0_BLOCK_NO; ++j)
        // {
        // AE_LOG("[%s()] %x, ", __FUNCTION__, m_strAAOProcInfo.LSC_Gain_Grid_G[i *AE_V4P0_BLOCK_NO + j]);
        // }
        // AE_LOG("[%s()] \n", __FUNCTION__);
        // }
    }
    //MUINT32 LSC_Gain_Grid_B[AE_V4P0_BLOCK_NO *AE_V4P0_BLOCK_NO];

    pinput->mFrameData.pAAOProcInfo.PreGain1_Unit = m_strAAOProcInfo.PreGain1_Unit;

    memcpy(&pinput->mFrameData.pAAOProcInfo.PreGain1, &m_strAAOProcInfo.PreGain1, sizeof(CCU_PRE_GAIN1_T));
    memcpy(pinput->mFrameData.pAAOProcInfo.RGB2Y_Coef, m_strAAOProcInfo.RGB2Y_Coef, 3 * sizeof(MUINT32));

    pinput->rAEStatCfg = m_rAEStatCfg;
    pinput->i4ZoomRatio = m_i4ZoomRatio;

    return S_AE_OK;
}

MRESULT AeMgr::prepareCCUStartParam(AE_FLOW_INPUT_INFO_T * pinput)
{
    pinput->pAEInitInput = &m_rAEInitInput;
    pinput->pPreviewTableF = &m_PreviewTableF;
    pinput->pPreviewTableCurrent = m_pPreviewTableCurrent;
    //pinput->pISPNvramISOInterval = &(m_rISPNvramISOInterval);
    pinput->pISPNvramOBC_Table = &(m_rISP5NvramOBC_Table[0]); /*&(m_rISPNvramOBC_Table[0]); */
    pinput->pISP5NvramOBC_Table = &(m_rISP5NvramOBC_Table[0]);
    pinput->i4SensorIdx = m_i4SensorIdx;
    pinput->eSensorMode = m_eSensorMode;
    pinput->u4AAOmode = m_i4AAOmode;
    pinput->u4FinerEVIdxBase = m_u4FinerEVIdxBase;
    pinput->u4IndexFMax = m_u4IndexFMax;
    pinput->u4IndexFMin = m_u4IndexFMin;
    pinput->u4Index = m_u4Index;
    pinput->u4IndexF = m_u4IndexF;
    pinput->u4Exp = m_rAEOutput.rPreviewMode.u4Eposuretime;
    pinput->u4Afe = m_rAEOutput.rPreviewMode.u4AfeGain;
    pinput->u4Isp = m_rAEOutput.rPreviewMode.u4IspGain;
    pinput->u4RealISO = m_rAEOutput.rPreviewMode.u4RealISO;
    pinput->bIsCCUAEInit = m_bIsCCUAEInit;
    pinput->u2Length = ISP_NVRAM_Ptr->Iso_Env.u2Length;
    pinput->u2IDX_Partition = &(ISP_NVRAM_Ptr->Iso_Env.IDX_Partition[0]);
    pinput->i4AEMaxFps = m_i4AEMaxFps;
    return S_AE_OK;
}

MRESULT AeMgr::copyCCUAEInfo2mgr(AE_CORE_MAIN_OUT * pCCUoutput)
{
    if ((m_eAEMode != LIB3A_AE_MODE_OFF) /*&& (pCCUoutput->enumCpuAction == AE_CPU_ACTION_DO_FULL_AE) */)
    {
        Mutex::Autolock lock(m_Lock);
        AE_LOG_IF(m_3ALogEnable, "[%s()] + add protection \n", __FUNCTION__);
        if (m_u4IndexF != pCCUoutput->a_Output.u4IndexF)
        {
            AE_LOG("[%s()] CCU NextidxF:%d->%d\n", __FUNCTION__, m_u4IndexF, pCCUoutput->a_Output.u4IndexF);
        }

        //Fixme: check strAEOutput is differ from strCcuAeOutput
        if (!mbIsIndexChange)
        {
            m_rAEOutput.rPreviewMode.u4Eposuretime = pCCUoutput->a_Output.EvSetting.u4Eposuretime;
            m_rAEOutput.rPreviewMode.u4AfeGain = pCCUoutput->a_Output.EvSetting.u4AfeGain;
            m_rAEOutput.rPreviewMode.u4IspGain = pCCUoutput->a_Output.EvSetting.u4IspGain;
            m_u4Index = pCCUoutput->a_Output.u4Index;
            m_u4IndexF = pCCUoutput->a_Output.u4IndexF;
        }
        m_i4AEidxNext = m_u4Index;
        //
        m_rAEOutput.rPreviewMode.u4CWValue = pCCUoutput->a_Output.u4CWValue;
        m_rAEOutput.rPreviewMode.u2FrameRate = pCCUoutput->a_Output.u2FrameRate;
        m_rAEOutput.rPreviewMode.u4RealISO = pCCUoutput->a_Output.u4ISO;
        m_rAEOutput.rPreviewMode.i2FlareOffset = pCCUoutput->a_Output.u4Prvflare;
        m_rAEOutput.rPreviewMode.i2FlareGain = 512 * 4095 / (4095 - pCCUoutput->a_Output.u4Prvflare);
        AE_LOG_IF(m_3ALogEnable, "[%s()] Flare offset:%d(%d) Gain:%d\n", __FUNCTION__, m_rAEOutput.rPreviewMode.i2FlareOffset, pCCUoutput->a_Output.u4Prvflare, m_rAEOutput.rPreviewMode.i2FlareGain);

        m_bAEStable = pCCUoutput->a_Output.bAEStable;
        if (m_bForceResetCCUStable == MTRUE)
        {
            if (m_bAEStable == MTRUE)
            {
                AE_LOG("[%s()] Skip AE stable, CCU bStable:%d Force CCU stable:%d\n", __FUNCTION__, pCCUoutput->a_Output.bAEStable, m_bForceResetCCUStable);
                m_bAEStable = MFALSE;
            }
            else
            {
                m_bForceResetCCUStable = MFALSE;
                AE_LOG("[%s()] AE is unstable, reset Force CCU stable:%d\n", __FUNCTION__, m_bForceResetCCUStable);
            }
        }

        m_u4FDProbForCCU = (pCCUoutput->a_Output.u4FaceAEStable & 0xffff0000) >> 16;
        m_u4FaceFailCntForCCU = pCCUoutput->a_Output.u4FaceFailCnt;
        m_u4FaceFoundCntForCCU = pCCUoutput->a_Output.u4FaceFoundCnt;
        // AE_LOG_IF(m_3ALogEnable, "[%s()] dbgccuout u4FaceAEStable=%d, m_u4FaceAEStable=%d, m_u4FDProb=%d\n", __FUNCTION__, out->a_Output.u4FaceAEStable, g_pAEAlgoData->m_u4FaceAEStable, g_pAEAlgoData->m_MeterFDSts.m_u4FDProb, 0);
        AE_LOG_IF(m_3ALogEnable, "[%s()] Eric 5 m_bAEStable:%d m_u4FDProbForCCU = %d m_u4FaceFailCntForCCU = %d m_u4FaceFoundCntForCCU = %d mbIsIndexChange: %d", __FUNCTION__, m_bAEStable, m_u4FDProbForCCU, m_u4FaceFailCntForCCU, m_u4FaceFoundCntForCCU, mbIsIndexChange);
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
        // m_i4AEidxNext = pCCUoutput->a_Output.i4AEidxNext;
        // m_i4AEidxCurrent = pCCUoutput->a_Output.i4AEidxCurrent;
        m_u4Prvflare = pCCUoutput->a_Output.u4Prvflare;
        m_i2AEFaceDiffIndex = pCCUoutput->a_Output.i2FaceDiffIndex;
        m_i4EVvalue = pCCUoutput->a_Output.i4EV;
        m_BVvalue = pCCUoutput->a_Output.Bv;
        if (m_bStrobeOn == MFALSE)
        {
            m_BVvalueWOStrobe = pCCUoutput->a_Output.Bv;
        }
        m_AOECompBVvalue = pCCUoutput->a_Output.AoeCompBv;
        m_u4CCUResultCwvy = pCCUoutput->a_Output.u4CWValue;
        m_u4CCUResultCwvyStable = pCCUoutput->a_Output.u4CWRecommendStable;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_COUNTWAIT, MFALSE);
        AE_LOG_IF(m_3ALogEnable, "[%s()] Eric 5 m_u4Index: 0x:%x BV = %d", __FUNCTION__, m_u4Index, m_AOECompBVvalue);
        m_u4CCUEndBankIdx = pCCUoutput->EndBankIdx;

        /*//workaround
        MUINT32 u4FinerEVIdxBase = m_u4IndexF * 10 / m_u4Index;

        if (u4FinerEVIdxBase < 15)
        {
            u4FinerEVIdxBase = 1;
        }
        else if (u4FinerEVIdxBase < 25)
        {
            u4FinerEVIdxBase = 2;
        }
        else if (u4FinerEVIdxBase < 35)
        {
            u4FinerEVIdxBase = 3;
        }
        //workaround
        if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
        {
            m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
            m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
            m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
        }*/

        m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax , m_u4IndexMin , m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
        m_pIAeAlgo->setFlareInfo(m_u4Prvflare);
        //updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
        // Update to CPU AE algorithm
        AE_CORE_CTRL_CPU_EXP_INFO rAECoreCtrlCPUInfo;
        memset(&rAECoreCtrlCPUInfo, 0, sizeof(AE_CORE_CTRL_CPU_EXP_INFO));
        rAECoreCtrlCPUInfo.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
        rAECoreCtrlCPUInfo.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
        rAECoreCtrlCPUInfo.u4IspGain = m_rAEOutput.rPreviewMode.u4IspGain;
        rAECoreCtrlCPUInfo.m_u4EffectiveIndex = m_i4AEidxNext;
        m_pIAeFlowCPU->update(&rAECoreCtrlCPUInfo);
    }
    else
    {
        if (m_eAEMode != LIB3A_AE_MODE_OFF && (pCCUoutput->a_Output.EvSetting.u4Eposuretime != 0) &&
            (pCCUoutput->a_Output.EvSetting.u4AfeGain != 0) && (pCCUoutput->a_Output.EvSetting.u4IspGain != 0))
        {
            m_rAEOutput.rPreviewMode.u4Eposuretime = pCCUoutput->a_Output.EvSetting.u4Eposuretime;
            m_rAEOutput.rPreviewMode.u4AfeGain = pCCUoutput->a_Output.EvSetting.u4AfeGain;
            m_rAEOutput.rPreviewMode.u4IspGain = pCCUoutput->a_Output.EvSetting.u4IspGain;
            //updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
        }
    }
    MFLOAT u2FrameRate = (MFLOAT)pCCUoutput->a_Output.u2FrameRate;
    if (u2FrameRate > m_i4AEMaxFps)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s()] Frame rate too large:%6.3f->%d\n", __FUNCTION__, u2FrameRate, m_i4AEMaxFps);
        u2FrameRate = (MFLOAT)m_i4AEMaxFps;
    }
    if (u2FrameRate != 0)
    {
        m_rSensorCurrentInfoCCU.u8FrameDuration = 10005000000L / u2FrameRate;
    }
    else
    {
        m_rSensorCurrentInfoCCU.u8FrameDuration = 33000000L;
        AE_LOG("[%s()] No Frame rate value, using default frame rate value (33ms)\n", __FUNCTION__);
    }
    m_rSensorCurrentInfoCCU.u8ExposureTime = (MINT64)pCCUoutput->a_Output.EvSetting.u4Eposuretime * 1000L;
    m_rSensorCurrentInfoCCU.u4Sensitivity = pCCUoutput->a_Output.u4ISO;
    if (m_rSensorCurrentInfoCCU.u8FrameDuration < m_rSensorCurrentInfoCCU.u8ExposureTime)
        m_rSensorCurrentInfoCCU.u8FrameDuration = m_rSensorCurrentInfoCCU.u8ExposureTime;
    ISP_NVRAM_OBC_T CCUObcResult;
    CCUObcResult.gain0.val = pCCUoutput->ObcResult.gain0.val;
    CCUObcResult.gain1.val = pCCUoutput->ObcResult.gain1.val;
    CCUObcResult.gain2.val = pCCUoutput->ObcResult.gain2.val;
    CCUObcResult.gain3.val = pCCUoutput->ObcResult.gain3.val;
    CCUObcResult.offst0.val = pCCUoutput->ObcResult.offst0.val;
    CCUObcResult.offst1.val = pCCUoutput->ObcResult.offst1.val;
    CCUObcResult.offst2.val = pCCUoutput->ObcResult.offst2.val;
    CCUObcResult.offst3.val = pCCUoutput->ObcResult.offst3.val;
    AE_LOG_IF(m_3ALogEnable, "[%s()] CCUObcResult gain0 = %d , offst0 = %d m_rSensorCurrentInfoCCU frame/exp/iso = %d/%d/%d \n",
        __FUNCTION__, CCUObcResult.gain0.val, CCUObcResult.offst0.val,
        m_rSensorCurrentInfoCCU.u8FrameDuration, m_rSensorCurrentInfoCCU.u8ExposureTime, m_rSensorCurrentInfoCCU.u4Sensitivity);
    m_pCCUresultCB->doNotifyCb(0, (MINTPTR)&CCUObcResult, NULL, NULL);

    return S_AE_OK;
}

MRESULT AeMgr::setAEInfo2CCU(MUINT32 exp, MUINT32 afe, MUINT32 isp, MUINT32 iso, MUINT32 effective_idx, MUINT32 /* idx */ , MUINT32 delata_idx)
{
    //(void)idx;
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
    {
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
    if(m_eAETargetMode == AE_MODE_NORMAL &&
        m_i4SMSubSamples == 1 &&
        !m_bAElimitor &&
        m_u4UpdateFrameRate_x10 <= LIB3A_AE_FRAMERATE_MODE_30FPS &&
        (m_eAEMode != LIB3A_AE_MODE_OFF || m_eSensorMode != SENSOR_SCENARIO_ID_NORMAL_CAPTURE) &&
        m_bPerframeAEFlag &&
        !m_bIsFrameRateLocked &&
        m_bEnableCustMode == 0)
    {
        res = MTRUE;
    }
    else
    {
        AE_LOG_IF(m_3ALogEnable, "[%s] CCU start condition not reached, m_eAEMode:%d m_bPerframeAEFlag:%d\n", __FUNCTION__, m_eAEMode, m_bPerframeAEFlag);
    }
    AE_LOG_IF(m_3ALogEnable, "[%s] AETargetMode:%d SMSubSamples:%d m_bAElimitor:%d FPS:%6.3f AEMode:%d IsFrameRateLocked:%d m_bEnableCustMode:%d\n", __FUNCTION__,
        m_eAETargetMode, m_i4SMSubSamples, m_bAElimitor, m_u4UpdateFrameRate_x10, m_eAEMode, m_bIsFrameRateLocked, m_bEnableCustMode);
    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// CPU flow implement //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
MRESULT AeMgr::prepareCPUStartParam(AE_FLOW_CPUSTART_INFO_T * pinput)
{
    pinput->pIAeAlgo = (MVOID*)m_pIAeAlgo;
    pinput->eAETargetMode = m_eAETargetMode;
    pinput->pAEInitInput = &(m_rAEInitInput.rAENVRAM->rAeParam);
    pinput->pAEInitInputNVRAM = &(m_p3ANVRAM->AE[m_u4AENVRAMIdx]);
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

MRESULT AeMgr::prepareSchedulerParam(AE_FLOW_SCHEDULE_INFO_T * pinput, E_AE_FLOW_CPU_T etype, MBOOL bAAASchedule, MINT32 i4ActiveAEItem)
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
    AE_LOG_IF(m_3ALogEnable, "[%s()] AFTouchROISet:%d", __FUNCTION__, m_bAFTouchROISet);
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
        pinput->i4CwvY = m_u4CCUResultCwvy;//m_pIAeAlgo->getBrightnessCenterWeightedValue();
        pinput->u4CwvYStable = m_u4CCUResultCwvyStable;
        AE_LOG_IF(m_3ALogEnable, "[%s()] CwvY:%d CwvYStable:%d", __FUNCTION__, m_u4CCUResultCwvy, m_u4CCUResultCwvyStable);
    }
    else if (m_pIAeAlgo != NULL)
    {
        pinput->i4CwvY = m_pIAeAlgo->getBrightnessCenterWeightedValue();
        pinput->u4CwvYStable = m_u4StableYValue;
    }
    else
    {
        AE_LOG_IF(m_3ALogEnable, "[%s()] m_pIAeAlgo is NULL ", __FUNCTION__);
    }

    pinput->bFaceAEAreaChange = m_bFaceAEAreaChage;
    pinput->i4WaitVDNum = m_i4WaitVDNum;
    pinput->u4WOFDcnt = m_u4WOFDCnt;
    pinput->u4HwMagicNum = m_u4HwMagicNum;
    return S_AE_OK;
}

MRESULT AeMgr::prepareCalculateParam(AE_FLOW_CALCULATE_INFO_T * pinput, E_AE_FLOW_CPU_T etype, MVOID * pStatistic, MINT64 i8timestamp)
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
    pinput->pAeCustParam = (MVOID*)&m_AeCustParam;
    pinput->i4ShotMode = m_i4SuperNightShotMode;
    pinput->i4SensorId = m_i4CurrSensorId;
    pinput->u41xGainISOvalue = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    pinput->u4MinSensorGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MinGain;
    pinput->u4MaxSensorGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain;
    pinput->u4AEMode = m_eAEMode;
    pinput->u4TargetExp = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime;
    pinput->u4TargetISO = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity;
    pinput->u4MaxISPGain = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4IndexMax].u4IspGain;
    pinput->u4MaxShutter = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[m_u4IndexMax].u4Eposuretime;
    pinput->u4FinerEVIdxBase = m_u4FinerEVIdxBase;
    return S_AE_OK;
}

MRESULT AeMgr::copyScheduleInfo2mgr(AE_FLOW_SCHEDULE_OUTPUT_T * poutput, MBOOL& bCalc, MBOOL& bApply)
{
    bCalc = *(poutput->pModifiedCalc);
    bApply = *(poutput->pModifiedApply);
    m_bPerframeAEFlag = *(poutput->pModifiedPerframeFlag);
    m_i4WaitVDNum = *(poutput->pWaitVDNum);
    return S_AE_OK;
}

MRESULT AeMgr::PresetControlCCU(MBOOL SkipCCUAE)
{
    AE_FLOW_CONTROL_INFO_T PresetcontrolInfo;
    AE_LOG_IF(m_3ALogEnable, "[%s()] SkipCCUAE:%d\n", __FUNCTION__, SkipCCUAE);
    if (SkipCCUAE)
        m_bForceSkipCCU = MTRUE;
    prepareCCUPvParam(&PresetcontrolInfo);
    m_pIAeFlowCCU->PresetControlCCU(&PresetcontrolInfo);
    mbIsPlineChange = MFALSE;
    mbIsIndexChange = MFALSE;
    return S_AE_OK;
}

MBOOL AeMgr::IsNeedPresetControlCCU()
{
    MBOOL res = m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING);
    return res;
}

MRESULT AeMgr::IsCCUAEInit(MBOOL bInit)
{
    m_bIsCCUAEInit = bInit;
    return S_AE_OK;
}

