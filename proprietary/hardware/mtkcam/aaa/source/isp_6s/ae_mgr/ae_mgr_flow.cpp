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
#include <awb_mgr/awb_mgr_if.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include "camera_custom_msdk.h"
#include <cutils/properties.h>


using namespace NS3A;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCcuIf;

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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                   prepare AE algo common information                       //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/* prepare initialization information for AE algo */
MRESULT AeMgr::prepareAlgoInitInfo(AE_CORE_INIT* pInfo)
{
    memset(pInfo, 0, sizeof(AE_CORE_INIT));
    /* assign AE NVRAM info */
    pInfo->pAeNVRAM             = (CCU_AE_NVRAM_T *)(m_rAEInitInput.rAENVRAM);
    pInfo->pCusAEParam          = (AE_CUST_PARAM_T *)(&m_rAECustPARAM);
    /*to do by P80*/
    pInfo->prAeSyncNvram        = (CCU_AESYNC_NVRAM_T *)(&(m_p3ANVRAM->AESYNC));
    pInfo->pCurrentGainList     = (CCU_strAEPLineGainList *)((&m_pAEPlineTable->AEGainList));
    pInfo->m_u4AETarget         = m_rAEInitInput.rAENVRAM->rCCTConfig.u4AETarget;
    pInfo->u4InitIndex          = m_rAEInitInput.rAENVRAM->rCCTConfig.u4InitIndex;
    pInfo->i4BVOffset           = m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset;
    pInfo->m_i4AEMaxBlockWidth  = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
    pInfo->m_i4AEMaxBlockHeight = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
    pInfo->pEVValueArray        = &(m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[0]);
    pInfo->pAETouchMovingRatio  = (CCU_strAEMovingRatio*)&((m_rAEInitInput.rAENVRAM->rAeParam.AETouchMovingRatio));
    AE_LOG_IF(m_3ALogEnable,"[%s] m_u4AETarget: %d, u4InitIndex: %d, i4BVOffset: %d, m_i4AEMaxBlockWidth: %d, m_i4AEMaxBlockHeight: %d\n",
              __FUNCTION__, pInfo->m_u4AETarget, pInfo->u4InitIndex, pInfo->i4BVOffset, pInfo->m_i4AEMaxBlockWidth, pInfo->m_i4AEMaxBlockHeight);

    /* assign AE window config info */
    pInfo->m_u4AAO_AWBValueWinSize = m_rAEInitInput.u4AAO_AWBValueWinSize;            // bytes of AWB Value window
    pInfo->m_u4AAO_AWBSumWinSize   = m_rAEInitInput.u4AAO_AWBSumWinSize;              // bytes of AWB Sum window
    pInfo->m_u4AAO_AEYWinSize      = m_rAEInitInput.u4AAO_AEYWinSize;                 // bytes of AEY window
    pInfo->m_u4AAO_AEYWinSizeSE    = m_rAEInitInput.u4AAO_AEYWinSizeSE;               // bytes of AEY window
    pInfo->m_u4AAO_AEOverWinSize   = m_rAEInitInput.u4AAO_AEOverWinSize;              // bytes of AEOverCnt window
    pInfo->m_u4AAO_HistSize        = m_rAEInitInput.u4AAO_HistSize;                   // bytes of each Hist
    pInfo->m_u4AAO_HistSizeSE      = m_rAEInitInput.u4AAO_HistSizeSE;                 // bytes of each Hist
    AWB_STAT_INFO_T awb_stat;
    //IAwbMgr::getInstance().getAWBStatInfo(m_eSensorDev,awb_stat);
    /* temp for CCU ae test by Kurt */
    pInfo->m_u4AAO_HistSize        = 4;// m_rAEInitInput.u4AAO_HistSize;                   // bytes of each Hist
    pInfo->m_u4AAO_HistSizeSE      = 4;//m_rAEInitInput.u4AAO_HistSizeSE;                 // bytes of each Hist
    pInfo->Call_AE_Core_init       = 1;
    pInfo->AWBStatConfig.u4WindowSizeX    = awb_stat.i4WindowSizeX;
    pInfo->AWBStatConfig.u4WindowSizeY    = awb_stat.i4WindowSizeY;
    pInfo->AWBStatConfig.u4LowThresholdR  = awb_stat.i4LowThresholdR;
    pInfo->AWBStatConfig.u4LowThresholdG  = awb_stat.i4LowThresholdG;
    pInfo->AWBStatConfig.u4LowThresholdB  = awb_stat.i4LowThresholdB;
    pInfo->AWBStatConfig.u4HighThresholdR = awb_stat.i4HighThresholdR;
    pInfo->AWBStatConfig.u4HighThresholdG = awb_stat.i4HighThresholdG;
    pInfo->AWBStatConfig.u4HighThresholdB = awb_stat.i4HighThresholdB;
    AE_LOG_IF(m_3ALogEnable,"[%s] m_u4AAO_AWBValueWinSize: %d, m_u4AAO_AWBSumWinSize: %d, m_u4AAO_AEYWinSize: %d, m_u4AAO_AEOverWinSize: %d, m_u4AAO_HistSize: %d WindowSizeX/Y: %d %d, u4LowThresholdR/G/B: %d %d %d, u4HighThresholdR/G/B: %d %d %d \n",
              __FUNCTION__, pInfo->m_u4AAO_AWBValueWinSize, pInfo->m_u4AAO_AWBSumWinSize, pInfo->m_u4AAO_AEYWinSize, pInfo->m_u4AAO_AEOverWinSize, pInfo->m_u4AAO_HistSize,
              pInfo->AWBStatConfig.u4WindowSizeX, pInfo->AWBStatConfig.u4WindowSizeY,pInfo->AWBStatConfig.u4LowThresholdR,pInfo->AWBStatConfig.u4LowThresholdG,
              pInfo->AWBStatConfig.u4LowThresholdB,pInfo->AWBStatConfig.u4HighThresholdR,pInfo->AWBStatConfig.u4HighThresholdG,pInfo->AWBStatConfig.u4HighThresholdB);

    /* assign AE mode info */
    pInfo->i4SensorMode       = m_rAEInitInput.i4SensorMode;
    pInfo->InitAETargetMode   = (CCU_AE_TargetMODE)m_eAETargetMode;
    pInfo->u4AEISOSpeed       = m_rAEInitInput.u4AEISOSpeed;
    pInfo->i4AEMaxFps         = m_rAEInitInput.i4AEMaxFps;
    pInfo->i4AEMinFps         = m_rAEInitInput.i4AEMinFps;
    pInfo->eSensorDev         = (CCU_AE_SENSOR_DEV_T)m_rAEInitInput.eSensorDev;
    pInfo->eAEScene           = (CCU_LIB3A_AE_SCENE_T)m_rAEInitInput.eAEScene;
    pInfo->eAECamMode         = (CCU_LIB3A_AECAM_MODE_T)m_rAEInitInput.eAECamMode;
    pInfo->eAEMeteringMode    = (CCU_LIB3A_AE_METERING_MODE_T)m_rAEInitInput.eAEMeteringMode;
    pInfo->eAEEVcomp          = (CCU_LIB3A_AE_EVCOMP_T)m_rAEInitInput.eAEEVcomp;
    pInfo->eAEFlickerMode     = (CCU_LIB3A_AE_FLICKER_MODE_T)m_rAEInitInput.eAEFlickerMode;
    pInfo->eAEAutoFlickerMode = (CCU_LIB3A_AE_FLICKER_AUTO_MODE_T)m_rAEInitInput.eAEAutoFlickerMode;
    pInfo->eAEScenario        = (CCU_CAM_SCENARIO_T)m_rAEInitInput.eAEScenario; // what is this? currently should only have NVRAM index
    AE_LOG_IF(m_3ALogEnable,"[%s] i4SensorMode: %d, eAETargetMode: %d, u4AEISOSpeed: %d, i4AEMaxFps: %d, i4AEMinFps: %d, eAEScene: %d, eAEEVcomp: %d\n",
              __FUNCTION__, pInfo->i4SensorMode, pInfo->InitAETargetMode, pInfo->u4AEISOSpeed, pInfo->i4AEMaxFps, pInfo->i4AEMinFps, pInfo->eAEScene, pInfo->eAEEVcomp);

    /* assign AE P-line info */
    pInfo->pCurrentTableF     = (CCU_strFinerEvPline*)(&m_PreviewTableF);
    pInfo->pCurrentTable      = (CCU_strEvPline*) m_pPreviewTableCurrent->pCurrentTable;
    pInfo->m_u4IndexFMax      = m_u4IndexFMax;
    pInfo->m_u4IndexFMin      = m_u4IndexFMin;
    pInfo->m_u4IndexMax       = m_u4IndexFMax/m_u4FinerEVIdxBase;
    pInfo->m_u4IndexMin       = m_u4IndexFMin/m_u4FinerEVIdxBase;
    pInfo->m_u4FinerEVIdxBase = m_u4FinerEVIdxBase; // Real index base
    pInfo->i4MaxBV            = m_pPreviewTableCurrent->i4MaxBV;
    pInfo->i4MinBV            = m_pPreviewTableCurrent->i4MinBV;
    AE_LOG_IF(m_3ALogEnable,"[%s] m_u4IndexFMax: %d, m_u4IndexFMin: %d, m_u4IndexMax: %d, m_u4IndexMin: %d, m_u4FinerEVIdxBase: %d, i4MaxBV: %d, i4MinBV: %d\n",
              __FUNCTION__, pInfo->m_u4IndexFMax, pInfo->m_u4IndexFMin, pInfo->m_u4IndexMax, pInfo->m_u4IndexMin, pInfo->m_u4FinerEVIdxBase, pInfo->i4MaxBV, pInfo->i4MinBV);

    /* assign initial AE exposure setting */
    memcpy(&(pInfo->InitAESetting), &(m_rAESettingPreview), sizeof(strAERealSetting));
    AE_LOG_IF(m_3ALogEnable,"[%s] init AE setting: %d/%d/%d, LE: %d/%d/%d ME: %d/%d/%d SE: %d/%d/%d, ISO: %d, u4Index/F: %d/%d(%d), u4CurHdrRatio: %d\n",
              __FUNCTION__, pInfo->InitAESetting.EvSetting.u4Eposuretime, pInfo->InitAESetting.EvSetting.u4AfeGain, pInfo->InitAESetting.EvSetting.u4IspGain,
              pInfo->InitAESetting.HdrEvSetting.i4LEExpo, pInfo->InitAESetting.HdrEvSetting.i4LEAfeGain, pInfo->InitAESetting.HdrEvSetting.i4LEIspGain,
              pInfo->InitAESetting.HdrEvSetting.i4MEExpo, pInfo->InitAESetting.HdrEvSetting.i4MEAfeGain, pInfo->InitAESetting.HdrEvSetting.i4MEIspGain,
              pInfo->InitAESetting.HdrEvSetting.i4SEExpo, pInfo->InitAESetting.HdrEvSetting.i4SEAfeGain, pInfo->InitAESetting.HdrEvSetting.i4SEIspGain,
              pInfo->InitAESetting.u4ISO, pInfo->InitAESetting.u4Index, pInfo->InitAESetting.u4IndexF, pInfo->InitAESetting.u4AEFinerEVIdxBase, pInfo->InitAESetting.HdrEvSetting.u4CurHdrRatio);

    /* temp: to be removed at p80 */
    pInfo->m_LumLog2x1000 = (MINT32 *)Log2x1000;

    return S_AE_OK;
}

