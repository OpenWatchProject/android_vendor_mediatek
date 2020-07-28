/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited
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
#include <sys/stat.h>
#include <utils/threads.h>
#include <cutils/properties.h>
#include <mtkcam/def/common.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <private/aaa_hal_private.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
//#include <CamDefs.h>
#include <faces.h>
#include <isp_tuning.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <ae_algo_if.h>
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include "camera_custom_zvhdr.h"
#include "camera_custom_mvhdr3expo.h"
#include <kd_camera_feature.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <aaa_scheduler.h>
#include <aaa_common_custom.h>
#include <time.h>
#include <android/sensor.h>                  // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h> // for g/gyro sensor listener
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <private/IopipeUtils.h>
#include <math.h>
#include <private/aaa_utils.h>
#include <array>

static unsigned long long AEtimer;
static unsigned int NumofUsers;
static int g_rExp;
static int g_rAfe;
static int g_rIsp;
MUINT32 u4FD_Prob;

#define EMLongestExp 500000
#define RealInitThrs 600000000 // 0.6s
// define log control
#define EN_3A_FLOW_LOG 1
#define EN_3A_SCHEDULE_LOG 2
// define AAO Separation size (test)
#define AAO_STT_BLOCK_NUMX 120
#define AAO_STT_BLOCK_NUMY 90
// define TG information
#define TG_INT_AE_RATIO 0.6
// define g / gyro info
#define SENSOR_ACCE_POLLING_MS 20
#define SENSOR_GYRO_POLLING_MS 20
#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

// for AE P-line index debounce at max index
#define MAX_INDEX1_STABLE_THRES 20
#define MAX_INDEX2_STABLE_THRES 10
#define ISO1_THRES_VALUE 2800
#define ISO2_THRES_VALUE 5600

#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-1*(x)))
#endif

using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;
using namespace NSCam::Utils;
using NSCam::NSIoPipe::NSCamIOPipe::INormalPipe;
using NSCam::NSIoPipe::NSPostProc::INormalStream;
using namespace NSCam::NSIoPipe;

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

#define MY_INST_AE_MGR NS3Av3::INST_T<AeMgr>
static std::array<MY_INST_AE_MGR, SENSOR_IDX_MAX> gMultiton;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr &AeMgr::getInstance(MINT32 const i4SensorDev)
{
    int i4SensorIndex = NS3Av3::mapSensorDevToIdx(i4SensorDev);
    if(i4SensorIndex >= SENSOR_IDX_MAX || i4SensorIndex < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIndex);
        //return NULL;
    }

    MY_INST_AE_MGR& rSingleton = gMultiton[i4SensorIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeMgr>(i4SensorIndex);
    } );

    return *(rSingleton.instance);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
AeMgr(MINT32 const i4SensorIndex)
    : m_pICcuAe(NULL)
    , m_pIAeAlgo(NULL)
    , m_pIAeTest(NULL)
    , m_pIAeFlowCPU(NULL)
    , m_pIAeFlowCCU(NULL)
    , m_pIAeSettingCPU(NULL)
    , m_pIAeSettingCCU(NULL)
    , m_eSensorDev((ESensorDev_T)NS3Av3::mapSensorIdxToDev(i4SensorIndex))
    , m_eSensorTG(ESensorTG_1)
    , m_eSensorMode(ESensorMode_Preview)
    , m_eISPProfile(EIspProfile_Preview)
    , m_eCamScenarioMode(CAM_SCENARIO_PREVIEW)
    , m_u4AENVRAMIdx(CAM_SCENARIO_PREVIEW)
    , m_i4SensorIdx(i4SensorIndex)
    , m_BVvalue(0)
    , m_i4DeltaBV(0)
    , m_AOECompBVvalue(0)
    , m_BVvalueWOStrobe(0)
    , m_i4EVvalue(0)
    , m_i4WaitVDNum(0)
    , m_i4RotateDegree(0)
    , m_i4TimeOutCnt(0)
    , m_i4ShutterDelayFrames(2)
    , m_i4SensorGainDelayFrames(2)
    , m_i4SensorGainDelayFramesWOShutter(1)
    , m_i4IspGainDelayFrames(0)
    , m_i4SensorCaptureDelayFrame(0)
    , m_i4TotalCaptureDelayFrame(3)
    , m_i4AEidxCurrent(0)
    , m_i4AEidxNext(0)
    , m_i4AEidxNextF(0)
    , m_i2AEFaceDiffIndex(0)
    , m_u4PreExposureTime(0)
    , m_u4PreSensorGain(0)
    , m_u4PreIspGain(0)
    , m_u4SmoothIspGain(0)
    , m_u4AECondition(0)
    , m_u4DynamicFrameCnt(0)
    , m_u4AFSGG1Gain(16)
    , m_i4Cycle0FullDeltaIdx(0)
    , m_i4FrameCnt(0)
    , m_bAESceneChanged(MFALSE)
    , m_bAELock(MFALSE)
    , m_bAPAELock(MFALSE)
    , m_bAFAELock(MFALSE)
    , m_bEnableAE(MFALSE)
    , m_bVideoDynamic(MTRUE)
    , m_bRealISOSpeed(MFALSE)
    , m_bAElimitor(MFALSE)
    , m_bAEStable(MFALSE)
    , m_bAEReadyCapture(MFALSE)
    , m_bLockExposureSetting(MFALSE)
    , m_bStrobeOn(MFALSE)
    , m_bAEMgrDebugEnable(MFALSE)
    , m_bRestoreAE(MFALSE)
    , m_bAECaptureUpdate(MFALSE)
    , m_bOtherIPRestoreAE(MFALSE)
    , m_eAEScene(LIB3A_AE_SCENE_AUTO)
    , m_eAEMode(LIB3A_AE_MODE_ON)
    , m_ePreAEState(MTK_CONTROL_AE_STATE_INACTIVE)
    , m_eAEState(MTK_CONTROL_AE_STATE_INACTIVE)
    , m_fEVCompStep(1)
    , m_i4EVIndex(0)
    , m_i4EVIndexPreset(0)
    , m_eAEMeterMode(LIB3A_AE_METERING_MODE_CENTER_WEIGHT)
    , m_u4AEISOSpeed(0)
    , m_eAEFlickerMode(LIB3A_AE_FLICKER_MODE_50HZ)
    , m_i4AEMaxFps(LIB3A_AE_FRAMERATE_MODE_30FPS)
    , m_i4AEMinFps(LIB3A_AE_FRAMERATE_MODE_05FPS)
    , m_eAEAutoFlickerMode(LIB3A_AE_FLICKER_AUTO_MODE_50HZ)
    , m_i4AEAutoFlickerMode(0)
    , m_eCamMode(eAppMode_PhotoMode)
    , m_eAECamMode(LIB3A_AECAM_MODE_PHOTO)
    , m_eShotMode(eShotMode_NormalShot)
    , m_eAEEVcomp(LIB3A_AE_EV_COMP_00)
    , m_eAEEVcompPreset(LIB3A_AE_EV_COMP_00)
    , m_AEState(AE_INIT_STATE)
    , m_bIsAutoFlare(MTRUE)
    , m_bFrameUpdate(MFALSE)
    , m_i4ObjectTrackNum(0)
    , m_i4GammaIdx(0)
    , m_i4LESE_Ratio(1)
    , m_i4HdrSeg(3072)
    , m_i4HdrTurn(768)
    , m_i4HdrOnOff(toLiteral(HDRDetectionResult::NONE))
    , m_u4SWHDR_SE(0)
    , m_u4MaxShutter(100000)
    , m_u4MaxISO(800)
    , m_u4StableCnt(0)
    , m_u4HwMagicNum(0)
    , m_u4veLTCRate(0)
    , m_u4veLimitGain(0)
    , m_pAEPlineTable(NULL)
    , m_pAEMapPlineTable(NULL)
    , m_p3ANVRAM(NULL)
    , m_eAETargetMode(AE_MODE_NORMAL)
    , m_eAETargetMode2(AE_MODE_NORMAL)
    , m_u4PrevExposureTime(0)
    , m_u4PrevSensorGain(0)
    , m_u4PrevIspGain(0)
    , m_ePrevAECamMode(LIB3A_AECAM_MODE_PHOTO)
    , m_3ALogEnable(0)
    , m_i4DgbLogE(0)
    , m_bAEOneShotControl(MFALSE)
    , bUpdateSensorAWBGain(MFALSE)
    , bApplyAEPlineTableByCCT(MFALSE)
    , m_u4UpdateShutterValue(0)
    , m_u4UpdateGainValue(0)
    , m_u4UpdateGainValue2(0)
    , m_u4UpdateGainValue3(0)
    , m_u4UpdateFrameRate_x10(LIB3A_AE_FRAMERATE_MODE_30FPS)
    , m_bTouchAEAreaChage(MFALSE)
    , m_bInitWaitFaceSkipAE(MFALSE)
    , m_u4InitWaitFaceCount(0)
    , m_u4InitWaitCount(0)
    , m_bMultiCap(MFALSE)
    , m_bCShot(MFALSE)
    , m_bLockBlackLevel(MFALSE)
    , m_u4WOFDCnt (0)
    , m_bFDenable(MFALSE)
    , m_bFaceAEAreaChage(MFALSE)
    , m_u4StableYValue(0)
    , m_bzcHDRshot(MFALSE)
    , m_bHDRshot(MFALSE)
    , m_bResetZHDRMode(MFALSE)
    , m_u4VsyncCnt(0)
    , m_Lock()
    , m_u4LEHDRshutter(0)
    , m_u4SEHDRshutter(0)
    , m_u4MEHDRshutter(0)
    , m_bIsFlareInManual(MFALSE)
    , m_uEvQueueSize(MAX_AE_PRE_EVSETTING)
    , m_u4PreviousSensorgain(0)
    , m_u4PreviousISPgain(0)
    , m_bPerframeAEFlag(0)
    , m_bTgIntAEEn(MFALSE)
    , m_i4AETgValidBlockWidth(0)
    , m_i4AETgValidBlockHeight(0)
    , m_pPreviewTableCurrent(NULL)
    , m_pPreviewTableNew(NULL)
    , m_pCaptureTable(NULL)
    , m_pStrobeTable(NULL)
    , m_u4IndexMax(0)
    , m_u4IndexMin(0)
    , m_u4Index(0)
    , m_u4IndexFMax(0)
    , m_u4IndexFMin(0)
    , m_u4IndexF(0)
    , m_u4Prvflare(0)
    , m_u4FinerEVIdxBase(1)
    , m_u4LCEGainStartIdx(0)
    , m_u4LCEGainEndIdx(0)
    , m_i4PreviewMaxBV(0)
    , m_i4VideoMaxBV(0)
    , m_i4Video1MaxBV(0)
    , m_i4Video2MaxBV(0)
    , m_i4ZSDMaxBV(0)
    , m_i4DeltaBVIdx(0)
    , m_i4DeltaSensitivityIdx(0)
    , m_eAEManualPline(EAEManualPline_Default)
    , m_eAEManualCapPline(EAEManualPline_Default)
    , m_bStereoManualPline(MFALSE)
    , m_u4StereoDenoiseMode(0)
    , m_i4AEUnderExpDeltaBVIdx(0)
    , m_u4EISExpRatio(100)
    , m_u4Hbinwidth(0)
    , m_u4Hbinheight(0)
    , m_u4BlockNumX(AAO_STT_BLOCK_NUMX)
    , m_u4BlockNumY(AAO_STT_BLOCK_NUMY)
    , m_bEnableHDRYConfig(1)
    , m_bEnableOverCntConfig(1)
    , m_bEnableTSFConfig(1)
    , m_bDisablePixelHistConfig(0)
    , m_u4AEOverExpCntThs(255)
    , m_u4AEOverExpCntShift(0)
    , m_u4AEYCoefR(5)
    , m_u4AEYCoefG(9)
    , m_u4AEYCoefB(2)
    , m_pPSOStatBuf(NULL)
    , m_u4VHDRratio(0)
    , m_bVHDRChangeFlag(MFALSE)
    , m_bEnSWBuffMode(MFALSE)
    , m_i4SMSubSamples(1)
    , m_u4HFSMSkipFrame(0)
    , m_u4HDRcountforrevert(0)
    , m_i4AAOmode(0)
    , m_u4AEHDRMode(0)
    , m_bCCUAEFlag(MTRUE)
    , m_i4OBCTableidx(0)
    , m_i4ISP5OBCITableidx(0)
    , m_pCCUresultCB(NULL)
    , m_bAFTouchROISet(MFALSE)
    , m_i4ZoomRatio(0)
    , m_u4CCUResultCwvy(0)
    , m_u4CCUResultCwvyStable(0)
    , m_bAETrueLaunch(MTRUE)
    , m_bPlineMappingChange(MFALSE)
    , m_bGryoVd(MFALSE)
    , m_bAcceVd(MFALSE)
#if MTK_CAM_NEW_NVRAM_SUPPORT
    , m_pIdxMgr(IdxMgr::createInstance(static_cast <ESensorDev_T>(NS3Av3::mapSensorIdxToDev(i4SensorIndex))))
    , ISP_NVRAM_Ptr(NULL)
#endif
    , m_u4CCUManualShutter(0)
    , m_u4CCUManualAfeGain(0)
    , m_u4CCUManualIspGain(0)
    , m_u4CCUManualEnable(MFALSE)
    , m_bIsCCUAEInit(MFALSE)
    , m_u4CCUEndBankIdx(0)
    , m_u4MaxIdxStableCount(0)
    , m_u4ISOIdx1StableCount(0)
    , m_u4ISOIdx2StableCount(0)
    , m_u4Idx1StableThres(MAX_INDEX1_STABLE_THRES)
    , m_u4Idx2StableThres(MAX_INDEX2_STABLE_THRES)
    , m_u4ISOIdx1Value(ISO1_THRES_VALUE)
    , m_u4ISOIdx2Value(ISO2_THRES_VALUE)
    , m_u4ISO1IdxStatus(0)
    , m_u4ISO2IdxStatus(0)
    , m_bIsMaxIndexStable(0)
    , m_bIsCPUNeedDebugInfo(MFALSE)
    , m_u4FDProbForCCU(0)
    , m_u4FaceFailCntForCCU( -1)
    , m_u4FaceFoundCntForCCU(0)
    , mbIsPlineChange(MFALSE)
    , mbIsIndexChange(MFALSE)
    , m_u4CCUManShutter(0)
    , m_u4CCUManISO(0)
    , m_bUpdateAELockIdx(MFALSE)
    , m_bMVHDR3FlashUpdate(MFALSE)
    , m_bResetMvhdrRatio(MTRUE)
    , m_bForceUpdateAEInfoByCCU(MFALSE)
    , m_bForceSkipCCU(MFALSE)
    , m_bIsLongExpControlbyCCU(MTRUE)
    , m_bIsFrameRateLocked(MFALSE)
    , m_bForceResetCCUStable(MFALSE)
    , m_i4CurrSensorId(0)
    , m_i4SuperNightShotMode(0)
    , NeedCPUUpdateSensor(MFALSE)
    , mIsBackupTorch(MFALSE)
    , m_uManualModeIndex(0)
    , m_bEnableCustMode(0)
    , m_bNeedApplyBackupAEInfo(MFALSE)
{
    mEngFileName = new char[200];
    mEngFileName[0] = 0;

    memset(&m_AeMgrCCTConfig, 0, sizeof(AE_CCT_CFG_T));
    memset(&m_eZoomWinInfo, 0, sizeof(EZOOM_WINDOW_T));
    memset(&m_eAEMeterArea, 0, sizeof(AEMeteringArea_T));
    memset(&m_eAEFDArea, 0, sizeof(AEMeteringArea_T));
    memset(&m_CurrentPreviewTable, 0, sizeof(strAETable));
    memset(&m_CurrentCaptureTable, 0, sizeof(strAETable));
    memset(&m_CurrentStrobetureTable, 0, sizeof(strAETable));
    memset(&m_PreviewTableF, 0, sizeof(strFinerEvPline));
    memset(&m_CaptureTableF, 0, sizeof(strFinerEvPline));
    memset(&mPreviewMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&mPreviewModeBackup, 0, sizeof(AE_MODE_CFG_T));
    memset(&mPreviewModeBackupTorch, 0, sizeof(AE_MODE_CFG_T));
    memset(&mCaptureMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&m_strHDROutputInfo, 0, sizeof(Hal3A_HDROutputParam_T));
    memset(&m_strIVHDROutputSetting, 0, sizeof(IVHDRExpSettingOutputParam_T));
    memset(&m_strIVHDRCaptureSetting, 0, sizeof(IVHDRExpSettingOutputParam_T));
    memset(&m_backupMeterArea, 0, sizeof(CameraMeteringArea_T));
    memset(&m_rAEInitInput, 0, sizeof(AE_INITIAL_INPUT_T));
    memset(&m_rAEOutput, 0, sizeof(AE_OUTPUT_T));
    memset(&m_rAFAEOutput, 0, sizeof(strAEOutput));
    memset(&m_rAEStatCfg, 0, sizeof(AE_STAT_PARAM_T));
    memset(&m_rSensorResolution[0], 0, 2 * sizeof(SENSOR_RES_INFO_T));
    memset(&m_rAEPLineLimitation, 0, sizeof(AE_PLINE_LIMITATION_T));
    memset(&m_rSensorVCInfo, 0, sizeof(SENSOR_VC_INFO_T));
    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));
    memset(&m_rSensorCurrentInfo, 0, sizeof(AE_SENSOR_PARAM_T));
    memset(&m_rSensorCurrentInfoCCU, 0, sizeof(AE_SENSOR_PARAM_T));
    memset(&m_rSensorCurrentInfo_aelock, 0, sizeof(AE_SENSOR_PARAM_T));
    memset(&m_rSensorCurrentInfoCCU_aelock, 0, sizeof(AE_SENSOR_PARAM_T));
    memset(&m_rISPNvramISOInterval, 0, sizeof(ISP_NVRAM_ISO_INTERVAL_T));
    memset(&m_rISPNvramOBC_Table[0], 0, 4 * sizeof(CCU_ISP_NVRAM_OBC_T));
    memset(&m_rISP5NvramISOInterval, 0, sizeof(ISP_NVRAM_ISO_INTERVAL_T));
    memset(&m_rISP5NvramOBC_Table[0], 0, 20 * sizeof(CCU_ISP_NVRAM_OBC_T));
    memset(&m_strAAOProcInfo, 0, sizeof(AAO_PROC_INFO_T));
    memset(&m_AeCustParam, 0, sizeof(AE_Cust_Param_T));
    memset(&rAEInfo2ISPBack, 0, sizeof(AE_INFO_T));
    memset(&rAEDebugInfoBack, 0, sizeof(AE_DEBUG_INFO_T));

    m_i4StereoDenoiserto[0] = m_i4StereoDenoiserto[1] = 1000;
    m_backupMeterArea.u4Count = 1;
    m_isAeMeterAreaEn = 1;
    m_rAEPLineLimitation.u4IncreaseISO_x100 = 100;
    m_rAEPLineLimitation.u4IncreaseShutter_x100 = 100;
    m_pIAeTest = IAeTest::getInstance(m_eSensorDev);
    m_pIAeFlowCPU = IAeFlow::getInstance(IAeFlow::E_AE_FLOW_TYPE_DFT , m_eSensorDev);
    m_pIAeFlowCCU = IAeFlow::getInstance(IAeFlow::E_AE_FLOW_TYPE_CCU , m_eSensorDev);
    m_pIAeSettingCPU = IAeSetting::getInstance(IAeSetting::E_AE_SETTING_TYPE_DFT , m_eSensorDev);
    m_pIAeSettingCCU = IAeSetting::getInstance(IAeSetting::E_AE_SETTING_TYPE_CCU , m_eSensorDev);
    m_i4AcceInfo[0] = m_i4AcceInfo[1] = m_i4AcceInfo[2] = 0;
    m_i4GyroInfo[0] = m_i4GyroInfo[1] = m_i4GyroInfo[2] = 0;
    mpSensorProvider = nullptr;

    AE_LOG( "[AeMgr] sensorIdx(%d), sensorDev(%d)\n",m_i4SensorIdx,m_eSensorDev);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
~AeMgr()
{
    delete []mEngFileName;
    AE_LOG("[~AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::PreviewAEInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    MRESULT err;
    MINT32 i4SutterDelay=0, i4SensorGainDelay=0, i4IspGainDelay=0;

    // set sensor type
    m_i4SensorIdx = i4SensorIdx;

    // set sensor initial
    err = AAASensorMgr::getInstance().init(m_i4SensorIdx);
    if (FAILED(err))
    {
        CAM_LOGE("[%s()] AAASensorMgr::getInstance().init fail i4SensorDev:%d\n", __FUNCTION__, m_i4SensorIdx);
        return err;
    }

    AAASensorMgr::getInstance().getSensorSyncinfo(m_eSensorDev, &i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay, &m_i4SensorCaptureDelayFrame);
    AAASensorMgr::getInstance().getSensorMaxFrameRate(m_eSensorDev, m_rSensorMaxFrmRate);

    if ((i4SutterDelay != i4SensorGainDelay) && (!m_pIAeTest->IsPerframeCtrl()))
    {
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAINTHREAD);
        i4SensorGainDelay = i4SutterDelay;
    }

    if (i4IspGainDelay >= m_i4SensorCaptureDelayFrame)
    {
        m_i4TotalCaptureDelayFrame = i4IspGainDelay + 1;
    }
    else
    {
        m_i4TotalCaptureDelayFrame = m_i4SensorCaptureDelayFrame;
    }

    AE_LOG("[%s()] Long capture delay frame:%d %d %d\n", __FUNCTION__, m_i4TotalCaptureDelayFrame, m_i4SensorCaptureDelayFrame, i4IspGainDelay);

    if ((i4SutterDelay <= 5) && (i4SensorGainDelay <= 5) && (i4IspGainDelay <= 5))
    {
        m_i4ShutterDelayFrames = i4SutterDelay;
        m_i4SensorGainDelayFrames = i4SensorGainDelay;
        if (i4IspGainDelay >= 2)
        {
            m_i4IspGainDelayFrames = i4IspGainDelay - ISP_GAIN_DELAY_OFFSET; // for CQ0 2 delay frame
        }
        else
        {
            m_i4IspGainDelayFrames = 0;
        }

        AE_LOG("[%s()] SensorDelayInfo-> AeMgrDelayInfo (shutter/sensor gain/isp gain):%d/%d/%d->%d/%d/%d\n", __FUNCTION__,
            i4SutterDelay, i4SensorGainDelay, i4IspGainDelay, m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames);
    }
    else
    {
        AE_LOG("[%s()] Delay info is incorrectly:%d/%d/%d\n", __FUNCTION__, i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
        m_i4ShutterDelayFrames = 0;
        m_i4SensorGainDelayFrames = 0;
        m_i4IspGainDelayFrames = 0; // for CQ0 2 delay frame
    }

    // Get sensor resolution
    err = getSensorResolution();
    if (FAILED(err))
    {
        CAM_LOGE("[%s()] getSensorResolution() fail\n", __FUNCTION__);
        return err;
    }

    // Get NVRAM data
    err = getNvramData(m_eSensorDev);
    if (FAILED(err))
    {
        CAM_LOGE("[%s()] getNvramData() fail\n", __FUNCTION__);
        return err;
    }

    if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
    {
        err = AAASensorMgr::getInstance().getSensorVCinfo(m_eSensorDev, &m_rSensorVCInfo);
        if (FAILED(err))
        {
            CAM_LOGE("[%s()] getSensorVCinfo() fail\n", __FUNCTION__);
            return err;
        }
    }

    // Init AE
    err = AEInit(rParam);
    if (FAILED(err))
    {
        CAM_LOGE("[%s()] AEInit() fail\n", __FUNCTION__);
        return err;
    }

    #if CAM3_DISABLE_AE_PER_FRAME
    m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth = MFALSE;
    #endif
    AE_LOG("[%s()] m_eSensorDev:%d m_i4SensorIdx:%d AEconfig:%d/%d/%d/%d/%d/%d AE_PERFRAME_EN:%d m_bPerframeAEFlag:%d m_bTgIntAEEn:%d AEYCoefR/G/B:%d/%d/%d\n", __FUNCTION__,
        m_eSensorDev, m_i4SensorIdx, m_u4BlockNumX, m_u4BlockNumY,
        m_bEnableHDRYConfig, m_bEnableOverCntConfig, m_bEnableTSFConfig, m_bDisablePixelHistConfig,
        AE_PERFRAME_EN, m_bPerframeAEFlag, m_bTgIntAEEn,
        m_u4AEYCoefR, m_u4AEYCoefG, m_u4AEYCoefB);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    AE_LOG("[%s()] m_eSensorDev:%d i4SensorIdx:%d m_eSensorMode:%d m_eCamMode:%d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode, m_eAECamMode);

    m_bRealISOSpeed = 0;
    m_i4WaitVDNum = 0;

    // set usage
    m_i4EVIndex = 0;
    m_eAEEVcomp = LIB3A_AE_EV_COMP_00;
    // preset usage
    m_i4EVIndexPreset = 0;
    m_eAEEVcompPreset = LIB3A_AE_EV_COMP_00;

    u4FD_Prob = 0;

    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));

    switch (m_eCamMode)
    {
        case eAppMode_EngMode:      // Engineer Mode
            m_bRealISOSpeed = 1;
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        case eAppMode_ZsdMode:      // ZSD Mode
            m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
            break;
        case eAppMode_VideoMode:    // Video Mode
            m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
            break;
        case eAppMode_StereoMode:   // S3D Mode
        case eAppMode_PhotoMode:    // Photo Mode
        case eAppMode_DefaultMode:  // Default Mode
        case eAppMode_AtvMode:      // ATV Mode
        case eAppMode_VtMode:       // VT Mode
        case eAppMode_FactoryMode:
        default:
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae_mgr_ccu.enable", value, "1");
    m_bCCUAEFlag = atoi(value);

    property_get("vendor.debug.ae_loge.enable", value, "0");
    m_i4DgbLogE = atoi(value);

    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);

    property_get("vendor.debug.ae.vhdr.ratio", value, "0");
    m_u4VHDRratio = atoi(value);
    m_strIVHDROutputSetting.u4LE_SERatio_x100 = 0;

    property_get("vendor.debug.ae_mgr.enable", value, "0");
    m_bAEMgrDebugEnable = atoi(value);

    m_u4Idx1StableThres = property_get_int32("vendor.debug.idx1.stable.thres", MAX_INDEX1_STABLE_THRES);
    m_u4Idx2StableThres = property_get_int32("vendor.debug.idx2.stable.thres", MAX_INDEX2_STABLE_THRES);
    m_u4ISOIdx1Value = property_get_int32("vendor.debug.iso1.thres.value", ISO1_THRES_VALUE);
    m_u4ISOIdx2Value = property_get_int32("vendor.debug.iso2.thres.value", ISO2_THRES_VALUE);
    m_u4ISOIdx1StableCount = m_u4Idx1StableThres;
    m_u4ISOIdx2StableCount = m_u4Idx2StableThres;

    m_pIAeTest->AeTestInit();
    m_pIAeSettingCPU->init();
    PreviewAEInit(i4SensorIdx, rParam);
    m_AEState = AE_AUTO_FRAMERATE_STATE;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::Start()
{
    MRESULT err;
    AE_LOG("[%s] m_eSensorDev:%d m_i4SensorIdx:%d m_eSensorMode:%d m_eCamMode:%d m_eAECamMode:%d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_eSensorMode, m_eCamMode, m_eAECamMode);
    // Adb Manaul Pline Control
    MBOOL bAdbControlPline = FALSE;
    //MUINT32 u4AdbPreviewPline = 0; /* fix build warning */
    //MUINT32 u4AdbCapturePline = 0; /* fix build warning */
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.ae_pline.manual", value, "0");
    bAdbControlPline = atoi(value);
    if (bAdbControlPline)
    {
        m_eAEManualPline = EAEManualPline_ADBCtrol;
        AE_LOG("[%s] ADB assigns P-lineID manually, m_eAEManualPline:%d\n", __FUNCTION__, m_eAEManualPline);
    }
    m_i4WaitVDNum = 0;
    m_bRestoreAE = MFALSE;
    if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
    {
        m_bResetZHDRMode = MTRUE;
    }

    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));
    m_bRealISOSpeed = 0;
    mIsBackupTorch = MFALSE;

    // Define m_eCamMode
    switch (m_eCamMode)
    {
        case eAppMode_EngMode:      // Engineer Mode
            m_bRealISOSpeed = 1;
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        case eAppMode_ZsdMode:      // ZSD Mode
            m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
            break;
        case eAppMode_VideoMode:    // Video Mode
            m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
            break;
        case eAppMode_StereoMode:   // S3D Mode
        case eAppMode_PhotoMode:    // Photo Mode
        case eAppMode_DefaultMode:  // Default Mode
        case eAppMode_AtvMode:      // ATV Mode
        case eAppMode_VtMode:       // VT Mode
        case eAppMode_FactoryMode:
        default:
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
    }
    // Sensor max framerate will be changed when changing sensor mode
    AAASensorMgr::getInstance().getSensorMaxFrameRate(m_eSensorDev, m_rSensorMaxFrmRate);

    // Get sensor resolution
    err = getSensorResolution();
    if (FAILED(err))
    {
        CAM_LOGE("[%s] getSensorResolution() fail\n", __FUNCTION__);
        return err;
    }

    if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
    {
        err = AAASensorMgr::getInstance().getSensorVCinfo(m_eSensorDev, &m_rSensorVCInfo);
        if (FAILED(err))
        {
            CAM_LOGE("[%s] getSensorVCinfo() fail\n", __FUNCTION__);
            return err;
        }
    }
    // AEInit

    // Query TG info
    IHalSensorList * const pIHalSensorList = MAKE_HalSensorList();

    if(!pIHalSensorList) {
      CAM_LOGE("[%s] Can not create pIHalSensorList\n", __FUNCTION__);
      return E_AE_NULL_AE_TABLE;
    }

    IHalSensor * pIHalSensor = pIHalSensorList->createSensor("ae_mgr", m_i4SensorIdx);
    SensorDynamicInfo rSensorDynamicInfo;

    if (pIHalSensor)
    {
        pIHalSensor->querySensorDynamicInfo(m_eSensorDev, &rSensorDynamicInfo);
        pIHalSensor->destroyInstance("ae_mgr");
    }

    m_u4InitWaitCount = 0;
    memset(&m_eAEMeterArea, 0, sizeof(AEMeteringArea_T));
    m_eAEMeterArea.u4Count = 1;
    m_pIAeAlgo->setAEMeteringArea(&m_eAEMeterArea);
    AE_LOG_IF(m_3ALogEnable, "[%s] TG:%d pixel mode:%d, Clear Metering Area\n", __FUNCTION__, rSensorDynamicInfo.TgInfo, rSensorDynamicInfo.pixelMode);

    switch (rSensorDynamicInfo.TgInfo)
    {
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            //CCU dual cam enable
            // if (m_bCCUAEFlag)
            // {
            // m_pIAeFlowCCU->uninit();
            // m_bCCUAEFlag = MFALSE;
            // AE_LOG("[%s] CCU TG_2 disable", __FUNCTION__);
            // }
            break;
        default:
            CAM_LOGE("[%s] rSensorDynamicInfo.TgInfo = %d", __FUNCTION__, rSensorDynamicInfo.TgInfo);
            return MFALSE;
    }

    AE_SETTING_INPUT_INFO_T rAESetInput;
    prepareSetStartParam(&rAESetInput);
    m_pIAeSettingCPU->start((MVOID *) &rAESetInput);

    AE_LOG_IF(m_3ALogEnable, "[%s] m_eSensorDev:%d m_eSensorTG:%d m_i4SensorIdx:%d m_eISPProfile:%d\n", __FUNCTION__, m_eSensorDev, m_eSensorTG, m_i4SensorIdx, m_eISPProfile);
    //Update AE info to Algorithm
    m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);

    AAA_TRACE_D("StartAEStatConfig");
    // AE statistics and histogram config
    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);
    err = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).setAEconfigParam(m_u4BlockNumX, m_u4BlockNumY,
        m_bEnableHDRYConfig, m_bEnableOverCntConfig, m_bEnableTSFConfig,
        m_bDisablePixelHistConfig, m_u4AEOverExpCntThs, m_u4AEOverExpCntShift, m_i4AAOmode);
    err = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).setAEYCoefParam(m_u4AEYCoefR, m_u4AEYCoefG, m_u4AEYCoefB);
    err = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAEStatCfg);
    if (FAILED(err))
    {
        CAM_LOGE("[%s] AE state hist config() fail\n", __FUNCTION__);
        AAA_TRACE_END_D;
        return err;
    }
    AAA_TRACE_END_D;

    m_bAEStable = MFALSE;
    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
    mPreviewMode = m_rAEOutput.rPreviewMode;
    m_i4AEAutoFlickerMode = (MINT32)(m_eAEAutoFlickerMode);
    m_bInitWaitFaceSkipAE = MTRUE;
    m_u4InitWaitFaceCount = 0;
    m_bMVHDR3FlashUpdate = MFALSE;
    m_bNeedApplyBackupAEInfo = MFALSE;
    m_u4HDRcountforrevert = 0;
    m_bTouchAEAreaChage = MFALSE;
    m_bAFTouchROISet = MFALSE;

    if ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) || m_bVHDRChangeFlag)
    {
        if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
        {
            m_rAEInitInput.i4AEMaxBlockWidth = m_rSensorVCInfo.u2VCStatWidth;
            m_rAEInitInput.i4AEMaxBlockHeight = m_rSensorVCInfo.u2VCStatHeight;
        }
        else
        {
            m_rAEInitInput.i4AEMaxBlockWidth = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
            m_rAEInitInput.i4AEMaxBlockHeight = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        }
        m_rAEInitInput.u4AEISOSpeed = m_u4AEISOSpeed;
        m_rAEInitInput.eAETargetMode = m_eAETargetMode;
        m_rAEInitInput.eAEFlickerMode = m_eAEFlickerMode;
        m_rAEInitInput.eAEAutoFlickerMode = m_eAEAutoFlickerMode;

        strAEOutput rAEOutput;
        //MBOOL bEnablePreIndex = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex; /* fix build warning */
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = MTRUE;
        AE_LOG("[%s] reset mvhdr ratio:%d\n", __FUNCTION__, m_bResetMvhdrRatio);
        m_rAEInitInput.rAENVRAM->rCCTConfig.rAEAOENVRAMParam.i4AOEStrengthIdx = m_bResetMvhdrRatio;
        m_pIAeAlgo->initAE(&m_rAEInitInput, &rAEOutput, &m_rAEStatCfg);
        // Set FDarea to algorithm
        m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
        m_bResetMvhdrRatio = MTRUE;
        m_rAEInitInput.rAENVRAM->rCCTConfig.rAEAOENVRAMParam.i4AOEStrengthIdx = 1;
        if ((m_eAETargetMode != AE_MODE_IVHDR_TARGET) && (m_eAETargetMode != AE_MODE_MVHDR_TARGET) && (m_eAETargetMode != AE_MODE_ZVHDR_TARGET) && (m_eAETargetMode != AE_MODE_4CELL_MVHDR_TARGET))
            UpdateSensorISPParams(AE_INIT_STATE);
        else
            UpdateSensorISPParams(AE_TWO_SHUTTER_INIT_STATE);
        m_bVHDRChangeFlag = MFALSE;
    }
    else
    {
        UpdateSensorISPParams(AE_INIT_STATE);
    }
    m_AEState = AE_AUTO_FRAMERATE_STATE;

    if (m_pIAeAlgo != NULL)
    {
        getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable, m_CurrentStrobetureTable);
        AE_LOG_IF(m_3ALogEnable, "[%s] getPlineTable PreId:%d CapId:%d Strobe:%d\n", __FUNCTION__, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, m_CurrentStrobetureTable.eID);
    }
    MVOID * pObc = NULL;
    IspTuningMgr::getInstance().getDefaultObc(m_eSensorDev, pObc);
    ISP_NVRAM_Ptr = (ISP_NVRAM_REGISTER_STRUCT*)pObc;

    if (ISP_NVRAM_Ptr != NULL)
    {
        m_rISP5NvramISOInterval = ISP_NVRAM_Ptr->Iso_Env;
    }
    else
    {
        AE_LOG("[%s] ISP_NVRAM_Ptr NULL", __FUNCTION__);
    }

    MUINT32 u4OperationMode;
    property_get("vendor.camera.operationmode", value, "0");
    u4OperationMode = atoi(value);
    if (u4OperationMode == 1)
    {
        //define by Cust to disable CCU
        m_bEnableCustMode = MTRUE;
    }
    else
    {
        m_bEnableCustMode = MFALSE;
    }
    AE_LOG("[%s] Enable Cust mode:%d u4OperationMode:%d\n", __FUNCTION__, m_bEnableCustMode, u4OperationMode);

    // Control CPU Start
    AE_FLOW_CPUSTART_INFO_T flowstartInfo;
    prepareCPUStartParam(&flowstartInfo);
    m_pIAeFlowCPU->start((MVOID*) &flowstartInfo);
    // Control CCU start
    //m_i4AEMaxFps = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapMaxFrameRate * 10;
    AE_LOG("[%s] AE start reset:%d\n", __FUNCTION__, m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapMaxFrameRate);
    // in ae_mgr start, always trigger ccu sensor init, avoid leaving camera without reset flag
    m_pIAeFlowCCU->controltrigger(E_AE_FLOW_CCU_TRIGGER_SENSOR_INIT, MTRUE);
    if ((m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapMaxFrameRate <= 30) && (m_bRealISOSpeed == MFALSE) && (m_u4HDRcountforrevert < 1) && m_bCCUAEFlag && ShouldCCUStart())
    {
        AE_LOG_IF(m_3ALogEnable, "[%s] aeccu_AemgrStart\n", __FUNCTION__);
        AE_FLOW_INPUT_INFO_T initInfo;
        prepareCCUStartParam(&initInfo);
        m_pIAeFlowCCU->start((MVOID*) &initInfo);
        m_bIsCCUAEInit = MFALSE;
        m_bForceResetCCUStable = MFALSE;
    }
    m_bForceSkipCCU = MFALSE;

    char value2[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 TestPattern = 0;
    property_get("vendor.debug.ae.TestPattern", value2, "0");
    TestPattern = atoi(value2);
    if (TestPattern)
    {
        AAASensorMgr::getInstance().enableSensorTestPattern(m_eSensorDev);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::uninit()
{
    MRESULT err;

    err = AAASensorMgr::getInstance().uninit();
    if (FAILED(err))
    {
        CAM_LOGE("[%s] AAASensorMgr::getInstance().uninit fail\n", __FUNCTION__);
        return err;
    }
    m_pIAeSettingCPU->uninit();
    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->destroyInstance();
        m_pIAeAlgo = NULL;
    }
    // uninit Gyro
    if (mpSensorProvider != NULL)
    {
        mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
        mpSensorProvider->disableSensor(SENSOR_TYPE_ACCELERATION);
        mpSensorProvider = NULL;
    }
    if (m_pIAeFlowCCU)
    {
        m_pIAeFlowCCU->stop();
        m_pIAeFlowCCU->uninit();
    }
    // Save uninit AEtime
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    AEtimer = (unsigned long long)time.tv_sec * 1000000000 + (unsigned long long)time.tv_nsec;
    if (NumofUsers > 0)
        NumofUsers--;
    setAECamMode(eAppMode_PhotoMode);
    AE_LOG("[%s] AETimer %lld/%lld/%lld, NumOfUsers:%d Reset AE CamMode to PhotoMode:%d\n", __FUNCTION__, AEtimer, (unsigned long long)time.tv_sec, (unsigned long long)time.tv_nsec, NumofUsers, m_eCamMode);

    AE_LOG("[%s] reset m_eAEScene from %d to %d(LIB3A_AE_SCENE_AUTO)\n", __FUNCTION__, m_eAEScene, LIB3A_AE_SCENE_AUTO);
    m_eAEScene = LIB3A_AE_SCENE_AUTO;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::Stop()
{
    m_pIAeSettingCPU->stop();
    m_u4HwMagicNum = 0;

    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
        AE_LOG_IF(m_3ALogEnable, "[%s] AeMgr Stop CCU getDebugInfo\n", __FUNCTION__);
        m_pIAeFlowCCU->getdebuginfo(rAEDebugInfoBack);
        m_pIAeFlowCCU->pause();
    }
    else
    {
        m_pIAeAlgo->getDebugInfo(rAEDebugInfoBack);
    }

    AE_LOG("[%s] reset m_eAEScene from %d to %d(LIB3A_AE_SCENE_AUTO)\n", __FUNCTION__, m_eAEScene, LIB3A_AE_SCENE_AUTO);
    m_eAEScene = LIB3A_AE_SCENE_AUTO;

    AE_LOG("[%s] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AeMgr::setAeMeterAreaEn(int en)
{
    AE_LOG("[%s()] m_isAeMeterAreaEn:%d->%d\n", __FUNCTION__, m_isAeMeterAreaEn, en);
    m_isAeMeterAreaEn = en;
    setAEMeteringArea(&m_backupMeterArea);
}

MRESULT AeMgr::setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea)
{
    Mutex::Autolock lock(m_Lock);
    if (CUST_ENABLE_TOUCH_AE() == MFALSE)
    {
        AE_LOG("[%s()] i4SensorDev:%d line:%d The set face AE is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_TOUCH_AE());
        return S_AE_OK;
    }
    AE_LOG_IF(m_3ALogEnable, "[%s()] m_isAeMeterAreaEn:%d\n", __FUNCTION__, m_isAeMeterAreaEn);
    CameraMeteringArea_T meterArea;
    memcpy(&m_backupMeterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));
    memcpy(&meterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));

    if (m_isAeMeterAreaEn == 0)
    {
        memset(&meterArea, 0, sizeof(CameraMeteringArea_T));
        meterArea.u4Count = 1;
    }

    MUINT32 i;
    MBOOL bAreaChage = MFALSE;
    MUINT32 u4AreaCnt;
    CameraMeteringArea_T *sAEMeteringArea = &meterArea;

    if (sAEMeteringArea->u4Count <= 0)
    {
        return S_AE_OK;
    }
    else if (sAEMeteringArea->u4Count > MAX_METERING_AREAS)
    {
        CAM_LOGE("[%s()] The AE Metering area cnt error: %d\n", __FUNCTION__, sAEMeteringArea->u4Count);
        return E_AE_UNSUPPORT_MODE;
    }

    u4AreaCnt = sAEMeteringArea->u4Count;

    for (i = 0; i < u4AreaCnt; i++)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s():InComing] AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n", __FUNCTION__, i, sAEMeteringArea->rAreas[i].i4Left, sAEMeteringArea->rAreas[i].i4Right, sAEMeteringArea->rAreas[i].i4Top, sAEMeteringArea->rAreas[i].i4Bottom, sAEMeteringArea->rAreas[i].i4Weight);
        if ((sAEMeteringArea->rAreas[i].i4Right > sAEMeteringArea->rAreas[i].i4Left) &&
            (sAEMeteringArea->rAreas[i].i4Bottom > sAEMeteringArea->rAreas[i].i4Top) &&
            (m_u4Hbinwidth!=0 && m_u4Hbinheight!=0))
        {
            // Mapping AE region from Hbin size to [-1000, 1000]
            sAEMeteringArea->rAreas[i].i4Left   = sAEMeteringArea->rAreas[i].i4Left   * 2000 / m_u4Hbinwidth  - 1000;
            sAEMeteringArea->rAreas[i].i4Right  = sAEMeteringArea->rAreas[i].i4Right  * 2000 / m_u4Hbinwidth  - 1000;
            sAEMeteringArea->rAreas[i].i4Top    = sAEMeteringArea->rAreas[i].i4Top    * 2000 / m_u4Hbinheight - 1000;
            sAEMeteringArea->rAreas[i].i4Bottom = sAEMeteringArea->rAreas[i].i4Bottom * 2000 / m_u4Hbinheight - 1000;
            // check the count of side reaching boundary
            int counter = 0;
            // check if out-of-boundary
            if (sAEMeteringArea->rAreas[i].i4Left   <= -1000) {sAEMeteringArea->rAreas[i].i4Left   = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Right  <= -1000) {sAEMeteringArea->rAreas[i].i4Right  = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Top    <= -1000) {sAEMeteringArea->rAreas[i].i4Top    = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Bottom <= -1000) {sAEMeteringArea->rAreas[i].i4Bottom = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Left   >=  1000) {sAEMeteringArea->rAreas[i].i4Left   =  1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Right  >=  1000) {sAEMeteringArea->rAreas[i].i4Right  =  1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Top    >=  1000) {sAEMeteringArea->rAreas[i].i4Top    =  1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Bottom >=  1000) {sAEMeteringArea->rAreas[i].i4Bottom =  1000; counter++;}
            // if counter > 2, the touch area is wrong
            if (counter > 2)
            {
                AE_LOG_IF(m_3ALogEnable, "[%s()] touch area more than two sides reach boundary, counter:%d\n", __FUNCTION__, counter);
            }
            else if (sAEMeteringArea->rAreas[i].i4Left != m_eAEMeterArea.rAreas[i].i4Left ||
                     sAEMeteringArea->rAreas[i].i4Right  != m_eAEMeterArea.rAreas[i].i4Right ||
                     sAEMeteringArea->rAreas[i].i4Top  != m_eAEMeterArea.rAreas[i].i4Top ||
                     sAEMeteringArea->rAreas[i].i4Bottom != m_eAEMeterArea.rAreas[i].i4Bottom)
            {
                AE_LOG("[%s():Previous] AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n",
                    __FUNCTION__, i, m_eAEMeterArea.rAreas[i].i4Left, m_eAEMeterArea.rAreas[i].i4Right, m_eAEMeterArea.rAreas[i].i4Top, m_eAEMeterArea.rAreas[i].i4Bottom, m_eAEMeterArea.rAreas[i].i4Weight);
                AE_LOG("[%s():Modified] AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n",
                    __FUNCTION__, i, sAEMeteringArea->rAreas[i].i4Left, sAEMeteringArea->rAreas[i].i4Right, sAEMeteringArea->rAreas[i].i4Top, sAEMeteringArea->rAreas[i].i4Bottom, sAEMeteringArea->rAreas[i].i4Weight);
                m_eAEMeterArea.rAreas[i].i4Left   = sAEMeteringArea->rAreas[i].i4Left;
                m_eAEMeterArea.rAreas[i].i4Right  = sAEMeteringArea->rAreas[i].i4Right;
                m_eAEMeterArea.rAreas[i].i4Top    = sAEMeteringArea->rAreas[i].i4Top;
                m_eAEMeterArea.rAreas[i].i4Bottom = sAEMeteringArea->rAreas[i].i4Bottom;
                m_eAEMeterArea.rAreas[i].i4Weight = sAEMeteringArea->rAreas[i].i4Weight;
                bAreaChage = MTRUE;
            }
        }
        else
        {
            m_bAFTouchROISet = MFALSE;
        }
    }
    if (bAreaChage == MTRUE)
    {
        m_bTouchAEAreaChage = MTRUE;
        m_bAFTouchROISet = MTRUE;
        m_eAEMeterArea.u4Count = u4AreaCnt;
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->setAEMeteringArea(&m_eAEMeterArea);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d TouchAEAreaChage:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, m_bTouchAEAreaChage);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setFDenable(MBOOL bFDenable)
{
    if (bFDenable != m_bFDenable)
    {
        AE_LOG("[%s()] FDenable:%d->%d\n", __FUNCTION__, m_bFDenable, bFDenable);
        m_bFDenable = bFDenable;
        m_bFaceAEAreaChage = MTRUE;
        m_u4WOFDCnt = 0;
    }
    return S_AE_OK;
}

MRESULT AeMgr::getAEFDMeteringAreaInfo(android::Vector <MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    vecOut.clear();
    MINT32 i4totalnumber = m_backupMeterArea.u4Count + m_eAEFDArea.u4Count;
    MINT32 u4AETouchCnt = m_backupMeterArea.u4Count;
    MINT32 u4AEFDCnt = m_eAEFDArea.u4Count;
    /*
     * The order of vector vecOut is :
     * type(0: Face AE) -> number of ROI -> left of ROI1 -> top of ROI1 -> right of ROI1 -> bottom of ROI1 -> result of ROI1 -> left of ROI2 -> top of ROI2 -> right of ROI2 -> bottom of ROI2 -> result of ROI2 -> etc.
     */
    MBOOL bAETouchAreaRef = MFALSE;
    MBOOL bAEFDAreaRef = MFALSE;
    #if 1 //wait algo sync
    m_pIAeAlgo->getTouchIsReferencedFlag(bAETouchAreaRef);
    m_pIAeAlgo->getFaceIsReferencedFlag(bAEFDAreaRef);
    #endif
    if (bAETouchAreaRef == MFALSE)
        u4AETouchCnt = 0;
    if (bAEFDAreaRef == MFALSE)
        u4AEFDCnt = 0;

    i4totalnumber = u4AETouchCnt + u4AEFDCnt;
    if (i4totalnumber != 0)
    {
        MINT32 type = 0;
        vecOut.push_back(type);
        MINT32 number = static_cast <MINT32> (i4totalnumber);
        vecOut.push_back(number);
        // Touch AE metering area info
        for (MINT32 ifaceIdx = 0; ifaceIdx < u4AETouchCnt; ifaceIdx++)
        {
            MINT32 i4Left   = (m_eAEMeterArea.rAreas[ifaceIdx].i4Left   + 1000) * i4tgwidth  / 2000;
            MINT32 i4Right  = (m_eAEMeterArea.rAreas[ifaceIdx].i4Right  + 1000) * i4tgwidth  / 2000;
            MINT32 i4Top    = (m_eAEMeterArea.rAreas[ifaceIdx].i4Top    + 1000) * i4tgheight / 2000;
            MINT32 i4Bottom = (m_eAEMeterArea.rAreas[ifaceIdx].i4Bottom + 1000) * i4tgheight / 2000;
            MINT32 i4result = 0;

            vecOut.push_back(i4Left);
            vecOut.push_back(i4Top);
            vecOut.push_back(i4Right);
            vecOut.push_back(i4Bottom);
            vecOut.push_back(i4result);
        }
        // FD AE metering area info
        for (MINT32 ifaceIdx = 0; ifaceIdx < u4AEFDCnt; ifaceIdx++)
        {
            MINT32 i4Left   = (m_eAEFDArea.rAreas[ifaceIdx].i4Left   + 1000) * i4tgwidth  / 2000;
            MINT32 i4Right  = (m_eAEFDArea.rAreas[ifaceIdx].i4Right  + 1000) * i4tgwidth  / 2000;
            MINT32 i4Top    = (m_eAEFDArea.rAreas[ifaceIdx].i4Top    + 1000) * i4tgheight / 2000;
            MINT32 i4Bottom = (m_eAEFDArea.rAreas[ifaceIdx].i4Bottom + 1000) * i4tgheight / 2000;
            MINT32 i4result = 0;

            vecOut.push_back(i4Left);
            vecOut.push_back(i4Top);
            vecOut.push_back(i4Right);
            vecOut.push_back(i4Bottom);
            vecOut.push_back(i4result);
        }
    }
    else
    {
        MINT32 type = 0;
        vecOut.push_back(type);

        MINT32 number = 0;
        vecOut.push_back(number);
    }
    return S_AE_OK;
}

MRESULT AeMgr::setFDInfo(MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    Mutex::Autolock lock(m_Lock);
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    MINT8 ifaceIdx = 0;

    if (CUST_ENABLE_FACE_AE() == MTRUE)
    {
        if (pFaces == NULL)
        {
            AE_LOG("[%s()] FD window is NULL pointer 1\n", __FUNCTION__);
            return S_AF_OK;
        }

        /* comment this since the coverity issue
        if ((pFaces->faces) == NULL)
        {
            AE_LOG("[%s()] FD window is NULL pointer 2\n", __FUNCTION__);
            return S_AF_OK;
        }
        */

        //First time with face to without face
        if (m_eAEFDArea.u4Count && (pFaces->number_of_faces == 0))
        {
            m_bFaceAEAreaChage = MTRUE;
            m_u4WOFDCnt = 0;
            AE_LOG("[%s()] First frame without face Count %d ->%d\n", __FUNCTION__, m_eAEFDArea.u4Count, pFaces->number_of_faces);
        }
        else if ((m_eAEFDArea.u4Count == 0) && (pFaces->number_of_faces != 0))
        {
            //First time without face to with face
            memset(&m_eAEMeterArea, 0, sizeof(AEMeteringArea_T));
            m_eAEMeterArea.u4Count = 1;
            m_pIAeAlgo->setAEMeteringArea(&m_eAEMeterArea);
            m_bTouchAEAreaChage = MFALSE;
            m_bAFTouchROISet = MFALSE;
            AE_LOG("[%s()] First frame with face, Clear Metering Area\n", __FUNCTION__);
        }

        AEMeteringArea_T eAEFDArea;
        memset(&eAEFDArea, 0, sizeof(AEMeteringArea_T));

        if (m_bFDenable == MTRUE)
        {
            // Define FD area number

            if (pFaces->number_of_faces > MAX_AE_METER_AREAS)
            {
                eAEFDArea.u4Count = MAX_AE_METER_AREAS;
            }
            else
            {
                eAEFDArea.u4Count = pFaces->number_of_faces;
            }

            eAEFDArea.i8TimeStamp = pFaces->timestamp;

            for (ifaceIdx = 0; (MUINT32)ifaceIdx < eAEFDArea.u4Count; ifaceIdx++)
            {
                eAEFDArea.rAreas[ifaceIdx].i4Left   = pFaces->faces[ifaceIdx].rect[0];
                eAEFDArea.rAreas[ifaceIdx].i4Right  = pFaces->faces[ifaceIdx].rect[2];
                eAEFDArea.rAreas[ifaceIdx].i4Top    = pFaces->faces[ifaceIdx].rect[1];
                eAEFDArea.rAreas[ifaceIdx].i4Bottom = pFaces->faces[ifaceIdx].rect[3];
                eAEFDArea.rAreas[ifaceIdx].i4Weight = 1;
                // new FD info +
                eAEFDArea.rAreas[ifaceIdx].i4Id = pFaces->faces[ifaceIdx].id;
                eAEFDArea.rAreas[ifaceIdx].i4Type = pFaces->faces_type[ifaceIdx];
                eAEFDArea.rAreas[ifaceIdx].i4LandMarkRip = pFaces->fld_rip[ifaceIdx];
                eAEFDArea.rAreas[ifaceIdx].i4LandMarkRop = pFaces->fld_rop[ifaceIdx];
                eAEFDArea.rAreas[ifaceIdx].i4ROP = pFaces->posInfo[ifaceIdx].rop_dir;
                eAEFDArea.rAreas[ifaceIdx].i4Motion[0] = pFaces->motion[ifaceIdx][0];
                eAEFDArea.rAreas[ifaceIdx].i4Motion[1] = pFaces->motion[ifaceIdx][1];
                // landmark cv
                eAEFDArea.rAreas[ifaceIdx].i4LandmarkCV = pFaces->fa_cv[ifaceIdx];
                if (pFaces->fa_cv[ifaceIdx] > 0)
                {
                    // left eye
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][0] = pFaces->leyex0[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][1] = pFaces->leyey0[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][2] = pFaces->leyex1[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][3] = pFaces->leyey1[ifaceIdx];
                    // right eye
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][0] = pFaces->reyex0[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][1] = pFaces->reyey0[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][2] = pFaces->reyex1[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][3] = pFaces->reyey1[ifaceIdx];
                    // mouth
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][0] = pFaces->mouthx0[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][1] = pFaces->mouthy0[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][2] = pFaces->mouthx1[ifaceIdx];
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][3] = pFaces->mouthy1[ifaceIdx];
                }
                // FD Area Info
                eAEFDArea.rAreas[ifaceIdx].i4Left   = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Left   * 2000 / (i4tgwidth);
                eAEFDArea.rAreas[ifaceIdx].i4Right  = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Right  * 2000 / (i4tgwidth);
                eAEFDArea.rAreas[ifaceIdx].i4Top    = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Top    * 2000 / (i4tgheight);
                eAEFDArea.rAreas[ifaceIdx].i4Bottom = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Bottom * 2000 / (i4tgheight);

                if (pFaces->fa_cv[ifaceIdx] > 0)
                {
                    // left eye
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][0] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][0] * 2000 / (i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][1] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][1] * 2000 / (i4tgheight);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][2] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][2] * 2000 / (i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][3] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][3] * 2000 / (i4tgheight);
                    // right eye
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][0] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][0] * 2000 / (i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][1] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][1] * 2000 / (i4tgheight);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][2] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][2] * 2000 / (i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][3] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][3] * 2000 / (i4tgheight);
                    // mouth
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][0] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][0] * 2000 / (i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][1] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][1] * 2000 / (i4tgheight);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][2] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][2] * 2000 / (i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][3] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][3] * 2000 / (i4tgheight);
                }

                AE_LOG("[%s():Modified] AE FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d TGWidth:%d TGHeight:%d\n", __FUNCTION__, ifaceIdx,
                    eAEFDArea.rAreas[ifaceIdx].i4Left, eAEFDArea.rAreas[ifaceIdx].i4Right,
                    eAEFDArea.rAreas[ifaceIdx].i4Top, eAEFDArea.rAreas[ifaceIdx].i4Bottom , i4tgwidth, i4tgheight);
                AE_LOG("[%s():Previous] AE FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d \n", __FUNCTION__, ifaceIdx,
                    pFaces->faces[ifaceIdx].rect[0], pFaces->faces[ifaceIdx].rect[2],
                    pFaces->faces[ifaceIdx].rect[1] , pFaces->faces[ifaceIdx].rect[3]);

                m_bFaceAEAreaChage = MTRUE;
            }

            if (eAEFDArea.u4Count > 0)
            {
                if ((m_eAEFDArea.rAreas[0].i4Left   != eAEFDArea.rAreas[0].i4Left)  ||
                    (m_eAEFDArea.rAreas[0].i4Right  != eAEFDArea.rAreas[0].i4Right) ||
                    (m_eAEFDArea.rAreas[0].i4Top    != eAEFDArea.rAreas[0].i4Top)   ||
                    (m_eAEFDArea.rAreas[0].i4Bottom != eAEFDArea.rAreas[0].i4Bottom))
                {
                    m_u4WOFDCnt = 0;
                }
            }
            memcpy(&m_eAEFDArea, &eAEFDArea, sizeof(AEMeteringArea_T));
            m_bInitWaitFaceSkipAE = MFALSE;
            AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d line:%d Face Number:%d FaceAEChange:%d m_bInitWaitFaceSkipAE = MFALSE", __FUNCTION__, m_eSensorDev, __LINE__, eAEFDArea.u4Count, m_bFaceAEAreaChage);
        }
    }
    else
    {
        AE_LOG("[%s()] i4SensorDev:%d line:%d The set face AE is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_FACE_AE());
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setOTInfo(MVOID* a_sOT)
{
    MtkCameraFaceMetadata *pOTWindow = (MtkCameraFaceMetadata *)a_sOT;
    MBOOL bEnableObjectTracking;

    if (m_i4ObjectTrackNum != pOTWindow->number_of_faces)
    {
        AE_LOG("[%s()] Object tracking:%d %d\n", __FUNCTION__, pOTWindow->number_of_faces, m_i4ObjectTrackNum);
        m_i4ObjectTrackNum = pOTWindow->number_of_faces;
        if (pOTWindow->number_of_faces > 0)
        {
            // Object tracking enable
            bEnableObjectTracking = MTRUE;
        }
        else
        {
            bEnableObjectTracking = MFALSE;
        }

        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->setAEObjectTracking(bEnableObjectTracking);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL (2)\n", __FUNCTION__);
        }
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep, MBOOL is_preset)
{
    MINT32 i4EVValue = 0, i4EVIndex = 0;
    LIB3A_AE_EVCOMP_T eAEEVcomp;

    // seperate set & preset member
    if (is_preset)
    {
        i4EVIndex = m_i4EVIndexPreset;
        eAEEVcomp = m_eAEEVcompPreset;
    }
    else
    {
        i4EVIndex = m_i4EVIndex;
        eAEEVcomp = m_eAEEVcomp;
    }

    if (i4EVIndex != i4NewEVIndex)
    {
        i4EVIndex = i4NewEVIndex;
        m_fEVCompStep = fStep;
        i4EVValue = (MINT32) (100 * m_fEVCompStep * i4NewEVIndex);

        if(i4EVValue < -590) { eAEEVcomp = LIB3A_AE_EV_COMP_n60; }
        else if(i4EVValue < -580) { eAEEVcomp = LIB3A_AE_EV_COMP_n59; }
        else if(i4EVValue < -570) { eAEEVcomp = LIB3A_AE_EV_COMP_n58; }
        else if(i4EVValue < -560) { eAEEVcomp = LIB3A_AE_EV_COMP_n57; }
        else if(i4EVValue < -550) { eAEEVcomp = LIB3A_AE_EV_COMP_n56; }
        else if(i4EVValue < -540) { eAEEVcomp = LIB3A_AE_EV_COMP_n55; }
        else if(i4EVValue < -530) { eAEEVcomp = LIB3A_AE_EV_COMP_n54; }
        else if(i4EVValue < -520) { eAEEVcomp = LIB3A_AE_EV_COMP_n53; }
        else if(i4EVValue < -510) { eAEEVcomp = LIB3A_AE_EV_COMP_n52; }
        else if(i4EVValue < -500) { eAEEVcomp = LIB3A_AE_EV_COMP_n51; }
        else if(i4EVValue < -490) { eAEEVcomp = LIB3A_AE_EV_COMP_n50; }
        else if(i4EVValue < -480) { eAEEVcomp = LIB3A_AE_EV_COMP_n49; }
        else if(i4EVValue < -470) { eAEEVcomp = LIB3A_AE_EV_COMP_n48; }
        else if(i4EVValue < -460) { eAEEVcomp = LIB3A_AE_EV_COMP_n47; }
        else if(i4EVValue < -450) { eAEEVcomp = LIB3A_AE_EV_COMP_n46; }
        else if(i4EVValue < -440) { eAEEVcomp = LIB3A_AE_EV_COMP_n45; }
        else if(i4EVValue < -430) { eAEEVcomp = LIB3A_AE_EV_COMP_n44; }
        else if(i4EVValue < -420) { eAEEVcomp = LIB3A_AE_EV_COMP_n43; }
        else if(i4EVValue < -410) { eAEEVcomp = LIB3A_AE_EV_COMP_n42; }
        else if(i4EVValue < -400) { eAEEVcomp = LIB3A_AE_EV_COMP_n41; }
        else if(i4EVValue < -390) { eAEEVcomp = LIB3A_AE_EV_COMP_n40; }
        else if(i4EVValue < -380) { eAEEVcomp = LIB3A_AE_EV_COMP_n39; }
        else if(i4EVValue < -370) { eAEEVcomp = LIB3A_AE_EV_COMP_n38; }
        else if(i4EVValue < -360) { eAEEVcomp = LIB3A_AE_EV_COMP_n37; }
        else if(i4EVValue < -350) { eAEEVcomp = LIB3A_AE_EV_COMP_n36; }
        else if(i4EVValue < -340) { eAEEVcomp = LIB3A_AE_EV_COMP_n35; }
        else if(i4EVValue < -330) { eAEEVcomp = LIB3A_AE_EV_COMP_n34; }
        else if(i4EVValue < -320) { eAEEVcomp = LIB3A_AE_EV_COMP_n33; }
        else if(i4EVValue < -310) { eAEEVcomp = LIB3A_AE_EV_COMP_n32; }
        else if(i4EVValue < -300) { eAEEVcomp = LIB3A_AE_EV_COMP_n31; }
        else if(i4EVValue < -290) { eAEEVcomp = LIB3A_AE_EV_COMP_n30; }
        else if(i4EVValue < -280) { eAEEVcomp = LIB3A_AE_EV_COMP_n29; }
        else if(i4EVValue < -270) { eAEEVcomp = LIB3A_AE_EV_COMP_n28; }
        else if(i4EVValue < -260) { eAEEVcomp = LIB3A_AE_EV_COMP_n27; }
        else if(i4EVValue < -250) { eAEEVcomp = LIB3A_AE_EV_COMP_n26; }
        else if(i4EVValue < -240) { eAEEVcomp = LIB3A_AE_EV_COMP_n25; }
        else if(i4EVValue < -230) { eAEEVcomp = LIB3A_AE_EV_COMP_n24; }
        else if(i4EVValue < -220) { eAEEVcomp = LIB3A_AE_EV_COMP_n23; }
        else if(i4EVValue < -210) { eAEEVcomp = LIB3A_AE_EV_COMP_n22; }
        else if(i4EVValue < -200) { eAEEVcomp = LIB3A_AE_EV_COMP_n21; }
        else if(i4EVValue < -190) { eAEEVcomp = LIB3A_AE_EV_COMP_n20; }
        else if(i4EVValue < -180) { eAEEVcomp = LIB3A_AE_EV_COMP_n19; }
        else if(i4EVValue < -170) { eAEEVcomp = LIB3A_AE_EV_COMP_n18; }
        else if(i4EVValue < -160) { eAEEVcomp = LIB3A_AE_EV_COMP_n17; }
        else if(i4EVValue < -150) { eAEEVcomp = LIB3A_AE_EV_COMP_n16; }
        else if(i4EVValue < -140) { eAEEVcomp = LIB3A_AE_EV_COMP_n15; }
        else if(i4EVValue < -130) { eAEEVcomp = LIB3A_AE_EV_COMP_n14; }
        else if(i4EVValue < -120) { eAEEVcomp = LIB3A_AE_EV_COMP_n13; }
        else if(i4EVValue < -110) { eAEEVcomp = LIB3A_AE_EV_COMP_n12; }
        else if(i4EVValue < -100) { eAEEVcomp = LIB3A_AE_EV_COMP_n11; }
        else if(i4EVValue < -90) { eAEEVcomp = LIB3A_AE_EV_COMP_n10; }
        else if(i4EVValue < -80) { eAEEVcomp = LIB3A_AE_EV_COMP_n09; }
        else if(i4EVValue < -70) { eAEEVcomp = LIB3A_AE_EV_COMP_n08; }
        else if(i4EVValue < -60) { eAEEVcomp = LIB3A_AE_EV_COMP_n07; }
        else if(i4EVValue < -50) { eAEEVcomp = LIB3A_AE_EV_COMP_n06; }
        else if(i4EVValue < -40) { eAEEVcomp = LIB3A_AE_EV_COMP_n05; }
        else if(i4EVValue < -30) { eAEEVcomp = LIB3A_AE_EV_COMP_n04; }
        else if(i4EVValue < -20) { eAEEVcomp = LIB3A_AE_EV_COMP_n03; }
        else if(i4EVValue < -10) { eAEEVcomp = LIB3A_AE_EV_COMP_n02; }
        else if(i4EVValue <   0) { eAEEVcomp = LIB3A_AE_EV_COMP_n01; }
        else if(i4EVValue == 0) { eAEEVcomp = LIB3A_AE_EV_COMP_00; }
        else if(i4EVValue < 20) { eAEEVcomp = LIB3A_AE_EV_COMP_01; }
        else if(i4EVValue < 30) { eAEEVcomp = LIB3A_AE_EV_COMP_02; }
        else if(i4EVValue < 40) { eAEEVcomp = LIB3A_AE_EV_COMP_03; }
        else if(i4EVValue < 50) { eAEEVcomp = LIB3A_AE_EV_COMP_04; }
        else if(i4EVValue < 60) { eAEEVcomp = LIB3A_AE_EV_COMP_05; }
        else if(i4EVValue < 70) { eAEEVcomp = LIB3A_AE_EV_COMP_06; }
        else if(i4EVValue < 80) { eAEEVcomp = LIB3A_AE_EV_COMP_07; }
        else if(i4EVValue < 90) { eAEEVcomp = LIB3A_AE_EV_COMP_08; }
        else if(i4EVValue < 100) { eAEEVcomp = LIB3A_AE_EV_COMP_09; }
        else if(i4EVValue < 110) { eAEEVcomp = LIB3A_AE_EV_COMP_10; }
        else if(i4EVValue < 120) { eAEEVcomp = LIB3A_AE_EV_COMP_11; }
        else if(i4EVValue < 130) { eAEEVcomp = LIB3A_AE_EV_COMP_12; }
        else if(i4EVValue < 140) { eAEEVcomp = LIB3A_AE_EV_COMP_13; }
        else if(i4EVValue < 150) { eAEEVcomp = LIB3A_AE_EV_COMP_14; }
        else if(i4EVValue < 160) { eAEEVcomp = LIB3A_AE_EV_COMP_15; }
        else if(i4EVValue < 170) { eAEEVcomp = LIB3A_AE_EV_COMP_16; }
        else if(i4EVValue < 180) { eAEEVcomp = LIB3A_AE_EV_COMP_17; }
        else if(i4EVValue < 190) { eAEEVcomp = LIB3A_AE_EV_COMP_18; }
        else if(i4EVValue < 200) { eAEEVcomp = LIB3A_AE_EV_COMP_19; }
        else if(i4EVValue < 210) { eAEEVcomp = LIB3A_AE_EV_COMP_20; }
        else if(i4EVValue < 220) { eAEEVcomp = LIB3A_AE_EV_COMP_21; }
        else if(i4EVValue < 230) { eAEEVcomp = LIB3A_AE_EV_COMP_22; }
        else if(i4EVValue < 240) { eAEEVcomp = LIB3A_AE_EV_COMP_23; }
        else if(i4EVValue < 250) { eAEEVcomp = LIB3A_AE_EV_COMP_24; }
        else if(i4EVValue < 260) { eAEEVcomp = LIB3A_AE_EV_COMP_25; }
        else if(i4EVValue < 270) { eAEEVcomp = LIB3A_AE_EV_COMP_26; }
        else if(i4EVValue < 280) { eAEEVcomp = LIB3A_AE_EV_COMP_27; }
        else if(i4EVValue < 290) { eAEEVcomp = LIB3A_AE_EV_COMP_28; }
        else if(i4EVValue < 300) { eAEEVcomp = LIB3A_AE_EV_COMP_29; }
        else if(i4EVValue < 310) { eAEEVcomp = LIB3A_AE_EV_COMP_30; }
        else if(i4EVValue < 320) { eAEEVcomp = LIB3A_AE_EV_COMP_31; }
        else if(i4EVValue < 330) { eAEEVcomp = LIB3A_AE_EV_COMP_32; }
        else if(i4EVValue < 340) { eAEEVcomp = LIB3A_AE_EV_COMP_33; }
        else if(i4EVValue < 350) { eAEEVcomp = LIB3A_AE_EV_COMP_34; }
        else if(i4EVValue < 360) { eAEEVcomp = LIB3A_AE_EV_COMP_35; }
        else if(i4EVValue < 370) { eAEEVcomp = LIB3A_AE_EV_COMP_36; }
        else if(i4EVValue < 380) { eAEEVcomp = LIB3A_AE_EV_COMP_37; }
        else if(i4EVValue < 390) { eAEEVcomp = LIB3A_AE_EV_COMP_38; }
        else if(i4EVValue < 400) { eAEEVcomp = LIB3A_AE_EV_COMP_39; }
        else { eAEEVcomp = LIB3A_AE_EV_COMP_40;  }

        // seperate set & preset member
        if (is_preset)
        {
            m_i4EVIndexPreset = i4EVIndex;
            m_eAEEVcompPreset = eAEEVcomp;
        }
        else
        {
            m_i4EVIndex = i4EVIndex;
            m_eAEEVcomp = eAEEVcomp;
        }

        m_bForceResetCCUStable = MTRUE;
        m_bAEStable = MFALSE;

        //AE_LOG("[%s()] m_i4EVIndex:%d EVComp:%d fEVCompStep:%f PreAEState:%d m_bAELock:%d m_bAEStable:%d, force reset CCU stable\n", __FUNCTION__, m_i4EVIndex, m_eAEEVcomp, m_fEVCompStep, m_ePreAEState, m_bAELock, m_bAEStable);
        AE_LOG("[%s()] m_i4EVIndex(set/preset):%d/%d m_eAEEVcomp(set/preset):%d/%d m_fEVCompStep:%f m_bAELock:%d is_preset:%d\n", __FUNCTION__,
            m_i4EVIndex, m_i4EVIndexPreset, m_eAEEVcomp, m_eAEEVcompPreset, m_fEVCompStep, m_bAELock, is_preset);

        if (!is_preset)
        {
            // AE Auto Test
            m_pIAeTest->recordAvgY(IAeTest::AE_AT_TYPE_EVCOMP, m_pIAeAlgo->getBrightnessAverageValue());
            m_pIAeAlgo->setEVCompensate(m_eAEEVcomp);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_COUNTWAIT, MTRUE);
            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
            if (m_bAELock)
            {
                m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
                m_i4WaitVDNum = (m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1);
                prepareCapParams(MFALSE,MTRUE);
                //AE_LOG("[%s()] EV compensation with AE lock, prepare capture param in advance\n", __FUNCTION__);
                AE_LOG("[%s()] EV compensation with AE lock, prepare capture param in advance at set timing\n", __FUNCTION__);
            }
        }
    }

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getEVCompensateIndex()
{
    MINT32 iEVIndex;

    switch (m_eAEEVcomp)
    {
        case LIB3A_AE_EV_COMP_03:  { iEVIndex =   3; break; }
        case LIB3A_AE_EV_COMP_05:  { iEVIndex =   5; break; }
        case LIB3A_AE_EV_COMP_07:  { iEVIndex =   7; break; }
        case LIB3A_AE_EV_COMP_10:  { iEVIndex =  10; break; }
        case LIB3A_AE_EV_COMP_13:  { iEVIndex =  13; break; }
        case LIB3A_AE_EV_COMP_15:  { iEVIndex =  15; break; }
        case LIB3A_AE_EV_COMP_17:  { iEVIndex =  17; break; }
        case LIB3A_AE_EV_COMP_20:  { iEVIndex =  20; break; }
        case LIB3A_AE_EV_COMP_25:  { iEVIndex =  25; break; }
        case LIB3A_AE_EV_COMP_30:  { iEVIndex =  30; break; }
        case LIB3A_AE_EV_COMP_35:  { iEVIndex =  35; break; }
        case LIB3A_AE_EV_COMP_40:  { iEVIndex =  40; break; }
        case LIB3A_AE_EV_COMP_n03: { iEVIndex =  -3; break; }
        case LIB3A_AE_EV_COMP_n05: { iEVIndex =  -5; break; }
        case LIB3A_AE_EV_COMP_n07: { iEVIndex =  -7; break; }
        case LIB3A_AE_EV_COMP_n10: { iEVIndex = -10; break; }
        case LIB3A_AE_EV_COMP_n13: { iEVIndex = -13; break; }
        case LIB3A_AE_EV_COMP_n15: { iEVIndex = -15; break; }
        case LIB3A_AE_EV_COMP_n17: { iEVIndex = -17; break; }
        case LIB3A_AE_EV_COMP_n20: { iEVIndex = -20; break; }
        case LIB3A_AE_EV_COMP_n25: { iEVIndex = -25; break; }
        case LIB3A_AE_EV_COMP_n30: { iEVIndex = -30; break; }
        case LIB3A_AE_EV_COMP_n35: { iEVIndex = -35; break; }
        case LIB3A_AE_EV_COMP_n40: { iEVIndex = -40; break; }
        default:
        case LIB3A_AE_EV_COMP_00:
            iEVIndex = 0;
            break;
    }
    return iEVIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMeteringMode(MUINT32 u4NewAEMeteringMode)
{
    AE_METERING_T eNewAEMeteringMode = static_cast <AE_METERING_T> (u4NewAEMeteringMode);
    LIB3A_AE_METERING_MODE_T eAEMeteringMode;

    if ((eNewAEMeteringMode < AE_METERING_BEGIN) || (eNewAEMeteringMode >= NUM_OF_AE_METER))
    {
        CAM_LOGE("[%s()] Unsupport AE Metering Mode: %d\n", __FUNCTION__, eNewAEMeteringMode);
        return E_AE_UNSUPPORT_MODE;
    }

    switch (eNewAEMeteringMode)
    {
        case AE_METERING_MODE_SOPT:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_SOPT;
            break;
        case AE_METERING_MODE_AVERAGE:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_AVERAGE;
            break;
        case AE_METERING_MODE_CENTER_WEIGHT:
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
        default:
            AE_LOG("[%s()] The AE metering mode enum value is incorrectly:%d\n", __FUNCTION__, eNewAEMeteringMode);
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
    }

    if (m_eAEMeterMode != eAEMeteringMode)
    {
        m_eAEMeterMode = eAEMeteringMode;
        AE_LOG("[%s()] m_eAEMeterMode:%d\n", __FUNCTION__, m_eAEMeterMode);
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->setAEMeteringMode(m_eAEMeterMode);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEMeterMode() const
{
    return static_cast <MINT32> (m_eAEMeterMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps)
{
    MINT32 i4NewMinFPS, i4NewMaxFPS, i4SensorMaxFPS;

    i4NewMinFPS = i4NewAEMinFps / 100;
    i4NewMaxFPS = i4NewAEMaxFps / 100;
    i4SensorMaxFPS = m_rSensorMaxFrmRate[m_eSensorMode];
    AE_LOG_IF(m_3ALogEnable, "[%s()] SensorMaxFPS:%d NewFPS(Min/Max):%d/%d CurFPS(Min/Max):%d/%d IsFrameRateLocked:%d\n", __FUNCTION__,
        i4SensorMaxFPS, i4NewMinFPS, i4NewMaxFPS, m_i4AEMinFps, m_i4AEMaxFps, m_bIsFrameRateLocked);
    if (!m_bIsFrameRateLocked)
    {
        if (i4NewMaxFPS > i4SensorMaxFPS)
        {
            AE_LOG_IF(m_u4HwMagicNum < 15, "[%s()-Clipping] i4NewMaxFPS:%d->%d\n", __FUNCTION__, i4NewMaxFPS, i4SensorMaxFPS);
            i4NewMaxFPS = i4SensorMaxFPS;
        }

        if ((i4NewMinFPS < LIB3A_AE_FRAMERATE_MODE_05FPS) || (i4NewMaxFPS > LIB3A_AE_FRAMERATE_MODE_MAX))
        {
            AE_LOG("[%s()] Unsupport AE frame rate range:%d~%d\n", __FUNCTION__, i4NewMinFPS, i4NewMaxFPS);
            return S_AE_OK;
        }
        else if (i4NewMinFPS > i4NewMaxFPS)
        {
            AE_LOG("[%s()-Clipping] i4NewMinFPS:%d->%d\n", __FUNCTION__, i4NewMinFPS, i4NewMaxFPS);
            i4NewMinFPS = i4NewMaxFPS;
        }

        if ((m_i4AEMinFps != i4NewMinFPS) || (m_i4AEMaxFps != i4NewMaxFPS))
        {
            m_i4AEMinFps = i4NewMinFPS;
            m_i4AEMaxFps = i4NewMaxFPS;
            AE_LOG("[%s()] m_i4AEMinFps:%d m_i4AEMaxFps:%d\n", __FUNCTION__, m_i4AEMinFps, m_i4AEMaxFps);

            if (m_i4AEMaxFps != 0)
            {
                m_rSensorCurrentInfo.u8FrameDuration = 10005000000L / m_i4AEMaxFps;
                AE_LOG("[%s()] update frame duration as default:%d\n", __FUNCTION__, m_rSensorCurrentInfo.u8FrameDuration);
            }
            else
            {
                    m_rSensorCurrentInfo.u8FrameDuration = 33000000L;
                    AE_LOG("[%s()] m_i4AEMaxFps is zero, using default frame rate value (33ms)\n", __FUNCTION__);
            }
            if (m_pIAeAlgo != NULL)
            {
                updateAEScenarioPline(m_eSensorMode);
                m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
                m_pIAeAlgo->setAEMinMaxFrameRate(m_i4AEMinFps, m_i4AEMaxFps);
                m_pIAeAlgo->setAECamMode(m_eAECamMode);
                if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
                {
                    if (m_i4AEMaxFps > LIB3A_AE_FRAMERATE_MODE_30FPS)
                    {
                        m_pIAeFlowCCU->pause();
                        AE_LOG("[%s()] MaxFps > 30 CCU pause\n", __FUNCTION__);
                    }
                    else
                    {
                        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
                        m_pIAeFlowCCU->controltrigger((MUINT32)E_AE_FLOW_CCU_TRIGGER_MAXFPS);
                    }
                }
                else if (m_bCCUAEFlag && (m_i4AEMaxFps <= LIB3A_AE_FRAMERATE_MODE_30FPS) && (m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapMaxFrameRate > 30))
                {
                    m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_SENSOR_SUPPORT, MTRUE);
                    NeedCPUUpdateSensor = MTRUE;
                    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET);
                }
                else
                {
                    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
                }
                //m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                m_i4WaitVDNum = (m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1);
                // MTK add frame rate control
                if (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
                {
                    if ((m_i4AEMinFps == m_i4AEMaxFps) || (m_i4AEMaxFps <= LIB3A_AE_FRAMERATE_MODE_60FPS))
                    {
                        // frame rate control
                        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_FRAMERATE, MTRUE);
                        m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                        AE_LOG_IF(m_3ALogEnable, "[%s()] update sensor fps early\n", __FUNCTION__);
                    }
                }
                // MTK add frame rate control
            }
            else
            {
                AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
            }
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEShotMode(MUINT32 u4NewAEShotMode)
{
    EShotMode eNewAEShotMode = static_cast <EShotMode> (u4NewAEShotMode);

    if (m_eShotMode != eNewAEShotMode)
    {
        m_eShotMode = eNewAEShotMode;
        m_bAElimitor = MFALSE;
        AE_LOG("[%s()] m_eAppShotMode:%d AE limitor:%d\n", __FUNCTION__, m_eShotMode, m_bAElimitor);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAAOMode(MUINT32 u4NewAEAAOMode)
{
    AE_LOG("[%s()] m_i4AAOmode:%d->%d (0:8/12bits, 1:14bits)\n", __FUNCTION__, m_i4AAOmode, u4NewAEAAOMode);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setCamScenarioMode(MUINT32 u4NewCamScenarioMode, MBOOL bInit) // currently only for AE_MODE_4CELL_MVHDR_TARGET
{
    CAM_SCENARIO_T eNewCamScenarioMode = static_cast <CAM_SCENARIO_T> (u4NewCamScenarioMode);

    if (m_eCamScenarioMode != eNewCamScenarioMode || bInit)
    {
        m_i4CurrSensorId = getsensorDevID(m_eSensorDev);
        AE_LOG("[%s()] m_eSensorDev: %d, m_eCamScenarioMode:%d->%d bInit:%d SensorID:0x%0X\n", __FUNCTION__, m_eSensorDev, m_eCamScenarioMode, eNewCamScenarioMode, bInit, m_i4CurrSensorId);
        m_eCamScenarioMode = eNewCamScenarioMode;

        switch (m_eSensorDev)
        {
            case ESensorDev_Main:
                if (m_eCamScenarioMode == CAM_SCENARIO_PREVIEW)
                {
                    // preivew
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_PREVIEW > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    // video
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_VIDEO > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CAPTURE)
                {
                    // capture
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_CAPTURE > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1)
                {
                    // HDR ON
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_CUSTOM1 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)
                {
                    // auto HDR
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_CUSTOM2 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3)
                {
                    // VT
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_CUSTOM3 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4)
                {
                    // stereo
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Main, CAM_SCENARIO_CUSTOM4 > ();
                }
                break;
            case ESensorDev_Sub:
                if (m_eCamScenarioMode == CAM_SCENARIO_PREVIEW)
                {
                    // preivew
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_PREVIEW > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    // video
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_VIDEO > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CAPTURE)
                {
                    // capture
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_CAPTURE > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1)
                {
                    // HDR ON
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_CUSTOM1 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)
                {
                    // auto HDR
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_CUSTOM2 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3)
                {
                    // VT
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_CUSTOM3 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4)
                {
                    // stereo
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_Sub, CAM_SCENARIO_CUSTOM4 > ();
                }
                break;
            case ESensorDev_MainSecond:
                if (m_eCamScenarioMode == CAM_SCENARIO_PREVIEW)
                {
                    // preivew
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_PREVIEW > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    // video
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_VIDEO > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CAPTURE)
                {
                    // capture
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_CAPTURE > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1)
                {
                    // HDR ON
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM1 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)
                {
                    // auto HDR
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM2 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3)
                {
                    // VT
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM3 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4)
                {
                    // stereo
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM4 > ();
                }
                break;
            case ESensorDev_MainThird:
                if (m_eCamScenarioMode == CAM_SCENARIO_PREVIEW)
                {
                    // preivew
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_PREVIEW > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    // video
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_VIDEO > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CAPTURE)
                {
                    // capture
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_CAPTURE > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1)
                {
                    // HDR ON
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_CUSTOM1 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)
                {
                    // auto HDR
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_CUSTOM2 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3)
                {
                    // VT
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_CUSTOM3 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4)
                {
                    // stereo
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_MainThird, CAM_SCENARIO_CUSTOM4 > ();
                }
                break;
            case ESensorDev_SubSecond:
                if (m_eCamScenarioMode == CAM_SCENARIO_PREVIEW)
                {
                    // preivew
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_PREVIEW > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    // video
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_VIDEO > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CAPTURE)
                {
                    // capture
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_CAPTURE > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1)
                {
                    // HDR ON
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM1 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)
                {
                    // auto HDR
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM2 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3)
                {
                    // VT
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM3 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4)
                {
                    // stereo
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM4 > ();
                }
                break;
            default:
                if (m_eCamScenarioMode == CAM_SCENARIO_PREVIEW)
                {
                    // preivew
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_PREVIEW > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_VIDEO)
                {
                    // video
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_VIDEO > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CAPTURE)
                {
                    // capture
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_CAPTURE > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1)
                {
                    // HDR ON
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_CUSTOM1 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)
                {
                    // auto HDR
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_CUSTOM2 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3)
                {
                    // VT
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_CUSTOM3 > ();
                }
                else if (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4)
                {
                    // stereo
                    m_rAEInitInput.rAEPARAM = getAEParamData < ESensorDev_None, CAM_SCENARIO_CUSTOM4 > ();
                }
                break;
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setNVRAMIndex(MUINT32 a_eNVRAMIndex, MBOOL bInit)
{
    if (m_u4AENVRAMIdx != a_eNVRAMIndex || bInit)
    {
        AE_LOG("[%s()] m_eSensorDev:%d m_u4AENVRAMIdx:%d->%d bInit:%d\n", __FUNCTION__, m_eSensorDev, m_u4AENVRAMIdx, a_eNVRAMIndex, bInit);
        m_u4AENVRAMIdx = a_eNVRAMIndex;
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
        if (m_p3ANVRAM != NULL)
        {
            m_rAEInitInput.rAENVRAM = &(m_p3ANVRAM->AE[m_u4AENVRAMIdx]);
        }
        else
        {
            CAM_LOGE("[%s()] Nvram 3A pointer is NULL\n", __FUNCTION__);
        }

        // AE max block width & width
        if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
        {
            m_rAEInitInput.i4AEMaxBlockWidth = m_rSensorVCInfo.u2VCStatWidth;
            m_rAEInitInput.i4AEMaxBlockHeight = m_rSensorVCInfo.u2VCStatHeight;
        }
        else
        {
            m_rAEInitInput.i4AEMaxBlockWidth = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
            m_rAEInitInput.i4AEMaxBlockHeight = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        }
        // [ISP 5.0] AAO / PSO buffer size for AEalgo
        m_u4BlockNumX = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
        m_u4BlockNumY = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        m_rAEInitInput.u4AAO_AWBValueWinSize = 8;
        m_rAEInitInput.u4AAO_AWBSumWinSize = 12 * AE_PARAM_AE_LINEAR_STAT_EN;
        m_rAEInitInput.u4AAO_AEYWinSize = 2;
        m_rAEInitInput.u4AAO_AEOverWinSize = 1 * (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableAEOVERCNTconfig);
        m_rAEInitInput.u4AAO_HistSize = 3 * AE_PARAM_AE_HST_DMA_OUT_EN;
        m_rAEInitInput.i4AAOLineByte = m_u4BlockNumX * (m_rAEInitInput.u4AAO_AWBValueWinSize + m_rAEInitInput.u4AAO_AWBSumWinSize + m_rAEInitInput.u4AAO_AEYWinSize + m_rAEInitInput.u4AAO_AEOverWinSize);
        m_rAEInitInput.u4PSO_SE_AWBWinSize = 8;
        m_rAEInitInput.u4PSO_LE_AWBWinSize = 0;
        m_rAEInitInput.u4PSO_SE_AEYWinSize = 2;
        m_rAEInitInput.u4PSO_LE_AEYWinSize = 0;
        m_rAEInitInput.u4PSO_SE_HistSize = 3;
        m_rAEInitInput.u4PSO_LE_HistSize = 0;
        // AE statistics and histogram config
        m_bEnableHDRYConfig = (MBOOL)(m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableHDRLSB > 0);
        m_bEnableOverCntConfig = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableAEOVERCNTconfig;
        m_bEnableTSFConfig = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableTSFSTATconfig;
        m_bDisablePixelHistConfig = ((m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePixelBaseHist == 1) ? MFALSE : MTRUE);
        m_u4AEOverExpCntThs = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.uOverExpoTHD;
        m_u4AEYCoefR = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEYCoefR;
        m_u4AEYCoefG = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEYCoefG;
        m_u4AEYCoefB = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEYCoefB;
        // AE perframe and TG -int switch
        m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth & AE_PERFRAME_EN;
        m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth & AE_PERFRAME_EN;
        if ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)) // HDR flow
            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth;
        else
            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;

        m_bTgIntAEEn = m_rAEInitInput.rAENVRAM->rAeParam.bTgIntAEEn;
        m_i4SensorGainDelayFramesWOShutter = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.uAESensorGainDelayCycleWOShutter;
        AE_LOG("[%s()] m_eSensorDev:%d m_i4SensorIdx:%d AE max block width:%d heigh:%d AAO separation size(To AeAlgo):%d/%d/%d/%d AAOLineByte:%d AEconfig:%d/%d/%d/%d/%d/%d AEYCoefR/G/B:%d/%d/%d AE_PERFRAME_EN:%d m_bPerframeAEFlag:%d m_bTgIntAEEn:%d FinerEVIdxBase:%d\n",
            __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_rAEInitInput.i4AEMaxBlockWidth, m_rAEInitInput.i4AEMaxBlockHeight,
            m_rAEInitInput.u4AAO_AWBValueWinSize, m_rAEInitInput.u4AAO_AWBSumWinSize, m_rAEInitInput.u4AAO_AEYWinSize, m_rAEInitInput.u4AAO_AEOverWinSize, m_rAEInitInput.i4AAOLineByte,
            m_u4BlockNumX, m_u4BlockNumY, m_bEnableHDRYConfig, m_bEnableOverCntConfig, m_bEnableTSFConfig, m_bDisablePixelHistConfig, m_u4AEYCoefR, m_u4AEYCoefG, m_u4AEYCoefB,
            AE_PERFRAME_EN, m_bPerframeAEFlag, m_bTgIntAEEn, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4FinerEvIndexBase);

        if (!bInit)
        {
            m_pIAeAlgo->updateAEParam(&m_rAEInitInput);
            updateAEScenarioPline(m_eSensorMode);
        }
        m_bPlineMappingChange = MTRUE;
    }

    if (m_bPlineMappingChange)
    {
        if (m_bStereoManualPline)
        {
            m_pAEMapPlineTable = (AE_PLINEMAPPINGTABLE_T *)(&(m_rAEInitInput.rAENVRAM->rAeParam.strStereoPlineMapping.sAESceneMapping[0]));
            AE_LOG("[%s()] change to stereo pline mapping table, m_u4AENVRAMIdx = %d, m_bStereoManualPline = %d\n", __FUNCTION__, m_u4AENVRAMIdx, m_bStereoManualPline);
        }
        else if (m_u4AEHDRMode == 1 || m_u4AEHDRMode == 2)
        {
            m_pAEMapPlineTable = (AE_PLINEMAPPINGTABLE_T *)(&(m_rAEInitInput.rAENVRAM->rAeParam.strHDRPlineMapping.sAESceneMapping[0]));
            AE_LOG("[%s()] change to HDR pline mapping table, m_u4AENVRAMIdx = %d, m_u4AEHDRMode = %d\n", __FUNCTION__, m_u4AENVRAMIdx, m_u4AEHDRMode);
        }
        else
        {
            m_pAEMapPlineTable = &(m_rAEInitInput.rAEPlineMapTable[0]);
            AE_LOG("[%s()] change to default pline mapping table, m_u4AENVRAMIdx = %d, m_bStereoManualPline = %d, m_u4AEHDRMode = %d\n", __FUNCTION__, m_u4AENVRAMIdx, m_bStereoManualPline, m_u4AEHDRMode);
        }
        updateAEScenarioPline(m_eSensorMode);
        m_bPlineMappingChange = MFALSE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEHDRMode(MUINT32 u4NewAEHDRMode)
{
    if (m_u4AEHDRMode != u4NewAEHDRMode)
    {
        // auto HDR: m_i4HdrOnOff reset
        m_i4HdrOnOff = toLiteral(HDRDetectionResult::NONE);
        if (u4NewAEHDRMode == 2)
        {
            m_i4HdrOnOff = toLiteral(HDRDetectionResult::NORMAL);
            AE_LOG("[%s()] turn on auto HDR\n", __FUNCTION__);
        }
        AE_LOG("[%s()] m_i4HdrOnOff:%d HDR AE Mode:%d->%d\n", __FUNCTION__, m_i4HdrOnOff, m_u4AEHDRMode, u4NewAEHDRMode);
        m_u4AEHDRMode = u4NewAEHDRMode;
        m_bPlineMappingChange = MTRUE;
    }

    if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET && m_bResetZHDRMode)
    {
        if ((m_eSensorMode == ESensorMode_Video) && (m_eISPProfile == EIspProfile_Preview))
        {
            AE_LOG("[%s()] ZHDR SensorMode:%d Change AE Pline table to Preview\n", __FUNCTION__, m_eSensorMode);
            updateAEScenarioPline(ESensorMode_Preview);
        }
        m_bResetZHDRMode = MFALSE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEHDROnOff()
{
    AE_LOG_IF(m_3ALogEnable && (m_i4HdrOnOff >= 0), "[%s] m_i4HdrOnOff:%d\n", __FUNCTION__, m_i4HdrOnOff);
    return m_i4HdrOnOff;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELimiterMode(MBOOL bAELimter)
{
    m_bAElimitor = bAELimter;
    AE_LOG("[%s()] ShotMode:%d AE limitor:%d\n", __FUNCTION__, m_eShotMode, m_bAElimitor);
    if (m_bAElimitor && m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
        m_pIAeFlowCCU->pause();
        AE_LOG("[%s()] AE_MODE_OFF CCU pause\n", __FUNCTION__);
    }
    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->setAElimitorEnable(m_bAElimitor);
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMode(MUINT32 u4NewAEMode)
{
    MUINT32 eNewAEMode = u4NewAEMode;
    //LIB3A_AE_MODE_T eAEMode; /* fix build warning */
    if ((MINT32)eNewAEMode != (MINT32)m_eAEMode)
    {
        if (m_eAEMode == LIB3A_AE_MODE_OFF)
        {
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
        }
        AE_LOG("[%s()] m_eAEMode:%d->%d AEState:%d %d\n", __FUNCTION__, m_eAEMode, eNewAEMode, m_eAEState, m_ePreAEState);

        switch (eNewAEMode)
        {
            case MTK_CONTROL_AE_MODE_OFF:
                m_eAEMode = LIB3A_AE_MODE_OFF;
                m_eAEState = MTK_CONTROL_AE_STATE_INACTIVE;
                AE_LOG("[%s()] AE_MODE_OFF CCU ON \n", __FUNCTION__);
                break;
            case MTK_CONTROL_AE_MODE_ON:
                m_bIsLongExpControlbyCCU = MTRUE;
                m_eAEMode = LIB3A_AE_MODE_ON;
                break;
            case MTK_CONTROL_AE_MODE_ON_AUTO_FLASH:
                m_eAEMode = LIB3A_AE_MODE_ON_AUTO_FLASH;
                break;
            case MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
                m_eAEMode = LIB3A_AE_MODE_ON_ALWAYS_FLASH;
                break;
            case MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE:
                m_eAEMode = LIB3A_AE_MODE_ON_AUTO_FLASH_REDEYE;
                break;
            default:
                break;
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEMode() const
{
    return static_cast <MINT32> (m_eAEMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEState() const
{
    return static_cast <MINT32> (m_eAEState);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::setSensorMode(MINT32 i4NewSensorMode , MUINT32 width, MUINT32 height)
{
    ESensorMode_T eNewSensorMode;
    MUINT32 u4BlocksizeX;
    MUINT32 u4BlocksizeY;
    AE_LOG("[%s()] m_eSensorDev:%d i4NewSensorMode:%d\n", __FUNCTION__, m_eSensorDev, i4NewSensorMode);

    switch (i4NewSensorMode)
    {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            eNewSensorMode = ESensorMode_Preview;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            eNewSensorMode = ESensorMode_Capture;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            eNewSensorMode = ESensorMode_Video;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            eNewSensorMode = ESensorMode_SlimVideo1;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            eNewSensorMode = ESensorMode_SlimVideo2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            eNewSensorMode = ESensorMode_Custom1;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            eNewSensorMode = ESensorMode_Custom2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            eNewSensorMode = ESensorMode_Custom3;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            eNewSensorMode = ESensorMode_Custom4;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            eNewSensorMode = ESensorMode_Custom5;
            break;
        default:
            CAM_LOGE("[%s()] E_AE_UNSUPPORT_SENSOR_MODE: %d\n", __FUNCTION__, i4NewSensorMode);
            return MFALSE;
    }

    // Window size
    u4BlocksizeX = ((width / m_u4BlockNumX) / 2) * 2;
    u4BlocksizeY = ((height / m_u4BlockNumY) / 2) * 2;
    m_u4AEOverExpCntShift = (((u4BlocksizeX / 4) * (u4BlocksizeY / 2) > 255) ? MTRUE : MFALSE);
    AE_LOG("[%s()] m_eSensorMode/Hbinwidth/Hbinheight:%d/%d/%d->%d/%d/%d, u4BlocksizeX/Y/Shift:%d/%d/%d\n", __FUNCTION__, m_eSensorMode, m_u4Hbinwidth, m_u4Hbinheight,
        eNewSensorMode, width, height, u4BlocksizeX , u4BlocksizeY, m_u4AEOverExpCntShift);
    m_u4Hbinwidth = width;
    m_u4Hbinheight = height;

    // TG interrupt AAO information
    MINT32 i4PitchY = height / m_u4BlockNumY;
    MINT32 i4OriginY = (height - (i4PitchY * m_u4BlockNumY)) / 2;
    MINT32 i4AETgValidBlockWidth = m_rAEInitInput.i4AEMaxBlockWidth;
    MINT32 i4AETgValidBlockHeight = (height * TG_INT_AE_RATIO - i4OriginY) / i4PitchY;
    AE_LOG_IF(m_3ALogEnable, "[%s()] i4AETgValidBlockWidth/Height:%d/%d->%d/%d, u4BlockNumX/Y/i4PitchY/i4OriginY:%d/%d/%d/%d\n", __FUNCTION__,
        m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight, i4AETgValidBlockWidth, i4AETgValidBlockHeight,
        m_u4BlockNumX, m_u4BlockNumY, i4PitchY, i4OriginY);
    m_i4AETgValidBlockWidth = i4AETgValidBlockWidth;
    m_i4AETgValidBlockHeight = i4AETgValidBlockHeight;

    if ((m_bAECaptureUpdate == MFALSE) && (width != 0) && (height != 0))
    {
        ModifyCaptureParamsBySensorMode(eNewSensorMode, ESensorMode_Capture);
    }

    if (m_eSensorMode != eNewSensorMode)
    {
        if (m_pAEMapPlineTable != NULL)
        {
            switchSensorModeMaxBVSensitivityDiff(eNewSensorMode, m_eSensorMode, m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
        }
        AE_LOG("[%s()] m_eSensorMode:%d->%d\n", __FUNCTION__, m_eSensorMode, eNewSensorMode);
        m_eSensorMode = eNewSensorMode;
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->setAESensorMode(m_eSensorMode);
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::updateAEScenarioMode(EIspProfile_T eIspProfile)
{
    if (m_eISPProfile != eIspProfile)
    {
        AE_LOG("[%s()] eIspProfile:%d->%d\n", __FUNCTION__, m_eISPProfile, eIspProfile);
        m_eISPProfile = eIspProfile;
        if ((m_eSensorMode == ESensorMode_Preview) && (eIspProfile == EIspProfile_Video))
        {
            AE_LOG("[%s()] SensorMode:%d Change AE Pline table to Video\n", __FUNCTION__, m_eSensorMode);
            // AE Auto Test
            m_pIAeTest->recordAvgY(IAeTest::AE_AT_TYPE_Pre2Video, m_pIAeAlgo->getBrightnessAverageValue());
            updateAEScenarioPline(ESensorMode_Video);
        }
        else if ((m_eSensorMode == ESensorMode_Video) && (eIspProfile == EIspProfile_Preview))
        {
            AE_LOG("[%s()] SensorMode:%d Change AE Pline table to Preview\n", __FUNCTION__, m_eSensorMode);
            updateAEScenarioPline(ESensorMode_Preview);
        }
        else
        {
            m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            AE_LOG("[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            setAEScene(m_eAEScene);
            setIsoSpeed(m_u4AEISOSpeed);
            setAEMinMaxFrameRatePlineIdx(m_i4AEMinFps , m_i4AEMaxFps);
            MUINT32 u4FinerEVIdxBase = 1;
            m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
            if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
            {
                m_u4FinerEVIdxBase = u4FinerEVIdxBase;
                m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
                m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
                m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
                searchPreviewIndexLimit();
            }
            AE_LOG("[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
            m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

            m_pIAeAlgo->setSensorModeMaxBVdiff(m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
            AE_LOG("[%s()] SensorMode:%d IspProfile:%d Change AE Pline table to Normal\n", __FUNCTION__, m_eSensorMode, eIspProfile);
        }
        // Reschedule if needed
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateAEScenarioPline(MINT32 i4SensorMode)
{
    LIB3A_AE_SCENE_T eAEScene = m_eAEScene;
    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->setAESensorMode(i4SensorMode);
        m_pIAeAlgo->setIsoSpeed(m_u4AEISOSpeed);

        if (m_pAEMapPlineTable != NULL)
        {
            switchSensorModeMaxBVSensitivityDiff(i4SensorMode, m_eSensorMode, m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
        }

        m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
        AE_LOG("[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);

        // Check if assigned ISO for overwrite AE scene
        switch (m_u4AEISOSpeed)
        {
            case LIB3A_AE_ISO_SPEED_12800:
                eAEScene = LIB3A_AE_SCENE_ISO12800;
                break;
            case LIB3A_AE_ISO_SPEED_6400:
                eAEScene = LIB3A_AE_SCENE_ISO6400;
                break;
            case LIB3A_AE_ISO_SPEED_3200:
                eAEScene = LIB3A_AE_SCENE_ISO3200;
                break;
            case LIB3A_AE_ISO_SPEED_1600:
                eAEScene = LIB3A_AE_SCENE_ISO1600;
                break;
            case LIB3A_AE_ISO_SPEED_800:
                eAEScene = LIB3A_AE_SCENE_ISO800;
                break;
            case LIB3A_AE_ISO_SPEED_400:
                eAEScene = LIB3A_AE_SCENE_ISO400;
                break;
            case LIB3A_AE_ISO_SPEED_200:
                eAEScene = LIB3A_AE_SCENE_ISO200;
                break;
            case LIB3A_AE_ISO_SPEED_100:
                eAEScene = LIB3A_AE_SCENE_ISO100;
                break;
            default:
                break;
        }

        MRESULT mr;
        eAETableID ePreviewPLineTableID, eCapturePLineTableID;
        if (m_pAEPlineTable != NULL && m_pAEMapPlineTable != NULL)
        {
            // protect the AE Pline table don't ready
            if (eAEScene != m_eAEScene)
                mr = getAEPLineMappingID(eAEScene, i4SensorMode, &ePreviewPLineTableID, &eCapturePLineTableID);
            else
                mr = getAEPLineMappingID(m_eAEScene, i4SensorMode, &ePreviewPLineTableID, &eCapturePLineTableID);
            if (FAILED(mr))
            {
                MY_ERR("[%s()] Get capture table ERROR:%d PLineID:%d %d\n", __FUNCTION__, m_eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
            }

            mr = setAETable(ePreviewPLineTableID, eCapturePLineTableID);
            if (FAILED(mr))
            {
                MY_ERR("[%s()] Capture table ERROR:%d PLineID:%d %d\n", __FUNCTION__, m_eAEScene, ePreviewPLineTableID, eCapturePLineTableID);
            }
        }
        else
        {
            AE_LOG("[%s()] setAEScene:%d, AE Pline table is NULL \n", __FUNCTION__, m_eAEScene);
        }

        //setIsoSpeed(m_u4AEISOSpeed);
        m_pPreviewTableCurrent = m_pPreviewTableNew;
        searchPreviewIndexLimit();
        //switchSensorModeMaxBVSensitivityDiff(MINT32 i4newSensorMode, MINT32 i4oldSensorMode, MINT32 & i4SenstivityDeltaIdx, MINT32 & i4BVDeltaIdx)
        MUINT32 u4FinerEVIdxBase = 1;
        m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
        if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
        {
            m_u4FinerEVIdxBase = u4FinerEVIdxBase;
            m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
            m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
            m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
            searchPreviewIndexLimit();
        }
        AE_LOG("[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
        m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
        m_pIAeAlgo->setSensorModeMaxBVdiff(m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);

    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getSensorMode() const
{
    return static_cast <MINT32> (m_eSensorMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::ClrCurrentInfo_AELock()
{
    memset(&m_rSensorCurrentInfo_aelock, 0, sizeof(AE_SENSOR_PARAM_T));
    memset(&m_rSensorCurrentInfoCCU_aelock, 0, sizeof(AE_SENSOR_PARAM_T));

    AE_LOG_IF(m_3ALogEnable, "[%s] %d %d %d %d %d %d \n", __FUNCTION__, m_rSensorCurrentInfo_aelock.u8FrameDuration, m_rSensorCurrentInfo_aelock.u8ExposureTime, m_rSensorCurrentInfo_aelock.u4Sensitivity, m_rSensorCurrentInfoCCU_aelock.u8FrameDuration, m_rSensorCurrentInfoCCU_aelock.u8ExposureTime, m_rSensorCurrentInfoCCU_aelock.u4Sensitivity);
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::GetCurrentInfo_AELock()
{
    AE_LOG_IF(m_3ALogEnable, "[%s] %d %d \n", __FUNCTION__, m_bAPAELock, m_bAFAELock);
    if(!(m_bAPAELock == MFALSE) && (m_bAFAELock == MFALSE))
    {
        if((m_eAEMode == LIB3A_AE_MODE_OFF) &&
           (m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime != 0) &&
           (m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity != 0))
            {
                m_rSensorCurrentInfo_aelock.u8FrameDuration = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4FrameDuration;
                m_rSensorCurrentInfo_aelock.u8ExposureTime = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime;
                m_rSensorCurrentInfo_aelock.u4Sensitivity = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity;

                m_rSensorCurrentInfoCCU_aelock.u8FrameDuration =  m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4FrameDuration;
                m_rSensorCurrentInfoCCU_aelock.u8ExposureTime = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime;
                m_rSensorCurrentInfoCCU_aelock.u4Sensitivity = m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity;
                AE_LOG_IF(m_3ALogEnable, "[%s] ae_mode off \n", __FUNCTION__);
            }
        else if((m_i4EVIndexPreset != 0) || (m_i4EVIndex != 0))
        {
            m_rSensorCurrentInfo_aelock.u8FrameDuration = m_rSensorCurrentInfo.u8FrameDuration;
            m_rSensorCurrentInfo_aelock.u8ExposureTime = m_rSensorCurrentInfo.u8ExposureTime;
            m_rSensorCurrentInfo_aelock.u4Sensitivity = m_rSensorCurrentInfo.u4Sensitivity;

            m_rSensorCurrentInfoCCU_aelock.u8FrameDuration =  m_rSensorCurrentInfoCCU.u8FrameDuration;
            m_rSensorCurrentInfoCCU_aelock.u8ExposureTime = m_rSensorCurrentInfoCCU.u8ExposureTime;
            m_rSensorCurrentInfoCCU_aelock.u4Sensitivity = m_rSensorCurrentInfoCCU.u4Sensitivity;
            AE_LOG("[%s:du] ae_mode on and EVIndex!=0\n", __FUNCTION__);
        }
        else
        {
            if(m_rSensorCurrentInfo_aelock.u8ExposureTime != 0)
            {
                m_rSensorCurrentInfo.u8FrameDuration = m_rSensorCurrentInfo_aelock.u8FrameDuration;
                m_rSensorCurrentInfo.u8ExposureTime = m_rSensorCurrentInfo_aelock.u8ExposureTime;
                m_rSensorCurrentInfo.u4Sensitivity = m_rSensorCurrentInfo_aelock.u4Sensitivity;
            }

            if(m_rSensorCurrentInfoCCU_aelock.u8ExposureTime != 0)
            {
                m_rSensorCurrentInfoCCU.u8FrameDuration = m_rSensorCurrentInfoCCU_aelock.u8FrameDuration;
                m_rSensorCurrentInfoCCU.u8ExposureTime = m_rSensorCurrentInfoCCU_aelock.u8ExposureTime;
                m_rSensorCurrentInfoCCU.u4Sensitivity = m_rSensorCurrentInfoCCU_aelock.u4Sensitivity;
            }
            AE_LOG_IF(m_3ALogEnable, "[%s] ae_mode on \n", __FUNCTION__);
        }
    }
    AE_LOG_IF(m_3ALogEnable, "[%s] %d %d %d %d %d %d \n",
        __FUNCTION__,
        m_rSensorCurrentInfo.u8FrameDuration, m_rSensorCurrentInfo.u8ExposureTime,
        m_rSensorCurrentInfo.u4Sensitivity, m_rSensorCurrentInfoCCU.u8FrameDuration,
        m_rSensorCurrentInfoCCU.u8ExposureTime, m_rSensorCurrentInfoCCU.u4Sensitivity);
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELock()
{
    if ((m_bAPAELock == MFALSE) && (m_bAFAELock == MFALSE))
    {
        // AE unlock
        AE_LOG("[%s] m_bAELock:%d->0 m_bAPAELock:%d m_bAFAELock:%d AEState:%d %d\n", __FUNCTION__, m_bAELock, m_bAPAELock, m_bAFAELock, m_eAEState, m_ePreAEState);
        m_bAELock = MFALSE;
        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
        ClrCurrentInfo_AELock();
    }
    else
    {
        // AE lock
        AE_LOG("[%s] m_bAELock:%d->1 m_bAPAELock:%d m_bAFAELock:%d AEState:%d %d\n", __FUNCTION__, m_bAELock, m_bAPAELock, m_bAFAELock, m_eAEState, m_ePreAEState);
        m_bAELock = MTRUE;
    }

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->lockAE(m_bAELock);
    }
    else
    {
        AE_LOG("[%s] The AE algo class is NULL, i4SensorDev:%d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAPAELock(MBOOL bAPAELock)
{
    if (m_bAPAELock != bAPAELock)
    {
        AE_LOG("[%s()] m_bAPAELock:%d->%d\n", __FUNCTION__, m_bAPAELock, bAPAELock);
        m_bAPAELock = bAPAELock;
        setAELock();
        if (m_bAPAELock == MFALSE)
        {
            // AE unlock
            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
        }
        else
        {
            // AE lock
            m_ePreAEState = m_eAEState;
            m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
            // avoid being too late to preparing Cap param after lock
            prepareCapParams(MFALSE,MFALSE);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAFAELock(MBOOL bAFAELock)
{
    if (m_bAFAELock != bAFAELock)
    {
        AE_LOG("[%s()] m_bAFAELock:%d->%d\n", __FUNCTION__, m_bAFAELock, bAFAELock);
        m_bAFAELock = bAFAELock;
        setAELock();
        if (bAFAELock == MFALSE)
        {
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_AERESET);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if ((u4XOffset <= 8192) && (u4YOffset <= 8192)
        && (u4Width <= 16384) && (u4Height <= 16384))
    {
        if ((m_eZoomWinInfo.u4XOffset != u4XOffset) || (m_eZoomWinInfo.u4XWidth != u4Width) ||
            (m_eZoomWinInfo.u4YOffset != u4YOffset) || (m_eZoomWinInfo.u4YHeight != u4Height))
        {
            AE_LOG("[%s()] New WinX:%d %d New WinY:%d %d Old WinX:%d %d Old WinY:%d %d\n", __FUNCTION__, u4XOffset, u4Width, u4YOffset, u4Height,
                m_eZoomWinInfo.u4XOffset, m_eZoomWinInfo.u4XWidth,
                m_eZoomWinInfo.u4YOffset, m_eZoomWinInfo.u4YHeight);
            m_eZoomWinInfo.bZoomChange = MTRUE;
            m_eZoomWinInfo.u4XOffset = u4XOffset;
            m_eZoomWinInfo.u4XWidth = u4Width;
            m_eZoomWinInfo.u4YOffset = u4YOffset;
            m_eZoomWinInfo.u4YHeight = u4Height;
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
        }
    }
    else
    {
        AE_LOG("[%s()] Wrong zoom window size %d/%d/%d/%d \n", __FUNCTION__, u4XOffset, u4Width, u4YOffset, u4Height);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AeMgr::getAEMaxMeterAreaNum()
{
    return MAX_METERING_AREAS;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAE()
{
    AE_LOG("[%s] m_bEnableAE:%d->1\n", __FUNCTION__, m_bEnableAE);
    m_bEnableAE = MTRUE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::disableAE()
{
    AE_LOG("[%s] m_bEnableAE:%d->0\n", __FUNCTION__, m_bEnableAE);
    m_bEnableAE = MFALSE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPvAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule, MUINT8 u1AeMode)
{
    AAA_TRACE_D("doPvAE");

    //AE_INFO_T rAEInfo2ISP; /* fix build warning */
    //MUINT32 u4DFpsThres = 0; /* fix build warning */
    int bRestore = 0;
    MVOID *pAEStatisticBuf;
    //MUINT8 *pAEStaticBuf; /* fix build warning */
    MBOOL bCalculateAE = MFALSE;
    MBOOL bApplyAE = MFALSE;
    MBOOL bIsHDRflow = ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET));

    // Sync magic num from 3A
    if (m_u4HwMagicNum != u4AAOUpdate)
    {
        m_u4HwMagicNum = u4AAOUpdate;
    }
    // check if ever pauseCCU - restartCCU
    if ((!NeedCPUUpdateSensor) && (m_bRealISOSpeed == MFALSE) && (m_u4HDRcountforrevert <= 1) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_SENSOR_SUPPORTED) && m_bCCUAEFlag && ShouldCCUStart())
    {
        AE_FLOW_INPUT_INFO_T initInfo;
        prepareCCUStartParam(&initInfo);
        if (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
            m_bForceSkipCCU = MFALSE;
        m_pIAeFlowCCU->start((MVOID*) &initInfo);
        m_bIsCCUAEInit = MFALSE;
    }
    AAA_TRACE_D("ControlCCU");
    // prepare control param for CCU
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && (!m_bRestoreAE))
    {
        if ((m_eSensorDev != ESensorDev_Sub) || (m_eSensorDev == ESensorDev_Sub && !m_bInitWaitFaceSkipAE))
        {
            if (m_eZoomWinInfo.bZoomChange == MTRUE)
            {
                if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET))
                {
                    m_rAEStatCfg.u4EndBankIdx = m_u4CCUEndBankIdx;
                    m_rAEStatCfg.u4UseDefaultHight = MFALSE;
                }
                else
                {
                    m_rAEStatCfg.u4UseDefaultHight = MTRUE;
                }
                if (m_pIAeAlgo != NULL)
                {
                    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);
                }
                else
                {
                    AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                }
                m_eZoomWinInfo.bZoomChange = MFALSE;
                // Update AE histogram window config
                AAA_TRACE_D("doPvAEStatConfig");
                ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAEStatCfg);
                AAA_TRACE_END_D;
                AE_LOG("[%s()] AE config change (Skip AeAlgo once)\n", __FUNCTION__);
            }

            AE_FLOW_CONTROL_INFO_T controlInfo;
            AE_CORE_MAIN_OUT CCUoutput;
            if ((!u1AeMode) && (m_eAEMode != LIB3A_AE_MODE_OFF) && (!bAAASchedule || (i4ActiveAEItem & E_AE_AE_CALC)))
                m_bForceSkipCCU = MFALSE;
            AE_LOG_IF(m_3ALogEnable, "[%s()] m_bForceSkipCCU:%d\n", __FUNCTION__, m_bForceSkipCCU);
            UpdateGyroInfo();
            prepareCCUPvParam(&controlInfo);
            m_pIAeFlowCCU->setupParamPvAE((MVOID *)&controlInfo, (MVOID *) &CCUoutput);
            if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET))
                copyCCUAEInfo2mgr(&CCUoutput);
        }
    }
    AAA_TRACE_END_D;
    // Reset flag when preview
    m_bAEReadyCapture = MFALSE; // reset have been ever Normal Captured flag
    m_i4TimeOutCnt = MFALSE; // reset timeout counter for doAFAE / doPreCapAE
    m_bFrameUpdate = MTRUE;

    if (m_u4InitWaitCount <= INIT_WOF_WAIT_CYCLE)
    {
        if (m_u4InitWaitCount == INIT_WOF_WAIT_CYCLE && (!m_bFDenable))
        {
            // Clear FD Area
            memset(&m_eAEFDArea, 0, sizeof(AEMeteringArea_T));
            m_bFaceAEAreaChage = MFALSE;
            m_bTouchAEAreaChage = MFALSE;
            m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
            AE_LOG("[%s()] m_u4InitWaitCount == INIT_WOF_WAIT_CYCLE, Clear AE FD Area\n ", __FUNCTION__);
        }
        m_u4InitWaitCount++;
        AE_LOG_IF(m_3ALogEnable, "[%s()] m_u4InitWaitCount:%d", __FUNCTION__, m_u4InitWaitCount);
    }

    // Get mVHDR mode buffer address
    if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
    {
        mVHDRInputParam_T rInput;
        mVHDROutputParam_T rOutput;
        // Dequeue AE DMA buffer
        rInput.u4SensorID = 0; //TBD
        rInput.u4OBValue = 0;
        rInput.u4ISPGain = m_strIVHDROutputSetting.u4LEISPGain; //m_rAEOutput.rPreviewMode.u4IspGain;
        rInput.u2StatWidth = m_rSensorVCInfo.u2VCStatWidth;
        rInput.u2StatHight = m_rSensorVCInfo.u2VCStatHeight;
        if (m_rSensorVCInfo.u2VCModeSelect == 0)
        {
            rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        }
        else
        {
            if (m_strIVHDROutputSetting.u4SEExpTimeInUS != 0)
            {
                rInput.u2ShutterRatio = (m_strIVHDROutputSetting.u4LEExpTimeInUS + (m_strIVHDROutputSetting.u4SEExpTimeInUS >> 1)) / m_strIVHDROutputSetting.u4SEExpTimeInUS;
            }
            else
            {
                rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
                AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d u4SEExpTimeInUS:%d\n", __FUNCTION__, m_eSensorDev, m_strIVHDROutputSetting.u4SEExpTimeInUS);
            }
        }
        rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        rInput.pDataPointer = reinterpret_cast <MVOID *> (pAEStatBuf);
        decodemVHDRStatistic(rInput, rOutput);
        bUpdateSensorAWBGain = rOutput.bUpdateSensorAWB;
        pAEStatisticBuf = reinterpret_cast <MVOID *> (rOutput.pDataPointer);
    }
    else
    {
        pAEStatisticBuf = pAEStatBuf;
    }

    if (m_pIAeAlgo != NULL)
    {
        if (pAEStatisticBuf != NULL)
        {
            Mutex::Autolock lock(m_Lock);
            // Set FDarea to algorithm
            if (m_bFaceAEAreaChage == MTRUE)
            {
                // Face AE window change
                AAA_TRACE_ALG(setAEFDArea);
                AAA_TRACE_D("setAEFDArea");
                m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
                AAA_TRACE_END_D;
                AAA_TRACE_END_ALG;
                if (m_u4WOFDCnt > m_rAEInitInput.rAENVRAM->rAeParam.FaceSmooth.u4FD_Lost_MaxCnt * 3)
                {
                    m_bFaceAEAreaChage = MFALSE;
                }
                m_u4WOFDCnt ++;
            }
            // Set AAO pointer to algorithm
            AaaTimer localTimer("AESatistic", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            AAA_TRACE_ALG(setAESatisticBufferAddr);
            if (m_bAEMgrDebugEnable || !m_bCCUAEFlag || (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING)) || (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)))
            {
                m_strAAOProcInfo.i8AAOTimeStamp = i8TimeStamp;
                m_pIAeAlgo->setAESatisticBufferAddr_v4p0(m_pPSOStatBuf, pAEStatisticBuf, &m_strAAOProcInfo);
            }
            AAA_TRACE_END_ALG;
            localTimer.End();
        }
        else
        {
            AE_LOG("[%s()] The AAO buffer pointer is NULL, i4SensorDev:%d line:%d u4AAOUpdate:%d i4ActiveAEItem:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, u4AAOUpdate, i4ActiveAEItem);
        }
    }
    else
    {
        AE_LOG("[%s():1] The AE algo class is NULL, i4SensorDev:%d \n", __FUNCTION__, m_eSensorDev);
    }
    // Schedule AE
    AE_FLOW_SCHEDULE_INFO_T inputinfo;
    AE_FLOW_SCHEDULE_OUTPUT_T outputinfo;
    prepareSchedulerParam(&inputinfo, E_AE_FLOW_CPU_DOPVAE, bAAASchedule, i4ActiveAEItem);
    m_pIAeFlowCPU->schedulePvAE((MVOID *) &inputinfo, (MVOID *) &outputinfo);
    copyScheduleInfo2mgr(&outputinfo, bCalculateAE, bApplyAE);
    MINT32 i4PVCwvY;

    // AE Auto Test
    if (m_pIAeAlgo != NULL)
    {
        i4PVCwvY = m_pIAeAlgo->getBrightnessAverageValue();
        //AE_LOG_IF(m_3ALogEnable, "[%s()] i4PVCwvY = %d", __FUNCTION__, i4PVCwvY);
    }
    else
    {
        i4PVCwvY = 0;
    }
    m_pIAeTest->monitorAvgY(i4PVCwvY);
    // AE Auto Test - Perframe control
    if (m_pIAeTest->IsPerframeCtrl())
    {
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
        AAA_TRACE_END_D;
        return S_AE_OK;
    }
    strAEOutput rAEOutput;
    memset(&rAEOutput, 0, sizeof(strAEOutput));
    if ((m_eAEMode == LIB3A_AE_MODE_OFF) &&
        (m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime != 0) &&
        (m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity != 0))
    {
        if ((bAAASchedule == MFALSE) && m_bInitWaitFaceSkipAE && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)))
        {
            // force pause CCU if CCU AE is not working
            AE_LOG("[%s()] AE MODE OFF, CCU AE not working, CCU pause", __FUNCTION__);
            m_pIAeFlowCCU->pause();
        }
        if (bAAASchedule == MFALSE)
            UpdateSensorISPParams(AE_SENSOR_PER_FRAME_STATE);
        else if (bApplyAE)
            UpdateSensorISPParams(AE_SENSOR_PER_FRAME_STATE);
        m_eAEState = MTK_CONTROL_AE_STATE_INACTIVE;
    }
    else if (m_bEnableAE)
    {
        if (bCalculateAE)
        {
            // Calculte AE
            if (m_eZoomWinInfo.bZoomChange == MTRUE)
            {
                if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET))
                {
                    m_rAEStatCfg.u4EndBankIdx = m_u4CCUEndBankIdx;
                    m_rAEStatCfg.u4UseDefaultHight = MFALSE;
                }
                else
                {
                    m_rAEStatCfg.u4UseDefaultHight = MTRUE;
                }
                if (m_pIAeAlgo != NULL)
                {
                    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);
                }
                else
                {
                    AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                }
                m_eZoomWinInfo.bZoomChange = MFALSE;
                // Update AE histogram window config
                AAA_TRACE_D("doPvAEStatConfig");
                ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAEStatCfg);
                AAA_TRACE_END_D;
                // Update CPU Info to CCU if CCU working
                if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET))
                    updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
                AE_LOG("[%s()] AE config change (Skip AeAlgo once)\n", __FUNCTION__);
            }
            else if ((m_eSensorDev == ESensorDev_Sub) && INIT_WAIT_FD_SKIP_AE && m_bInitWaitFaceSkipAE && (i4PVCwvY > PV_CWVY_FLOOR) && (i4PVCwvY < PV_CWVY_CEILING))
            {
                //temp patch for customer
                m_u4InitWaitFaceCount++;
                if (m_u4InitWaitFaceCount >= INIT_WOF_WAIT_CYCLE) {
                    AE_LOG("[%s()] InitWaitFace Skip AE End Count:%d SkipAE:%d i4PVCwvY:%d, INIT_WOF_WAIT_CYCLE:%d INIT_WAIT_FD_SKIP_AE:%d, also skip CCU setting", __FUNCTION__, m_u4InitWaitFaceCount, m_bInitWaitFaceSkipAE, i4PVCwvY, INIT_WOF_WAIT_CYCLE, INIT_WAIT_FD_SKIP_AE);
                    m_bInitWaitFaceSkipAE = MFALSE;
                    m_u4InitWaitFaceCount = 0;
                } else if(m_u4InitWaitFaceCount == 1) {
                    AE_LOG("[%s()] InitWaitFace Skip AE Start Count:%d SkipAE:%d i4PVCwvY:%d INIT_WOF_WAIT_CYCLE:%d INIT_WAIT_FD_SKIP_AE:%d, also skip CCU setting", __FUNCTION__, m_u4InitWaitFaceCount, m_bInitWaitFaceSkipAE, i4PVCwvY, INIT_WOF_WAIT_CYCLE, INIT_WAIT_FD_SKIP_AE);
                }
            }
            else
            {
                if ((!u1AeMode) && (m_eAEMode != LIB3A_AE_MODE_OFF))
                    AE_LOG("[%s()] skip AE one time due to manual control AAO\n", __FUNCTION__);
                else if (m_pIAeAlgo != NULL)
                {
                    if (m_bRestoreAE == MFALSE)
                    {
                        if ((m_bHDRshot == MFALSE) && (m_u4HDRcountforrevert < 1))
                        {
                            if (m_u4HFSMSkipFrame == 0)
                            {
                                if (m_bAEMgrDebugEnable || !m_bCCUAEFlag || (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING)))
                                {
                                    AE_FLOW_CALCULATE_INFO_T inputinfo;
                                    prepareCalculateParam(&inputinfo, E_AE_FLOW_CPU_DOPVAE , pAEStatisticBuf, i8TimeStamp);
                                    AAA_TRACE_D("doPvAEAlgo");
                                    m_pIAeFlowCPU->calculateAE((MVOID*) &inputinfo, (MVOID*) &rAEOutput);
                                    AAA_TRACE_END_D;
                                    copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, (strAEOutput*) &rAEOutput);
                                    m_bAEStable = rAEOutput.bAEStable;
                                }
                                m_bInitWaitFaceSkipAE = MFALSE;
                                m_u4InitWaitFaceCount = 0;
                            }
                            else
                            {
                                mPreviewMode = mPreviewModeBackup;
                                m_u4HFSMSkipFrame--;
                                AE_LOG("[%s()] m_u4HFSMSkipFrame:%d, Backup:%d/%d/%d\n", __FUNCTION__,
                                    m_u4HFSMSkipFrame,
                                    mPreviewMode.u4Eposuretime,
                                    mPreviewMode.u4AfeGain,
                                    mPreviewMode.u4IspGain);
                                m_bAEStable = MFALSE;
                            }
                        }
                        else
                        {
                            m_bAEStable = MFALSE;
                        }
                        mPreviewMode = m_rAEOutput.rPreviewMode;
                        m_i4WaitVDNum = 0; // reset the delay frame
                        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
                    }
                    else
                    {
                        if (m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_RESTORE_WAIT))
                        {
                            m_bRestoreAE = MTRUE;
                        }
                        else
                        {
                            m_bRestoreAE = MFALSE;
                        }
                        bRestore = 1;
                        AE_LOG("[%s()] Restore AE, skip AE one time\n", __FUNCTION__);
                    }
                }
                else
                {
                    AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                }
                // Param adjustment at stable
                if (m_bAEStable)
                {
                    if (m_bAFAELock == MFALSE)
                    {
                        if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
                        {
                            m_u4StableYValue = m_u4CCUResultCwvyStable;
                        }
                        else
                        {
                            m_u4StableYValue = m_pIAeAlgo->getBrightnessCenterWeightedValue();
                        }
                    }
                    if (m_bTouchAEAreaChage == MTRUE)
                    {
                        m_bTouchAEAreaChage = MFALSE;
                        if (!m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableTouchSmooth)
                            m_bPerframeAEFlag = MFALSE;
                        AE_LOG("[%s()] Disable one shot for AE stable\n", __FUNCTION__);
                    }
                }

                // AE state control
                if (m_bAEStable == MTRUE)
                {
                    if (m_bAPAELock == MFALSE)
                    {
                        /*
                        if (m_bAFAELock == MTRUE)
                        {
                            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                        }
                        else
                        {
                            m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
                        }
                        */
                        m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
                    }
                    else
                    {
                        m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
                        prepareCapParams(MFALSE,MFALSE);
                    }
                }
                else
                {
                    if (m_bAPAELock == MTRUE)
                    {
                        m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
                        prepareCapParams(MFALSE,MFALSE);
                    }
                    else if (m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_MANUAL_OVERSPEC))
                    {
                        m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
                        AE_LOG("[%s()] AE state change to converged by manual over spec bManualOverSpec:%d\n", __FUNCTION__, m_pIAeFlowCPU->queryStatus(E_AE_FLOW_CPU_MANUAL_OVERSPEC));
                    }
                    else
                    {
                        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                    }
                }
                if (m_bLockExposureSetting == MTRUE)
                {
                    AE_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
                    AAA_TRACE_END_D;
                    return S_AE_OK;
                }
                if (bApplyAE)
                {
                    // apply AE
                    // Calculate Flare and Apply
                    if ((m_bIsAutoFlare == MTRUE) && (m_bRestoreAE == MFALSE))
                    {
                        if (m_bAEMgrDebugEnable || !m_bCCUAEFlag || (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING)))
                        {
                            AaaTimer localTimer("DoPreFlare", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                            AAA_TRACE_ALG(DoPreFlare);
                            m_pIAeAlgo->DoPreFlare(pAEStatisticBuf);
                            AAA_TRACE_END_ALG;
                            localTimer.End();
                        }
                    }
                    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_ADDCOUNT);
                    if (bIsHDRflow)
                    {
                        UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                        if (m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth && (m_eAETargetMode != AE_MODE_MVHDR_TARGET) && (m_eAETargetMode != AE_MODE_4CELL_MVHDR_TARGET))
                        {
                            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth;
                        }
                    }
                    else
                    {
                        if (bRestore == 0)
                        {
                            if ((m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING)) && (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET)))
                                AE_LOG("[%s()] CCU result not get, skip update sensor & AEinfo\n", __FUNCTION__);
                            else
                                UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
                        }
                        else
                        {
                            UpdateSensorISPParams(AE_AF_RESTORE_STATE);
                        }
                    }
                    NeedCPUUpdateSensor = MFALSE;
                }
            }
        }
        else if (bApplyAE)
        {
            // apply AE
            if (m_pIAeAlgo != NULL)
            {
                AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d AE Wait Vd frame:%d Enable:%d avgY:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum, m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue());
                // continue update for preview or AF state
                // Calculate Flare and Apply
                if ((m_bIsAutoFlare == MTRUE) && (m_bRestoreAE == MFALSE))
                {
                    if (m_bAEMgrDebugEnable || !m_bCCUAEFlag || (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING)))
                    {
                        AaaTimer localTimer("DoPreFlare", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
                        AAA_TRACE_ALG(DoPreFlare);
                        m_pIAeAlgo->DoPreFlare(pAEStatisticBuf);
                        AAA_TRACE_END_ALG;
                        localTimer.End();
                    }
                }
                m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_ADDCOUNT);
                if (bIsHDRflow)
                {
                    UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                }
                else
                {
                    m_bPerframeAEFlag = MFALSE;
                    if ((m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING)) && (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_RESULT_GET))) AE_LOG("[%s()] CCU result not get, skip update sensor & AEinfo\n", __FUNCTION__);
                        else UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                    m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
                }
                NeedCPUUpdateSensor = MFALSE;
            }
        }
        if ((m_bHDRshot == MFALSE) && (m_u4HDRcountforrevert > 0))
        {
            m_u4HDRcountforrevert--;
            AE_LOG("[%s()] HDRshot/HDRcount = %d/%d, bApplyAE:%d\n", __FUNCTION__, m_bHDRshot, m_u4HDRcountforrevert, bApplyAE);
        }
    }
    else
    {
        AE_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }
    AAA_TRACE_END_D;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPvAE_TG_INT(MVOID * /* pAEStatBuf */ ) /* fix build warning */
{
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doAFAEAAO(MVOID *pAEStatBuf, MUINT32 u4AAOUpdate)
{
    MBOOL bAAOUpdate = 0;
    MVOID *pAEStatisticBuf;

    if (m_u4HwMagicNum != u4AAOUpdate)
    {
        m_u4HwMagicNum = u4AAOUpdate;
        bAAOUpdate = MTRUE;
    }
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
        AE_FLOW_CONTROL_INFO_T controlInfo;
        AE_CORE_MAIN_OUT CCUoutput;
        prepareCCUPvParam(&controlInfo);
        m_pIAeFlowCCU->setupParamPvAE((MVOID *)&controlInfo, (MVOID *) &CCUoutput);
        copyCCUAEInfo2mgr(&CCUoutput);
        updateAEInfo2ISP(AE_AF_STATE, m_rAEOutput.rAFMode.u4IspGain);
    }

    // Get mVHDR mode buffer address
    if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
    {
        mVHDRInputParam_T rInput;
        mVHDROutputParam_T rOutput;
        // Dequeue AE DMA buffer
        rInput.u4SensorID = 0; //TBD
        rInput.u4OBValue = 0;
        rInput.u4ISPGain = m_strIVHDROutputSetting.u4LEISPGain; //m_rAEOutput.rPreviewMode.u4IspGain;
        rInput.u2StatWidth = m_rSensorVCInfo.u2VCStatWidth;
        rInput.u2StatHight = m_rSensorVCInfo.u2VCStatHeight;
        if (m_rSensorVCInfo.u2VCModeSelect == 0)
        {
            rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        }
        else
        {
            if (m_strIVHDROutputSetting.u4SEExpTimeInUS != 0)
            {
                rInput.u2ShutterRatio = (m_strIVHDROutputSetting.u4LEExpTimeInUS + (m_strIVHDROutputSetting.u4SEExpTimeInUS >> 1)) / m_strIVHDROutputSetting.u4SEExpTimeInUS;
            }
            else
            {
                rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
                AE_LOG("[%s()] The AE algo class is NULL i4SensorDev = %d u4SEExpTimeInUS:%d\n", __FUNCTION__, m_eSensorDev, m_strIVHDROutputSetting.u4SEExpTimeInUS);
            }
        }
        rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        rInput.pDataPointer = reinterpret_cast <MVOID *> (pAEStatBuf);
        decodemVHDRStatistic(rInput, rOutput);
        bUpdateSensorAWBGain = rOutput.bUpdateSensorAWB;
        pAEStatisticBuf = reinterpret_cast <MVOID *> (rOutput.pDataPointer);
    }
    else
    {
        pAEStatisticBuf = pAEStatBuf;
    }

    if (m_bEnableAE && (m_eAEMode != LIB3A_AE_MODE_OFF))
    {
        if ((m_pIAeAlgo != NULL) && (pAEStatisticBuf != NULL))
        {
            m_pIAeAlgo->setAESatisticBufferAddr_v4p0(m_pPSOStatBuf, pAEStatisticBuf, &m_strAAOProcInfo);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class or AAO is NULL, i4SensorDev = %d \n", __FUNCTION__, m_eSensorDev);
        }
        if (m_pIAeAlgo != NULL)
        {
            MUINT32 CWVY = 0;
            if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
            {
                CWVY = m_u4CCUResultCwvy;
            }
            else
            {
                CWVY = m_pIAeAlgo->getBrightnessCenterWeightedValue();
            }
            AE_LOG("[%s()] Magic:%d Stable:%d VdCnt:%d FaceWOCnt:%d TouchArea:%d u4CwvYcur:%d u4CwvYpre:%d u4AvgYcur:%d\n", __FUNCTION__,
                m_u4HwMagicNum, m_bAEStable, m_i4WaitVDNum,
                m_u4WOFDCnt, m_bTouchAEAreaChage, CWVY, m_u4StableYValue, m_pIAeAlgo->getBrightnessAverageValue());
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doAFAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule,MBOOL bIsFlashCapture)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    //AE_INFO_T rAEInfo2ISP; /* fix build warning */
    MBOOL bCalculateAE = MFALSE;
    MBOOL bApplyAE = MFALSE;
    MVOID *pAEStatisticBuf = NULL;
    MBOOL bIsHDRFlow = ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET));

    memset(&rAEOutput, 0, sizeof(strAEOutput));
    memset(&rAEInput , 0, sizeof(strAEInput));
    if (m_u4HwMagicNum != u4AAOUpdate)
    {
        m_u4HwMagicNum = u4AAOUpdate;
    }
    AE_LOG("[%s()]:%d %d i8TimeStamp:%lld ActiveAE:%d eAETargetMode:%d bTouchAEAreaChage:%d m_AEState:%d bIsFlashCapture=%d\n", __FUNCTION__, m_i4TimeOutCnt, m_i4WaitVDNum, (long long)i8TimeStamp, i4ActiveAEItem, m_eAETargetMode, m_bTouchAEAreaChage, m_AEState,bIsFlashCapture);
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
      if(bIsFlashCapture)
      {
        m_pIAeFlowCCU->pause();
        AE_LOG( "[doAFAE] pause CCU\n");
        // Control CPU Start
        AE_FLOW_CPUSTART_INFO_T flowstartInfo;
        prepareCPUStartParam(&flowstartInfo);
        m_pIAeFlowCPU->start((MVOID*) &flowstartInfo);
      }else
      {
        AE_FLOW_CONTROL_INFO_T controlInfo;
        AE_CORE_MAIN_OUT CCUoutput;
        prepareCCUPvParam(&controlInfo);
        m_pIAeFlowCCU->setupParamPvAE((MVOID *)&controlInfo, (MVOID *) &CCUoutput);
        copyCCUAEInfo2mgr(&CCUoutput);
      }
    }
    // Shound be forced to be AF params when timeout
    if (m_i4TimeOutCnt > 18)
    {
        AE_LOG("[%s()] Time out happen\n", __FUNCTION__);
        if (m_bLockExposureSetting == MTRUE)
        {
            AE_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
            return S_AE_OK;
        }
        m_rAEOutput.rAFMode = mPreviewMode;
        UpdateSensorISPParams(AE_AF_STATE);
        return S_AE_OK;
    }
    else
    {
        m_i4TimeOutCnt++;
    }

    // Get mVHDR mode buffer address
    if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
    {
        mVHDRInputParam_T rInput;
        mVHDROutputParam_T rOutput;
        // Dequeue AE DMA buffer
        rInput.u4SensorID = 0; //TBD
        rInput.u4OBValue = 0;
        rInput.u4ISPGain = m_strIVHDROutputSetting.u4LEISPGain; //m_rAEOutput.rPreviewMode.u4IspGain;
        rInput.u2StatWidth = m_rSensorVCInfo.u2VCStatWidth;
        rInput.u2StatHight = m_rSensorVCInfo.u2VCStatHeight;
        if (m_rSensorVCInfo.u2VCModeSelect == 0)
        {
            rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        }
        else
        {
            if (m_strIVHDROutputSetting.u4SEExpTimeInUS != 0)
            {
                rInput.u2ShutterRatio = (m_strIVHDROutputSetting.u4LEExpTimeInUS + (m_strIVHDROutputSetting.u4SEExpTimeInUS >> 1)) / m_strIVHDROutputSetting.u4SEExpTimeInUS;
            }
            else
            {
                rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
                AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d u4SEExpTimeInUS:%d\n", __FUNCTION__, m_eSensorDev, m_strIVHDROutputSetting.u4SEExpTimeInUS);
            }
        }
        rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
        rInput.pDataPointer = reinterpret_cast <MVOID *> (pAEStatBuf);
        decodemVHDRStatistic(rInput, rOutput);
        bUpdateSensorAWBGain = rOutput.bUpdateSensorAWB;
        pAEStatisticBuf = reinterpret_cast <MVOID *> (rOutput.pDataPointer);
    }
    else
    {
        pAEStatisticBuf = pAEStatBuf;
    }

    if (m_bEnableAE && (m_eAEMode != LIB3A_AE_MODE_OFF))
    {
        if ((m_pIAeAlgo != NULL) && (pAEStatisticBuf != NULL))
        {
            Mutex::Autolock lock(m_Lock);
            // Set FDarea to algorithm
            if (m_bFaceAEAreaChage == MTRUE)
            {
                // Face AE window change
                AAA_TRACE_ALG(setAEFDArea);
                AAA_TRACE_D("setAEFDArea");
                m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
                AAA_TRACE_END_D;
                AAA_TRACE_END_ALG;
                if (m_u4WOFDCnt > m_rAEInitInput.rAENVRAM->rAeParam.FaceSmooth.u4FD_Lost_MaxCnt *3)
                {
                    m_bFaceAEAreaChage = MFALSE;
                }
                m_u4WOFDCnt ++;
            }
            //AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain(); /* fix build warning */
            AAA_TRACE_ALG(setAESatisticBufferAddr);
            m_strAAOProcInfo.i8AAOTimeStamp = i8TimeStamp;
            m_pIAeAlgo->setAESatisticBufferAddr_v4p0(m_pPSOStatBuf, pAEStatisticBuf, &m_strAAOProcInfo);
            AAA_TRACE_END_ALG;
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d u4AAOUpdate:%d i4ActiveAEItem:%d\n", __FUNCTION__, m_eSensorDev, __LINE__, u4AAOUpdate, i4ActiveAEItem);
        }

        // Schedule AE
        AE_FLOW_SCHEDULE_INFO_T inputinfo;
        AE_FLOW_SCHEDULE_OUTPUT_T outputinfo;
        prepareSchedulerParam(&inputinfo, E_AE_FLOW_CPU_DOAFAE, bAAASchedule, i4ActiveAEItem);
        m_pIAeFlowCPU->schedulePvAE((MVOID *) &inputinfo, (MVOID *) &outputinfo);
        copyScheduleInfo2mgr(&outputinfo, bCalculateAE, bApplyAE);

        if (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
        {
            m_bAEStable = MFALSE;
        }
        else
        {
            AE_LOG("[%s()] CCU working, skip set m_bAEStable = MFALSE\n", __FUNCTION__);
        }
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
        m_bTouchAEAreaChage = MFALSE;

        if (bCalculateAE)
        {
            if (bIsHDRFlow && (m_eAETargetMode != AE_MODE_4CELL_MVHDR_TARGET))
            {
                m_bAEStable = MTRUE;
                m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
            }
            else
            {
                AE_FLOW_CALCULATE_INFO_T inputinfo;
                prepareCalculateParam(&inputinfo, E_AE_FLOW_CPU_DOAFAE , pAEStatisticBuf, i8TimeStamp);
                AAA_TRACE_D("doAFAEAlgo");
                m_pIAeFlowCPU->calculateAE((MVOID*) &inputinfo, (MVOID*) &rAEOutput);
                AAA_TRACE_END_D;
                if (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
                {
                    copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);
                }
                else
                {
                    AE_LOG("[%s()] CCU working, don't update CPU AE info\n", __FUNCTION__);
                }
                m_rAFAEOutput = rAEOutput;
                mPreviewMode = m_rAEOutput.rPreviewMode;
                m_i4WaitVDNum = 0; // reset the delay frame

                // AE is stable, change to AF state
                if (rAEOutput.bAEStable == MTRUE)
                {
                    inputinfo.eAEFlowType = E_AE_FLOW_CPU_DOAFASSIST;
                    m_pIAeFlowCPU->calculateAE((MVOID*) &inputinfo, (MVOID*) &rAEOutput);
                    copyAEInfo2mgr(&m_rAEOutput.rAFMode, &rAEOutput);
                    m_rAFAEOutput = rAEOutput;
                    m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
                    m_AEState = AE_AF_STATE;
                    if (m_eAEState != MTK_CONTROL_AE_STATE_PRECAPTURE)
                    {
                        m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
                    }
                    AE_LOG("[%s()] m_AEState = AE_AF_STATE m_eAEState:%d\n", __FUNCTION__, m_eAEState);
                }
                else
                {
                    if (m_eAEState != MTK_CONTROL_AE_STATE_PRECAPTURE)
                    {
                        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                    }
                    AE_LOG("[%s()] m_eAEState:%d\n", __FUNCTION__, m_eAEState);
                }

                if (m_bLockExposureSetting == MTRUE)
                {
                    AE_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
                    return S_AE_OK;
                }
                // Using preview state to do AE before AE stable
                if (bApplyAE)
                {
                    // apply AE
                    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_ADDCOUNT);
                    // continue update for preview or AF state
                    if (m_AEState == AE_AF_STATE)
                    {
                        UpdateSensorISPParams(AE_AF_STATE);
                    }
                    else if (bIsHDRFlow)
                    {
                        UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                        if ((m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth) && ((m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_IVHDR_TARGET)))
                        {
                            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth;
                        }
                    }
                    else
                    {
                        if (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableTouchSmooth && (!bAAASchedule))
                            m_bPerframeAEFlag = MTRUE;
                        else
                            m_bPerframeAEFlag = MFALSE;
                        UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                        // Save to Evsetting Queue (for algorithm)
                        m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
                    }
                }
            }
        }
        else if (bApplyAE)
        {
            //apply AE
            if (m_pIAeAlgo != NULL)
            {
                AE_LOG("[%s()] i4SensorDev:%d AE Wait Vd frame:%d Enable:%d avgY:%d State:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum, m_bEnableAE, m_pIAeAlgo->getBrightnessAverageValue(), m_AEState);
            }
            // continue update for preview or AF state
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_ADDCOUNT);
            if (m_AEState == AE_AF_STATE)
            {
                UpdateSensorISPParams(AE_AF_STATE);
            }
            else if (bIsHDRFlow)
            {
                UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
            }
            else
            {
                m_bPerframeAEFlag = MFALSE;
                UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                // Save to Evsetting Queue (for algorithm)
                m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
            }
        }
    }
    else
    {
        m_bAEStable = MTRUE;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
        AE_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPreCapAE(MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    (void)u4AAOUpdate;
    //AE_INFO_T rAEInfo2ISP; /* fix build warning */
    MVOID *pAEStatisticBuf;
    MBOOL bCalculateAE = MFALSE;
    MBOOL bApplyAE = MFALSE;
    MBOOL bIsHDRFlow = ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET));
    strAEOutput rAEOutput;

    if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
        if (m_bAEStable)
            m_bIsCPUNeedDebugInfo = MTRUE;
        else
            m_bIsCPUNeedDebugInfo = MFALSE;
        AE_LOG("[%s()] CPUNeedDebugInfo:%d", __FUNCTION__, m_bIsCPUNeedDebugInfo);
        m_pIAeFlowCCU->pause();
    }

    memset(&rAEOutput, 0, sizeof(strAEOutput));
    if (m_bEnableAE && (m_eAEMode != LIB3A_AE_MODE_OFF))
    {
        if ((m_bAEReadyCapture == MFALSE) || (m_bCShot == MTRUE))
        {
            AE_LOG("[%s()] i4SensorDev:%d Ready:%d isStrobe:%d TimeOut:%d \n", __FUNCTION__, m_eSensorDev, m_bAEReadyCapture, bIsStrobeFired, m_i4TimeOutCnt);
            if (((m_i4TimeOutCnt > 18) && (m_bStrobeOn == MFALSE) && (m_bCShot == MFALSE)) || ((m_i4TimeOutCnt > 29) && ((m_bCShot == MTRUE) || (m_bStrobeOn == MTRUE))))
            {
                AE_LOG("[%s()] Time out happen\n", __FUNCTION__);
                if (m_bLockExposureSetting == MTRUE)
                {
                    AE_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
                    return S_AE_OK;
                }
                m_rAEOutput.rAFMode = m_rAEOutput.rPreviewMode;
                UpdateSensorISPParams(AE_AF_STATE);
                return S_AE_OK;
            }
            else
            {
                m_i4TimeOutCnt++;
            }

            if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
            {
                mVHDRInputParam_T rInput;
                mVHDROutputParam_T rOutput;
                // Dequeue AE DMA buffer
                rInput.u4SensorID = 0; //TBD
                rInput.u4OBValue = 0;
                rInput.u4ISPGain = m_rAEOutput.rPreviewMode.u4IspGain;
                rInput.u2StatWidth = m_rSensorVCInfo.u2VCStatWidth;
                rInput.u2StatHight = m_rSensorVCInfo.u2VCStatHeight;
                rInput.u2ShutterRatio = m_rSensorVCInfo.u2VCShutterRatio;
                rInput.pDataPointer = reinterpret_cast <MVOID *> (pAEStatBuf);
                decodemVHDRStatistic(rInput, rOutput);
                bUpdateSensorAWBGain = rOutput.bUpdateSensorAWB;
                pAEStatisticBuf = reinterpret_cast <MVOID *> (rOutput.pDataPointer);
            }
            else
            {
                pAEStatisticBuf = pAEStatBuf;
            }

            if ((m_bAEStable == MFALSE) || (m_bCShot == MTRUE) || m_bIsCPUNeedDebugInfo)
            {
                // AAO pointer to algorithm
                if ((m_pIAeAlgo != NULL) && (pAEStatBuf != NULL))
                {
                    Mutex::Autolock lock(m_Lock);
                    // Set FDarea to algorithm
                    if (m_bFaceAEAreaChage == MTRUE)
                    {
                        // Face AE window change
                        AAA_TRACE_ALG(setAEFDArea);
                        AAA_TRACE_D("setAEFDArea");
                        m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
                        AAA_TRACE_END_D;
                        AAA_TRACE_END_ALG;
                        if (m_u4WOFDCnt > m_rAEInitInput.rAENVRAM->rAeParam.FaceSmooth.u4FD_Lost_MaxCnt *3)
                        {
                            m_bFaceAEAreaChage = MFALSE;
                        }
                        m_u4WOFDCnt ++;
                        AE_LOG("[%s()] AAO pointer to algorithm", __FUNCTION__);
                    }
                    m_strAAOProcInfo.i8AAOTimeStamp = i8TimeStamp;
                    m_pIAeAlgo->setAESatisticBufferAddr_v4p0(m_pPSOStatBuf, pAEStatisticBuf, &m_strAAOProcInfo);
                }
                else
                {
                    AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                }
                // AE scheduler
                AE_FLOW_SCHEDULE_INFO_T inputinfo;
                AE_FLOW_SCHEDULE_OUTPUT_T outputinfo;
                prepareSchedulerParam(&inputinfo, E_AE_FLOW_CPU_DOPRECAPAE, bAAASchedule, i4ActiveAEItem);
                m_pIAeFlowCPU->schedulePvAE((MVOID *) &inputinfo, (MVOID *) &outputinfo);
                copyScheduleInfo2mgr(&outputinfo, bCalculateAE, bApplyAE);
                if (m_bIsCPUNeedDebugInfo)
                {
                    bCalculateAE = MTRUE;
                    bApplyAE = MTRUE;
                    AE_LOG("[%s()] CPU AE needs calculation for debug info, bCalculateAE = %d, bApplyAE = %d", __FUNCTION__, bCalculateAE, bApplyAE);
                }

                if (bCalculateAE)
                {
                    AE_FLOW_CALCULATE_INFO_T inputinfo;
                    prepareCalculateParam(&inputinfo, E_AE_FLOW_CPU_DOPRECAPAE , pAEStatisticBuf, i8TimeStamp);
                    AAA_TRACE_D("doPreCapAEAlgo");
                    m_pIAeFlowCPU->calculateAE((MVOID*) &inputinfo, (MVOID*) &rAEOutput);
                    AAA_TRACE_END_D;
                    if (!m_bIsCPUNeedDebugInfo)
                        copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput); // please keep if (!m_bIsCPUNeedDebugInfo)
                    m_i4WaitVDNum = 0; // reset the delay frame
                    mPreviewMode = m_rAEOutput.rPreviewMode;
                    if (m_bCShot == MTRUE)
                    {
                        m_bAEStable = rAEOutput.bAEStable;
                    }
                    if (m_bLockExposureSetting == MTRUE)
                    {
                        m_bAEStable = MTRUE;
                        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
                        AE_LOG("[%s()] Lock sensor setting:%d\n", __FUNCTION__, m_bLockExposureSetting);
                        return S_AE_OK;
                    }
                    // AE is stable, update capture info
                    if (m_bAEStable == MTRUE || m_bIsCPUNeedDebugInfo == MTRUE)
                    {
                        m_bIsCPUNeedDebugInfo = MFALSE;
                        UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                        AE_LOG("[%s()] AE stable already, update CPU algo\n", __FUNCTION__);
                    }
                    else if (rAEOutput.bAEStable == MTRUE)
                    {
                        if (m_bStrobeOn)
                            m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
                        UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                    }
                    else if (bApplyAE)
                    {
                        // Using preview state to do AE before AE stable
                        if (bIsHDRFlow)
                        {
                            UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                        }
                        else
                        {
                            m_bPerframeAEFlag = MFALSE;
                            UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
                        }
                    }
                }
                else if (bApplyAE)
                {
                    // apply AE
                    AE_LOG("[%s()] i4SensorDev:%d AE Wait Vd frame:%d Enable:%d State:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum, m_bEnableAE, m_AEState);
                    // continue update for preview or AF state
                    if (bIsHDRFlow)
                    {
                        UpdateSensorISPParams(AE_TWO_SHUTTER_FRAME_STATE);
                    }
                    else
                    {
                        m_bPerframeAEFlag = MFALSE;
                        UpdateSensorISPParams(AE_AUTO_FRAMERATE_STATE);
                        m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
                    }
                }
            }
            else
            {
                AE_LOG("[%s()] AE stable already\n", __FUNCTION__);
                UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
                //doBackAEInfo(); // do back up AE for Precapture AF state.
            }

        }
        else
        {
            m_bAEStable = MTRUE;
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
            AE_LOG("[%s()] i4SensorDev:%d Do Nothing Ready:%d isStrobe:%d\n", __FUNCTION__, m_eSensorDev, m_bAEReadyCapture, bIsStrobeFired);
        }
    }
    else
    {
        m_bAEStable = MTRUE;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
        AE_LOG("[%s()] AE don't enable Enable:%d\n", __FUNCTION__, m_bEnableAE);
    }

    m_eAEState = MTK_CONTROL_AE_STATE_PRECAPTURE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEState2Converge()
{
    AE_LOG("[%s] Chage to converge state, m_eAEState:%d->%d\n", __FUNCTION__, m_eAEState, MTK_CONTROL_AE_STATE_CONVERGED);
    m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapAE()
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    memset(&rAEOutput, 0, sizeof(strAEOutput));
    memset(&rAEInput, 0, sizeof(strAEInput));
    MBOOL bIsHDRFlow = ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET));
    // Stop CCU
    if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
    {
        if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
            m_bForceUpdateAEInfoByCCU = MTRUE;
        else
            m_bNeedApplyBackupAEInfo = TRUE;
        AE_LOG("[%s] m_bForceUpdateAEInfoByCCU: %d\n", __FUNCTION__, m_bForceUpdateAEInfoByCCU);
        m_pIAeFlowCCU->pause(MFALSE);
    }
    AE_LOG("[%s] i4SensorDev:%d CamMode:%d TargetMode:%d\n", __FUNCTION__, m_eSensorDev, m_eCamMode, m_eAETargetMode);

    Mutex::Autolock lock(m_Lock);
    // Set FDarea to algorithm
    //m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);

    if ((m_bIsAutoFlare == FALSE) && (m_eSensorMode == ESensorMode_Capture))
    {
        AE_FLOW_CALCULATE_INFO_T inputinfo;
        prepareCalculateParam(&inputinfo, E_AE_FLOW_CPU_DOCAPAE , NULL, 0);
        AAA_TRACE_D("doCapAEAlgo");
        m_pIAeFlowCPU->calculateAE((MVOID*) &inputinfo, (MVOID*) &rAEOutput);
        AAA_TRACE_END_D;
        copyAEInfo2mgr(&mCaptureMode, &rAEOutput);
        AE_LOG("[%s] i4SensorDev:%d Update flare offset:%d Flare Gain:%d\n", __FUNCTION__, m_eSensorDev, mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain);
    }

    if ((m_bStrobeOn == MFALSE) && (m_bRealISOSpeed == MTRUE) && (m_u4AEISOSpeed != (MUINT32) LIB3A_AE_ISO_SPEED_AUTO))
    {
        updateEMISOCaptureParams(mCaptureMode);
    }
    if (((m_eCamMode == eAppMode_EngMode) || (m_bzcHDRshot == MTRUE)) && bIsHDRFlow)
    {
        // engineer mode and target mode
        UpdateSensorISPParams(AE_TWO_SHUTTER_CAPTURE_STATE);
    }
    else
    {
        UpdateSensorISPParams(AE_CAPTURE_STATE);
        // cHDR Normal capture
        if ((m_bAEReadyCapture == MFALSE) && (m_bAECaptureUpdate == MTRUE) && (m_bStrobeOn == MFALSE) && (m_bHDRshot == MFALSE))
        {
            AE_CORE_CTRL_CPU_EXP_INFO rAECoreCtrlCPUInfo;
            memset(&rAECoreCtrlCPUInfo, 0, sizeof(AE_CORE_CTRL_CPU_EXP_INFO));
            rAECoreCtrlCPUInfo.u4Eposuretime = mCaptureMode.u4Eposuretime;
            rAECoreCtrlCPUInfo.u4AfeGain = mCaptureMode.u4AfeGain;
            rAECoreCtrlCPUInfo.u4IspGain = mCaptureMode.u4IspGain;
            rAECoreCtrlCPUInfo.m_u4EffectiveIndex = m_i4AEidxNext;
            m_pIAeFlowCPU->update(&rAECoreCtrlCPUInfo);
        }
    }
    m_bAECaptureUpdate = MFALSE; // reset have been Updated Capture param flag
    m_SensorQueueCtrl.uOutputIndex = m_SensorQueueCtrl.uInputIndex;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT AeMgr::doBackAEInfo(MBOOL bTorchMode)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    memset(&rAEInput, 0, sizeof(strAEInput));

    if (m_bEnableAE)
    {
        AE_LOG("[%s()] i4SensorDev:%d bTorchMode:%d\n", __FUNCTION__, m_eSensorDev, bTorchMode);
        if (bTorchMode)
        {
            mPreviewModeBackupTorch = m_rAEOutput.rPreviewMode;
            mIsBackupTorch = MTRUE;
        }
        else
        {
            rAEInput.eAeState = AE_STATE_BACKUP_PREVIEW;
            rAEInput.pAESatisticBuffer = NULL;
            rAEInput.rHdrGyroInfo.i4Acce[0] = m_i4AcceInfo[0];
            rAEInput.rHdrGyroInfo.i4Acce[1] = m_i4AcceInfo[1];
            rAEInput.rHdrGyroInfo.i4Acce[2] = m_i4AcceInfo[2];
            rAEInput.rHdrGyroInfo.i4Gyro[0] = m_i4GyroInfo[0];
            rAEInput.rHdrGyroInfo.i4Gyro[1] = m_i4GyroInfo[1];
            rAEInput.rHdrGyroInfo.i4Gyro[2] = m_i4GyroInfo[2];

            m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_STORE, MTRUE);

            if (m_pIAeAlgo != NULL)
            {
                m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            }
            else
            {
                AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
            }
        }
    }
    else
    {
        AE_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AeMgr::setExp(int exp)
{
    AE_LOG("[%s()] setExp(%d)", __FUNCTION__, exp);
    AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, exp);
}

void AeMgr::setAfe(int afe)
{
    AE_LOG("[%s()] setAfe(%d)", __FUNCTION__, afe);
    AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, afe);
}

void AeMgr::setIsp(int isp)
{
    AE_LOG("[%s()] setIsp(%d)", __FUNCTION__, isp);
    AE_INFO_T rAEInfo2ISP;
    MUINT32 u4PreviewBaseGain = 1024;
    MUINT32 u4PreviewBaseISO = 100;

    if (m_p3ANVRAM != NULL)
    {
        u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    }
    else
    {
        MY_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
    }

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
    }

    rAEInfo2ISP.i4GammaIdx = m_i4GammaIdx;
    rAEInfo2ISP.i4LESE_Ratio = m_i4LESE_Ratio;
    rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
    rAEInfo2ISP.i4HdrSeg = m_i4HdrSeg;
    rAEInfo2ISP.i4HdrTurn = m_i4HdrTurn;
    rAEInfo2ISP.u4MaxISO = m_u4MaxISO * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100;
    rAEInfo2ISP.u4AEStableCnt = m_u4StableCnt;
    rAEInfo2ISP.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
    rAEInfo2ISP.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
    rAEInfo2ISP.u4IspGain = isp;
    rAEInfo2ISP.u4RealISOValue = u4PreviewBaseISO * ((rAEInfo2ISP.u4AfeGain * isp) >> 10) / u4PreviewBaseGain;
    rAEInfo2ISP.u4EVRatio = m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp];
    rAEInfo2ISP.bAEStable = m_bAEStable;
    rAEInfo2ISP.bAELock = m_bAELock;
    rAEInfo2ISP.bAELimiter = m_bAElimitor;
    rAEInfo2ISP.u4EISExpRatio = m_u4EISExpRatio;
    rAEInfo2ISP.bIsMaxIndexStable = m_bIsMaxIndexStable;
    rAEInfo2ISP.bIsndexSmallOffset = 0;
    rAEInfo2ISP.u4MaxIndexStableCount = m_u4MaxIdxStableCount;
    rAEInfo2ISP.bDisableOBC = 0; //CCU should be disabled when flash -on
    IspTuningMgr::getInstance().setAEInfo((ESensorDev_T)m_eSensorDev, rAEInfo2ISP);
    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, isp >> 1);
}

void AeMgr::setPfPara(int exp, int afe, int isp)
{
    AE_LOG("[%s()] setPfPara(%d/%d/%d)", __FUNCTION__, exp, afe, isp);
    AE_INFO_T rAEInfo2ISP;
    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP);
    }
    rAEInfo2ISP.i4GammaIdx = m_i4GammaIdx;
    rAEInfo2ISP.i4LESE_Ratio = m_i4LESE_Ratio;
    rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
    rAEInfo2ISP.i4HdrSeg = m_i4HdrSeg;
    rAEInfo2ISP.i4HdrTurn = m_i4HdrTurn;
    rAEInfo2ISP.u4MaxISO = m_u4MaxISO * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100;
    rAEInfo2ISP.u4AEStableCnt = m_u4StableCnt;
    rAEInfo2ISP.u4Eposuretime = exp;
    rAEInfo2ISP.u4AfeGain = afe;
    rAEInfo2ISP.u4IspGain = isp;

    if (m_p3ANVRAM != NULL)
    {
        rAEInfo2ISP.u4RealISOValue = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain * ((rAEInfo2ISP.u4AfeGain * isp) >> 10) / 1024;
    }
    else
    {
        rAEInfo2ISP.u4RealISOValue = 100 * ((rAEInfo2ISP.u4AfeGain * isp) >> 10) / 1024;
        MY_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
    }

    rAEInfo2ISP.u4EVRatio = m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp];
    rAEInfo2ISP.bAEStable = m_bAEStable;
    rAEInfo2ISP.bAELock = m_bAELock;
    rAEInfo2ISP.bAELimiter = m_bAElimitor;
    rAEInfo2ISP.u4EISExpRatio = m_u4EISExpRatio;
    rAEInfo2ISP.bDisableOBC = 0; //CCU should be disabled when flash -on
    rAEInfo2ISP.bIsMaxIndexStable = m_bIsMaxIndexStable;
    rAEInfo2ISP.bIsndexSmallOffset = 0;
    rAEInfo2ISP.u4MaxIndexStableCount = m_u4MaxIdxStableCount;
    IspTuningMgr::getInstance().setAEInfo((ESensorDev_T)m_eSensorDev, rAEInfo2ISP);
    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, isp >> 1);
}

void AeMgr::setRestore(int frm)
{
    AE_LOG("[%s()] Frame:%d DelayFrm:%d %d %d\n", __FUNCTION__, frm, m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames);
    int expSetFrm;
    int afeSetFrm;
    int ispSetFrm;
    int minFrmDelay = 0;
    minFrmDelay = m_i4ShutterDelayFrames;
    if (minFrmDelay > m_i4SensorGainDelayFrames)
        minFrmDelay = m_i4SensorGainDelayFrames;
    if (minFrmDelay > m_i4IspGainDelayFrames)
        minFrmDelay = m_i4IspGainDelayFrames;
    expSetFrm = m_i4ShutterDelayFrames - minFrmDelay;
    afeSetFrm = m_i4SensorGainDelayFrames - minFrmDelay;
    ispSetFrm = m_i4IspGainDelayFrames - minFrmDelay;

    AE_LOG("[%s()] exp, afe, isp setFrame %d %d %d\n", __FUNCTION__, expSetFrm, afeSetFrm, ispSetFrm);
    // Set Exposure time
    if (frm == expSetFrm) //on exp
    {
        AE_LOG("[%s()] setExp frm0", __FUNCTION__);
        m_pIAeSettingCPU->updateSensorExp((MVOID*) &g_rExp);
    }
    // Set Afe gain
    if (frm == afeSetFrm) //on afe
    {
        AE_LOG("[%s()] setAfe(%d) frm0", __FUNCTION__, g_rAfe);
        m_pIAeSettingCPU->updateSensorGain((MVOID*) &g_rAfe);
    }
    // Set ISP gain
    if (frm == ispSetFrm) //on isp
    {
        AE_LOG("[%s()] setIsp frm0", __FUNCTION__);
        setIsp(g_rIsp);
    }
    AE_CORE_CTRL_CPU_EXP_INFO rAECoreCtrlCPUInfo;
    memset(&rAECoreCtrlCPUInfo, 0, sizeof(AE_CORE_CTRL_CPU_EXP_INFO));
    rAECoreCtrlCPUInfo.u4Eposuretime = g_rExp;
    rAECoreCtrlCPUInfo.u4AfeGain = g_rAfe;
    rAECoreCtrlCPUInfo.u4IspGain = g_rIsp;
    rAECoreCtrlCPUInfo.m_u4EffectiveIndex = m_i4AEidxNext;
    m_pIAeFlowCPU->update(&rAECoreCtrlCPUInfo);
    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_RESET, MTRUE);
}

MRESULT AeMgr::doRestoreAEInfo(MBOOL bRestorePrvOnly, MBOOL bTorchMode)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    memset(&rAEInput, 0, sizeof(strAEInput));

    if (m_bEnableAE)
    {
        AE_LOG("[%s()] i4SensorDev:%d bRestorePrvOnly:%d bTorchMode:%d\n", __FUNCTION__, m_eSensorDev, bRestorePrvOnly, bTorchMode);
        if (bTorchMode && !mIsBackupTorch)
        {
            AE_LOG("[%s()] Torch AEInfo didn't back up \n", __FUNCTION__);
            return S_AE_OK;
        }
        else if (bTorchMode)
        {
            m_rAEOutput.rPreviewMode = mPreviewModeBackupTorch;
            mIsBackupTorch = MFALSE;
        }
        else
        {
            rAEInput.eAeState = AE_STATE_RESTORE_PREVIEW;
            rAEInput.pAESatisticBuffer = NULL;
            rAEInput.rHdrGyroInfo.i4Acce[0] = m_i4AcceInfo[0];
            rAEInput.rHdrGyroInfo.i4Acce[1] = m_i4AcceInfo[1];
            rAEInput.rHdrGyroInfo.i4Acce[2] = m_i4AcceInfo[2];
            rAEInput.rHdrGyroInfo.i4Gyro[0] = m_i4GyroInfo[0];
            rAEInput.rHdrGyroInfo.i4Gyro[1] = m_i4GyroInfo[1];
            rAEInput.rHdrGyroInfo.i4Gyro[2] = m_i4GyroInfo[2];

            if (m_pIAeAlgo != NULL)
            {
                m_pIAeAlgo->setSensorModeMaxBVdiff(m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
                m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            }
            else
            {
                AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
            }
            copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput);
        }
        AE_LOG("[%s()] Preview Shutter:%d Sensor gain:%d Isp gain:%d flare:%d %d ISO:%d\n", __FUNCTION__,
            m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain,
            m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.u4RealISO);

        g_rExp = m_rAEOutput.rPreviewMode.u4Eposuretime;
        g_rAfe = m_rAEOutput.rPreviewMode.u4AfeGain;
        g_rIsp = m_rAEOutput.rPreviewMode.u4IspGain;

        mPreviewMode = m_rAEOutput.rPreviewMode;

        m_u4PreExposureTime = m_rAEOutput.rPreviewMode.u4Eposuretime;
        m_u4PreSensorGain = m_rAEOutput.rPreviewMode.u4AfeGain;
        m_u4UpdateShutterValue = m_rAEOutput.rPreviewMode.u4Eposuretime;
        m_u4UpdateGainValue = m_rAEOutput.rPreviewMode.u4AfeGain;

        if (bRestorePrvOnly == MFALSE)
        {
            prepareCapParams(MFALSE,MFALSE);
        }

        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
        m_bAEStable = MFALSE;
        m_bAEReadyCapture = MFALSE;
        m_bForceSkipCCU = MFALSE;
        m_bRestoreAE = MTRUE; // restore AE
    }
    else
    {
        AE_LOG("[%s()] i4SensorDev:%d AE don't enable Enable:%d\n", __FUNCTION__, m_eSensorDev, m_bEnableAE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAAOProcInfo(MVOID *pPSOStatBuf, AAO_PROC_INFO_T const *sNewAAOProcInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.setaaoprocinfo.enable", value, "0");
    MBOOL bAAOLogEn = atoi(value);

    m_pPSOStatBuf = pPSOStatBuf;
    memcpy(&m_strAAOProcInfo, sNewAAOProcInfo, sizeof(AAO_PROC_INFO_T));
    m_strAAOProcInfo.OB_Gain_Unit = 512;
    m_strAAOProcInfo.awb_gain_Unit = 512;
    m_strAAOProcInfo.PreGain1_Unit = 512;
    m_strAAOProcInfo.CCM_Gain_Unit = 512;
    m_strAAOProcInfo.LSC_Gain_Unit = 8192;
    m_strAAOProcInfo.RGB2Y_Coef[0] = m_u4AEYCoefR;
    m_strAAOProcInfo.RGB2Y_Coef[1] = m_u4AEYCoefG;
    m_strAAOProcInfo.RGB2Y_Coef[2] = m_u4AEYCoefB;

    if (m_i4AAOmode == 1)
    {
        m_strAAOProcInfo.OB_Offset[0] = 0x3FFF - m_strAAOProcInfo.OB_Offset[0];
        m_strAAOProcInfo.OB_Offset[1] = 0x3FFF - m_strAAOProcInfo.OB_Offset[1];
        m_strAAOProcInfo.OB_Offset[2] = 0x3FFF - m_strAAOProcInfo.OB_Offset[2];
        m_strAAOProcInfo.OB_Offset[3] = 0x3FFF - m_strAAOProcInfo.OB_Offset[3];
    }
    else
    {
        m_strAAOProcInfo.OB_Offset[0] = 0x1FFF - m_strAAOProcInfo.OB_Offset[0];
        m_strAAOProcInfo.OB_Offset[1] = 0x1FFF - m_strAAOProcInfo.OB_Offset[1];
        m_strAAOProcInfo.OB_Offset[2] = 0x1FFF - m_strAAOProcInfo.OB_Offset[2];
        m_strAAOProcInfo.OB_Offset[3] = 0x1FFF - m_strAAOProcInfo.OB_Offset[3];
    }
    AE_LOG_IF(bAAOLogEn, "[%s()] Pso_Output_Path: %d, OB_Offset: %d/%d/%d/%d, OB_Gain: %d/%d/%d/%d\n", __FUNCTION__, m_strAAOProcInfo.Pso_Output_Path,
        m_strAAOProcInfo.OB_Offset[0], m_strAAOProcInfo.OB_Offset[1], m_strAAOProcInfo.OB_Offset[2], m_strAAOProcInfo.OB_Offset[3],
        m_strAAOProcInfo.OB_Gain[0], m_strAAOProcInfo.OB_Gain[1], m_strAAOProcInfo.OB_Gain[2], m_strAAOProcInfo.OB_Gain[3]);
    AE_LOG_IF(bAAOLogEn, "[%s()] awb_gain: %d/%d/%d, PreGain1: %d/%d/%d, CCM: [%d/%d/%d/%d/%d/%d/%d/%d/%d] \n", __FUNCTION__,
        m_strAAOProcInfo.awb_gain.i4R, m_strAAOProcInfo.awb_gain.i4G, m_strAAOProcInfo.awb_gain.i4B,
        m_strAAOProcInfo.PreGain1.i4R, m_strAAOProcInfo.PreGain1.i4G, m_strAAOProcInfo.PreGain1.i4B,
        m_strAAOProcInfo.CCM[0], m_strAAOProcInfo.CCM[1], m_strAAOProcInfo.CCM[2],
        m_strAAOProcInfo.CCM[3], m_strAAOProcInfo.CCM[4], m_strAAOProcInfo.CCM[5],
        m_strAAOProcInfo.CCM[6], m_strAAOProcInfo.CCM[7], m_strAAOProcInfo.CCM[8]);
    for (int i = 0; i < AE_V4P0_BLOCK_NO; i++)
    {
        AE_LOG_IF(bAAOLogEn, "[%s():LSC_Gain_Grid_R - %d] 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x, 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x, 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x\n", __FUNCTION__, i,
            m_strAAOProcInfo.LSC_Gain_Grid_R[i + 0], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 1], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 2], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 3], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 4],
            m_strAAOProcInfo.LSC_Gain_Grid_R[i + 5], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 6], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 7], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 8], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 9],
            m_strAAOProcInfo.LSC_Gain_Grid_R[i + 10], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 11], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 12], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 13], m_strAAOProcInfo.LSC_Gain_Grid_R[i + 14]);
    }
    for (int i = 0; i < AE_V4P0_BLOCK_NO; i++)
    {
        AE_LOG_IF(bAAOLogEn, "[%s():LSC_Gain_Grid_G - %d] 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x, 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x, 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x\n", __FUNCTION__, i,
            m_strAAOProcInfo.LSC_Gain_Grid_G[i + 0], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 1], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 2], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 3], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 4],
            m_strAAOProcInfo.LSC_Gain_Grid_G[i + 5], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 6], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 7], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 8], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 9],
            m_strAAOProcInfo.LSC_Gain_Grid_G[i + 10], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 11], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 12], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 13], m_strAAOProcInfo.LSC_Gain_Grid_G[i + 14]);
    }
    for (int i = 0; i < AE_V4P0_BLOCK_NO; i++)
    {
        AE_LOG_IF(bAAOLogEn, "[%s():LSC_Gain_Grid_B - %d] 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x, 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x, 0x%08x/0x%08x/0x%08x/0x%08x/0x%08x\n", __FUNCTION__, i,
            m_strAAOProcInfo.LSC_Gain_Grid_B[i + 0], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 1], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 2], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 3], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 4],
            m_strAAOProcInfo.LSC_Gain_Grid_B[i + 5], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 6], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 7], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 8], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 9],
            m_strAAOProcInfo.LSC_Gain_Grid_B[i + 10], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 11], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 12], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 13], m_strAAOProcInfo.LSC_Gain_Grid_B[i + 14]);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setMVHDR3ExpoProcInfo(MVOID *pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100)
{
    if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
    {
        m_pIAeFlowCPU->setMVHDR3ExpoProcInfo(pMVHDR3ExpoStatBuf, u4MVHDRRatio_x100);
        bUpdateSensorAWBGain = 1;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapFlare(MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    //AE_MODE_CFG_T a_rCaptureInfo; /* fix build warning */
    MVOID * pAEStatisticBuf;
    memset(&rAEInput, 0, sizeof(strAEInput));
    if ((m_bIsAutoFlare == FALSE) || (m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET))
    {
        AE_LOG("[%s()] i4SensorDev:%d Using predict flare:m_bIsAutoFlare:%d m_eAETargetMode:%d\n", __FUNCTION__, m_eSensorDev, m_bIsAutoFlare, m_eAETargetMode);
        return S_AE_OK;
    }

    if ((m_pIAeAlgo != NULL) && (pAEStatBuf != NULL))
    {
        pAEStatisticBuf = pAEStatBuf;
        m_pIAeAlgo->setAESatisticBufferAddr_v4p0(m_pPSOStatBuf, pAEStatisticBuf, &m_strAAOProcInfo);
        m_pIAeAlgo->CalculateCaptureFlare(pAEStatisticBuf, bIsStrobe);

        rAEInput.eAeState = AE_STATE_POST_CAPTURE;
        rAEInput.pAESatisticBuffer = NULL;
        rAEInput.rHdrGyroInfo.i4Acce[0] = m_i4AcceInfo[0];
        rAEInput.rHdrGyroInfo.i4Acce[1] = m_i4AcceInfo[1];
        rAEInput.rHdrGyroInfo.i4Acce[2] = m_i4AcceInfo[2];
        rAEInput.rHdrGyroInfo.i4Gyro[0] = m_i4GyroInfo[0];
        rAEInput.rHdrGyroInfo.i4Gyro[1] = m_i4GyroInfo[1];
        rAEInput.rHdrGyroInfo.i4Gyro[2] = m_i4GyroInfo[2];

        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        mCaptureMode.i2FlareOffset = rAEOutput.i2FlareOffset;
        mCaptureMode.i2FlareGain = rAEOutput.i2FlareGain;

        // AE Auto Test
        m_pIAeTest->resetAvgY(IAeTest::AE_AT_TYPE_Pre2Cap, m_pIAeAlgo->getBrightnessAverageValue());

    }

    if (m_bStrobeOn == TRUE)
    {
        if (m_rAEInitInput.rAENVRAM->rCCTConfig.bEnableStrobeThres == MFALSE)
        {
            mCaptureMode.i2FlareOffset = m_rAEInitInput.rAENVRAM->rCCTConfig.u4StrobeFlareOffset;
            mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - m_rAEInitInput.rAENVRAM->rCCTConfig.u4StrobeFlareOffset));
        }
    }
    else if (m_eSensorMode == ESensorMode_Video)
    {
        if (m_rAEInitInput.rAENVRAM->rCCTConfig.bEnableVideoThres == MFALSE)
        {
            mCaptureMode.i2FlareOffset = m_rAEInitInput.rAENVRAM->rCCTConfig.u4VideoFlareOffset;
            mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - m_rAEInitInput.rAENVRAM->rCCTConfig.u4VideoFlareOffset));
        }
    }
    else
    {
        if (m_rAEInitInput.rAENVRAM->rCCTConfig.bEnableCaptureThres == MFALSE)
        {
            mCaptureMode.i2FlareOffset = m_rAEInitInput.rAENVRAM->rCCTConfig.u4CaptureFlareOffset;
            mCaptureMode.i2FlareGain = (MINT16)(FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - m_rAEInitInput.rAENVRAM->rCCTConfig.u4CaptureFlareOffset));
        }
    }
    AE_LOG("[%s()] i4SensorDev:%d i2FlareOffset:%d i2FlareGain:%d Sensor Mode:%d Strobe:%d Enable:%d %d %d avgY:%d\n", __FUNCTION__, m_eSensorDev,
        mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain, m_eSensorMode, m_bStrobeOn, m_rAEInitInput.rAENVRAM->rCCTConfig.bEnableCaptureThres,
        m_rAEInitInput.rAENVRAM->rCCTConfig.bEnableVideoThres, m_rAEInitInput.rAENVRAM->rCCTConfig.bEnableStrobeThres, m_pIAeAlgo->getBrightnessAverageValue());

    UpdateSensorISPParams(AE_POST_CAPTURE_STATE);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getCCUresultCBActive(MVOID * pfCB)
{
    AE_LOG("[%s()] \n", __FUNCTION__);
    m_pCCUresultCB = (NS3Av3::Hal3AFlowCtrl*) pfCB;
    return m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getLVvalue(MBOOL isStrobeOn)
{
    if (isStrobeOn == MTRUE)
    {
        return (m_BVvalue + 50);
    }
    else
    {
        return (m_BVvalueWOStrobe + 50);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAOECompLVvalue(MBOOL isStrobeOn)
{
    if (isStrobeOn == MTRUE)
    {
        return (m_BVvalue + 50);
    }
    else
    {
        return (m_AOECompBVvalue + 50);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getCaptureLVvalue()
{
    MINT32 i4LVValue_10x = 0;

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->CalculateCaptureLV(&i4LVValue_10x);
    }
    else
    {
        AE_LOG("[%s] The AE algo class is NULL, i4SensorDev:%d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return (i4LVValue_10x);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo)
{
    MINT32 u4AEPlineInfoSize, u4AEPlineMappingSize, u4AFTableSize, u4GainTableSize;
    AE_EASYTUNING_PLINE_INFO_T rEasytuningTable;
    MINT32 i4LVValue_10x = 0;

    if (m_pIAeAlgo != NULL)
    {
        rAEPlineDebugInfo.u4Size = sizeof(AE_PLINE_DEBUG_INFO_T);
        rAEPlineDebugInfo.u4IsTrue = MTRUE;
        m_pIAeAlgo->CalculateCaptureLV(&i4LVValue_10x);
        // temp disable ccu getdebuginfo 1207
        if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
        {
            AE_LOG_IF(m_3ALogEnable, "[%s()] AeMgr CCU getDebugInfo\n", __FUNCTION__);
            m_pIAeFlowCCU->getdebuginfo(rAEDebugInfo);
        }
        else if (m_bForceUpdateAEInfoByCCU)
        {
            AE_LOG("[%s()] Force Update AEInfo By CCU, AeMgr CCU getDebugInfo\n", __FUNCTION__);
            m_pIAeFlowCCU->getdebuginfo(rAEDebugInfo);
        }
        else
        {
            m_pIAeAlgo->getDebugInfo(rAEDebugInfo);
            AE_LOG_IF(m_3ALogEnable, "[%s()] AeMgr CPU getDebugInfo\n", __FUNCTION__);
        }

        if (m_bNeedApplyBackupAEInfo)
        {
            AE_LOG_IF(m_3ALogEnable, "[%s()] getDebugInfo Fill exif by Backup \n", __FUNCTION__);
            memcpy(&rAEDebugInfo, &rAEDebugInfoBack, sizeof(AE_DEBUG_INFO_T));
        }
        else if (m_u4HDRcountforrevert == 0)
        {
            memcpy(&rAEDebugInfoBack, &rAEDebugInfo, sizeof(AE_DEBUG_INFO_T));
        }

        AE_LOG_IF(m_3ALogEnable, "[%s()] FDY: %d \n", __FUNCTION__, rAEDebugInfo.Tag[AE_TAG_FACE_20_FACE_Y].u4FieldValue);
        //m_pIAeFlowCCU->getdebuginfo(rAEDebugInfo);
        u4AEPlineInfoSize = sizeof(m_pAEPlineTable->AEPlineInfo);
        u4AEPlineMappingSize = sizeof(strAESceneMapping);
        u4AFTableSize = sizeof(m_rAEInitInput.rAENVRAM->rAeParam.strAFPLine);
        u4GainTableSize = sizeof(m_pAEPlineTable->AEGainList);
        AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d TotalSize:%d AEPlineInfoSize:%d Mapping:%d AFTable:%d u4GainTableSize:%d\n", __FUNCTION__, m_eSensorDev, rAEPlineDebugInfo.u4Size, u4AEPlineInfoSize, u4AEPlineMappingSize, u4AFTableSize, u4GainTableSize);
        memcpy(&(rEasytuningTable.AEPLineInfomation), &(m_pAEPlineTable->AEPlineInfo), u4AEPlineInfoSize);
        memcpy(&(rEasytuningTable.AEPLineMapping), &(m_pAEPlineTable->sAEScenePLineMapping), u4AEPlineMappingSize);
        memcpy(&(rEasytuningTable.normalAFPlineInfo), &(m_rAEInitInput.rAENVRAM->rAeParam.strAFPLine), u4AFTableSize);
        memcpy(&(rEasytuningTable.ZSDAFPlineInfo), &(m_rAEInitInput.rAENVRAM->rAeParam.strAFZSDPLine), u4AFTableSize);
        memcpy(&(rEasytuningTable.AEPLineGainList), &(m_pAEPlineTable->AEGainList), u4GainTableSize);
        if (sizeof(rAEPlineDebugInfo.PlineInfo) >= sizeof(rEasytuningTable))
        {
            memcpy(&rAEPlineDebugInfo.PlineInfo[0], &rEasytuningTable, sizeof(rEasytuningTable));
        }
        else
        {
            memcpy(&rAEPlineDebugInfo.PlineInfo[0], &rEasytuningTable, sizeof(rAEPlineDebugInfo.PlineInfo));
        }
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    if (strlen(mEngFileName) != 0)
    {
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->SaveAeCoreInfo(mEngFileName);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAENvramData(AE_NVRAM_T &rAENVRAM)
{
    AE_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
    if (m_p3ANVRAM == NULL)
    {
        CAM_LOGE("[%s()] Nvram 3A pointer NULL\n", __FUNCTION__);
    }
    else
    {
        rAENVRAM = m_p3ANVRAM->AE[m_u4AENVRAMIdx];
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo, MBOOL binit)
{
    MBOOL bIsHDRFlow = ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET));
    sAEOutputInfo->u4CWValue = (MUINT16)sAEInfo->u4CWValue; // for N3D sync
    sAEOutputInfo->u2FrameRate = sAEInfo->u2FrameRate;
    sAEOutputInfo->u4RealISO = sAEInfo->u4ISO;
    sAEOutputInfo->i2FlareOffset = sAEInfo->i2FlareOffset;
    sAEOutputInfo->i2FlareGain = sAEInfo->i2FlareGain;

    if (m_i4HdrOnOff != toLiteral(HDRDetectionResult::NONE))
    {
        m_i4HdrOnOff = (sAEInfo->HdrEvSetting.i4HdrOnOff == 1) ?
        toLiteral(HDRDetectionResult::HDR) :
        toLiteral(HDRDetectionResult::NORMAL);
    }

    if (bIsHDRFlow && (m_eCamMode == eAppMode_EngMode || m_bAEReadyCapture == MFALSE || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)))
    {
        m_strIVHDROutputSetting.u4LEExpTimeInUS = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEExpo;
        m_strIVHDROutputSetting.u4LESensorGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEAfeGain;
        m_strIVHDROutputSetting.u4LEISPGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4LEIspGain;
        m_strIVHDROutputSetting.u4SEExpTimeInUS = sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEExpo;
        m_strIVHDROutputSetting.u4SESensorGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEAfeGain;
        m_strIVHDROutputSetting.u4SEISPGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4SEIspGain;
        m_strIVHDROutputSetting.u4MEExpTimeInUS = sAEInfo->HdrEvSetting.rHdrEVInfo.i4MEExpo;
        m_strIVHDROutputSetting.u4MESensorGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4MEAfeGain;
        m_strIVHDROutputSetting.u4MEISPGain = sAEInfo->HdrEvSetting.rHdrEVInfo.i4MEIspGain;
        m_strIVHDROutputSetting.u4LE_SERatio_x100 = sAEInfo->HdrEvSetting.rHdrEVInfo.i4HdrRatio;

        if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
        {
            m_i4HdrSeg = sAEInfo->HdrEvSetting.i4RMGk;
            m_i4HdrTurn = sAEInfo->HdrEvSetting.i4RMGt;
            m_u4EISExpRatio = 100 * m_strIVHDROutputSetting.u4LEExpTimeInUS / m_strIVHDROutputSetting.u4SEExpTimeInUS;
        }
        else
        {
            m_strIVHDROutputSetting.bEnableWorkaround = MFALSE;
            m_u4AFSGG1Gain = (m_u4AFSGG1Gain == MTRUE) ? 16 * m_strIVHDROutputSetting.u4SEISPGain / 1024 : 16;
            m_rSensorVCInfo.u2VCShutterRatio = m_strIVHDROutputSetting.u4LE_SERatio_x100 * 101 / 10000;
        }

        if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
        {
            m_u4veLTCRate = sAEInfo->HdrEvSetting.u4veLTCRate;
            m_u4veLimitGain = sAEInfo->HdrEvSetting.u4veLimitGain;
        }

        AE_LOG("[%s()] LE(%d, %d, %d) SE(%d, %d, %d) ME(%d, %d, %d) R(%d) RMGtk(%d, %d) HdrOnOff(%d) veLTCRate(%d) veLimitGain(%d)\n", __FUNCTION__, m_strIVHDROutputSetting.u4LEExpTimeInUS, m_strIVHDROutputSetting.u4LESensorGain, m_strIVHDROutputSetting.u4LEISPGain,
            m_strIVHDROutputSetting.u4SEExpTimeInUS, m_strIVHDROutputSetting.u4SESensorGain, m_strIVHDROutputSetting.u4SEISPGain,
            m_strIVHDROutputSetting.u4MEExpTimeInUS, m_strIVHDROutputSetting.u4MESensorGain, m_strIVHDROutputSetting.u4MEISPGain,
            m_i4LESE_Ratio, m_i4HdrSeg, m_i4HdrTurn, m_i4HdrOnOff, m_u4veLTCRate, m_u4veLimitGain);

        sAEOutputInfo->u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
        sAEOutputInfo->u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
        sAEOutputInfo->u4IspGain = m_strIVHDROutputSetting.u4LEISPGain;
    }
    else
    {
        sAEOutputInfo->u4Eposuretime = sAEInfo->EvSetting.u4Eposuretime;
        sAEOutputInfo->u4AfeGain = sAEInfo->EvSetting.u4AfeGain;
        sAEOutputInfo->u4IspGain = sAEInfo->EvSetting.u4IspGain;
    }

    // for 3DNR, no used for capture mode
    modifyAEInfoby3DNR(sAEOutputInfo->u4Eposuretime, sAEOutputInfo->u4AfeGain, sAEOutputInfo->u4IspGain);

    if (sAEOutputInfo->u4IspGain < 1024)
    {
        AE_LOG("[%s()] i4SensorDev:%d ISP gain too small:%d\n", __FUNCTION__, m_eSensorDev, sAEOutputInfo->u4IspGain);
        sAEOutputInfo->u4IspGain = 1024;
    }

    m_i4LESE_Ratio = m_strIVHDROutputSetting.u4LE_SERatio_x100;
    m_u4AECondition = sAEInfo->u4AECondition;
    m_i4AEidxCurrent = sAEInfo->i4AEidxCurrent;
    m_i4AEidxNext = sAEInfo->i4AEidxNext;
    m_i4AEidxNextF = sAEInfo->i4AEidxNextF;

    if (!m_bCCUAEFlag || (!m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)))
    {
        if (!binit)
        {
            //initAE don't update BV
            m_BVvalue = sAEInfo->Bv;
            m_AOECompBVvalue = sAEInfo->AoeCompBv;
        }
        if (m_bStrobeOn == MFALSE)
        {
            m_BVvalueWOStrobe = sAEInfo->Bv;
        }
        m_i4EVvalue = sAEInfo->i4EV;
        m_i4DeltaBV = sAEInfo->i4DeltaBV;
        m_u4Index = sAEInfo->i4AEidxNext;
        m_u4IndexF = sAEInfo->i4AEidxNextF;
        AE_LOG_IF(m_3ALogEnable, "[%s()] m_u4Index:%d m_u4IndexF:%d BV/BVWOStrobe:%d/%d\n", __FUNCTION__, m_u4Index, m_u4IndexF, m_BVvalue, m_BVvalueWOStrobe);
    }
    m_i2AEFaceDiffIndex = sAEInfo->i2FaceDiffIndex;
    m_i4GammaIdx = sAEInfo->i4gammaidx;
    m_u4SWHDR_SE = ABS(sAEInfo->i4cHdrSEDeltaEV); //for HDR SE , -x EV , compare with converge AE
    return S_3A_OK;
}

MRESULT AeMgr::modifyAEInfoby3DNR(MUINT32& u4Exp, MUINT32& u4Afe, MUINT32& u4Isp)
{
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain = 1024;
    MUINT32 u4PreviewBaseISO = 100;
    MUINT32 u4ISOValue;
    AE_EXP_GAIN_MODIFY_T rSensorInputData, rSensorOutputData;

    //Initialize rSensorOutputData to fix coveriy check
    memset(&rSensorOutputData, 0, sizeof(AE_EXP_GAIN_MODIFY_T));

    if ((m_rAEPLineLimitation.bEnable == MTRUE) && (m_bAEReadyCapture == MFALSE))
    {
        if (m_rAEPLineLimitation.bEquivalent == MTRUE)
        {
            if (m_rAEPLineLimitation.u4IncreaseISO_x100 > 100)
            {
                rSensorInputData.u4SensorExpTime = 100 * m_u4MaxShutter / m_rAEPLineLimitation.u4IncreaseISO_x100;
                rSensorInputData.u4SensorGain = u4Exp * u4Afe / rSensorInputData.u4SensorExpTime;
                rSensorInputData.u4IspGain = u4Isp;
                if (m_pIAeAlgo != NULL)
                {
                    m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData); // send to 3A to calculate the exposure time and gain
                }
                else
                {
                    AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                }
                AE_LOG("[%s()] i4SensorDev:%d NewExpSetting:%d/%d/%d, Old:%d/%d/%d\n", __FUNCTION__, m_eSensorDev,
                    rSensorOutputData.u4SensorExpTime, rSensorOutputData.u4SensorGain, rSensorOutputData.u4IspGain,
                    u4Exp, u4Afe, u4Isp);
                u4Exp = rSensorOutputData.u4SensorExpTime;
                u4Afe = rSensorOutputData.u4SensorGain;
                u4Isp = rSensorOutputData.u4IspGain;
            }
            else if (m_rAEPLineLimitation.u4IncreaseISO_x100 < 100)
            {
                if (m_p3ANVRAM != NULL)
                {
                    u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
                }
                else
                {
                    AE_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
                }
                u4FinalGain = (u4Afe * u4Isp) >> 10;
                u4ISOValue = u4PreviewBaseISO * u4FinalGain / u4PreviewBaseGain;
                AE_LOG("[%s()] u4ISOValue:%d m_u4MaxISO:%d Ratio:%d\n", __FUNCTION__, u4ISOValue, m_u4MaxISO, m_rAEPLineLimitation.u4IncreaseISO_x100);

                if (u4ISOValue > (m_u4MaxISO * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100))
                {
                    rSensorInputData.u4SensorGain = u4Afe * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100;
                    rSensorInputData.u4SensorExpTime = u4Exp * u4Afe / rSensorInputData.u4SensorGain;
                    rSensorInputData.u4IspGain = u4Isp;
                    if (m_pIAeAlgo != NULL)
                    {
                        m_pIAeAlgo->switchSensorExposureGain(MFALSE, rSensorInputData, rSensorOutputData); // send to 3A to calculate the exposure time and gain
                    }
                    else
                    {
                        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
                    }
                    AE_LOG("[%s()] i4SensorDev:%d NewExpSetting:%d/%d/%d, Old:%d/%d/%d, Ratio:%d\n", __FUNCTION__, m_eSensorDev,
                        rSensorOutputData.u4SensorExpTime, rSensorOutputData.u4SensorGain, rSensorOutputData.u4IspGain,
                        u4Exp, u4Afe, u4Isp, m_rAEPLineLimitation.u4IncreaseISO_x100);
                    u4Exp = rSensorOutputData.u4SensorExpTime;
                    u4Afe = rSensorOutputData.u4SensorGain;
                    u4Isp = rSensorOutputData.u4IspGain;
                }
            }
        }
        else
        {
            AE_LOG("[%s()] i4SensorDev:%d No rquivalent, Please use AE Pline table:%d ISO:%d Shutter:%d\n", __FUNCTION__, m_eSensorDev, m_rAEPLineLimitation.bEnable, m_rAEPLineLimitation.u4IncreaseISO_x100, m_rAEPLineLimitation.u4IncreaseShutter_x100);
        }
    }

    return S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorResolution()
{
    MRESULT err = S_AE_OK;

    if ((m_eSensorDev == ESensorDev_Main) || (m_eSensorDev == ESensorDev_Sub))
    {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_eSensorDev, &m_rSensorResolution[0]);
    }
    else
    {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_eSensorDev, &m_rSensorResolution[1]);
    }

    if (FAILED(err))
    {
        CAM_LOGE("[%s] AAASensorMgr::getInstance().getSensorWidthHeight fail\n", __FUNCTION__);
        return err;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <nvbuf_util.h>
MRESULT AeMgr::getNvramData(MINT32 i4SensorDev, MBOOL isForce)
{
    AE_LOG("[%s()] i4SensorDev:%d\n", __FUNCTION__, i4SensorDev);

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SensorDev, (void *& )m_p3ANVRAM, isForce);
    if (err != 0)
        CAM_LOGE("[%s()] Nvram 3A pointer NULL err=%d\n", __FUNCTION__, err);

    strAEPLineTable sAEPlineTable;
    if (isForce == MTRUE) // fix me plz
    sAEPlineTable = ((AE_PLINETABLE_T*)m_pAEPlineTable)->AEPlineTable;

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_AE_PLINETABLE, i4SensorDev, (void *& )m_pAEPlineTable, isForce);
    if (err != 0)
        CAM_LOGE("[%s()] Nvram AE Pline table pointer NULL err=%d\n", __FUNCTION__, err);

    if (isForce == MTRUE && m_pAEPlineTable != NULL) // fix me plz
    {
        for (int i = 0; i < MAX_PLINE_TABLE; i++)
        {
            ((AE_PLINETABLE_T*)m_pAEPlineTable)->AEPlineTable.sPlineTable[i].pCurrentTable = sAEPlineTable.sPlineTable[i].pCurrentTable;
        }
    }
    NVRAM_CAMERA_ISP_PARAM_STRUCT * pISPNvramTemp;
    MINT32 i4ISOinterval = 0;
    // Get OBC tuning for CCU setting ISP gain
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, i4SensorDev, (void *& )pISPNvramTemp);
    NVRAM_CAMERA_ISP_PARAM_STRUCT * const pISPNvram = pISPNvramTemp;
    memcpy(&m_rISPNvramISOInterval, &(pISPNvram->ISPRegs.Iso_Env), sizeof(ISP_NVRAM_ISO_INTERVAL_T));
    memcpy(&m_rISPNvramOBC_Table[0], &(pISPNvram->ISPRegs.OBC[0]), 4 * sizeof(CCU_ISP_NVRAM_OBC_T));
    i4ISOinterval = m_rISPNvramISOInterval.u2Length;
    AE_LOG("[%s()] i4SensorDev:%d ISPISOInterval's length/IDX[2]/IDX[End]=%d/%d/%d\n", __FUNCTION__, i4SensorDev,
        i4ISOinterval, m_rISPNvramISOInterval.IDX_Partition[2], m_rISPNvramISOInterval.IDX_Partition[i4ISOinterval -1]);
    AE_LOG("[%s()] m_i4SensorDev:%d i4OBCtableidx:%d [0][1][2][3](offset/gain) = [%d/%d][%d/%d][%d/%d][%d/%d] \n",
        __FUNCTION__, m_eSensorDev, m_i4OBCTableidx,
        m_rISPNvramOBC_Table[0].offst0.val, m_rISPNvramOBC_Table[0].gain0.val,
        m_rISPNvramOBC_Table[1].offst0.val, m_rISPNvramOBC_Table[1].gain0.val,
        m_rISPNvramOBC_Table[2].offst0.val, m_rISPNvramOBC_Table[2].gain0.val,
        m_rISPNvramOBC_Table[3].offst0.val, m_rISPNvramOBC_Table[3].gain0.val);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::AEInit(Param_T &rParam)
{
    (void)rParam;
    strAEOutput rAEOutput;
    MINT32 i, j;
    unsigned long long u8initAEtimer;
    MBOOL brealAEinit = MFALSE;
    MBOOL bEnablePreIndex = MFALSE;

    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    u8initAEtimer = (unsigned long long)time.tv_sec * 1000000000 + (unsigned long long)time.tv_nsec;
    brealAEinit = (u8initAEtimer > AEtimer + RealInitThrs) ? MTRUE : MFALSE;
    NumofUsers++;

    AE_LOG("[%s()] CCU enable:%d\n", __FUNCTION__, m_bCCUAEFlag);

    // if (NumofUsers != 1)
    // m_bCCUAEFlag = MFALSE;
    AE_LOG("[%s()] i4SensorDev:%d AEtimer(uninit/init):%lld/%lld/%lld bRealAEInit:%d NumOfUsers:%d\n", __FUNCTION__, m_eSensorDev, AEtimer, u8initAEtimer, u8initAEtimer -AEtimer, brealAEinit, NumofUsers);

    // Query TG info
    IHalSensorList * const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor * pIHalSensor = pIHalSensorList->createSensor("ae_mgr", m_i4SensorIdx);
    SensorDynamicInfo rSensorDynamicInfo;
    pIHalSensor->querySensorDynamicInfo(m_eSensorDev, &rSensorDynamicInfo);

    m_bEnableAE = MTRUE;
    // Judge which camscenario mode to be used
    setCamScenarioMode(m_eCamScenarioMode, MTRUE);
    setNVRAMIndex(m_u4AENVRAMIdx, MTRUE);

    if (pIHalSensor)
    {
        pIHalSensor->destroyInstance("ae_mgr");
    }

    AE_LOG("[%s()] TG = %d, pixel mode = %d\n", __FUNCTION__, rSensorDynamicInfo.TgInfo, rSensorDynamicInfo.pixelMode);

    switch (rSensorDynamicInfo.TgInfo)
    {
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
    }
    // --- Create SensorProvider Object ---
    AAA_TRACE_D("AE SensorProvider");
    if (mpSensorProvider == NULL)
    {
        mpSensorProvider = SensorProvider::createInstance(LOG_TAG);

        if (mpSensorProvider != nullptr)
        {
            MUINT32 interval = 30;

            interval = property_get_int32("vendor.debug.ae_mgr.gyrointerval", SENSOR_GYRO_POLLING_MS);
            if (mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval))
            {
                m_bGryoVd = MTRUE;
                CAM_LOGD("Dev(%d):enable SensorProvider success for Gyro ", m_eSensorDev);
            }
            else
            {
                m_bGryoVd = MFALSE;
                CAM_LOGE("[%s()] Enable SensorProvider fail for Gyro", __FUNCTION__);
            }
            interval = property_get_int32("vendor.debug.ae_mgr.gyrointerval", SENSOR_ACCE_POLLING_MS);
            if (mpSensorProvider->enableSensor(SENSOR_TYPE_ACCELERATION, interval))
            {
                m_bAcceVd = MTRUE;
                CAM_LOGD("Dev(%d):enable SensorProvider success for Acce ", m_eSensorDev);
            }
            else
            {
                m_bAcceVd = MFALSE;
                CAM_LOGE("[%s()] Enable SensorProvider fail for Acce", __FUNCTION__);
            }
        }
    }
    AAA_TRACE_END_D;

    if (bApplyAEPlineTableByCCT == MTRUE)
    {
        // update by CCT
        CAM_LOGE("[%s()] AE Pline table update by CCT\n", __FUNCTION__);
    }
    else if (m_pAEPlineTable != NULL)
    {
        m_rAEInitInput.rAEPlineTable = m_pAEPlineTable;

        // sensor table mapping
        for (j = 0; j < MAX_PLINE_MAP_TABLE; j++)
        {
            for (i = 0; i < LIB3A_SENSOR_MODE_MAX; i++)
            {
                m_rAEInitInput.rAEPlineMapTable[j].eAEScene = m_pAEPlineTable->sAEScenePLineMapping.sAESceneMapping[j].eAEScene;
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[i] = m_pAEPlineTable->sAEScenePLineMapping.sAESceneMapping[j].ePLineID[i];
            }
        }
    }
    else
    {
        CAM_LOGE("[%s()] Nvram AE Pline table pointer is NULL\n", __FUNCTION__);
    }

    // Clear FD Area
    memset(&m_eAEFDArea, 0, sizeof(AEMeteringArea_T));
    m_bFaceAEAreaChage = MFALSE;
    m_bTouchAEAreaChage = MFALSE;

    // ezoom info default
    m_eZoomWinInfo.u4XOffset = 0;
    m_eZoomWinInfo.u4YOffset = 0;
    m_eZoomWinInfo.u4XWidth = 2000;
    m_eZoomWinInfo.u4YHeight = 1000;
    m_rAEInitInput.rEZoomWin = m_eZoomWinInfo;
    m_rAEInitInput.eAEMeteringMode = m_eAEMeterMode;
    m_rAEInitInput.eAEScene = m_eAEScene;
    m_rAEInitInput.eAECamMode = m_eAECamMode;
    m_rAEInitInput.eAEFlickerMode = m_eAEFlickerMode;
    m_rAEInitInput.eAEAutoFlickerMode = m_eAEAutoFlickerMode;
    m_rAEInitInput.eAEEVcomp = m_eAEEVcomp;
    m_rAEInitInput.u4AEISOSpeed = m_u4AEISOSpeed;
    m_rAEInitInput.i4AEMaxFps = m_i4AEMaxFps;
    m_rAEInitInput.i4AEMinFps = m_i4AEMinFps;
    m_rAEInitInput.i4SensorMode = m_eSensorMode;
    m_rAEInitInput.eAETargetMode = m_eAETargetMode;
    AE_LOG("[%s()] i4SensorDev:%d SensorIdx:%d AE meter:%d CamMode:%d AEScene:%d Flicker:%d %d EVcomp:%d ISO:%d %d MinFps:%d MaxFps:%d Limiter:%d\n", __FUNCTION__,
        m_eSensorDev, m_i4SensorIdx, m_eAEMeterMode,
        m_eAECamMode, m_eAEScene, m_eAEFlickerMode, m_eAEAutoFlickerMode, m_eAEEVcomp, m_u4AEISOSpeed, m_bRealISOSpeed,
        m_i4AEMinFps, m_i4AEMaxFps, m_bAElimitor);

    #if USE_OPEN_SOURCE_AE
    m_pIAeAlgo = NS3A::IAeAlgo::createInstance <NS3A::EAAAOpt_OpenSource> (m_i4SensorIdx);
    #else
    m_pIAeAlgo = NS3A::IAeAlgo::createInstance <NS3A::EAAAOpt_MTK> (m_i4SensorIdx);
    if (m_bCCUAEFlag)
    {
        m_pIAeFlowCCU->init();
        m_bIsCCUAEInit = MTRUE;
    }
    #endif

    if (!m_pIAeAlgo)
    {
        CAM_LOGE("[%s()] AeAlgo::createInstance() fail \n", __FUNCTION__);
        return E_AE_ALGO_INIT_ERR;
    }
    if (m_bAETrueLaunch)
    {
        bEnablePreIndex = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex;
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = MFALSE;
    }
    else if (!brealAEinit)
    {
        bEnablePreIndex = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex;
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = MTRUE;
    }
    m_pIAeAlgo->lockAE(m_bAELock);
    m_pIAeAlgo->setAERealISOSpeed(m_bRealISOSpeed);
    m_pIAeAlgo->setAEVideoDynamicEnable(m_bVideoDynamic);
    // m_pIAeAlgo->setAELowLightTargetValue(m_rAEInitInput.rAENVRAM->rCCTConfig.u4AETarget, m_rAEInitInput.rAENVRAM->rCCTConfig.u4InDoorEV - 50, m_rAEInitInput.rAENVRAM->rCCTConfig.u4InDoorEV); // set AE lowlight target 47 and low light threshold LV5
    m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    AE_LOG("[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    setAEScene(m_eAEScene);
    setIsoSpeed(m_u4AEISOSpeed);
    setAEMinMaxFrameRatePlineIdx(m_i4AEMinFps , m_i4AEMaxFps);
    //switchSensorModeMaxBVSensitivityDiff(MINT32 i4newSensorMode, MINT32 i4oldSensorMode, MINT32 & i4SenstivityDeltaIdx, MINT32 & i4BVDeltaIdx)
    MUINT32 u4FinerEVIdxBase = 1;
    m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
    if (u4FinerEVIdxBase != m_u4FinerEVIdxBase)
    {
        m_u4FinerEVIdxBase = u4FinerEVIdxBase;
        m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
        m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
        m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
        searchPreviewIndexLimit();
    }
    AE_LOG("[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    m_pIAeAlgo->setSensorModeMaxBVdiff(m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
    m_pIAeAlgo->initAE(&m_rAEInitInput, &rAEOutput, &m_rAEStatCfg);
    copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput, MTRUE);
    m_pIAeAlgo->setAElimitorEnable(m_bAElimitor); // update limiter
    m_pIAeAlgo->setAEObjectTracking(MFALSE);
    m_pIAeAlgo->setAEFDArea(&m_eAEFDArea);
    m_pIAeAlgo->getAEMaxISO(m_u4MaxShutter, m_u4MaxISO);
    //m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight);

    m_rAEStatCfg.u4UseDefaultHight = MTRUE;

    m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);

    if (!brealAEinit || m_bAETrueLaunch)
    {
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = bEnablePreIndex;
        m_bAETrueLaunch = MFALSE;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::prepareCapParams(MBOOL bRemosaicEn,MBOOL IsEVChange)
{
    AE_LOG("[%s()] i4SensorDev:%d m_eShotMode:%d m_bNeedApplyBackupAEInfo:%d\n", __FUNCTION__, m_eSensorDev, m_eShotMode, m_bNeedApplyBackupAEInfo);

    if(m_bAPAELock && (!IsEVChange))
    {
        mCaptureMode.u4ExposureMode = eAE_EXPO_TIME;
        mCaptureMode.u4Eposuretime = mPreviewMode.u4Eposuretime;
        mCaptureMode.u4AfeGain = mPreviewMode.u4AfeGain;
        mCaptureMode.u4IspGain = mPreviewMode.u4IspGain;
        mCaptureMode.u4RealISO = mPreviewMode.u4RealISO;
        mCaptureMode.i2FlareGain = mPreviewMode.i2FlareGain;
        mCaptureMode.i2FlareOffset = mPreviewMode.i2FlareOffset;
    }else{
        strAEInput rAEInput;
        strAEOutput rAEOutput;
        memset(&rAEInput, 0, sizeof(strAEInput));
        memset(&rAEOutput, 0, sizeof(strAEOutput));
        m_bNeedApplyBackupAEInfo = bRemosaicEn;

        rAEInput.eAeState = AE_STATE_CAPTURE;
        rAEInput.rHdrGyroInfo.i4Acce[0] = m_i4AcceInfo[0];
        rAEInput.rHdrGyroInfo.i4Acce[1] = m_i4AcceInfo[1];
        rAEInput.rHdrGyroInfo.i4Acce[2] = m_i4AcceInfo[2];
        rAEInput.rHdrGyroInfo.i4Gyro[0] = m_i4GyroInfo[0];
        rAEInput.rHdrGyroInfo.i4Gyro[1] = m_i4GyroInfo[1];
        rAEInput.rHdrGyroInfo.i4Gyro[2] = m_i4GyroInfo[2];
        if (m_pIAeAlgo != NULL)
        {
            MUINT32 Cap2PreRatioBackUp = 1024;
            if(!bRemosaicEn)
            {
                Cap2PreRatioBackUp = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio;
                m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio = 1024;
            }
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
            if(!bRemosaicEn) m_rAEInitInput.rAENVRAM->rDevicesInfo.u4Cap2PreRatio = Cap2PreRatioBackUp;
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorIdx:%d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }

        MUINT32 exp_ratio = (1024 / m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Cap2PreRatio);
        AE_LOG("[%s()] handleAE exp_ratio:%d Exp:%d Afe:%d Isp:%d ISO:%d",
            __FUNCTION__, exp_ratio, rAEOutput.EvSetting.u4Eposuretime, rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain, rAEOutput.u4ISO);

        if ((bRemosaicEn) &&
            (exp_ratio > 1) &&
            (m_u4AEISOSpeed == LIB3A_AE_ISO_SPEED_AUTO) &&
            (m_eAEScene == LIB3A_AE_SCENE_AUTO || m_eAEScene == LIB3A_AE_SCENE_FIREWORKS))
        {
            if ((m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) || (!bRemosaicEn))
            {
                // customize for 4 -cell mvhdr preview -> normal capture
                exp_ratio = 1;
                AE_LOG("[%s()] i4SensorDev:%d, 4 -cell mvhdr preview, exp_ratio:%d bRemosaicEn:%d\n", __FUNCTION__, m_eSensorDev, exp_ratio, bRemosaicEn);
            }
            AE_LOG("[%s()] FOUR_CELL_ADD_SHUTTER exp_ratio:%d Exp:%d Afe:%d Isp:%d ISO:%d", __FUNCTION__, exp_ratio, rAEOutput.EvSetting.u4Eposuretime, rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain, rAEOutput.u4ISO);
            ModifyCaptureParamByCustom(exp_ratio, &m_rAEOutput.rPreviewMode, &rAEOutput, (MUINT32)m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain);
        }

        m_bAEReadyCapture = MTRUE; // capture ready flag
        m_i4TimeOutCnt = MFALSE;
        m_bCShot = MFALSE;
        copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[0], &rAEOutput);
        copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[1], &rAEOutput);
        copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[2], &rAEOutput);
        mCaptureMode = m_rAEOutput.rCaptureMode[0];

        if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
        {
            m_strIVHDRCaptureSetting.u4LEExpTimeInUS = rAEOutput.HdrEvSetting.rHdrEVInfo.i4LEExpo;
            m_strIVHDRCaptureSetting.u4LESensorGain = rAEOutput.HdrEvSetting.rHdrEVInfo.i4LEAfeGain;
            m_strIVHDRCaptureSetting.u4LEISPGain = rAEOutput.HdrEvSetting.rHdrEVInfo.i4LEIspGain;
            m_strIVHDRCaptureSetting.u4SEExpTimeInUS = rAEOutput.HdrEvSetting.rHdrEVInfo.i4SEExpo;
            m_strIVHDRCaptureSetting.u4SESensorGain = rAEOutput.HdrEvSetting.rHdrEVInfo.i4SEAfeGain;
            m_strIVHDRCaptureSetting.u4SEISPGain = rAEOutput.HdrEvSetting.rHdrEVInfo.i4SEIspGain;
            m_strIVHDRCaptureSetting.u4LE_SERatio_x100 = rAEOutput.HdrEvSetting.rHdrEVInfo.i4HdrRatio;
            m_i4LESE_Ratio = m_strIVHDRCaptureSetting.u4LE_SERatio_x100;
            AE_LOG("[%s():ZVHDR] i4SensorDev:%d LE:%d/%d/%d SE:%d/%d/%d R:%d\n", __FUNCTION__, m_eSensorDev,
                rAEOutput.HdrEvSetting.rHdrEVInfo.i4LEExpo, rAEOutput.HdrEvSetting.rHdrEVInfo.i4LEAfeGain, rAEOutput.HdrEvSetting.rHdrEVInfo.i4LEIspGain,
                rAEOutput.HdrEvSetting.rHdrEVInfo.i4SEExpo, rAEOutput.HdrEvSetting.rHdrEVInfo.i4SEAfeGain, rAEOutput.HdrEvSetting.rHdrEVInfo.i4SEIspGain,
                rAEOutput.HdrEvSetting.rHdrEVInfo.i4HdrRatio);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAECapPlineTable(MINT32 *i4CapIndex, strAETable &a_AEPlineTable)
{
    strAETable strAEPlineTable;
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    memset(&rAEInput, 0, sizeof(strAEInput));
    AE_LOG("[%s()] i4SensorDev:%d m_eShotMode:%d\n", __FUNCTION__, m_eSensorDev, m_eShotMode);
    rAEInput.eAeState = AE_STATE_CAPTURE;
    rAEInput.rHdrGyroInfo.i4Acce[0] = m_i4AcceInfo[0];
    rAEInput.rHdrGyroInfo.i4Acce[1] = m_i4AcceInfo[1];
    rAEInput.rHdrGyroInfo.i4Acce[2] = m_i4AcceInfo[2];
    rAEInput.rHdrGyroInfo.i4Gyro[0] = m_i4GyroInfo[0];
    rAEInput.rHdrGyroInfo.i4Gyro[1] = m_i4GyroInfo[1];
    rAEInput.rHdrGyroInfo.i4Gyro[2] = m_i4GyroInfo[2];

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getCapPlineTable(i4CapIndex, strAEPlineTable);
        a_AEPlineTable = strAEPlineTable;
        AE_LOG("[%s()] i4SensorDev:%d i4CapIndex:%d CapId:%d\n", __FUNCTION__, m_eSensorDev, *i4CapIndex, strAEPlineTable.eID);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    if (m_p3ANVRAM != NULL)
    {
        a_rDeviceInfo = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo;
    }
    else
    {
        AE_LOG("[%s()] i4SensorDev:%d NVRAM Data is NULL\n", __FUNCTION__, m_eSensorDev);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEConfigParams(MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen)
{
    bHDRen = m_bEnableHDRYConfig;
    bOverCnten = m_bEnableOverCntConfig;
    bTSFen = m_bEnableTSFConfig;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsDoAEInPreAF()
{
    AE_LOG("[%s] i4SensorDev:%d DoAEbeforeAF:%d\n", __FUNCTION__, m_eSensorDev, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bPreAFLockAE);
    return m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bPreAFLockAE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsAEStable()
{
    if (m_bAEMgrDebugEnable)
    {
        AE_LOG("[%s] i4SensorDev:%d m_bAEStable:%d\n", __FUNCTION__, m_eSensorDev, m_bAEStable);
    }
    return m_bAEStable;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsAELock()
{
    if (m_bAEMgrDebugEnable)
    {
        AE_LOG("[%s] i4SensorDev:%d m_bAELock/m_bAPAELock/m_bAFAELock:%d/%d/%d\n", __FUNCTION__, m_eSensorDev, m_bAELock, m_bAPAELock, m_bAFAELock);
    }
    return m_bAELock;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsAPAELock()
{
    if (m_bAEMgrDebugEnable)
    {
        AE_LOG("[%s] i4SensorDev:%d m_bAELock/m_bAPAELock/m_bAFAELock:%d/%d/%d\n", __FUNCTION__, m_eSensorDev, m_bAELock, m_bAPAELock, m_bAFAELock);
    }
    return m_bAPAELock;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getBVvalue()
{
    return (m_BVvalue);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getDeltaBV()
{
    return (m_i4DeltaBV);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsStrobeBVTrigger()
{
    MBOOL bStrobeBVTrigger;
    MINT32 i4Bv = 0;

    if (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEV0TriggerStrobe == MTRUE)
    {
        // The strobe trigger by the EV 0 index
        i4Bv = m_BVvalueWOStrobe;
    }
    else
    {
        if (m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp])
        {
            if (m_pIAeAlgo != NULL)
            {
                i4Bv = m_BVvalueWOStrobe + m_pIAeAlgo->getSenstivityDeltaIndex(1024 *1024 / m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp]);
            }
            else
            {
                i4Bv = m_BVvalueWOStrobe;
                AE_LOG("[%s] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
            }
        }
    }

    if (m_pIAeAlgo != NULL)
    {
        getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable, m_CurrentStrobetureTable);
        AE_LOG_IF(m_3ALogEnable, "[%s] getPlineTable PreId:%d CapId:%d Strobe:%d\n", __FUNCTION__, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, m_CurrentStrobetureTable.eID);
    }

    bStrobeBVTrigger = (i4Bv < m_CurrentCaptureTable.i4StrobeTrigerBV)?MTRUE:MFALSE;

    AE_LOG_IF(m_3ALogEnable, "[%s] i4SensorDev:%d bStrobeBVTrigger:%d BV:%d %d\n", __FUNCTION__, m_eSensorDev, bStrobeBVTrigger, i4Bv, m_CurrentCaptureTable.i4StrobeTrigerBV);

    return bStrobeBVTrigger;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setStrobeMode(MBOOL bIsStrobeOn)
{
    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->setStrobeMode(bIsStrobeOn);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    m_bStrobeOn = bIsStrobeOn;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MBOOL bReCalcul)
{
    if(bReCalcul) {
      strAEInput rAEInput;
      strAEOutput rAEOutput;
      memset(&rAEInput, 0, sizeof(strAEInput));

      rAEInput.eAeState = AE_STATE_AELOCK;
      if(m_pIAeAlgo != NULL) {
          Mutex::Autolock lock(m_Lock);
          AE_LOG_IF(m_3ALogEnable,"[%s]+ add protection \n", __FUNCTION__);
          m_pIAeAlgo->setEVCompensate(m_eAEEVcomp);
          m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
      } else {
          AE_LOG( "[%s()] The AE algo class is NULL  i4SensorIdx = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
      }
      MY_LOG("[%s()] i4SensorDev:%d u4Eposuretime:%d SensorGain:%d ISPGain:%d\n", __FUNCTION__, m_eSensorDev, rAEOutput.EvSetting.u4Eposuretime, rAEOutput.EvSetting.u4AfeGain, rAEOutput.EvSetting.u4IspGain);

      a_rPreviewInfo.u4ExposureMode = 0;
      a_rPreviewInfo.u4Eposuretime=rAEOutput.EvSetting.u4Eposuretime;
      a_rPreviewInfo.u4AfeGain=rAEOutput.EvSetting.u4AfeGain;
      a_rPreviewInfo.u4IspGain=rAEOutput.EvSetting.u4IspGain;
      a_rPreviewInfo.i2FlareGain=rAEOutput.i2FlareGain;
      a_rPreviewInfo.i2FlareOffset=rAEOutput.i2FlareOffset;
      a_rPreviewInfo.u4RealISO=rAEOutput.u4ISO;
   } else {
    //For update both master & slave cams' CWV
    if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
    {
        if (m_u4InitWaitCount < 3)
        {
            AE_CORE_MAIN_OUT CCUoutput;
            m_pIAeFlowCCU->getCCUResult(&CCUoutput, MFALSE);

            m_rAEOutput.rPreviewMode.u4Eposuretime = CCUoutput.a_Output.EvSetting.u4Eposuretime;
            m_rAEOutput.rPreviewMode.u4AfeGain = CCUoutput.a_Output.EvSetting.u4AfeGain;
            m_rAEOutput.rPreviewMode.u4IspGain = CCUoutput.a_Output.EvSetting.u4IspGain;

            m_u4Index = CCUoutput.a_Output.u4Index;
            m_i4AEidxNext = m_u4Index;
            m_u4IndexF = CCUoutput.a_Output.u4IndexF;
            m_u4CCUResultCwvy = CCUoutput.a_Output.u4CWValue;
            m_u4CCUResultCwvyStable = CCUoutput.a_Output.u4CWRecommendStable;
        }
        m_rAEOutput.rPreviewMode.u4CWValue = m_u4CCUResultCwvy;
        m_rAEOutput.rPreviewMode.u4AvgY = m_u4CCUResultCwvy;
        //copyCCUAEInfo2mgr(&CCUoutput);
    }
    else
    {
        if (m_pIAeAlgo != NULL)
        {
            m_rAEOutput.rPreviewMode.u4CWValue = m_pIAeAlgo->getBrightnessCenterWeightedValue();
            m_rAEOutput.rPreviewMode.u4AvgY = m_pIAeAlgo->getBrightnessAverageValue();
        }
        else
        {
            m_rAEOutput.rPreviewMode.u4CWValue = 0;
            AE_LOG_IF(m_3ALogEnable, "[%s()] m_pIAeAlgo NULL", __FUNCTION__);
        }
    }
    m_rAEOutput.rPreviewMode.u4AEFinerEVIdxBase = m_u4FinerEVIdxBase;
    a_rPreviewInfo = m_rAEOutput.rPreviewMode;
  }
    AE_LOG_IF(m_3ALogEnable, "[%s()] SensorDev:%d Preview Shutter:%d Sensor gain:%d Isp gain:%d flare:%d %d ISO:%d IdxBase:%d %d \n", __FUNCTION__, m_eSensorDev, a_rPreviewInfo.u4Eposuretime, a_rPreviewInfo.u4AfeGain,
        a_rPreviewInfo.u4IspGain, a_rPreviewInfo.i2FlareGain, a_rPreviewInfo.i2FlareOffset, a_rPreviewInfo.u4RealISO, m_u4FinerEVIdxBase, m_u4InitWaitCount);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsGetManualExpState()
{
    MBOOL   ccu_state = MFALSE;
    ccu_state= (m_bIsLongExpControlbyCCU && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && (m_u4HDRcountforrevert == 0));

    AE_LOG_IF(m_3ALogEnable, "[%s()] ccu_state:%d, m_eAEMode:%d, u4ExposureTime:%d, u4Sensitivity:%d \n", __FUNCTION__, ccu_state, m_eAEMode, m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime, m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity);

    if((ccu_state) && (m_eAEMode == LIB3A_AE_MODE_OFF) &&
       (m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4ExposureTime != 0) &&
       (m_SensorQueueCtrl.rSensorParamQueue[m_uManualModeIndex].u4Sensitivity != 0))
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAFAEOutput(strAEOutput &a_rAEOutput)
{
    a_rAEOutput = m_rAFAEOutput;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorListenerParams(MINT32 *i4SensorInfo)
{
    GyroSensor_Param_T *pGyroSensorParam = reinterpret_cast <GyroSensor_Param_T *> (i4SensorInfo);
    m_i4AcceInfo[0] = pGyroSensorParam->i4AcceInfo[0];
    m_i4AcceInfo[1] = pGyroSensorParam->i4AcceInfo[1];
    m_i4AcceInfo[2] = pGyroSensorParam->i4AcceInfo[2];

    m_i4GyroInfo[0] = pGyroSensorParam->i4GyroInfo[0];
    m_i4GyroInfo[1] = pGyroSensorParam->i4GyroInfo[1];
    m_i4GyroInfo[2] = pGyroSensorParam->i4GyroInfo[2];

    AE_LOG_IF(m_3ALogEnable, "[%s()] Acce = %d/%d/%d , Gyro = %d/%d/%d\n",
        __FUNCTION__, m_i4AcceInfo[0], m_i4AcceInfo[1], m_i4AcceInfo[2],
        m_i4GyroInfo[0], m_i4GyroInfo[1], m_i4GyroInfo[2]);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateAEUnderExpdeltaBVIdx(MINT32 i4AEdeltaBV)
{
    if (m_i4AEUnderExpDeltaBVIdx != i4AEdeltaBV)
    {
        MY_LOG("[%s()] i4SensorDev:%d DeltaBVidx:%d->%d\n", __FUNCTION__, m_eSensorDev, m_i4AEUnderExpDeltaBVIdx, i4AEdeltaBV);
        m_i4AEUnderExpDeltaBVIdx = i4AEdeltaBV;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext, MINT32 i4AEidxNextF)
{
    mPreviewMode = a_rPreviewInfo;
    m_rAEOutput.rAFMode = mPreviewMode;
    m_rAEOutput.rPreviewMode = mPreviewMode;
    m_i4WaitVDNum = 0; // reset the delay frame
    m_i4AEidxNext = i4AEidxNext;
    m_i4AEidxNextF = i4AEidxNextF;

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->updateAEIndex(i4AEidxNext, i4AEidxNextF);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    AE_LOG("[%s()] m_i4SensorDev:%d i4AEidxNext:%d i4AEidxNextF:%d Exp. mode = %d Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d\n",
        __FUNCTION__, m_eSensorDev, i4AEidxNext, i4AEidxNextF, mPreviewMode.u4ExposureMode, mPreviewMode.u4Eposuretime,
        mPreviewMode.u4AfeGain, mPreviewMode.u4IspGain, mPreviewMode.u2FrameRate, mPreviewMode.i2FlareGain, mPreviewMode.i2FlareOffset, mPreviewMode.u4RealISO);

    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateISPNvramOBCTable(MVOID * pISPNvramOBCTable, MINT32 i4Tableidx)
{
    if (m_i4OBCTableidx != i4Tableidx)
    {
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
        //AE_LOG("[%s()] pObc(0x%x)", __FUNCTION__, pISPNvramOBCTable);
        memcpy(&m_rISPNvramOBC_Table[0], pISPNvramOBCTable, 4 * sizeof(CCU_ISP_NVRAM_OBC_T));
        AE_LOG("[%s()] m_i4SensorDev:%d i4OBCtableidx:%d->%d [0][1][2][3](offset/gain) = [%d/%d][%d/%d][%d/%d][%d/%d]\n",
            __FUNCTION__, m_eSensorDev, m_i4OBCTableidx, i4Tableidx,
            m_rISPNvramOBC_Table[0].offst0.val, m_rISPNvramOBC_Table[0].gain0.val,
            m_rISPNvramOBC_Table[1].offst0.val, m_rISPNvramOBC_Table[1].gain0.val,
            m_rISPNvramOBC_Table[2].offst0.val, m_rISPNvramOBC_Table[2].gain0.val,
            m_rISPNvramOBC_Table[3].offst0.val, m_rISPNvramOBC_Table[3].gain0.val);
        m_i4OBCTableidx = i4Tableidx;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateISPNvramOBCIndex(MVOID * rMappingInfo, MINT32 i4Tableidx)
{
    AE_LOG_IF(m_3ALogEnable, "[%s()] m_i4SensorDev:%d i4OBCtableidx:%d->%d", __FUNCTION__, m_eSensorDev, m_i4ISP5OBCITableidx, i4Tableidx);
    if ((m_i4ISP5OBCITableidx != i4Tableidx) || (m_u4InitWaitCount == 0))
    {
        AE_LOG_IF(m_3ALogEnable, "[%s()] m_i4SensorDev:%d i4OBCtableidx:%d->%d", __FUNCTION__, m_eSensorDev, m_i4ISP5OBCITableidx, i4Tableidx);
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
        MINT32 i, OBCIndex;
        CAM_IDX_QRY_COMB* MappingInfo = (CAM_IDX_QRY_COMB*)rMappingInfo;

        for (i = 0; i < EISO_NUM; i++)
        {
            MappingInfo->eISO_Idx = (EISO_T)i;
            OBCIndex = m_pIdxMgr->query(static_cast <ESensorDev_T> (m_eSensorDev), NSIspTuning::EModule_OBC, *MappingInfo, __FUNCTION__);
            memcpy(&m_rISP5NvramOBC_Table[i], &ISP_NVRAM_Ptr->OBC[OBCIndex], sizeof(CCU_ISP_NVRAM_OBC_T));
            AE_LOG_IF(m_3ALogEnable, "[%s()] m_rISP5NvramOBC_Table[%d] offset = %d", __FUNCTION__, i, m_rISP5NvramOBC_Table[i].offst0.val);
        }
        m_i4ISP5OBCITableidx = i4Tableidx;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
    MUINT8 i;
    MUINT32 rAEHistogram[AE_HISTOGRAM_BIN] = {0}, rFlareHistogram[128] = {0};
    HDR_CHDR_INFO_T rSWHdrInfo;
    strAETable strCurrentCaptureTable;
    MUINT32 u4NewSensitivityRatio = 1024, u4OldSensitivityRatio = 1024, u4SensitivityRatio = 1024;
    MINT32 i4TableDeltaIdx = 0;
    MUINT32 u4PreviewBaseGain = 1024;
    MUINT32 u4PreviewBaseISO = 100;
    MUINT32 u4FinalGain;
    MUINT32 exp_ratio = 1; //add for determine capture shutter ratio
    MBOOL bManualPlineTable = (m_eAEManualCapPline == EAEManualPline_Default) ? MFALSE : MTRUE;

    memset(&strCurrentCaptureTable, 0, sizeof(strAETable));

    if (m_p3ANVRAM != NULL)
    {
        u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    }
    else
    {
        MY_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
    }

    strHDRInputSetting.u4MaxSensorAnalogGain = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
    strHDRInputSetting.u4MaxAEExpTimeInUS = 500000; // 0.5sec
    strHDRInputSetting.u4MinAEExpTimeInUS = 500; // 500us
    if (m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapExpUnit < 10000)
    {
        strHDRInputSetting.u4ShutterLineTime = 1000 *m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapExpUnit;
    }
    else
    {
        strHDRInputSetting.u4ShutterLineTime = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapExpUnit;
    }

    strHDRInputSetting.u4MaxAESensorGain = 8 *m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
    strHDRInputSetting.u4MinAESensorGain = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MinGain;

    if (m_bRealISOSpeed == MFALSE && m_p3ANVRAM != NULL)
    {
        u4OldSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video2PreRatio;
        u4NewSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Cap2PreRatio;

        if (u4OldSensitivityRatio != 0)
        {
            u4SensitivityRatio = 1024 * u4NewSensitivityRatio / u4OldSensitivityRatio;
        }
        else
        {
            u4SensitivityRatio = 1024;
            AE_LOG("[%s()] i4SensorDev:%d u4OldSensitivityRatio is zero\n", __FUNCTION__, m_eSensorDev);
        }

        if (u4SensitivityRatio <= 300)
        {
            exp_ratio = 4;
        }
        else if (u4SensitivityRatio <= 450)
        {
            exp_ratio = 3;
        }
        else if (u4SensitivityRatio <= 768)
        {
            exp_ratio = 2;
        }
        else
        {
            exp_ratio = 1;
        }

        // Manual AE pline when AIS / BMDN / MFHR mode
        if (bManualPlineTable)
        {
            eAETableID eManualTableID = m_pPreviewTableCurrent->eID;
            MBOOL bNewPLineTableID = MFALSE;
            getAEManualCapPline(m_eAEManualCapPline, eManualTableID);

            // Search for eID
            for (i = 0; i < (MUINT32)MAX_PLINE_TABLE; i++)
            {
                if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].eID == eManualTableID)
                {
                    AE_LOG_IF(m_3ALogEnable, "[%s()] i/ePreviewPLineTableID/TotalIdx:%d/%d/%d\n", __FUNCTION__, i, eManualTableID, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
                    bNewPLineTableID = MTRUE;
                    if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex == 0)
                    {
                        AE_LOG("[%s()] The Preview AE table total index is zero:%d->%d\n", __FUNCTION__, eManualTableID, m_pPreviewTableCurrent->eID);
                        eManualTableID = m_pPreviewTableCurrent->eID;
                    }
                }
                AE_LOG_IF(m_3ALogEnable, "[%s()] i/TotalIdx:%d/%d\n", __FUNCTION__, i, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
            }
            // Check AE Pline if not existed
            if (!bNewPLineTableID)
            {
                eManualTableID = m_pPreviewTableCurrent->eID;
                AE_LOG("[%s()] The Preview AE table enum value is not supported:%d->%d\n", __FUNCTION__, bNewPLineTableID, m_pPreviewTableCurrent->eID);
            }
            getOnePlineTable(eManualTableID, strCurrentCaptureTable);
        }
        else
        {
            getOnePlineTable(m_pCaptureTable->eID, strCurrentCaptureTable);
        }
        // Calculate new index in new pline
        i4TableDeltaIdx = strCurrentCaptureTable.i4MaxBV - m_pPreviewTableCurrent->i4MaxBV;
        MUINT32 u4Newtableindex = m_i4AEidxNext + i4TableDeltaIdx;
        // Check if new index exceeds capture total index
        if (u4Newtableindex < 0)
            u4Newtableindex = 0;
        else if (u4Newtableindex >= strCurrentCaptureTable.u4TotalIndex)
            u4Newtableindex = strCurrentCaptureTable.u4TotalIndex - 1;
        // With AE Under exposure delta index in stereo BMDN and MFHR mode
        if ((m_eAEManualCapPline == EAEManualPline_BMDNCapture || m_eAEManualCapPline == EAEManualPline_MFHRCapture) && (m_i4AEUnderExpDeltaBVIdx > 0))
        {
            AE_LOG_IF(m_3ALogEnable, "[%s()] m_eAEManualCapPline = %d, NewtableIdx/UnderExpDeltaIdx = %d/%d \n", __FUNCTION__, m_eAEManualCapPline, u4Newtableindex, m_i4AEUnderExpDeltaBVIdx);
            u4Newtableindex = u4Newtableindex + m_i4AEUnderExpDeltaBVIdx;
        }
        // Use new table index to choose exposure setting in new AE Pline
        if (u4Newtableindex < strCurrentCaptureTable.u4TotalIndex)
        {
            // if ((!bManualPlineTable) && (exp_ratio > 1) && (m_u4AEISOSpeed == LIB3A_AE_ISO_SPEED_AUTO) && (m_eAEScene == LIB3A_AE_SCENE_AUTO)){
            // ModifyCaptureParamByCustom(exp_ratio, &m_rAEOutput.rPreviewMode, &m_rAEOutput.rCaptureMode[0], (MUINT32)m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain);
            // }
            // else{
            m_rAEOutput.rCaptureMode[0].u4Eposuretime = strCurrentCaptureTable.pCurrentTable->sPlineTable[u4Newtableindex].u4Eposuretime;
            m_rAEOutput.rCaptureMode[0].u4AfeGain = strCurrentCaptureTable.pCurrentTable->sPlineTable[u4Newtableindex].u4AfeGain;
            m_rAEOutput.rCaptureMode[0].u4IspGain = strCurrentCaptureTable.pCurrentTable->sPlineTable[u4Newtableindex].u4IspGain;
            u4FinalGain = (m_rAEOutput.rCaptureMode[0].u4AfeGain * m_rAEOutput.rCaptureMode[0].u4IspGain) >> 10;
            m_rAEOutput.rCaptureMode[0].u4RealISO = u4PreviewBaseISO * u4FinalGain / u4PreviewBaseGain;
            m_rAEOutput.rCaptureMode[0].i2FlareGain = m_rAEOutput.rPreviewMode.i2FlareGain;
            m_rAEOutput.rCaptureMode[0].i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
            // }
            AE_LOG("[%s()] m_i4SensorDev:%d Modify Capture Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d flare:%d %d ISO:%d Index:%d tablediffindex:%d FOUR_CELL_ADD_SHUTTER: %d, AEISOSpeed: %d, m_eAEScene: %d, exp_ratio = %d\n", __FUNCTION__, m_eSensorDev,
                m_rAEOutput.rCaptureMode[0].u4Eposuretime, m_rAEOutput.rCaptureMode[0].u4AfeGain, m_rAEOutput.rCaptureMode[0].u4IspGain, m_rAEOutput.rCaptureMode[0].u2FrameRate,
                m_rAEOutput.rCaptureMode[0].i2FlareGain, m_rAEOutput.rCaptureMode[0].i2FlareOffset, m_rAEOutput.rCaptureMode[0].u4RealISO, m_i4AEidxNext, i4TableDeltaIdx,
                FOUR_CELL_ADD_SHUTTER, m_u4AEISOSpeed, m_eAEScene, exp_ratio);
        }
        else
        {
            AE_LOG("[%s():Warning] m_i4SensorDev:%d Index:%d tablediffindex:%d\n", __FUNCTION__, m_eSensorDev, m_i4AEidxNext, i4TableDeltaIdx);
        }
    }
    strHDRInputSetting.u4ExpTimeInUS0EV = m_rAEOutput.rCaptureMode[0].u4Eposuretime;
    strHDRInputSetting.u4SensorGain0EV = (m_rAEOutput.rCaptureMode[0].u4AfeGain) *(m_rAEOutput.rCaptureMode[0].u4IspGain) >> 10;
    strHDRInputSetting.u4ISOValue = m_rAEOutput.rCaptureMode[0].u4RealISO;
    strHDRInputSetting.u1FlareOffset0EV = m_rAEOutput.rCaptureMode[0].i2FlareOffset;

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAEHistogram(2, rAEHistogram); // Y histogram output
        m_pIAeAlgo->getAEHistogram(0, rFlareHistogram); // Y histogram output
        m_pIAeAlgo->getAESWHdrInfo(rSWHdrInfo);

        strHDRInputSetting.u4LE_SERatio_x100 = 0;
        strHDRInputSetting.u4ExpRatio = exp_ratio;

        if ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET))
        {
            strHDRInputSetting.u4AOEMode = 1;
            strHDRInputSetting.i4GainBase0EV = rSWHdrInfo.i4GainBase;
            strHDRInputSetting.i4LE_LowAvg = rSWHdrInfo.i4LEInfo;
            strHDRInputSetting.i4SEDeltaEVx100 = rSWHdrInfo.i4SEInfo;
        }
        else if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
        {
            mVHDR_SWHDR_InputParam_T rInput;
            mVHDR_SWHDR_OutputParam_T rOutput;

            if (m_rSensorVCInfo.u2VCModeSelect == 0)
                rInput.i4Ratio = m_rSensorVCInfo.u2VCShutterRatio;
            else
            rInput.i4Ratio = (m_strIVHDROutputSetting.u4LEExpTimeInUS + (m_strIVHDROutputSetting.u4SEExpTimeInUS >> 1)) / m_strIVHDROutputSetting.u4SEExpTimeInUS;
            rInput.LEMax = rSWHdrInfo.i4LEInfo;
            rInput.SEMax = rSWHdrInfo.i4SEInfo;

            getMVHDR_AEInfo(rInput, rOutput);

            AE_LOG_IF(m_3ALogEnable, "[%s()] Ratio:%d, LEMax:%d, SEMax:%d, SEDeltaEV:%d\n", __FUNCTION__, rInput.i4Ratio, rInput.LEMax, rInput.SEMax, rOutput.i4SEDeltaEVx100);

            strHDRInputSetting.u4AOEMode = 1;
            strHDRInputSetting.i4GainBase0EV = rSWHdrInfo.i4GainBase;
            strHDRInputSetting.i4LE_LowAvg = 0;
            strHDRInputSetting.i4SEDeltaEVx100 = rOutput.i4SEDeltaEVx100;
        }
        else if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
        {
            MINT32 i4Ratio = m_strIVHDROutputSetting.u4LE_SERatio_x100 * 101 / 10000;
            MINT32 i4SEDeltaEVx100 = 0;

            getMVHDR3Expo_AEInfo(i4Ratio, i4SEDeltaEVx100);

            strHDRInputSetting.u4AOEMode = 1;
            strHDRInputSetting.i4GainBase0EV = rSWHdrInfo.i4GainBase;
            strHDRInputSetting.i4LE_LowAvg = 0;
            strHDRInputSetting.i4SEDeltaEVx100 = i4SEDeltaEVx100;
            strHDRInputSetting.u4LE_SERatio_x100 = m_strIVHDROutputSetting.u4LE_SERatio_x100;
            strHDRInputSetting.u4ExpRatio = 2;
            AE_LOG_IF(m_3ALogEnable, "[%s()] MVHDR3Expo u4LE_SERatio_x100:%d, u4ExpRatio:%d\n", __FUNCTION__, strHDRInputSetting.u4LE_SERatio_x100, strHDRInputSetting.u4ExpRatio);
        }
        else
        {
            strHDRInputSetting.u4AOEMode = 0;
            strHDRInputSetting.i4GainBase0EV = 0;
            strHDRInputSetting.i4LE_LowAvg = 0;
            strHDRInputSetting.i4SEDeltaEVx100 = 0;
        }

        AE_LOG_IF(m_3ALogEnable, "[%s()] u4AOEMode = %d, i4GainBase0EV = %d, i4LE_LowAvg = %d, i4SEDeltaEVx100 = %d\n", __FUNCTION__,
            strHDRInputSetting.u4AOEMode, strHDRInputSetting.i4GainBase0EV,
            strHDRInputSetting.i4LE_LowAvg, strHDRInputSetting.i4SEDeltaEVx100);

    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL (26)\n", __FUNCTION__);
    }

    strHDRInputSetting.bDetectFace = (m_eAEFDArea.u4Count > 0)? 1:0;

    for (i = 0; i < AE_HISTOGRAM_BIN; i++)
    {
        strHDRInputSetting.u4Histogram[i] = rAEHistogram[i];
        strHDRInputSetting.u4FlareHistogram[i] = rFlareHistogram[i];
    }

    AE_LOG_IF(m_3ALogEnable, "[%s()] Input MaxSensorAnalogGain:%d MaxExpTime:%d MinExpTime:%d LineTime:%d MaxSensorGain:%d ExpTime:%d SensorGain:%d FlareOffset:%d DetectFace:%d %d\n", __FUNCTION__,
        strHDRInputSetting.u4MaxSensorAnalogGain, strHDRInputSetting.u4MaxAEExpTimeInUS, strHDRInputSetting.u4MinAEExpTimeInUS, strHDRInputSetting.u4ShutterLineTime,
        strHDRInputSetting.u4MaxAESensorGain, strHDRInputSetting.u4ExpTimeInUS0EV, strHDRInputSetting.u4SensorGain0EV, strHDRInputSetting.u1FlareOffset0EV, strHDRInputSetting.bDetectFace, m_eAEFDArea.u4Count);

    for (i = 0; i < AE_HISTOGRAM_BIN; i += 8)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s()] Input Histogram%d~%d:%d %d %d %d %d %d %d %d\n", __FUNCTION__, i, i + 7, strHDRInputSetting.u4Histogram[i],
            strHDRInputSetting.u4Histogram[i + 1], strHDRInputSetting.u4Histogram[i + 2], strHDRInputSetting.u4Histogram[i + 3], strHDRInputSetting.u4Histogram[i + 4],
            strHDRInputSetting.u4Histogram[i + 5], strHDRInputSetting.u4Histogram[i + 6], strHDRInputSetting.u4Histogram[i + 7]);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo)
{
    //strAEOutput rAEOutput; /* fix build warning */

    a_rCaptureInfo = m_rAEOutput.rCaptureMode[0];

    AE_LOG("[%s()] m_i4SensorDev:%d Capture Exp. mode:%d Shutter:%d Sensor gain:%d Isp gain:%d flare:%d %d ISO:%d\n", __FUNCTION__, m_eSensorDev, a_rCaptureInfo.u4ExposureMode, a_rCaptureInfo.u4Eposuretime, a_rCaptureInfo.u4AfeGain,
        a_rCaptureInfo.u4IspGain, a_rCaptureInfo.i2FlareGain, a_rCaptureInfo.i2FlareOffset, a_rCaptureInfo.u4RealISO);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo, MBOOL bRemosaicEn)
{
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain = 1024;
    MUINT32 u4PreviewBaseISO = 100;
    AE_EXP_GAIN_MODIFY_T rSensorInputData, rSensorOutputData;
    memset(&rSensorOutputData, 0, sizeof(AE_EXP_GAIN_MODIFY_T));
    if (m_p3ANVRAM != NULL)
    {
        u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    }
    else
    {
        AE_LOG("[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
    }
    mCaptureMode = a_rCaptureInfo;
    u4FinalGain = (mCaptureMode.u4AfeGain *mCaptureMode.u4IspGain) >> 10;
    mCaptureMode.u4RealISO = u4PreviewBaseISO * u4FinalGain / u4PreviewBaseGain;
    AE_LOG("[%s()] m_i4SensorDev:%d Exp mode:%d Capture Shutter:%d Sensor gain:%d Isp gain:%d flare:%d %d ISO:%d, without modify cap param\n",
        __FUNCTION__, m_eSensorDev, mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
        mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset, mCaptureMode.u4RealISO);

    if (m_bEnableAE)
    {
        if (bRemosaicEn)
        {
            MUINT32 u4NewSensitivityRatio = 1024, u4OldSensitivityRatio = 1024, u4SensitivityRatio = 1024;
            MUINT32 exp_ratio = 1; //add for determine capture shutter ratio
            AE_MODE_CFG_T rCaptureMode;
            memset(&rCaptureMode, 0, sizeof(AE_MODE_CFG_T));
            if (m_p3ANVRAM != NULL)
            {
                u4OldSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video2PreRatio;
                u4NewSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Cap2PreRatio;

                if (u4OldSensitivityRatio != 0)
                {
                    u4SensitivityRatio = 1024 *u4NewSensitivityRatio / u4OldSensitivityRatio;
                }
                else
                {
                    u4SensitivityRatio = 1024;
                    AE_LOG("[%s()] i4SensorDev:%d u4OldSensitivityRatio is zero\n", __FUNCTION__, m_eSensorDev);
                }
                if (u4SensitivityRatio <= 300)
                {
                    exp_ratio = 4;
                }
                else if (u4SensitivityRatio <= 450)
                {
                    exp_ratio = 3;
                }
                else if (u4SensitivityRatio <= 768)
                {
                    exp_ratio = 2;
                }
                else
                {
                    exp_ratio = 1;
                }
                ModifyCaptureParamByCustom(exp_ratio, &mCaptureMode, &rCaptureMode, (MUINT32)m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain);
                memcpy(&mCaptureMode, &rCaptureMode, sizeof(AE_MODE_CFG_T));
                AE_LOG("[%s()] enable remosaic binning sum transform, i4SensorDev:%d exp_ratio:%d Capture Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d\n", __FUNCTION__, m_eSensorDev,
                    exp_ratio, mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u4RealISO);
            }
        }

        if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
        {
            AE_LOG("[%s()] MVHDR3Expo: flash update\n", __FUNCTION__);
            m_bMVHDR3FlashUpdate = MTRUE;
        }
        if (((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)))
        {
            strEvSetting rEvSetting;
            strEvHdrSetting rHdrEvSetting;
            rEvSetting.u4Eposuretime = mCaptureMode.u4Eposuretime;
            rEvSetting.u4AfeGain = mCaptureMode.u4AfeGain;
            rEvSetting.u4IspGain = mCaptureMode.u4IspGain;
            rEvSetting.uIris = 0;
            rEvSetting.uSensorMode = 0;
            rEvSetting.uFlag = 0;
            if ((m_rAEInitInput.rAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.i4FlashRatio !=-1) && ((m_eAEMode == LIB3A_AE_MODE_ON_AUTO_FLASH) || (m_eAEMode == LIB3A_AE_MODE_ON_ALWAYS_FLASH)))
            {
                AE_LOG("[%s()] HDR Flash Ratio=%d", __FUNCTION__, m_rAEInitInput.rAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.i4FlashRatio);
                if (m_pIAeAlgo != NULL)
                {
                    m_pIAeAlgo->setHdrRatio(m_rAEInitInput.rAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.i4FlashRatio);
                }
            }
            if (m_pIAeAlgo != NULL)
            {
                m_pIAeAlgo->calRatio2AEInfoIf(rEvSetting);
                m_pIAeAlgo->updateHdrInfo(rHdrEvSetting);
            }
            else
            {
                AE_LOG("[%s()] The AE algo class is NULL", __FUNCTION__);
            }
            m_strIVHDRCaptureSetting.u4LEExpTimeInUS = rHdrEvSetting.rHdrEVInfo.i4LEExpo;
            m_strIVHDRCaptureSetting.u4LESensorGain = rHdrEvSetting.rHdrEVInfo.i4LEAfeGain;
            m_strIVHDRCaptureSetting.u4LEISPGain = rHdrEvSetting.rHdrEVInfo.i4LEIspGain;
            m_strIVHDRCaptureSetting.u4SEExpTimeInUS = rHdrEvSetting.rHdrEVInfo.i4SEExpo;
            m_strIVHDRCaptureSetting.u4SESensorGain = rHdrEvSetting.rHdrEVInfo.i4SEAfeGain;
            m_strIVHDRCaptureSetting.u4SEISPGain = rHdrEvSetting.rHdrEVInfo.i4SEIspGain;
            m_strIVHDRCaptureSetting.u4MEExpTimeInUS = rHdrEvSetting.rHdrEVInfo.i4MEExpo;
            m_strIVHDRCaptureSetting.u4MESensorGain = rHdrEvSetting.rHdrEVInfo.i4MEAfeGain;
            m_strIVHDRCaptureSetting.u4MEISPGain = rHdrEvSetting.rHdrEVInfo.i4MEIspGain;
            m_strIVHDRCaptureSetting.u4LE_SERatio_x100 = rHdrEvSetting.rHdrEVInfo.i4HdrRatio;
            m_i4LESE_Ratio = m_strIVHDRCaptureSetting.u4LE_SERatio_x100;
        }
    }

    m_rAEOutput.rCaptureMode[0] = mCaptureMode;
    m_bAECaptureUpdate = MTRUE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setSensorDirectly(CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting)
{
    AE_MODE_CFG_T rCaptureInfo;

    AE_LOG("[%s()] m_i4SensorDev:%d Mode:%d Shutter:%d Sensor Gain:%d ISP Gain:%d ISO:%d Flare Offset:%d %d\n", __FUNCTION__, m_eSensorDev, a_rCaptureInfo.u4ExposureMode, a_rCaptureInfo.u4Eposuretime,
        a_rCaptureInfo.u4AfeGain, a_rCaptureInfo.u4IspGain, a_rCaptureInfo.u4RealISO, a_rCaptureInfo.u4FlareGain, a_rCaptureInfo.u4FlareOffset);

    rCaptureInfo.u4ExposureMode = a_rCaptureInfo.u4ExposureMode;
    rCaptureInfo.u4Eposuretime = a_rCaptureInfo.u4Eposuretime;
    rCaptureInfo.u4AfeGain = a_rCaptureInfo.u4AfeGain;
    rCaptureInfo.u4IspGain = a_rCaptureInfo.u4IspGain;
    rCaptureInfo.u4RealISO = a_rCaptureInfo.u4RealISO;
    rCaptureInfo.i2FlareGain = (MINT16)a_rCaptureInfo.u4FlareGain;
    rCaptureInfo.i2FlareOffset = (MINT16)a_rCaptureInfo.u4FlareOffset;

    updateCaptureParams(rCaptureInfo);

    if (bDirectlySetting)
    {
        if ((m_eCamMode == eAppMode_EngMode) && ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)))
        {
            // engineer mode and target mode
            UpdateSensorISPParams(AE_TWO_SHUTTER_INIT_STATE);
        }
        else
        {
            UpdateSensorISPParams(AE_CAPTURE_STATE);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    Mutex::Autolock lock(m_Lock);
    MUINT8 iValue = 0;
    AEMeterArea_T sAEMeteringArea = rWinSize;
    // Mapping AE region from Hbin size to [ -1000, 1000]
    sAEMeteringArea.i4Left   = sAEMeteringArea.i4Left   * 2000 / m_u4Hbinwidth  - 1000;
    sAEMeteringArea.i4Right  = sAEMeteringArea.i4Right  * 2000 / m_u4Hbinwidth  - 1000;
    sAEMeteringArea.i4Top    = sAEMeteringArea.i4Top    * 2000 / m_u4Hbinheight - 1000;
    sAEMeteringArea.i4Bottom = sAEMeteringArea.i4Bottom * 2000 / m_u4Hbinheight - 1000;
    // Bounding check
    if (sAEMeteringArea.i4Left   < -1000) {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000) {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000) {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000) {sAEMeteringArea.i4Bottom = -1000;}
    if (sAEMeteringArea.i4Left   >  1000) {sAEMeteringArea.i4Left   =  1000;}
    if (sAEMeteringArea.i4Right  >  1000) {sAEMeteringArea.i4Right  =  1000;}
    if (sAEMeteringArea.i4Top    >  1000) {sAEMeteringArea.i4Top    =  1000;}
    if (sAEMeteringArea.i4Bottom >  1000) {sAEMeteringArea.i4Bottom =  1000;}
    AE_LOG_IF(m_3ALogEnable, "[%s()] AE meter area Left:%d Right:%d Top:%d Bottom:%d \n", __FUNCTION__,
        sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom);

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAEMeteringAreaValue(sAEMeteringArea, &iValue);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    *uYvalue = iValue;

    // AE_LOG("[%s()] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", __FUNCTION__, sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringBlockAreaValue(AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    Mutex::Autolock lock(m_Lock);
    AEMeterArea_T sAEMeteringArea = rWinSize;
    // Mapping AE region from Hbin size to [ -1000, 1000]
    sAEMeteringArea.i4Left   = sAEMeteringArea.i4Left   * 2000 / m_u4Hbinwidth  - 1000;
    sAEMeteringArea.i4Right  = sAEMeteringArea.i4Right  * 2000 / m_u4Hbinwidth  - 1000;
    sAEMeteringArea.i4Top    = sAEMeteringArea.i4Top    * 2000 / m_u4Hbinheight - 1000;
    sAEMeteringArea.i4Bottom = sAEMeteringArea.i4Bottom * 2000 / m_u4Hbinheight - 1000;
    // Bounding check
    if (sAEMeteringArea.i4Left   < -1000) {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000) {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000) {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000) {sAEMeteringArea.i4Bottom = -1000;}
    if (sAEMeteringArea.i4Left   >  1000) {sAEMeteringArea.i4Left   =  1000;}
    if (sAEMeteringArea.i4Right  >  1000) {sAEMeteringArea.i4Right  =  1000;}
    if (sAEMeteringArea.i4Top    >  1000) {sAEMeteringArea.i4Top    =  1000;}
    if (sAEMeteringArea.i4Bottom >  1000) {sAEMeteringArea.i4Bottom =  1000;}
    AE_LOG_IF(m_3ALogEnable, "[%s()] AE meter area Left:%d Right:%d Top:%d Bottom:%d\n", __FUNCTION__,
        sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom);

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAEMeteringBlockAreaValue(sAEMeteringArea, uYvalue, u2YCnt);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL (25)\n", __FUNCTION__);
    }

    // AE_LOG("[%s()] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", __FUNCTION__, sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size)
{
    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAEBlockYvalues(pYvalues, size);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getRTParams(FrameOutputParam_T &a_strFrameInfo)
{
    strAEInput rAEInput;
    strAEOutput rAEOutput;
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain = 1024;
    MUINT32 u4PreviewBaseISO = 100;
    memset(&rAEInput, 0, sizeof(strAEInput));
    a_strFrameInfo.u4AEIndex = m_i4AEidxNext;
    a_strFrameInfo.u4AEIndexF = m_i4AEidxNextF;
    a_strFrameInfo.u4FinerEVIdxBase = m_u4FinerEVIdxBase;
    a_strFrameInfo.u4PreviewShutterSpeed_us = m_rAEOutput.rPreviewMode.u4Eposuretime;
    a_strFrameInfo.u4PreviewSensorGain_x1024 = m_rAEOutput.rPreviewMode.u4AfeGain;
    a_strFrameInfo.u4PreviewISPGain_x1024 = m_rAEOutput.rPreviewMode.u4IspGain;
    u4FinalGain = (a_strFrameInfo.u4PreviewSensorGain_x1024 *a_strFrameInfo.u4PreviewISPGain_x1024) >> 10;
    if (m_p3ANVRAM != NULL)
    {
        a_strFrameInfo.u4RealISOValue = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain * u4FinalGain / u4PreviewBaseGain;
    }
    else
    {
        a_strFrameInfo.u4RealISOValue = u4PreviewBaseISO * u4FinalGain / u4PreviewBaseGain;
    }

    if (m_eCamMode == eAppMode_EngMode)
    {
        rAEInput.eAeState = AE_STATE_CAPTURE;
        rAEInput.rHdrGyroInfo.i4Acce[0] = m_i4AcceInfo[0];
        rAEInput.rHdrGyroInfo.i4Acce[1] = m_i4AcceInfo[1];
        rAEInput.rHdrGyroInfo.i4Acce[2] = m_i4AcceInfo[2];
        rAEInput.rHdrGyroInfo.i4Gyro[0] = m_i4GyroInfo[0];
        rAEInput.rHdrGyroInfo.i4Gyro[1] = m_i4GyroInfo[1];
        rAEInput.rHdrGyroInfo.i4Gyro[2] = m_i4GyroInfo[2];

        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->handleAE(&rAEInput, &rAEOutput);
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        // VHDR or non -VHDR condition
        if ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET))
        {
            a_strFrameInfo.u4CapShutterSpeed_us = m_strIVHDROutputSetting.u4LEExpTimeInUS;
            a_strFrameInfo.u4CapSensorGain_x1024 = m_strIVHDROutputSetting.u4LESensorGain;
            a_strFrameInfo.u4CapISPGain_x1024 = m_strIVHDROutputSetting.u4LEISPGain;
        }
        else
        {
            copyAEInfo2mgr(&m_rAEOutput.rCaptureMode[0], &rAEOutput);
            a_strFrameInfo.u4CapShutterSpeed_us = m_rAEOutput.rCaptureMode[0].u4Eposuretime;
            a_strFrameInfo.u4CapSensorGain_x1024 = m_rAEOutput.rCaptureMode[0].u4AfeGain;
            a_strFrameInfo.u4CapISPGain_x1024 = m_rAEOutput.rCaptureMode[0].u4IspGain;
        }
    }
    else
    {
        a_strFrameInfo.u4CapShutterSpeed_us = m_rAEOutput.rPreviewMode.u4Eposuretime;
        a_strFrameInfo.u4CapSensorGain_x1024 = m_rAEOutput.rPreviewMode.u4AfeGain;
        a_strFrameInfo.u4CapISPGain_x1024 = m_rAEOutput.rPreviewMode.u4IspGain;
    }
    a_strFrameInfo.u4FRameRate_x10 = m_rAEOutput.rPreviewMode.u2FrameRate;
    a_strFrameInfo.i4BrightValue_x10 = m_BVvalue;
    a_strFrameInfo.i4ExposureValue_x10 = m_i4EVvalue;
    a_strFrameInfo.i4LightValue_x10 = (m_BVvalue + 50);
    a_strFrameInfo.u4AvgY = m_rAEOutput.rPreviewMode.u4AvgY;
    a_strFrameInfo.i4LuxValue_x10000 = 10000 * pow(2, (a_strFrameInfo.i4LightValue_x10 / 10 - 3));
    a_strFrameInfo.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
    a_strFrameInfo.i2FlareGain = m_rAEOutput.rPreviewMode.i2FlareGain;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAECondition(MUINT32 i4AECondition)
{
    if (i4AECondition & m_u4AECondition)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getLCEPlineInfo(LCEInfo_T &a_rLCEInfo)
{
    MUINT32 u4LCEStartIdx = 0, u4LCEEndIdx = 0;

    if (m_pIAeAlgo != NULL)
    {
        m_pIAeAlgo->getAELCEIndexInfo(&u4LCEStartIdx, &u4LCEEndIdx);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    a_rLCEInfo.i4AEidxCur = m_i4AEidxCurrent;
    a_rLCEInfo.i4AEidxNext = m_i4AEidxNext;
    a_rLCEInfo.i4AEidxNextF = m_i4AEidxNextF;
    a_rLCEInfo.i4NormalAEidx = (MINT32) u4LCEStartIdx;
    a_rLCEInfo.i4LowlightAEidx = (MINT32) u4LCEEndIdx;

    if (m_bAEMgrDebugEnable)
    {
        AE_LOG("[%s()] m_i4SensorDev:%d i4AEidxCur:%d i4AEidxNext:%d i4AEidxNextF:%d i4NormalAEidx:%d i4LowlightAEidx:%d\n", __FUNCTION__, m_eSensorDev, a_rLCEInfo.i4AEidxCur, a_rLCEInfo.i4AEidxNext, a_rLCEInfo.i4AEidxNextF, a_rLCEInfo.i4NormalAEidx, a_rLCEInfo.i4LowlightAEidx);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAERotateDegree(MINT32 i4RotateDegree)
{
    if (m_i4RotateDegree == i4RotateDegree)
    {
        // the same degree
        return S_AE_OK;
    }

    AE_LOG("[%s()] m_i4SensorDev:%d old:%d\n", __FUNCTION__, m_eSensorDev, m_i4RotateDegree);
    m_i4RotateDegree = i4RotateDegree;

    if (m_pIAeAlgo != NULL)
    {
        if ((i4RotateDegree == 90) || (i4RotateDegree == 270))
        {
            m_pIAeAlgo->setAERotateWeighting(MTRUE);
        }
        else
        {
            m_pIAeAlgo->setAERotateWeighting(MFALSE);
        }
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateSensorISPParams(AE_STATE_T eNewAEState)
{
    MRESULT err;
    //AE_INFO_T rAEInfo2ISP; /* fix build warning */
    //MUINT32 u4IndexRatio; /* fix build warning */
    //MUINT32 u4Index = 0;
    MUINT32 u4ISOValue = 100;
    //MUINT32 u4FinalGain; /* fix build warning */
    //MUINT32 u4PreviewBaseGain = 1024; /* fix build warning */
    //MUINT32 u4PreviewBaseISO = 100; /* fix build warning */
    //MBOOL bCaptureState = MFALSE; /* fix build warning */
    MUINT16 u2FrameRate;
    MUINT32 u4ISPGain = 1024;
    MUINT32 u4MinIsoGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    MUINT32 u4MaxGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain;

    m_AEState = eNewAEState;

    switch (eNewAEState)
    {
        case AE_INIT_STATE:
        case AE_REINIT_STATE:
            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth;
            // sensor initial and send shutter / gain default value
            AE_LOG("[%s()] i4SensorDev:%d Exp Mode:%d Shutter:%d Sensor Gain:%d Isp Gain:%d Flare Gain/offset:%d/%d\n",
                __FUNCTION__, m_eSensorDev, m_rAEOutput.rPreviewMode.u4ExposureMode, m_rAEOutput.rPreviewMode.u4Eposuretime,
                m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain, m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset);

            (m_rAEOutput.rPreviewMode.u4ExposureMode == eAE_EXPO_TIME) ? (err = AAASensorMgr::getInstance().setPreviewParams((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain))
            : (err = AAASensorMgr::getInstance().setPreviewLineBaseParams((ESensorDev_T)m_eSensorDev, m_rAEOutput.rPreviewMode.u4Eposuretime, m_rAEOutput.rPreviewMode.u4AfeGain));

            if (FAILED(err))
            {
                CAM_LOGE("[%s()] AAASensorMgr::getInstance().setPreviewParams fail\n", __FUNCTION__);
                return err;
            }

            if (m_i4SMSubSamples > 1)
            {
                m_i4AEMaxFps = m_rSensorMaxFrmRate[m_eSensorMode];
                m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                AAASensorMgr::getInstance().setPreviewMaxFrameRate((ESensorDev_T)m_eSensorDev, m_u4UpdateFrameRate_x10, m_eSensorMode);
                MY_LOG("[%s()] Forced set max frame rate, m_i4SMSubSamples:%d m_u4UpdateFrameRate_x10:%6.3f\n", __FUNCTION__, m_i4SMSubSamples, m_u4UpdateFrameRate_x10);
            }

            m_u4PrevExposureTime = m_u4PreExposureTime = m_rAEOutput.rPreviewMode.u4Eposuretime;
            m_u4PrevSensorGain = m_u4PreSensorGain = m_rAEOutput.rPreviewMode.u4AfeGain;
            m_u4PrevIspGain = m_u4PreIspGain = m_rAEOutput.rPreviewMode.u4IspGain;

            m_bAEStable = MFALSE;
            updateAEInfo2ISP(AE_INIT_STATE, 0x00);
            m_u4StableCnt = 0;
            m_i4WaitVDNum = 0;
            if(!m_bAELock) m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
            m_u4PreviousSensorgain = m_rAEOutput.rPreviewMode.u4AfeGain;
            m_u4PreviousISPgain = m_rAEOutput.rPreviewMode.u4IspGain;
            break;
        case AE_AUTO_FRAMERATE_STATE:
        case AE_MANUAL_FRAMERATE_STATE:
            if (m_bAEStable == MFALSE)
            {
                m_u4StableCnt = 0;
            }
            else
            {
                if (m_u4StableCnt < 0xFFFF)
                {
                    m_u4StableCnt++;
                }
            }

            AE_LOG_IF(m_3ALogEnable, "[%s():s] i4SensorDev:%d VDNum %d, Prev %d/%d/%d, Output %d/%d/%d Smooth:%d m_eAEState:%d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum,
                m_u4PreExposureTime,
                m_u4PreSensorGain,
                m_u4PreIspGain,
                mPreviewMode.u4Eposuretime,
                mPreviewMode.u4AfeGain,
                mPreviewMode.u4IspGain,
                m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableAESmoothGain, m_eAEState);
            if (m_bEnSWBuffMode)
            {
                updatePreviewParamsByHFSM(&mPreviewMode);
            }
            if ((m_u4PrevExposureTime == 0) || (m_u4PrevSensorGain == 0) || (m_u4PrevIspGain == 0))
            {
                m_u4PrevExposureTime = mPreviewMode.u4Eposuretime;
                m_u4PrevSensorGain = mPreviewMode.u4AfeGain;
                m_u4PrevIspGain = mPreviewMode.u4IspGain;
            }
            if (m_bPerframeAEFlag && (!m_bEnSWBuffMode))
            {
                // Frame rate control
                if ((m_i4AEMinFps == m_i4AEMaxFps) || (m_i4AEMaxFps <= LIB3A_AE_FRAMERATE_MODE_60FPS))
                {
                    // frame rate control
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_FRAMERATE, MTRUE);
                    m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                }
                // Raise shutter command flag for I2C
                if (m_i4ShutterDelayFrames == 0)
                {
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
                    m_u4UpdateShutterValue = m_rAEOutput.rPreviewMode.u4Eposuretime;
                }
                if ((m_i4SensorGainDelayFrames == m_i4ShutterDelayFrames) || m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth)
                {
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
                    m_u4UpdateGainValue = m_rAEOutput.rPreviewMode.u4AfeGain;
                }
                if (m_i4SensorGainDelayFrames == m_i4ShutterDelayFrames + 1)
                {
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
                    m_u4UpdateGainValue = m_u4PreviousSensorgain;
                }
                // Send ISP gain command
                if ((m_i4IspGainDelayFrames == m_i4ShutterDelayFrames) || m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth)
                {
                    updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
                    AE_LOG_IF(m_3ALogEnable, "[%s()] Isp gain value(current) = %d\n", __FUNCTION__, m_rAEOutput.rPreviewMode.u4IspGain);
                }
                else if (m_i4IspGainDelayFrames == m_i4ShutterDelayFrames + 1)
                {
                    updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_u4PreviousISPgain);
                    AE_LOG("[%s()] Isp gain value(previous) = %d\n", __FUNCTION__, m_u4PreviousISPgain);
                }
                else
                {
                    AE_LOG("[%s()] Perframe update AE warning : ISPGain delay should be more than 1.\n", __FUNCTION__);
                }
                // Prepare Isp Gain for command sending
                if ((m_i4IspGainDelayFrames - m_i4ShutterDelayFrames) == 1)
                {
                    m_u4PreviousISPgain = mPreviewMode.u4IspGain;
                }
                // Prepare Afe Gain for command sending
                if ((m_i4SensorGainDelayFrames - m_i4ShutterDelayFrames) == 1)
                {
                    m_u4PreviousSensorgain = mPreviewMode.u4AfeGain;
                }
                // reset condition
                m_u4PrevExposureTime = m_u4PreExposureTime = m_rAEOutput.rPreviewMode.u4Eposuretime;
                m_u4PrevSensorGain = m_u4PreSensorGain = m_rAEOutput.rPreviewMode.u4AfeGain;
                m_u4PrevIspGain = m_u4PreIspGain = m_rAEOutput.rPreviewMode.u4IspGain;

            }
            else
            {
                if (m_i4WaitVDNum <= (m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET))
                {
                    // restart
                    m_u4PreExposureTime = m_u4PrevExposureTime;
                    m_u4PreSensorGain = m_u4PrevSensorGain;
                    m_u4PreIspGain = m_u4PrevIspGain;

                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_FRAMERATE, MTRUE);
                    m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;

                    if (m_i4WaitVDNum == m_i4ShutterDelayFrames)
                    {
                        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
                        m_u4UpdateShutterValue = mPreviewMode.u4Eposuretime;
                        m_u4PrevExposureTime = mPreviewMode.u4Eposuretime;
                    }
                    if (m_i4WaitVDNum == m_i4SensorGainDelayFrames)
                    {
                        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
                        m_u4UpdateGainValue = mPreviewMode.u4AfeGain;
                        m_u4PrevSensorGain = mPreviewMode.u4AfeGain;
                    }

                    AE_LOG_IF(m_3ALogEnable, "[%s():e] i4SensorDev:%d VDNum %d, Delay %d/%d/%d, Prev %d/%d/%d, Output %d/%d/%d State:%d FrameRate:%d/%d Flare:%d/%d \n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum,
                        m_i4ShutterDelayFrames,
                        m_i4SensorGainDelayFrames,
                        m_i4IspGainDelayFrames,
                        m_u4PreExposureTime,
                        m_u4PreSensorGain,
                        m_u4PreIspGain,
                        mPreviewMode.u4Eposuretime,
                        mPreviewMode.u4AfeGain,
                        mPreviewMode.u4IspGain,
                        m_AEState, m_i4AEMinFps, m_i4AEMaxFps,
                        mPreviewMode.i2FlareOffset, mPreviewMode.i2FlareGain);

                    m_AEState = eNewAEState;
                    updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
                    m_u4PrevIspGain = mPreviewMode.u4IspGain;

                }
                else
                {
                    //AE_LOG("[%s()] i4SensorDev:%d m_i4WaitVDNum:%d \n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum);
                }
                m_i4WaitVDNum++;

                u2FrameRate = m_rAEOutput.rPreviewMode.u2FrameRate;

                if (u2FrameRate > m_i4AEMaxFps)
                {
                    AE_LOG_IF(m_3ALogEnable, "[%s()] Frame rate too large:%d %d\n", __FUNCTION__, u2FrameRate, m_i4AEMaxFps);
                    u2FrameRate = m_i4AEMaxFps;
                }

                if (u2FrameRate != 0)
                {
                    m_rSensorCurrentInfo.u8FrameDuration = 10000000000L / u2FrameRate;
                }
                else
                {
                    m_rSensorCurrentInfo.u8FrameDuration = 33000000L;
                    AE_LOG("[%s()] No Frame rate value, using default frame rate value (33ms)\n", __FUNCTION__);
                }

            }
            break;
        case AE_AF_STATE:
            // if the AF setting is the same with preview, skip the re -setting
            if ((m_eAETargetMode != AE_MODE_4CELL_MVHDR_TARGET) && ((mPreviewMode.u4Eposuretime != m_rAEOutput.rAFMode.u4Eposuretime) || (mPreviewMode.u4AfeGain != m_rAEOutput.rAFMode.u4AfeGain) ||
                    (mPreviewMode.u4IspGain != m_rAEOutput.rAFMode.u4IspGain)))
            {
                if (m_i4WaitVDNum == m_i4ShutterDelayFrames)
                {
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
                    m_u4UpdateShutterValue = m_rAEOutput.rAFMode.u4Eposuretime;
                }
                if (m_i4WaitVDNum == m_i4SensorGainDelayFrames)
                {
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
                    m_u4UpdateGainValue = m_rAEOutput.rAFMode.u4AfeGain;
                }
                if (m_i4WaitVDNum >= m_i4IspGainDelayFrames)
                {
                    updateAEInfo2ISP(AE_AF_STATE, m_rAEOutput.rAFMode.u4IspGain);
                    m_u4PrevExposureTime = 0;
                    m_u4PrevSensorGain = 0;
                    m_u4PrevIspGain = 0;
                    AE_LOG("[doAFAE] ISP Gain:%d\n", m_rAEOutput.rAFMode.u4IspGain);
                    m_bAEStable = MTRUE;
                    m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
                    prepareCapParams(MFALSE,MFALSE);
                }
                //m_i4WaitVDNum ++;
                AE_LOG("[doAFAE] Shutter:%d Sensor Gain:%d\n", m_rAEOutput.rAFMode.u4Eposuretime, m_rAEOutput.rAFMode.u4AfeGain);
            }
            else
            {
                m_bAEStable = MTRUE;
                m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_COUNT, 0);
                m_i4WaitVDNum = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1;
                prepareCapParams(MFALSE,MFALSE);
                AE_LOG("[doAFAE] AE Stable\n");
            }
            m_i4WaitVDNum++;
            AE_LOG("[doAFAE] AF Mode SensorDev:%d Shutter:%-5d Sensor gain:%-5d Isp gain:%-4d ISO:%-4d flare gain/offset:%d/%d m_i4WaitVDNum:%d\n", m_eSensorDev,
                m_rAEOutput.rAFMode.u4Eposuretime, m_rAEOutput.rAFMode.u4AfeGain, m_rAEOutput.rAFMode.u4IspGain, m_rAEOutput.rAFMode.u4RealISO,
                m_rAEOutput.rAFMode.i2FlareGain, m_rAEOutput.rAFMode.i2FlareOffset, m_i4WaitVDNum);
            AE_LOG("[doAFAE] Capture SensorDev:%d Shutter:%-5d Sensor gain:%-5d Isp gain:%-4d ISO:%-4d flare gain/offset:%d/%d\n", m_eSensorDev,
                m_rAEOutput.rCaptureMode[0].u4Eposuretime, m_rAEOutput.rCaptureMode[0].u4AfeGain, m_rAEOutput.rCaptureMode[0].u4IspGain, m_rAEOutput.rCaptureMode[0].u4RealISO,
                m_rAEOutput.rCaptureMode[0].i2FlareGain, m_rAEOutput.rCaptureMode[0].i2FlareOffset);
            break;
        case AE_PRE_CAPTURE_STATE:
            m_bAEStable = MTRUE;
            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULE_STABLE, m_bAEStable);
            prepareCapParams(MFALSE,MFALSE);
            //auto test
            m_pIAeTest->recordAvgY(IAeTest::AE_AT_TYPE_Pre2Cap, m_pIAeAlgo->getBrightnessAverageValue());
            AE_LOG("[doPreCapAE] State:%d SensorDev:%d AE is stable\n", eNewAEState, m_eSensorDev);
            AE_LOG("[doPreCapAE] Preview Exp mode:%d Shutter:%-5d Sensor gain:%-5d Isp gain:%-4d ISO:%-4d flare gain/offest:%d/%d\n",
                m_rAEOutput.rPreviewMode.u4ExposureMode, m_rAEOutput.rPreviewMode.u4Eposuretime,
                m_rAEOutput.rPreviewMode.u4AfeGain, m_rAEOutput.rPreviewMode.u4IspGain,
                m_rAEOutput.rPreviewMode.u4RealISO, m_rAEOutput.rPreviewMode.i2FlareGain, m_rAEOutput.rPreviewMode.i2FlareOffset);
            AE_LOG("[doPreCapAE] AF Mode Exp mode:%d Shutter:%-5d Sensor gain:%-5d Isp gain:%-4d ISO:%-4d flare gain/offest:%d/%d\n",
                m_rAEOutput.rAFMode.u4ExposureMode, m_rAEOutput.rAFMode.u4Eposuretime,
                m_rAEOutput.rAFMode.u4AfeGain, m_rAEOutput.rAFMode.u4IspGain,
                m_rAEOutput.rAFMode.u4RealISO, m_rAEOutput.rAFMode.i2FlareGain, m_rAEOutput.rAFMode.i2FlareOffset);
            AE_LOG("[doPreCapAE] Capture Exp mode:%d Shutter:%-5d Sensor gain:%-5d Isp gain:%-4d ISO:%-4d flare gain/offest:%d/%d\n",
                m_rAEOutput.rCaptureMode[0].u4ExposureMode, m_rAEOutput.rCaptureMode[0].u4Eposuretime,
                m_rAEOutput.rCaptureMode[0].u4AfeGain, m_rAEOutput.rCaptureMode[0].u4IspGain,
                m_rAEOutput.rCaptureMode[0].u4RealISO, m_rAEOutput.rCaptureMode[0].i2FlareGain, m_rAEOutput.rCaptureMode[0].i2FlareOffset);
            updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
            break;
        case AE_CAPTURE_STATE:
            if (mCaptureMode.u4ExposureMode == eAE_EXPO_TIME)
            {
                AAASensorMgr::getInstance().setSensorExpTime((ESensorDev_T)m_eSensorDev, mCaptureMode.u4Eposuretime);
                m_pIAeSettingCPU->updateSensorGain((MVOID*) &mCaptureMode.u4AfeGain);
            }
            else
            {
                AAASensorMgr::getInstance().setCaptureLineBaseParams((ESensorDev_T)m_eSensorDev, mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain);
            }
            m_u4VsyncCnt = m_i4SensorCaptureDelayFrame;
            AE_LOG("[doCapAE] SensorDev:%d Exp Mode:%d Capture Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d flare gain/offset:%d/%d\n", m_eSensorDev, mCaptureMode.u4ExposureMode, mCaptureMode.u4Eposuretime,
                mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u4RealISO, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset);

            // Update to isp tuning
            updateAEInfo2ISP(AE_CAPTURE_STATE, mCaptureMode.u4IspGain);
            break;
        case AE_POST_CAPTURE_STATE:
            AE_LOG("[doPostCapAE] SensorDev:%d Capture Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d frame rate:%d flare gain/offset:%d/%d\n", m_eSensorDev, mCaptureMode.u4Eposuretime,
                mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, mCaptureMode.u4RealISO, mCaptureMode.u2FrameRate, mCaptureMode.i2FlareGain, mCaptureMode.i2FlareOffset);

            updateAEInfo2ISP(AE_POST_CAPTURE_STATE, mCaptureMode.u4IspGain);
            break;
        case AE_SENSOR_PER_FRAME_CAPTURE_STATE:
        case AE_SENSOR_PER_FRAME_STATE:

            if (m_SensorQueueCtrl.uInputIndex != m_SensorQueueCtrl.uOutputIndex)
            {
                // frame rate control
                m_u4UpdateFrameRate_x10 = (MFLOAT)10000000.000 / m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4FrameDuration;
                AE_LOG_IF(m_3ALogEnable, "[%s()] m_u4UpdateFrameRate_x10:%6.3f\n", __FUNCTION__, m_u4UpdateFrameRate_x10);

                if (m_u4UpdateFrameRate_x10 > (MUINT32)m_i4AEMaxFps)
                {
                    AE_LOG("[AE_SENSOR_PER_FRAME_STATE] Frame rate too large:%6.3f->%d\n", m_u4UpdateFrameRate_x10, m_i4AEMaxFps);
                    m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
                }

                m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
                if (m_i4ShutterDelayFrames == m_i4SensorGainDelayFrames)
                {
                    m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
                }

                m_u4UpdateShutterValue = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4ExposureTime;
                u4ISOValue = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity;
                // check if input iso is lower than sensor min iso
                if (u4ISOValue < u4MinIsoGain)
                    u4ISOValue = u4MinIsoGain;
                u4ISPGain = 1024;
                if (m_p3ANVRAM != NULL)
                {
                    m_u4UpdateGainValue = (u4ISOValue * 1024) / u4MinIsoGain;
                    if (m_u4UpdateGainValue > u4MaxGain)
                    {
                        u4ISPGain = 1024 * m_u4UpdateGainValue / u4MaxGain;
                        m_u4UpdateGainValue = u4MaxGain;
                    }
                }
                else
                {
                    AE_LOG("[%s()] NVRAM is NULL\n", __FUNCTION__);
                    m_u4UpdateGainValue = 1024;
                }
                // VHDR exposure setting manually
                if ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET))
                {
                    m_u4LEHDRshutter = m_u4UpdateShutterValue;
                    if ((m_u4VHDRratio == 0) || (m_u4VHDRratio == 9999))
                        m_u4SEHDRshutter = m_u4UpdateShutterValue;
                    else if (m_u4VHDRratio > 1000)
                    {
                        m_u4SEHDRshutter = m_u4UpdateShutterValue *1000 / m_u4VHDRratio;
                        m_u4UpdateGainValue2 = m_u4UpdateGainValue;
                    }
                    else if (m_u4VHDRratio > 100 && m_u4VHDRratio < 1000)
                    {
                        m_u4SEHDRshutter = m_u4UpdateShutterValue;
                        m_u4UpdateGainValue2 = m_u4UpdateGainValue *100 / m_u4VHDRratio;
                        if (m_u4UpdateGainValue2 < 1024)
                        {
                            m_u4UpdateGainValue2 = 1024;
                            m_u4SEHDRshutter = ((MUINT64)m_u4UpdateShutterValue *m_u4UpdateGainValue *100) / (m_u4UpdateGainValue2 *m_u4VHDRratio);
                        }
                    }
                    else
                    {
                        m_u4SEHDRshutter = m_u4UpdateShutterValue;
                    }
                    AE_LOG("[%s()] VHDR manual setting: LE/SE/afe/afe2/ISP/ratio:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4LEHDRshutter, m_u4SEHDRshutter, m_u4UpdateGainValue, m_u4UpdateGainValue2, u4ISPGain, m_u4VHDRratio);
                }
                updateAEInfo2ISP(AE_SENSOR_PER_FRAME_STATE, u4ISPGain);
                m_SensorQueueCtrl.uOutputIndex++;

                if (m_SensorQueueCtrl.uOutputIndex >= AE_SENSOR_MAX_QUEUE)
                {
                    m_SensorQueueCtrl.uOutputIndex = 0;
                }
            }
            else
            {
                AE_LOG("[%s()] VDNum:%d No data input, In/Out index:%d/%d\n", __FUNCTION__, m_i4WaitVDNum, m_SensorQueueCtrl.uInputIndex, m_SensorQueueCtrl.uOutputIndex);
            }

            break;
        case AE_TWO_SHUTTER_CAPTURE_STATE:
            if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                // MVHDR 3-expo
                AAASensorMgr::getInstance().set3ShutterControl((ESensorDev_T)m_eSensorDev, m_strIVHDRCaptureSetting.u4LEExpTimeInUS, m_strIVHDRCaptureSetting.u4MEExpTimeInUS, m_strIVHDRCaptureSetting.u4SEExpTimeInUS);
                AAASensorMgr::getInstance().setSensorHDRTriGain((ESensorDev_T)m_eSensorDev, m_strIVHDRCaptureSetting.u4LESensorGain, m_strIVHDRCaptureSetting.u4MESensorGain, m_strIVHDRCaptureSetting.u4SESensorGain);
                AAASensorMgr::getInstance().setSensorATR((ESensorDev_T)m_eSensorDev, m_u4veLimitGain, m_u4veLTCRate);
            }
            else
            {
                AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, m_strIVHDRCaptureSetting.u4LEExpTimeInUS, m_strIVHDRCaptureSetting.u4SEExpTimeInUS, (m_BVvalue + 50));
                if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
                {
                    AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, m_strIVHDRCaptureSetting.u4LESensorGain, m_strIVHDRCaptureSetting.u4SESensorGain);
                }
                else
                {
                    AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, m_strIVHDRCaptureSetting.u4LESensorGain);
                }
            }
            if (bUpdateSensorAWBGain)
            {
                strSensorAWBGain rSensorAWBGain;
                AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
                rSensorAWBGain.u4R = rCurrentIspAWBGain.i4R;
                rSensorAWBGain.u4GR = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4GB = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4B = rCurrentIspAWBGain.i4B;

                AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)m_eSensorDev, &rSensorAWBGain);
            }
            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth;
            m_u4StableCnt = 0;
            updateAEInfo2ISP(AE_TWO_SHUTTER_CAPTURE_STATE, m_strIVHDRCaptureSetting.u4SEISPGain);

            AE_LOG("[%s()] [CAPTURE] i4SensorDev:%d Delay %d/%d/%d LE(%d, %d, %d) SE(%d, %d, %d) ME(%d, %d, %d) Flare gain:%d offset:%d State:%d FrameRate(Min/Max):%d %d AETargetMode:%d veLimitGain: %d veLTCRate: %d\n", __FUNCTION__, m_eSensorDev,
                m_i4ShutterDelayFrames,
                m_i4SensorGainDelayFrames,
                m_i4IspGainDelayFrames,
                m_strIVHDRCaptureSetting.u4LEExpTimeInUS,
                m_strIVHDRCaptureSetting.u4LESensorGain,
                m_strIVHDRCaptureSetting.u4LEISPGain,
                m_strIVHDRCaptureSetting.u4SEExpTimeInUS,
                m_strIVHDRCaptureSetting.u4SESensorGain,
                m_strIVHDRCaptureSetting.u4SEISPGain,
                m_strIVHDRCaptureSetting.u4MEExpTimeInUS,
                m_strIVHDRCaptureSetting.u4MESensorGain,
                m_strIVHDRCaptureSetting.u4MEISPGain,
                m_rAEOutput.rPreviewMode.i2FlareGain,
                m_rAEOutput.rPreviewMode.i2FlareOffset,
                m_AEState, m_i4AEMinFps, m_i4AEMaxFps, m_eAETargetMode,
                m_u4veLimitGain, m_u4veLTCRate);
            break;
        case AE_TWO_SHUTTER_INIT_STATE:
            if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                // MVHDR 3-expo
                AAASensorMgr::getInstance().set3ShutterControl((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LEExpTimeInUS, m_strIVHDROutputSetting.u4MEExpTimeInUS, m_strIVHDROutputSetting.u4SEExpTimeInUS);
                AAASensorMgr::getInstance().setSensorHDRTriGain((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LESensorGain, m_strIVHDROutputSetting.u4MESensorGain, m_strIVHDROutputSetting.u4SESensorGain);
                AAASensorMgr::getInstance().setSensorATR((ESensorDev_T)m_eSensorDev, m_u4veLimitGain, m_u4veLTCRate);
            }
            else
            {
                AAASensorMgr::getInstance().set2ShutterControl((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LEExpTimeInUS, m_strIVHDROutputSetting.u4SEExpTimeInUS, (m_BVvalue + 50));
                if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
                {
                    AAASensorMgr::getInstance().setSensorDualGain((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LESensorGain, m_strIVHDROutputSetting.u4SESensorGain);
                }
                else
                {
                    AAASensorMgr::getInstance().setSensorGain((ESensorDev_T)m_eSensorDev, m_strIVHDROutputSetting.u4LESensorGain);
                }
            }
            if (bUpdateSensorAWBGain)
            {
                strSensorAWBGain rSensorAWBGain;
                AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();
                rSensorAWBGain.u4R = rCurrentIspAWBGain.i4R;
                rSensorAWBGain.u4GR = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4GB = rCurrentIspAWBGain.i4G;
                rSensorAWBGain.u4B = rCurrentIspAWBGain.i4B;

                AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)m_eSensorDev, &rSensorAWBGain);
            }
            m_bPerframeAEFlag = m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameHDRAESmooth;
            m_u4StableCnt = 0;
            updateAEInfo2ISP(AE_TWO_SHUTTER_INIT_STATE, m_strIVHDROutputSetting.u4SEISPGain);

            AE_LOG("[%s()] [INIT] i4SensorDev:%d Delay %d/%d/%d LE(%d, %d, %d) SE(%d, %d, %d) ME(%d, %d, %d) Flare gain:%d offset:%d State:%d FrameRate(Min/Max):%d %d AETargetMode:%d veLimitGain: %d veLTCRate: %d\n", __FUNCTION__, m_eSensorDev,
                m_i4ShutterDelayFrames,
                m_i4SensorGainDelayFrames,
                m_i4IspGainDelayFrames,
                m_strIVHDROutputSetting.u4LEExpTimeInUS,
                m_strIVHDROutputSetting.u4LESensorGain,
                m_strIVHDROutputSetting.u4LEISPGain,
                m_strIVHDROutputSetting.u4SEExpTimeInUS,
                m_strIVHDROutputSetting.u4SESensorGain,
                m_strIVHDROutputSetting.u4SEISPGain,
                m_strIVHDROutputSetting.u4MEExpTimeInUS,
                m_strIVHDROutputSetting.u4MESensorGain,
                m_strIVHDROutputSetting.u4MEISPGain,
                m_rAEOutput.rPreviewMode.i2FlareGain,
                m_rAEOutput.rPreviewMode.i2FlareOffset,
                m_AEState, m_i4AEMinFps, m_i4AEMaxFps, m_eAETargetMode,
                m_u4veLimitGain, m_u4veLTCRate);
            break;
        case AE_TWO_SHUTTER_FRAME_STATE:

            if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                // MVHDR 3-expo
                AAASensorMgr::getInstance().setSensorATR((ESensorDev_T)m_eSensorDev, m_u4veLimitGain, m_u4veLTCRate);
            }

            if ((m_i4AEMinFps == m_i4AEMaxFps) || (m_i4AEMaxFps <= LIB3A_AE_FRAMERATE_MODE_30FPS))
            {
                // frame rate control
                m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_FRAMERATE, MTRUE);
                m_u4UpdateFrameRate_x10 = m_i4AEMaxFps;
            }

            if (m_i4WaitVDNum == m_i4ShutterDelayFrames)
            {
                m_u4SEHDRshutter = m_strIVHDROutputSetting.u4SEExpTimeInUS;
                m_u4LEHDRshutter = m_strIVHDROutputSetting.u4LEExpTimeInUS;
                m_u4MEHDRshutter = m_strIVHDROutputSetting.u4MEExpTimeInUS;
                m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MTRUE);
            }

            if (m_i4WaitVDNum == m_i4SensorGainDelayFrames)
            {
                m_u4UpdateGainValue = m_strIVHDROutputSetting.u4LESensorGain;
                m_u4UpdateGainValue2 = m_strIVHDROutputSetting.u4SESensorGain;
                m_u4UpdateGainValue3 = m_strIVHDROutputSetting.u4MESensorGain;
                m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MTRUE);
            }

            if (m_i4WaitVDNum == m_i4IspGainDelayFrames)
            {
                // restart
                m_u4StableCnt = 0;
                if (m_bAEStable == MFALSE)
                {
                    m_u4StableCnt = 0;
                }
                else
                {
                    if (m_u4StableCnt < 0xFFFF)
                    {
                        m_u4StableCnt++;
                    }
                }
                updateAEInfo2ISP(AE_TWO_SHUTTER_FRAME_STATE, m_strIVHDROutputSetting.u4SEISPGain);
            }
            AE_LOG("[%s()] [FRAME] i4SensorDev:%d VDNum %d Delay %d/%d/%d LE(%d, %d, %d) SE(%d, %d, %d) ME(%d, %d, %d) Flare gain:%d offset:%d State:%d FrameRate(Min/Max): %d %d AETargetMode:%d veLimitGain: %d veLTCRate: %d\n", __FUNCTION__, m_eSensorDev, m_i4WaitVDNum,
                m_i4ShutterDelayFrames,
                m_i4SensorGainDelayFrames,
                m_i4IspGainDelayFrames,
                m_strIVHDROutputSetting.u4LEExpTimeInUS,
                m_strIVHDROutputSetting.u4LESensorGain,
                m_strIVHDROutputSetting.u4LEISPGain,
                m_strIVHDROutputSetting.u4SEExpTimeInUS,
                m_strIVHDROutputSetting.u4SESensorGain,
                m_strIVHDROutputSetting.u4SEISPGain,
                m_strIVHDROutputSetting.u4MEExpTimeInUS,
                m_strIVHDROutputSetting.u4MESensorGain,
                m_strIVHDROutputSetting.u4MEISPGain,
                m_rAEOutput.rPreviewMode.i2FlareGain,
                m_rAEOutput.rPreviewMode.i2FlareOffset,
                m_AEState, m_i4AEMinFps, m_i4AEMaxFps, m_eAETargetMode,
                m_u4veLimitGain, m_u4veLTCRate);
            m_i4WaitVDNum++;
            break;
        case AE_AF_RESTORE_STATE:
            updateAEInfo2ISP(AE_AF_RESTORE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
            break;
        default:
            break;
    }
    return S_AE_OK;
}

MRESULT AeMgr::UpdateFlare2ISP()
{
    if (m_eAEMode == LIB3A_AE_MODE_OFF)
    {
        if (m_SensorQueueCtrl.uOutputIndex >= AE_SENSOR_MAX_QUEUE)
        {
            m_SensorQueueCtrl.uOutputIndex = 0;
        }
        MUINT32 u4MinIsoGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 u4MaxGain = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MaxGain;
        MUINT32 u4ISPGain = 1024;
        MUINT32 u4ISOValue = m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uOutputIndex].u4Sensitivity;
        if (m_p3ANVRAM != NULL)
        {
            m_u4UpdateGainValue = (u4ISOValue * 1024) / u4MinIsoGain;
            if (m_u4UpdateGainValue > u4MaxGain)
            {
                u4ISPGain = 1024 * m_u4UpdateGainValue / u4MaxGain;
                m_u4UpdateGainValue = u4MaxGain;
            }
        }
        else
        {
            MY_LOG("[%s] NVRAM is NULL\n", __FUNCTION__);
            m_u4UpdateGainValue = 1024;
        }
        updateAEInfo2ISP(AE_SENSOR_PER_FRAME_STATE, u4ISPGain);
    }
    else
    {
        updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, 0x00);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 AeMgr::getAEFaceDiffIndex()
{
    AE_LOG_IF(m_3ALogEnable, "[%s] m_i2AEFaceDiffIndex =%d", __FUNCTION__, m_i2AEFaceDiffIndex);
    return m_i2AEFaceDiffIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorDelayInfo(MINT32 * i4SutterDelay, MINT32 * i4SensorGainDelay, MINT32 * i4IspGainDelay)
{
    m_i4ShutterDelayFrames = *i4IspGainDelay - *i4SutterDelay;
    m_i4SensorGainDelayFrames = *i4IspGainDelay - *i4SensorGainDelay;
    if (*i4IspGainDelay > 2)
    {
        m_i4IspGainDelayFrames = *i4IspGainDelay - ISP_GAIN_DELAY_OFFSET; // for CQ0 1 delay frame
    }
    else
    {
        m_i4IspGainDelayFrames = 0; // for CQ0 1 delay frame
    }

    AE_LOG("[%s()] i4SensorDev:%d m_i4ShutterDelayFrames:%d m_i4SensorGainDelayFrames:%d Isp gain:%d %d %d %d\n", __FUNCTION__, m_eSensorDev,
        m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, *i4SutterDelay, *i4SensorGainDelay, *i4IspGainDelay);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getBrightnessValue(MBOOL * bFrameUpdate, MINT32 * i4Yvalue)
{
    MUINT8 uYvalue[5][5];

    if (m_bFrameUpdate == MTRUE)
    {
        if (m_pIAeAlgo != NULL)
        {
            m_pIAeAlgo->getAEBlockYvalues(&uYvalue[0][0], AE_BLOCK_NO *AE_BLOCK_NO);
            * i4Yvalue = uYvalue[2][2];
        }
        else
        {
            AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
        }

        *bFrameUpdate = m_bFrameUpdate;
        m_bFrameUpdate = MFALSE;
    }
    else
    {
        *i4Yvalue = 0;
        *bFrameUpdate = MFALSE;
    }

    AE_LOG("[%s()] i4SensorDev:%d Yvalue:%d FrameUpdate:%d\n", __FUNCTION__, m_eSensorDev, * i4Yvalue, *bFrameUpdate);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::SetAETargetMode(eAETargetMODE eAETargetMode)
{
    if (m_eAETargetMode != eAETargetMode)
    {
        AE_LOG("[%s()] i4SensorDev:%d TargetMode:%d %d\n", __FUNCTION__, m_eSensorDev, m_eAETargetMode, eAETargetMode);
        m_eAETargetMode = eAETargetMode;
        if (m_eAETargetMode == AE_MODE_NORMAL)
        {
            m_strIVHDROutputSetting.u4LE_SERatio_x100 = 100;
            m_u4EISExpRatio = 100;
        }
        m_bVHDRChangeFlag = MTRUE;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setEMVHDRratio(MUINT32 u4ratio)
{
    if (m_u4VHDRratio != u4ratio)
    {
        AE_LOG("[%s()] i4SensorDev:%d m_u4VHDRratio:%d -> %d\n", __FUNCTION__, m_eSensorDev, m_u4VHDRratio, u4ratio);
        m_u4VHDRratio = u4ratio;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAESensorActiveCycle(MINT32 * i4ActiveCycle)
{
    * i4ActiveCycle = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1;
    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d Cycle:%d\n", __FUNCTION__, m_eSensorDev, *i4ActiveCycle);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setVideoDynamicFrameRate(MBOOL bVdoDynamicFps)
{
    if (m_bVideoDynamic != bVdoDynamicFps)
    {
        AE_LOG("[%s()] i4SensorDev:%d TargetMode:%d %d\n", __FUNCTION__, m_eSensorDev, m_bVideoDynamic, bVdoDynamicFps);
        m_bVideoDynamic = bVdoDynamicFps;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::modifyAEPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    if ((m_rAEPLineLimitation.bEnable != bEnable) || (m_rAEPLineLimitation.bEquivalent != bEquivalent) || (m_rAEPLineLimitation.u4IncreaseISO_x100 != u4IncreaseISO_x100) || (m_rAEPLineLimitation.u4IncreaseShutter_x100 != u4IncreaseShutter_x100))
    {
        AE_LOG("[%s()] i4SensorDev:%d Enable:%d %d Equivalent:%d %d IncreaseISO:%d %d IncreaseShutter: %d %d\n", __FUNCTION__, m_eSensorDev,
            m_rAEPLineLimitation.bEnable, bEnable,
            m_rAEPLineLimitation.bEquivalent, bEquivalent,
            m_rAEPLineLimitation.u4IncreaseISO_x100, u4IncreaseISO_x100,
            m_rAEPLineLimitation.u4IncreaseShutter_x100, u4IncreaseShutter_x100);
        m_rAEPLineLimitation.bEnable = bEnable;
        m_rAEPLineLimitation.bEquivalent = bEquivalent;
        m_rAEPLineLimitation.u4IncreaseISO_x100 = u4IncreaseISO_x100;
        m_rAEPLineLimitation.u4IncreaseShutter_x100 = u4IncreaseShutter_x100;
    }
    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MRESULT AeMgr::ModifyCaptureParamsBySensorMode(MINT32 i4newSensorMode, MINT32 i4oldSensorMode)
{
    //MINT32 err; /* fix build warning */
    //MUINT32 u4AFEGain = 0, u4IspGain = 1024, u4BinningRatio = 1; /* fix build warning */
    MUINT32 u4BinningRatio = 1;
    MUINT32 u4NewSensitivityRatio = 1024, u4OldSensitivityRatio = 1024, u4SensitivityRatio = 1024;
    //strAEInput rAEInput; /* fix build warning */
    //strAEOutput rAEOutput; /* fix build warning */
    //MBOOL bStrobeOn; /* fix build warning */

    if (m_p3ANVRAM == NULL)
    {
        AE_LOG("[%s()] i4SensorDev:%d Nvram 3A pointer is NULL\n", __FUNCTION__, m_eSensorDev);
        return S_AE_OK;
    }

    // Update new sensor mode senstivity ratio
    switch (i4newSensorMode)
    {
        case ESensorMode_Capture:
            u4NewSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Cap2PreRatio;
            break;
        case ESensorMode_Video:
            u4NewSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video2PreRatio;
            break;
        case ESensorMode_SlimVideo1:
            u4NewSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video12PreRatio;
            break;
        case ESensorMode_SlimVideo2:
            u4NewSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video22PreRatio;
            break;
        case ESensorMode_Preview:
        default:
            u4NewSensitivityRatio = 1024;
            break;
    }

    // Update old sensor mode senstivity ratio
    switch (i4oldSensorMode)
    {
        case ESensorMode_Capture:
            u4OldSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Cap2PreRatio;
            break;
        case ESensorMode_Video:
            u4OldSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video2PreRatio;
            break;
        case ESensorMode_SlimVideo1:
            u4OldSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video12PreRatio;
            break;
        case ESensorMode_SlimVideo2:
            u4OldSensitivityRatio = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4Video22PreRatio;
            break;
        case ESensorMode_Preview:
        default:
            u4OldSensitivityRatio = 1024;
            break;
    }

    if (u4OldSensitivityRatio != 0)
    {
        u4SensitivityRatio = 1024 * u4OldSensitivityRatio / u4NewSensitivityRatio;
    }
    else
    {
        u4SensitivityRatio = 1024;
        AE_LOG("[%s()] i4SensorDev:%d u4OldSensitivityRatio is zero\n", __FUNCTION__, m_eSensorDev);
    }

    if (u4SensitivityRatio <= 300)
    {
        u4BinningRatio = 4;
    }
    else if (u4SensitivityRatio <= 450)
    {
        u4BinningRatio = 3;
    }
    else if (u4SensitivityRatio <= 768)
    {
        u4BinningRatio = 2;
    }
    else
    {
        u4BinningRatio = 1;
    }

    if (i4newSensorMode != ESensorMode_Capture)
    {
        MUINT32 i4Totalgain = mCaptureMode.u4AfeGain * mCaptureMode.u4IspGain / u4BinningRatio / 1024;
        if (i4Totalgain > m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain)
        {
            mCaptureMode.u4AfeGain = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
            mCaptureMode.u4IspGain = i4Totalgain * 1024 / mCaptureMode.u4AfeGain;
        }
        else if (i4Totalgain > 1024)
        {
            mCaptureMode.u4AfeGain = i4Totalgain;
            mCaptureMode.u4IspGain = 1024;
        }
        else
        {
            mCaptureMode.u4Eposuretime = mCaptureMode.u4Eposuretime / u4BinningRatio;
        }
        mCaptureMode.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
        mCaptureMode.i2FlareGain = m_rAEOutput.rPreviewMode.i2FlareGain;
        AE_LOG("[%s()] i4SensorDev:%d New Capture:%d/%d/%d Binning:%d Flare:%d %d\n", __FUNCTION__, m_eSensorDev, mCaptureMode.u4Eposuretime, mCaptureMode.u4AfeGain, mCaptureMode.u4IspGain, u4BinningRatio,
            mCaptureMode.i2FlareOffset, mCaptureMode.i2FlareGain);
    }
    return S_AE_OK;
}

MRESULT AeMgr::getAESGG1Gain(MUINT32 *pSGG1Gain)
{
    *pSGG1Gain = m_u4AFSGG1Gain;
    return S_AE_OK;
}

MRESULT AeMgr::switchCapureDiffEVState(MINT8 iDiffEV, strAEOutput &aeoutput)
{
    if (m_bMVHDR3FlashUpdate)
    {
        // customize for 4 -cell mvhdr flashlight
        aeoutput.EvSetting.u4Eposuretime = mCaptureMode.u4Eposuretime;
        aeoutput.EvSetting.u4AfeGain = mCaptureMode.u4AfeGain;
        aeoutput.EvSetting.u4IspGain = mCaptureMode.u4IspGain;
        aeoutput.u4ISO = mCaptureMode.u4RealISO;
        AE_LOG("[%s()] MVHDR3Expo: updated by flash, u4Eposuretime/u4AfeGain/u4IspGain/u4ISO = %d/%d/%d/%d\n", __FUNCTION__,
            aeoutput.EvSetting.u4Eposuretime, aeoutput.EvSetting.u4AfeGain, aeoutput.EvSetting.u4IspGain, aeoutput.u4ISO);
        return S_AE_OK;
    }

    if (m_pIAeAlgo != NULL)
    {
        prepareCapParams(MFALSE,MFALSE);
        m_pIAeAlgo->switchCapureDiffEVState(&aeoutput, iDiffEV);
        AE_LOG("[%s()] i4SensorDev:%d iDiffEV:%d\n", __FUNCTION__, m_eSensorDev, iDiffEV);
    }
    else
    {
        AE_LOG("[%s()] The AE algo class is NULL, i4SensorDev:%d line:%d\n", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEOneShotControl(MBOOL bAEControl)
{
    if (m_bAEOneShotControl != bAEControl)
    {
        AE_LOG("[%s()] i4SensorDev:%d One Shot control old:%d new:%d\n", __FUNCTION__, m_eSensorDev, m_bAEOneShotControl, bAEControl);
        m_bAEOneShotControl = bAEControl;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsNeedUpdateSensor()
{
    return (m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_NEEDUPDATED));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorbyI2CBufferMode()
{
    //MINT32 err = S_AE_OK; /* fix build warning */
    // CPU sensor setting (frame rate / shutter / sensor gain)
    AE_SETTING_CONTROL_INFO_T rAESetControlInfo;
    prepareSetControlParam(&rAESetControlInfo);
    m_pIAeSettingCPU->updateSensorFast((MVOID *) &rAESetControlInfo);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorbyI2C()
{
    //MINT32 err = S_AE_OK; /* fix build warning */
    // Auto test
    if (m_pIAeTest->IsPerframeCtrl())
    {
        AaaTimer localTimer("startSensorPerFrameControl", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
        m_pIAeTest->PerframeCtrl(m_pIAeAlgo->getBrightnessAverageValue(), (m_i4ShutterDelayFrames == m_i4SensorGainDelayFrames), m_eSensorMode, m_pAEPlineTable, &m_PreviewTableF, m_u4IndexFMax);
        localTimer.End();
        return S_AE_OK;
    }
    // Check if Pass CPU sensor driver setting
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_bCCUAEFlag && m_bIsLongExpControlbyCCU && (m_u4HDRcountforrevert == 0))
    {
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_FRAMERATE, MFALSE);
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SHUTTER, MFALSE);
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAIN, MFALSE);
        AE_LOG_IF(m_3ALogEnable, "[%s] Pass CPU I2C setting \n", __FUNCTION__);
    }
    // CPU sensor setting (frame rate / shutter / sensor gain)
    AE_SETTING_CONTROL_INFO_T rAESetControlInfo;
    prepareSetControlParam(&rAESetControlInfo);
    AAA_TRACE_D("updateSensorbyI2C");
    m_pIAeSettingCPU->updateSensor((MVOID *) &rAESetControlInfo);
    AAA_TRACE_END_D;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateAEInfo2ISP(AE_STATE_T eNewAEState, MUINT32 u4ISPGain)
{
    AE_INFO_T rAEInfo2ISP;
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseGain = 1024;
    MUINT32 u4PreviewBaseISO = 100;
    MINT16 i2FlareGain;
    MFLOAT u2FrameRate = 300;
    memset(&rAEInfo2ISP, 0, sizeof(AE_INFO_T));

    // Update to isp tuning
    if (m_pIAeAlgo != NULL)
    {
        MINT16 i2FlareOffset;
        MINT16 i2FlareGain;
        if ((eNewAEState != AE_CAPTURE_STATE) && (m_u4HDRcountforrevert == 0))
            m_bForceUpdateAEInfoByCCU = MFALSE;

        if ((m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)) || m_bForceUpdateAEInfoByCCU)
        {
            m_pIAeFlowCCU->getAEInfoForISP(rAEInfo2ISP);
            AE_LOG_IF(m_3ALogEnable, "[%s()] m_bForceUpdateAEInfoByCCU:%d\n", __FUNCTION__, m_bForceUpdateAEInfoByCCU);
        }
        else
        {
            m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP, LIB3A_SENSOR_MODE_PRVIEW);
            m_pIAeAlgo->getPreviewFlare(&i2FlareOffset, &i2FlareGain);
            m_rAEOutput.rPreviewMode.i2FlareOffset = i2FlareOffset;
            m_rAEOutput.rPreviewMode.i2FlareGain = i2FlareGain;
            AE_LOG_IF(m_3ALogEnable, "[%s()] Preview Flare Offset:%d Gain:%d\n", __FUNCTION__, m_rAEOutput.rPreviewMode.i2FlareOffset, m_rAEOutput.rPreviewMode.i2FlareGain);
        }

        if (m_bNeedApplyBackupAEInfo)
        {
            memcpy(&rAEInfo2ISP, &rAEInfo2ISPBack, sizeof(AE_INFO_T));
            AE_LOG_IF(m_3ALogEnable, "[%s()] Remosaic capture, use last preview AE info\n", __FUNCTION__);
        }
        else if (((eNewAEState == AE_AUTO_FRAMERATE_STATE) || (eNewAEState == AE_TWO_SHUTTER_FRAME_STATE)) && (m_u4HDRcountforrevert == 0))
        {
            memcpy(&rAEInfo2ISPBack, &rAEInfo2ISP, sizeof(AE_INFO_T));
            AE_LOG_IF(m_3ALogEnable, "[%s()] Backup preview AE info\n", __FUNCTION__);
        }

        //For update both master & slave cams' LV
        if (m_bStereoManualPline)
        {
            m_BVvalue = rAEInfo2ISP.i4LightValue_x10 - 50;
            m_AOECompBVvalue = rAEInfo2ISP.i4RealLightValue_x10 - 50; //real BV value
            AE_LOG_IF(m_3ALogEnable, "[%s()] m_AOECompBVvalue:%d\n", __FUNCTION__, m_AOECompBVvalue);
        }
    }
    else
    {
        AE_LOG("[%s()] AE algo class is NULL\n", __FUNCTION__);
    }
    // Update from NVram
    if (m_p3ANVRAM != NULL)
    {
        u4PreviewBaseISO = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    }
    else
    {
        AE_LOG("[%s()] NVRAM data is NULL\n", __FUNCTION__);
    }
    rAEInfo2ISP.i4GammaIdx = m_i4GammaIdx;
    rAEInfo2ISP.i4LESE_Ratio = m_i4LESE_Ratio;
    rAEInfo2ISP.i4HdrSeg = m_i4HdrSeg;
    rAEInfo2ISP.i4HdrTurn = m_i4HdrTurn;
    rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
    rAEInfo2ISP.u4MaxISO = m_u4MaxISO * m_rAEPLineLimitation.u4IncreaseISO_x100 / 100;
    rAEInfo2ISP.u4AEStableCnt = m_u4StableCnt;
    rAEInfo2ISP.u4EVRatio = m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp];
    rAEInfo2ISP.bAEStable = m_bAEStable;
    rAEInfo2ISP.bAELock = m_bAPAELock;
    rAEInfo2ISP.bAELimiter = m_bAElimitor;
    rAEInfo2ISP.bDisableOBC = (m_bIsLongExpControlbyCCU && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && (m_u4HDRcountforrevert == 0));
    rAEInfo2ISP.u4EISExpRatio = m_u4EISExpRatio;
    rAEInfo2ISP.i4AEComp = (MINT32)10*m_i4EVIndex*m_fEVCompStep;
    if (eNewAEState == AE_INIT_STATE)
    {
        rAEInfo2ISP.bDisableOBC = 1;
        AE_LOG("[%s()] force set DisableOBC = 1 at AE_INIT_STATE\n", __FUNCTION__);
    }

    switch (eNewAEState)
    {
        case AE_INIT_STATE:
        case AE_REINIT_STATE:
        case AE_AUTO_FRAMERATE_STATE:
        case AE_MANUAL_FRAMERATE_STATE:
        case AE_AF_RESTORE_STATE:
            rAEInfo2ISP.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
            rAEInfo2ISP.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
            rAEInfo2ISP.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;

            if (u4ISPGain != 0x00)
            {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                AE_LOG_IF(m_3ALogEnable, "[%s()] Smooth ISP Gain:%d\n", __FUNCTION__, rAEInfo2ISP.u4IspGain);
            }
            else
            {
                rAEInfo2ISP.u4IspGain = m_rAEOutput.rPreviewMode.u4IspGain;
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain * rAEInfo2ISP.u4IspGain) >> 10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO * u4FinalGain) / u4PreviewBaseGain;
            u2FrameRate = m_rAEOutput.rPreviewMode.u2FrameRate;
            if (eNewAEState == AE_AUTO_FRAMERATE_STATE)
                u4FD_Prob = rAEInfo2ISP.u4FDProb;
            break;
        case AE_AF_STATE:
            rAEInfo2ISP.u4Eposuretime = m_rAEOutput.rAFMode.u4Eposuretime;
            rAEInfo2ISP.u4AfeGain = m_rAEOutput.rAFMode.u4AfeGain;
            rAEInfo2ISP.u4IspGain = m_rAEOutput.rAFMode.u4IspGain;
            rAEInfo2ISP.i2FlareOffset = m_rAEOutput.rAFMode.i2FlareOffset;
            rAEInfo2ISP.u4RealISOValue = m_rAEOutput.rAFMode.u4RealISO;
            u2FrameRate = m_rAEOutput.rAFMode.u2FrameRate;
            break;
        case AE_CAPTURE_STATE:
        case AE_POST_CAPTURE_STATE:
            if ((!m_bForceUpdateAEInfoByCCU) && (!m_bNeedApplyBackupAEInfo))
            {
                if (m_pIAeAlgo != NULL)
                {
                    m_pIAeAlgo->getAEInfoForISP(rAEInfo2ISP, LIB3A_SENSOR_MODE_CAPTURE);
                }
                else
                {
                    AE_LOG("[%s()] The AE algo class is NULL (updateAEInfo2ISP) \n", __FUNCTION__);
                }
            }
            rAEInfo2ISP.u4Eposuretime = mCaptureMode.u4Eposuretime;
            rAEInfo2ISP.i2FlareOffset = mCaptureMode.i2FlareOffset;
            rAEInfo2ISP.u4AfeGain = mCaptureMode.u4AfeGain;
            rAEInfo2ISP.u4IspGain = mCaptureMode.u4IspGain;
            u4FinalGain = (rAEInfo2ISP.u4AfeGain * rAEInfo2ISP.u4IspGain) >> 10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO * u4FinalGain) / u4PreviewBaseGain;
            u2FrameRate = mCaptureMode.u2FrameRate;
            rAEInfo2ISP.u4FDProb = u4FD_Prob;
            break;
        case AE_SENSOR_PER_FRAME_STATE :
            rAEInfo2ISP.u4Eposuretime = m_u4UpdateShutterValue;
            rAEInfo2ISP.u4AfeGain = m_u4UpdateGainValue;
            rAEInfo2ISP.i2FlareOffset = 0;
            if (m_bIsFlareInManual)
            {
                rAEInfo2ISP.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
            }

            if (u4ISPGain != 0x00)
            {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                if (m_pIAeAlgo != NULL)
                    AE_LOG_IF(m_3ALogEnable, "[%s()] Smooth ISP Gain:%d avgY:%d\n", __FUNCTION__, rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            }
            else
            {
                rAEInfo2ISP.u4IspGain = 1024;
                if (m_pIAeAlgo != NULL)
                    AE_LOG("[%s()] No Smooth ISP Gain:%d avgY:%d\n", __FUNCTION__, rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain * rAEInfo2ISP.u4IspGain) >> 10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO * u4FinalGain) / u4PreviewBaseGain;
            u2FrameRate = m_u4UpdateFrameRate_x10;
            break;
        case AE_TWO_SHUTTER_CAPTURE_STATE :
            rAEInfo2ISP.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
            rAEInfo2ISP.u4Eposuretime = m_strIVHDRCaptureSetting.u4LEExpTimeInUS;
            rAEInfo2ISP.u4AfeGain = m_strIVHDRCaptureSetting.u4LESensorGain;
            rAEInfo2ISP.u4IspGain = m_strIVHDRCaptureSetting.u4LEISPGain;
            rAEInfo2ISP.i4LESE_Ratio = m_strIVHDRCaptureSetting.u4LE_SERatio_x100;
            rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
            rAEInfo2ISP.i4HdrSeg = m_i4HdrSeg;
            rAEInfo2ISP.i4HdrTurn = m_i4HdrTurn;

            if (u4ISPGain != 0x00)
            {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                if (m_pIAeAlgo != NULL)
                    AE_LOG_IF(m_3ALogEnable, "[%s()] Smooth ISP Gain:%d avgY:%d\n", __FUNCTION__, rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain * rAEInfo2ISP.u4IspGain) >> 10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO * u4FinalGain) / u4PreviewBaseGain;
            u2FrameRate = m_u4UpdateFrameRate_x10;
            break;
        case AE_TWO_SHUTTER_INIT_STATE :
            rAEInfo2ISP.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
            rAEInfo2ISP.u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
            rAEInfo2ISP.u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
            rAEInfo2ISP.u4IspGain = m_strIVHDROutputSetting.u4LEISPGain;
            rAEInfo2ISP.i4LESE_Ratio = m_strIVHDROutputSetting.u4LE_SERatio_x100;
            rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
            rAEInfo2ISP.i4HdrSeg = m_i4HdrSeg;
            rAEInfo2ISP.i4HdrTurn = m_i4HdrTurn;

            if (u4ISPGain != 0x00)
            {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                if (m_pIAeAlgo != NULL)
                    AE_LOG_IF(m_3ALogEnable, "[%s()] Smooth ISP Gain:%d avgY:%d\n", __FUNCTION__, rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain * rAEInfo2ISP.u4IspGain) >> 10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO * u4FinalGain) / u4PreviewBaseGain;
            u2FrameRate = m_u4UpdateFrameRate_x10;
            break;
        case AE_TWO_SHUTTER_FRAME_STATE :
            rAEInfo2ISP.i2FlareOffset = m_rAEOutput.rPreviewMode.i2FlareOffset;
            rAEInfo2ISP.u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
            rAEInfo2ISP.u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
            rAEInfo2ISP.u4IspGain = m_strIVHDROutputSetting.u4LEISPGain;
            rAEInfo2ISP.i4LESE_Ratio = m_strIVHDROutputSetting.u4LE_SERatio_x100;
            rAEInfo2ISP.u4SWHDR_SE = m_u4SWHDR_SE;
            rAEInfo2ISP.i4HdrSeg = m_i4HdrSeg;
            rAEInfo2ISP.i4HdrTurn = m_i4HdrTurn;

            if (u4ISPGain != 0x00)
            {
                rAEInfo2ISP.u4IspGain = u4ISPGain;
                if (m_pIAeAlgo != NULL)
                    AE_LOG_IF(m_3ALogEnable, "[%s()] Smooth ISP Gain:%d avgY:%d\n", __FUNCTION__, rAEInfo2ISP.u4IspGain, m_pIAeAlgo->getBrightnessAverageValue());
            }

            u4FinalGain = (rAEInfo2ISP.u4AfeGain * rAEInfo2ISP.u4IspGain) >> 10;
            rAEInfo2ISP.u4RealISOValue = (u4PreviewBaseISO * u4FinalGain) / u4PreviewBaseGain;
            u2FrameRate = m_u4UpdateFrameRate_x10;
            break;
        case AE_PRE_CAPTURE_STATE:
        case AE_SENSOR_PER_FRAME_CAPTURE_STATE:
            AE_LOG("[%s()] Nothing to do State:%d\n", __FUNCTION__, eNewAEState);
            return S_AE_OK;
    }
    AE_LOG_IF(m_3ALogEnable, "[%s()] Frame rate:%6.3f", __FUNCTION__, u2FrameRate);

    if (u2FrameRate > m_i4AEMaxFps)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s()] Frame rate too large:%6.3f->%d\n", __FUNCTION__, u2FrameRate, m_i4AEMaxFps);
        u2FrameRate = m_i4AEMaxFps;
    }

    if (u2FrameRate != 0)
    {
        m_rSensorCurrentInfo.u8FrameDuration = 10005000000L / u2FrameRate;
    }
    else
    {
        m_rSensorCurrentInfo.u8FrameDuration = 33000000L;
        AE_LOG("[%s()] No Frame rate value, using default frame rate value (33ms)\n", __FUNCTION__);
    }

    m_rSensorCurrentInfo.u8ExposureTime = (MUINT64)rAEInfo2ISP.u4Eposuretime * 1000L;
    m_rSensorCurrentInfo.u4Sensitivity = rAEInfo2ISP.u4RealISOValue;
    i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - rAEInfo2ISP.i2FlareOffset);
    m_i4AEAutoFlickerMode = (MINT32)(m_eAEAutoFlickerMode);
    if (eNewAEState == AE_POST_CAPTURE_STATE)
    {
        i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - m_rAEOutput.rPreviewMode.i2FlareOffset);
        IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, g_rIsp >> 1);
        IspTuningMgr::getInstance().setIspFlareGainOffset((ESensorDev_T)m_eSensorDev, i2FlareGain, ( -1 *m_rAEOutput.rPreviewMode.i2FlareOffset));
        AE_LOG("[%s()] AE_POST_CAPTURE_STATE - ISP/FlareOffset:%d/%d\n", __FUNCTION__, g_rIsp, m_rAEOutput.rPreviewMode.i2FlareOffset);
    }
    else
    {
        IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)m_eSensorDev, MFALSE, rAEInfo2ISP.u4IspGain >> 1);
        IspTuningMgr::getInstance().setIspFlareGainOffset((ESensorDev_T)m_eSensorDev, i2FlareGain, ( -1 *rAEInfo2ISP.i2FlareOffset));
    }

    // Check AE Pline index reach to max index or not
    if (m_u4Index >= m_u4IndexMax)
    {
        m_u4MaxIdxStableCount++;
    }
    else
    {
        m_u4MaxIdxStableCount = 0;
    }

    if (m_u4MaxIdxStableCount > m_u4Idx1StableThres)
    {
        m_bIsMaxIndexStable = MTRUE;
    }
    else
    {
        m_bIsMaxIndexStable = MFALSE;
    }

    if (rAEInfo2ISP.u4RealISOValue >= m_u4ISOIdx1Value)
    {
        if (m_u4ISOIdx1StableCount < 2 *m_u4Idx1StableThres)
        {
            m_u4ISOIdx1StableCount++;
        }
    }
    else
    {
        if (m_u4ISOIdx1StableCount > 0)
        {
            m_u4ISOIdx1StableCount--;
        }
    }

    if (rAEInfo2ISP.u4RealISOValue >= m_u4ISOIdx2Value)
    {
        if (m_u4ISOIdx2StableCount < 2 *m_u4Idx2StableThres)
        {
            m_u4ISOIdx2StableCount++;
        }
    }
    else
    {
        if (m_u4ISOIdx2StableCount > 0)
        {
            m_u4ISOIdx2StableCount--;
        }
    }

    rAEInfo2ISP.bIsMaxIndexStable = m_bIsMaxIndexStable;
    rAEInfo2ISP.bIsndexSmallOffset = 0;
    rAEInfo2ISP.u4MaxIndexStableCount = m_u4MaxIdxStableCount;

    AAA_TRACE_D("AEInfo2CCU");
    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && (u4ISPGain != 0x00))
    {
        setAEInfo2CCU(rAEInfo2ISP.u4Eposuretime, rAEInfo2ISP.u4AfeGain, rAEInfo2ISP.u4IspGain, rAEInfo2ISP.u4RealISOValue, rAEInfo2ISP.u4AEidxCurrentF, rAEInfo2ISP.u4AEidxNextF, rAEInfo2ISP.i4DeltaIdx);
    }
    AAA_TRACE_END_D;
    // Stereo denoise OB2
    if (m_bStereoManualPline && m_u4StereoDenoiseMode)
    {
        MINT32 i4StereoDenoiserto = 1000;
        if (m_u4StereoDenoiseMode == 1)
            i4StereoDenoiserto = m_i4StereoDenoiserto[0];
        else if (m_u4StereoDenoiseMode == 2)
            i4StereoDenoiserto = m_i4StereoDenoiserto[1];

        rAEInfo2ISP.u4IspGain = rAEInfo2ISP.u4IspGain * i4StereoDenoiserto / 1000;
        rAEInfo2ISP.u4RealISOValue = rAEInfo2ISP.u4RealISOValue * i4StereoDenoiserto / 1000;
        m_rSensorCurrentInfo.u4Sensitivity = rAEInfo2ISP.u4RealISOValue;
        AE_LOG("[%s()][More OB2] Denoise mode:%d Dev:%d, Stereo ZSD capture - Ratio = %d \n", __FUNCTION__, m_u4StereoDenoiseMode, m_eSensorDev, i4StereoDenoiserto);
    }
    if (eNewAEState == AE_POST_CAPTURE_STATE)
    {
        IspTuningMgr::getInstance().setAEInfoP2((ESensorDev_T)m_eSensorDev, rAEInfo2ISP);
        AE_LOG("[%s()][AE_POST_CAPTURE_STATE] setAEInfoP2 \n", __FUNCTION__);
    }
    else
    {
        IspTuningMgr::getInstance().setAEInfo((ESensorDev_T)m_eSensorDev, rAEInfo2ISP);
    }

    AE_LOG("[%s()] State:%d eAEstate:%d VDNum:%d Exp/Afe/Isp/ISO:%d/%d/%d/%d CurrentidxF:%d CCU ON:%d FrameRate:%6.3f FrameDuration:%lld Flare offset/gain:%d/%d ExpRatio:%d m_bIsMaxIndexStable:%d m_u4ISOIdx1StableCount:%d m_u4ISOIdx2StableCount:%d FDY:%d FDT:%d EVComp:%d\n", __FUNCTION__,
        eNewAEState, m_eAEState, m_i4WaitVDNum, rAEInfo2ISP.u4Eposuretime, rAEInfo2ISP.u4AfeGain, rAEInfo2ISP.u4IspGain, rAEInfo2ISP.u4RealISOValue, rAEInfo2ISP.u4AEidxCurrentF, rAEInfo2ISP.bDisableOBC, u2FrameRate, (long long)m_rSensorCurrentInfo.u8FrameDuration,
        rAEInfo2ISP.i2FlareOffset, i2FlareGain, rAEInfo2ISP.u4EISExpRatio, m_bIsMaxIndexStable, m_u4ISOIdx1StableCount, m_u4ISOIdx2StableCount, rAEInfo2ISP.i4Crnt_FDY, rAEInfo2ISP.u4MeterFDTarget,rAEInfo2ISP.i4AEComp);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::get3ACaptureDelayFrame()
{
    MINT32 i4CaptureDelayFrame = m_i4SensorCaptureDelayFrame;
    getAESensorActiveCycle(&i4CaptureDelayFrame);
    if ((m_bEnableAE == MTRUE) && (mCaptureMode.u4Eposuretime > m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4LongCaptureThres) && (m_bMultiCap == MFALSE) && (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableLongCaptureCtrl == MTRUE))
    {
        AAASensorMgr::getInstance().getSensorModeDelay(m_eSensorMode, &i4CaptureDelayFrame);
        if (i4CaptureDelayFrame < (m_i4IspGainDelayFrames + 3))
        {
            i4CaptureDelayFrame = m_i4TotalCaptureDelayFrame;
        }
        else
        {
            m_i4TotalCaptureDelayFrame = i4CaptureDelayFrame;
            m_u4VsyncCnt = m_i4TotalCaptureDelayFrame;
        }
    }
    AE_LOG("[%s] i4SensorDev:%d i4CaptureDelayFrame:%d %d %d Shutter:%d MultiCap:%d Thres:%d Enable:%d\n", __FUNCTION__, m_eSensorDev, i4CaptureDelayFrame, m_i4SensorCaptureDelayFrame,
        m_i4TotalCaptureDelayFrame, mCaptureMode.u4Eposuretime, m_bMultiCap, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4LongCaptureThres, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableLongCaptureCtrl);
    return i4CaptureDelayFrame;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEAutoFlickerState()
{
    return m_i4AEAutoFlickerMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setFlickerFrameRateActive(MBOOL a_bFlickerFPSAvtive)
{
    //AE_LOG("[%s()] i4SensorDev:%d FlickerFRActive %d->%d\n", __FUNCTION__, m_eSensorDev, a_bFlickerFPSAvtive);
    m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_FLICKER_FPS, a_bFlickerFPSAvtive);
    if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
    {
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
    }
    else
    {
        AAASensorMgr::getInstance().setFlickerFrameRateActive((MINT32)m_eSensorDev, a_bFlickerFPSAvtive);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsMultiCapture(MBOOL bMultiCap)
{
    AE_LOG("[%s()] i4SensorDev:%d bMultiCap:%d\n", __FUNCTION__, m_eSensorDev, bMultiCap);
    m_bMultiCap = bMultiCap;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsAEContinueShot(MBOOL bCShot)
{
    AE_LOG("[%s()] i4SensorDev:%d bCShot:%d\n", __FUNCTION__, m_eSensorDev, bCShot);
    m_bCShot = bCShot;
    if (m_bCShot == MTRUE)
    {
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableEISRecording(MBOOL bEISRecording)
{
    if (bEISRecording)
    {
        m_eAEManualPline = EAEManualPline_EISRecord;
    }
    else
    {
        m_eAEManualPline = EAEManualPline_Default;
    }
    /*
    if(m_eISPProfile ==EIspProfile_Video)
        updateAEScenarioPline(ESensorMode_Video);
    else if(m_eISPProfile ==EIspProfile_Preview)
        updateAEScenarioPline(ESensorMode_Preview);
    */
    AE_LOG("[%s()] i4SensorDev:%d manualAETable: %d m_eAEManualPline: %d\n", __FUNCTION__, m_eSensorDev, bEISRecording, m_eAEManualPline);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setzCHDRShot(MBOOL bHDRShot)
{
    if ((bHDRShot == 1) || (bHDRShot != m_bzcHDRshot))
    {
        AE_LOG("[%s()] i4SensorDev:%d bZCHDRShot:%d->%d\n", __FUNCTION__, m_eSensorDev, m_bzcHDRshot, bHDRShot);
        m_bzcHDRshot = bHDRShot;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableHDRShot(MBOOL bHDRShot)
{
    if ((bHDRShot == 1) || (bHDRShot != m_bHDRshot))
    {
        AE_LOG("[%s()] i4SensorDev:%d bHDRShot:%d->%d\n", __FUNCTION__, m_eSensorDev, m_bHDRshot, bHDRShot);
        m_bHDRshot = bHDRShot;
        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
        if (m_rAEInitInput.rAENVRAM->rAeParam.bPerFrameAESmooth)
            m_u4HDRcountforrevert = 3;
        else
            m_u4HDRcountforrevert = 1;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureShutterValue()
{
    if ((m_bEnableAE == MTRUE) && (m_bMultiCap == MFALSE) && (mCaptureMode.u4Eposuretime > m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4LongCaptureThres) && (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableLongCaptureCtrl == MTRUE))
    {
        m_u4VsyncCnt--;
        if ((MINT32)m_u4VsyncCnt == (m_i4TotalCaptureDelayFrame -1))
        {
            setExp(mCaptureMode.u4Eposuretime);
        }
        else if ((m_u4VsyncCnt == 1) && (m_bCShot == MFALSE))
        {
            // if m_bQuick2Preview is TRUE mean the capture is continue shot
            setExp(30000);
        }
    }
    AE_LOG("[%s] i4SensorDev:%d m_u4VsyncCnt:%d m_bMultiCap:%d m_bCShot:%d\n", __FUNCTION__, m_eSensorDev, m_u4VsyncCnt, m_bMultiCap, m_bCShot);
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::prepareCapParam() //for optimized capture flow
{
    AE_LOG("[%s] AE stable already\n", __FUNCTION__);
    UpdateSensorISPParams(AE_PRE_CAPTURE_STATE);
    doBackAEInfo(); // do back up AE for Precapture AF state.
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAAOLineByteSize() const
{
    AE_LOG("[%s] i4SensorDev:%d i4AAOLineByte:%d\n", __FUNCTION__, m_eSensorDev, m_rAEInitInput.i4AAOLineByte);
    return m_rAEInitInput.i4AAOLineByte;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAAOSize(MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH)
{
    u4BlockNumW = m_u4BlockNumX;
    u4BlockNumH = m_u4BlockNumY;
    AE_LOG("[%s()] i4SensorDev: %d u4BlockNumW/H: %d/%d\n", __FUNCTION__, m_eSensorDev, u4BlockNumW, u4BlockNumH);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::isLVChangeTooMuch()
{
    MUINT32 CWVY = 0;

    if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING))
        CWVY = m_u4CCUResultCwvy;
    else
        CWVY = m_pIAeAlgo->getBrightnessCenterWeightedValue();
    if (ABS((DOUBLE)CWVY - (DOUBLE)m_u4StableYValue) / ((DOUBLE)m_u4StableYValue) > 0.3)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s] CWVYcur:%d CWVYStable:%d Change/Thres percentage:%d/%d \n", __FUNCTION__ , CWVY, m_u4StableYValue, 100 * ABS((MINT32)CWVY - (MINT32)m_u4StableYValue)/((MINT32)m_u4StableYValue), 30);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL AeMgr::SaveAEMgrInfo(const char * fname)
{
    AE_LOG("[%s()] strlen(mEngFileName): %zu \n", __FUNCTION__, strlen(mEngFileName)); /* fix build warning */
    strncpy(mEngFileName, fname, strlen(mEngFileName));
    return MTRUE;
}

MRESULT AeMgr::updateEMISOCaptureParams(AE_MODE_CFG_T &inputparam)
{
    MUINT32 u4OriExposure = inputparam.u4Eposuretime;
    MUINT32 u4OriAfeGain = inputparam.u4AfeGain;
    MUINT32 u4OriIspGain = inputparam.u4IspGain;
    inputparam.u4AfeGain = m_u4AEISOSpeed * 1024 / 100;
    if (inputparam.u4AfeGain <= m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain)
    {
        inputparam.u4IspGain = 1024;
    }
    else
    {
        inputparam.u4IspGain = inputparam.u4AfeGain * 1024 / m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
        inputparam.u4AfeGain = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
    }
    inputparam.u4Eposuretime = u4OriExposure * u4OriAfeGain / inputparam.u4AfeGain * u4OriIspGain / inputparam.u4IspGain;
    inputparam.u4Eposuretime = (inputparam.u4Eposuretime > EMLongestExp) ? EMLongestExp : inputparam.u4Eposuretime;
    if (inputparam.u4Eposuretime < m_pPreviewTableCurrent->pCurrentTable->sPlineTable[0].u4Eposuretime)
    {
        AE_LOG("[%s()] Calculated/PlineMin %d/%d \n", __FUNCTION__, inputparam.u4Eposuretime, m_pPreviewTableCurrent->pCurrentTable->sPlineTable[0].u4Eposuretime);
        inputparam.u4Eposuretime = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[0].u4Eposuretime;
    }
    AE_LOG("[%s():EM -ISO] m_u4AEISOSpeed:%d Exp/Afe/Isp:%d/%d/%d -> Exp/Afe/Isp:%d/%d/%d \n", __FUNCTION__, m_u4AEISOSpeed, u4OriExposure, u4OriAfeGain, u4OriIspGain , inputparam.u4Eposuretime, inputparam.u4AfeGain, inputparam.u4IspGain);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::setTGInfo(MINT32 const i4TGInfo)
{
    AE_LOG("[%s()] i4TGInfo: %d \n", __FUNCTION__, i4TGInfo);

    switch (i4TGInfo)
    {
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
        default:
            CAM_LOGE("[%s()] i4TGInfo = %d", __FUNCTION__, i4TGInfo);
            return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    AE_LOG("[%s()]\n", __FUNCTION__);

    AE_NVRAM_T *pAENVRAM = reinterpret_cast <AE_NVRAM_T *> (a_pAENVRAM);

    getNvramData(m_eSensorDev);

    if (pAENVRAM != NULL)
    {
        *pAENVRAM = m_p3ANVRAM->AE[m_u4AENVRAMIdx];
    }
    else
    {
        AE_LOG("[%s()] NVRAM is NULL\n", __FUNCTION__);
    }

    *a_pOutLen = sizeof(AE_NVRAM_T);

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getAEInitExpoSetting(AEInitExpoSetting_T &a_rAEInitExpoSetting)
{
    AE_LOG("[%s()] m_eSensorDev:%d u4SensorMode:%d u4AETargetMode:%d\n", __FUNCTION__,
        m_eSensorDev, a_rAEInitExpoSetting.u4SensorMode, a_rAEInitExpoSetting.u4AETargetMode);
    // customize for 4cell mvhdr preview <-- normal preview +
    MINT32 i4IndexOffset = 0;
    if ((m_eSensorMode == ESensorMode_Preview && a_rAEInitExpoSetting.u4SensorMode == SENSOR_SCENARIO_ID_NORMAL_PREVIEW) &&
        (m_eAETargetMode2 == AE_MODE_NORMAL && a_rAEInitExpoSetting.u4AETargetMode == AE_MODE_4CELL_MVHDR_TARGET))
    {
        setAEHDRMode(1);
        setNVRAMIndex(3);
        i4IndexOffset = 10;
    }
    else if ((m_eSensorMode == ESensorMode_Preview && a_rAEInitExpoSetting.u4SensorMode == SENSOR_SCENARIO_ID_NORMAL_PREVIEW) &&
        (m_eAETargetMode2 == AE_MODE_4CELL_MVHDR_TARGET && a_rAEInitExpoSetting.u4AETargetMode == AE_MODE_NORMAL))
    {
        i4IndexOffset = -10;
    }
    m_eAETargetMode2 = (eAETargetMODE)(a_rAEInitExpoSetting.u4AETargetMode);
    // customize for 4cell mvhdr preview <-- normal preview -
    SetAETargetMode((eAETargetMODE)(a_rAEInitExpoSetting.u4AETargetMode));
    setSensorMode((MINT32)(a_rAEInitExpoSetting.u4SensorMode), 0, 0);

    // Finer EV
    m_pIAeAlgo->getEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    AE_LOG("[%s()-getEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    setAEScene(m_eAEScene);
    setIsoSpeed(m_u4AEISOSpeed);
    setAEMinMaxFrameRatePlineIdx(m_i4AEMinFps , m_i4AEMaxFps);
    MUINT32 u4FinerEVIdxBase = 1;
    // customize for 4cell mvhdr preview <-- normal preview +
    m_u4Index = m_u4Index + i4IndexOffset;
    AE_LOG("[%s()] m_eSensorDev:%d new m_u4Index:%d i4IndexOffset:%d\n", __FUNCTION__, m_eSensorDev, m_u4Index, i4IndexOffset);
    // customize for 4cell mvhdr preview <-- normal preview -
    m_pIAeAlgo->updateAEPlineInfo_v4p0(m_pPreviewTableCurrent, m_pCaptureTable, &m_PreviewTableF, &m_CaptureTableF, &(m_pAEPlineTable->AEGainList), m_rAEInitInput.rAENVRAM, &(m_rAEInitInput.rAENVRAM->rAeParam), u4FinerEVIdxBase);
    if (u4FinerEVIdxBase != m_u4FinerEVIdxBase || (i4IndexOffset != 0))
    {
        m_u4FinerEVIdxBase = u4FinerEVIdxBase;
        m_u4IndexF = m_u4Index * m_u4FinerEVIdxBase;
        m_u4IndexFMin = m_u4IndexMin * m_u4FinerEVIdxBase;
        m_u4IndexFMax = m_u4IndexMax * m_u4FinerEVIdxBase;
        searchPreviewIndexLimit();
    }
    AE_LOG("[%s()-setEVIdxInfo_v4p0] FinerEVIdxBase:%d MaxIdx/MinIdx/Idx/MaxIdxF/MinIdxF/IdxF:%d/%d/%d/%d/%d/%d\n", __FUNCTION__, m_u4FinerEVIdxBase, m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax, m_u4IndexMin, m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
    m_pIAeAlgo->setSensorModeMaxBVdiff(m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
    updateAEidxtoExpsetting();

    if ((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) || m_bVHDRChangeFlag)
    {
        m_rAEInitInput.u4AEISOSpeed = m_u4AEISOSpeed;
        m_rAEInitInput.eAETargetMode = m_eAETargetMode;

        strAEOutput rAEOutput;
        MBOOL bEnablePreIndex = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex;
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = MTRUE;
        AE_LOG("[%s()] reset mvhdr ratio:%d\n", __FUNCTION__, m_bResetMvhdrRatio);
        m_rAEInitInput.rAENVRAM->rCCTConfig.rAEAOENVRAMParam.i4AOEStrengthIdx = m_bResetMvhdrRatio;
        m_pIAeAlgo->initAE(&m_rAEInitInput, &rAEOutput, &m_rAEStatCfg);
        resetZVHDRFlag();
        copyAEInfo2mgr(&m_rAEOutput.rPreviewMode, &rAEOutput, MTRUE);
        if ((m_eAETargetMode != AE_MODE_IVHDR_TARGET) && (m_eAETargetMode != AE_MODE_MVHDR_TARGET) && (m_eAETargetMode != AE_MODE_ZVHDR_TARGET) && (m_eAETargetMode != AE_MODE_4CELL_MVHDR_TARGET))
        {
            a_rAEInitExpoSetting.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
            a_rAEInitExpoSetting.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
            // UpdateSensorISPParams(AE_INIT_STATE);
        }
        else
        {
            if (m_eAETargetMode == AE_MODE_MVHDR_TARGET)
            {
                a_rAEInitExpoSetting.u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
                a_rAEInitExpoSetting.u4Eposuretime_se = m_strIVHDROutputSetting.u4SEExpTimeInUS;
                a_rAEInitExpoSetting.u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
            }
            else if (m_eAETargetMode == AE_MODE_ZVHDR_TARGET)
            {
                a_rAEInitExpoSetting.u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
                a_rAEInitExpoSetting.u4Eposuretime_se = m_strIVHDROutputSetting.u4SEExpTimeInUS;
                a_rAEInitExpoSetting.u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
                a_rAEInitExpoSetting.u4AfeGain_se = m_strIVHDROutputSetting.u4SESensorGain;
            }
            else if (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)
            {
                a_rAEInitExpoSetting.u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
                a_rAEInitExpoSetting.u4Eposuretime_se = m_strIVHDROutputSetting.u4SEExpTimeInUS;
                a_rAEInitExpoSetting.u4Eposuretime_me = m_strIVHDROutputSetting.u4MEExpTimeInUS;
                a_rAEInitExpoSetting.u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
                a_rAEInitExpoSetting.u4AfeGain_se = m_strIVHDROutputSetting.u4SESensorGain;
                a_rAEInitExpoSetting.u4AfeGain_me = m_strIVHDROutputSetting.u4MESensorGain;
            }
            else
            {
                a_rAEInitExpoSetting.u4Eposuretime = m_strIVHDROutputSetting.u4LEExpTimeInUS;
                a_rAEInitExpoSetting.u4Eposuretime_se = m_strIVHDROutputSetting.u4SEExpTimeInUS;
                a_rAEInitExpoSetting.u4AfeGain = m_strIVHDROutputSetting.u4LESensorGain;
            }
            // UpdateSensorISPParams(AE_TWO_SHUTTER_INIT_STATE);
            m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = bEnablePreIndex;
        }
    }
    else
    {
        a_rAEInitExpoSetting.u4Eposuretime = m_rAEOutput.rPreviewMode.u4Eposuretime;
        a_rAEInitExpoSetting.u4AfeGain = m_rAEOutput.rPreviewMode.u4AfeGain;
        // UpdateSensorISPParams(AE_INIT_STATE);
    }

    AE_LOG("[%s()] u4Eposuretime(%d) u4AfeGain(%d) u4Eposuretime_se(%d) u4AfeGain_se(%d) u4Eposuretime_me(%d) u4AfeGain_me(%d) u4Eposuretime_vse(%d) u4AfeGain_vse(%d)\n",
        __FUNCTION__,
        a_rAEInitExpoSetting.u4Eposuretime, a_rAEInitExpoSetting.u4AfeGain,
        a_rAEInitExpoSetting.u4Eposuretime_se, a_rAEInitExpoSetting.u4AfeGain_se,
        a_rAEInitExpoSetting.u4Eposuretime_me, a_rAEInitExpoSetting.u4AfeGain_me,
        a_rAEInitExpoSetting.u4Eposuretime_vse, a_rAEInitExpoSetting.u4AfeGain_vse);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAE2AFInfo(AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo)
{
    MUINT8 iYvalue = 0;
    getAEMeteringYvalue(rAeWinSize, &iYvalue);
    rAEInfo.iYvalue = (MINT64)iYvalue;
    rAEInfo.i4ISO = m_rSensorCurrentInfo.u4Sensitivity;
    rAEInfo.i4IsAEStable = m_bAEStable;
    rAEInfo.i4SceneLV = getLVvalue(MTRUE);
    rAEInfo.ishutterValue = m_rAEOutput.rPreviewMode.u4Eposuretime;
    rAEInfo.i4DeltaBV = m_i4DeltaBV;
    getAEBlockYvalues(rAEInfo.aeBlockV, 25);
    MUINT16 u2YCnt = 0;
    //getAEMeteringBlockAreaValue(rAeWinSize, rAEInfo.pAEBlockAreaYvalue, &u2YCnt);
    //rAEInfo.i4AEBlockAreaYCnt = (MINT32)u2YCnt;
    rAEInfo.i4IsAELocked = IsAELock() || IsAPAELock();
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg)
{
    MRESULT err;
    if (pDBinInfo == NULL || pOutRegCfg == NULL)
    {
        CAM_LOGE("[%s()] : pDBinInfo is NULL or pOutRegCfg is NULL", __FUNCTION__);
        return S_AE_OK;
    }
    BIN_INPUT_INFO *psDBinInfo = static_cast <BIN_INPUT_INFO *> (pDBinInfo);
    MUINT32 width = psDBinInfo->TarQBNOut_W;
    MUINT32 height = psDBinInfo->TarBinOut_H;

    if (m_u4Hbinwidth != width || m_u4Hbinheight != height)
    {
        AE_LOG("[%s()] QBN:(%d,%d)->(%d,%d) m_u4Hbinwidth/m_u4Hbinheight:(%d,%d)->(%d,%d)", __FUNCTION__,
            psDBinInfo->CurQBNOut_W, psDBinInfo->CurBinOut_H, width, height,
            m_u4Hbinwidth, m_u4Hbinheight, width, height);
        m_u4Hbinwidth = width;
        m_u4Hbinheight = height;
        m_pIAeAlgo->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);

        // Window size
        MUINT32 u4BlocksizeX = ((m_u4Hbinwidth / m_u4BlockNumX) / 2) * 2;
        MUINT32 u4BlocksizeY = ((m_u4Hbinheight / m_u4BlockNumY) / 2) * 2;
        m_u4AEOverExpCntShift = (((u4BlocksizeX / 4) * (u4BlocksizeY / 2) > 255) ? MTRUE : MFALSE);

        AAA_TRACE_D("AEStatReconfig");
        err = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).setAEconfigParam(m_u4BlockNumX, m_u4BlockNumY,
            m_bEnableHDRYConfig, m_bEnableOverCntConfig, m_bEnableTSFConfig,
            m_bDisablePixelHistConfig, m_u4AEOverExpCntThs, m_u4AEOverExpCntShift, m_i4AAOmode);
        err = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).reconfig(pDBinInfo, pOutRegCfg, m_rAEStatCfg);
        AAA_TRACE_END_D;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAESMBuffermode(MBOOL bSMBuffmode, MINT32 i4subframeCnt)
{
    if (m_bEnSWBuffMode != bSMBuffmode)
    {
        AE_LOG("[%s()] Slow Motion AE Buffer Mode Enable:%d->%d SubSample:%d\n", __FUNCTION__, m_bEnSWBuffMode, bSMBuffmode, i4subframeCnt);
        m_bEnSWBuffMode = bSMBuffmode;
        m_i4SMSubSamples = i4subframeCnt;
        //AAASensorMgr::getInstance().enableSensorI2CBuffMode((MINT32)m_eSensorDev, m_bEnSWBuffMode);
        if (m_i4SMSubSamples == 8)
        {
            m_eAEManualPline = EAEManualPline_SM240FPS;
        }
        else
        {
            m_eAEManualPline = EAEManualPline_Default;
        }

        updateAEScenarioPline(m_eSensorMode);
        AE_LOG("[%s()] i4SensorDev:%d manualAETable:%d m_eAEManualPline:%d\n", __FUNCTION__, m_eSensorDev, m_i4SMSubSamples, m_eAEManualPline);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEManualPline(EAEManualPline_T eAEManualPline, MBOOL bEnable)
{
    AE_LOG("[%s()] m_eAEManualPline:%d eAEManualPline:%d bEnable:%d\n", __FUNCTION__, m_eAEManualPline, eAEManualPline, bEnable);
    if (!bEnable)
        eAEManualPline = EAEManualPline_Default;
    if (m_eAEManualPline != eAEManualPline)
    {
        AE_LOG("[%s()] i4SensorDev:%d m_eAEManualPline:%d->%d\n", __FUNCTION__, m_eSensorDev, m_eAEManualPline, eAEManualPline);
        m_eAEManualPline = eAEManualPline;
        updateAEScenarioPline(m_eSensorMode);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEManualCapPline(EAEManualPline_T eAEManualCapPline, MBOOL bEnable)
{
    AE_LOG("[%s()] m_eAEManualCapPline:%d eAEManualCapPline:%d bEnable:%d\n", __FUNCTION__, m_eAEManualCapPline, eAEManualCapPline, bEnable);
    if (!bEnable)
        eAEManualCapPline = EAEManualPline_Default;
    if (m_eAEManualCapPline != eAEManualCapPline)
    {
        AE_LOG("[%s()] i4SensorDev:%d m_eAEManualCapPline:%d->%d\n", __FUNCTION__, m_eSensorDev, m_eAEManualCapPline, eAEManualCapPline);
        m_eAEManualCapPline = eAEManualCapPline;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updatePreviewParamsByHFSM(AE_MODE_CFG_T *sAEInfo)
{
    MUINT32 u4exp, u4afe, u4isp;
    MUINT32 u4exppre, u4afepre, u4isppre;
    u4exp = sAEInfo->u4Eposuretime;
    u4afe = sAEInfo->u4AfeGain;
    u4isp = sAEInfo->u4IspGain;
    u4exppre = m_u4PreExposureTime;
    u4afepre = m_u4PreSensorGain;
    u4isppre = m_u4PreIspGain;
    float deltaratio = 0.0;

    // Both Exp and Afe should be modified
    if ((u4exp != u4exppre) && (u4afe != u4afepre))
    {
        MUINT64 u8totalexpgain;
        MUINT64 u8totalexpgainpre;
        deltaratio = ((float)u4exp) / u4exppre * u4afe / u4afepre * u4isp / u4isppre;
        AE_LOG("[%s():s] %d/%d/%d -> %d/%d/%d , Delta:%f \n", __FUNCTION__, u4exppre, u4afepre, u4isppre, u4exp, u4afe, u4isp,
            deltaratio);
        u8totalexpgain = (MUINT64)u4exp * u4afe * u4isp;
        u8totalexpgainpre = (MUINT64)u4exppre * u4afepre * u4isppre;
        if (u8totalexpgainpre <= u8totalexpgain)
        {
            u4exp = sAEInfo->u4Eposuretime;
            u4afe = m_u4PreSensorGain;
            u4isp = sAEInfo->u4AfeGain * sAEInfo->u4IspGain / m_u4PreSensorGain;
            if ((u4isp < 1024) || (u4isp > 8192))
            {
                if (u4isp < 1024)
                    u4isp = 1024;
                else
                    u4isp = 8192;
                u4exp = m_u4PreExposureTime;
                u4afe = sAEInfo->u4AfeGain;
                m_u4HFSMSkipFrame = 1;
                mPreviewModeBackup = *sAEInfo;
                AE_LOG_IF(m_3ALogEnable, "[%s()Pre < Cur] m_u4HFSMSkipFrame:%d , Backup:%d/%d/%d \n", __FUNCTION__,
                    m_u4HFSMSkipFrame,
                    mPreviewModeBackup.u4Eposuretime,
                    mPreviewModeBackup.u4AfeGain,
                    mPreviewModeBackup.u4IspGain);
            }
        }
        else
        {
            u4exp = sAEInfo->u4Eposuretime;
            u4afe = m_u4PreSensorGain;
            u4isp = sAEInfo->u4AfeGain * sAEInfo->u4IspGain / m_u4PreSensorGain;
            if ((u4isp < 1024) || (u4isp > 8192))
            {
                if (u4isp < 1024)
                {
                    u4isp = 1024;
                }
                else
                {
                    u4isp = 8192;
                }
            }
            m_u4HFSMSkipFrame = 1;
            mPreviewModeBackup = *sAEInfo;
            AE_LOG_IF(m_3ALogEnable, "[%s()Pre > Cur] m_u4HFSMSkipFrame:%d , Backup:%d/%d/%d \n", __FUNCTION__,
                m_u4HFSMSkipFrame,
                mPreviewModeBackup.u4Eposuretime,
                mPreviewModeBackup.u4AfeGain,
                mPreviewModeBackup.u4IspGain);
        }
        deltaratio = ((float)u4exp) / u4exppre * u4afe / u4afepre * u4isp / u4isppre;
        AE_LOG("[%s():e] %d/%d/%d -> %d/%d/%d , Delta:%f\n", __FUNCTION__, u4exppre, u4afepre, u4isppre, u4exp, u4afe, u4isp, deltaratio);
        sAEInfo->u4Eposuretime = u4exp;
        sAEInfo->u4AfeGain = u4afe;
        sAEInfo->u4IspGain = u4isp;
    }
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAISManualPline(MUINT32 u4AISPline)
{
    EAEManualPline_T eAEManualCapPline = EAEManualPline_AIS1Capture;

    switch (u4AISPline)
    {
        case 0x01:
            eAEManualCapPline = EAEManualPline_AIS1Capture;
            break;
        case 0x02:
            eAEManualCapPline = EAEManualPline_AIS2Capture;
            break;
        case 0x00:
        default:
            eAEManualCapPline = EAEManualPline_Default;
            break;
    }

    if (m_eAEManualCapPline != eAEManualCapPline)
    {
        AE_LOG("[%s()] i4SensorDev:%d m_eAEManualCapPline:%d->%d\n", __FUNCTION__, m_eSensorDev, m_eAEManualCapPline, eAEManualCapPline);
        m_eAEManualCapPline = eAEManualCapPline;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableMFHRManualPline(MBOOL bMFHRPline)
{
    EAEManualPline_T eAEManualCapPline = EAEManualPline_MFHRCapture;
    if (!bMFHRPline)
        eAEManualCapPline = EAEManualPline_Default;
    if (m_eAEManualCapPline != eAEManualCapPline)
    {
        AE_LOG("[%s()] i4SensorDev:%d m_eAEManualCapPline:%d->%d\n", __FUNCTION__, m_eSensorDev, m_eAEManualCapPline, eAEManualCapPline);
        m_eAEManualCapPline = eAEManualCapPline;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableBMDNManualPline(MBOOL bBMDNPline)
{
    EAEManualPline_T eAEManualCapPline = EAEManualPline_BMDNCapture;
    if (!bBMDNPline)
        eAEManualCapPline = EAEManualPline_Default;
    if (m_eAEManualCapPline != eAEManualCapPline)
    {
        AE_LOG("[%s()] i4SensorDev:%d m_eAEManualCapPline:%d->%d\n", __FUNCTION__, m_eSensorDev, m_eAEManualCapPline, eAEManualCapPline);
        m_eAEManualCapPline = eAEManualCapPline;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateStereoDenoiseRatio(MINT32 * i4StereoDenoiserto)
{
    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d BMDN/MFHR:%d/%d->%d/%d\n", __FUNCTION__, m_eSensorDev, m_i4StereoDenoiserto[0], m_i4StereoDenoiserto[1], *i4StereoDenoiserto, *(i4StereoDenoiserto + 1));
    m_i4StereoDenoiserto[0] = *i4StereoDenoiserto;
    m_i4StereoDenoiserto[1] = *(i4StereoDenoiserto + 1);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableStereoDenoiseRatio(MUINT32 u4enableStereoDenoise)
{
    if (m_u4StereoDenoiseMode != u4enableStereoDenoise)
    {
        AE_LOG_IF(m_3ALogEnable, "[%s():e] i4SensorDev:%d Stereo denoise:%d->%d\n", __FUNCTION__, m_eSensorDev, m_u4StereoDenoiseMode, u4enableStereoDenoise);
        m_u4StereoDenoiseMode = u4enableStereoDenoise;
        //update pass2 OB2
        updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableFlareInManualControl(MBOOL bIsFlareInManual)
{
    if (m_bIsFlareInManual != bIsFlareInManual)
    {
        AE_LOG("[%s()] i4SensorDev:%d bIsFlareInManual:%d->%d\n", __FUNCTION__, m_eSensorDev, m_bIsFlareInManual, bIsFlareInManual);
        m_bIsFlareInManual = bIsFlareInManual;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEStereoManualPline(MBOOL bstereomode)
{
    if (m_bStereoManualPline != bstereomode)
    {
        AE_LOG("[%s()] Stereo:%d->%d m_eSensorMode:%d\n", __FUNCTION__, m_bStereoManualPline, bstereomode, m_eSensorMode);

        if ((bstereomode == MTRUE) && (m_bStereoManualPline == MFALSE))
        {
            // Switch to Stereo Pline
            m_pAEMapPlineTable = (AE_PLINEMAPPINGTABLE_T *)(&(m_rAEInitInput.rAENVRAM->rAeParam.strStereoPlineMapping.sAESceneMapping[0]));
            // CCU control
            // if (m_bCCUAEFlag){
            // m_pIAeFlowCCU->stop();
            // m_pIAeFlowCCU->uninit();
            // m_bCCUAEFlag = MFALSE;
            // }
        }
        else if ((bstereomode == MFALSE) && (m_bStereoManualPline == MTRUE))
        {
            // Switch to Stereo Pline
            m_pAEMapPlineTable = &(m_rAEInitInput.rAEPlineMapTable[0]);
        }

        m_bStereoManualPline = bstereomode;
        for (MINT32 i = 0; i < 30; i++)
        {
            AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d eID/ePlinetable = %d - %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d \n", __FUNCTION__, m_eSensorDev, (MINT32)m_pAEMapPlineTable[i].eAEScene,
                (MINT32)m_pAEMapPlineTable[i].ePLineID[0], (MINT32)m_pAEMapPlineTable[i].ePLineID[1], (MINT32)m_pAEMapPlineTable[i].ePLineID[2],
                (MINT32)m_pAEMapPlineTable[i].ePLineID[3], (MINT32)m_pAEMapPlineTable[i].ePLineID[4], (MINT32)m_pAEMapPlineTable[i].ePLineID[5],
                (MINT32)m_pAEMapPlineTable[i].ePLineID[6], (MINT32)m_pAEMapPlineTable[i].ePLineID[7], (MINT32)m_pAEMapPlineTable[i].ePLineID[8],
                (MINT32)m_pAEMapPlineTable[i].ePLineID[9], (MINT32)m_pAEMapPlineTable[i].ePLineID[10]);
        }
        m_bPlineMappingChange = MTRUE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEdelayInfo(MINT32 * i4ExpDelay, MINT32 * i4AEGainDelay, MINT32 * i4IspGainDelay)
{
    //MRESULT err; /* fix build warning */
    *i4ExpDelay = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET - m_i4ShutterDelayFrames;
    *i4AEGainDelay = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET - m_i4SensorGainDelayFrames;
    *i4IspGainDelay = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET;
    if (m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD))
        *i4AEGainDelay = *i4AEGainDelay - 1;
    AE_LOG("[%s()] Exp/Afe/Isp delay frame:%d/%d/%d AfeGainFlg:%d\n", __FUNCTION__, *i4ExpDelay, *i4AEGainDelay, *i4IspGainDelay, m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD));
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getTgIntAEInfo(MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio)
{
    bTgIntAEEn = MFALSE;
    fTgIntAERatio = TG_INT_AE_RATIO;
    AE_LOG("[%s()] m_bTgIntAEEn:%d m_bPerframeAEFlag:%d TG_INT_AE_RATIO:%f\n", __FUNCTION__, m_bTgIntAEEn, m_bPerframeAEFlag, fTgIntAERatio);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPerframeAEFlag(MBOOL &bPerframeAE)
{
    bPerframeAE = m_bPerframeAEFlag;
    bPerframeAE = MFALSE; // temp: turn off perframe AE stereo
    return S_AE_OK;
}

#include <math.h>

MRESULT AeMgr::updateAEBV(MVOID *pAEStatBuf)
{
    MINT32 i4DeltaBV = 0;
    MVOID *pAEStatisticBuf;
    MUINT32 u4IndexMax, u4IndexMin, u4Index = 0;
    MUINT32 u4CWVY = 0;
    double d8target = 47;

    pAEStatisticBuf = pAEStatBuf;

    //Calculate CWVY
    if (m_pIAeAlgo != NULL)
    {
        if (pAEStatisticBuf != NULL)
        {
            Mutex::Autolock lock(m_Lock);
            AaaTimer localTimer("AESatistic", m_eSensorDev, (m_3ALogEnable & EN_3A_SCHEDULE_LOG));
            m_pIAeAlgo->setAESatisticBufferAddr_v4p0(m_pPSOStatBuf, pAEStatisticBuf, &m_strAAOProcInfo);
            localTimer.End();
        }
        else
        {
            MY_LOG("[%s()] The AAO buffer pointer is NULL i4SensorDev = %d line:%d \n", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        // monitor AE state
        u4CWVY = m_pIAeAlgo->getBrightnessCenterWeightedValue();
        m_pIAeAlgo->getEVIdxInfo(u4IndexMax , u4IndexMin , u4Index);

    }
    else
    {
        MY_LOG("[%s():1] The AE algo class is NULL i4SensorDev = %d \n", __FUNCTION__, m_eSensorDev);
    }
    // protection to avoid u4CWVY = 0
    if (u4CWVY < 1)
        u4CWVY = 1;

    i4DeltaBV = (MINT32)((log2(d8target)) * 1000 - log2(double(u4CWVY)) * 1000) + ((MINT32)u4Index * 100);
    m_BVvalue = ((m_pPreviewTableCurrent->i4MaxBV - m_rAEInitInput.rAENVRAM->rCCTConfig.i4BVOffset) * 100 - i4DeltaBV) / 100;
    m_AOECompBVvalue = m_BVvalue;

    MY_LOG("[monitorAndReschedule:%s] i4SensorDev:%d CWVY:%d->47 index:%d i4DeltaBV:%d m_BVvalue:%d\n", __FUNCTION__, m_eSensorDev, u4CWVY, u4Index, i4DeltaBV, m_BVvalue);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setDigZoomRatio(MINT32 i4ZoomRatio)
{
    if (m_i4ZoomRatio != i4ZoomRatio)
    {
        MY_LOG("[%s] i4SensorDev:%d ZoomRatio:%d->%d\n", __FUNCTION__, m_eSensorDev, m_i4ZoomRatio, i4ZoomRatio);
        m_i4ZoomRatio = i4ZoomRatio;
        m_pIAeAlgo->setZoomRatio(m_i4ZoomRatio);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setCCUOnOff(MBOOL enable)
{
    if (enable)
    {
        // Control CCU start
        if (ShouldCCUStart())
        {
            AE_LOG("[%s()] CCU start\n", __FUNCTION__);
            AE_FLOW_INPUT_INFO_T initInfo;
            prepareCCUStartParam(&initInfo);
            m_pIAeFlowCCU->start((MVOID*) &initInfo);
            m_bCCUAEFlag = MTRUE;
        }
    }
    else
    {
        // Stop CCU
        if (m_bCCUAEFlag)
        {
            AE_LOG("[%s()] CCU pause\n", __FUNCTION__);
            m_pIAeFlowCCU->pause();
            m_bCCUAEFlag = MFALSE;
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::switchExpSettingByShutterISOpriority(AE_EXP_SETTING_T &a_strExpInput, AE_EXP_SETTING_T &a_strExpOutput)
{
    if ((a_strExpInput.u4Sensitivity == 0) && (a_strExpInput.u4ExposureTime != 0))
    {
        // ISO auto
        a_strExpOutput.u4ExposureTime = a_strExpInput.u4ExposureTime;
        a_strExpOutput.u4Sensitivity = ((MINT64)m_rAEOutput.rCaptureMode[0].u4Eposuretime * m_rAEOutput.rCaptureMode[0].u4RealISO) / a_strExpInput.u4ExposureTime;
        if ((MUINT32)a_strExpOutput.u4Sensitivity < m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain)
        {
            a_strExpOutput.u4Sensitivity = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        }
    }
    else if ((a_strExpInput.u4ExposureTime == 0) && (a_strExpInput.u4Sensitivity != 0))
    {
        // Exp auto
        a_strExpOutput.u4Sensitivity = a_strExpInput.u4Sensitivity;
        a_strExpOutput.u4ExposureTime = ((MINT64)m_rAEOutput.rCaptureMode[0].u4Eposuretime * m_rAEOutput.rCaptureMode[0].u4RealISO) / a_strExpInput.u4Sensitivity;
    }
    else if ((a_strExpInput.u4ExposureTime != 0) && (a_strExpInput.u4Sensitivity != 0))
    {
        // keep value
        a_strExpOutput.u4ExposureTime = a_strExpInput.u4ExposureTime;
        a_strExpOutput.u4Sensitivity = a_strExpInput.u4Sensitivity;
    }
    else
    {
        a_strExpOutput.u4ExposureTime = m_rAEOutput.rCaptureMode[0].u4Eposuretime;
        a_strExpOutput.u4Sensitivity = m_rAEOutput.rCaptureMode[0].u4RealISO;
    }

    MY_LOG("[%s] Input Shutter:%d ISO:%d, Output Shutter:%d ISO:%d Capture Shutter:%d ISO:%d MinISO(%d):%d\n",
        __FUNCTION__, a_strExpInput.u4ExposureTime, a_strExpInput.u4Sensitivity, a_strExpOutput.u4ExposureTime, a_strExpOutput.u4Sensitivity, m_rAEOutput.rCaptureMode[0].u4Eposuretime, m_rAEOutput.rCaptureMode[0].u4RealISO, m_u4AENVRAMIdx, m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    if (a_ISOIdx1Status != NULL)
    {
        if (m_u4ISOIdx1StableCount >= 2 *m_u4Idx1StableThres)
        {
            // > ISO value 1 and stable
            *a_ISOIdx1Status = 0;
        }
        else if (m_u4ISOIdx1StableCount <= 0)
        {
            // < ISO value 1 and stable
            *a_ISOIdx1Status = 1;
        }
        else
        {
            // unstable
            *a_ISOIdx1Status = 2;
        }
    }
    else
    {
        MY_LOG("[%s] a_ISOIdx1Status:NULL pointer m_u4ISOIdx1StableCount:%d, m_u4ISOIdx2StableCount:%d MAX_INDEX1_STABLE_THRES:%d->%d m_u4ISOIdx1StableCount:%d m_u4Index:%d m_u4IndexMax:%d\n",
            __FUNCTION__, m_u4ISOIdx1StableCount, m_u4ISOIdx2StableCount, MAX_INDEX1_STABLE_THRES, m_u4Idx1StableThres, m_u4ISOIdx1StableCount, m_u4Index, m_u4IndexMax);
    }

    if (a_ISOIdx2Status != NULL)
    {
        if (m_u4ISOIdx2StableCount >= 2 *m_u4Idx2StableThres)
        {
            // > ISO value 2 and stable
            *a_ISOIdx2Status = 0;
        }
        else if (m_u4ISOIdx2StableCount <= 0)
        {
            // < ISO value 1 and stable
            *a_ISOIdx2Status = 1;
        }
        else
        {
            // unstable
            *a_ISOIdx2Status = 2;
        }
    }
    else
    {
        MY_LOG("[%s] a_ISOIdx2Status:NULL pointer m_u4ISOIdx1StableCount:%d, m_u4ISOIdx2StableCount:%d MAX_INDEX_STABLE2_THRES:%d->%d m_u4ISOIdx1StableCount:%d m_u4Index:%d m_u4IndexMax:%d\n",
            __FUNCTION__, m_u4ISOIdx1StableCount, m_u4ISOIdx2StableCount, MAX_INDEX2_STABLE_THRES, m_u4Idx2StableThres, m_u4ISOIdx1StableCount, m_u4Index, m_u4IndexMax);
    }

    if ((a_ISOIdx1Status != NULL) && (a_ISOIdx2Status != NULL))
    {
        MY_LOG("[%s] a_ISOIdx1Status:%d m_u4ISOIdx1StableCount:%d, a_ISOIdx2Status:%d m_u4ISOIdx2StableCount:%d INDEX1_STABLE_THRES:%d->%d INDEX2_STABLE_THRES:%d->%d m_u4Index:%d m_u4IndexMax:%d\n",
            __FUNCTION__, *a_ISOIdx1Status, m_u4ISOIdx1StableCount, *a_ISOIdx2Status, m_u4ISOIdx2StableCount, MAX_INDEX1_STABLE_THRES, m_u4Idx1StableThres, MAX_INDEX2_STABLE_THRES, m_u4Idx2StableThres, m_u4Index, m_u4IndexMax);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::resetGetISOThresStatus(MINT32 * /* a_ISOIdx1Status */ , MINT32 * /* a_ISOIdx2Status */ ) /* fix build warning */
{
    m_u4ISOIdx1StableCount = m_u4Idx1StableThres;
    m_u4ISOIdx2StableCount = m_u4Idx2StableThres;
    MY_LOG("[%s] reset m_u4ISOIdx1StableCount / m_u4ISOIdx2StableCount to 0\n", __FUNCTION__);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateGyroInfo()
{
    #if 1
    // get gyro / acceleration data
    SensorData gyroDa;
    MBOOL gyroDaVd = m_bGryoVd ? mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, gyroDa) : MFALSE;

    if (gyroDaVd && gyroDa.timestamp)
    {
        //m_u8PreGyroTS = m_u8GyroTS;
        m_i4GyroInfo[0] = gyroDa.gyro[0] * SENSOR_GYRO_SCALE;
        m_i4GyroInfo[1] = gyroDa.gyro[1] * SENSOR_GYRO_SCALE;
        m_i4GyroInfo[2] = gyroDa.gyro[2] * SENSOR_GYRO_SCALE;
        AE_LOG_IF(m_3ALogEnable, "[%s] m_i4GyroInfo[2] = %d \n", __FUNCTION__, m_i4GyroInfo[2]);
        //m_u8GyroTS = gyroDa.timestamp;
        //u4GyroScale = m_u8GyroTS != m_u8PreGyroTS ? SENSOR_GYRO_SCALE : 0;
    }
    else
    {
        AE_LOG("[%s] Gyro InValid!", __FUNCTION__);
        //u4GyroScale = 0; // set scale 0 means invalid to algo
    }
    SensorData acceDa;
    MBOOL acceDaVd = m_bAcceVd ? mpSensorProvider->getLatestSensorData(SENSOR_TYPE_ACCELERATION, acceDa) : MFALSE;

    if (acceDaVd && acceDa.timestamp)
    {
        //m_u8PreAcceTS = m_u8AcceTS;
        m_i4AcceInfo[0] = acceDa.acceleration[0] * SENSOR_ACCE_SCALE;
        m_i4AcceInfo[1] = acceDa.acceleration[1] * SENSOR_ACCE_SCALE;
        m_i4AcceInfo[2] = acceDa.acceleration[2] * SENSOR_ACCE_SCALE;
        //AE_LOG("[%s] m_i4AcceInfo[1] = %d \n", __FUNCTION__, m_i4AcceInfo[1]);
        //m_u8AcceTS = acceDa.timestamp;

        //u4ACCEScale = m_u8AcceTS != m_u8PreAcceTS ? SENSOR_ACCE_SCALE : 0;
    }
    else
    {
        AE_LOG("[%s] Acce InValid!", __FUNCTION__);
        //u4ACCEScale = 0; // set scale 0 means invalid to algo
    }
    #endif
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setFrameRateLock(MBOOL enable)
{
    if (enable)
    {
        // disable CCU
        if (m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING && m_i4AEMaxFps > LIB3A_AE_FRAMERATE_MODE_30FPS))
        {
            AE_LOG("[%s()] CCU pause\n", __FUNCTION__);
            m_pIAeFlowCCU->pause();
        }
        m_bIsFrameRateLocked = MTRUE;
    }
    else
    {
        m_bIsFrameRateLocked = MFALSE;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::resetMvhdrRatio(MBOOL bReset)
{
    AE_LOG("[%s()] reset mvhdr ratio:%d->%d\n", __FUNCTION__, m_bResetMvhdrRatio, bReset);
    m_bResetMvhdrRatio = bReset;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AeMgr::getISPRegNormalPipe(MUINT32 RegAddr)
{
    MUINT32 ret = 0;
    MUINT32 retValue = 0;

    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
    if (pPipe == NULL)
    {
        CAM_LOGE("[%s()] Fail to create NormalPipe", __FUNCTION__);
    }
    else
    {
        ret = pPipe->getIspReg(RegAddr, 1, &retValue, MTRUE);
        pPipe->destroyInstance(LOG_TAG);
    }

    return retValue;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getCaptureMaxFPS()
{
    return m_rAEInitInput.rAENVRAM->rDevicesInfo.u4CapMaxFrameRate;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateCustParams(AE_Cust_Param_T *a_rCustomParam)
{
    memcpy(&m_AeCustParam, a_rCustomParam, sizeof(AE_Cust_Param_T));
    AE_LOG_IF(m_3ALogEnable, "[%s()] ID:%d\n", __FUNCTION__, m_AeCustParam.u4Id);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::EnableSuperNightShot(MINT32 i4SuperNightShot)
{
    AE_LOG_IF(m_3ALogEnable, "[%s()] Super Night Shot:%d->%d\n", __FUNCTION__, m_i4SuperNightShotMode, i4SuperNightShot);
    if (m_i4SuperNightShotMode != i4SuperNightShot)
    {
        AE_LOG("[%s()] Super Night Shot:%d->%d\n", __FUNCTION__, m_i4SuperNightShotMode, i4SuperNightShot);
        m_i4SuperNightShotMode = i4SuperNightShot;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AeMgr::getsensorDevID(ESensorDev_T eSensorDev)
{
    SensorStaticInfo sInfo;
    MINT32 i4SensorDevID;
    i4SensorDevID = NS3Av3::mapSensorIdxToDev(m_i4SensorIdx);
    // Get Sensor ID
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    pIHalSensorList->querySensorStaticInfo(i4SensorDevID, &sInfo);
    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d sensorID:%d\n", __FUNCTION__, eSensorDev, sInfo.sensorDevID);

    return sInfo.sensorDevID;
}
