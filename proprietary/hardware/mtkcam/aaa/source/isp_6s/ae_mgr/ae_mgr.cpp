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
    #define ENABLE_MY_LOG       (1)
#endif
#include <sys/stat.h>
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
#include <flash_param.h>
#include <ae_param.h>
//#include <CamDefs.h>
#include <faces.h>
#include <isp_tuning.h>
//#include <isp_mgr.h>
//#include <isp_tuning_mgr.h>
#include <dbg_aaa_param.h>
#include <dbg_ae_param.h>

//
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include "camera_custom_zvhdr.h"
#include "camera_custom_mvhdr3expo.h"
#include <kd_camera_feature.h>
#include <mtkcam/drv/IHalSensor.h>
#include <ae_config.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <aaa_scheduler.h>
#include <aaa_common_custom.h>
#include <time.h>
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#include "camera_custom_msdk.h"
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <private/IopipeUtils.h>
#include <nvbuf_util.h>
#include <ae_calc_if.h>


#include <android/sensor.h>                     // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorProvider.h>
#include <math.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>

static unsigned long long AEtimer;
static unsigned int NumofUsers;

#define EMLongestExp 500000
#define RealInitThrs 600000000 // 0.6s
// define log control
#define EN_3A_FLOW_LOG        1
// define TG information
#define TG_INT_AE_RATIO 0.6 // temp: to remove at p80
// define g/gyro info
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_LIGHT_POLLING_MS  20

#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

// for AE P-line index debounce at max index
#define MAX_INDEX1_STABLE_THRES 20
#define MAX_INDEX2_STABLE_THRES 10
#define ISO1_THRES_VALUE 2800
#define ISO2_THRES_VALUE 5600

#ifndef ABS
    #define ABS(x)                                                                  (((x) > 0) ? (x) : (-1*(x)))
#endif

using namespace NS3A;
using namespace NS3Av3;
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