/* prepare frame sync information for AE algo */
MRESULT AeMgr::prepareAlgoFrameInfo(AE_CORE_CTRL_CCU_VSYNC_INFO* pInfo)
{
    /* assign AE mode info */
    pInfo->i4SensorMode          = m_eSensorMode;
    pInfo->u4AEISOSpeed          = m_u4AEISOSpeed;
    pInfo->bIsoSpeedReal         = m_bRealISOSpeed;
    pInfo->i4AEMaxFps            = m_i4AEMaxFps;
    pInfo->i4AEMinFps            = m_i4AEMinFps;
    pInfo->i4DeltaSensitivityIdx = m_i4DeltaSensitivityIdx;
    pInfo->i4DeltaBVIdx          = m_i4DeltaBVIdx;
    pInfo->eAEScene              = (CCU_LIB3A_AE_SCENE_T) m_eAEScene;
    pInfo->eAECamMode            = (CCU_LIB3A_AECAM_MODE_T) m_eAECamMode;
    pInfo->eAEMeteringMode       = (CCU_LIB3A_AE_METERING_MODE_T) m_eAEMeterMode;
    pInfo->eAEComp               = (CCU_LIB3A_AE_EVCOMP_T) m_eAEEVcomp;
    pInfo->eAEFlickerMode        = (CCU_LIB3A_AE_FLICKER_MODE_T) m_eAEFlickerMode;
    pInfo->eAEAutoFlickerMode    = (CCU_LIB3A_AE_FLICKER_AUTO_MODE_T) m_eAEAutoFlickerMode;
    AE_LOG_IF(m_3ALogEnable,"[%s] i4SensorMode: %d, u4AEISOSpeed: %d, i4AEMaxFps: %d, i4AEMinFps: %d, i4DeltaSensitivityIdx: %d, i4DeltaBVIdx: %d\n",
              __FUNCTION__, pInfo->i4SensorMode, pInfo->u4AEISOSpeed, pInfo->i4AEMaxFps, pInfo->i4AEMinFps, pInfo->i4DeltaSensitivityIdx, pInfo->i4DeltaBVIdx);
    AE_LOG_IF(m_3ALogEnable,"[%s] eAEScene: %d, eAECamMode: %d, eAEMeteringMode: %d, eAEComp: %d, eAEFlickerMode: %d, eAEAutoFlickerMode: %d\n",
              __FUNCTION__, pInfo->eAEScene, pInfo->eAECamMode, pInfo->eAEMeteringMode, pInfo->eAEComp, pInfo->eAEFlickerMode, pInfo->eAEAutoFlickerMode);

    /* assign AE control info */
    pInfo->camReqNumber          = m_u4RequestNum;
    pInfo->AECtrl.eAEState       = (AE_STATE_ENUM)(getAEAlgoState());
    pInfo->u4CycleCnt            = getAECycleCnt();
    pInfo->AECtrl.bIsCalculate   = !(m_bSkipCalAE || m_bFDSkipCalAE || !m_bEnableAE || (m_bHDRshot || m_u4HDRcountforrevert));
    pInfo->AECtrl.bIsAPLock      = (m_bAPAELock || m_bAdbAELock | m_bLockExposureSetting) ? MTRUE : MFALSE;
    pInfo->AECtrl.bIsAFLock      = m_bAFAELock;
    pInfo->u4MagicNo             = m_u4HwMagicNum;
    pInfo->bIsLimitMode          = m_bAElimitor;
    pInfo->bIsSlowMotion         = m_bEnSWBuffMode;
    if (m_bHDRshot == MFALSE && m_u4HDRcountforrevert >= 1) {
        AE_LOG("[%s] skip AE calculation after HDR shot, m_u4HDRcountforrevert: %d\n", __FUNCTION__, m_u4HDRcountforrevert);
        m_u4HDRcountforrevert--;
    }
    AE_LOG_IF(m_3ALogEnable,"[%s] u4ReqNum: %d, eAEState: %d, u4CycleCnt: %d, bIsCalculate: %d, bIsAPLock/bIsAFLock: %d/%d, m_bSkipCalAE: %d, m_bFDSkipCalAE: %d, m_eAEMode: %d, m_bEnableAE: %d, m_bHDRshot: %d, m_u4HDRcountforrevert: %d\n",
              __FUNCTION__, pInfo->camReqNumber, pInfo->AECtrl.eAEState, pInfo->u4CycleCnt, pInfo->AECtrl.bIsCalculate, pInfo->AECtrl.bIsAPLock, pInfo->AECtrl.bIsAFLock, m_bSkipCalAE, m_bFDSkipCalAE, m_eAEMode, m_bEnableAE, m_bHDRshot, m_u4HDRcountforrevert);

    /* assign window config info & AWB gain */
    pInfo->m_AEWinBlock.u4XHi   = m_rAEStatCfg.m_AEWinBlock.u4XHi;
    pInfo->m_AEWinBlock.u4XLow  = m_rAEStatCfg.m_AEWinBlock.u4XLow;
    pInfo->m_AEWinBlock.u4YHi   = m_rAEStatCfg.m_AEWinBlock.u4YHi;
    pInfo->m_AEWinBlock.u4YLow  = m_rAEStatCfg.m_AEWinBlock.u4YLow;
    pInfo->pAAOProcInfo.awb_gain_Unit = m_strAAOProcInfo.awb_gain_Unit;
    pInfo->pAAOProcInfo.awb_gain.i4R  = m_strAAOProcInfo.awb_gain.i4R;
    pInfo->pAAOProcInfo.awb_gain.i4G  = m_strAAOProcInfo.awb_gain.i4G;
    pInfo->pAAOProcInfo.awb_gain.i4B  = m_strAAOProcInfo.awb_gain.i4B;
    AE_LOG_IF(m_3ALogEnable,"[%s] AE stat config, m_AEWinBlock: %d/%d/%d/%d, awb_gain: %d/%d/%d (%d)\n",
              __FUNCTION__, pInfo->m_AEWinBlock.u4XHi, pInfo->m_AEWinBlock.u4XLow, pInfo->m_AEWinBlock.u4YHi, pInfo->m_AEWinBlock.u4YLow,
              pInfo->pAAOProcInfo.awb_gain.i4R, pInfo->pAAOProcInfo.awb_gain.i4G, pInfo->pAAOProcInfo.awb_gain.i4B, pInfo->pAAOProcInfo.awb_gain_Unit);

    /* assign FD info */
    getFDInfo(&pInfo->m_eAEFDArea);
    pInfo->ZoomWinInfo.u4XWidth  = m_eZoomWinInfo.u4XWidth;
    pInfo->ZoomWinInfo.u4YHeight = m_eZoomWinInfo.u4YHeight;
    pInfo->ZoomWinInfo.u4XOffset = m_eZoomWinInfo.u4XOffset;
    pInfo->ZoomWinInfo.u4YOffset = m_eZoomWinInfo.u4YOffset;
    for(MINT8 j=0;j<3;j++){
        pInfo->GyroRot[j] = m_i4GyroInfo[j];
        pInfo->GyroAcc[j] = m_i4AcceInfo[j];
    }
    AE_LOG_IF(m_3ALogEnable,"[%s] ZoomWinInfo: %d/%d/%d/%d, Gyro:%d/%d/%d, Acc:%d/%d/%d\n", __FUNCTION__,
              pInfo->ZoomWinInfo.u4XWidth, pInfo->ZoomWinInfo.u4YHeight, pInfo->ZoomWinInfo.u4XOffset, pInfo->ZoomWinInfo.u4YOffset,
              pInfo->GyroRot[0], pInfo->GyroRot[1], pInfo->GyroRot[2], pInfo->GyroAcc[0], pInfo->GyroAcc[1], pInfo->GyroAcc[2]);

    /* assign touch info */
    memset(&pInfo->m_AETOUCHWinBlock,0,CCU_MAX_AE_METER_AREAS*sizeof(CCU_AE_TOUCH_BLOCK_WINDOW_T));
    pInfo->m_u4MeteringCnt =0;
    if(m_eAEMeterArea.u4Count!=0)
    {
        pInfo->m_u4MeteringCnt = m_eAEMeterArea.u4Count;
        for(MUINT8 i=0; i< m_eAEMeterArea.u4Count; i++) //copy touch info
        {
            pInfo->m_AETOUCHWinBlock[i].u4XLow   = m_eAEMeterArea.rAreas[i].i4Left;
            pInfo->m_AETOUCHWinBlock[i].u4XHi    = m_eAEMeterArea.rAreas[i].i4Right;
            pInfo->m_AETOUCHWinBlock[i].u4YLow   = m_eAEMeterArea.rAreas[i].i4Top;
            pInfo->m_AETOUCHWinBlock[i].u4YHi    = m_eAEMeterArea.rAreas[i].i4Bottom;
            pInfo->m_AETOUCHWinBlock[i].u4Weight = m_eAEMeterArea.rAreas[i].i4Weight;
            AE_LOG_IF(m_3ALogEnable,"[%s] m_AETOUCHWinBlock: %d/%d/%d/%d (%d)\n", __FUNCTION__,
              pInfo->m_AETOUCHWinBlock[i].u4XLow, pInfo->m_AETOUCHWinBlock[i].u4XHi, pInfo->m_AETOUCHWinBlock[i].u4YLow, pInfo->m_AETOUCHWinBlock[i].u4YHi, pInfo->m_AETOUCHWinBlock[i].u4Weight);
        }
    }

    /* assign manual target for dualcam calibration */
    checkADBAlgoCfg(&(pInfo->ae_adb_cfg));

    /* [LCE request: send backup] special case for backup FD infomation: backup FD information at AFAE state, and send backup FD information to algo at POSTCAP state */
    if (m_bMainFlashON && pInfo->AECtrl.eAEState == AE_STATE_POSTCAP) {
        AE_LOG("[%s] send backup FD information to algo at POSTCAP state (during main flash)\n", __FUNCTION__);
        memcpy(&pInfo->m_eAEFDArea, &m_eAEFDAreaBackup4Flash, sizeof(CCU_AEMeteringArea_T));
    }

    return S_AE_OK;
}

/* prepare on-change information for AE algo */
MRESULT AeMgr::prepareAlgoOnchInfo(AE_CORE_CTRL_RUN_TIME_INFO* pInfo)
{
    /* assign AE P-line table infomation */
    pInfo->u4NvramIdx         = m_u4AENVRAMIdx;
    pInfo->i4MaxBV            = m_pPreviewTableCurrent->i4MaxBV;
    pInfo->i4MinBV            = m_pPreviewTableCurrent->i4MinBV;
    pInfo->i4CapMaxBV         = m_pCaptureTable->i4MaxBV;
    pInfo->i4CapMinBV         = m_pCaptureTable->i4MinBV;
    pInfo->m_u4FinerEVIdxBase = m_u4FinerEVIdxBase;
    pInfo->u4TotalIndex       = m_pPreviewTableCurrent->u4TotalIndex;
    pInfo->u4CapTotalIndex    = m_pCaptureTable->u4TotalIndex;
    pInfo->eID                = (CCU_eAETableID)m_pPreviewTableCurrent->eID;
    pInfo->eCapID             = (CCU_eAETableID)m_pCaptureTable->eID;
    pInfo->pCurrentTable      = (CCU_strEvPline*)m_pPreviewTableCurrent->pCurrentTable;
    pInfo->pCurrentTableF     = (CCU_strFinerEvPline*)(&m_PreviewTableF);
    pInfo->pAeNVRAM           = (CCU_AE_NVRAM_T *)m_rAEInitInput.rAENVRAM;
	pInfo->pCusAEParam        = (AE_CUST_PARAM_T *)(&m_rAECustPARAM);
    // LTM paramter
    pInfo->LtmNvramParam.u4ClipThdPerc =  m_LTMParam.clipping_thd_perc;
    pInfo->LtmNvramParam.HighBoundPerc =  m_LTMParam.high_bound_perc;
    memcpy(&(pInfo->LtmNvramParam.MidPercLut),&(m_LTMParam.mid_perc_lut),sizeof(ltm_mid_perc_lut));
    memcpy(&(pInfo->LtmNvramParam.u4StrengthLtpMeanIdx),&(m_LTMParam.strength_ltp_mean_idx),sizeof(m_LTMParam.strength_ltp_mean_idx));
    memcpy(&(pInfo->LtmNvramParam.u4StrengthLtp),&(m_LTMParam.strength_ltp),sizeof(m_LTMParam.strength_ltp));
    memcpy(&(pInfo->LtmNvramParam.LocalGlobalBlending),&(m_LTMParam.local_global_blending),sizeof(ltm_local_global_blending));
    /* assign customized shutter/ISO priority param */
    memcpy(&(pInfo->PriorityInfo), &(m_AePriorityParam), sizeof(CCU_AE_PRIORITY_INFO));

    AE_LOG_IF(m_3ALogEnable,"[%s] u4NvramIdx: %d, m_pPreviewTableCurrent->eID: %d, i4MaxBV: %d, i4MinBV: %d, m_u4FinerEVIdxBase: %d\n",
              __FUNCTION__, pInfo->u4NvramIdx, pInfo->eID, pInfo->i4MaxBV, pInfo->i4MinBV, pInfo->m_u4FinerEVIdxBase);

    pInfo->u4IndexMax         = m_u4IndexMax;
    pInfo->u4IndexMin         = m_u4IndexMin;

    if(pInfo->PriorityInfo.eMode != CCU_AE_PRIORITY_OFF)
    AE_LOG_IF(m_3ALogEnable,"[%s] eMode: %d, u4FixShutter: %d, u4FixISO: %d, u4MaxShutter: %d, u4MaxISO: %d\n",
              __FUNCTION__, pInfo->PriorityInfo.eMode, pInfo->PriorityInfo.u4FixShutter, pInfo->PriorityInfo.u4FixISO, pInfo->PriorityInfo.u4MaxShutter, pInfo->PriorityInfo.u4MaxISO);

    return S_AE_OK;
}

/* copy AE algo result to AE mgr */
MRESULT AeMgr::copyAlgoResult2Mgr(AE_CORE_MAIN_OUT* pAlgoOutput)
{
    if (m_eAEMode != LIB3A_AE_MODE_OFF) {
        memcpy(&(m_rAEAlgoOutput), &(pAlgoOutput->RealOutput), sizeof(strAERealSetting));
        AE_LOG_IF(m_3ALogEnable,"[%s] AE algo output: AETargetMode(%d) Expo(%d/%d/%d) ISO(%d) u4Index/F/base(%d/%d/%d) FrameRate(%d)\n",
                  __FUNCTION__, m_rAEAlgoOutput.m_AETargetMode,
                  m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
                  m_rAEAlgoOutput.u4Index, m_rAEAlgoOutput.u4IndexF, m_rAEAlgoOutput.u4AEFinerEVIdxBase, m_rAEAlgoOutput.u2FrameRate);

        if (m_rAEAlgoOutput.m_AETargetMode != AE_MODE_NORMAL) {
            m_u4CurVHDRratio = m_rAEAlgoOutput.HdrEvSetting.u4AECHdrRatio;
            AE_LOG( "[%s] vHDR algo output: LE(%d,%d,%d) ME(%d,%d,%d) SE(%d,%d,%d) R(%d)\n", __FUNCTION__,
                    m_rAEAlgoOutput.HdrEvSetting.i4LEExpo, m_rAEAlgoOutput.HdrEvSetting.i4LEAfeGain, m_rAEAlgoOutput.HdrEvSetting.i4LEIspGain,
                    m_rAEAlgoOutput.HdrEvSetting.i4MEExpo, m_rAEAlgoOutput.HdrEvSetting.i4MEAfeGain, m_rAEAlgoOutput.HdrEvSetting.i4MEIspGain,
                    m_rAEAlgoOutput.HdrEvSetting.i4SEExpo, m_rAEAlgoOutput.HdrEvSetting.i4SEAfeGain, m_rAEAlgoOutput.HdrEvSetting.i4SEIspGain,
                    m_rAEAlgoOutput.HdrEvSetting.u4AECHdrRatio);
        }

        m_u4Index        = m_rAEAlgoOutput.u4Index;
        m_u4IndexF       = m_rAEAlgoOutput.u4IndexF;
        m_bAEStable      = pAlgoOutput->bAEStable;
    } else {
        m_bAEStable = MTRUE;
    }

    if (m_bAEStable && !(pAlgoOutput->bIsAFLock))
        m_u4StableYValue = m_u4CWVY;
    m_u4CWVY         = pAlgoOutput->u4CWValue;
    m_i4EVvalue      = pAlgoOutput->i4EV;
    m_i4BVvalue      = pAlgoOutput->i4Bv;
    if (m_bStrobeOn == MFALSE)
        m_i4BVvalueWOStrobe = pAlgoOutput->i4Bv;
    m_i4AOECompBVvalue = pAlgoOutput->i4AoeCompBv;
    m_bAlgoAPAELock  = pAlgoOutput->bIsAPLock;
    m_bAlgoAFAELock  = pAlgoOutput->bIsAFLock;
    AE_LOG_IF(m_3ALogEnable,"[%s] m_bAEStable: %d, m_bAlgoAPAELock/m_bAlgoAFAELock: %d/%d, CWVY: %d, m_u4StableYValue: %d, EV/BV/BVwoStrobe/AOEcompBV: %d/%d/%d/%d\n", __FUNCTION__,
              m_bAEStable, m_bAlgoAPAELock, m_bAlgoAFAELock, m_u4CWVY, m_u4StableYValue, m_i4EVvalue, m_i4BVvalue, m_i4BVvalueWOStrobe, m_i4AOECompBVvalue);

    m_bAlgoResultUpdate = MTRUE;
    return S_AE_OK;
}

/* get AE_INFO for ISP */
MRESULT AeMgr::getAEInfoForISP(AE_INFO_T &ae_info)
{
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)) {
        AE_LOG_IF(m_3ALogEnable,"[%s] from CCU AE\n", __FUNCTION__);
        m_pIAeFlowCCU->getAEInfoForISP(ae_info);
    }
    else {
        AE_LOG_IF(m_3ALogEnable,"[%s] from CPU AE\n", __FUNCTION__);
        m_pIAeFlowCPU->getAEInfoForISP(ae_info);
    }
    return S_AE_OK;
}

/* get AE algo EXIF debug information */
MRESULT AeMgr::getAlgoDebugInfo(MVOID* exif_info, MVOID* dbg_data_info)
{
    MBOOL isFromCCU = MFALSE;
    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT) {
        m_pIAeFlowCCU->getDebugInfo(exif_info, dbg_data_info);
        isFromCCU = MTRUE;
    }
    else {
        m_pIAeFlowCPU->getDebugInfo(exif_info, dbg_data_info);
    }

    if(exif_info == NULL)
        AE_LOG("[%s] exif_info is NULL\n", __FUNCTION__);
    else if (m_eAlgoType != E_AE_ALGO_CPU_CUSTOM) {
        AE_DEBUG_INFO_T* pexif_info = (AE_DEBUG_INFO_T*)exif_info;
        AE_LOG_IF(m_3ALogEnable,"[%s] isFromCCU: %d, AE_TAG_PRV_INDEXF: %d, AE_TAG_PRV_SHUTTER_TIME: %d, AE_TAG_PRV_SENSOR_GAIN: %d, AE_TAG_PRV_ISP_GAIN: %d, AE_TAG_PRV_ISO_REAL: %d, AE_TAG_CWV: %d, AE_TAG_FDY: %d\n",
                  __FUNCTION__, isFromCCU,
                  pexif_info->Tag[AE_TAG_PRV_INDEXF].u4FieldValue,
                  pexif_info->Tag[AE_TAG_PRV_SHUTTER_TIME].u4FieldValue,
                  pexif_info->Tag[AE_TAG_PRV_SENSOR_GAIN].u4FieldValue,
                  pexif_info->Tag[AE_TAG_PRV_ISP_GAIN].u4FieldValue,
                  pexif_info->Tag[AE_TAG_PRV_ISO_REAL].u4FieldValue,
                  pexif_info->Tag[AE_TAG_CWV].u4FieldValue,
                  pexif_info->Tag[AE_TAG_FDY].u4FieldValue);
    }

    return S_AE_OK;
}

/* get AE cycle count by scenerio */
MUINT32 AeMgr::getAECycleCnt()
{
    MUINT32 u4CycleCnt = 3;
    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT && m_eAETargetMode == AE_MODE_NORMAL && !m_bStereoManualPline) u4CycleCnt = 2;
//    if (m_bEnSWBuffMode) u4CycleCnt = 1;
    return u4CycleCnt;
}

/* get FD info */
MRESULT AeMgr::getFDInfo(CCU_AEMeteringArea_T* pFDInfo)
{
    memset(pFDInfo,0,sizeof(CCU_AEMeteringArea_T));
    pFDInfo->u4Count =0;
    if(m_eAEFDArea.u4Count !=0)
    {
        pFDInfo->u4Count = m_eAEFDArea.u4Count;

        for(MUINT8 i=0; i< m_eAEFDArea.u4Count; i++) //copy FD info
        {
            /* Structure need to be fixed, low->left... */
            pFDInfo->rAreas[i].i4Left      = m_eAEFDArea.rAreas[i].i4Left;
            pFDInfo->rAreas[i].i4Right     = m_eAEFDArea.rAreas[i].i4Right;
            pFDInfo->rAreas[i].i4Top       = m_eAEFDArea.rAreas[i].i4Top;
            pFDInfo->rAreas[i].i4Bottom    = m_eAEFDArea.rAreas[i].i4Bottom;
            pFDInfo->rAreas[i].i4Weight    = (MUINT16) m_eAEFDArea.rAreas[i].i4Weight;
            pFDInfo->rAreas[i].i4Id        = (MUINT16) m_eAEFDArea.rAreas[i].i4Id;
            pFDInfo->rAreas[i].i4Type      = (MUINT8) m_eAEFDArea.rAreas[i].i4Type; // 0:GFD, 1:LFD, 2:OT //change type
            pFDInfo->rAreas[i].i4ROP       = (MUINT8) m_eAEFDArea.rAreas[i].i4ROP;
            pFDInfo->rAreas[i].i4Motion[0] = (MINT8) m_eAEFDArea.rAreas[i].i4Motion[0];
            pFDInfo->rAreas[i].i4Motion[1] = (MINT8) m_eAEFDArea.rAreas[i].i4Motion[1];

            pFDInfo->rAreas[i].i4Landmark[0][0] = m_eAEFDArea.rAreas[i].i4Landmark[0][0];
            pFDInfo->rAreas[i].i4Landmark[1][0] = m_eAEFDArea.rAreas[i].i4Landmark[1][0];
            pFDInfo->rAreas[i].i4Landmark[2][0] = m_eAEFDArea.rAreas[i].i4Landmark[2][0];
            pFDInfo->rAreas[i].i4Landmark[0][1] = m_eAEFDArea.rAreas[i].i4Landmark[0][1];
            pFDInfo->rAreas[i].i4Landmark[1][1] = m_eAEFDArea.rAreas[i].i4Landmark[1][1];
            pFDInfo->rAreas[i].i4Landmark[2][1] = m_eAEFDArea.rAreas[i].i4Landmark[2][1];
            pFDInfo->rAreas[i].i4Landmark[0][2] = m_eAEFDArea.rAreas[i].i4Landmark[0][2];
            pFDInfo->rAreas[i].i4Landmark[1][2] = m_eAEFDArea.rAreas[i].i4Landmark[1][2];
            pFDInfo->rAreas[i].i4Landmark[2][2] = m_eAEFDArea.rAreas[i].i4Landmark[2][2];
            pFDInfo->rAreas[i].i4Landmark[0][3] = m_eAEFDArea.rAreas[i].i4Landmark[0][3];
            pFDInfo->rAreas[i].i4Landmark[1][3] = m_eAEFDArea.rAreas[i].i4Landmark[1][3];
            pFDInfo->rAreas[i].i4Landmark[2][3] = m_eAEFDArea.rAreas[i].i4Landmark[2][3];
            pFDInfo->rAreas[i].i4LandmarkCV     = m_eAEFDArea.rAreas[i].i4LandmarkCV;
            pFDInfo->rAreas[i].i4LandMarkRip    = m_eAEFDArea.rAreas[i].i4LandMarkRip;
            pFDInfo->rAreas[i].i4LandMarkRop    = m_eAEFDArea.rAreas[i].i4LandMarkRop;

            AE_LOG_IF(m_3ALogEnable,"[%s] AEFDArea: %d/%d/%d/%d (%d)\n", __FUNCTION__,
              pFDInfo->rAreas[i].i4Left, pFDInfo->rAreas[i].i4Right, pFDInfo->rAreas[i].i4Top, pFDInfo->rAreas[i].i4Bottom, pFDInfo->rAreas[i].i4Weight);
        }
    }
    return S_AE_OK;
}