AeMgr* AeMgr::s_pAeMgr = MNULL;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AeMgrDev : public AeMgr
{
public:
    static    AeMgr&    getInstance() {
        static AeMgrDev<eSensorDev> singleton;
        AeMgr::s_pAeMgr = &singleton;
        return singleton;
    }

    AeMgrDev() : AeMgr(eSensorDev) {}
    virtual ~AeMgrDev() {}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr &AeMgr::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev) {
        case ESensorDev_Main: //  Main Sensor
            return  AeMgrDev<ESensorDev_Main>::getInstance();
        case ESensorDev_MainSecond: //  Main Second Sensor
            return  AeMgrDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_Sub: //  Sub Sensor
            return  AeMgrDev<ESensorDev_Sub>::getInstance();
        case ESensorDev_SubSecond: //  Sub Sensor
            return  AeMgrDev<ESensorDev_SubSecond>::getInstance();
        default:
            CAM_LOGD("i4SensorDev = %d", i4SensorDev);
            if (AeMgr::s_pAeMgr) {
                return  *AeMgr::s_pAeMgr;
            } else {
                return  AeMgrDev<ESensorDev_Main>::getInstance();
            }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
AeMgr(ESensorDev_T eSensorDev)
    : m_pIAeCalc(NULL)
    , m_pIAeFlowCPU(NULL)
    , m_pIAeFlowCCU(NULL)
    , m_pIAeSettingCPU(NULL)
    , m_pIAeSettingCCU(NULL)
    , m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_1)
    , m_eSensorMode(ESensorMode_Preview)
    , m_eISPProfile(EIspProfile_Preview)
    , m_eCamScenarioMode(CAM_SCENARIO_PREVIEW)
    , m_u4AENVRAMIdx(CAM_SCENARIO_PREVIEW)
    , m_i4SensorIdx(0)
    , m_i4BVvalue(0)
    , m_i4DeltaBV(0)
    , m_i4AOECompBVvalue(0)
    , m_i4BVvalueWOStrobe(0)
    , m_i4EVvalue(0)
    , m_i4WaitVDNum(0)
    , m_i4RotateDegree(0)
    , m_i4ShutterDelayFrames(2)
    , m_i4SensorGainDelayFrames(2)
    , m_i4IspGainDelayFrames(0)
    , m_i4SensorCaptureDelayFrame(0)
    , m_i4TotalCaptureDelayFrame(3)
    , m_u4SmoothIspGain(0)
    , m_u4AECondition(0)
    , m_u4DynamicFrameCnt(0)
    , m_u4AFSGG1Gain(16)
    , m_bAESceneChanged(MFALSE)
    , m_bAELock(MFALSE)
    , m_bAPAELock(MFALSE)
    , m_bAFAELock(MFALSE)
    , m_bAlgoAPAELock(MFALSE)
    , m_bAlgoAFAELock(MFALSE)
    , m_bEnableAE(MFALSE)
    , m_bVideoDynamic(MTRUE)
    , m_bRealISOSpeed(MFALSE)
    , m_bAElimitor(MFALSE)
    , m_bAEStable(MFALSE)
    , m_bAEReadyCapture(MFALSE)
    , m_bLockExposureSetting(MFALSE)
    , m_bStrobeOn(MFALSE)
    , m_bAEMgrDebugEnable(MFALSE)
    , m_i4DgbRtvEnable(0)
    , m_bAECaptureUpdate(MFALSE)
    , m_bNeedCheckAEPline(MFALSE)
    , m_bCCTEnable(MFALSE)
    , m_eAEScene(LIB3A_AE_SCENE_AUTO)
    , m_eAEMode(LIB3A_AE_MODE_ON)
    , m_eAEState(MTK_CONTROL_AE_STATE_INACTIVE)
    , m_fEVCompStep(1)
    , m_i4EVIndex(0)
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
    , m_u4AEMgrStateCPU(AE_MGR_STATE_INIT)
    , m_u4AEMgrStateCCU(AE_MGR_STATE_INIT)
    , m_bForceResetCCUStable(MFALSE)
    , m_u4HwMagicNum(0)
    , m_u4RequestNum(0)
    , m_u4veLTCRate(0)
    , m_u4veLimitGain(0)
    , m_pAEPlineTable(NULL)
    , m_pAEMapPlineTable(NULL)
    , m_p3ANVRAM(NULL)
    , m_eAETargetMode(AE_MODE_NORMAL)
    , m_u4PrevExposureTime(0)
    , m_u4PrevSensorGain(0)
    , m_u4PrevIspGain(0)
    , m_ePrevAECamMode(LIB3A_AECAM_MODE_PHOTO)
    , m_3ALogEnable(0)
    , m_i4DgbLogE(0)
    , m_bAEOneShotControl(MFALSE)
    , bUpdateSensorAWBGain(MFALSE)
    , bApplyAEPlineTableByCCT(MFALSE)
    , m_u4UpdateFrameRate_x10(LIB3A_AE_FRAMERATE_MODE_30FPS)
    , m_bMultiCap(MFALSE)
    , m_bCShot(MFALSE)
    , m_bLockBlackLevel(MFALSE)
    , m_u4WOFDCnt (0)
    , m_bFDenable(MFALSE)
    , m_u4StableYValue(0)
    , m_bzcHDRshot(MFALSE)
    , m_bHDRshot(MFALSE)
    , m_u4VsyncCnt(0)
    , m_Lock()
    , m_LockControlCCU()
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
    , m_u4FinerEVIdxBase(1)
    , m_bFirstLaunchCam(MTRUE)
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
    , m_pAAOStatBuf(NULL)
    , m_u4EMVHDRratio(0)
    , m_u4CurVHDRratio(100)
    , m_bVHDRChangeFlag(MFALSE)
    , m_bEnSWBuffMode(MFALSE)
    , m_i4SMSubSamples(1)
    , m_u4HFSMSkipFrame(0)
    , m_u4HDRcountforrevert(0)
    , m_u4AEHDRMode(0)
    , m_u4MVHDRRatio_x100(100)
    , m_pMVHDR3ExpoStatBuf(NULL)
    , m_bCCUAEFlag(MTRUE)
    , m_pCCUresultCB(NULL)
    , m_bIsCCUStart(MFALSE)
    , m_bCCUIsSensorSupported(MFALSE)
    , m_i4ZoomRatio(0)
    , m_bAFTouchROISet(MFALSE)
    , m_bAETrueLaunch(MTRUE)
    , m_bPlineMappingChange(MFALSE)
    , m_bGryoVd(MFALSE)
    , m_bAcceVd(MFALSE)
    , m_bLightVd(MFALSE)
    , m_bIsCCUAEInit(MFALSE)
    , m_pIdxMgr(IdxMgr::createInstance(static_cast<ESensorDev_T>(eSensorDev)))
    , m_u4ISOIdx1StableCount(0)
    , m_u4ISOIdx2StableCount(0)
    , m_u4Idx1StableThres(MAX_INDEX1_STABLE_THRES)
    , m_u4Idx2StableThres(MAX_INDEX2_STABLE_THRES)
    , m_u4ISOIdx1Value(ISO1_THRES_VALUE)
    , m_u4ISOIdx2Value(ISO2_THRES_VALUE)
    , m_u4ISO1IdxStatus(0)
    , m_u4ISO2IdxStatus(0)
    , m_bIsCPUNeedDebugInfo(MFALSE)
    , m_bUpdateAELockIdx(MFALSE)
    , m_bAlgoResultUpdate(MFALSE)
    , m_bAdbAELock(MFALSE)
    , m_bAdbAEPreviewUpdate(MFALSE)
    , m_u4IndexTestCnt(0)
    , m_eAlgoType(E_AE_ALGO_CPU_DEFAULT)
    , m_eSettingType(E_AE_SETTING_CPU)
    , m_u4ExposureMode(eAE_EXPO_TIME)
    , m_u4DynamicHFPS(30)
    , m_u4BinSumRatio(1)
    , m_bSkipCalAE(MFALSE)
    , m_bFDSkipCalAE(MFALSE)
    , m_bShutterISOPriorityMode(MFALSE)
    , m_bManualByShutterISOPriorityMode(MFALSE)
    , m_bSuperNightShotMode(MFALSE)
    , m_bInitAE(MFALSE)
    , m_i4ISP6LTMidx(-1)
    , m_bMainFlashON(MFALSE)
    , m_u4AdbISOspeed(0)
{
    mEngFileName = new char[200];
    mEngFileName[0]=0;

    memset(&m_AeMgrCCTConfig, 0, sizeof(AE_CCT_CFG_T));
    memset(&m_eZoomWinInfo, 0, sizeof(EZOOM_WINDOW_T));
    memset(&m_eAEMeterArea, 0, sizeof(CCU_AEMeteringArea_T));
    memset(&m_eAEFDArea, 0, sizeof(CCU_AEMeteringArea_T));
    memset(&m_eAEFDAreaBackup4Flash, 0, sizeof(CCU_AEMeteringArea_T));
    memset(&m_CurrentPreviewTable, 0, sizeof(strAETable));
    memset(&m_CurrentCaptureTable, 0, sizeof(strAETable));
    memset(&m_CurrentStrobetureTable, 0, sizeof(strAETable));
    memset(&m_PreviewTableF, 0, sizeof(strFinerEvPline));
    memset(&m_CaptureTableF, 0, sizeof(strFinerEvPline));
    memset(&mPreviewMode, 0, sizeof(AE_MODE_CFG_T));
    memset(&mPreviewModeBackup, 0, sizeof(AE_MODE_CFG_T));
    memset(&m_strHDROutputInfo, 0, sizeof(Hal3A_HDROutputParam_T));
    memset(&m_backupMeterArea, 0, sizeof(CameraMeteringArea_T));
    memset(&m_rAEInitInput, 0, sizeof(AE_INITIAL_INPUT_T));
    memset(&m_rAFAEOutput, 0, sizeof(CCU_strAEOutput));

    memset(&m_rAEInfoPreview, 0, sizeof(AE_INFO_T));
    memset(&m_rAEInfoCapture, 0, sizeof(AE_INFO_T));
    memset(&m_rAEAlgoOutput, 0, sizeof(strAERealSetting));
    memset(&m_rAESettingPreview, 0, sizeof(strAERealSetting));
    memset(&m_rAESettingCapture, 0, sizeof(strAERealSetting));
    memset(&m_rAESettingAE2AF,   0, sizeof(strAERealSetting));
    memset(&m_rAESettingBackup,  0, sizeof(strAERealSetting));
    memset(&m_rADBAESetting,     0, sizeof(strAERealSetting));

    memset(&m_rAEStatCfg, 0, sizeof(AE_STAT_PARAM_T));
    memset(&m_rAEConfigInfo, 0, sizeof(AE_CONFIG_INFO_T));
    memset(&m_rSensorResolution[0], 0, 2*sizeof(SENSOR_RES_INFO_T));
    memset(&m_rAEPLineLimitation, 0, sizeof(AE_PLINE_LIMITATION_T));
    memset(&m_rSensorVCInfo, 0, sizeof(SENSOR_VC_INFO_T));
    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));
    memset(&m_rSensorCurrentInfo, 0, sizeof(AE_SENSOR_PARAM_T));
    memset(&m_strAAOProcInfo, 0, sizeof(CCU_AAO_PROC_INFO_T));
    memset(&m_rCusAEStat, 0, sizeof(CCU_AE_STAT));
    memset(&m_rAECustPARAM, 0, sizeof(AE_CUST_PARAM_T));
    memset(&m_AEPerFrameInfo, 0, sizeof(AE_PERFRAME_INFO_T));

    memset(&m_u4AESettingEXP, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AESettingAFE, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AESettingISP, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingEXP_LE, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingEXP_ME, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingEXP_SE, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingAFE_LE, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingAFE_ME, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingAFE_SE, 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
    memset(&m_u4AEVHDRSettingISP   , 0, MAX_AE_SETTING_NUM*sizeof(MUINT32));
	memset(&m_LTMParam             , 0, sizeof(ISP_AUTO_LTM_TUNING_T));
    memset(&m_AeCustParam, 0, sizeof(AE_Cust_Param_T));
    memset(&m_AePriorityParam, 0, sizeof(CCU_AE_PRIORITY_INFO));

    m_i4StereoDenoiserto[0] = m_i4StereoDenoiserto[1] = 1000;
    m_backupMeterArea.u4Count = 1;
    m_isAeMeterAreaEn=1;
    m_rAEPLineLimitation.u4IncreaseISO_x100 = 100;
    m_rAEPLineLimitation.u4IncreaseShutter_x100 = 100;
    m_pIAeFlowCPU = IAeFlow::getInstance(IAeFlow::E_AE_FLOW_TYPE_DFT ,m_eSensorDev);
    m_pIAeFlowCCU = IAeFlow::getInstance(IAeFlow::E_AE_FLOW_TYPE_CCU ,m_eSensorDev);
    m_pIAeSettingCPU = IAeSetting::getInstance(IAeSetting::E_AE_SETTING_TYPE_DFT ,m_eSensorDev);
    m_pIAeSettingCCU = IAeSetting::getInstance(IAeSetting::E_AE_SETTING_TYPE_CCU ,m_eSensorDev);
    m_i4AcceInfo[0] = m_i4AcceInfo[1] = m_i4AcceInfo[2] = 0;
    m_i4GyroInfo[0] = m_i4GyroInfo[1] = m_i4GyroInfo[2] = 0;
    mpSensorProvider = nullptr;

    AE_LOG( "[AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AeMgr::
~AeMgr()
{
    delete []mEngFileName;
    AE_LOG( "[~AeMgr]\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    AE_LOG( "[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d CamMode:%d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode, m_eAECamMode);

    m_bRealISOSpeed = 0;
    m_i4WaitVDNum = 0;
    m_i4EVIndex = 0;
    m_eAEEVcomp = LIB3A_AE_EV_COMP_00;

    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));

    switch(m_eCamMode) {
        case eAppMode_EngMode:        //  Engineer Mode
            m_bRealISOSpeed = 1;
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        case eAppMode_ZsdMode:        //  ZSD Mode
            m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
            break;
        case eAppMode_VideoMode:     //  Video Mode
            m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
            break;
        case eAppMode_StereoMode:        //  S3D Mode
        case eAppMode_PhotoMode:     //  Photo Mode
        case eAppMode_DefaultMode:   //  Default Mode
        case eAppMode_AtvMode:         //  ATV Mode
        case eAppMode_VtMode:           //  VT Mode
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
    m_u4EMVHDRratio= atoi(value);
    m_u4CurVHDRratio = 100;
    m_rAESettingPreview.HdrEvSetting.u4AECHdrRatio = 0;

    property_get("vendor.debug.ae_mgr.enable", value, "0");
    m_bAEMgrDebugEnable = atoi(value);

    property_get("vendor.debug.ae.rtv.enable", value, "0");
    m_i4DgbRtvEnable = atoi(value);
    if (m_i4DgbRtvEnable)
    {
        m_rtvId[0] = property_get_int32("vendor.debug.ae.rtv.id_1", E_RTV_AE_SHUTTER);
        m_rtvId[1] = property_get_int32("vendor.debug.ae.rtv.id_2", E_RTV_AE_ISO);
        m_rtvId[2] = property_get_int32("vendor.debug.ae.rtv.id_3", E_RTV_AE_LV);
        m_rtvId[3] = property_get_int32("vendor.debug.ae.rtv.id_4", E_RTV_AE_STATE);
        m_rtvId[4] = property_get_int32("vendor.debug.ae.rtv.id_5", E_RTV_AE_SCENARIO_MODE);
        m_rtvId[5] = property_get_int32("vendor.debug.ae.rtv.id_6", E_RTV_AE_CWV);
        m_rtvId[6] = property_get_int32("vendor.debug.ae.rtv.id_7", E_RTV_AE_INDEXF);
        m_rtvId[7] = property_get_int32("vendor.debug.ae.rtv.id_8", E_RTV_AE_STABLE);
    }

    m_u4Idx1StableThres = property_get_int32("vendor.debug.idx1.stable.thres", MAX_INDEX1_STABLE_THRES);
    m_u4Idx2StableThres = property_get_int32("vendor.debug.idx2.stable.thres", MAX_INDEX2_STABLE_THRES);
    m_u4ISOIdx1Value = property_get_int32("vendor.debug.iso1.thres.value", ISO1_THRES_VALUE);
    m_u4ISOIdx2Value = property_get_int32("vendor.debug.iso2.thres.value", ISO2_THRES_VALUE);
    m_u4ISOIdx1StableCount = m_u4Idx1StableThres;
    m_u4ISOIdx2StableCount = m_u4Idx2StableThres;

    m_pIAeSettingCPU->init();
    m_pIAeSettingCCU->init();
    PreviewAEInit(i4SensorIdx, rParam);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::PreviewAEInit(MINT32 i4SensorIdx, Param_T &rParam)
{
    MRESULT err;
    MINT32 i4SutterDelay, i4SensorGainDelay, i4IspGainDelay;

    // set sensor type
    m_i4SensorIdx = i4SensorIdx;

    // set sensor initial
    err = AAASensorMgr::getInstance().init(m_i4SensorIdx);
    if (FAILED(err)) {
        CAM_LOGE("AAASensorMgr::getInstance().init fail i4SensorDev:%d\n", m_i4SensorIdx);
        return err;
    }

    AAASensorMgr::getInstance().getSensorSyncinfo(m_eSensorDev, &i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay, &m_i4SensorCaptureDelayFrame);
    AAASensorMgr::getInstance().getSensorMaxFrameRate(m_eSensorDev, m_rSensorMaxFrmRate);

    if (i4SutterDelay != i4SensorGainDelay){
        m_pIAeSettingCPU->controltrigger(E_AE_SETTING_CPU_SENSORGAINTHREAD);
        i4SensorGainDelay = i4SutterDelay;
    }

    if(i4IspGainDelay >= m_i4SensorCaptureDelayFrame) {
        m_i4TotalCaptureDelayFrame = i4IspGainDelay + 1;
    } else {
        m_i4TotalCaptureDelayFrame = m_i4SensorCaptureDelayFrame;
    }

    AE_LOG( "[PreviewAEInit] Long capture delay frame :%d %d %d\n", m_i4TotalCaptureDelayFrame, m_i4SensorCaptureDelayFrame, i4IspGainDelay);

    if((i4SutterDelay <= 5) && (i4SensorGainDelay <= 5) && (i4IspGainDelay <= 5)) {
        m_i4ShutterDelayFrames = i4SutterDelay;
        m_i4SensorGainDelayFrames = i4SensorGainDelay;
        if(i4IspGainDelay >= 2) {
            m_i4IspGainDelayFrames = i4IspGainDelay - ISP_GAIN_DELAY_OFFSET; // for CQ0 2 delay frame
        } else {
            m_i4IspGainDelayFrames = 0;
        }

        AE_LOG( "[PreviewAEInit] SensorDelayInfo-> AeMgrDelayInfo (shutter/sensor gain/isp gain): %d/%d/%d -> %d/%d/%d  \n",
            i4SutterDelay, i4SensorGainDelay, i4IspGainDelay, m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames);
    } else {
        AE_LOG( "[PreviewAEInit] Delay info is incorrectly :%d %d %d\n", i4SutterDelay, i4SensorGainDelay, i4IspGainDelay);
        m_i4ShutterDelayFrames = 0;
        m_i4SensorGainDelayFrames = 0;
        m_i4IspGainDelayFrames = 0; // for CQ0 2 delay frame
    }

    // Get sensor resolution
    err = getSensorResolution();
    if (FAILED(err)) {
        CAM_LOGE("getSensorResolution() fail\n");
        return err;
    }

    // Get NVRAM data
    err = getNvramData(m_eSensorDev);
    if (FAILED(err)) {
        CAM_LOGE("getNvramData() fail\n");
        return err;
    }

    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
        err = AAASensorMgr::getInstance().getSensorVCinfo(m_eSensorDev, &m_rSensorVCInfo);
        if (FAILED(err)) {
            CAM_LOGE("getSensorVCinfo() fail\n");
            return err;
        }
    }

    // Init AE
    err = AEInit(rParam);
    if (FAILED(err)) {
        CAM_LOGE("AEInit() fail\n");
        return err;
    }

    AE_LOG( "[PreviewAEInit] m_eSensorDev:%d m_i4SensorIdx:%d AEconfig:%d/%d/%d/%d/%d/%d AEYCoefR/G/B:%d/%d/%d\n",
             m_eSensorDev, m_i4SensorIdx, m_rAEConfigInfo.u4BlockNumW, m_rAEConfigInfo.u4BlockNumH,
             m_rAEConfigInfo.bEnableHDRYConfig, m_rAEConfigInfo.bEnableOverCntConfig, m_rAEConfigInfo.bEnableTSFConfig, m_rAEConfigInfo.bDisablePixelHistConfig,
             m_rAEConfigInfo.u4AEYCoefR, m_rAEConfigInfo.u4AEYCoefG, m_rAEConfigInfo.u4AEYCoefB);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::Start()
{
    AE_LOG( "[%s() +] m_eSensorDev: %d\n", __FUNCTION__, m_eSensorDev);

    // Define m_eCamMode
    m_bRealISOSpeed = 0;
    m_i4LightInfo = MFALSE;
    switch(m_eCamMode) {
        case eAppMode_EngMode:       //  Engineer Mode
            m_bRealISOSpeed = 1;
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
        case eAppMode_ZsdMode:       //  ZSD Mode
            m_eAECamMode = LIB3A_AECAM_MODE_ZSD;
            break;
        case eAppMode_VideoMode:     //  Video Mode
            m_eAECamMode = LIB3A_AECAM_MODE_VIDEO; // for board support package test used.
            break;
        case eAppMode_StereoMode:    //  S3D Mode
        case eAppMode_PhotoMode:     //  Photo Mode
        case eAppMode_DefaultMode:   //  Default Mode
        case eAppMode_AtvMode:       //  ATV Mode
        case eAppMode_VtMode:        //  VT Mode
        case eAppMode_FactoryMode:
        default:
            m_eAECamMode = LIB3A_AECAM_MODE_PHOTO;
            break;
    }
    AE_LOG( "[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_eSensorMode: %d, CamMode: %d, m_eCamMode: %d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_eSensorMode, m_eAECamMode, m_eCamMode);

    // Sensor max framerate will be changed when changing sensor mode
    AAASensorMgr::getInstance().getSensorMaxFrameRate(m_eSensorDev, m_rSensorMaxFrmRate);

    // Get sensor resolution
    MRESULT err = getSensorResolution();
    if (FAILED(err)) {
        CAM_LOGE("getSensorResolution() fail\n");
        return err;
    }

    if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
        err = AAASensorMgr::getInstance().getSensorVCinfo(m_eSensorDev, &m_rSensorVCInfo);
        if (FAILED(err)) {
            CAM_LOGE("getSensorVCinfo() fail\n");
            return err;
        }
    }

    // Query TG info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("ae_mgr", m_i4SensorIdx);
    SensorDynamicInfo rSensorDynamicInfo;
    switch(m_eSensorDev) {
        case ESensorDev_Main:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
            break;
        case ESensorDev_SubSecond:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorDynamicInfo);
            break;
        default:    //  Shouldn't happen.
            CAM_LOGE("Invalid sensor device: %d", m_eSensorDev);
    }
    if(pIHalSensor) {
        pIHalSensor->destroyInstance("ae_mgr");
    }
    AE_LOG_IF(m_3ALogEnable,"TG = %d, pixel mode = %d, Clear Metering Area\n", rSensorDynamicInfo.TgInfo, rSensorDynamicInfo.pixelMode);

    switch(rSensorDynamicInfo.TgInfo){
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
        case CAM_TG_3:
            m_eSensorTG = ESensorTG_3;
            break;
        default:
            CAM_LOGE("rSensorDynamicInfo.TgInfo = %d", rSensorDynamicInfo.TgInfo);
            return MFALSE;
    }

    AE_LOG_IF(m_3ALogEnable,"[Start] m_eSensorDev:%d m_eSensorTG:%d m_i4SensorIdx:%d m_eISPProfile:%d,  LTM index=%d, clipping_thd_perc/high_bound_perc = [%d/%d] \n", m_eSensorDev, m_eSensorTG, m_i4SensorIdx, m_eISPProfile,
              m_i4ISP6LTMidx,m_LTMParam.clipping_thd_perc,m_LTMParam.high_bound_perc);

    // AE statistics and histogram config
    AAA_TRACE_D("StartAEStatConfig");
    m_pIAeCalc->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);
    ISP_AE_CONFIG_T::getInstance(m_eSensorDev).AEConfig(&m_rAEStatCfg, &m_rAEConfigInfo, &m_sAEResultConfig);
    AAA_TRACE_END_D;

    m_bAEStable = MFALSE;
    m_bNeedCheckAEPline = MTRUE;
    m_bMainFlashON = MFALSE;
    m_i4WaitVDNum = 0;
    m_u4IndexTestCnt = 0;
    m_u4DynamicHFPS = 30;
    m_bForceResetCCUStable = MFALSE;
    memset(&m_SensorQueueCtrl, 0, sizeof(AE_SENSOR_QUEUE_CTRL_T));
    memset(&m_eAEMeterArea,0,sizeof(CCU_AEMeteringArea_T));
    m_eAEMeterArea.u4Count =1;
    m_u4StableYValue = 0;
    m_i4AEAutoFlickerMode = (MINT32)(m_eAEAutoFlickerMode);
    m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
    m_u4AEMgrStateCPU = m_u4AEMgrStateCCU = AE_MGR_STATE_INIT;

    /* CCU initialization */
    AE_FLOW_CCU_INIT_INFO_T rCCUInitInfo;
    initializeCCU(&rCCUInitInfo);

    /* AE algo type decision */
    MINT32 i4CustomFlow = property_get_int32("vendor.debug.ae.custom.flow", 0);
    if (i4CustomFlow) { // customizes AE algo, decided by sensor EEPROM
        m_eAlgoType    = E_AE_ALGO_CPU_CUSTOM;
        m_eSettingType = E_AE_SETTING_CPU;
        initializeCPUAE(m_eAlgoType);
        startAESetting (m_eSettingType);
        AE_LOG("[%s()] init AE algo: E_AE_ALGO_CPU_CUSTOM, start AE setting: E_AE_SETTING_CPU",__FUNCTION__);
    }
    else if (!m_bCCUAEFlag) { // default CPU AE algo
        m_eAlgoType    = E_AE_ALGO_CPU_DEFAULT;
        m_eSettingType = E_AE_SETTING_CPU;
        initializeCPUAE(m_eAlgoType);
        startAESetting (m_eSettingType);
        AE_LOG("[%s()] init AE algo: E_AE_ALGO_CPU_DEFAULT, start AE setting: E_AE_SETTING_CPU",__FUNCTION__);
    }
    else { // default CCU AE algo
        m_eAlgoType    = E_AE_ALGO_CCU_DEFAULT;
        m_eSettingType = E_AE_SETTING_CCU_MANUAL;
        initializeCCUAE(&rCCUInitInfo);
        startAESetting (m_eSettingType);
        AE_LOG("[%s()] init AE algo: E_AE_ALGO_CCU_DEFAULT, start AE setting: E_AE_SETTING_CCU_MANUAL",__FUNCTION__);
    }
    UpdateSensorISPParams(AE_FLOW_STATE_INIT, m_rAESettingPreview);

    char value2[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 TestPattern = 0;
    property_get("vendor.debug.ae.TestPattern", value2, "0");
    TestPattern = atoi(value2);
    if(TestPattern){
      AAASensorMgr::getInstance().enableSensorTestPattern(m_eSensorDev);
    }

    AE_LOG( "[%s() -] m_eSensorDev: %d, m_u4BinSumRatio: %d\n", __FUNCTION__, m_eSensorDev, m_u4BinSumRatio);

    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::uninit()
{
    MRESULT err;

    err = AAASensorMgr::getInstance().uninit();
    if (FAILED(err)) {
        CAM_LOGE("AAASensorMgr::getInstance().uninit fail\n");
        return err;
    }
    m_pIAeSettingCPU->uninit();
    m_pIAeSettingCCU->uninit();
    if(m_pIAeCalc != NULL) {
        m_pIAeCalc->destroyInstance();
        m_pIAeCalc = NULL;
    }
    // uninit Gyro
    if( mpSensorProvider != NULL)
    {
       mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
       mpSensorProvider->disableSensor(SENSOR_TYPE_ACCELERATION);
       mpSensorProvider->disableSensor(SENSOR_TYPE_LIGHT);
       mpSensorProvider = NULL;
    }
    if (m_bCCUAEFlag)
    {
        m_pIAeFlowCCU->stop();
        m_pIAeFlowCCU->uninit();
    }

    // Save uninit AEtime
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    AEtimer = (unsigned long long)time.tv_sec*1000000000 + (unsigned long long)time.tv_nsec;
    if(NumofUsers>0)
        NumofUsers--;
    AE_LOG( "[AeMgr::uninit] AETimer %lld/%lld/%lld, NumOfUsers=%d\n", AEtimer, (unsigned long long)time.tv_sec, (unsigned long long)time.tv_nsec, NumofUsers);

    AE_LOG( "[%s()] reset m_eAEScene from eAEScene:%d to LIB3A_AE_SCENE_AUTO \n",__FUNCTION__, m_eAEScene);
    m_eAEScene = LIB3A_AE_SCENE_AUTO;

    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::Stop()
{
    m_pIAeSettingCPU->stop();
    m_pIAeSettingCCU->stop();
    m_u4HwMagicNum = 0;
    m_u4RequestNum = 0;

    if (m_bCCUAEFlag){
     m_pIAeFlowCCU->pause();
     uninitializeCCU();
    }

    if(m_pIAeFlowCPU){
        m_pIAeFlowCPU->stop();
    }

    m_AePriorityParam.eMode = CCU_AE_PRIORITY_OFF;
    enableAEManualPline(EAEManualPline_ShtterISOPriority, MFALSE);
    m_bShutterISOPriorityMode = m_bManualByShutterISOPriorityMode = m_bSuperNightShotMode = MFALSE;
    AE_LOG( "[%s()] reset shutter/ISO priority & super night shot mode \n",__FUNCTION__, m_eAEScene);

    AE_LOG( "[%s()] reset m_eAEScene from eAEScene:%d to LIB3A_AE_SCENE_AUTO \n",__FUNCTION__, m_eAEScene);
    m_eAEScene = LIB3A_AE_SCENE_AUTO;

    AE_LOG( "[%s()] i4SensorDev:%d \n",__FUNCTION__, m_eSensorDev);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea)
{
    std::lock_guard<std::mutex> lock((m_Lock));
    if(CUST_ENABLE_TOUCH_AE() == MFALSE) {
        AE_LOG( "[%s()] i4SensorDev:%d line:%d The set face AE is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_TOUCH_AE());
        return S_AE_OK;
    }
    AE_LOG_IF(m_3ALogEnable,"setAEMeteringArea m_isAeMeterAreaEn=%d",m_isAeMeterAreaEn);
    CameraMeteringArea_T meterArea;
    memcpy(&m_backupMeterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));
    memcpy(&meterArea, sNewAEMeteringArea, sizeof(CameraMeteringArea_T));

    if(m_isAeMeterAreaEn==0)
    {
      memset(&meterArea, 0, sizeof(CameraMeteringArea_T));
      meterArea.u4Count = 1;
    }


    MUINT32 i;
    MBOOL bAreaChage = MFALSE;
    MUINT32 u4AreaCnt;
    CameraMeteringArea_T *sAEMeteringArea = &meterArea;

    if (sAEMeteringArea->u4Count <= 0) {
        return S_AE_OK;
    } else if (sAEMeteringArea->u4Count > MAX_METERING_AREAS) {
        CAM_LOGE("The AE Metering area cnt error: %d\n", sAEMeteringArea->u4Count);
        return E_AE_UNSUPPORT_MODE;
    }

    u4AreaCnt = sAEMeteringArea->u4Count;

    for(i=0; i<u4AreaCnt; i++) {

        AE_LOG_IF(m_3ALogEnable,"[%s():New] AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n",__FUNCTION__, i, sAEMeteringArea->rAreas[i].i4Left, sAEMeteringArea->rAreas[i].i4Right, sAEMeteringArea->rAreas[i].i4Top, sAEMeteringArea->rAreas[i].i4Bottom, sAEMeteringArea->rAreas[i].i4Weight);
        if ((sAEMeteringArea->rAreas[i].i4Right>sAEMeteringArea->rAreas[i].i4Left) &&
            (sAEMeteringArea->rAreas[i].i4Bottom>sAEMeteringArea->rAreas[i].i4Top) &&
            (m_u4Hbinwidth!=0 && m_u4Hbinheight!=0)){
            // Mapping AE region from Hbin size to [-1000,1000]
            sAEMeteringArea->rAreas[i].i4Left   = sAEMeteringArea->rAreas[i].i4Left*2000  /m_u4Hbinwidth - 1000;
            sAEMeteringArea->rAreas[i].i4Right  = sAEMeteringArea->rAreas[i].i4Right*2000 /m_u4Hbinwidth - 1000;
            sAEMeteringArea->rAreas[i].i4Top    = sAEMeteringArea->rAreas[i].i4Top*2000   /m_u4Hbinheight - 1000;
            sAEMeteringArea->rAreas[i].i4Bottom = sAEMeteringArea->rAreas[i].i4Bottom*2000/m_u4Hbinheight - 1000;
            // check the count of side reaching boundary
            int counter = 0;
            // check if out-of-boundary
            if (sAEMeteringArea->rAreas[i].i4Left   < -1000)  {sAEMeteringArea->rAreas[i].i4Left   = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Right  < -1000)  {sAEMeteringArea->rAreas[i].i4Right  = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Top    < -1000)  {sAEMeteringArea->rAreas[i].i4Top    = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Bottom < -1000)  {sAEMeteringArea->rAreas[i].i4Bottom = -1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Left   > 1000)  {sAEMeteringArea->rAreas[i].i4Left   = 1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Right  > 1000)  {sAEMeteringArea->rAreas[i].i4Right  = 1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Top    > 1000)  {sAEMeteringArea->rAreas[i].i4Top    = 1000; counter++;}
            if (sAEMeteringArea->rAreas[i].i4Bottom > 1000)  {sAEMeteringArea->rAreas[i].i4Bottom = 1000; counter++;}

            // if counter > 2, the touch area is wrong
            if (counter > 2){
                AE_LOG_IF(m_3ALogEnable, "[%s()] touch area more than two sides reach boundary, counter:%d\n", __FUNCTION__, counter);
            }
            else if((sAEMeteringArea->rAreas[i].i4Left != m_eAEMeterArea.rAreas[i].i4Left) ||
                    (sAEMeteringArea->rAreas[i].i4Right != m_eAEMeterArea.rAreas[i].i4Right) ||
                    (sAEMeteringArea->rAreas[i].i4Top != m_eAEMeterArea.rAreas[i].i4Top) ||
                    (sAEMeteringArea->rAreas[i].i4Bottom != m_eAEMeterArea.rAreas[i].i4Bottom)) {
                AE_LOG("[%s()] Previous AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n",__FUNCTION__, i, m_eAEMeterArea.rAreas[i].i4Left, m_eAEMeterArea.rAreas[i].i4Right, m_eAEMeterArea.rAreas[i].i4Top, m_eAEMeterArea.rAreas[i].i4Bottom, m_eAEMeterArea.rAreas[i].i4Weight);
                AE_LOG("[%s()] Modified AE meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d Weight:%d\n",__FUNCTION__, i, sAEMeteringArea->rAreas[i].i4Left, sAEMeteringArea->rAreas[i].i4Right, sAEMeteringArea->rAreas[i].i4Top, sAEMeteringArea->rAreas[i].i4Bottom, sAEMeteringArea->rAreas[i].i4Weight);
                m_eAEMeterArea.rAreas[i].i4Left = sAEMeteringArea->rAreas[i].i4Left;
                m_eAEMeterArea.rAreas[i].i4Right = sAEMeteringArea->rAreas[i].i4Right;
                m_eAEMeterArea.rAreas[i].i4Top = sAEMeteringArea->rAreas[i].i4Top;
                m_eAEMeterArea.rAreas[i].i4Bottom = sAEMeteringArea->rAreas[i].i4Bottom;
                m_eAEMeterArea.rAreas[i].i4Weight = sAEMeteringArea->rAreas[i].i4Weight;
                bAreaChage = MTRUE;
            }
        }
        else{
            m_bAFTouchROISet = MFALSE;
        }
    }
    if(bAreaChage == MTRUE) {
        m_bAFTouchROISet = MTRUE;
        m_eAEMeterArea.u4Count = u4AreaCnt;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setFDenable(MBOOL bFDenable)
{
    if (bFDenable != m_bFDenable){
        AE_LOG( "[%s()] FDenable is from %d -> %d\n", __FUNCTION__, m_bFDenable, bFDenable);
        m_bFDenable = bFDenable;
        m_u4WOFDCnt = 0 ;
    }
    return S_AE_OK;
}
MRESULT AeMgr::getAEFDMeteringAreaInfo(std::vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    vecOut.clear();
    MINT32 i4totalnumber = m_backupMeterArea.u4Count + m_eAEFDArea.u4Count;
    MINT32 u4AETouchCnt = m_backupMeterArea.u4Count;
    MINT32 u4AEFDCnt = m_eAEFDArea.u4Count;
    /*
     * The order of vector vecOut is :
     * type(0: Face AE) -> number of ROI -> left of ROI1 -> top of ROI1 -> right of ROI1 -> bottom of ROI1 -> result of ROI1 -> left of ROI2 -> top of ROI2 -> right of ROI2 -> bottom of ROI2 -> result of ROI2 -> etc.
     */
    i4totalnumber = u4AETouchCnt + u4AEFDCnt;
    if (i4totalnumber != 0)
    {
        MINT32 type = 0;
        vecOut.push_back(type);
        MINT32 number = static_cast<MINT32>(i4totalnumber);
        vecOut.push_back(number);
        // Touch AE metering area info
        for (MINT32 ifaceIdx = 0; ifaceIdx < u4AETouchCnt; ifaceIdx ++)
        {
            MINT32 i4Left   = (m_eAEMeterArea.rAreas[ifaceIdx].i4Left + 1000)   * i4tgwidth  / 2000;
            MINT32 i4Right  = (m_eAEMeterArea.rAreas[ifaceIdx].i4Right + 1000)  * i4tgwidth  / 2000;
            MINT32 i4Top    = (m_eAEMeterArea.rAreas[ifaceIdx].i4Top + 1000)    * i4tgheight / 2000;
            MINT32 i4Bottom = (m_eAEMeterArea.rAreas[ifaceIdx].i4Bottom + 1000) * i4tgheight / 2000;
            MINT32 i4result = 0;

            vecOut.push_back(i4Left);
            vecOut.push_back(i4Top);
            vecOut.push_back(i4Right);
            vecOut.push_back(i4Bottom);
            vecOut.push_back(i4result);
        }
        // FD AE metering area info
        for (MINT32 ifaceIdx = 0; ifaceIdx < u4AEFDCnt; ifaceIdx ++)
        {
            MINT32 i4Left   = (m_eAEFDArea.rAreas[ifaceIdx].i4Left + 1000)   * i4tgwidth  / 2000;
            MINT32 i4Right  = (m_eAEFDArea.rAreas[ifaceIdx].i4Right + 1000)  * i4tgwidth  / 2000;
            MINT32 i4Top    = (m_eAEFDArea.rAreas[ifaceIdx].i4Top + 1000)    * i4tgheight / 2000;
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
    std::lock_guard<std::mutex> lock((m_Lock));
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    MINT8 ifaceIdx = 0;

    if(CUST_ENABLE_FACE_AE() == MTRUE) {
        if(pFaces == NULL) {
            AE_LOG( "[%s()] FD window is NULL pointer 1\n", __FUNCTION__);
                return S_AF_OK;
        }

        if((pFaces->faces) == NULL) {
            AE_LOG( "[%s()] FD window is NULL pointer 2\n", __FUNCTION__);
            return S_AF_OK;
        }
        //First time with face to without face
        if (m_eAEFDArea.u4Count &&(pFaces->number_of_faces == 0)){
            m_u4WOFDCnt = 0 ;
            AE_LOG( "[%s()] First frame without face Count %d ->%d \n", __FUNCTION__,m_eAEFDArea.u4Count,pFaces->number_of_faces);
        }
        CCU_AEMeteringArea_T eAEFDArea;
        memset(&eAEFDArea, 0, sizeof(CCU_AEMeteringArea_T));

        if(m_bFDenable == MTRUE){
            // Define FD area number

            if(pFaces->number_of_faces > MAX_AE_METER_AREAS) {
                eAEFDArea.u4Count = MAX_AE_METER_AREAS;
            } else {
                eAEFDArea.u4Count = pFaces->number_of_faces;
            }

            //eAEFDArea.i8TimeStamp = pFaces->timestamp;

            for(ifaceIdx=0; (MUINT32)ifaceIdx < eAEFDArea.u4Count; ifaceIdx++) {
                eAEFDArea.rAreas[ifaceIdx].i4Left = pFaces->faces[ifaceIdx].rect[0];
                eAEFDArea.rAreas[ifaceIdx].i4Right = pFaces->faces[ifaceIdx].rect[2];
                eAEFDArea.rAreas[ifaceIdx].i4Top = pFaces->faces[ifaceIdx].rect[1];
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
                if (pFaces->fa_cv[ifaceIdx] > 0) {
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
                eAEFDArea.rAreas[ifaceIdx].i4Left   = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Left*2000/(i4tgwidth);
                eAEFDArea.rAreas[ifaceIdx].i4Right  = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Right*2000/(i4tgwidth);
                eAEFDArea.rAreas[ifaceIdx].i4Top    = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Top*2000/(i4tgheight);
                eAEFDArea.rAreas[ifaceIdx].i4Bottom = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Bottom*2000/(i4tgheight);

                if(pFaces->fa_cv[ifaceIdx] > 0)
                {
                    // left eye
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][0] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][0]*2000/(i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][1] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][1]*2000/(i4tgheight);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][2] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][2]*2000/(i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][3] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[0][3]*2000/(i4tgheight);
                    // right eye
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][0] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][0]*2000/(i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][1] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][1]*2000/(i4tgheight);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][2] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][2]*2000/(i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][3] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[1][3]*2000/(i4tgheight);
                    // mouth
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][0] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][0]*2000/(i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][1] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][1]*2000/(i4tgheight);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][2] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][2]*2000/(i4tgwidth);
                    eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][3] = -1000 + eAEFDArea.rAreas[ifaceIdx].i4Landmark[2][3]*2000/(i4tgheight);
                }

                AE_LOG( "[%s():Modified] AE FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d TGWidth:%d TGHeight:%d\n",__FUNCTION__, ifaceIdx,
                            eAEFDArea.rAreas[ifaceIdx].i4Left, eAEFDArea.rAreas[ifaceIdx].i4Right,
                            eAEFDArea.rAreas[ifaceIdx].i4Top, eAEFDArea.rAreas[ifaceIdx].i4Bottom ,i4tgwidth,i4tgheight );
                AE_LOG( "[%s():Previous] AE FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d \n",__FUNCTION__, ifaceIdx,
                          pFaces->faces[ifaceIdx].rect[0], pFaces->faces[ifaceIdx].rect[2],
                          pFaces->faces[ifaceIdx].rect[1] , pFaces->faces[ifaceIdx].rect[3] );

            }

            if(eAEFDArea.u4Count > 0) {
                if((m_eAEFDArea.rAreas[0].i4Left != eAEFDArea.rAreas[0].i4Left) ||
                   (m_eAEFDArea.rAreas[0].i4Right != eAEFDArea.rAreas[0].i4Right) ||
                   (m_eAEFDArea.rAreas[0].i4Top != eAEFDArea.rAreas[0].i4Top) ||
                   (m_eAEFDArea.rAreas[0].i4Bottom != eAEFDArea.rAreas[0].i4Bottom)) {
                    m_u4WOFDCnt = 0 ;
                }
            }
            memcpy(&m_eAEFDArea, &eAEFDArea, sizeof(CCU_AEMeteringArea_T));

            if (m_eAEFDArea.u4Count > 0 && m_bIsCCUStart) { // to reduce 1 frame FD info latency from MGR to CCU AE
                AE_CORE_CTRL_FD_INFO rCCUFDinfo;
                getFDInfo(&(rCCUFDinfo.AEFDArea));
                AE_LOG("[%s] MSG_TO_CCU_AE_SET_FD_INFO, AEFDArea[0]: %d/%d/%d/%d (%d)\n", __FUNCTION__,
                        rCCUFDinfo.AEFDArea.rAreas[0].i4Left, rCCUFDinfo.AEFDArea.rAreas[0].i4Right, rCCUFDinfo.AEFDArea.rAreas[0].i4Top, rCCUFDinfo.AEFDArea.rAreas[0].i4Bottom, rCCUFDinfo.AEFDArea.rAreas[0].i4Weight);
                m_pICcuAe->ccuControl(MSG_TO_CCU_AE_SET_FD_INFO, &rCCUFDinfo, NULL);
            }

            AE_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d line:%d Face Number:%d", __FUNCTION__, m_eSensorDev, __LINE__, eAEFDArea.u4Count);
        }
    } else {
        AE_LOG( "[%s()] i4SensorDev:%d line:%d The set face AE is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_FACE_AE());
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

    if( m_i4ObjectTrackNum != pOTWindow->number_of_faces) {
        AE_LOG( "[setOTInfo] Object tracking:%d %d\n", pOTWindow->number_of_faces, m_i4ObjectTrackNum);
        m_i4ObjectTrackNum = pOTWindow->number_of_faces;
        if(pOTWindow->number_of_faces > 0) { // Object tracking enable
            bEnableObjectTracking = MTRUE;
        } else {
            bEnableObjectTracking = MFALSE;
        }
    }
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep)
{
MINT32 i4EVValue;

    if (m_i4EVIndex != i4NewEVIndex) {
        m_i4EVIndex = i4NewEVIndex;
        m_fEVCompStep = fStep;
        i4EVValue = (MINT32) (100 * m_fEVCompStep * i4NewEVIndex);

        if(i4EVValue < -590) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n60; }
        else if(i4EVValue < -580) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n59; }
        else if(i4EVValue < -570) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n58; }
        else if(i4EVValue < -560) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n57; }
        else if(i4EVValue < -550) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n56; }
        else if(i4EVValue < -540) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n55; }
        else if(i4EVValue < -530) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n54; }
        else if(i4EVValue < -520) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n53; }
        else if(i4EVValue < -510) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n52; }
        else if(i4EVValue < -500) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n51; }
        else if(i4EVValue < -490) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n50; }
        else if(i4EVValue < -480) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n49; }
        else if(i4EVValue < -470) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n48; }
        else if(i4EVValue < -460) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n47; }
        else if(i4EVValue < -450) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n46; }
        else if(i4EVValue < -440) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n45; }
        else if(i4EVValue < -430) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n44; }
        else if(i4EVValue < -420) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n43; }
        else if(i4EVValue < -410) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n42; }
        else if(i4EVValue < -400) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n41; }
        else if(i4EVValue < -390) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n40; }
        else if(i4EVValue < -380) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n39; }
        else if(i4EVValue < -370) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n38; }
        else if(i4EVValue < -360) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n37; }
        else if(i4EVValue < -350) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n36; }
        else if(i4EVValue < -340) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n35; }
        else if(i4EVValue < -330) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n34; }
        else if(i4EVValue < -320) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n33; }
        else if(i4EVValue < -310) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n32; }
        else if(i4EVValue < -300) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n31; }
        else if(i4EVValue < -290) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n30; }
        else if(i4EVValue < -280) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n29; }
        else if(i4EVValue < -270) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n28; }
        else if(i4EVValue < -260) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n27; }
        else if(i4EVValue < -250) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n26; }
        else if(i4EVValue < -240) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n25; }
        else if(i4EVValue < -230) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n24; }
        else if(i4EVValue < -220) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n23; }
        else if(i4EVValue < -210) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n22; }
        else if(i4EVValue < -200) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n21; }
        else if(i4EVValue < -190) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n20; }
        else if(i4EVValue < -180) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n19; }
        else if(i4EVValue < -170) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n18; }
        else if(i4EVValue < -160) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n17; }
        else if(i4EVValue < -150) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n16; }
        else if(i4EVValue < -140) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n15; }
        else if(i4EVValue < -130) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n14; }
        else if(i4EVValue < -120) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n13; }
        else if(i4EVValue < -110) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n12; }
        else if(i4EVValue < -100) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n11; }
        else if(i4EVValue < -90) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n10; }
        else if(i4EVValue < -80) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n09; }
        else if(i4EVValue < -70) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n08; }
        else if(i4EVValue < -60) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n07; }
        else if(i4EVValue < -50) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n06; }
        else if(i4EVValue < -40) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_n05; }
        else if(i4EVValue < -30) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n04; }
        else if(i4EVValue < -20) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n03; }
        else if(i4EVValue < -10) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n02; }
        else if(i4EVValue <   0) { m_eAEEVcomp = LIB3A_AE_EV_COMP_n01;   }
        else if(i4EVValue == 0) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_00;   }
        else if(i4EVValue < 20) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_01;   }
        else if(i4EVValue < 30) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_02;   }
        else if(i4EVValue < 40) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_03;   }
        else if(i4EVValue < 50) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_04;   }
        else if(i4EVValue < 60) {     m_eAEEVcomp = LIB3A_AE_EV_COMP_05;  }
        else if(i4EVValue < 70) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_06;   }
        else if(i4EVValue < 80) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_07;   }
        else if(i4EVValue < 90) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_08;   }
        else if(i4EVValue < 100) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_09;   }
        else if(i4EVValue < 110) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_10;   }
        else if(i4EVValue < 120) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_11;   }
        else if(i4EVValue < 130) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_12;   }
        else if(i4EVValue < 140) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_13;   }
        else if(i4EVValue < 150) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_14;   }
        else if(i4EVValue < 160) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_15;   }
        else if(i4EVValue < 170) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_16;   }
        else if(i4EVValue < 180) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_17;   }
        else if(i4EVValue < 190) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_18;   }
        else if(i4EVValue < 200) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_19;   }
        else if(i4EVValue < 210) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_20;   }
        else if(i4EVValue < 220) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_21;   }
        else if(i4EVValue < 230) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_22;   }
        else if(i4EVValue < 240) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_23;   }
        else if(i4EVValue < 250) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_24;   }
        else if(i4EVValue < 260) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_25;   }
        else if(i4EVValue < 270) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_26;   }
        else if(i4EVValue < 280) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_27;   }
        else if(i4EVValue < 290) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_28;   }
        else if(i4EVValue < 300) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_29;   }
        else if(i4EVValue < 310) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_30;   }
        else if(i4EVValue < 320) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_31;   }
        else if(i4EVValue < 330) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_32;   }
        else if(i4EVValue < 340) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_33;   }
        else if(i4EVValue < 350) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_34;   }
        else if(i4EVValue < 360) {   m_eAEEVcomp = LIB3A_AE_EV_COMP_35;   }
        else if(i4EVValue < 370) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_36;   }
        else if(i4EVValue < 380) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_37;   }
        else if(i4EVValue < 390) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_38;   }
        else if(i4EVValue < 400) {    m_eAEEVcomp = LIB3A_AE_EV_COMP_39;   }
        else { m_eAEEVcomp = LIB3A_AE_EV_COMP_40;  }