/* remapping AE_MGR_STATE to AE_ALGO_STATE */
MUINT32 AeMgr::getAEAlgoState()
{
    MUINT32 u4AEAlgoState = AE_STATE_PREVIEW;
    MUINT32 u4AEMgrState;

    if(m_eAlgoType == E_AE_ALGO_CCU_DEFAULT)
        u4AEMgrState = m_u4AEMgrStateCCU;
    else
        u4AEMgrState = m_u4AEMgrStateCPU;

    if(u4AEMgrState == AE_MGR_STATE_DOAFAE)
        u4AEAlgoState = AE_STATE_AFAE;
    else if(u4AEMgrState == AE_MGR_STATE_DOPRECAPAE)
        u4AEAlgoState = AE_STATE_PRECAP;
    else if(u4AEMgrState == AE_MGR_STATE_DOCAPAE || u4AEMgrState == AE_MGR_STATE_DOPOSTCAPAE)
        u4AEAlgoState = AE_STATE_POSTCAP;
    else
        u4AEAlgoState = AE_STATE_PREVIEW;

    return u4AEAlgoState;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                           CPU flow implement                               //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/* CPU AE: prepare CPU initialization informatiion */
MRESULT AeMgr::prepareCPUInitInfo(AE_FLOW_CPU_INIT_INFO_T* pInfo, E_AE_ALGO_TYPE_T eCPUAlgoType)
{
    pInfo->mCPUAlgoType = eCPUAlgoType;
    pInfo->i4SensorIdx = m_i4SensorIdx;
    prepareAlgoInitInfo(&(pInfo->mInitData));
    return S_AE_OK;
}

/* CPU AE: initialize CPU AE */
MRESULT AeMgr::initializeCPUAE(E_AE_ALGO_TYPE_T eCPUAlgoType)
{
    AE_LOG("[%s +] m_eSensorDev: %d, eCPUAlgoType: %d\n", __FUNCTION__, m_eSensorDev, eCPUAlgoType);
    AE_FLOW_CPU_INIT_INFO_T rCPUInitInfo;
    rCPUInitInfo.bIsCCUAEInit = m_bIsCCUAEInit;
    prepareCPUInitInfo(&rCPUInitInfo, eCPUAlgoType);
    m_pIAeFlowCPU->start((MVOID*) &rCPUInitInfo);
    AE_LOG("[%s -]\n", __FUNCTION__);
    return S_AE_OK;
}

/* CPU AE: prepare CPU calculate infomation */
MRESULT AeMgr::prepareCPUCalculateInfo(AE_FLOW_CPU_CALCULATE_INFO_T* pInfo)
{
    /* update frame sync information */
    {
        UpdateGyroInfo();
        prepareAlgoFrameInfo(&(pInfo->mVsyncInfo));
        m_pIAeFlowCPU->controltrigger((MUINT32) E_AE_FLOW_CPU_TRIGGER_FRAME_INFO_UPDATE);
    }
    /* update onchange information */
    if(m_bNeedCheckAEPline) {
        checkAEPline(); // check if AE Pline needs interpolation
        prepareAlgoOnchInfo(&(pInfo->mRunTimeInfo));
        m_pIAeFlowCPU->controltrigger((MUINT32) E_AE_FLOW_CPU_TRIGGER_ONCH_INFO_UPDATE);
    }

    pInfo->mAEMainIn.pAEBuffer = (MUINT8*)m_pAAOStatBuf;
    pInfo->mAEMainIn.pHistBuffer = (MUINT8*)m_pAAOStatBuf + (2880*90) ;
    pInfo->mAEMainIn.CrntBankIdx = 0;
    pInfo->mAEMainIn.YLineNum = 90;
    pInfo->mAEMainIn.YOffset = 0;

    pInfo->mAEMainIn.MHDRStatsData.a_pMHDRSensorStatsBuffer = (MUINT32*) m_pMVHDR3ExpoStatBuf;
    pInfo->mAEMainIn.u4CurHDRRatio = m_u4MVHDRRatio_x100;

    AE_LOG_IF(m_3ALogEnable,"[%s] m_eSensorDev: %d, m_i4ShutterDelayFrames/m_i4SensorGainDelayFrames/m_i4IspGainDelayFrames : %d/%d/%d", __FUNCTION__,
              m_eSensorDev, m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames);

    return S_AE_OK;
}

/* CPU AE: calculate CPU AE */
MRESULT AeMgr::calculateCPUAE()
{
    AE_FLOW_CPU_CALCULATE_INFO_T rCPUCalInput;
    AE_CORE_MAIN_OUT rCPUCalOutput;
    memset(&rCPUCalOutput, 0, sizeof(AE_CORE_MAIN_OUT));
    checkADBCmdCtrl();
    prepareCPUCalculateInfo(&rCPUCalInput);
    if (m_pIAeFlowCPU->calculateAE((MVOID*) &rCPUCalInput, (MVOID*) &rCPUCalOutput)) {
        copyAlgoResult2Mgr(&rCPUCalOutput);
        if (m_bAdbAEPreviewUpdate)
            memcpy(&(m_rAESettingPreview), &(m_rADBAESetting), sizeof(strAERealSetting));
        else
            memcpy(&(m_rAESettingPreview), &(m_rAEAlgoOutput), sizeof(strAERealSetting));
        getAEInfoForISP(m_rAEInfoPreview);
    }
    else
        AE_LOG( "[%s()] SensorDev(%d) can't get AE algo result", __FUNCTION__, m_eSensorDev);
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                           CCU flow implement                               //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/* CCU AE: initialize CCU AE */
MRESULT AeMgr::initializeCCU(AE_FLOW_CCU_INIT_INFO_T* pInfo)
{
    AE_LOG("[%s() +] m_eSensorDev: %d, m_bIsCCUAEInit: %d\n", __FUNCTION__, m_eSensorDev, m_bIsCCUAEInit);

    pInfo->pAEInitInput = &m_rAEInitInput;
    pInfo->i4SensorIdx = m_i4SensorIdx;
    pInfo->eSensorMode = m_eSensorMode;
    pInfo->u4AAOmode = m_rAEConfigInfo.u4AAOmode;
    pInfo->u4Index = m_u4Index;
    pInfo->u4IndexF = m_u4IndexF;
    pInfo->u4Exp = m_rAESettingPreview.EvSetting.u4Eposuretime;
    pInfo->u4Afe = m_rAESettingPreview.EvSetting.u4AfeGain;
    pInfo->u4Isp = m_rAESettingPreview.EvSetting.u4IspGain;
    pInfo->u4RealISO = m_rAESettingPreview.u4ISO;
    pInfo->bIsCCUAEInit = m_bIsCCUAEInit;
    pInfo->eAETargetMode = (CCU_AE_TargetMODE)m_eAETargetMode;

    if(!m_bIsCCUStart)
    {
        m_i4SensorIdx = pInfo->i4SensorIdx;
        m_eSensorMode = pInfo->eSensorMode;
        m_pICcuAe = ICcuCtrlAe::getInstance(m_i4SensorIdx, m_eSensorDev);
        AE_LOG_IF(m_3ALogEnable,"[%s()] %d/%d/%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_i4SensorIdx, m_eSensorMode);

        SENSOR_INFO_IN_T rSensorinfoInput;
        SENSOR_INFO_OUT_T rSensorinfoOutput;
        CCU_AE_INITI_PARAM_T rAEInitparam;
        CCU_AE_CONTROL_CFG_T rAEControlcfg;
        memset(&rSensorinfoInput, 0, sizeof(SENSOR_INFO_IN_T));
        memset(&rSensorinfoOutput, 0, sizeof(SENSOR_INFO_OUT_T));
        memset(&rAEControlcfg, 0, sizeof(CCU_AE_CONTROL_CFG_T));

        // init CCU
        AE_LOG_IF(m_3ALogEnable,"[%s()] CCU initial start, sensor id: %x\n", __FUNCTION__, getsensorDevID(m_eSensorDev));
        if(m_pICcuAe->init(m_i4SensorIdx, m_eSensorDev) != CCU_CTRL_SUCCEED)
        {
            AE_LOG("[%s()] m_pICcuAe->init() fail", __FUNCTION__);
            m_bCCUIsSensorSupported = MFALSE;
            return E_AE_CCU_CONTROL_INIT_FAIL;
        }

        if (m_eSensorDev & ESensorDev_Main){
            rSensorinfoInput.eWhichSensor = WHICH_SENSOR_MAIN;
        }else if (m_eSensorDev & ESensorDev_Sub){
            rSensorinfoInput.eWhichSensor = WHICH_SENSOR_SUB;
        }else if (m_eSensorDev & ESensorDev_MainSecond){
            rSensorinfoInput.eWhichSensor = WHICH_SENSOR_MAIN_2;
        }else{
            AE_LOG("[%s()] CCU sensor start failed, m_eSensorDev = %d\n", __FUNCTION__, m_eSensorDev);
            //m_pICcu->uninit();
            m_bCCUIsSensorSupported = MFALSE;
            return E_AE_CCU_CONTROL_INIT_FAIL;
        }
        AE_LOG("[%s()] CCU Sensor initial done\n", __FUNCTION__);


        // start Sensor and I2C
        //rSensorinfoInput.u32SensorId = (MUINT32)m_eSensorDev;
        //rSensorinfoInput.u32SensorId = getSensorId(m_i4SensorIdx, m_eSensorDev);

        rSensorinfoInput.eScenario = static_cast<MUINT32>(m_eSensorMode);
        AE_LOG("[%s()] rSensorinfoInput.eScenario : %d", __FUNCTION__,rSensorinfoInput.eScenario);
        //AE_LOG("rSensorinfoInput.eWhichSensor : %d",rSensorinfoInput.eWhichSensor);
        //AE_LOG("rSensorinfoInput.pu8BufferVAddr : %d",rSensorinfoInput.pu8BufferVAddr);
        //AE_LOG("rSensorinfoInput.sensorI2cSlaveAddr : %d",rSensorinfoInput.sensorI2cSlaveAddr);
        //AE_LOG("rSensorinfoInput.u16BufferLen : %d",rSensorinfoInput.u16BufferLen);
        //AE_LOG("rSensorinfoInput.u16FPS : %d",rSensorinfoInput.u16FPS);


        if ( CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_SENSOR_INIT, &rSensorinfoInput, &rSensorinfoOutput))
        {
            AE_LOG("[%s()] ccu_sensor_initialize fail\n", __FUNCTION__);
            m_bCCUIsSensorSupported = MFALSE;
            return E_AE_CCU_CONTROL_INIT_FAIL;
        }
        else
        {
            AE_LOG("[%s()] ccu_sensor_initialize success\n", __FUNCTION__);
            if(!rSensorinfoOutput.u8SupportedByCCU)
            {
                AE_LOG("[%s()] this sensor is not supported by CCU\n", __FUNCTION__);
                m_bCCUIsSensorSupported = MFALSE;
                return E_AE_CCU_CONTROL_INIT_FAIL;
            }
        }
        // init ccu ae
        rAEControlcfg.hdr_stat_en = (MBOOL)(pInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.bEnableHDRLSB>0);
        rAEControlcfg.ae_overcnt_en = pInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.bEnableAEOVERCNTconfig;
        rAEControlcfg.tsf_stat_en = pInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.bEnableTSFSTATconfig;
        rAEControlcfg.bits_num = (pInfo->u4AAOmode == 1)? 14:12;
        rAEControlcfg.ae_footprint_hum = pInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
        rAEControlcfg.ae_footprint_vum = pInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        memcpy(&(rAEControlcfg.exp_on_start), &(m_rAESettingPreview), sizeof(strAERealSetting));
        rAEControlcfg.aao_line_width = 2880;

        // custom statistic config
        rAEControlcfg.ae_cust_stat_enable= property_get_int32("vendor.debug.ae.dump.stat", 0);
        rAEControlcfg.ae_cust_stat_width = 64;
        rAEControlcfg.ae_cust_stat_height = 48;
        rAEControlcfg.bin_sum_ratio = m_u4BinSumRatio;
        rAEControlcfg.ae_target_mode = pInfo->eAETargetMode;
        rAEInitparam.control_cfg = rAEControlcfg;

        AE_LOG("[%s()] CCU controlcfg: stat_en(%x/%x/%x), footprint(%x/%x), target_mode(%x), bin_ratio(%x), Index/F(%x/%x), expo(%x/%x/%x,%x), cust_stat(%d/%d/%d)",
               __FUNCTION__, rAEControlcfg.hdr_stat_en, rAEControlcfg.ae_overcnt_en, rAEControlcfg.tsf_stat_en,
               rAEControlcfg.ae_footprint_hum, rAEControlcfg.ae_footprint_vum, rAEControlcfg.ae_target_mode, rAEControlcfg.bin_sum_ratio,
               rAEControlcfg.exp_on_start.u4Index, rAEControlcfg.exp_on_start.u4IndexF,
               rAEControlcfg.exp_on_start.EvSetting.u4Eposuretime, rAEControlcfg.exp_on_start.EvSetting.u4AfeGain, rAEControlcfg.exp_on_start.EvSetting.u4IspGain, rAEControlcfg.exp_on_start.u4ISO,
               rAEControlcfg.ae_cust_stat_enable, rAEControlcfg.ae_cust_stat_width, rAEControlcfg.ae_cust_stat_height);

        //init CCU AE control first
        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_INIT, &rAEInitparam, NULL))
        {
            AE_LOG("[%s()] ccu_ae_ctrl_initialize fail\n", __FUNCTION__);
            m_bCCUIsSensorSupported = MFALSE;
            return E_AE_CCU_CONTROL_INIT_FAIL;
        }
        AE_LOG("[%s()] ccu_ae_ctrl_initialize success\n", __FUNCTION__);

        // switch flag after config done
        m_bIsCCUStart = MTRUE;
        m_bCCUIsSensorSupported = MTRUE;
    }

    AE_LOG("[%s() -] m_eSensorDev: %d, m_bIsCCUAEInit: %d\n", __FUNCTION__, m_eSensorDev, m_bIsCCUAEInit);
    return S_AE_OK;
}

MRESULT AeMgr::uninitializeCCU()
{
    m_bIsCCUStart = MFALSE;
    m_bCCUIsSensorSupported = MFALSE;
    AE_LOG("[%s()] m_bIsCCUStart: %d, m_bCCUIsSensorSupported: %d\n", __FUNCTION__, m_bIsCCUStart, m_bCCUIsSensorSupported);
    return S_AE_OK;
}


/* CCU AE: prepare CCU initialization informatiion */
MRESULT AeMgr::prepareCCUInitInfo(AE_FLOW_CCU_INIT_INFO_T* pInfo)
{
    prepareAlgoInitInfo(&(pInfo->mInitData));
    return S_AE_OK;
}

/* CCU AE: initialize CCU AE */
MRESULT AeMgr::initializeCCUAE(AE_FLOW_CCU_INIT_INFO_T* pInfo)
{
    AE_LOG("[%s +] m_eSensorDev: %d\n", __FUNCTION__, m_eSensorDev);
    prepareCCUInitInfo(pInfo);
    m_pIAeFlowCCU->start((MVOID*) pInfo);
    AE_LOG("[%s -]\n", __FUNCTION__);
    return S_AE_OK;
}

/* CCU AE: prepare CCU control infomation */
MRESULT AeMgr::prepareCCUControlInfo(AE_FLOW_CCU_CONTROL_INFO_T* pInfo)
{
    /* update frame sync information */
    {
        UpdateGyroInfo();
        prepareAlgoFrameInfo(&(pInfo->mFrameData));
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_FRAME_INFO_UPDATE);
    }
    /* update onchange information */
    if(m_bNeedCheckAEPline) {
        checkAEPline(); // check if AE Pline needs interpolation
        prepareAlgoOnchInfo(&(pInfo->mOnchData));
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_ONCH_INFO_UPDATE);
    }

    pInfo->u4HwMagicNum = m_u4HwMagicNum;
    pInfo->u4RequestNum = m_u4RequestNum;
    AE_LOG_IF(m_3ALogEnable,"[%s] m_eSensorDev: %d, u4HwMagicNum/u4RequestNum: %d/%d", __FUNCTION__, m_eSensorDev, pInfo->u4HwMagicNum, pInfo->u4RequestNum);

    pInfo->bForceResetCCUStable = m_bForceResetCCUStable;
	if(m_bForceResetCCUStable) {
      m_bForceResetCCUStable = MFALSE;
    }
    AE_LOG_IF(pInfo->bForceResetCCUStable,"[%s] m_eSensorDev: %d, force reset CCU AE stable of next frame", __FUNCTION__, m_eSensorDev);

    return S_AE_OK;
}

/* CCU AE: control CCU AE */
MRESULT AeMgr::controlCCUAE()
{
    std::lock_guard<std::mutex> lock(m_LockControlCCU);
    AE_FLOW_CCU_CONTROL_INFO_T rCCUControlInput;
    checkADBCmdCtrl();
    prepareCCUControlInfo(&rCCUControlInput);
    m_pIAeFlowCCU->controlCCU(&rCCUControlInput);
    return S_AE_OK;
}

/* CCU AE: get CCU AE result */
MRESULT AeMgr::getCCUResult()
{
    std::lock_guard<std::mutex> lock(m_LockControlCCU);
    ccu_ae_output rCCUControlOutput;
    memset(&rCCUControlOutput, 0, sizeof(ccu_ae_output));
    m_pIAeFlowCCU->getCCUResult((MVOID *) &rCCUControlOutput);
    if(m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET)) {
        copyAlgoResult2Mgr(&(rCCUControlOutput.algo_output));
        if (m_bAdbAEPreviewUpdate)
            memcpy(&(m_rAESettingPreview), &(m_rADBAESetting), sizeof(strAERealSetting));
        else
            memcpy(&(m_rAESettingPreview), &(rCCUControlOutput.real_setting), sizeof(strAERealSetting));
        getAEInfoForISP(m_rAEInfoPreview);
    }
    else
        AE_LOG( "[%s()] SensorDev(%d) can't get AE algo result", __FUNCTION__, m_eSensorDev);
    return S_AE_OK;
}

/* CCU AE: reset CCU AE stable next frame */
MRESULT AeMgr::resetCCUStable()
{
    if (m_bIsCCUStart) {
        m_bForceResetCCUStable = MTRUE;
        m_bAEStable = MFALSE;
        AE_LOG( "[%s()] reset CCU AE stable next frame, m_bAEStable = MFALSE \n", __FUNCTION__);
    }
    return S_AE_OK;
}

/* CCU AE: disable manual CCU exposure setting */
MRESULT AeMgr::disableManualCCU(MBOOL bPresetCtrl)
{
    if (m_bIsCCUStart && !m_bAdbAEPreviewUpdate) {
        ccu_manual_exp_disable_info rCcuManualExpoDisableInfo;
        rCcuManualExpoDisableInfo.timing = bPresetCtrl ? CCU_MANUAL_EXP_TIMING_PRESET : CCU_MANUAL_EXP_TIMING_SET;
        m_pICcuAe->ccuControl(MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE, &rCcuManualExpoDisableInfo, NULL);
        AE_LOG( "[%s()] disable CCU manual control, bPresetCtrl = %d \n", __FUNCTION__, bPresetCtrl);
    }
    return S_AE_OK;
}

MRESULT AeMgr::IsCCUAEInit(MBOOL bInit)
{
  m_bIsCCUAEInit = bInit;
  return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                     verification flow implement                            //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/* check if controlled by ADB cmd */
MRESULT AeMgr::checkADBCmdCtrl()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae_mgr.lock", value, "0");
    m_bAdbAELock = atoi(value);
    property_get("vendor.debug.ae_mgr.preview.update", value, "0");
    m_bAdbAEPreviewUpdate = atoi(value);
    property_get("vendor.debug.ae_mgr.ISOspeed", value, "0");
    m_u4AdbISOspeed = atoi(value);

    if (m_bAdbAELock) {
        AE_LOG( "[%s()] ADB AE lock, i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
    }
    if (m_bAdbAEPreviewUpdate) {
        { // if (m_eAETargetMode == AE_MODE_NORMAL)
            property_get("vendor.debug.ae_mgr.shutter",    value, "30000");
            m_rADBAESetting.EvSetting.u4Eposuretime = atoi(value);
            property_get("vendor.debug.ae_mgr.sensorgain", value, "1024");
            m_rADBAESetting.EvSetting.u4AfeGain = atoi(value);
            property_get("vendor.debug.ae_mgr.ispgain",    value, "4096");
            m_rADBAESetting.EvSetting.u4IspGain = atoi(value);
            verifyPerframeCtrl();

            MUINT32 u4FinalGain;
            MUINT32 u4PreviewBaseISO = 100;
            if(m_p3ANVRAM != NULL) {
                u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
            } else {
                AE_LOG( "[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
            }
            u4FinalGain = ( m_rADBAESetting.EvSetting.u4AfeGain * m_rADBAESetting.EvSetting.u4IspGain)/AE_GAIN_BASE_ISP;
            m_rADBAESetting.u4ISO = u4PreviewBaseISO*u4FinalGain/AE_GAIN_BASE_AFE;

            AE_LOG( "[%s()] ADB AE preview update, i4SensorDev: %d, m_eAETargetMode: %d, Apply: %d/%d/%d (%d)\n",
                     __FUNCTION__, m_eSensorDev, m_eAETargetMode,
                     m_rADBAESetting.EvSetting.u4Eposuretime, m_rADBAESetting.EvSetting.u4AfeGain, m_rADBAESetting.EvSetting.u4IspGain, m_rADBAESetting.u4ISO);
        }

        if (m_eAETargetMode != AE_MODE_NORMAL) {
            /* ADB shutter */
            property_get("vendor.debug.ae_mgr.shutter_le",  value, "30000");
            m_rADBAESetting.HdrEvSetting.i4LEExpo = atoi(value);
            property_get("vendor.debug.ae_mgr.shutter_me",  value, "30000");
            m_rADBAESetting.HdrEvSetting.i4MEExpo = atoi(value);
            property_get("vendor.debug.ae_mgr.shutter_se",  value, "30000");
            m_rADBAESetting.HdrEvSetting.i4SEExpo = atoi(value);
            /* ADB sensor gain */
            property_get("vendor.debug.ae_mgr.sensorgain_le",  value, "1024");
            m_rADBAESetting.HdrEvSetting.i4LEAfeGain = atoi(value);
            property_get("vendor.debug.ae_mgr.sensorgain_me",  value, "1024");
            m_rADBAESetting.HdrEvSetting.i4MEAfeGain = atoi(value);
            property_get("vendor.debug.ae_mgr.sensorgain_se",  value, "1024");
            m_rADBAESetting.HdrEvSetting.i4SEAfeGain = atoi(value);
            /* ADB ISP gain */
            property_get("vendor.debug.ae_mgr.ispgain", value, "4096");
            m_rADBAESetting.HdrEvSetting.i4LEIspGain =
            m_rADBAESetting.HdrEvSetting.i4MEIspGain =
            m_rADBAESetting.HdrEvSetting.i4SEIspGain = atoi(value);
            AE_LOG( "[%s()] ADB AE preview update, i4SensorDev:%d, m_eAETargetMode: %d, Apply LE:%d/%d/%d ME:%d/%d/%d SE:%d/%d/%d \n",
                     __FUNCTION__, m_eSensorDev, m_eAETargetMode,
                     m_rADBAESetting.HdrEvSetting.i4LEExpo,  m_rADBAESetting.HdrEvSetting.i4LEAfeGain,  m_rADBAESetting.HdrEvSetting.i4LEIspGain,
                     m_rADBAESetting.HdrEvSetting.i4MEExpo,  m_rADBAESetting.HdrEvSetting.i4MEAfeGain,  m_rADBAESetting.HdrEvSetting.i4MEIspGain,
                     m_rADBAESetting.HdrEvSetting.i4SEExpo,  m_rADBAESetting.HdrEvSetting.i4SEAfeGain,  m_rADBAESetting.HdrEvSetting.i4SEIspGain);
        }
    }
    if(m_u4AdbISOspeed >0){/*ADB to set specified ISO speed*/
        AE_LOG("[%s()]ADB ISO speed, i4SensorDev:%d, u4ISOspeed:%d\n",__FUNCTION__, m_eSensorDev, m_u4AdbISOspeed);
        sendAECtrl(EAECtrl_EnableShutterISOPriority, MTRUE, -1, m_u4AdbISOspeed, NULL);
    }
    return S_AE_OK;
}

/* perframe exposure control verification */
MRESULT AeMgr::verifyPerframeCtrl()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae_mgr.perframetest.enable", value, "0");
    MUINT32 u4AdbVerificationCase = atoi(value);

    if (u4AdbVerificationCase == 0)
        return S_AE_OK;

    MUINT32 u4TestStep = 10;
    if(m_u4IndexTestCnt < 300) {
        switch(u4AdbVerificationCase) {
            case 1: // Shutter delay frames [Exp:10ms <-> 20ms, 1x ]
                if( m_u4IndexTestCnt%20 < u4TestStep ) {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE;
                } else {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000*2;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE;
                }
                break;
            case 2: // Gain delay frames [Gain:2x <-> 4x , 10ms ]
                if( m_u4IndexTestCnt%20 < u4TestStep ) {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*2;
                } else {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*4;
                }
                break;
            case 3: // Delay frames IT [20ms/2x <-> 10ms/4x ]
                if( m_u4IndexTestCnt%20 < u4TestStep ) { // 0-9 20ms/1x
                    m_rADBAESetting.EvSetting.u4Eposuretime = 20000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*2;
                } else {                        //10-19 10ms/2x
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*4;
                }
                break;
            case 4: // Perframe shutter command [10ms<->20ms,1x]
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000*(m_u4IndexTestCnt%2 + 1);
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE;
                break;
            case 5: // Perframe gain command [2x<->4x,10ms]
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*2*(m_u4IndexTestCnt%2 + 1);
                break;
            case 6: // Perframe command IT [10ms/6x<->30ms/2x]
                if(m_u4IndexTestCnt%2 == 0) {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*6;
                } else if(m_u4IndexTestCnt%2 == 1) {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000*3;
                     m_rADBAESetting.EvSetting.u4AfeGain    = AE_GAIN_BASE_AFE*2;
                }
                break;
            case 7: // Perframe framerate command [10ms/5x <-> 50ms/1x]
                if(m_u4IndexTestCnt%2 == 0) {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*5;
                } else if(m_u4IndexTestCnt%2 == 1) {
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000*5;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE;
                }
                break;
            case 8: // Shutter Linearity
                    m_rADBAESetting.EvSetting.u4Eposuretime = 5000*((m_u4IndexTestCnt%160)/u4TestStep + 1);
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE;
                break;
            case 9: // Gain Linearity
                    m_rADBAESetting.EvSetting.u4Eposuretime = 10000;
                    m_rADBAESetting.EvSetting.u4AfeGain     = AE_GAIN_BASE_AFE*((m_u4IndexTestCnt%160)/u4TestStep + 1);
                break;
            default:
                AE_LOG( "[%s()] Test item:%d isn't supported \n", __FUNCTION__, u4AdbVerificationCase);
                break;
        }
        m_rADBAESetting.EvSetting.u4IspGain = AE_GAIN_BASE_ISP;
        m_u4IndexTestCnt++;
    } else {
        m_rADBAESetting.EvSetting.u4Eposuretime = 1000;
        m_rADBAESetting.EvSetting.u4AfeGain = AE_GAIN_BASE_AFE;
        m_rADBAESetting.EvSetting.u4IspGain = AE_GAIN_BASE_ISP;
    }
    AE_LOG( "[%s()] i4SensorDev: %d, u4AdbVerificationCase: %d, m_u4IndexTestCnt: %d, test exposure setting: %d/%d/%d \n",
             __FUNCTION__, m_eSensorDev, u4AdbVerificationCase, m_u4IndexTestCnt,
             m_rADBAESetting.EvSetting.u4Eposuretime, m_rADBAESetting.EvSetting.u4AfeGain, m_rADBAESetting.EvSetting.u4IspGain);
    return S_AE_OK;
}

/* check if controlled by ADB cmd */
MRESULT AeMgr::checkADBAlgoCfg(AE_ADB_CFG *pInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae.manual.target_en", value, "0");
    pInfo->bEableManualTarget = atoi(value);
    property_get("vendor.debug.ae.manual.target", value, "47");
    pInfo->u4ManualTarget = atoi(value);
    property_get("vendor.debug.ae.calib_en", value, "0");
    pInfo->bEableCalib = atoi(value);
    property_get("vendor.debug.ae.rtt.enable", value, "0");
    pInfo->bEnableRTT = atoi(value);
    if (pInfo->bEnableRTT)
    {
        MUINT32 u4Cnt = 0;
        MUINT32 u4Cmd = 0;
        MUINT32 u4Val = 0;

        property_get("vendor.debug.ae.rtt.cnt", value, "0");
        u4Cnt = atoi(value);

        pInfo->u4RTTCnt = (u4Cnt > AE_ADB_RTT_TOTAL_CNT) ? AE_ADB_RTT_TOTAL_CNT : u4Cnt;
        for(MUINT8 i=0; i< u4Cnt; i++) //copy FD info
        {
            switch(i)
            {
            case 0:
                property_get("vendor.debug.ae.rtt.cmd.0", value, "0");
                u4Cmd = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                property_get("vendor.debug.ae.rtt.value.0", value, "0");
                u4Val = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                break;
            case 1:
                property_get("vendor.debug.ae.rtt.cmd.1", value, "0");
                u4Cmd = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                property_get("vendor.debug.ae.rtt.value.1", value, "0");
                u4Val = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                break;
            case 2:
                property_get("vendor.debug.ae.rtt.cmd.2", value, "0");
                u4Cmd = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                property_get("vendor.debug.ae.rtt.value.2", value, "0");
                u4Val = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                break;
            case 3:
                property_get("vendor.debug.ae.rtt.cmd.3", value, "0");
                u4Cmd = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                property_get("vendor.debug.ae.rtt.value.3", value, "0");
                u4Val = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                break;
            case 4:
                property_get("vendor.debug.ae.rtt.cmd.4", value, "0");
                u4Cmd = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                property_get("vendor.debug.ae.rtt.value.4", value, "0");
                u4Val = (value[0] == '0' && value[1] == 'x') ? strtol(value, NULL, 0) : atoi(value);
                break;
            default:
                AE_LOG( "[%s()] RTT item:%d isn't supported \n", __FUNCTION__, i);
                break;
            }

            pInfo->RTTList[i].u4Cmd = u4Cmd;
            pInfo->RTTList[i].u4Value = u4Val;
        }
    }

    if(pInfo->bEableManualTarget || pInfo->bEableCalib)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s] bEableManualTarget: %d, u4ManualTarget: %d, bEableCalib: %d\n",
                  __FUNCTION__, pInfo->bEableManualTarget, pInfo->u4ManualTarget, pInfo->bEableCalib);
    }

    if(pInfo->bEnableRTT)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s] bEnableRTT: %d, u4RTTCnt: %d, u4Cmd_0: 0x%08x, u4Value_0: %d\n",
                  __FUNCTION__, pInfo->bEnableRTT, pInfo->u4RTTCnt, pInfo->RTTList[0].u4Cmd, pInfo->RTTList[0].u4Value);
    }
    return S_AE_OK;
}