//        resetCCUStable();
        AE_LOG( "[setAEEVCompIndex] m_i4EVIndex: %d EVComp:%d fEVCompStep:%f m_bAELock:%d, force reset CCU stable\n", m_i4EVIndex, m_eAEEVcomp, m_fEVCompStep, m_bAELock);
//        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
        if(m_bAELock) {
            m_i4WaitVDNum = (m_i4IspGainDelayFrames+ISP_GAIN_DELAY_OFFSET+1);
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

    switch(m_eAEEVcomp){
        case LIB3A_AE_EV_COMP_03: { iEVIndex = 3;   break; }
        case LIB3A_AE_EV_COMP_05: { iEVIndex = 5;   break; }
        case LIB3A_AE_EV_COMP_07: { iEVIndex = 7;   break; }
        case LIB3A_AE_EV_COMP_10: { iEVIndex = 10;  break; }
        case LIB3A_AE_EV_COMP_13: { iEVIndex = 13;  break; }
        case LIB3A_AE_EV_COMP_15: { iEVIndex = 15;  break; }
        case LIB3A_AE_EV_COMP_17: { iEVIndex = 17;  break; }
        case LIB3A_AE_EV_COMP_20: { iEVIndex = 20;  break; }
        case LIB3A_AE_EV_COMP_25: { iEVIndex = 25;  break; }
        case LIB3A_AE_EV_COMP_30: { iEVIndex = 30;  break; }
        case LIB3A_AE_EV_COMP_35: { iEVIndex = 35;  break; }
        case LIB3A_AE_EV_COMP_40: { iEVIndex = 40;  break; }
        case LIB3A_AE_EV_COMP_n03: { iEVIndex = -3;   break; }
        case LIB3A_AE_EV_COMP_n05: { iEVIndex = -5;   break; }
        case LIB3A_AE_EV_COMP_n07: { iEVIndex = -7;   break; }
        case LIB3A_AE_EV_COMP_n10: { iEVIndex = -10;  break; }
        case LIB3A_AE_EV_COMP_n13: { iEVIndex = -13;  break; }
        case LIB3A_AE_EV_COMP_n15: { iEVIndex = -15;  break; }
        case LIB3A_AE_EV_COMP_n17: { iEVIndex = -17;  break; }
        case LIB3A_AE_EV_COMP_n20: { iEVIndex = -20;  break; }
        case LIB3A_AE_EV_COMP_n25: { iEVIndex = -25;  break; }
        case LIB3A_AE_EV_COMP_n30: { iEVIndex = -30;  break; }
        case LIB3A_AE_EV_COMP_n35: { iEVIndex = -35;  break; }
        case LIB3A_AE_EV_COMP_n40: { iEVIndex = -40;  break; }
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
    AE_METERING_T eNewAEMeteringMode = static_cast<AE_METERING_T>(u4NewAEMeteringMode);
    LIB3A_AE_METERING_MODE_T eAEMeteringMode;

    if ((eNewAEMeteringMode < AE_METERING_BEGIN) || (eNewAEMeteringMode >= NUM_OF_AE_METER)) {
        CAM_LOGE("Unsupport AE Metering Mode: %d\n", eNewAEMeteringMode);
        return E_AE_UNSUPPORT_MODE;
    }

    switch(eNewAEMeteringMode) {
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
            AE_LOG( "The AE metering mode enum value is incorrectly:%d\n", eNewAEMeteringMode);
            eAEMeteringMode = LIB3A_AE_METERING_MODE_CENTER_WEIGHT;
            break;
    }

    if (m_eAEMeterMode != eAEMeteringMode) {
        m_eAEMeterMode = eAEMeteringMode;
        AE_LOG( "[setAEMeteringMode] m_eAEMeterMode: %d\n", m_eAEMeterMode);
    }

    return S_AE_OK;
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

    if (i4NewMaxFPS > i4SensorMaxFPS)
    {
        AE_LOG_IF(m_u4HwMagicNum<15, "[%s] Clipping: i4NewMaxFPS(%d), i4SensorMaxFPS(%d)", __FUNCTION__, i4NewMaxFPS, i4SensorMaxFPS);
        i4NewMaxFPS = i4SensorMaxFPS;
    }

    if ((i4NewMinFPS < LIB3A_AE_FRAMERATE_MODE_05FPS) || (i4NewMaxFPS > LIB3A_AE_FRAMERATE_MODE_MAX)) {
        AE_LOG( "[setAEMinMaxFrameRate] Unsupport AE frame rate range value: %d %d\n", i4NewMinFPS, i4NewMaxFPS);
        return S_AE_OK;
    } else if(i4NewMinFPS > i4NewMaxFPS) {
        AE_LOG( "[setAEMinMaxFrameRate] Unsupport AE frame rate: %d %d\n", i4NewMinFPS, i4NewMaxFPS);
        i4NewMinFPS = i4NewMaxFPS;
    }

    if ((m_i4AEMinFps != i4NewMinFPS) || (m_i4AEMaxFps != i4NewMaxFPS)) {
        m_i4AEMinFps = i4NewMinFPS;
        m_i4AEMaxFps = i4NewMaxFPS;
        m_bNeedCheckAEPline = MTRUE;
        AE_LOG( "[setAEMinMaxFrameRate] m_i4AEMinFps: %d m_i4AEMaxFps:%d\n", m_i4AEMinFps, m_i4AEMaxFps);
//        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_MAX_FPS_UPDATE);
//        if(m_pIAeCalc != NULL) {
//            searchPreviewIndexLimit();
//            m_pIAeCalc->setEVIdxInfo_v4p0(m_u4IndexMax ,m_u4IndexMin ,m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_u4IndexF);
//            m_pIAeCalc->setAEMinMaxFrameRate(m_i4AEMinFps, m_i4AEMaxFps);
//            m_pIAeCalc->setAECamMode(m_eAECamMode);
//            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
//            if(m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING))
//                 m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_MAX_FPS_UPDATE);
//            m_i4WaitVDNum = (m_i4IspGainDelayFrames+ISP_GAIN_DELAY_OFFSET+1);
//        } else {
//            AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
//        }
    }

    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEShotMode(MUINT32 u4NewAEShotMode)
{
    EShotMode eNewAEShotMode = static_cast<EShotMode>(u4NewAEShotMode);

    if (m_eShotMode != eNewAEShotMode) {
        m_eShotMode = eNewAEShotMode;
        m_bAElimitor = MFALSE;
        AE_LOG( "[setAEShotMode] m_eAppShotMode:%d AE limitor:%d\n", m_eShotMode, m_bAElimitor);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setCamScenarioMode(MUINT32 u4NewCamScenarioMode, MBOOL bInit) // currently only for AE_MODE_4CELL_MVHDR_TARGET
{
    CAM_SCENARIO_T eNewCamScenarioMode = static_cast<CAM_SCENARIO_T>(u4NewCamScenarioMode);
    CAM_CAL_DATA_STRUCT GetCamCalData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MUINT32 result=0;
    MINT32 i4SensorDevID, i4CurrSensorId, i4CurrModuleId, i4CurrLensId;

    if( m_eCamScenarioMode != eNewCamScenarioMode || bInit) {
        AE_LOG( "[%s()] m_eSensorDev: %d, m_eCamScenarioMode: %d -> %d, bInit: %d\n", __FUNCTION__, m_eSensorDev, m_eCamScenarioMode, eNewCamScenarioMode, bInit);
        m_eCamScenarioMode = eNewCamScenarioMode;

        switch (m_eSensorDev)
        {
        case ESensorDev_Main:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub:
            i4SensorDevID = NSCam::SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_Main3D:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN_3D;
            break;
        case ESensorDev_SubSecond:
            i4SensorDevID = NSCam::SENSOR_DEV_SUB_2;
            break;
        default:
            i4SensorDevID = NSCam::SENSOR_DEV_NONE;
            break;
        }
        
        CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum=CAMERA_CAM_CAL_DATA_MODULE_VERSION;        
        result= pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);

        i4CurrSensorId = getsensorDevID(m_eSensorDev);
        i4CurrModuleId = (result == CAM_CAL_ERR_NO_ERR)? (GetCamCalData.PartNumber[1] << 8) + GetCamCalData.PartNumber[0] : 0;
        i4CurrLensId = (result == CAM_CAL_ERR_NO_ERR)? (GetCamCalData.PartNumber[9] << 8) + GetCamCalData.PartNumber[8] : 0;
        m_rAECustPARAM = getAEModuleParamData(m_eCamScenarioMode, i4CurrSensorId, i4CurrModuleId, i4CurrLensId);
        AE_LOG("[%s()] (0x%8x)=m_pCamCalDrvObj->GetCamCalCalData SensorID:0x%0X ModuleID:0x%0X LensID:0x%0X", __FUNCTION__, result, i4CurrSensorId, i4CurrModuleId, i4CurrLensId);

        switch(m_eSensorDev) {
        case ESensorDev_Main:
            if(m_eCamScenarioMode == CAM_SCENARIO_PREVIEW) { // preivew
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_PREVIEW>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_VIDEO) { // video
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_VIDEO>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CAPTURE) { // capture
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_CAPTURE>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1) { // HDR ON
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM1>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2) { // auto HDR
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM2>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3) { // VT
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM3>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4) { // stereo
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM4>();
            }
            break;
        case ESensorDev_Sub:
            if(m_eCamScenarioMode == CAM_SCENARIO_PREVIEW) { // preivew
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_PREVIEW>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_VIDEO) { // video
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_VIDEO>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CAPTURE) { // capture
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CAPTURE>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1) { // HDR ON
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM1>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2) { // auto HDR
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM2>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3) { // VT
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM3>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4) { // stereo
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_Sub, CAM_SCENARIO_CUSTOM4>();
            }
            break;
        case ESensorDev_MainSecond:
            if(m_eCamScenarioMode == CAM_SCENARIO_PREVIEW) { // preivew
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_PREVIEW>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_VIDEO) { // video
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_VIDEO>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CAPTURE) { // capture
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CAPTURE>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1) { // HDR ON
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM1>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2) { // auto HDR
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM2>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3) { // VT
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM3>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4) { // stereo
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_MainSecond, CAM_SCENARIO_CUSTOM4>();
            }
            break;
        case ESensorDev_SubSecond:
            if(m_eCamScenarioMode == CAM_SCENARIO_PREVIEW) { // preivew
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_PREVIEW>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_VIDEO) { // video
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_VIDEO>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CAPTURE) { // capture
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_CAPTURE>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1) { // HDR ON
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM1>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2) { // auto HDR
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM2>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM3) { // VT
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM3>();
            }
            else if(m_eCamScenarioMode == CAM_SCENARIO_CUSTOM4) { // stereo
                m_rAEInitInput.rAEPARAM = getAEParamData<ESensorDev_SubSecond, CAM_SCENARIO_CUSTOM4>();
            }
            break;
        default:    //  Shouldn't happen.
            MY_ERR("Invalid sensor device: %d", m_eSensorDev);
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setNVRAMIndex(MUINT32 a_eNVRAMIndex, MBOOL bInit)
{
    if( m_u4AENVRAMIdx != a_eNVRAMIndex || bInit) {
        AE_LOG( "[%s()] m_eSensorDev: %d, m_u4AENVRAMIdx: %d -> %d bInit:%d\n", __FUNCTION__, m_eSensorDev, m_u4AENVRAMIdx, a_eNVRAMIndex, bInit);
        m_u4AENVRAMIdx = a_eNVRAMIndex;
        if(m_p3ANVRAM != NULL) {
            m_rAEInitInput.rAENVRAM = reinterpret_cast<CCU_AE_NVRAM_T*>(&(m_p3ANVRAM->AE[m_u4AENVRAMIdx]));

        } else {
             CAM_LOGE("Nvram 3A pointer is NULL\n");
        }

        // AE max block width & width
        if(m_eAETargetMode == AE_MODE_MVHDR_TARGET) {
            m_rAEInitInput.i4AEMaxBlockWidth = m_rSensorVCInfo.u2VCStatWidth;
            m_rAEInitInput.i4AEMaxBlockHeight = m_rSensorVCInfo.u2VCStatHeight;
        } else {
            m_rAEInitInput.i4AEMaxBlockWidth = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
            m_rAEInitInput.i4AEMaxBlockHeight = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        }

        // [ISP 6.0] AAO config
        m_rAEConfigInfo.u4BlockNumW = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
        m_rAEConfigInfo.u4BlockNumH = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        m_rAEInitInput.u4AAO_AWBValueWinSize = 4;
        m_rAEInitInput.u4AAO_AWBSumWinSize = 12*AE_PARAM_AE_LINEAR_STAT_EN;
        m_rAEInitInput.u4AAO_AEYWinSize = 2;
        m_rAEInitInput.u4AAO_AEYWinSizeSE = 2;
        m_rAEInitInput.u4AAO_AEOverWinSize = 4*(m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableAEOVERCNTconfig);
        m_rAEInitInput.u4AAO_HistSize = 3*AE_PARAM_AE_HST_DMA_OUT_EN;
        m_rAEInitInput.u4AAO_HistSizeSE = 3*AE_PARAM_AE_HST_DMA_OUT_EN;
        m_rAEInitInput.i4AAOLineByte =  m_rAEConfigInfo.u4BlockNumW * (m_rAEInitInput.u4AAO_AWBValueWinSize + m_rAEInitInput.u4AAO_AWBSumWinSize + m_rAEInitInput.u4AAO_AEYWinSize + m_rAEInitInput.u4AAO_AEOverWinSize);
        // AE statistics and histogram config
        m_rAEConfigInfo.bEnableHDRYConfig = (MBOOL)(m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableHDRLSB>0);
        m_rAEConfigInfo.bEnableOverCntConfig = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableAEOVERCNTconfig;
        m_rAEConfigInfo.bEnableTSFConfig = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableTSFSTATconfig;
        m_rAEConfigInfo.bDisablePixelHistConfig = ((m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePixelBaseHist==1)?MFALSE:MTRUE);
        m_rAEConfigInfo.u4AEOverExpCntThr = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.uOverExpoTHD;
        m_rAEConfigInfo.u4AEYCoefR = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEYCoefR;
        m_rAEConfigInfo.u4AEYCoefG = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEYCoefG;
        m_rAEConfigInfo.u4AEYCoefB = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4AEYCoefB;
        m_rAEConfigInfo.bEnableAEHSTDMAOut = AE_PARAM_AE_HST_DMA_OUT_EN;
        m_rAEConfigInfo.bEnableAELINEARStat = AE_PARAM_AE_LINEAR_STAT_EN;
        m_rAEConfigInfo.i4PSEXPMode = AE_PARAM_PS_EXP_MODE;
        m_rAEConfigInfo.i4PSHSTSepG = AE_PARAM_PS_HST_SEP_G;
        m_rAEConfigInfo.i4PSCCUHSTEnd = AE_PARAM_PS_CCU_HST_END;
        m_rAEConfigInfo.eAETargetMode = m_eAETargetMode;

        if(m_bFirstLaunchCam) {
            m_u4FinerEVIdxBase = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4FinerEvIndexBase;
            m_u4Index          = m_rAEInitInput.rAENVRAM->rCCTConfig.u4InitIndex;
            m_u4IndexF         = m_u4Index * m_u4FinerEVIdxBase;
            m_bFirstLaunchCam  = MFALSE;
            AE_LOG( "[%s()] reset pline index since first camera launch, m_eSensorDev: %d, m_u4Index/F: %d/%d, m_u4FinerEVIdxBase: %d \n",
                     __FUNCTION__, m_eSensorDev, m_u4Index, m_u4IndexF, m_u4FinerEVIdxBase);
        }

        AE_LOG( "[%s()] m_eSensorDev:%d m_i4SensorIdx:%d AE max block width:%d heigh:%d AAO separation size(To AeAlgo):%d/%d/%d/%d AAOLineByte:%d AEconfig:%d/%d/%d/%d/%d/%d AEYCoefR/G/B:%d/%d/%d FinerEVIdxBase: %d\n",
                 __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_rAEInitInput.i4AEMaxBlockWidth, m_rAEInitInput.i4AEMaxBlockHeight,
                 m_rAEInitInput.u4AAO_AWBValueWinSize, m_rAEInitInput.u4AAO_AWBSumWinSize, m_rAEInitInput.u4AAO_AEYWinSize, m_rAEInitInput.u4AAO_AEOverWinSize, m_rAEInitInput.i4AAOLineByte,
                 m_rAEConfigInfo.u4BlockNumW, m_rAEConfigInfo.u4BlockNumH, m_rAEConfigInfo.bEnableHDRYConfig, m_rAEConfigInfo.bEnableOverCntConfig, m_rAEConfigInfo.bEnableTSFConfig, m_rAEConfigInfo.bDisablePixelHistConfig,
                 m_rAEConfigInfo.u4AEYCoefR, m_rAEConfigInfo.u4AEYCoefG, m_rAEConfigInfo.u4AEYCoefB, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4FinerEvIndexBase);

        if(!bInit) {
            m_pIAeCalc->updateAEParam(&m_rAEInitInput);
        }
        m_bPlineMappingChange = MTRUE;
    }

    if(m_bPlineMappingChange) {
        if(m_bStereoManualPline) {
            m_pAEMapPlineTable = (AE_PLINEMAPPINGTABLE_T *)(&(m_rAEInitInput.rAENVRAM->rAeParam.strStereoPlineMapping.sAESceneMapping[0]));
            AE_LOG("[%s] change to stereo pline mapping table, m_u4AENVRAMIdx = %d, m_bStereoManualPline = %d\n", __FUNCTION__, m_u4AENVRAMIdx, m_bStereoManualPline);
        }
        else if(m_u4AEHDRMode == 1 || m_u4AEHDRMode == 2) {
            m_pAEMapPlineTable = (AE_PLINEMAPPINGTABLE_T *)(&(m_rAEInitInput.rAENVRAM->rAeParam.strHDRPlineMapping.sAESceneMapping[0]));
            AE_LOG("[%s] change to HDR pline mapping table, m_u4AENVRAMIdx = %d, m_u4AEHDRMode = %d\n", __FUNCTION__, m_u4AENVRAMIdx, m_u4AEHDRMode);
        }
        else {
            m_pAEMapPlineTable = &(m_rAEInitInput.rAEPlineMapTable[0]);
            AE_LOG("[%s] change to default pline mapping table, m_u4AENVRAMIdx = %d, m_bStereoManualPline = %d, m_u4AEHDRMode = %d\n", __FUNCTION__, m_u4AENVRAMIdx, m_bStereoManualPline, m_u4AEHDRMode);
        }
        m_bNeedCheckAEPline = MTRUE;
        m_bPlineMappingChange = MFALSE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEHDRMode(MUINT32 u4NewAEHDRMode)
{
    if (m_u4AEHDRMode != u4NewAEHDRMode){ // auto HDR: m_i4HdrOnOff reset
        m_i4HdrOnOff = toLiteral(HDRDetectionResult::NONE);
        if (u4NewAEHDRMode == 2){
            m_i4HdrOnOff = toLiteral(HDRDetectionResult::NORMAL);
            AE_LOG("[%s] turn on auto HDR\n", __FUNCTION__);
        }
        AE_LOG("[%s] m_i4HdrOnOff:%d HDR AE Mode=%d->%d\n", __FUNCTION__, m_i4HdrOnOff, m_u4AEHDRMode, u4NewAEHDRMode);
        m_u4AEHDRMode = u4NewAEHDRMode;
        m_bPlineMappingChange = MTRUE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEHDROnOff()
{
    AE_LOG_IF(m_3ALogEnable && (m_i4HdrOnOff>=0),"[%s] m_i4HdrOnOff:%d\n", __FUNCTION__, m_i4HdrOnOff);
    return m_i4HdrOnOff;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEMode(MUINT32 u4NewAEMode, MBOOL isForce)
{
    MUINT32 eNewAEMode = u4NewAEMode;
    LIB3A_AE_MODE_T eAEMode;

    if((m_bCCTEnable == MTRUE) && (isForce == MFALSE)) {
        AE_LOG_IF(m_3ALogEnable, "[%s()] Skip AE MODE setting u4NewAEMode:%d \n",__FUNCTION__, u4NewAEMode);
        return S_AE_OK;
    }
    if ((MINT32)eNewAEMode != (MINT32)m_eAEMode){
        if (!m_bShutterISOPriorityMode)
            AE_LOG( "[%s()] m_eAEMode %d -> %d AEState:%d bCCTEnable:%d isForce:%d\n", __FUNCTION__, m_eAEMode, eNewAEMode, m_eAEState, m_bCCTEnable, isForce);

        if (m_eAEMode == LIB3A_AE_MODE_OFF) { // AE mode off -> on
            sendAECtrl(EAECtrl_SetSkipCalAE, (MBOOL)MFALSE, NULL, NULL, NULL);
            disableManualCCU(MFALSE);
        }

        switch(eNewAEMode) {
            case MTK_CONTROL_AE_MODE_OFF:
                if (m_bShutterISOPriorityMode) {
                    AE_LOG( "[%s()] shutter-ISO priority mode, skip AE_MODE_OFF \n",__FUNCTION__);
                    break;
                }
                m_eAEMode = LIB3A_AE_MODE_OFF;
                m_eAEState = MTK_CONTROL_AE_STATE_INACTIVE;
                sendAECtrl(EAECtrl_SetSkipCalAE, (MBOOL)MTRUE, NULL, NULL, NULL); // AE mode on -> off
                AE_LOG( "[%s()] AE_MODE_OFF CCU ON \n",__FUNCTION__);
                break;
            case MTK_CONTROL_AE_MODE_ON:
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
            case MTK_CONTROL_AE_MODE_ON_EXTERNAL_FLASH:
                m_eAEMode = LIB3A_AE_MODE_ON_EXTERNAL_FLASH;
                m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                resetCCUStable();
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
    return static_cast<MINT32>(m_eAEMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAEState() const
{
    return static_cast<MINT32>(m_eAEState);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::setSensorMode(MINT32 i4NewSensorMode , MUINT32 width, MUINT32 height)
{
    ESensorMode_T eNewSensorMode;
    MUINT32 u4BlocksizeX;
    MUINT32 u4BlocksizeY;
    MINT32 i4Scenario = 0;
    AE_LOG( "[%s()] m_eSensorDev: %d i4NewSensorMode: %d\n", __FUNCTION__, m_eSensorDev, i4NewSensorMode);

    switch (i4NewSensorMode) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            eNewSensorMode = ESensorMode_Preview;
            i4Scenario = CAM_SCENARIO_PREVIEW;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            eNewSensorMode = ESensorMode_Capture;
            i4Scenario = CAM_SCENARIO_CAPTURE;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            eNewSensorMode = ESensorMode_Video;
            i4Scenario = CAM_SCENARIO_VIDEO;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            eNewSensorMode = ESensorMode_SlimVideo1;
            i4Scenario = CAM_SCENARIO_VIDEO;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            eNewSensorMode = ESensorMode_SlimVideo2;
            i4Scenario = CAM_SCENARIO_VIDEO;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            eNewSensorMode = ESensorMode_Custom1;
            i4Scenario = CAM_SCENARIO_CUSTOM1;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            eNewSensorMode = ESensorMode_Custom2;
            i4Scenario = CAM_SCENARIO_CUSTOM2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            eNewSensorMode = ESensorMode_Custom3;
            i4Scenario = CAM_SCENARIO_CUSTOM3;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            eNewSensorMode = ESensorMode_Custom4;
            i4Scenario = CAM_SCENARIO_CUSTOM4;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            eNewSensorMode = ESensorMode_Custom5;
            i4Scenario = CAM_SCENARIO_PREVIEW;
            break;
        default:
            CAM_LOGE("E_AE_UNSUPPORT_SENSOR_MODE: %d\n", i4NewSensorMode);
            return MFALSE;
    }

    // Window size
    u4BlocksizeX = ((width / m_rAEConfigInfo.u4BlockNumW)/2)*2;
    u4BlocksizeY = ((height / m_rAEConfigInfo.u4BlockNumH)/2)*2;
    m_rAEConfigInfo.u4AEOverExpCntShift = (((u4BlocksizeX/4)*(u4BlocksizeY/2)>255)?MTRUE:MFALSE);
    AE_LOG( "[%s()] m_eSensorMode/Hbinwidth/Hbinheight: %d/%d/%d -> %d/%d/%d , u4BlocksizeX/Y/Shift = %d/%d/%d \n", __FUNCTION__, m_eSensorMode,m_u4Hbinwidth,m_u4Hbinheight,
           eNewSensorMode, width, height, u4BlocksizeX , u4BlocksizeY, m_rAEConfigInfo.u4AEOverExpCntShift);
    m_u4Hbinwidth = width;
    m_u4Hbinheight = height;

    // TG interrupt AAO information
    MINT32 i4PitchY  = height / m_rAEConfigInfo.u4BlockNumH;
    MINT32 i4OriginY = (height - (i4PitchY * m_rAEConfigInfo.u4BlockNumH)) / 2;
    MINT32 i4AETgValidBlockWidth  = m_rAEInitInput.i4AEMaxBlockWidth;
    MINT32 i4AETgValidBlockHeight = (height * TG_INT_AE_RATIO - i4OriginY) / i4PitchY;
    AE_LOG_IF(m_3ALogEnable,"[%s()] i4AETgValidBlockWidth/Height: %d/%d -> %d/%d , u4BlockNumX/Y/i4PitchY/i4OriginY = %d/%d/%d/%d \n", __FUNCTION__,
           m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight, i4AETgValidBlockWidth, i4AETgValidBlockHeight,
           m_rAEConfigInfo.u4BlockNumW, m_rAEConfigInfo.u4BlockNumH, i4PitchY, i4OriginY);
    m_i4AETgValidBlockWidth  = i4AETgValidBlockWidth;
    m_i4AETgValidBlockHeight = i4AETgValidBlockHeight;

//    if((m_bAECaptureUpdate == MFALSE) && (width != 0) && (height != 0)) {
//        ModifyCaptureParamsBySensorMode(eNewSensorMode, ESensorMode_Capture);
//    }

    if (m_eSensorMode != eNewSensorMode) {
        if (m_pAEMapPlineTable != NULL){
            switchSensorModeMaxBVSensitivityDiff(eNewSensorMode, m_eSensorMode, m_i4DeltaSensitivityIdx, m_i4DeltaBVIdx);
        }
        m_eSensorMode = eNewSensorMode;
    }

    // Update scenario param    
    setCamScenarioMode(i4Scenario, m_bFirstLaunchCam);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAELock()
{
    AE_LOG( "[AeMgr::setAELock] m_bAELock: %d m_bAPAELock:%d m_bAFAELock:%d\n", m_bAELock, m_bAPAELock, m_bAFAELock, m_eAEState);
    if((m_bAPAELock == MFALSE) && (m_bAFAELock == MFALSE)) {    // AE unlock
        m_bAELock = MFALSE;
//        m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
    } else {    // AE lock
        m_bAELock = MTRUE;
    }
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAPAELock(MBOOL bAPAELock, MBOOL bMetaSetCtrl)
{
    if (m_bAPAELock != bAPAELock) {
        AE_LOG( "[AeMgr::setAPAELock] m_bAPAELock: %d -> %d\n", m_bAPAELock, bAPAELock);
        m_bAPAELock = bAPAELock;
        setAELock();
        if(m_bAPAELock == MFALSE) {    // AE unlock
//            m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
        } else {    // AE lock
            prepareCapParams();
//            m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
        }
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAFAELock(MBOOL bAFAELock)
{
    if (m_bAFAELock != bAFAELock) {
        AE_LOG( "[AeMgr::setAFAELock] m_bAFAELock: %d -> %d\n", m_bAFAELock, bAFAELock);
        m_bAFAELock = bAFAELock;
        setAELock();
        if (bAFAELock == MFALSE){
//            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_CYCLEINFO_AERESET);
        }
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if(u4Width == 0 || u4Height == 0)
        return S_AE_OK;
    if((u4XOffset <= 8192)&&(u4YOffset <= 8192)
        &&(u4Width <= 16384)&&(u4Height <= 16384)){
        if((m_eZoomWinInfo.u4XOffset != u4XOffset) || (m_eZoomWinInfo.u4XWidth != u4Width) ||
           (m_eZoomWinInfo.u4YOffset != u4YOffset) || (m_eZoomWinInfo.u4YHeight != u4Height)) {
            AE_LOG( "[AeMgr::setZoomWinInfo] New WinX:%d %d New WinY:%d %d Old WinX:%d %d Old WinY:%d %d\n", u4XOffset, u4Width, u4YOffset, u4Height,
                   m_eZoomWinInfo.u4XOffset, m_eZoomWinInfo.u4XWidth,
                   m_eZoomWinInfo.u4YOffset, m_eZoomWinInfo.u4YHeight);
            m_eZoomWinInfo.bZoomChange = MTRUE;
            m_eZoomWinInfo.u4XOffset = u4XOffset;
            m_eZoomWinInfo.u4XWidth = u4Width;
            m_eZoomWinInfo.u4YOffset = u4YOffset;
            m_eZoomWinInfo.u4YHeight = u4Height;
//            m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MFALSE);
        }
    } else {
        AE_LOG( "[AeMgr::setZoomWinInfo] Wrong zoom window size %d/%d/%d/%d \n", u4XOffset, u4Width, u4YOffset, u4Height);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::setAEMgrStateCPU(MUINT32 u4AEMgrStateCPU)
{
    if (m_u4AEMgrStateCPU != u4AEMgrStateCPU) {
        AE_LOG( "[%s()] m_u4AEMgrStateCPU: %d -> %d", __FUNCTION__, m_u4AEMgrStateCPU, u4AEMgrStateCPU);
        m_u4AEMgrStateCPU = u4AEMgrStateCPU;
        if ((m_eAlgoType == E_AE_ALGO_CCU_DEFAULT) && (m_u4AEMgrStateCPU == AE_MGR_STATE_DOPVAE || m_u4AEMgrStateCPU == AE_MGR_STATE_DOAFAE || m_u4AEMgrStateCPU == AE_MGR_STATE_DOPRECAPAE) && (m_eAEMode!=LIB3A_AE_MODE_OFF) ){
            disableManualCCU(MFALSE); //no need to disable manual in profession mode
            return MTRUE; // to force reset stable
        }
    }
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPvAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    AAA_TRACE_D("doPvAE");
    m_u4HwMagicNum = u4AAOUpdate;
    m_pAAOStatBuf  = pAEStatBuf;
    m_bAEReadyCapture = MFALSE;  // reset have been ever Normal Captured flag
    m_bFrameUpdate = MTRUE;
    m_bAlgoResultUpdate = MFALSE;
    MBOOL resetStable = setAEMgrStateCPU(AE_MGR_STATE_DOPVAE);

    /* handle zoom window change */
    if (m_eZoomWinInfo.bZoomChange == MTRUE) {
        if (m_pIAeCalc != NULL) {
            m_pIAeCalc->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);
        } else {
            AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }
        m_eZoomWinInfo.bZoomChange = MFALSE;
        // Update AE histogram window config
        AAA_TRACE_D("doPvAEStatConfig");
        ISP_AE_CONFIG_T::getInstance(m_eSensorDev).AEConfig(&m_rAEStatCfg, &m_rAEConfigInfo, &m_sAEResultConfig);
        AAA_TRACE_END_D;
    }

    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT) {
        AAA_TRACE_D("GetCCUResult");
        getCCUResult();
        AAA_TRACE_END_D;
    }
    else {
        AAA_TRACE_D("CalculateCPUAE");
        calculateCPUAE();
        AAA_TRACE_END_D;
    }

    if (resetStable) {
        AE_LOG( "[%s()] for CCU AE, force reset stable: m_bAEStable: %d -> 0", __FUNCTION__, m_bAEStable);
        m_bAEStable = MFALSE;
    }

    AE_LOG_IF(m_bAlgoResultUpdate, "[%s():monitor] SensorDev: %d, Mag: %d, CWVY: %d, AEStable: %d, AlgoAPAELock/AlgoAFAELock: %d/%d, Index/F: %d/%d, Algo: %d/%d/%d (%d), Real: %d/%d/%d (%d), EV/BV/BVWOStrobe: %d/%d/%d", __FUNCTION__,
              m_eSensorDev, m_u4HwMagicNum, m_u4CWVY, m_bAEStable, m_bAlgoAPAELock, m_bAlgoAFAELock, m_u4Index, m_u4IndexF,
              m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
              m_rAESettingPreview.EvSetting.u4Eposuretime, m_rAESettingPreview.EvSetting.u4AfeGain, m_rAESettingPreview.EvSetting.u4IspGain, m_rAESettingPreview.u4ISO,
              m_i4EVvalue, m_i4BVvalue, m_i4BVvalueWOStrobe);

    if ((m_eAEMode == LIB3A_AE_MODE_OFF && !m_bAdbAEPreviewUpdate) || m_bManualByShutterISOPriorityMode) {
        strAERealSetting rTempSetting;
        memset(&rTempSetting, 0, sizeof(strAERealSetting));
        MBOOL isValidMunual = getAEManualSetting(&rTempSetting);
        if (isValidMunual)
            UpdateSensorISPParams(AE_FLOW_STATE_MANUAL, rTempSetting);
    } else {
        // AE state control
        if (m_bAEStable == MTRUE) {
            if (m_bAlgoAPAELock == MFALSE) {
                /*
                if (m_bAlgoAFAELock == MTRUE) {
                    m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
                } else {
                    m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
                }
                */
                m_eAEState = MTK_CONTROL_AE_STATE_CONVERGED;
            } else {
                m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
            }
        } else {
            if (m_bAlgoAPAELock == MTRUE) {
                m_eAEState = MTK_CONTROL_AE_STATE_LOCKED;
            }
            else {
                m_eAEState = MTK_CONTROL_AE_STATE_SEARCHING;
            }
        }
        UpdateSensorISPParams(AE_FLOW_STATE_PREVIEW, m_rAESettingPreview);
    }

    AAA_TRACE_END_D;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doAFAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    if (m_eAEMode == LIB3A_AE_MODE_OFF) {
        AE_LOG_IF(m_3ALogEnable,"[%s()] skip doAFAE due to AE_MODE_OFF", __FUNCTION__);
        return S_AE_OK;
    }

    m_u4HwMagicNum = u4AAOUpdate;
    m_pAAOStatBuf  = pAEStatBuf;
    m_bAlgoResultUpdate = MFALSE;
    MBOOL resetStable = setAEMgrStateCPU(AE_MGR_STATE_DOAFAE);

    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT) {
        AAA_TRACE_D("GetCCUResult");
        getCCUResult();
        AAA_TRACE_END_D;
    }
    else {
        AAA_TRACE_D("CalculateCPUAE");
        calculateCPUAE();
        AAA_TRACE_END_D;
    }

    if (resetStable) {
        AE_LOG( "[%s()] for CCU AE, force reset stable: m_bAEStable: %d -> 0", __FUNCTION__, m_bAEStable);
        m_bAEStable = MFALSE;
    }

    AE_LOG_IF(m_bAlgoResultUpdate, "[%s():monitor] SensorDev: %d, Mag: %d, CWVY: %d, AEStable: %d, Index/F: %d/%d, Algo: %d/%d/%d (%d), Real: %d/%d/%d (%d), EV/BV/BVWOStrobe: %d/%d/%d", __FUNCTION__,
              m_eSensorDev, m_u4HwMagicNum, m_u4CWVY, m_bAEStable, m_u4Index, m_u4IndexF,
              m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
              m_rAESettingPreview.EvSetting.u4Eposuretime, m_rAESettingPreview.EvSetting.u4AfeGain, m_rAESettingPreview.EvSetting.u4IspGain, m_rAESettingPreview.u4ISO,
              m_i4EVvalue, m_i4BVvalue, m_i4BVvalueWOStrobe);

    // continue update for preview or AF state
    if(m_bAEStable == MTRUE) { // AE is stable, change to AF state
        prepareCapParams();
        prepareAE2AFParams();
        UpdateSensorISPParams(AE_FLOW_STATE_AE2AF, m_rAESettingAE2AF);
    } else {
        UpdateSensorISPParams(AE_FLOW_STATE_PREVIEW, m_rAESettingPreview);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPreCapAE(MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule)
{
    if(m_bAEReadyCapture && !m_bCShot) {
        m_bAEStable = MTRUE;
        AE_LOG( "[%s()] skip preCapAE, i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
        return S_AE_OK;
    }

    m_u4HwMagicNum = u4AAOUpdate;
    m_pAAOStatBuf  = pAEStatBuf;
    m_bAlgoResultUpdate = MFALSE;
    MBOOL resetStable = setAEMgrStateCPU(AE_MGR_STATE_DOPRECAPAE);

    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT) {
        AAA_TRACE_D("GetCCUResult");
        getCCUResult();
        AAA_TRACE_END_D;
    }
    else {
        AAA_TRACE_D("CalculateCPUAE");
        calculateCPUAE();
        AAA_TRACE_END_D;
    }

    if (resetStable) {
        AE_LOG( "[%s()] for CCU AE, force reset stable: m_bAEStable: %d -> 0", __FUNCTION__, m_bAEStable);
        m_bAEStable = MFALSE;
    }

    AE_LOG_IF(m_bAlgoResultUpdate, "[%s():monitor] SensorDev: %d, Mag: %d, CWVY: %d, AEStable: %d, Index/F: %d/%d, Algo: %d/%d/%d (%d), Real: %d/%d/%d (%d), EV/BV/BVWOStrobe: %d/%d/%d", __FUNCTION__,
              m_eSensorDev, m_u4HwMagicNum, m_u4CWVY, m_bAEStable, m_u4Index, m_u4IndexF,
              m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
              m_rAESettingPreview.EvSetting.u4Eposuretime, m_rAESettingPreview.EvSetting.u4AfeGain, m_rAESettingPreview.EvSetting.u4IspGain, m_rAESettingPreview.u4ISO,
              m_i4EVvalue, m_i4BVvalue, m_i4BVvalueWOStrobe);

    // AE is stable, update capture info
    if(m_bAEStable == MTRUE)
        prepareCapParams();
	m_eAEState = MTK_CONTROL_AE_STATE_PRECAPTURE;
    UpdateSensorISPParams(AE_FLOW_STATE_PREVIEW, m_rAESettingPreview);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doCapAE()
{
    setAEMgrStateCPU(AE_MGR_STATE_DOCAPAE);
    AE_LOG( "[%s()] i4SensorDev:%d CamMode:%d TargetMode:%d ISO=%d, minISO = %d\n", __FUNCTION__, m_eSensorDev, m_eCamMode, m_eAETargetMode,m_u4CCUManISO,m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain);

    if ( (m_bStrobeOn == MFALSE) && (m_bRealISOSpeed == MTRUE) && (m_u4AEISOSpeed != (MUINT32) LIB3A_AE_ISO_SPEED_AUTO) ){
//        updateEMISOCaptureParams(mCaptureMode); // temp: to fix me at p80
    }
    UpdateSensorISPParams(AE_FLOW_STATE_CAPTURE, m_rAESettingCapture);

    m_bAECaptureUpdate = MFALSE; // reset have been Updated Capture param flag
    m_SensorQueueCtrl.uOutputIndex = m_SensorQueueCtrl.uInputIndex;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doPostCapAE(MVOID *pAEStatBuf, MBOOL bIsStrobe)
{
    m_pAAOStatBuf  = pAEStatBuf;
    m_bAlgoResultUpdate = MFALSE;
    setAEMgrStateCPU(AE_MGR_STATE_DOPOSTCAPAE);

    if (m_eAlgoType == E_AE_ALGO_CCU_DEFAULT) {
        AAA_TRACE_D("GetCCUResult");
        getCCUResult();
        AAA_TRACE_END_D;
    }
    else {
        AAA_TRACE_D("CalculateCPUAE");
        calculateCPUAE();
        AAA_TRACE_END_D;
    }

    AE_LOG_IF(m_bAlgoResultUpdate, "[%s():monitor] SensorDev: %d, Mag: %d, CWVY: %d, AEStable: %d, Index/F: %d/%d, Algo: %d/%d/%d (%d), Real: %d/%d/%d (%d), EV/BV/BVWOStrobe: %d/%d/%d", __FUNCTION__,
              m_eSensorDev, m_u4HwMagicNum, m_u4CWVY, m_bAEStable, m_u4Index, m_u4IndexF,
              m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
              m_rAESettingPreview.EvSetting.u4Eposuretime, m_rAESettingPreview.EvSetting.u4AfeGain, m_rAESettingPreview.EvSetting.u4IspGain, m_rAESettingPreview.u4ISO,
              m_i4EVvalue, m_i4BVvalue, m_i4BVvalueWOStrobe);

    UpdateSensorISPParams(AE_FLOW_STATE_POSTCAP, m_rAESettingCapture);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doBackupAE()
{
    AE_LOG( "[%s()+] i4SensorDev:%d", __FUNCTION__, m_eSensorDev);
    setAEMgrStateCPU(AE_MGR_STATE_DOBACKUPAE);
    memcpy(&m_rAESettingBackup, &m_rAEAlgoOutput, sizeof(strAERealSetting));
    AE_LOG( "[%s()-] i4SensorDev:%d, m_rAESettingBackup.EvSetting: %d/%d/%d (ISO: %d), m_rAESettingBackup.u4Index/F: %d/%d (IndexBase: %d)\n", __FUNCTION__, m_eSensorDev,
              m_rAESettingBackup.EvSetting.u4Eposuretime, m_rAESettingBackup.EvSetting.u4AfeGain, m_rAESettingBackup.EvSetting.u4IspGain, m_rAESettingBackup.u4ISO,
              m_rAESettingBackup.u4Index, m_rAESettingBackup.u4IndexF, m_rAESettingBackup.u4AEFinerEVIdxBase);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::doRestoreAE()
{
    setAEMgrStateCPU(AE_MGR_STATE_DORESTOREAE);
    AE_LOG( "[%s()] i4SensorDev:%d, m_rAESettingBackup.EvSetting: %d/%d/%d (ISO: %d), m_rAESettingBackup.u4Index/F: %d/%d (IndexBase: %d)\n", __FUNCTION__, m_eSensorDev,
              m_rAESettingBackup.EvSetting.u4Eposuretime, m_rAESettingBackup.EvSetting.u4AfeGain, m_rAESettingBackup.EvSetting.u4IspGain, m_rAESettingBackup.u4ISO,
              m_rAESettingBackup.u4Index, m_rAESettingBackup.u4IndexF, m_rAESettingBackup.u4AEFinerEVIdxBase);
    m_rAESettingBackup.bResetIndex = MTRUE;
    UpdateSensorISPParams(AE_FLOW_STATE_RESTORE, m_rAESettingBackup);
    m_bAEStable = MFALSE;
    m_bAEReadyCapture = MFALSE;

    /* [LCE request: backup] special case for backup FD infomation: backup FD information at AFAE state, and send backup FD information to algo at POSTCAP state */
    if (getAEAlgoState() == AE_STATE_AFAE) {
        AE_LOG("[%s] backup FD information at AFAE state (for main flash)\n", __FUNCTION__);
        getFDInfo(&m_eAEFDAreaBackup4Flash);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAAOProcInfo(CCU_AAO_PROC_INFO_T const *sNewAAOProcInfo)
{
    memcpy(&m_strAAOProcInfo, sNewAAOProcInfo, sizeof(CCU_AAO_PROC_INFO_T));
    m_strAAOProcInfo.awb_gain_Unit = 512;
    AE_LOG_IF(m_3ALogEnable,"[%s()] awb_gain: %d/%d/%d\n", __FUNCTION__, m_strAAOProcInfo.awb_gain.i4R, m_strAAOProcInfo.awb_gain.i4G, m_strAAOProcInfo.awb_gain.i4B);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setMVHDR3ExpoProcInfo(MVOID *pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100)
{
    if(m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET) {
        AE_LOG( "[%s()] Update 3HDR Ratio:%d->%d\n", __FUNCTION__, m_u4MVHDRRatio_x100, u4MVHDRRatio_x100);
        m_pMVHDR3ExpoStatBuf = pMVHDR3ExpoStatBuf;
        m_u4MVHDRRatio_x100 = u4MVHDRRatio_x100;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getCCUresultCBActive(MVOID* pfCB)
{
    AE_LOG( "[%s()] \n", __FUNCTION__);
    m_pCCUresultCB = (NS3Av3::Hal3AFlowCtrl*) pfCB;
    return m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getLVvalue(MBOOL isStrobeOn)
{
    if(isStrobeOn == MTRUE) {
        return (m_i4BVvalue + 50);
    } else {
        return (m_i4BVvalueWOStrobe + 50);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getAOECompLVvalue(MBOOL isStrobeOn)
{

    if(isStrobeOn == MTRUE) {
        return (m_i4BVvalue + 50);
    } else {
        return (m_i4AOECompBVvalue + 50);
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::getCaptureLVvalue()
{
    MINT32 i4LVValue_10x = 0;
    // temp
    return (i4LVValue_10x);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAENvramData(AE_NVRAM_T &rAENVRAM)
{
    AE_LOG( "[%s()] i4SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
    if(m_p3ANVRAM == NULL) {
        CAM_LOGE("Nvram 3A pointer NULL\n");
    } else {
        rAENVRAM = m_p3ANVRAM->AE[m_u4AENVRAMIdx];
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorResolution()
{
    MRESULT err = S_AE_OK;

    if ((m_eSensorDev == ESensorDev_Main) || (m_eSensorDev == ESensorDev_Sub)) {
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_eSensorDev, &m_rSensorResolution[0]);
    } else if((m_eSensorDev == ESensorDev_MainSecond) ||(m_eSensorDev == ESensorDev_SubSecond)){
        err = AAASensorMgr::getInstance().getSensorWidthHeight(m_eSensorDev, &m_rSensorResolution[1]);
    } else {
        CAM_LOGE("Error sensor device\n");
    }

    if (FAILED(err)) {
        CAM_LOGE("AAASensorMgr::getInstance().getSensorWidthHeight fail\n");
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
    AE_LOG( "[%s()] i4SensorDev:%d\n", __FUNCTION__, i4SensorDev);

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SensorDev, (void*&)m_p3ANVRAM, isForce);
    if(err!=0)
        CAM_LOGE("Nvram 3A pointer NULL err=%d\n",err);

    strAEPLineTable sAEPlineTable;
    if(isForce == MTRUE) // fix me plz
        sAEPlineTable = ((AE_PLINETABLE_T*)m_pAEPlineTable)->AEPlineTable;

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_AE_PLINETABLE, i4SensorDev, (void*&)m_pAEPlineTable, isForce);
    if(err!=0)
        CAM_LOGE("Nvram AE Pline table pointer NULL err=%d\n",err);

    if(isForce == MTRUE && m_pAEPlineTable != NULL) // fix me plz
    {
        for(int i = 0; i < MAX_PLINE_TABLE; i++)
        {
            ((AE_PLINETABLE_T*)m_pAEPlineTable)->AEPlineTable.sPlineTable[i].pCurrentTable = sAEPlineTable.sPlineTable[i].pCurrentTable;
        }
    }

    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateISPNvramLTMIndex(MINT32 i4Tableidx)
{
  //AE_LOG_IF(1,"[%s()] m_i4SensorDev:%d i4OBCtableidx: %d->%d",__FUNCTION__,m_eSensorDev,m_i4ISP6LTMidx,i4Tableidx);
  if( (m_i4ISP6LTMidx != i4Tableidx) ){
    AE_LOG_IF(m_3ALogEnable,"[%s()] m_i4SensorDev:%d i4OBCtableidx: %d->%d",__FUNCTION__,m_eSensorDev,m_i4ISP6LTMidx,i4Tableidx);

    NVRAM_CAMERA_ISP_PARAM_STRUCT* pISPNvramTemp;
    // Get LTM nvram for CCU AE algo
    MRESULT err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, m_eSensorDev, (void*&)pISPNvramTemp);
    NVRAM_CAMERA_ISP_PARAM_STRUCT* const pISPNvram = pISPNvramTemp;

    if(i4Tableidx>=0){
      m_i4ISP6LTMidx = i4Tableidx;
    }else{
      AE_LOG_IF(m_3ALogEnable,"[%s()] m_i4ISP6LTMidx =0 \n",__FUNCTION__);
      m_i4ISP6LTMidx = 0;
    }
	m_bNeedCheckAEPline = MTRUE;
    memcpy(&m_LTMParam,&(pISPNvram->ISPToneMap.LTM[m_i4ISP6LTMidx].rAutoLTMParam),sizeof(ISP_AUTO_LTM_TUNING_T));
  }
  return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::AEInit(Param_T &rParam)
{
    (void)rParam;
    CCU_strAEOutput rAEOutput;
    MINT32 i, j;
    unsigned long long u8initAEtimer;
    MBOOL brealAEinit = MFALSE;
    MBOOL bEnablePreIndex = MFALSE;

    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    u8initAEtimer = (unsigned long long)time.tv_sec*1000000000 + (unsigned long long)time.tv_nsec;
    brealAEinit=(u8initAEtimer>AEtimer+RealInitThrs)?MTRUE:MFALSE;
    NumofUsers++;
    m_bInitAE = MTRUE;

    AE_LOG("CCU enable : %d\n", m_bCCUAEFlag);

    // if (NumofUsers != 1)
    //     m_bCCUAEFlag = MFALSE;
    AE_LOG( "[%s()] i4SensorDev:%d , AEtimer(uninit/init):%lld/%lld/%lld, bRealAEInit:%d, NumOfUsers=%d, m_bInitAE:%d\n", __FUNCTION__, m_eSensorDev, AEtimer, u8initAEtimer, u8initAEtimer-AEtimer, brealAEinit, NumofUsers, m_bInitAE);

    // Query TG info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("ae_mgr", m_i4SensorIdx);
    SensorDynamicInfo rSensorDynamicInfo;

    switch(m_eSensorDev) {
        case ESensorDev_Main:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
            m_bEnableAE = isAEEnabled<ESensorDev_Main>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_MAIN;
            break;
        case ESensorDev_Sub:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
            m_bEnableAE = isAEEnabled<ESensorDev_Sub>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_SUB;
            break;
        case ESensorDev_MainSecond:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
            m_bEnableAE = isAEEnabled<ESensorDev_MainSecond>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_MAIN2;
            break;
        case ESensorDev_SubSecond:
            pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorDynamicInfo);
            m_bEnableAE = isAEEnabled<ESensorDev_SubSecond>();
            m_rAEInitInput.eSensorDev = AE_SENSOR_SUB2;
            break;
        default:    //  Shouldn't happen.
            CAM_LOGE("Invalid sensor device: %d", m_eSensorDev);
    }
    // Judge which camscenario mode to be used
    setCamScenarioMode(m_eCamScenarioMode, m_bFirstLaunchCam);
    setNVRAMIndex(m_u4AENVRAMIdx, m_bFirstLaunchCam);

    if(pIHalSensor) {
        pIHalSensor->destroyInstance("ae_mgr");
    }

    AE_LOG( "TG = %d, pixel mode = %d\n", rSensorDynamicInfo.TgInfo, rSensorDynamicInfo.pixelMode);

    switch(rSensorDynamicInfo.TgInfo){
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
        case CAM_TG_3:
            m_eSensorTG = ESensorTG_3;
            break;
    }
    // --- Create SensorProvider Object ---
    AAA_TRACE_D("AE SensorProvider");
    if( mpSensorProvider == NULL) {
      mpSensorProvider = SensorProvider::createInstance(LOG_TAG);

      if( mpSensorProvider!=nullptr) {
          MUINT32 interval= 30;

          interval = property_get_int32("vendor.debug.ae_mgr.gyrointerval", SENSOR_GYRO_POLLING_MS);
          if( mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval)) {
              m_bGryoVd = MTRUE;
              CAM_LOGD("Dev(%d):enable SensorProvider success for Gyro ", m_eSensorDev);
          } else {
              m_bGryoVd = MFALSE;
              CAM_LOGE("Enable SensorProvider fail for Gyro");
          }

          interval = property_get_int32("vendor.debug.ae_mgr.gyrointerval", SENSOR_ACCE_POLLING_MS);
          if( mpSensorProvider->enableSensor(SENSOR_TYPE_ACCELERATION, interval)) {
              m_bAcceVd = MTRUE;
              CAM_LOGD("Dev(%d):enable SensorProvider success for Acce ", m_eSensorDev);
          } else {
              m_bAcceVd = MFALSE;
              CAM_LOGE("Enable SensorProvider fail for Acce");
          }

          interval = property_get_int32("vendor.debug.ae_mgr.gyrointerval", SENSOR_LIGHT_POLLING_MS);
          if( mpSensorProvider->enableSensor(SENSOR_TYPE_LIGHT, interval)) {
              m_bLightVd = MTRUE;
              CAM_LOGD("Dev(%d):enable SensorProvider success for Light ", m_eSensorDev);
          } else {
              m_bLightVd = MFALSE;
              CAM_LOGE("Enable SensorProvider fail for Light");
          }
      }
    }
    AAA_TRACE_END_D;

    if(bApplyAEPlineTableByCCT == MTRUE) {   // update by CCT
        CAM_LOGE("AE Pline table update by CCT\n");
    } else if(m_pAEPlineTable != NULL) {
        m_rAEInitInput.rAEPlineTable = m_pAEPlineTable;

        // sensor table mapping
        for(j=0; j < MAX_PLINE_MAP_TABLE; j++) {
            for(i = 0; i < LIB3A_SENSOR_MODE_MAX; i++) {
                m_rAEInitInput.rAEPlineMapTable[j].eAEScene = m_pAEPlineTable->sAEScenePLineMapping.sAESceneMapping[j].eAEScene;
                m_rAEInitInput.rAEPlineMapTable[j].ePLineID[i] = m_pAEPlineTable->sAEScenePLineMapping.sAESceneMapping[j].ePLineID[i];
            }
        }
    } else {
         CAM_LOGE("Nvram AE Pline table pointer is NULL\n");
    }

    // Clear FD Area
    memset(&m_eAEFDArea, 0, sizeof(CCU_AEMeteringArea_T));
    memset(&m_eAEFDAreaBackup4Flash, 0, sizeof(CCU_AEMeteringArea_T));

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
    AE_LOG( "[%s()] i4SensorDev:%d SensorIdx:%d AE meter:%d CamMode:%d AEScene:%d Flicker :%d %d EVcomp:%d ISO:%d %d MinFps:%d MaxFps:%d Limiter:%d\n", __FUNCTION__,
        m_eSensorDev, m_i4SensorIdx, m_eAEMeterMode,
        m_eAECamMode, m_eAEScene, m_eAEFlickerMode, m_eAEAutoFlickerMode, m_eAEEVcomp, m_u4AEISOSpeed, m_bRealISOSpeed,
        m_i4AEMinFps, m_i4AEMaxFps, m_bAElimitor);

#if USE_OPEN_SOURCE_AE
    m_pIAeCalc = NS3A::IAeAlgo::createInstance<NS3A::EAAAOpt_OpenSource>(m_eSensorDev);
#else
    m_pIAeCalc = NS3A::IAeCalc::createInstance<NS3A::EAAAOpt_MTK>(m_eSensorDev);
    if (m_bCCUAEFlag){
        m_pIAeFlowCCU->init();
        m_bIsCCUAEInit = MTRUE;
    }
#endif

    if (!m_pIAeCalc) {
        CAM_LOGE("AeCalc::createInstance() fail \n");
        return E_AE_ALGO_INIT_ERR;
    }

    if(m_bAETrueLaunch){
        bEnablePreIndex = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex;
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = MFALSE;
    }
    else if(!brealAEinit){
        bEnablePreIndex = m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex;
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = MTRUE;
    }

    checkAEPline();
    m_pIAeCalc->initAE(&m_rAEInitInput, &m_rAEStatCfg);
    m_pIAeCalc->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);
    m_rAEStatCfg.u4UseDefaultHight = MTRUE;

    if (!brealAEinit || m_bAETrueLaunch){
        m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnablePreIndex = bEnablePreIndex;
        m_bAETrueLaunch = MFALSE;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::prepareCapParams()
{
    if(m_pIAeCalc != NULL) {
        m_pIAeCalc->updateAEAlgoInfo(&m_rAEInfoPreview);
        m_pIAeCalc->switchCaptureState(&m_rAEAlgoOutput, &m_rAESettingCapture, 0);
        AE_LOG( "[%s()] i4SensorDev: %d, preview setting: %d/%d/%d (%d) -> capture setting: %d/%d/%d (%d)\n", __FUNCTION__, m_eSensorDev,
                m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
                m_rAESettingCapture.EvSetting.u4Eposuretime, m_rAESettingCapture.EvSetting.u4AfeGain, m_rAESettingCapture.EvSetting.u4IspGain, m_rAESettingCapture.u4ISO);
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    memcpy(&m_rAEInfoCapture, &m_rAEInfoPreview, sizeof(AE_INFO_T));
    m_bAEReadyCapture = MTRUE;  // capture ready flag
    m_bCShot = MFALSE;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::prepareAE2AFParams()
{
    if(m_pIAeCalc != NULL) {
        m_pIAeCalc->updateAEAlgoInfo(&m_rAEInfoPreview);
        m_pIAeCalc->switchAFAssistState(&m_rAEAlgoOutput, &m_rAESettingAE2AF);
        AE_LOG( "[%s()] i4SensorDev: %d, preview setting: %d/%d/%d (%d) -> AE2AF setting: %d/%d/%d (%d)\n", __FUNCTION__, m_eSensorDev,
                m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
                m_rAESettingAE2AF.EvSetting.u4Eposuretime,   m_rAESettingAE2AF.EvSetting.u4AfeGain,   m_rAESettingAE2AF.EvSetting.u4IspGain,   m_rAESettingAE2AF.u4ISO);
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAECapPlineTable(MINT32 *i4CapIndex, strAETable &a_AEPlineTable)
{
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo)
{
    if(m_p3ANVRAM != NULL) {
        a_rDeviceInfo = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo;
    } else {
        AE_LOG( "[%s()] i4SensorDev:%d NVRAM Data is NULL\n", __FUNCTION__, m_eSensorDev);
    }
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT AeMgr::getAEConfigParams(MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen)
{
    bHDRen = m_rAEConfigInfo.bEnableHDRYConfig;
    bOverCnten = m_rAEConfigInfo.bEnableOverCntConfig;
    bTSFen = m_rAEConfigInfo.bEnableTSFConfig;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsStrobeBVTrigger()
{
    MBOOL bStrobeBVTrigger;
    MINT32 i4Bv = 0;

    if(m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEV0TriggerStrobe == MTRUE) {         // The strobe trigger by the EV 0 index
        i4Bv = m_i4BVvalueWOStrobe;
    } else {
        if(m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp]) {
            if(m_pIAeCalc != NULL) {
                i4Bv = m_i4BVvalueWOStrobe + m_pIAeCalc->getSenstivityDeltaIndex(1024 *1024/ m_rAEInitInput.rAENVRAM->rAeParam.EVValueArray.EVValue[m_eAEEVcomp]);
            } else {
                i4Bv = m_i4BVvalueWOStrobe;
                AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
            }
        }
    }

    getPlineTable(m_CurrentPreviewTable, m_CurrentCaptureTable, m_CurrentStrobetureTable);
    bStrobeBVTrigger = (i4Bv < m_CurrentCaptureTable.i4StrobeTrigerBV) ? MTRUE : MFALSE;

    AE_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d bStrobeBVTrigger:%d BV:%d %d etPlineTable PreId:%d CapId:%d Strobe:%d\n", __FUNCTION__, m_eSensorDev,
              bStrobeBVTrigger, i4Bv, m_CurrentCaptureTable.i4StrobeTrigerBV, m_CurrentPreviewTable.eID, m_CurrentCaptureTable.eID, m_CurrentStrobetureTable.eID);

    return bStrobeBVTrigger;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setStrobeMode(MBOOL bIsStrobeOn)
{
    m_bStrobeOn = bIsStrobeOn;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo)
{
    a_rPreviewInfo.u4Eposuretime = m_rAESettingPreview.EvSetting.u4Eposuretime;
    a_rPreviewInfo.u4AfeGain     = m_rAESettingPreview.EvSetting.u4AfeGain;
    a_rPreviewInfo.u4IspGain     = m_rAESettingPreview.EvSetting.u4IspGain;
    a_rPreviewInfo.u4RealISO     = m_rAESettingPreview.u4ISO;
    a_rPreviewInfo.u2FrameRate   = m_rAESettingPreview.u2FrameRate;
    a_rPreviewInfo.u4CWValue     = m_u4CWVY;
    a_rPreviewInfo.u4AvgY        = m_u4CWVY;//TBD
    a_rPreviewInfo.u4AEFinerEVIdxBase = m_u4FinerEVIdxBase;
    AE_LOG_IF(m_3ALogEnable,"[getPreviewParams] SensorDev=%d, Preview Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d IdxBase:%d\n", m_eSensorDev, a_rPreviewInfo.u4Eposuretime, a_rPreviewInfo.u4AfeGain,
                   a_rPreviewInfo.u4IspGain, a_rPreviewInfo.u4RealISO,m_u4FinerEVIdxBase);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAFAEOutput(CCU_strAEOutput &a_rAEOutput)
{
    a_rAEOutput = m_rAFAEOutput;
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorListenerParams(MINT32 *i4SensorInfo)
{
    GyroSensor_Param_T *pGyroSensorParam = reinterpret_cast<GyroSensor_Param_T *>(i4SensorInfo);
    m_i4AcceInfo[0] = pGyroSensorParam->i4AcceInfo[0];
	m_i4AcceInfo[1] = pGyroSensorParam->i4AcceInfo[1];
	m_i4AcceInfo[2] = pGyroSensorParam->i4AcceInfo[2];

	m_i4GyroInfo[0] = pGyroSensorParam->i4GyroInfo[0];
	m_i4GyroInfo[1] = pGyroSensorParam->i4GyroInfo[1];
	m_i4GyroInfo[2] = pGyroSensorParam->i4GyroInfo[2];

    AE_LOG_IF(m_3ALogEnable,"[%s()] Acce = %d/%d/%d , Gyro = %d/%d/%d\n",
        __FUNCTION__, m_i4AcceInfo[0], m_i4AcceInfo[1],m_i4AcceInfo[2],
        m_i4GyroInfo[0], m_i4GyroInfo[1], m_i4GyroInfo[2]);

    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateAEUnderExpdeltaBVIdx(MINT32 i4AEdeltaBV)
{
    if(m_i4AEUnderExpDeltaBVIdx != i4AEdeltaBV){
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
    m_rAESettingPreview.EvSetting.u4Eposuretime = a_rPreviewInfo.u4Eposuretime;
    m_rAESettingPreview.EvSetting.u4AfeGain     = a_rPreviewInfo.u4AfeGain;
    m_rAESettingPreview.EvSetting.u4IspGain     = a_rPreviewInfo.u4IspGain;
    m_rAESettingPreview.u4ISO         = a_rPreviewInfo.u4RealISO;
    m_rAESettingPreview.u2FrameRate   = a_rPreviewInfo.u2FrameRate;
    memcpy(&m_rAESettingAE2AF, &m_rAESettingPreview, sizeof(strAERealSetting));
    m_i4WaitVDNum = 0; // reset the delay frame
    m_u4Index = i4AEidxNext;
    m_u4IndexF = i4AEidxNextF;

    AE_LOG( "[%s()] m_i4SensorDev:%d m_u4Index:%d m_u4IndexF:%d Preview Shutter:%d Sensor gain:%d Isp gain:%d frame rate:%d ISO:%d\n",
            __FUNCTION__, m_eSensorDev, m_u4Index, m_u4IndexF,
            m_rAESettingPreview.EvSetting.u4Eposuretime, m_rAESettingPreview.EvSetting.u4AfeGain, m_rAESettingPreview.EvSetting.u4IspGain,
            m_rAESettingPreview.u2FrameRate, m_rAESettingPreview.u4ISO);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCaptureSetting(strAERealSetting* pCapSetting)
{
    // check capture pline
    strAETable strCurrentCaptureTable;
    if (m_eAEManualCapPline != EAEManualPline_Default) { // manual capture pline
        eAETableID eManualTableID = m_pPreviewTableCurrent->eID;
        getAEManualCapPline(m_eAEManualCapPline, eManualTableID);

        // Search for eID
        MBOOL bNewPLineTableID = MFALSE;
        for(MUINT8 i=0;i<(MUINT32)MAX_PLINE_TABLE;i++) {
            if(m_pAEPlineTable->AEPlineTable.sPlineTable[i].eID == eManualTableID){
                AE_LOG( "[%s()] i/ePreviewPLineTableID/TotalIdx: %d/%d/%d \n", __FUNCTION__, i, eManualTableID, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
                bNewPLineTableID = MTRUE;
                if (m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex == 0){
                    AE_LOG( "[%s()] The Preview AE table total index is zero : %d ->%d\n", __FUNCTION__, eManualTableID, m_pPreviewTableCurrent->eID);
                    eManualTableID = m_pPreviewTableCurrent->eID;
                }
            }
            AE_LOG_IF(m_3ALogEnable,"[%s()] i/TotalIdx: %d/%d \n", __FUNCTION__, i, m_pAEPlineTable->AEPlineTable.sPlineTable[i].u4TotalIndex);
        }
        // Check AE Pline if not existed
        if (!bNewPLineTableID) {
            eManualTableID = m_pPreviewTableCurrent->eID;
            AE_LOG( "[%s()] The Preview AE table enum value is not supported: %d ->%d\n", __FUNCTION__, bNewPLineTableID, m_pPreviewTableCurrent->eID);
        }
        getOnePlineTable(eManualTableID, strCurrentCaptureTable);
    }else {
        getOnePlineTable(m_pCaptureTable->eID, strCurrentCaptureTable);
    }

    m_pIAeCalc->updateAEExpInfo_v4p0(pCapSetting, &m_rAEAlgoOutput, &strCurrentCaptureTable, m_pPreviewTableCurrent, &(m_pAEPlineTable->AEGainList));
    AE_LOG( "[%s()] m_i4SensorDev: %d, input index/F/base: %d/%d/%d, input setting: %d/%d/%d (%d), output setting: %d/%d/%d (%d)\n",
             __FUNCTION__, m_eSensorDev, m_rAEAlgoOutput.u4Index, m_rAEAlgoOutput.u4IndexF, m_rAEAlgoOutput.u4AEFinerEVIdxBase,
             m_rAEAlgoOutput.EvSetting.u4Eposuretime, m_rAEAlgoOutput.EvSetting.u4AfeGain, m_rAEAlgoOutput.EvSetting.u4IspGain, m_rAEAlgoOutput.u4ISO,
             pCapSetting->EvSetting.u4Eposuretime, pCapSetting->EvSetting.u4AfeGain, pCapSetting->EvSetting.u4IspGain, pCapSetting->u4ISO);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
    strAERealSetting rCapSetting;
    getCaptureSetting(&rCapSetting);
    strHDRInputSetting.u4ExpTimeInUS0EV = rCapSetting.EvSetting.u4Eposuretime;
    strHDRInputSetting.u4SensorGain0EV  = (rCapSetting.EvSetting.u4AfeGain)*(rCapSetting.EvSetting.u4IspGain) >>10;
    strHDRInputSetting.u4ISOValue       = rCapSetting.u4ISO;

    AE_LOG( "[%s()] m_i4SensorDev:%d Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d, u4ExpTimeInUS0EV:%d u4SensorGain0EV:%d u4ISOValue:%d\n", __FUNCTION__,
            m_eSensorDev, rCapSetting.EvSetting.u4Eposuretime, rCapSetting.EvSetting.u4AfeGain, rCapSetting.EvSetting.u4IspGain, rCapSetting.u4ISO,
            strHDRInputSetting.u4ExpTimeInUS0EV, strHDRInputSetting.u4SensorGain0EV, strHDRInputSetting.u4ISOValue);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo)
{
    strAERealSetting rCapSetting;
    memset(&rCapSetting, 0, sizeof(strAERealSetting));
    getCaptureSetting(&rCapSetting);
    a_rCaptureInfo.u4Eposuretime  = rCapSetting.EvSetting.u4Eposuretime;
    a_rCaptureInfo.u4AfeGain      = rCapSetting.EvSetting.u4AfeGain;
    a_rCaptureInfo.u4IspGain      = rCapSetting.EvSetting.u4IspGain;
    a_rCaptureInfo.u2FrameRate    = rCapSetting.u2FrameRate;
    a_rCaptureInfo.u4ExposureMode = rCapSetting.u4ExposureMode;
    a_rCaptureInfo.u4RealISO      = rCapSetting.u4ISO;

    AE_LOG( "[%s()] m_i4SensorDev:%d Capture Exp. mode:%d Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d\n", __FUNCTION__,
            m_eSensorDev, a_rCaptureInfo.u4ExposureMode, a_rCaptureInfo.u4Eposuretime, a_rCaptureInfo.u4AfeGain,
            a_rCaptureInfo.u4IspGain, a_rCaptureInfo.u4RealISO);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo)
{
    MUINT32 u4FinalGain;
    MUINT32 u4PreviewBaseISO=100;
    AE_EXP_GAIN_MODIFY_T  rSensorInputData, rSensorOutputData;

    if(m_p3ANVRAM != NULL) {
        u4PreviewBaseISO=m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
    } else {
        AE_LOG( "[%s()] m_i4SensorDev:%d NVRAM data is NULL\n", __FUNCTION__, m_eSensorDev);
    }
    m_rAESettingCapture.EvSetting.u4Eposuretime = a_rCaptureInfo.u4Eposuretime;
    m_rAESettingCapture.EvSetting.u4AfeGain     = a_rCaptureInfo.u4AfeGain;
    m_rAESettingCapture.EvSetting.u4IspGain     = a_rCaptureInfo.u4IspGain;
    m_rAESettingCapture.u2FrameRate             = a_rCaptureInfo.u2FrameRate;
    m_rAESettingCapture.u4ExposureMode          = a_rCaptureInfo.u4ExposureMode = eAE_EXPO_TIME;
    m_rAESettingCapture.u4Index                 = m_u4Index;
    m_rAESettingCapture.u4IndexF                = m_u4IndexF;
    m_rAESettingCapture.u4AEFinerEVIdxBase      = m_u4FinerEVIdxBase;
    u4FinalGain = (m_rAESettingCapture.EvSetting.u4AfeGain * m_rAESettingCapture.EvSetting.u4IspGain)/AE_GAIN_BASE_ISP;
    m_rAESettingCapture.u4ISO = u4PreviewBaseISO * u4FinalGain / AE_GAIN_BASE_AFE;
    AE_LOG( "[%s()] m_i4SensorDev:%d Exp. mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d u4Index/F/base: %d/%d/%d\n",
            __FUNCTION__, m_eSensorDev, m_rAESettingCapture.u4ExposureMode,
            m_rAESettingCapture.EvSetting.u4Eposuretime, m_rAESettingCapture.EvSetting.u4AfeGain, m_rAESettingCapture.EvSetting.u4IspGain, m_rAESettingCapture.u4ISO,
            m_rAESettingCapture.u4Index, m_rAESettingCapture.u4IndexF, m_rAESettingCapture.u4AEFinerEVIdxBase);

    if(m_bEnableAE) {
        rSensorInputData.u4SensorExpTime = m_rAESettingCapture.EvSetting.u4Eposuretime;
        rSensorInputData.u4SensorGain    = m_rAESettingCapture.EvSetting.u4AfeGain;
        rSensorInputData.u4IspGain       = m_rAESettingCapture.EvSetting.u4IspGain;
        if(!m_bStrobeOn){
            AE_LOG( "[%s()] m_bStrobeOn == 0\n", __FUNCTION__);
            if(m_pIAeCalc != NULL) {
                m_pIAeCalc->switchSensorExposureGain(MTRUE, rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
            } else {
                AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
            }
            m_rAESettingCapture.EvSetting.u4Eposuretime = rSensorOutputData.u4SensorExpTime;
            m_rAESettingCapture.EvSetting.u4AfeGain     = rSensorOutputData.u4SensorGain;
            m_rAESettingCapture.EvSetting.u4IspGain     = rSensorOutputData.u4IspGain;
        }
        u4FinalGain = ( m_rAESettingCapture.EvSetting.u4AfeGain * m_rAESettingCapture.EvSetting.u4IspGain)/AE_GAIN_BASE_ISP;
        m_rAESettingCapture.u4ISO = u4PreviewBaseISO*u4FinalGain/AE_GAIN_BASE_AFE;
        AE_LOG( "[%s()] m_i4SensorDev:%d Modify Exp. mode = %d Capture Shutter:%d Sensor gain:%d Isp gain:%d ISO:%d\n",
                __FUNCTION__, m_eSensorDev, m_rAESettingCapture.u4ExposureMode,
                m_rAESettingCapture.EvSetting.u4Eposuretime, m_rAESettingCapture.EvSetting.u4AfeGain, m_rAESettingCapture.EvSetting.u4IspGain, m_rAESettingCapture.u4ISO);
        if(((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET)|| (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET))) {
            strEvSetting rEvSetting;
            //mark for p80
            //strEvHdrSetting rHdrEvSetting;
            rEvSetting.u4Eposuretime = m_rAESettingCapture.EvSetting.u4Eposuretime;
            rEvSetting.u4AfeGain     = m_rAESettingCapture.EvSetting.u4AfeGain;
            rEvSetting.u4IspGain     = m_rAESettingCapture.EvSetting.u4IspGain;
            rEvSetting.uIris =0;
            rEvSetting.uSensorMode =0;
            rEvSetting.uFlag =0;
#if 0
            if( (m_rAEInitInput.rAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.i4FlashRatio !=-1) && ((m_eAEMode == LIB3A_AE_MODE_ON_AUTO_FLASH) || (m_eAEMode == LIB3A_AE_MODE_ON_ALWAYS_FLASH)) ){
                AE_LOG( "[%s()] HDR Flash Ratio=%d", __FUNCTION__, m_rAEInitInput.rAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.i4FlashRatio);
                if(m_pIAeCalc != NULL) {
                //m_pIAeCalc->setHdrRatio(m_rAEInitInput.rAENVRAM->rAeParam.HdrAECFG.rHdrAutoEnhance.i4FlashRatio);//Algo remove
                }
            }
#endif
            if(m_pIAeCalc != NULL) {
                //m_pIAeCalc->calRatio2AEInfoIf(rEvSetting); //Algo remove
                //m_pIAeCalc->updateHdrInfo(rHdrEvSetting); //Algo remove
            } else {
                AE_LOG( "[%s()] The AE algo class is NULL", __FUNCTION__);
            }
            
            //mark for P80
            //m_rAESettingCapture.HdrEvSetting.i4LEExpo    = rHdrEvSetting.rHdrEVInfo.i4LEExpo;
            //m_rAESettingCapture.HdrEvSetting.i4LEAfeGain = rHdrEvSetting.rHdrEVInfo.i4LEAfeGain;
            //m_rAESettingCapture.HdrEvSetting.i4LEIspGain = rHdrEvSetting.rHdrEVInfo.i4LEIspGain;
            //m_rAESettingCapture.HdrEvSetting.i4MEExpo    = rHdrEvSetting.rHdrEVInfo.i4MEExpo;
            //m_rAESettingCapture.HdrEvSetting.i4MEAfeGain = rHdrEvSetting.rHdrEVInfo.i4MEAfeGain;
            //m_rAESettingCapture.HdrEvSetting.i4MEIspGain = rHdrEvSetting.rHdrEVInfo.i4MEIspGain;
            //m_rAESettingCapture.HdrEvSetting.i4SEExpo    = rHdrEvSetting.rHdrEVInfo.i4SEExpo;
            //m_rAESettingCapture.HdrEvSetting.i4SEAfeGain = rHdrEvSetting.rHdrEVInfo.i4SEAfeGain;
            //m_rAESettingCapture.HdrEvSetting.i4SEIspGain = rHdrEvSetting.rHdrEVInfo.i4SEIspGain;
            //m_rAESettingCapture.HdrEvSetting.u4AECHdrRatio = rHdrEvSetting.rHdrEVInfo.i4HdrRatio;
            //m_i4LESE_Ratio = m_rAESettingCapture.HdrEvSetting.u4AECHdrRatio;

        }
    }
    memcpy(&m_rAEInfoCapture, &m_rAEInfoPreview, sizeof(AE_INFO_T));
    m_bAECaptureUpdate = MTRUE;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringYvalue(CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue)
{
    std::lock_guard<std::mutex> lock((m_Lock));
    MUINT8 iValue = 0;
    CCU_AEMeterArea_T sAEMeteringArea = rWinSize;
    // Mapping AE region from Hbin size to [-1000,1000]
    sAEMeteringArea.i4Left   = sAEMeteringArea.i4Left*2000/m_u4Hbinwidth - 1000;
    sAEMeteringArea.i4Right  = sAEMeteringArea.i4Right*2000/m_u4Hbinwidth - 1000;
    sAEMeteringArea.i4Top    = sAEMeteringArea.i4Top*2000/m_u4Hbinheight - 1000;
    sAEMeteringArea.i4Bottom = sAEMeteringArea.i4Bottom*2000/m_u4Hbinheight - 1000;
    // Bounding check
    if (sAEMeteringArea.i4Left   < -1000)  {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000)  {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000)  {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000)  {sAEMeteringArea.i4Bottom = -1000;}

    if (sAEMeteringArea.i4Left   > 1000)  {sAEMeteringArea.i4Left   = 1000;}
    if (sAEMeteringArea.i4Right  > 1000)  {sAEMeteringArea.i4Right  = 1000;}
    if (sAEMeteringArea.i4Top    > 1000)  {sAEMeteringArea.i4Top    = 1000;}
    if (sAEMeteringArea.i4Bottom > 1000)  {sAEMeteringArea.i4Bottom = 1000;}
    AE_LOG_IF(m_3ALogEnable,"[%s()] AE meter area Left:%d Right:%d Top:%d Bottom:%d \n",__FUNCTION__,
        sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom);

    if(m_pIAeCalc != NULL) {
        //m_pIAeCalc->getAEMeteringAreaValue(sAEMeteringArea, &iValue); //Algo remove
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    *uYvalue = iValue;

//    AE_LOG( "[getMeteringYvalue] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEMeteringBlockAreaValue(CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt)
{
    std::lock_guard<std::mutex> lock((m_Lock));
    CCU_AEMeterArea_T sAEMeteringArea = rWinSize;
    // Mapping AE region from Hbin size to [-1000,1000]
    sAEMeteringArea.i4Left   = sAEMeteringArea.i4Left*2000/m_u4Hbinwidth - 1000;
    sAEMeteringArea.i4Right  = sAEMeteringArea.i4Right*2000/m_u4Hbinwidth - 1000;
    sAEMeteringArea.i4Top    = sAEMeteringArea.i4Top*2000/m_u4Hbinheight - 1000;
    sAEMeteringArea.i4Bottom = sAEMeteringArea.i4Bottom*2000/m_u4Hbinheight - 1000;
    // Bounding check
    if (sAEMeteringArea.i4Left   < -1000)  {sAEMeteringArea.i4Left   = -1000;}
    if (sAEMeteringArea.i4Right  < -1000)  {sAEMeteringArea.i4Right  = -1000;}
    if (sAEMeteringArea.i4Top    < -1000)  {sAEMeteringArea.i4Top    = -1000;}
    if (sAEMeteringArea.i4Bottom < -1000)  {sAEMeteringArea.i4Bottom = -1000;}

    if (sAEMeteringArea.i4Left   > 1000)  {sAEMeteringArea.i4Left   = 1000;}
    if (sAEMeteringArea.i4Right  > 1000)  {sAEMeteringArea.i4Right  = 1000;}
    if (sAEMeteringArea.i4Top    > 1000)  {sAEMeteringArea.i4Top    = 1000;}
    if (sAEMeteringArea.i4Bottom > 1000)  {sAEMeteringArea.i4Bottom = 1000;}
    AE_LOG_IF(m_3ALogEnable,"[%s()] AE meter area Left:%d Right:%d Top:%d Bottom:%d\n",__FUNCTION__,
    sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom);

    if(m_pIAeCalc != NULL) {
        //m_pIAeCalc->getAEMeteringBlockAreaValue(sAEMeteringArea, uYvalue, u2YCnt); //Algo remove
    } else {
        AE_LOG( "The AE algo class is NULL (25)\n");
    }

//    AE_LOG( "[getMeteringYvalue] AE meter area Left:%d Right:%d Top:%d Bottom:%d Y:%d %d\n", sAEMeteringArea.i4Left, sAEMeteringArea.i4Right, sAEMeteringArea.i4Top, sAEMeteringArea.i4Bottom, iValue, *uYvalue);
    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size)
{
    if(m_pIAeCalc != NULL) {
        //m_pIAeCalc->getAEBlockYvalues(pYvalues, size); //Algo remove
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::getAECondition(MUINT32 i4AECondition)
{
    if(i4AECondition & m_u4AECondition) {
        return MTRUE;
    } else {
        return MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getLCEPlineInfo(LCEInfo_T &a_rLCEInfo)
{
MUINT32 u4LCEStartIdx = 0, u4LCEEndIdx = 0;


    if(m_pIAeCalc != NULL) {
        //m_pIAeCalc->getAELCEIndexInfo(&u4LCEStartIdx, &u4LCEEndIdx); //Algo remove
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    a_rLCEInfo.i4AEidxNext = m_u4Index;
    a_rLCEInfo.i4AEidxNextF = m_u4IndexF;
    a_rLCEInfo.i4NormalAEidx = (MINT32) u4LCEStartIdx;
    a_rLCEInfo.i4LowlightAEidx = (MINT32) u4LCEEndIdx;

    if (m_bAEMgrDebugEnable) {
        AE_LOG( "[%s()] m_i4SensorDev:%d i4AEidxCur:%d i4AEidxNext:%d i4AEidxNextF:%d i4NormalAEidx:%d i4LowlightAEidx:%d\n", __FUNCTION__, m_eSensorDev, a_rLCEInfo.i4AEidxCur, a_rLCEInfo.i4AEidxNext,a_rLCEInfo.i4AEidxNextF, a_rLCEInfo.i4NormalAEidx, a_rLCEInfo.i4LowlightAEidx);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAERotateDegree(MINT32 i4RotateDegree)
{
    if(m_i4RotateDegree == i4RotateDegree) {  // the same degree
        return S_AE_OK;
    }

    AE_LOG( "[%s()] m_i4SensorDev:%d old:%d\n", __FUNCTION__, m_eSensorDev, m_i4RotateDegree);
    m_i4RotateDegree = i4RotateDegree;

    if(m_pIAeCalc != NULL) {
        if((i4RotateDegree == 90) || (i4RotateDegree == 270)){
            //m_pIAeCalc->setAERotateWeighting(MTRUE);//Algo remove
        } else {
           // m_pIAeCalc->setAERotateWeighting(MFALSE);//Algo remove
        }
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT16 AeMgr::getAEFaceDiffIndex()
{
 return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay)
{
    m_i4ShutterDelayFrames = *i4IspGainDelay - *i4SutterDelay;
    m_i4SensorGainDelayFrames = *i4IspGainDelay - *i4SensorGainDelay;
    if(*i4IspGainDelay > 2) {
        m_i4IspGainDelayFrames = *i4IspGainDelay - ISP_GAIN_DELAY_OFFSET; // for CQ0 1 delay frame
    } else {
        m_i4IspGainDelayFrames = 0; // for CQ0 1 delay frame
    }

    AE_LOG( "[%s] i4SensorDev:%d m_i4ShutterDelayFrames:%d m_i4SensorGainDelayFrames:%d Isp gain:%d %d %d %d\n", __FUNCTION__, m_eSensorDev,
        m_i4ShutterDelayFrames, m_i4SensorGainDelayFrames, m_i4IspGainDelayFrames, *i4SutterDelay, *i4SensorGainDelay, *i4IspGainDelay);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue)
{
MUINT8 uYvalue[5][5];

    if(m_bFrameUpdate == MTRUE) {
        if(m_pIAeCalc != NULL) {
            //m_pIAeCalc->getAEBlockYvalues(&uYvalue[0][0], AE_BLOCK_NO*AE_BLOCK_NO); //Algo remove
            * i4Yvalue = uYvalue[2][2];
        } else {
            AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
        }

        *bFrameUpdate = m_bFrameUpdate;
        m_bFrameUpdate = MFALSE;
    } else {
        * i4Yvalue = 0;
        *bFrameUpdate = MFALSE;
    }

    AE_LOG( "[%s] i4SensorDev:%d Yvalue:%d FrameUpdate:%d\n", __FUNCTION__, m_eSensorDev, * i4Yvalue, *bFrameUpdate);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::SetAETargetMode(eAETargetMODE eAETargetMode)
{
    if(m_eAETargetMode != eAETargetMode) {
       AE_LOG( "[%s] i4SensorDev:%d TargetMode:%d %d\n", __FUNCTION__, m_eSensorDev, m_eAETargetMode, eAETargetMode);
       m_eAETargetMode = eAETargetMode;
       if (m_eAETargetMode == AE_MODE_NORMAL){
           m_rAESettingPreview.HdrEvSetting.u4AECHdrRatio = 100;
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
    if(m_u4EMVHDRratio != u4ratio) {
       AE_LOG( "[%s] i4SensorDev:%d m_u4EMVHDRratio:%d -> %d\n", __FUNCTION__, m_eSensorDev, m_u4EMVHDRratio, u4ratio);
       m_u4EMVHDRratio = u4ratio;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAESensorActiveCycle(MINT32* i4ActiveCycle)
{
    * i4ActiveCycle = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET + 1;
    AE_LOG_IF(m_3ALogEnable,"[%s] i4SensorDev:%d Cycle:%d\n", __FUNCTION__, m_eSensorDev, *i4ActiveCycle);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setVideoDynamicFrameRate(MBOOL bVdoDynamicFps)
{
    if(m_bVideoDynamic != bVdoDynamicFps) {
       AE_LOG( "[%s] i4SensorDev:%d TargetMode:%d %d\n", __FUNCTION__, m_eSensorDev, m_bVideoDynamic, bVdoDynamicFps);
       m_bVideoDynamic = bVdoDynamicFps;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::modifyAEPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100)
{
    if((m_rAEPLineLimitation.bEnable != bEnable) || (m_rAEPLineLimitation.bEquivalent != bEquivalent) || (m_rAEPLineLimitation.u4IncreaseISO_x100 != u4IncreaseISO_x100) || (m_rAEPLineLimitation.u4IncreaseShutter_x100 != u4IncreaseShutter_x100)) {
        AE_LOG( "[%s] i4SensorDev:%d Enable:%d %d Equivalent:%d %d IncreaseISO:%d %d IncreaseShutter: %d %d\n", __FUNCTION__, m_eSensorDev,
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
    MINT32 err;
    MUINT32 u4AFEGain = 0, u4IspGain = AE_GAIN_BASE_ISP, u4BinningRatio = 1;
    MUINT32 u4NewSensitivityRatio = 1024, u4OldSensitivityRatio = 1024, u4SensitivityRatio = 1024;
    CCU_strAEInput rAEInput;
    CCU_strAEOutput rAEOutput;
    MBOOL bStrobeOn;

    if(m_p3ANVRAM == NULL) {
        AE_LOG( "[%s] i4SensorDev:%d Nvram 3A pointer is NULL\n", __FUNCTION__, m_eSensorDev);
        return S_AE_OK;
    }

    // Update new sensor mode senstivity ratio
    switch(i4newSensorMode) {
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
    switch(i4oldSensorMode) {
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

    if(u4OldSensitivityRatio != 0) {
        u4SensitivityRatio = 1024 *  u4OldSensitivityRatio / u4NewSensitivityRatio;
    } else {
        u4SensitivityRatio = 1024;
        AE_LOG( "[%s] i4SensorDev:%d u4OldSensitivityRatio is zero\n", __FUNCTION__, m_eSensorDev);
    }

    if(u4SensitivityRatio <= 300) {
        u4BinningRatio = 4;
    } else if(u4SensitivityRatio <= 450) {
        u4BinningRatio = 3;
    } else if(u4SensitivityRatio <= 768) {
        u4BinningRatio = 2;
    } else {
        u4BinningRatio = 1;
    }

    if(i4newSensorMode != ESensorMode_Capture) {
        MUINT32 i4Totalgain = m_rAESettingCapture.EvSetting.u4AfeGain*m_rAESettingCapture.EvSetting.u4IspGain/u4BinningRatio/1024;
        if (i4Totalgain > m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain){
            m_rAESettingCapture.EvSetting.u4AfeGain = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
            m_rAESettingCapture.EvSetting.u4IspGain = i4Totalgain*AE_GAIN_BASE_ISP/m_rAESettingCapture.EvSetting.u4AfeGain;
        }else if(i4Totalgain > AE_GAIN_BASE_ISP){
            m_rAESettingCapture.EvSetting.u4AfeGain = i4Totalgain;
            m_rAESettingCapture.EvSetting.u4IspGain = AE_GAIN_BASE_ISP;
        }else{
            m_rAESettingCapture.EvSetting.u4Eposuretime = m_rAESettingCapture.EvSetting.u4Eposuretime / u4BinningRatio;
        }
        AE_LOG( "[%s] i4SensorDev:%d New Capture:%d/%d/%d Binning:%d \n", __FUNCTION__, m_eSensorDev, m_rAESettingCapture.EvSetting.u4Eposuretime, m_rAESettingCapture.EvSetting.u4AfeGain, m_rAESettingCapture.EvSetting.u4IspGain, u4BinningRatio);
    }
    return S_AE_OK;
}

MRESULT AeMgr::getAESGG1Gain(MUINT32 *pSGG1Gain)
{
    *pSGG1Gain = m_u4AFSGG1Gain;
    return S_AE_OK;
}

MRESULT AeMgr::switchCapureDiffEVState(MINT32 iDiffEVx1000, CCU_strAEOutput &aeoutput)
{
    if(m_pIAeCalc != NULL) {
        strAERealSetting rAERealSetting;
        m_pIAeCalc->updateAEAlgoInfo(&m_rAEInfoPreview);
        m_pIAeCalc->switchCaptureState(&m_rAEAlgoOutput, &rAERealSetting, iDiffEVx1000);
        memcpy(&(aeoutput.EvSetting), &(rAERealSetting.EvSetting), sizeof(strEvSetting));
        aeoutput.u4ISO = rAERealSetting.u4ISO;
        AE_LOG( "[%s()] i4SensorDev: %d, iDiffEVx1000: %d, exposure setting: %d/%d/%d (%d)\n", __FUNCTION__, m_eSensorDev, iDiffEVx1000,
                aeoutput.EvSetting.u4Eposuretime, aeoutput.EvSetting.u4AfeGain, aeoutput.EvSetting.u4IspGain, aeoutput.u4ISO);
    } else {
        AE_LOG( "[%s()] The AE algo class is NULL  i4SensorDev = %d line:%d", __FUNCTION__, m_eSensorDev, __LINE__);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEOneShotControl(MBOOL bAEControl)
{
    if(m_bAEOneShotControl != bAEControl) {
        AE_LOG( "[%s] i4SensorDev:%d One Shot control old:%d new:%d\n", __FUNCTION__, m_eSensorDev, m_bAEOneShotControl, bAEControl);
       m_bAEOneShotControl = bAEControl;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AeMgr::get3ACaptureDelayFrame()
{
    MINT32 i4CaptureDelayFrame = m_i4SensorCaptureDelayFrame;
    getAESensorActiveCycle(&i4CaptureDelayFrame);
    if((m_bEnableAE == MTRUE) && (m_rAESettingCapture.EvSetting.u4Eposuretime > m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4LongCaptureThres) && (m_bMultiCap == MFALSE) && (m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableLongCaptureCtrl == MTRUE)) {
        AAASensorMgr::getInstance().getSensorModeDelay(m_eSensorMode, &i4CaptureDelayFrame);
        if (i4CaptureDelayFrame < (m_i4IspGainDelayFrames+3)){
            i4CaptureDelayFrame = m_i4TotalCaptureDelayFrame;
        } else {
            m_i4TotalCaptureDelayFrame = i4CaptureDelayFrame;
            m_u4VsyncCnt = m_i4TotalCaptureDelayFrame;
        }
    }
    AE_LOG( "[%s()] i4SensorDev:%d i4CaptureDelayFrame:%d %d %d Shutter:%d MultiCap:%d Thres:%d Enable:%d\n", __FUNCTION__, m_eSensorDev, i4CaptureDelayFrame, m_i4SensorCaptureDelayFrame,
      m_i4TotalCaptureDelayFrame, m_rAESettingCapture.EvSetting.u4Eposuretime, m_bMultiCap, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.u4LongCaptureThres, m_rAEInitInput.rAENVRAM->rAeParam.strAEParasetting.bEnableLongCaptureCtrl);
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
    //AE_LOG( "[%s()] i4SensorDev:%d FlickerFRActive %d->%d\n", __FUNCTION__, m_eSensorDev,a_bFlickerFPSAvtive);
    m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_FLICKER_ACTIVE_UPDATE, a_bFlickerFPSAvtive);
    if (m_bCCUAEFlag && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_WORKING) && m_pIAeFlowCCU->queryStatus((MUINT32)E_AE_FLOW_CCU_AE_WORKING)){
//        m_pIAeFlowCCU->controltrigger((MUINT32) E_AE_FLOW_CCU_TRIGGER_APINFO);
    }else{
        AAASensorMgr::getInstance().setFlickerFrameRateActive((MINT32)m_eSensorDev, a_bFlickerFPSAvtive);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsMultiCapture(MBOOL bMultiCap)
{
    AE_LOG( "[%s()] i4SensorDev:%d bMultiCap:%d\n", __FUNCTION__, m_eSensorDev, bMultiCap);
    m_bMultiCap = bMultiCap;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::IsAEContinueShot(MBOOL bCShot)
{
    AE_LOG( "[%s()] i4SensorDev:%d bCShot:%d\n", __FUNCTION__, m_eSensorDev, bCShot);
    m_bCShot = bCShot;
    if(m_bCShot == MTRUE){
//        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setzCHDRShot(MBOOL bHDRShot)
{
   if ((bHDRShot == 1) || (bHDRShot!=m_bzcHDRshot)){
        AE_LOG( "[%s()] i4SensorDev:%d bZCHDRShot:%d->%d\n", __FUNCTION__, m_eSensorDev, m_bzcHDRshot ,bHDRShot);
        m_bzcHDRshot = bHDRShot;
//        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
   }
   return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableHDRShot(MBOOL bHDRShot)
{
   if ((bHDRShot == 1) || (bHDRShot!=m_bHDRshot)){
        AE_LOG( "[%s()] i4SensorDev:%d bHDRShot:%d->%d\n", __FUNCTION__, m_eSensorDev, m_bHDRshot ,bHDRShot);
        m_bHDRshot = bHDRShot;
//        m_pIAeFlowCPU->controltrigger(E_AE_FLOW_CPU_SCHEDULER_RESET, MTRUE);
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
MRESULT AeMgr::getAAOSize(MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH)
{
    u4BlockNumW = m_rAEConfigInfo.u4BlockNumW;
    u4BlockNumH = m_rAEConfigInfo.u4BlockNumH;
    AE_LOG( "[%s()] i4SensorDev: %d u4BlockNumW/H: %d/%d\n", __FUNCTION__, m_eSensorDev, u4BlockNumW, u4BlockNumH);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::isLVChangeTooMuch()
{
    if(ABS((DOUBLE)m_u4CWVY - (DOUBLE)m_u4StableYValue)/((DOUBLE)m_u4StableYValue) > 0.3){
        AE_LOG( "[%s()] CWVYcur:%d CWVYStable:%d Change/Thres percentage:%d/%d \n", __FUNCTION__ , m_u4CWVY, m_u4StableYValue, 100*ABS((MINT32)m_u4CWVY - (MINT32)m_u4StableYValue)/((MINT32)m_u4StableYValue), 30);
        return MTRUE;
    }
    return MFALSE;
}

MRESULT AeMgr::updateEMISOCaptureParams(AE_MODE_CFG_T &inputparam){

    MUINT32 u4OriExposure = inputparam.u4Eposuretime;
    MUINT32 u4OriAfeGain  = inputparam.u4AfeGain;
    MUINT32 u4OriIspGain  = inputparam.u4IspGain;
    inputparam.u4AfeGain     = m_u4AEISOSpeed*AE_GAIN_BASE_AFE/100;
    if (inputparam.u4AfeGain <= m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain){
        inputparam.u4IspGain = AE_GAIN_BASE_ISP;
    } else {
        inputparam.u4IspGain = inputparam.u4AfeGain*AE_GAIN_BASE_ISP/m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
        inputparam.u4AfeGain = m_rAEInitInput.rAENVRAM->rDevicesInfo.u4MaxGain;
    }
    inputparam.u4Eposuretime = u4OriExposure*u4OriAfeGain/inputparam.u4AfeGain*u4OriIspGain/inputparam.u4IspGain;
    inputparam.u4Eposuretime = (inputparam.u4Eposuretime > EMLongestExp)? EMLongestExp : inputparam.u4Eposuretime;
    if (inputparam.u4Eposuretime < m_pPreviewTableCurrent->pCurrentTable->sPlineTable[0].u4Eposuretime){
        AE_LOG( "[updateEMISOCaptureParams] Calculated/PlineMin %d/%d \n",inputparam.u4Eposuretime, m_pPreviewTableCurrent->pCurrentTable->sPlineTable[0].u4Eposuretime);
        inputparam.u4Eposuretime = m_pPreviewTableCurrent->pCurrentTable->sPlineTable[0].u4Eposuretime;
    }
    AE_LOG( "[%s():EM-ISO] m_u4AEISOSpeed:%d Exp/Afe/Isp:%d/%d/%d -> Exp/Afe/Isp:%d/%d/%d \n", __FUNCTION__,m_u4AEISOSpeed, u4OriExposure,u4OriAfeGain,u4OriIspGain ,inputparam.u4Eposuretime, inputparam.u4AfeGain, inputparam.u4IspGain);
    return S_AE_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::setTGInfo(MINT32 const i4TGInfo)
{
    AE_LOG( "[%s()] i4TGInfo: %d \n", __FUNCTION__, i4TGInfo);

    switch (i4TGInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
        break;
    case CAM_TG_3:
        m_eSensorTG = ESensorTG_3;
        break;
    default:
        CAM_LOGE("i4TGInfo = %d", i4TGInfo);
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen)
{
    AE_LOG( "[getNVRAMParam]\n");

    AE_NVRAM_T *pAENVRAM = reinterpret_cast<AE_NVRAM_T*>(a_pAENVRAM);

    getNvramData(m_eSensorDev);

    if(pAENVRAM != NULL) {
        *pAENVRAM = m_p3ANVRAM->AE[m_u4AENVRAMIdx];
    } else {
        AE_LOG( "NVRAM is NULL\n");
    }

    *a_pOutLen = sizeof(AE_NVRAM_T);

    return S_AE_OK;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AeMgr::getAEInitExpoSetting(AEInitExpoSetting_T &a_rAEInitExpoSetting)
{
    AE_LIGHTSENSOR_OUTPUT_T IndexOutput;

    AE_LOG( "[%s()] m_eSensorDev: %d u4SensorMode: %d u4AETargetMode: %d\n",
            __FUNCTION__, m_eSensorDev,
            a_rAEInitExpoSetting.u4SensorMode, a_rAEInitExpoSetting.u4AETargetMode);
    SetAETargetMode((eAETargetMODE)(a_rAEInitExpoSetting.u4AETargetMode));
    setSensorMode((MINT32)(a_rAEInitExpoSetting.u4SensorMode), 0, 0);

    checkAEPline();

    if(((m_eSensorDev == ESensorDev_Sub) /*|| (m_eSensorDev == ESensorDev_Main)*/) && (m_bInitAE == MTRUE)){
        UpdateLightInfo();
        if((m_i4LightInfo > 0) && (m_i4LightInfo <= 180)){
            IndexOutput.u4Index = m_u4Index;
            IndexOutput.u4IndexF = m_u4IndexF;
            m_pIAeCalc->updateAELSInitIndex(m_i4LightInfo-50, m_i4BVvalue, m_u4IndexMax, m_u4IndexMin, &IndexOutput);
            AE_LOG( "[%s()] Index change by light sensor Index:%d->%d IndexF:%d->%d LightInfo:%d\n m_bInitAE:%d", __FUNCTION__,
                    m_u4Index, IndexOutput.u4Index, m_u4IndexF, IndexOutput.u4IndexF, m_i4LightInfo, m_bInitAE);

            m_u4Index = IndexOutput.u4Index;
            m_u4IndexF = IndexOutput.u4IndexF;
        }
        m_bInitAE = MFALSE;
    }

    updateAEidxtoExpsetting(m_u4Index, m_u4IndexF);
    memcpy(&m_rAEAlgoOutput, &m_rAESettingPreview, sizeof(strAERealSetting));

    // for init exp setting binning sum transformation
    strAERealSetting InitAESettingPreview; // init setting for sensor drv
    memcpy(&InitAESettingPreview, &m_rAESettingPreview, sizeof(strAERealSetting));
    m_u4BinSumRatio = AAASensorMgr::getInstance().getSensorBinningType(m_eSensorDev, a_rAEInitExpoSetting.sensorCfg);
    getBinSumTransSetting(&InitAESettingPreview, m_u4BinSumRatio);

    if (m_eAETargetMode != AE_MODE_NORMAL) {
        a_rAEInitExpoSetting.u4Eposuretime    = InitAESettingPreview.HdrEvSetting.i4LEExpo;
        a_rAEInitExpoSetting.u4Eposuretime_me = InitAESettingPreview.HdrEvSetting.i4MEExpo;
        a_rAEInitExpoSetting.u4Eposuretime_se = InitAESettingPreview.HdrEvSetting.i4SEExpo;
        a_rAEInitExpoSetting.u4AfeGain        = InitAESettingPreview.HdrEvSetting.i4LEAfeGain;
        a_rAEInitExpoSetting.u4AfeGain_me     = InitAESettingPreview.HdrEvSetting.i4MEAfeGain;
        a_rAEInitExpoSetting.u4AfeGain_se     = InitAESettingPreview.HdrEvSetting.i4SEAfeGain;
    } else {
        a_rAEInitExpoSetting.u4Eposuretime = InitAESettingPreview.EvSetting.u4Eposuretime;
        a_rAEInitExpoSetting.u4AfeGain     = InitAESettingPreview.EvSetting.u4AfeGain;
    }

    AE_LOG("[%s()] m_u4BinSumRatio(%d) u4Eposuretime(%d) u4AfeGain(%d) u4Eposuretime_se(%d) u4AfeGain_se(%d) u4Eposuretime_me(%d) u4AfeGain_me(%d) u4Eposuretime_vse(%d) u4AfeGain_vse(%d)\n", __FUNCTION__,
            m_u4BinSumRatio,
            a_rAEInitExpoSetting.u4Eposuretime,     a_rAEInitExpoSetting.u4AfeGain,
            a_rAEInitExpoSetting.u4Eposuretime_se,  a_rAEInitExpoSetting.u4AfeGain_se,
            a_rAEInitExpoSetting.u4Eposuretime_me,  a_rAEInitExpoSetting.u4AfeGain_me,
            a_rAEInitExpoSetting.u4Eposuretime_vse, a_rAEInitExpoSetting.u4AfeGain_vse);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getAE2AFInfo(CCU_AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo)
{

    MUINT8 iYvalue = 0;
    getAEMeteringYvalue(rAeWinSize, &iYvalue);
    rAEInfo.iYvalue = (MINT64)iYvalue;
    rAEInfo.i4ISO = m_rSensorCurrentInfo.u4Sensitivity;
    rAEInfo.i4IsAEStable = m_bAEStable;
    rAEInfo.i4SceneLV = getLVvalue(MTRUE);
    AE_LOG_IF(m_3ALogEnable,"[%s] : rAEInfo.i4SceneLV = %d",__FUNCTION__,rAEInfo.i4SceneLV);
    rAEInfo.ishutterValue = m_rAESettingPreview.EvSetting.u4Eposuretime;
    rAEInfo.i4DeltaBV = m_i4DeltaBV;
    getAEBlockYvalues(rAEInfo.aeBlockV, 25);
    MUINT16 u2YCnt = 0;
    getAEMeteringBlockAreaValue(rAeWinSize, rAEInfo.pAEBlockAreaYvalue, &u2YCnt);
    rAEInfo.i4AEBlockAreaYCnt = (MINT32)u2YCnt;
    rAEInfo.i4IsAELocked = m_bAELock || m_bAPAELock;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg)
{
#if 0
    MRESULT err;
    if(pDBinInfo == NULL || pOutRegCfg == NULL) {
        CAM_LOGE("[%s] : pDBinInfo is NULL or pOutRegCfg is NULL", __FUNCTION__);
        return S_AE_OK;
    }
    BIN_INPUT_INFO *psDBinInfo = static_cast<BIN_INPUT_INFO*>(pDBinInfo);
    MUINT32 width  = psDBinInfo->TarQBNOut_W;
    MUINT32 height = psDBinInfo->TarBinOut_H;

    if(m_u4Hbinwidth != width || m_u4Hbinheight != height) {
        AE_LOG("[%s] : QBN (%d , %d) -> (%d , %d), m_u4Hbinwidth/m_u4Hbinheight (%d , %d) -> (%d , %d)", __FUNCTION__,
                 psDBinInfo->CurQBNOut_W, psDBinInfo->CurBinOut_H, width, height,
                 m_u4Hbinwidth, m_u4Hbinheight, width, height);
        m_u4Hbinwidth = width;
        m_u4Hbinheight = height;
        m_pIAeCalc->modifyHistogramWinConfig(m_eZoomWinInfo, &m_rAEStatCfg, m_u4Hbinwidth, m_u4Hbinheight, m_i4AETgValidBlockWidth, m_i4AETgValidBlockHeight);

        // Window size
        MUINT32 u4BlocksizeX = ((m_u4Hbinwidth / m_rAEConfigInfo.u4BlockNumW)/2)*2;
        MUINT32 u4BlocksizeY = ((m_u4Hbinheight / m_rAEConfigInfo.u4BlockNumH)/2)*2;
        m_rAEConfigInfo.u4AEOverExpCntShift = (((u4BlocksizeX/4)*(u4BlocksizeY/2)>255)?MTRUE:MFALSE);

        AAA_TRACE_D("AEStatReconfig");
        ISP_AE_CONFIG_T::getInstance(m_eSensorDev).AEConfig(&m_rAEStatCfg, &m_rAEConfigInfo, &m_sAEResultConfig);
        AAA_TRACE_END_D;
    }
#endif
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::updateStereoDenoiseRatio(MINT32* i4StereoDenoiserto)
{
    AE_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d BMDN/MFHR:%d/%d->%d/%d\n", __FUNCTION__, m_eSensorDev, m_i4StereoDenoiserto[0], m_i4StereoDenoiserto[1], *i4StereoDenoiserto, *(i4StereoDenoiserto+1));
    m_i4StereoDenoiserto[0] = *i4StereoDenoiserto;
    m_i4StereoDenoiserto[1] = *(i4StereoDenoiserto+1);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableStereoDenoiseRatio(MUINT32 u4enableStereoDenoise)
{
    if(m_u4StereoDenoiseMode != u4enableStereoDenoise){
        AE_LOG_IF(m_3ALogEnable,"[%s():e] i4SensorDev:%d Stereo denoise:%d->%d\n", __FUNCTION__, m_eSensorDev, m_u4StereoDenoiseMode, u4enableStereoDenoise);
        m_u4StereoDenoiseMode = u4enableStereoDenoise;
        //update pass2 OB2
        updateAEInfo2ISP(AE_FLOW_STATE_PREVIEW, m_rAESettingPreview, m_rAESettingPreview);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::enableAEStereoManualPline(MBOOL bstereomode)
{
    if (m_bStereoManualPline != bstereomode){
        AE_LOG( "[%s()] Stereo %d -> %d , m_eSensorMode = %d\n", __FUNCTION__, m_bStereoManualPline, bstereomode, m_eSensorMode);

        if ((bstereomode == MTRUE) && (m_bStereoManualPline == MFALSE)){
            // Switch to Stereo Pline
            m_pAEMapPlineTable = (AE_PLINEMAPPINGTABLE_T *)(&(m_rAEInitInput.rAENVRAM->rAeParam.strStereoPlineMapping.sAESceneMapping[0]));
            // CCU control
            // if (m_bCCUAEFlag){
            //   m_pIAeFlowCCU->stop();
            //   m_pIAeFlowCCU->uninit();
            //   m_bCCUAEFlag = MFALSE;
            // }
        } else if ((bstereomode == MFALSE) && (m_bStereoManualPline == MTRUE)){
            // Switch to Stereo Pline
            m_pAEMapPlineTable = &(m_rAEInitInput.rAEPlineMapTable[0]);
        }

       m_bStereoManualPline = bstereomode;
       for (MINT32 i = 0; i<30 ; i++){
               AE_LOG_IF(m_3ALogEnable,"[%s()] i4SensorDev:%d  eID/ePlinetable = %d - %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d \n", __FUNCTION__, m_eSensorDev, (MINT32)m_pAEMapPlineTable[i].eAEScene,
               (MINT32)m_pAEMapPlineTable[i].ePLineID[0],(MINT32)m_pAEMapPlineTable[i].ePLineID[1],(MINT32)m_pAEMapPlineTable[i].ePLineID[2],
               (MINT32)m_pAEMapPlineTable[i].ePLineID[3],(MINT32)m_pAEMapPlineTable[i].ePLineID[4],(MINT32)m_pAEMapPlineTable[i].ePLineID[5],
               (MINT32)m_pAEMapPlineTable[i].ePLineID[6],(MINT32)m_pAEMapPlineTable[i].ePLineID[7],(MINT32)m_pAEMapPlineTable[i].ePLineID[8],
               (MINT32)m_pAEMapPlineTable[i].ePLineID[9],(MINT32)m_pAEMapPlineTable[i].ePLineID[10]);
       }
       m_bPlineMappingChange = MTRUE;
    }

    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT AeMgr::getAEdelayInfo(MINT32* i4ExpDelay, MINT32* i4AEGainDelay, MINT32* i4IspGainDelay)
{
    MRESULT err;
//    AE_LOG( "[%s()] Exp/Afe/Isp delay1 frame = %d/%d/%d , AfeGainFlg = %d\n", __FUNCTION__, *i4ExpDelay, *i4AEGainDelay, *i4IspGainDelay, m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD));
    *i4ExpDelay = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET - m_i4ShutterDelayFrames;
    *i4AEGainDelay = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET - m_i4SensorGainDelayFrames;
    *i4IspGainDelay = m_i4IspGainDelayFrames + ISP_GAIN_DELAY_OFFSET;
    if(m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD))
        *i4AEGainDelay = *i4AEGainDelay-1;
    AE_LOG( "[%s()] Exp/Afe/Isp delay frame = %d/%d/%d , AfeGainFlg = %d\n", __FUNCTION__, *i4ExpDelay, *i4AEGainDelay, *i4IspGainDelay, m_pIAeSettingCPU->queryStatus(E_AE_SETTING_CPU_STATUS_SENSORGAINTHREAD));
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getTgIntAEInfo(MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio)
{
    bTgIntAEEn = MFALSE;
    fTgIntAERatio = 0.6;
    return S_AE_OK;
}

MRESULT AeMgr::updateAEBV(MVOID *pAEStatBuf)
{
   // temp: to remove this function at p80, please call dopvae + skip cal for update

   MY_LOG("[monitorAndReschedule : %s] temp: to remove this function at p80, please call dopvae + skip cal for update \n", __FUNCTION__);

   return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setDigZoomRatio(MINT32 i4ZoomRatio)
{
    if(m_i4ZoomRatio != i4ZoomRatio){
        MY_LOG("[%s] i4SensorDev:%d, ZoomRatio:%d->%d\n", __FUNCTION__, m_eSensorDev, m_i4ZoomRatio, i4ZoomRatio);
        m_i4ZoomRatio = i4ZoomRatio;
    }

    return S_AE_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setCCUOnOff(MBOOL enable)
{
  if(enable){
      AE_LOG("setCCUOnOff : On\n");
      AE_FLOW_CCU_INIT_INFO_T rCCUInitInfo;
      initializeCCU(&rCCUInitInfo);
      initializeCCUAE(&rCCUInitInfo);
      m_bCCUAEFlag = MTRUE;
  }
  else {
    // Stop CCU
    if (m_bCCUAEFlag){
      AE_LOG("setCCUOnOff : Off\n");
      m_pIAeFlowCCU->pause();
      uninitializeCCU();
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
    // temp: fix me at p80
//    if((a_strExpInput.u4Sensitivity == 0) && (a_strExpInput.u4ExposureTime != 0)) {              // ISO auto
//        a_strExpOutput.u4ExposureTime = a_strExpInput.u4ExposureTime;
//        a_strExpOutput.u4Sensitivity = ((MINT64)m_rAEOutput.rCaptureMode[0].u4Eposuretime * m_rAEOutput.rCaptureMode[0].u4RealISO) / a_strExpInput.u4ExposureTime;
//        if(a_strExpOutput.u4Sensitivity < m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain) {
//            a_strExpOutput.u4Sensitivity = m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
//        }
//    } else if((a_strExpInput.u4ExposureTime == 0) && (a_strExpInput.u4Sensitivity != 0)) {     // Exp auto
//        a_strExpOutput.u4Sensitivity = a_strExpInput.u4Sensitivity;
//        a_strExpOutput.u4ExposureTime = ((MINT64)m_rAEOutput.rCaptureMode[0].u4Eposuretime * m_rAEOutput.rCaptureMode[0].u4RealISO) / a_strExpInput.u4Sensitivity;
//    } else if((a_strExpInput.u4ExposureTime != 0) && (a_strExpInput.u4Sensitivity != 0)){                                         // keep value
//        a_strExpOutput.u4ExposureTime = a_strExpInput.u4ExposureTime;
//        a_strExpOutput.u4Sensitivity = a_strExpInput.u4Sensitivity;
//    } else {
//        a_strExpOutput.u4ExposureTime = m_rAEOutput.rCaptureMode[0].u4Eposuretime;
//        a_strExpOutput.u4Sensitivity = m_rAEOutput.rCaptureMode[0].u4RealISO;
//    }

//    MY_LOG("[%s] Input Shutter:%d ISO:%d, Output Shutter:%d ISO:%d Capture Shutter:%d ISO:%d MinISO(%d):%d\n",
//        __FUNCTION__, a_strExpInput.u4ExposureTime, a_strExpInput.u4Sensitivity, a_strExpOutput.u4ExposureTime, a_strExpOutput.u4Sensitivity, m_rAEOutput.rCaptureMode[0].u4Eposuretime, m_rAEOutput.rCaptureMode[0].u4RealISO, m_u4AENVRAMIdx, m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain);

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    if(a_ISOIdx1Status != NULL) {
        if(m_u4ISOIdx1StableCount >= 2*m_u4Idx1StableThres) {   // > ISO value 1 and stable
            *a_ISOIdx1Status = 0;
        } else if (m_u4ISOIdx1StableCount <= 0) {   // < ISO value 1 and stable
            *a_ISOIdx1Status = 1;
        } else {                                    // unstable
            *a_ISOIdx1Status = 2;
        }
    } else {
        MY_LOG("[%s] a_ISOIdx1Status:NULL pointer m_u4ISOIdx1StableCount:%d, m_u4ISOIdx2StableCount:%d MAX_INDEX1_STABLE_THRES:%d->%d m_u4ISOIdx1StableCount:%d m_u4Index:%d m_u4IndexMax:%d\n",
            __FUNCTION__, m_u4ISOIdx1StableCount, m_u4ISOIdx2StableCount, MAX_INDEX1_STABLE_THRES, m_u4Idx1StableThres, m_u4ISOIdx1StableCount,  m_u4Index, m_u4IndexMax);
    }

    if(a_ISOIdx2Status != NULL) {
        if(m_u4ISOIdx2StableCount >= 2*m_u4Idx2StableThres) {   // > ISO value 2 and stable
            *a_ISOIdx2Status = 0;
        } else if (m_u4ISOIdx2StableCount <= 0) {   // < ISO value 1 and stable
            *a_ISOIdx2Status = 1;
        } else {                                    // unstable
            *a_ISOIdx2Status = 2;
        }
    } else {
        MY_LOG("[%s] a_ISOIdx2Status:NULL pointer m_u4ISOIdx1StableCount:%d, m_u4ISOIdx2StableCount:%d MAX_INDEX_STABLE2_THRES:%d->%d m_u4ISOIdx1StableCount:%d m_u4Index:%d m_u4IndexMax:%d\n",
            __FUNCTION__, m_u4ISOIdx1StableCount, m_u4ISOIdx2StableCount, MAX_INDEX2_STABLE_THRES, m_u4Idx2StableThres, m_u4ISOIdx1StableCount,  m_u4Index, m_u4IndexMax);
    }

    if((a_ISOIdx1Status != NULL) && (a_ISOIdx2Status != NULL)) {
        MY_LOG("[%s] a_ISOIdx1Status:%d m_u4ISOIdx1StableCount:%d, a_ISOIdx2Status:%d m_u4ISOIdx2StableCount:%d INDEX1_STABLE_THRES:%d->%d INDEX2_STABLE_THRES:%d->%d m_u4Index:%d m_u4IndexMax:%d\n",
        __FUNCTION__, *a_ISOIdx1Status, m_u4ISOIdx1StableCount, *a_ISOIdx2Status, m_u4ISOIdx2StableCount, MAX_INDEX1_STABLE_THRES, m_u4Idx1StableThres, MAX_INDEX2_STABLE_THRES, m_u4Idx2StableThres, m_u4Index, m_u4IndexMax);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::resetGetISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status)
{
    m_u4ISOIdx1StableCount = m_u4Idx1StableThres;
    m_u4ISOIdx2StableCount = m_u4Idx2StableThres;
    MY_LOG("[%s] reset m_u4ISOIdx1StableCount / m_u4ISOIdx2StableCount to 0\n",
        __FUNCTION__);
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateGyroInfo()
{
 #if 1
  // get gyro/acceleration data
  SensorData gyroDa;
  MBOOL gyroDaVd = m_bGryoVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_GYRO, gyroDa) : MFALSE;

  if( gyroDaVd && gyroDa.timestamp)
  {
    //m_u8PreGyroTS = m_u8GyroTS;
    m_i4GyroInfo[0] = gyroDa.gyro[0] * SENSOR_GYRO_SCALE;
    m_i4GyroInfo[1] = gyroDa.gyro[1] * SENSOR_GYRO_SCALE;
    m_i4GyroInfo[2] = gyroDa.gyro[2] * SENSOR_GYRO_SCALE;
    AE_LOG_IF(m_3ALogEnable,"[%s()] m_i4GyroInfo0/1/2 = %d/%d/%d \n", __FUNCTION__,m_i4GyroInfo[0], m_i4GyroInfo[1], m_i4GyroInfo[2]);
    //m_u8GyroTS = gyroDa.timestamp;
    //u4GyroScale = m_u8GyroTS!=m_u8PreGyroTS ? SENSOR_GYRO_SCALE : 0;
  }
  else
  {
    AE_LOG("Gyro InValid!");
    //u4GyroScale = 0; // set scale 0 means invalid to algo
  }
  SensorData acceDa;
  MBOOL acceDaVd = m_bAcceVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_ACCELERATION, acceDa) : MFALSE;

 if( acceDaVd && acceDa.timestamp)
 {
   //m_u8PreAcceTS = m_u8AcceTS;
   m_i4AcceInfo[0] = acceDa.acceleration[0] * SENSOR_ACCE_SCALE;
   m_i4AcceInfo[1] = acceDa.acceleration[1] * SENSOR_ACCE_SCALE;
   m_i4AcceInfo[2] = acceDa.acceleration[2] * SENSOR_ACCE_SCALE;
   //AE_LOG( "[%s()] m_i4AcceInfo[1] = %d \n", __FUNCTION__,m_i4AcceInfo[1]);
   //m_u8AcceTS = acceDa.timestamp;

   //u4ACCEScale = m_u8AcceTS!=m_u8PreAcceTS ? SENSOR_ACCE_SCALE : 0;
 }
 else
 {
   AE_LOG("Acce InValid!");
   //u4ACCEScale = 0; // set scale 0 means invalid to algo
 }
 #endif
 return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateLightInfo()
{
 #if 1
  // get light data
  SensorData lightDa;
  MBOOL lightDaVd = m_bLightVd ? mpSensorProvider->getLatestSensorData(SENSOR_TYPE_LIGHT, lightDa) : MFALSE;

  if(lightDaVd && lightDa.timestamp) {
      if(lightDa.light == 0) {
        m_i4LightInfo = 0; // (0+3)*10
      } else {
        m_i4LightInfo = (MINT32) (((double)log(lightDa.light)/log(2)+3)*10);
      }
      AE_LOG("[%s()] m_i4LightInfo:%d log:%f light:%f\n", __FUNCTION__,m_i4LightInfo, log(lightDa.light), lightDa.light);
  } else {
      AE_LOG("[%s()] Light InValid! lightDaVd:%d timestamp:%lld light:%f\n", __FUNCTION__,lightDaVd, lightDa.timestamp, lightDa.light);
  }
 #endif
 return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::resetMvhdrRatio(MBOOL bReset)
{
    if (bReset) {
        m_u4CurVHDRratio = 100;
        AE_LOG( "[%s()] reset mvhdr ratio\n", __FUNCTION__);
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
    switch(eSensorDev) {
        case ESensorDev_Main:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub:
            i4SensorDevID = NSCam::SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_Main3D:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN_3D;
            break;
        case ESensorDev_SubSecond:
            i4SensorDevID = NSCam::SENSOR_DEV_SUB_2;
            break;
        default:
            i4SensorDevID = NSCam::SENSOR_DEV_NONE;
            break;
    }
    // Get Sensor ID
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    pIHalSensorList->querySensorStaticInfo(i4SensorDevID, &sInfo);
    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev:%d sensorID:%d\n", __FUNCTION__, eSensorDev, sInfo.sensorDevID);

    return sInfo.sensorDevID;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AeMgr::getISPRegNormalPipe(MUINT32 RegAddr)
{
    MUINT32 ret = 0;
    MUINT32 retValue =0;

    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe( m_i4SensorIdx, LOG_TAG);
    if( pPipe==NULL)
    {
        CAM_LOGE( "Fail to create NormalPipe");
    }
    else
    {
        ret = pPipe->getIspReg( RegAddr, 1, &retValue, MTRUE);
        pPipe->destroyInstance( LOG_TAG);
    }

    return retValue;
}

MRESULT AeMgr::setRequestNum(MUINT32 u4ReqNum)
{
    AE_LOG_IF(m_3ALogEnable, "[%s()] i4SensorDev: %d, m_u4RequestNum: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_u4RequestNum, u4ReqNum);
    m_u4RequestNum = u4ReqNum;

    return S_AE_OK;
}

MBOOL AeMgr::getAEInfo(AE_PERFRAME_INFO_T& rAEPerframeInfo)
{
    memcpy(&rAEPerframeInfo, &m_AEPerFrameInfo, sizeof(AE_PERFRAME_INFO_T));

    return MTRUE;
}

MRESULT AeMgr::configReg(AEResultConfig_T *pResultConfig)
{

    memcpy(pResultConfig, &m_sAEResultConfig, sizeof(AEResultConfig_T));
    return S_AE_OK;
}