// temp: to be removed
MUINT32 Log2x1000[LumLog2x1000_TABLE_SIZE] = { // temp put in here, need to move to AE calculation library in P80
    0,    0,   1000,    1585,    2000,    2322,    2585,    2807,
    3000,   3170,   3322,   3459,   3585,   3700,   3807,   3907,
    4000,   4087,   4170,   4248,   4322,   4392,   4459,   4524,
    4585,   4644,   4700,   4755,   4807,   4858,   4907,   4954,
    5000,   5044,   5087,   5129,   5170,   5209,   5248,   5285,
    5322,   5358,   5392,   5426,   5459,   5492,   5524,   5555,
    5585,   5615,   5644,   5672,   5700,   5728,   5755,   5781,
    5807,   5833,   5858,   5883,   5907,   5931,   5954,   5977,
    6000,   6022,   6044,   6066,   6087,   6109,   6129,   6150,
    6170,   6190,   6209,   6229,   6248,   6267,   6285,   6304,
    6322,   6340,   6358,   6375,   6392,   6409,   6426,   6443,
    6459,   6476,   6492,   6508,   6524,   6539,   6555,   6570,
    6585,   6600,   6615,   6629,   6644,   6658,   6672,   6687,
    6700,   6714,   6728,   6741,   6755,   6768,   6781,   6794,
    6807,   6820,   6833,   6845,   6858,   6870,   6883,   6895,
    6907,   6919,   6931,   6943,   6954,   6966,   6977,   6989,
    7000,   7011,   7022,   7033,   7044,   7055,   7066,   7077,
    7087,   7098,   7109,   7119,   7129,   7140,   7150,   7160,
    7170,   7180,   7190,   7200,   7209,   7219,   7229,   7238,
    7248,   7257,   7267,   7276,   7285,   7295,   7304,   7313,
    7322,   7331,   7340,   7349,   7358,   7366,   7375,   7384,
    7392,   7401,   7409,   7418,   7426,   7435,   7443,   7451,
    7459,   7468,   7476,   7484,   7492,   7500,   7508,   7516,
    7524,   7531,   7539,   7547,   7555,   7562,   7570,   7577,
    7585,   7592,   7600,   7607,   7615,   7622,   7629,   7637,
    7644,   7651,   7658,   7665,   7672,   7679,   7687,   7693,
    7700,   7707,   7714,   7721,   7728,   7735,   7741,   7748,
    7755,   7762,   7768,   7775,   7781,   7788,   7794,   7801,
    7807,   7814,   7820,   7827,   7833,   7839,   7845,   7852,
    7858,   7864,   7870,   7877,   7883,   7889,   7895,   7901,
    7907,   7913,   7919,   7925,   7931,   7937,   7943,   7948,
    7954,   7960,   7966,   7972,   7977,   7983,   7989,   7994,
    8000,   8006,   8011,   8017,   8022,   8028,   8033,   8039,
    8044,   8050,   8055,   8061,   8066,   8071,   8077,   8082,
    8087,   8093,   8098,   8103,   8109,   8114,   8119,   8124,
    8129,   8134,   8140,   8145,   8150,   8155,   8160,   8165,
    8170,   8175,   8180,   8185,   8190,   8195,   8200,   8205,
    8209,   8214,   8219,   8224,   8229,   8234,   8238,   8243,
    8248,   8253,   8257,   8262,   8267,   8271,   8276,   8281,
    8285,   8290,   8295,   8299,   8304,   8308,   8313,   8317,
    8322,   8326,   8331,   8335,   8340,   8344,   8349,   8353,
    8358,   8362,   8366,   8371,   8375,   8379,   8384,   8388,
    8392,   8397,   8401,   8405,   8409,   8414,   8418,   8422,
    8426,   8430,   8435,   8439,   8443,   8447,   8451,   8455,
    8459,   8464,   8468,   8472,   8476,   8480,   8484,   8488,
    8492,   8496,   8500,   8504,   8508,   8512,   8516,   8520,
    8524,   8527,   8531,   8535,   8539,   8543,   8547,   8551,
    8555,   8558,   8562,   8566,   8570,   8574,   8577,   8581,
    8585,   8589,   8592,   8596,   8600,   8604,   8607,   8611,
    8615,   8618,   8622,   8626,   8629,   8633,   8637,   8640,
    8644,   8647,   8651,   8655,   8658,   8662,   8665,   8669,
    8672,   8676,   8679,   8683,   8687,   8690,   8693,   8697,
    8700,   8704,   8707,   8711,   8714,   8718,   8721,   8725,
    8728,   8731,   8735,   8738,   8741,   8745,   8748,   8752,
    8755,   8758,   8762,   8765,   8768,   8771,   8775,   8778,
    8781,   8785,   8788,   8791,   8794,   8798,   8801,   8804,
    8807,   8811,   8814,   8817,   8820,   8823,   8827,   8830,
    8833,   8836,   8839,   8842,   8845,   8849,   8852,   8855,
    8858,   8861,   8864,   8867,   8870,   8873,   8877,   8880,
    8883,   8886,   8889,   8892,   8895,   8898,   8901,   8904,
    8907,   8910,   8913,   8916,   8919,   8922,   8925,   8928,
    8931,   8934,   8937,   8940,   8943,   8945,   8948,   8951,
    8954,   8957,   8960,   8963,   8966,   8969,   8972,   8974,
    8977,   8980,   8983,   8986,   8989,   8992,   8994,   8997,
    9000,   9003,   9006,   9008,   9011,   9014,   9017,   9020,
    9022,   9025,   9028,   9031,   9033,   9036,   9039,   9042,
    9044,   9047,   9050,   9053,   9055,   9058,   9061,   9063,
    9066,   9069,   9071,   9074,   9077,   9079,   9082,   9085,
    9087,   9090,   9093,   9095,   9098,   9101,   9103,   9106,
    9109,   9111,   9114,   9116,   9119,   9122,   9124,   9127,
    9129,   9132,   9134,   9137,   9140,   9142,   9145,   9147,
    9150,   9152,   9155,   9157,   9160,   9162,   9165,   9167,
    9170,   9172,   9175,   9177,   9180,   9182,   9185,   9187,
    9190,   9192,   9195,   9197,   9200,   9202,   9205,   9207,
    9209,   9212,   9214,   9217,   9219,   9222,   9224,   9226,
    9229,   9231,   9234,   9236,   9238,   9241,   9243,   9246,
    9248,   9250,   9253,   9255,   9257,   9260,   9262,   9264,
    9267,   9269,   9271,   9274,   9276,   9278,   9281,   9283,
    9285,   9288,   9290,   9292,   9295,   9297,   9299,   9301,
    9304,   9306,   9308,   9311,   9313,   9315,   9317,   9320,
    9322,   9324,   9326,   9329,   9331,   9333,   9335,   9338,
    9340,   9342,   9344,   9347,   9349,   9351,   9353,   9355,
    9358,   9360,   9362,   9364,   9366,   9369,   9371,   9373,
    9375,   9377,   9379,   9382,   9384,   9386,   9388,   9390,
    9392,   9394,   9397,   9399,   9401,   9403,   9405,   9407,
    9409,   9412,   9414,   9416,   9418,   9420,   9422,   9424,
    9426,   9428,   9430,   9433,   9435,   9437,   9439,   9441,
    9443,   9445,   9447,   9449,   9451,   9453,   9455,   9457,
    9459,   9461,   9464,   9466,   9468,   9470,   9472,   9474,
    9476,   9478,   9480,   9482,   9484,   9486,   9488,   9490,
    9492,   9494,   9496,   9498,   9500,   9502,   9504,   9506,
    9508,   9510,   9512,   9514,   9516,   9518,   9520,   9522,
    9524,   9526,   9527,   9529,   9531,   9533,   9535,   9537,
    9539,   9541,   9543,   9545,   9547,   9549,   9551,   9553,
    9555,   9557,   9558,   9560,   9562,   9564,   9566,   9568,
    9570,   9572,   9574,   9576,   9577,   9579,   9581,   9583,
    9585,   9587,   9589,   9591,   9592,   9594,   9596,   9598,
    9600,   9602,   9604,   9605,   9607,   9609,   9611,   9613,
    9615,   9617,   9618,   9620,   9622,   9624,   9626,   9628,
    9629,   9631,   9633,   9635,   9637,   9638,   9640,   9642,
    9644,   9646,   9647,   9649,   9651,   9653,   9655,   9656,
    9658,   9660,   9662,   9664,   9665,   9667,   9669,   9671,
    9672,   9674,   9676,   9678,   9679,   9681,   9683,   9685,
    9687,   9688,   9690,   9692,   9693,   9695,   9697,   9699,
    9700,   9702,   9704,   9706,   9707,   9709,   9711,   9713,
    9714,   9716,   9718,   9719,   9721,   9723,   9725,   9726,
    9728,   9730,   9731,   9733,   9735,   9736,   9738,   9740,
    9741,   9743,   9745,   9747,   9748,   9750,   9752,   9753,
    9755,   9757,   9758,   9760,   9762,   9763,   9765,   9767,
    9768,   9770,   9771,   9773,   9775,   9776,   9778,   9780,
    9781,   9783,   9785,   9786,   9788,   9790,   9791,   9793,
    9794,   9796,   9798,   9799,   9801,   9803,   9804,   9806,
    9807,   9809,   9811,   9812,   9814,   9815,   9817,   9819,
    9820,   9822,   9823,   9825,   9827,   9828,   9830,   9831,
    9833,   9834,   9836,   9838,   9839,   9841,   9842,   9844,
    9845,   9847,   9849,   9850,   9852,   9853,   9855,   9856,
    9858,   9860,   9861,   9863,   9864,   9866,   9867,   9869,
    9870,   9872,   9873,   9875,   9877,   9878,   9880,   9881,
    9883,   9884,   9886,   9887,   9889,   9890,   9892,   9893,
    9895,   9896,   9898,   9899,   9901,   9902,   9904,   9905,
    9907,   9908,   9910,   9911,   9913,   9914,   9916,   9917,
    9919,   9920,   9922,   9923,   9925,   9926,   9928,   9929,
    9931,   9932,   9934,   9935,   9937,   9938,   9940,   9941,
    9943,   9944,   9945,   9947,   9948,   9950,   9951,   9953,
    9954,   9956,   9957,   9959,   9960,   9961,   9963,   9964,
    9966,   9967,   9969,   9970,   9972,   9973,   9974,   9976,
    9977,   9979,   9980,   9982,   9983,   9984,   9986,   9987,
    9989,   9990,   9992,   9993,   9994,   9996,   9997,   9999,
    10000,   10001,   10003,   10004,   10006,   10007,   10008,   10010,
    10011,   10013,   10014,   10015,   10017,   10018,   10020,   10021,
    10022,   10024,   10025,   10027,   10028,   10029,   10031,   10032,
    10033,   10035,   10036,   10038,   10039,   10040,   10042,   10043,
    10044,   10046,   10047,   10048,   10050,   10051,   10053,   10054,
    10055,   10057,   10058,   10059,   10061,   10062,   10063,   10065,
    10066,   10067,   10069,   10070,   10071,   10073,   10074,   10075,
    10077,   10078,   10079,   10081,   10082,   10083,   10085,   10086,
    10087,   10089,   10090,   10091,   10093,   10094,   10095,   10097,
    10098,   10099,   10101,   10102,   10103,   10105,   10106,   10107,
    10109,   10110,   10111,   10112,   10114,   10115,   10116,   10118,
    10119,   10120,   10122,   10123,   10124,   10125,   10127,   10128,
    10129,   10131,   10132,   10133,   10134,   10136,   10137,   10138,
    10140,   10141,   10142,   10143,   10145,   10146,   10147,   10148,
    10150,   10151,   10152,   10154,   10155,   10156,   10157,   10159,
    10160,   10161,   10162,   10164,   10165,   10166,   10167,   10169,
    10170,   10171,   10172,   10174,   10175,   10176,   10177,   10179,
    10180,   10181,   10182,   10184,   10185,   10186,   10187,   10189,
    10190,   10191,   10192,   10194,   10195,   10196,   10197,   10198,
    10200,   10201,   10202,   10203,   10205,   10206,   10207,   10208,
    10209,   10211,   10212,   10213,   10214,   10216,   10217,   10218,
    10219,   10220,   10222,   10223,   10224,   10225,   10226,   10228,
    10229,   10230,   10231,   10232,   10234,   10235,   10236,   10237,
    10238,   10240,   10241,   10242,   10243,   10244,   10246,   10247,
    10248,   10249,   10250,   10251,   10253,   10254,   10255,   10256,
    10257,   10259,   10260,   10261,   10262,   10263,   10264,   10266,
    10267,   10268,   10269,   10270,   10271,   10273,   10274,   10275,
    10276,   10277,   10278,   10280,   10281,   10282,   10283,   10284,
    10285,   10287,   10288,   10289,   10290,   10291,   10292,   10293,
    10295,   10296,   10297,   10298,   10299,   10300,   10301,   10303,
    10304,   10305,   10306,   10307,   10308,   10309,   10311,   10312,
    10313,   10314,   10315,   10316,   10317,   10319,   10320,   10321,
    10322,   10323,   10324,   10325,   10326,   10328,   10329,   10330,
    10331,   10332,   10333,   10334,   10335,   10337,   10338,   10339,
    10340,   10341,   10342,   10343,   10344,   10345,   10347,   10348,
    10349,   10350,   10351,   10352,   10353,   10354,   10355,   10356,
    10358,   10359,   10360,   10361,   10362,   10363,   10364,   10365,
    10366,   10367,   10369,   10370,   10371,   10372,   10373,   10374,
    10375,   10376,   10377,   10378,   10379,   10380,   10382,   10383,
    10384,   10385,   10386,   10387,   10388,   10389,   10390,   10391,
    10392,   10393,   10394,   10396,   10397,   10398,   10399,   10400,
    10401,   10402,   10403,   10404,   10405,   10406,   10407,   10408,
    10409,   10410,   10412,   10413,   10414,   10415,   10416,   10417,
    10418,   10419,   10420,   10421,   10422,   10423,   10424,   10425,
    10426,   10427,   10428,   10429,   10430,   10431,   10433,   10434,
    10435,   10436,   10437,   10438,   10439,   10440,   10441,   10442,
    10443,   10444,   10445,   10446,   10447,   10448,   10449,   10450,
    10451,   10452,   10453,   10454,   10455,   10456,   10457,   10458,
    10459,   10460,   10461,   10463,   10464,   10465,   10466,   10467,
    10468,   10469,   10470,   10471,   10472,   10473,   10474,   10475,
    10476,   10477,   10478,   10479,   10480,   10481,   10482,   10483,
    10484,   10485,   10486,   10487,   10488,   10489,   10490,   10491,
    10492,   10493,   10494,   10495,   10496,   10497,   10498,   10499,
    10500,   10501,   10502,   10503,   10504,   10505,   10506,   10507,
    10508,   10509,   10510,   10511,   10512,   10513,   10514,   10515,
    10516,   10517,   10518,   10519,   10520,   10521,   10522,   10523,
    10524,   10525,   10526,   10526,   10527,   10528,   10529,   10530,
    10531,   10532,   10533,   10534,   10535,   10536,   10537,   10538,
    10539,   10540,   10541,   10542,   10543,   10544,   10545,   10546,
    10547,   10548,   10549,   10550,   10551,   10552,   10553,   10554,
    10555,   10556,   10557,   10557,   10558,   10559,   10560,   10561,
    10562,   10563,   10564,   10565,   10566,   10567,   10568,   10569,
    10570,   10571,   10572,   10573,   10574,   10575,   10576,   10576,
    10577,   10578,   10579,   10580,   10581,   10582,   10583,   10584,
    10585,   10586,   10587,   10588,   10589,   10590,   10591,   10592,
    10592,   10593,   10594,   10595,   10596,   10597,   10598,   10599,
    10600,   10601,   10602,   10603,   10604,   10605,   10605,   10606,
    10607,   10608,   10609,   10610,   10611,   10612,   10613,   10614,
    10615,   10616,   10617,   10617,   10618,   10619,   10620,   10621,
    10622,   10623,   10624,   10625,   10626,   10627,   10628,   10628,
    10629,   10630,   10631,   10632,   10633,   10634,   10635,   10636,
    10637,   10638,   10638,   10639,   10640,   10641,   10642,   10643,
    10644,   10645,   10646,   10647,   10647,   10648,   10649,   10650,
    10651,   10652,   10653,   10654,   10655,   10656,   10656,   10657,
    10658,   10659,   10660,   10661,   10662,   10663,   10664,   10664,
    10665,   10666,   10667,   10668,   10669,   10670,   10671,   10672,
    10672,   10673,   10674,   10675,   10676,   10677,   10678,   10679,
    10679,   10680,   10681,   10682,   10683,   10684,   10685,   10686,
    10687,   10687,   10688,   10689,   10690,   10691,   10692,   10693,
    10693,   10694,   10695,   10696,   10697,   10698,   10699,   10700,
    10700,   10701,   10702,   10703,   10704,   10705,   10706,   10706,
    10707,   10708,   10709,   10710,   10711,   10712,   10713,   10713,
    10714,   10715,   10716,   10717,   10718,   10719,   10719,   10720,
    10721,   10722,   10723,   10724,   10725,   10725,   10726,   10727,
    10728,   10729,   10730,   10730,   10731,   10732,   10733,   10734,
    10735,   10736,   10736,   10737,   10738,   10739,   10740,   10741,
    10741,   10742,   10743,   10744,   10745,   10746,   10747,   10747,
    10748,   10749,   10750,   10751,   10752,   10752,   10753,   10754,
    10755,   10756,   10757,   10757,   10758,   10759,   10760,   10761,
    10762,   10762,   10763,   10764,   10765,   10766,   10767,   10767,
    10768,   10769,   10770,   10771,   10771,   10772,   10773,   10774,
    10775,   10776,   10776,   10777,   10778,   10779,   10780,   10781,
    10781,   10782,   10783,   10784,   10785,   10785,   10786,   10787,
    10788,   10789,   10790,   10790,   10791,   10792,   10793,   10794,
    10794,   10795,   10796,   10797,   10798,   10798,   10799,   10800,
    10801,   10802,   10803,   10803,   10804,   10805,   10806,   10807,
    10807,   10808,   10809,   10810,   10811,   10811,   10812,   10813,
    10814,   10815,   10815,   10816,   10817,   10818,   10819,   10819,
    10820,   10821,   10822,   10823,   10823,   10824,   10825,   10826,
    10827,   10827,   10828,   10829,   10830,   10831,   10831,   10832,
    10833,   10834,   10834,   10835,   10836,   10837,   10838,   10838,
    10839,   10840,   10841,   10842,   10842,   10843,   10844,   10845,
    10845,   10846,   10847,   10848,   10849,   10849,   10850,   10851,
    10852,   10853,   10853,   10854,   10855,   10856,   10856,   10857,
    10858,   10859,   10860,   10860,   10861,   10862,   10863,   10863,
    10864,   10865,   10866,   10867,   10867,   10868,   10869,   10870,
    10870,   10871,   10872,   10873,   10873,   10874,   10875,   10876,
    10877,   10877,   10878,   10879,   10880,   10880,   10881,   10882,
    10883,   10883,   10884,   10885,   10886,   10886,   10887,   10888,
    10889,   10890,   10890,   10891,   10892,   10893,   10893,   10894,
    10895,   10896,   10896,   10897,   10898,   10899,   10899,   10900,
    10901,   10902,   10902,   10903,   10904,   10905,   10905,   10906,
    10907,   10908,   10908,   10909,   10910,   10911,   10911,   10912,
    10913,   10914,   10914,   10915,   10916,   10917,   10917,   10918,
    10919,   10920,   10920,   10921,   10922,   10923,   10923,   10924,
    10925,   10926,   10926,   10927,   10928,   10929,   10929,   10930,
    10931,   10931,   10932,   10933,   10934,   10934,   10935,   10936,
    10937,   10937,   10938,   10939,   10940,   10940,   10941,   10942,
    10943,   10943,   10944,   10945,   10945,   10946,   10947,   10948,
    10948,   10949,   10950,   10951,   10951,   10952,   10953,   10953,
    10954,   10955,   10956,   10956,   10957,   10958,   10959,   10959,
    10960,   10961,   10961,   10962,   10963,   10964,   10964,   10965,
    10966,   10967,   10967,   10968,   10969,   10969,   10970,   10971,
    10972,   10972,   10973,   10974,   10974,   10975,   10976,   10977,
    10977,   10978,   10979,   10979,   10980,   10981,   10982,   10982,
    10983,   10984,   10984,   10985,   10986,   10987,   10987,   10988,
    10989,   10989,   10990,   10991,   10992,   10992,   10993,   10994,
    10994,   10995,   10996,   10996,   10997,   10998,   10999,   10999,
    11000,   11001,   11001,   11002,   11003,   11004,   11004,   11005,
    11006,   11006,   11007,   11008,   11008,   11009,   11010,   11011,
    11011,   11012,   11013,   11013,   11014,   11015,   11015,   11016,
    11017,   11018,   11018,   11019,   11020,   11020,   11021,   11022,
    11022,   11023,   11024,   11024,   11025,   11026,   11027,   11027,
    11028,   11029,   11029,   11030,   11031,   11031,   11032,   11033,
    11033,   11034,   11035,   11035,   11036,   11037,   11038,   11038,
    11039,   11040,   11040,   11041,   11042,   11042,   11043,   11044,
    11044,   11045,   11046,   11046,   11047,   11048,   11048,   11049,
    11050,   11051,   11051,   11052,   11053,   11053,   11054,   11055,
    11055,   11056,   11057,   11057,   11058,   11059,   11059,   11060,
    11061,   11061,   11062,   11063,   11063,   11064,   11065,   11065,
    11066,   11067,   11067,   11068,   11069,   11069,   11070,   11071,
    11071,   11072,   11073,   11073,   11074,   11075,   11075,   11076,
    11077,   11077,   11078,   11079,   11079,   11080,   11081,   11081,
    11082,   11083,   11083,   11084,   11085,   11085,   11086,   11087,
    11087,   11088,   11089,   11089,   11090,   11091,   11091,   11092,
    11093,   11093,   11094,   11095,   11095,   11096,   11097,   11097,
    11098,   11099,   11099,   11100,   11101,   11101,   11102,   11103,
    11103,   11104,   11105,   11105,   11106,   11107,   11107,   11108,
    11109,   11109,   11110,   11110,   11111,   11112,   11112,   11113,
    11114,   11114,   11115,   11116,   11116,   11117,   11118,   11118,
    11119,   11120,   11120,   11121,   11122,   11122,   11123,   11123,
    11124,   11125,   11125,   11126,   11127,   11127,   11128,   11129,
    11129,   11130,   11131,   11131,   11132,   11132,   11133,   11134,
    11134,   11135,   11136,   11136,   11137,   11138,   11138,   11139,
    11140,   11140,   11141,   11141,   11142,   11143,   11143,   11144,
    11145,   11145,   11146,   11147,   11147,   11148,   11148,   11149,
    11150,   11150,   11151,   11152,   11152,   11153,   11154,   11154,
    11155,   11155,   11156,   11157,   11157,   11158,   11159,   11159,
    11160,   11161,   11161,   11162,   11162,   11163,   11164,   11164,
    11165,   11166,   11166,   11167,   11167,   11168,   11169,   11169,
    11170,   11171,   11171,   11172,   11172,   11173,   11174,   11174,
    11175,   11176,   11176,   11177,   11177,   11178,   11179,   11179,
    11180,   11181,   11181,   11182,   11182,   11183,   11184,   11184,
    11185,   11185,   11186,   11187,   11187,   11188,   11189,   11189,
    11190,   11190,   11191,   11192,   11192,   11193,   11194,   11194,
    11195,   11195,   11196,   11197,   11197,   11198,   11198,   11199,
    11200,   11200,   11201,   11202,   11202,   11203,   11203,   11204,
    11205,   11205,   11206,   11206,   11207,   11208,   11208,   11209,
    11209,   11210,   11211,   11211,   11212,   11212,   11213,   11214,
    11214,   11215,   11216,   11216,   11217,   11217,   11218,   11219,
    11219,   11220,   11220,   11221,   11222,   11222,   11223,   11223,
    11224,   11225,   11225,   11226,   11226,   11227,   11228,   11228,
    11229,   11229,   11230,   11231,   11231,   11232,   11232,   11233,
    11234,   11234,   11235,   11235,   11236,   11237,   11237,   11238,
    11238,   11239,   11240,   11240,   11241,   11241,   11242,   11243,
    11243,   11244,   11244,   11245,   11246,   11246,   11247,   11247,
    11248,   11249,   11249,   11250,   11250,   11251,   11251,   11252,
    11253,   11253,   11254,   11254,   11255,   11256,   11256,   11257,
    11257,   11258,   11259,   11259,   11260,   11260,   11261,   11262,
    11262,   11263,   11263,   11264,   11264,   11265,   11266,   11266,
    11267,   11267,   11268,   11269,   11269,   11270,   11270,   11271,
    11271,   11272,   11273,   11273,   11274,   11274,   11275,   11276,
    11276,   11277,   11277,   11278,   11278,   11279,   11280,   11280,
    11281,   11281,   11282,   11283,   11283,   11284,   11284,   11285,
    11285,   11286,   11287,   11287,   11288,   11288,   11289,   11289,
    11290,   11291,   11291,   11292,   11292,   11293,   11293,   11294,
    11295,   11295,   11296,   11296,   11297,   11297,   11298,   11299,
    11299,   11300,   11300,   11301,   11301,   11302,   11303,   11303,
    11304,   11304,   11305,   11305,   11306,   11307,   11307,   11308,
    11308,   11309,   11309,   11310,   11311,   11311,   11312,   11312,
    11313,   11313,   11314,   11315,   11315,   11316,   11316,   11317,
    11317,   11318,   11319,   11319,   11320,   11320,   11321,   11321,
    11322,   11322,   11323,   11324,   11324,   11325,   11325,   11326,
    11326,   11327,   11328,   11328,   11329,   11329,   11330,   11330,
    11331,   11331,   11332,   11333,   11333,   11334,   11334,   11335,
    11335,   11336,   11337,   11337,   11338,   11338,   11339,   11339,
    11340,   11340,   11341,   11342,   11342,   11343,   11343,   11344,
    11344,   11345,   11345,   11346,   11347,   11347,   11348,   11348,
    11349,   11349,   11350,   11350,   11351,   11351,   11352,   11353,
    11353,   11354,   11354,   11355,   11355,   11356,   11356,   11357,
    11358,   11358,   11359,   11359,   11360,   11360,   11361,   11361,
    11362,   11362,   11363,   11364,   11364,   11365,   11365,   11366,
    11366,   11367,   11367,   11368,   11369,   11369,   11370,   11370,
    11371,   11371,   11372,   11372,   11373,   11373,   11374,   11374,
    11375,   11376,   11376,   11377,   11377,   11378,   11378,   11379,
    11379,   11380,   11380,   11381,   11382,   11382,   11383,   11383,
    11384,   11384,   11385,   11385,   11386,   11386,   11387,   11387,
    11388,   11389,   11389,   11390,   11390,   11391,   11391,   11392,
    11392,   11393,   11393,   11394,   11394,   11395,   11396,   11396,
    11397,   11397,   11398,   11398,   11399,   11399,   11400,   11400,
    11401,   11401,   11402,   11402,   11403,   11404,   11404,   11405,
    11405,   11406,   11406,   11407,   11407,   11408,   11408,   11409,
    11409,   11410,   11410,   11411,   11412,   11412,   11413,   11413,
    11414,   11414,   11415,   11415,   11416,   11416,   11417,   11417,
    11418,   11418,   11419,   11419,   11420,   11420,   11421,   11422,
    11422,   11423,   11423,   11424,   11424,   11425,   11425,   11426,
    11426,   11427,   11427,   11428,   11428,   11429,   11429,   11430,
    11430,   11431,   11431,   11432,   11433,   11433,   11434,   11434,
    11435,   11435,   11436,   11436,   11437,   11437,   11438,   11438,
    11439,   11439,   11440,   11440,   11441,   11441,   11442,   11442,
    11443,   11443,   11444,   11444,   11445,   11446,   11446,   11447,
    11447,   11448,   11448,   11449,   11449,   11450,   11450,   11451,
    11451,   11452,   11452,   11453,   11453,   11454,   11454,   11455,
    11455,   11456,   11456,   11457,   11457,   11458,   11458,   11459,
    11459,   11460,   11460,   11461,   11461,   11462,   11463,   11463,
    11464,   11464,   11465,   11465,   11466,   11466,   11467,   11467,
    11468,   11468,   11469,   11469,   11470,   11470,   11471,   11471,
    11472,   11472,   11473,   11473,   11474,   11474,   11475,   11475,
    11476,   11476,   11477,   11477,   11478,   11478,   11479,   11479,
    11480,   11480,   11481,   11481,   11482,   11482,   11483,   11483,
    11484,   11484,   11485,   11485,   11486,   11486,   11487,   11487,
    11488,   11488,   11489,   11489,   11490,   11490,   11491,   11491,
    11492,   11492,   11493,   11493,   11494,   11494,   11495,   11495,
    11496,   11496,   11497,   11497,   11498,   11498,   11499,   11499,
    11500,   11500,   11501,   11501,   11502,   11502,   11503,   11503,
    11504,   11504,   11505,   11505,   11506,   11506,   11507,   11507,
    11508,   11508,   11509,   11509,   11510,   11510,   11511,   11511,
    11512,   11512,   11513,   11513,   11514,   11514,   11515,   11515,
    11516,   11516,   11517,   11517,   11518,   11518,   11519,   11519,
    11520,   11520,   11521,   11521,   11522,   11522,   11523,   11523,
    11524,   11524,   11525,   11525,   11526,   11526,   11526,   11527,
    11527,   11528,   11528,   11529,   11529,   11530,   11530,   11531,
    11531,   11532,   11532,   11533,   11533,   11534,   11534,   11535,
    11535,   11536,   11536,   11537,   11537,   11538,   11538,   11539,
    11539,   11540,   11540,   11541,   11541,   11542,   11542,   11543,
    11543,   11544,   11544,   11544,   11545,   11545,   11546,   11546,
    11547,   11547,   11548,   11548,   11549,   11549,   11550,   11550,
    11551,   11551,   11552,   11552,   11553,   11553,   11554,   11554,
    11555,   11555,   11556,   11556,   11557,   11557,   11557,   11558,
    11558,   11559,   11559,   11560,   11560,   11561,   11561,   11562,
    11562,   11563,   11563,   11564,   11564,   11565,   11565,   11566,
    11566,   11567,   11567,   11567,   11568,   11568,   11569,   11569,
    11570,   11570,   11571,   11571,   11572,   11572,   11573,   11573,
    11574,   11574,   11575,   11575,   11576,   11576,   11576,   11577,
    11577,   11578,   11578,   11579,   11579,   11580,   11580,   11581,
    11581,   11582,   11582,   11583,   11583,   11584,   11584,   11584,
    11585,   11585,   11586,   11586,   11587,   11587,   11588,   11588,
    11589,   11589,   11590,   11590,   11591,   11591,   11592,   11592,
    11592,   11593,   11593,   11594,   11594,   11595,   11595,   11596,
    11596,   11597,   11597,   11598,   11598,   11599,   11599,   11599,
    11600,   11600,   11601,   11601,   11602,   11602,   11603,   11603,
    11604,   11604,   11605,   11605,   11605,   11606,   11606,   11607,
    11607,   11608,   11608,   11609,   11609,   11610,   11610,   11611,
    11611,   11611,   11612,   11612,   11613,   11613,   11614,   11614,
    11615,   11615,   11616,   11616,   11617,   11617,   11617,   11618,
    11618,   11619,   11619,   11620,   11620,   11621,   11621,   11622,
    11622,   11623,   11623,   11623,   11624,   11624,   11625,   11625,
    11626,   11626,   11627,   11627,   11628,   11628,   11628,   11629,
    11629,   11630,   11630,   11631,   11631,   11632,   11632,   11633,
    11633,   11633,   11634,   11634,   11635,   11635,   11636,   11636,
    11637,   11637,   11638,   11638,   11638,   11639,   11639,   11640,
    11640,   11641,   11641,   11642,   11642,   11643,   11643,   11643,
    11644,   11644,   11645,   11645,   11646,   11646,   11647,   11647,
    11647,   11648,   11648,   11649,   11649,   11650,   11650,   11651,
    11651,   11652,   11652,   11652,   11653,   11653,   11654,   11654,
    11655,   11655,   11656,   11656,   11656,   11657,   11657,   11658,
    11658,   11659,   11659,   11660,   11660,   11660,   11661,   11661,
    11662,   11662,   11663,   11663,   11664,   11664,   11664,   11665,
    11665,   11666,   11666,   11667,   11667,   11668,   11668,   11668,
    11669,   11669,   11670,   11670,   11671,   11671,   11672,   11672,
    11672,   11673,   11673,   11674,   11674,   11675,   11675,   11676,
    11676,   11676,   11677,   11677,   11678,   11678,   11679,   11679,
    11679,   11680,   11680,   11681,   11681,   11682,   11682,   11683,
    11683,   11683,   11684,   11684,   11685,   11685,   11686,   11686,
    11687,   11687,   11687,   11688,   11688,   11689,   11689,   11690,
    11690,   11690,   11691,   11691,   11692,   11692,   11693,   11693,
    11693,   11694,   11694,   11695,   11695,   11696,   11696,   11697,
    11697,   11697,   11698,   11698,   11699,   11699,   11700,   11700,
    11700,   11701,   11701,   11702,   11702,   11703,   11703,   11703,
    11704,   11704,   11705,   11705,   11706,   11706,   11706,   11707,
    11707,   11708,   11708,   11709,   11709,   11710,   11710,   11710,
    11711,   11711,   11712,   11712,   11713,   11713,   11713,   11714,
    11714,   11715,   11715,   11716,   11716,   11716,   11717,   11717,
    11718,   11718,   11719,   11719,   11719,   11720,   11720,   11721,
    11721,   11722,   11722,   11722,   11723,   11723,   11724,   11724,
    11725,   11725,   11725,   11726,   11726,   11727,   11727,   11727,
    11728,   11728,   11729,   11729,   11730,   11730,   11730,   11731,
    11731,   11732,   11732,   11733,   11733,   11733,   11734,   11734,
    11735,   11735,   11736,   11736,   11736,   11737,   11737,   11738,
    11738,   11739,   11739,   11739,   11740,   11740,   11741,   11741,
    11741,   11742,   11742,   11743,   11743,   11744,   11744,   11744,
    11745,   11745,   11746,   11746,   11747,   11747,   11747,   11748,
    11748,   11749,   11749,   11749,   11750,   11750,   11751,   11751,
    11752,   11752,   11752,   11753,   11753,   11754,   11754,   11754,
    11755,   11755,   11756,   11756,   11757,   11757,   11757,   11758,
    11758,   11759,   11759,   11759,   11760,   11760,   11761,   11761,
    11762,   11762,   11762,   11763,   11763,   11764,   11764,   11764,
    11765,   11765,   11766,   11766,   11767,   11767,   11767,   11768,
    11768,   11769,   11769,   11769,   11770,   11770,   11771,   11771,
    11771,   11772,   11772,   11773,   11773,   11774,   11774,   11774,
    11775,   11775,   11776,   11776,   11776,   11777,   11777,   11778,
    11778,   11778,   11779,   11779,   11780,   11780,   11781,   11781,
    11781,   11782,   11782,   11783,   11783,   11783,   11784,   11784,
    11785,   11785,   11785,   11786,   11786,   11787,   11787,   11787,
    11788,   11788,   11789,   11789,   11790,   11790,   11790,   11791,
    11791,   11792,   11792,   11792,   11793,   11793,   11794,   11794,
    11794,   11795,   11795,   11796,   11796,   11796,   11797,   11797,
    11798,   11798,   11798,   11799,   11799,   11800,   11800,   11800,
    11801,   11801,   11802,   11802,   11803,   11803,   11803,   11804,
    11804,   11805,   11805,   11805,   11806,   11806,   11807,   11807,
    11807,   11808,   11808,   11809,   11809,   11809,   11810,   11810,
    11811,   11811,   11811,   11812,   11812,   11813,   11813,   11813,
    11814,   11814,   11815,   11815,   11815,   11816,   11816,   11817,
    11817,   11817,   11818,   11818,   11819,   11819,   11819,   11820,
    11820,   11821,   11821,   11821,   11822,   11822,   11823,   11823,
    11823,   11824,   11824,   11825,   11825,   11825,   11826,   11826,
    11827,   11827,   11827,   11828,   11828,   11829,   11829,   11829,
    11830,   11830,   11831,   11831,   11831,   11832,   11832,   11832,
    11833,   11833,   11834,   11834,   11834,   11835,   11835,   11836,
    11836,   11836,   11837,   11837,   11838,   11838,   11838,   11839,
    11839,   11840,   11840,   11840,   11841,   11841,   11842,   11842,
    11842,   11843,   11843,   11844,   11844,   11844,   11845,   11845,
    11845,   11846,   11846,   11847,   11847,   11847,   11848,   11848,
    11849,   11849,   11849,   11850,   11850,   11851,   11851,   11851,
    11852,   11852,   11853,   11853,   11853,   11854,   11854,   11854,
    11855,   11855,   11856,   11856,   11856,   11857,   11857,   11858,
    11858,   11858,   11859,   11859,   11860,   11860,   11860,   11861,
    11861,   11861,   11862,   11862,   11863,   11863,   11863,   11864,
    11864,   11865,   11865,   11865,   11866,   11866,   11867,   11867,
    11867,   11868,   11868,   11868,   11869,   11869,   11870,   11870,
    11870,   11871,   11871,   11872,   11872,   11872,   11873,   11873,
    11873,   11874,   11874,   11875,   11875,   11875,   11876,   11876,
    11877,   11877,   11877,   11878,   11878,   11878,   11879,   11879,
    11880,   11880,   11880,   11881,   11881,   11881,   11882,   11882,
    11883,   11883,   11883,   11884,   11884,   11885,   11885,   11885,
    11886,   11886,   11886,   11887,   11887,   11888,   11888,   11888,
    11889,   11889,   11890,   11890,   11890,   11891,   11891,   11891,
    11892,   11892,   11893,   11893,   11893,   11894,   11894,   11894,
    11895,   11895,   11896,   11896,   11896,   11897,   11897,   11897,
    11898,   11898,   11899,   11899,   11899,   11900,   11900,   11900,
    11901,   11901,   11902,   11902,   11902,   11903,   11903,   11904,
    11904,   11904,   11905,   11905,   11905,   11906,   11906,   11907,
    11907,   11907,   11908,   11908,   11908,   11909,   11909,   11910,
    11910,   11910,   11911,   11911,   11911,   11912,   11912,   11913,
    11913,   11913,   11914,   11914,   11914,   11915,   11915,   11916,
    11916,   11916,   11917,   11917,   11917,   11918,   11918,   11918,
    11919,   11919,   11920,   11920,   11920,   11921,   11921,   11921,
    11922,   11922,   11923,   11923,   11923,   11924,   11924,   11924,
    11925,   11925,   11926,   11926,   11926,   11927,   11927,   11927,
    11928,   11928,   11929,   11929,   11929,   11930,   11930,   11930,
    11931,   11931,   11931,   11932,   11932,   11933,   11933,   11933,
    11934,   11934,   11934,   11935,   11935,   11936,   11936,   11936,
    11937,   11937,   11937,   11938,   11938,   11938,   11939,   11939,
    11940,   11940,   11940,   11941,   11941,   11941,   11942,   11942,
    11943,   11943,   11943,   11944,   11944,   11944,   11945,   11945,
    11945,   11946,   11946,   11947,   11947,   11947,   11948,   11948,
    11948,   11949,   11949,   11949,   11950,   11950,   11951,   11951,
    11951,   11952,   11952,   11952,   11953,   11953,   11953,   11954,
    11954,   11955,   11955,   11955,   11956,   11956,   11956,   11957,
    11957,   11957,   11958,   11958,   11959,   11959,   11959,   11960,
    11960,   11960,   11961,   11961,   11961,   11962,   11962,   11963,
    11963,   11963,   11964,   11964,   11964,   11965,   11965,   11965,
    11966,   11966,   11967,   11967,   11967,   11968,   11968,   11968,
    11969,   11969,   11969,   11970,   11970,   11970,   11971,   11971,
    11972,   11972,   11972,   11973,   11973,   11973,   11974,   11974,
    11974,   11975,   11975,   11975,   11976,   11976,   11977,   11977,
    11977,   11978,   11978,   11978,   11979,   11979,   11979,   11980,
    11980,   11980,   11981,   11981,   11982,   11982,   11982,   11983,
    11983,   11983,   11984,   11984,   11984,   11985,   11985,   11985,
    11986,   11986,   11987,   11987,   11987,   11988,   11988,   11988,
    11989,   11989,   11989,   11990,   11990,   11990,   11991,   11991,
    11992,   11992,   11992,   11993,   11993,   11993,   11994,   11994,
    11994,   11995,   11995,   11995,   11996,   11996,   11996,   11997,
    11997,   11998,   11998,   11998,   11999,   11999,   11999,   12000
};

