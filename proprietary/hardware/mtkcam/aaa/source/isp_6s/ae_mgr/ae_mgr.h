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
/**
 * @file ae_mgr.h
 * @brief AE manager
 */

#ifndef _AE_MGR_H_
#define _AE_MGR_H_

#include <isp_tuning.h>
#include <ae_mgr_if.h>
//#include <ae_algo_if.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_ivhdr.h"
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <IThread.h>
#include <AeFlow/IAeFlow.h>
#include <AeSetting/IAeSetting.h>
#include <Hal3AFlowCtrl.h>
#include "camera_custom_ae_tuning.h"
#include "camera_custom_ae.h"

//CCU
#define AE_MGR_INCLUDING_CCU
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "algorithm/ccu_n3d_ae_algo_data.h"
#include "algorithm/ccu_ae_param.h"
#include "algorithm/cpu_ae_algo_ctrl_if.h" // temp
#include "iccu_ctrl_ae.h"

#include <ae_calc_if.h>

#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>

typedef enum
{
    AE_FLOW_STATE_INIT    = -1,
    AE_FLOW_STATE_PREVIEW =  0,
    AE_FLOW_STATE_AE2AF,
    AE_FLOW_STATE_CAPTURE,
    AE_FLOW_STATE_POSTCAP,
    AE_FLOW_STATE_MANUAL,
    AE_FLOW_STATE_RESTORE,
    /* VHDR FLOW STATE */
    AE_FLOW_STATE_VHDR_INIT,
    AE_FLOW_STATE_VHDR_PREVIEW,
    AE_FLOW_STATE_VHDR_CAPTURE,
    AE_FLOW_STATE_VHDR_MANUAL,
} AE_FLOW_STATE_T;

typedef enum
{
    E_RTV_AE_INVALID        = 200,
    E_RTV_AE_SHUTTER        = 201,
    E_RTV_AE_ISO            = 202,
    E_RTV_AE_LV             = 203,
    E_RTV_AE_STATE          = 204,
    E_RTV_AE_SCENARIO_MODE  = 205,
    E_RTV_AE_CWV            = 206,
    E_RTV_AE_INDEXF         = 207,
    E_RTV_AE_STABLE         = 208,
    E_RTV_AE_FACE_NUM       = 209,
    E_RTV_AE_FACE_TARGET    = 210,
    E_RTV_AE_FINAL_TARGET   = 211,
    E_RTV_AE_AVGY           = 212,
    E_RTV_AE_PRE_PLINE      = 213,
    E_RTV_AE_CAP_PLINE      = 214,
    E_RTV_AE_HDR_RATIO      = 215,
    E_RTV_AE_MAGIC_NUM      = 216,
    E_RTV_AE_ENUM_INDEX_MAX, // the total number of possible rtv info.
} RTV_AE_ENUM_INDEX;

#define ISP_GAIN_DELAY_OFFSET 2
#define INIT_WOF_WAIT_CYCLE 5
#define PV_CWVY_FLOOR 30
#define PV_CWVY_CEILING 200
#define INIT_WAIT_FD_SKIP_AE 0
#define MAX_AE_SETTING_NUM 3

// Adb Manual  ISO speed Max
#define MAX_ADB_ISO_VALUE 12800

// Manual AE Pline control
//#define AE_PLINE_EISRECORD 25       //g_AE_SceneTable9   (Manual)
//#define AE_PLINE_SM240FPS 26        //g_AE_SceneTable10  (Manual)
//#define AE_PLINE_STEREO_PREVIEW 27  //g_AE_SceneTable11  (Stereo mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_STEREO_CAPTURE 28  //g_AE_SceneTable12  (Stereo mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_STEREO_VIDEO 29    //g_AE_SceneTable13  (Stereo mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_AIS_CAPTURE 30     //g_AE_SceneTable14  (Manual Control for getExposureInfo)
//#define AE_PLINE_STEREO_ISO100 31   //g_AE_SceneTable15  (Stereo mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_HDR_Preview 32     //g_AE_SceneTable16  (HDR mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_HDR_Capture 33     //g_AE_SceneTable17  (HDR mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_HDR_Video 34       //g_AE_SceneTable18  (HDR mapping table - Ae_tuning_cusom_main.cpp)
//#define AE_PLINE_STEREO_BMDN 35     //g_AE_SceneTable19  (Manual Control for getExposureInfo)
//#define AE_PLINE_STEREO_MFHR 36     //g_AE_SceneTable20  (Manual Control for getExposureInfo)

// define 4-cell AE parameter mode
#define FOUR_CELL_ADD_SHUTTER 1

namespace NSCam
{
namespace Utils
{
class SensorProvider;
}
}

namespace NS3Av3
{

/*******************************************************************************
*
*******************************************************************************/
/**
 * @brief AE manager
 */

class AeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AeMgr(AeMgr const&);
    //  Copy-assignment operator is disallowed.
    AeMgr& operator=(AeMgr const&);

public:  ////
    AeMgr(ESensorDev_T eSensorDev);
    ~AeMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AeMgr& getInstance(MINT32 const i4SensorDev);
    static AeMgr* s_pAeMgr; // FIXME: REMOVED LATTER
    MRESULT cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam);
    MRESULT Start();
    MRESULT uninit();
    MRESULT Stop();
    MRESULT setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea);
    MRESULT setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep);
    MRESULT setAEMeteringMode(MUINT32 u4NewAEMeteringMode);
    MRESULT setAEISOSpeed(MUINT32 i4NewAEISOSpeed);
    MRESULT setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    MRESULT setAEFlickerMode(MUINT32 u4NewAEFLKMode);
    MRESULT setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode);
    MRESULT setFlickerFrameRateActive(MBOOL a_bFlickerFPSAvtive);
    MRESULT enableAEManualPline(EAEManualPline_T eAEManualPline, MBOOL bEnable);
    MRESULT enableAEManualCapPline(EAEManualPline_T eAEManualCapPline, MBOOL bEnable);
    MRESULT setAECamMode(MUINT32 u4NewAECamMode);
    MRESULT setAEShotMode(MUINT32 u4NewAEShotMode);
    MRESULT setCamScenarioMode(MUINT32 u4NewCamScenarioMode, MBOOL bInit = MFALSE);
    MRESULT setNVRAMIndex(MUINT32 a_eNVRAMIndex, MBOOL bInit = MFALSE);
    MRESULT setAEHDRMode(MUINT32 u4NewAEHDRMode);
    MINT32 getAEHDROnOff();
    MRESULT setSceneMode(MUINT32 u4NewScene);
    MINT32 getAEScene() const;
    MRESULT setAEMode(MUINT32 u4NewAEmode, MBOOL isForce = MFALSE);
    MINT32 getAEMode() const;
    MINT32 getAEState() const;
    MBOOL setSensorMode(MINT32 i4NewSensorMode, MUINT32 width, MUINT32 height);
    MRESULT setAPAELock(MBOOL bAPAELock, MBOOL bMetaSetCtrl = MFALSE);
    MRESULT setAFAELock(MBOOL bAFAELock);
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT setAAOProcInfo(CCU_AAO_PROC_INFO_T const *sNewAAOProcInfo);
    MRESULT setMVHDR3ExpoProcInfo(MVOID *pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100);
    MRESULT doPostCapAE(MVOID *pAEStatBuf, MBOOL bIsStrobe);
    MRESULT doAFAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MRESULT doPreCapAE(MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MRESULT doCapAE();
    MRESULT doBackupAE();
    MRESULT doRestoreAE();
    MRESULT doPvAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MINT32 getLVvalue(MBOOL isStrobeOn);
    MINT32 getAOECompLVvalue(MBOOL isStrobeOn);
    MINT32 getCaptureLVvalue();
    MINT32 getEVCompensateIndex();
    MRESULT getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo);
    MRESULT getCurrentPlineTableF(strFinerEvPline &a_PrvAEPlineTable);
    MRESULT getAECapPlineTable(MINT32 *i4CapIndex, strAETable &a_AEPlineTable);
    MRESULT getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo);
    MBOOL IsStrobeBVTrigger();
    MRESULT getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo);
    MRESULT updateSensorListenerParams(MINT32 *i4SensorInfo);
    MRESULT updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext, MINT32 i4AEidxNextF);
    MRESULT updateAEUnderExpdeltaBVIdx(MINT32 i4AEdeltaBV);
    MRESULT getCaptureSetting(strAERealSetting* pCapSetting);
    MRESULT getExposureInfo(ExpSettingParam_T &strHDRInputSetting);
    MRESULT getCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT getAEMeteringYvalue(CCU_AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    MRESULT getAEMeteringBlockAreaValue(CCU_AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt);
    MRESULT setFDenable(MBOOL bFDenable);
    MRESULT getAEFDMeteringAreaInfo(std::vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight);
    MRESULT setFDInfo(MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight);
    MRESULT setOTInfo(MVOID* a_sOT);
    MRESULT setStrobeMode(MBOOL bIsStrobeOn);
    MRESULT setAERotateDegree(MINT32 i4RotateDegree);
    MBOOL getAECondition(MUINT32 i4AECondition);
    MRESULT getLCEPlineInfo(LCEInfo_T &a_rLCEInfo);
    MINT16 getAEFaceDiffIndex();
    MRESULT updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay);
    MRESULT getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue);
    MBOOL getAENvramData(AE_NVRAM_T &rAENVRAM);
    MRESULT getNvramData(MINT32 i4SensorDev, MBOOL isForce = MFALSE);
	MRESULT updateISPNvramLTMIndex(MINT32 i4Tableidx);
    MRESULT getAEBlockYvalues(MUINT8 *pYvalues, MUINT8 size);
    MRESULT SetAETargetMode(eAETargetMODE eAETargetMode);
    MRESULT setEMVHDRratio(MUINT32 u4ratio);
    MRESULT getAESensorActiveCycle(MINT32* i4ActiveCycle);
    MRESULT setVideoDynamicFrameRate(MBOOL bVdoDynamicFps);
    MRESULT modifyAEPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100);
    MRESULT getAESGG1Gain(MUINT32 *pSGG1Gain);
    MRESULT enableAEOneShotControl(MBOOL bAEControl);
    MRESULT getAEPlineTable(eAETableID eTableID, strAETable &a_AEPlineTable);
    MBOOL IsNeedUpdateSensor();
    MINT32 getAEAutoFlickerState();
    MRESULT updateSensorbyI2C();
    MRESULT updateSensorbyI2CBufferMode();
    MRESULT UpdateSensorParams(AE_SENSOR_PARAM_T strSensorParams, MBOOL bForceUpdate = MFALSE);
    MRESULT bBlackLevelLock(MBOOL bLockBlackLevel);
    MRESULT getSensorParams(AE_SENSOR_PARAM_T &a_rSensorInfo);
    MRESULT getAEConfigParams(MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen);
    MINT64 getSensorRollingShutter() const;
    MINT32 get3ACaptureDelayFrame();
    MRESULT IsMultiCapture(MBOOL bMultiCap);
    MRESULT IsAEContinueShot(MBOOL bCShot);
    MRESULT updateStereoDenoiseRatio(MINT32* i4StereoDenoiserto);
    MRESULT enableStereoDenoiseRatio(MUINT32 u4enableStereoDenoise);
    MRESULT enableAEStereoManualPline(MBOOL bstereomode);
    MRESULT setzCHDRShot(MBOOL bHDRShot);
    MRESULT enableHDRShot(MBOOL bHDRShot);
    MBOOL isLVChangeTooMuch();
    MRESULT switchCapureDiffEVState(MINT32 iDiffEVx1000, CCU_strAEOutput &aeoutput);
    MBOOL setTGInfo(MINT32 const i4TGInfo);
    MRESULT getAAOSize(MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH);
    MRESULT getAEdelayInfo(MINT32* i4ExpDelay, MINT32* i4AEGainDelay, MINT32* i4IspGainDelay);
    MRESULT getTgIntAEInfo(MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio);
    MRESULT getAFAEOutput(CCU_strAEOutput &a_rAEOutput);
    // EM feature APIs.
    MINT32 getNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    MRESULT updateAEBV(MVOID *pAEStatBuf);
    MRESULT getAEInitExpoSetting(AEInitExpoSetting_T &a_rAEInitExpoSetting);
    MRESULT getAE2AFInfo(CCU_AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo);
    MRESULT reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg);
    MRESULT switchExpSettingByShutterISOpriority(AE_EXP_SETTING_T &strExpInput, AE_EXP_SETTING_T &strExpOutput);
    MRESULT getISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status);
    MRESULT resetGetISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status);
    MRESULT resetMvhdrRatio(MBOOL bReset);

    // CCT feature APIs.
    MINT32 CCTOPAEEnable();
    MINT32 CCTOPAEDisable();
    MINT32 CCTOPAEGetEnableInfo(MINT32 *a_pEnableAE, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetSceneMode(MINT32 a_AEScene);
    MINT32 CCTOPAEGetAEScene(MINT32 *a_pAEScene, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetMeteringMode(MINT32 a_AEMeteringMode);
    MINT32 CCTOPAEApplyExpParam(MVOID *a_pAEExpParam);
    MINT32 CCTOPAESetFlickerMode(MINT32 a_AEFlickerMode);
    MINT32 CCTOPAEGetExpParam(MVOID *a_pAEExpParamIn, MVOID *a_pAEExpParamOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetFlickerMode(MINT32 *a_pAEFlickerMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEGetMeteringMode(MINT32 *a_pAEMEteringMode, MUINT32 *a_pOutLen);
    MINT32 CCTOPAEApplyNVRAMParam(MVOID *a_pAENVRAM, MUINT32 u4CamScenarioMode);
    MINT32 CCTOPAEGetNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESaveNVRAMParam();
    MINT32 CCTOPAEGetCurrentEV(MINT32 *a_pAECurrentEV, MUINT32 *a_pOutLen);
    MINT32 CCTOPAELockExpSetting();
    MINT32 CCTOPAEUnLockExpSetting();
    MINT32 CCTOSetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOGetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOPSetAETargetValue(MUINT32 u4AETargetValue);
    MINT32 CCTOPAEApplyPlineNVRAM(MVOID *a_pAEPlineNVRAM);
    MINT32 CCTOPAEGetPlineNVRAM(MVOID *a_pAEPlineNVRAM,MUINT32 *a_pOutLen);
    MINT32 CCTOPAESavePlineNVRAM();
    MBOOL getCCUresultCBActive(MVOID* pfCB);
    MRESULT setDigZoomRatio(MINT32 i4ZoomRatio);
    MRESULT setCCUOnOff(MBOOL enable);
    MRESULT CCUManualControl(MUINT32 u4ExpTime,MUINT32 u4AfeGain,MUINT32 u4IspGain,MBOOL EnableManual);
    MRESULT setExpoSetting(MUINT32 u4ExpTime, MUINT32 u4AfeGain, MUINT32 u4IspGain);
    MRESULT IsCCUAEInit(MBOOL bInit);
    MRESULT setRequestNum(MUINT32 u4ReqNum);
    MRESULT getAEInfo(AE_PERFRAME_INFO_T& rAEPerframeInfo);
    MRESULT configReg(AEResultConfig_T *pResultConfig);
    //----ae_mgr_ctrl.cpp----//
	MRESULT sendAECtrl(EAECtrl_T eAECtrl, MINTPTR iArg1, MINTPTR iArg2, MINTPTR iArg3, MINTPTR iArg4);
	MRESULT setAEParams(AE_PARAM_SET_INFO const &rNewParam);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MRESULT getSensorResolution();

    MRESULT AEInit(Param_T &rParam);
    MRESULT prepareCapParams();
    MRESULT prepareAE2AFParams();
    MRESULT PreviewAEInit(MINT32 m_i4SensorIdx, Param_T &rParam);
    MRESULT updateEMISOCaptureParams(AE_MODE_CFG_T &inputparam);
    MRESULT ModifyCaptureParamsBySensorMode(MINT32 i4newSensorMode, MINT32 i4oldSensorMode);
    MRESULT setAELock();
    MBOOL   setAEMgrStateCPU(MUINT32 u4AEMgrStateCPU);
    // AeMgr p-line
    MRESULT checkAEPline();
    MRESULT getAEPLineMappingID(LIB3A_AE_SCENE_T  a_eAESceneID, MINT32 i4SensorMode, eAETableID *pAEPrePLineID, eAETableID *pAECapPLineID);
    MRESULT searchAETable(AE_PLINETABLE_T *a_aepara ,eAETableID id,strAETable** a_ppPreAETable);
    MRESULT searchPreviewIndexLimit();
    MRESULT setAEMinMaxFrameRatePlineIdx(MINT32 a_eAEMinFrameRate, MINT32 a_eAEMaxFrameRate);
    MRESULT getSenstivityDeltaIndex(MUINT32 u4NextSenstivity);
    MRESULT setAETable(eAETableID a_AEPreTableID, eAETableID a_AECapTableID);
    MRESULT setAEScene(LIB3A_AE_SCENE_T  a_eAEScene);
    MRESULT getPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable);
    MRESULT getOnePlineTable(eAETableID eTableID, strAETable &a_AEPlineTable);
    MRESULT getAETableMaxBV(const AE_PLINETABLE_T *a_aepara ,eAETableID id,MINT32 *i4MaxBV);
    MRESULT setIsoSpeed(MUINT32  a_u4ISO);
    MRESULT switchSensorModeMaxBVSensitivityDiff(MINT32 i4newSensorMode, MINT32 i4oldSensorMode, MINT32 &i4SenstivityDeltaIdx ,MINT32 &i4BVDeltaIdx );
    MRESULT setManualAETable(EAEManualPline_T a_eAEManualPline);
    MRESULT updateAEidxtoExpsetting(MUINT32 u4Index, MUINT32 u4IndexF);
    MRESULT UpdateGyroInfo();
    MRESULT UpdateLightInfo();
    MUINT32 getsensorDevID(ESensorDev_T eSensorDev);
    MUINT32 getISPRegNormalPipe(MUINT32 RegAddr);

    /* =============== ae_mgr_flow.cpp =============== */

    /* Common AE algo control */
    MRESULT prepareAlgoInitInfo (AE_CORE_INIT* pInfo);
    MRESULT prepareAlgoFrameInfo(AE_CORE_CTRL_CCU_VSYNC_INFO* pInfo);
    MRESULT prepareAlgoOnchInfo (AE_CORE_CTRL_RUN_TIME_INFO*  pInfo);
    MRESULT copyAlgoResult2Mgr  (AE_CORE_MAIN_OUT* pAlgoOutput);
    MRESULT getAEInfoForISP     (AE_INFO_T &ae_info);
    MRESULT getAlgoDebugInfo    (MVOID* exif_info, MVOID* dbg_data_info);
    MUINT32 getAECycleCnt();
    MRESULT getFDInfo(CCU_AEMeteringArea_T* pFDInfo);
    MUINT32 getAEAlgoState();

    /* CPU AE algo control */
    MRESULT prepareCPUInitInfo(AE_FLOW_CPU_INIT_INFO_T* pInfo, E_AE_ALGO_TYPE_T eCPUAlgoType);
    MRESULT initializeCPUAE(E_AE_ALGO_TYPE_T eCPUAlgoType); // CPU control interface of ae_mgr
    MRESULT prepareCPUCalculateInfo(AE_FLOW_CPU_CALCULATE_INFO_T* pInfo);
    MRESULT calculateCPUAE(); // CPU control interface of ae_mgr

    /* CCU AE init control */
    MRESULT initializeCCU(AE_FLOW_CCU_INIT_INFO_T* pInfo); // CCU control interface of ae_mgr
    MRESULT uninitializeCCU();

    /* CCU AE algo control */
    MRESULT prepareCCUInitInfo(AE_FLOW_CCU_INIT_INFO_T* pInfo);
    MRESULT initializeCCUAE(AE_FLOW_CCU_INIT_INFO_T* pInfo); // CCU control interface of ae_mgr
    MRESULT prepareCCUControlInfo(AE_FLOW_CCU_CONTROL_INFO_T* pInfo);
    MRESULT controlCCUAE(); // CCU control interface of ae_mgr
    MRESULT getCCUResult(); // CCU control interface of ae_mgr
    MRESULT resetCCUStable();
    MRESULT disableManualCCU(MBOOL bPresetCtrl);

    /* Verification control */
    MRESULT checkADBCmdCtrl();
    MRESULT verifyPerframeCtrl();
    MRESULT checkADBAlgoCfg(AE_ADB_CFG *pInfo);

    /* =============== ae_mgr_setting.cpp =============== */

    MRESULT prepareSetStartParam(AE_SETTING_INPUT_INFO_T* pinput);
    MRESULT startAESetting(E_AE_SETTING_TYPE_T eAESettingType);
    MRESULT prepareSetControlParam(AE_SETTING_CONTROL_INFO_T* pinput, MBOOL bTriggerControlbyI2C = MFALSE);
    MRESULT updateAESetting(strAERealSetting &rAERealSetting);
    MRESULT controlCPUExpoSetting(strAERealSetting &rAERealSetting, MBOOL bTriggerControlbyI2C = MFALSE);
    MBOOL   controlCCUExpoSetting(strAERealSetting &rSettingAfterConv, strAERealSetting &rSettingBeforeConv, MBOOL bCcuPresetControl = MFALSE);
    MRESULT controlCPUSensorMaxFPS(MUINT32 u4SensorMaxFPS, MBOOL bTriggerControlbyI2C = MFALSE);
    MRESULT controlCCUSensorMaxFPS(MUINT32 u4SensorMaxFPS);
    MRESULT UpdateSensorISPParams(AE_FLOW_STATE_T eAEFlowState, strAERealSetting &rAERealSetting);
    MRESULT updateAEInfo2ISP(AE_FLOW_STATE_T eAEFlowState, strAERealSetting &rSettingBeforeConv, strAERealSetting &rSettingAfterConv, MBOOL bForceSetByCPU = MFALSE);
    MBOOL   getAEManualSetting(strAERealSetting* pAERealSetting);
    MRESULT getBinSumTransSetting(strAERealSetting* pAERealSetting, MUINT32 u4BinSumRatio);
    /* dynamic high framerate */
    MRESULT setDynamicHFPS();
    MRESULT presetCtrlDynamicHFPS();
    MINT64 getFrameDuration(MUINT32 u4FrameRateX100);
    /* AE string for Real Time Viewer */
    MINT32 updateRTVString(AE_ISP_INFO_T &rAEISPInfo, char* stringBuffer);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    NSCcuIf::ICcuCtrlAe* m_pICcuAe;
    NS3A::IAeCalc* m_pIAeCalc;
    IAeFlow* m_pIAeFlowCPU;
    IAeFlow* m_pIAeFlowCCU;
    IAeSetting* m_pIAeSettingCPU;
    IAeSetting* m_pIAeSettingCCU;
    AE_CCT_CFG_T m_AeMgrCCTConfig;
    EZOOM_WINDOW_T m_eZoomWinInfo;
    CCU_AEMeteringArea_T m_eAEMeterArea;
    CCU_AEMeteringArea_T m_eAEFDArea;
    CCU_AEMeteringArea_T m_eAEFDAreaBackup4Flash;
    ESensorDev_T m_eSensorDev;
    ESensorTG_T m_eSensorTG;
    ESensorMode_T m_eSensorMode;
    EIspProfile_T m_eISPProfile;
    CAM_SCENARIO_T m_eCamScenarioMode;
    MUINT32 m_u4AENVRAMIdx;
    MINT32 m_i4SensorIdx;
    MINT32 m_i4BVvalue;
    MINT32 m_i4DeltaBV;
    MINT32 m_i4AOECompBVvalue;
    MINT32 m_i4BVvalueWOStrobe;
    MINT32 m_i4EVvalue;
    MINT32 m_i4WaitVDNum;
    MINT32 m_i4RotateDegree;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4SensorGainDelayFrames;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4SensorCaptureDelayFrame;
    MINT32 m_i4TotalCaptureDelayFrame;
    MUINT32 m_u4SmoothIspGain;
    MUINT32 m_u4AECondition;
    MUINT32 m_u4DynamicFrameCnt;
    MUINT32 m_u4AFSGG1Gain;
    MINT32  m_i4Cycle0FullDeltaIdx;
    MINT32  m_i4FrameCnt;
    MBOOL m_bAESceneChanged;
    MBOOL m_bAELock;
    MBOOL m_bAPAELock;
    MBOOL m_bAFAELock;
    MBOOL m_bAlgoAPAELock;
    MBOOL m_bAlgoAFAELock;
    MBOOL m_bEnableAE;
    MBOOL m_bVideoDynamic;
    MBOOL m_bRealISOSpeed;
    MBOOL m_bAElimitor;
    MBOOL m_bAEStable;
    MBOOL m_bAEReadyCapture;
    MBOOL m_bLockExposureSetting;
    MBOOL m_bStrobeOn;
    MBOOL m_bAEMgrDebugEnable;
    MBOOL m_bAECaptureUpdate;
    MBOOL m_bNeedCheckAEPline;
	MBOOL m_bCCTEnable;
    LIB3A_AE_SCENE_T m_eAEScene;     // change AE Pline
    LIB3A_AE_MODE_T m_eAEMode;
    mtk_camera_metadata_enum_android_control_ae_state_t m_eAEState;
    MFLOAT  m_fEVCompStep;
    MINT32  m_i4EVIndex;
    LIB3A_AE_METERING_MODE_T    m_eAEMeterMode;
    MUINT32 m_u4AEISOSpeed;   // change AE Pline
    LIB3A_AE_FLICKER_MODE_T    m_eAEFlickerMode;    // change AE Pline
    MINT32    m_i4AEMaxFps;
    MINT32    m_i4AEMinFps;
    LIB3A_AE_FLICKER_AUTO_MODE_T    m_eAEAutoFlickerMode;   // change AE Pline
    MINT32 m_i4AEAutoFlickerMode;
    NSCam::EAppMode m_eCamMode;
    LIB3A_AECAM_MODE_T m_eAECamMode;
    NSCam::EShotMode m_eShotMode;
    strAETable m_CurrentPreviewTable;
    strAETable m_CurrentCaptureTable;
    strAETable m_CurrentStrobetureTable;
    LIB3A_AE_EVCOMP_T m_eAEEVcomp;
    AE_MODE_CFG_T mPreviewMode;
    AE_MODE_CFG_T mPreviewModeBackup;
    Hal3A_HDROutputParam_T m_strHDROutputInfo;
    MBOOL m_bFrameUpdate;
    MINT32 m_i4ObjectTrackNum;
    MINT32 m_i4GammaIdx;   // next gamma idx
    MINT32 m_i4LESE_Ratio;    // LE/SE ratio
    MINT32 m_i4HdrSeg;
    MINT32 m_i4HdrTurn;
    MINT32 m_i4HdrOnOff;
    MUINT32 m_u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
    MUINT32 m_u4MaxShutter;
    MUINT32 m_u4MaxISO;
    MUINT32 m_u4HwMagicNum;
    MUINT32 m_u4RequestNum;
    MUINT32 m_u4veLTCRate;
    MUINT32 m_u4veLimitGain;

    int m_isAeMeterAreaEn;
    AE_PLINETABLE_T* m_pAEPlineTable;
    AE_PLINEMAPPINGTABLE_T *m_pAEMapPlineTable;
    NVRAM_CAMERA_3A_STRUCT* m_p3ANVRAM;
    eAETargetMODE m_eAETargetMode;
    CameraMeteringArea_T m_backupMeterArea;
    AE_INITIAL_INPUT_T m_rAEInitInput;
    CCU_strAEOutput m_rAFAEOutput;
    AE_STAT_PARAM_T m_rAEStatCfg;
    AE_CONFIG_INFO_T m_rAEConfigInfo;
    SENSOR_RES_INFO_T m_rSensorResolution[2]; // [0]: for TG1 (main/sub), [1]: for TG2(main_2)
    AE_PLINE_LIMITATION_T m_rAEPLineLimitation;
    MUINT32 m_u4PrevExposureTime;
    MUINT32 m_u4PrevSensorGain;
    MUINT32 m_u4PrevIspGain;
    LIB3A_AECAM_MODE_T m_ePrevAECamMode;
    MUINT32 m_3ALogEnable;
    MINT32 m_i4DgbLogE;

    // for Real Time Viewer
    MINT32 m_i4DgbRtvEnable;
    #define RTVMAXCOUNT 8
    MINT32 m_rtvId[RTVMAXCOUNT];
    #define PROPERTY_STRING_LENGTH 92

    MBOOL m_bAEOneShotControl;
    MBOOL bUpdateSensorAWBGain;
    SENSOR_VC_INFO_T m_rSensorVCInfo;
    MBOOL bApplyAEPlineTableByCCT;
    // for Camera 3
    MBOOL m_bMultiCap;
    MBOOL m_bCShot;
    AE_SENSOR_QUEUE_CTRL_T m_SensorQueueCtrl;
    MBOOL m_bLockBlackLevel;
    AE_SENSOR_PARAM_T m_rSensorCurrentInfo;
    MUINT32 m_u4WOFDCnt;
    MBOOL m_bFDenable;
    MUINT32 m_u4StableYValue;
    char* mEngFileName;
    MBOOL m_bzcHDRshot;
    MBOOL m_bHDRshot;
    MUINT32 m_u4VsyncCnt;
    MUINT32 m_rSensorMaxFrmRate[NSIspTuning::ESensorMode_NUM];
    mutable std::mutex m_Lock;
    mutable std::mutex m_LockControlCCU;
    MINT32   m_i4AETgValidBlockWidth;  // AE TG valid block width
    MINT32   m_i4AETgValidBlockHeight; // AE TG valid block height
    // AeMgr p-line
    strAETable *m_pPreviewTableCurrent;
    strAETable *m_pPreviewTableNew;
    strAETable *m_pCaptureTable;
    strAETable *m_pStrobeTable;
    // finer EV index & p-line
    strFinerEvPline m_PreviewTableF;      // current finer EV table for preview
    strFinerEvPline m_CaptureTableF;      // current finer EV table for capture
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;  //boundary of AE table
    MUINT32 m_u4Index;       // current AE index
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexF;
    MUINT32 m_u4FinerEVIdxBase;
    MBOOL   m_bFirstLaunchCam; // to reset pline index by NVRAM param
    // current AE index
    MINT32  m_i4PreviewMaxBV;
    MINT32  m_i4VideoMaxBV;
    MINT32  m_i4Video1MaxBV;
    MINT32  m_i4Video2MaxBV;
    MINT32  m_i4ZSDMaxBV;
    MINT32  m_i4DeltaBVIdx;
    MINT32  m_i4DeltaSensitivityIdx;
    EAEManualPline_T m_eAEManualPline;
    EAEManualPline_T m_eAEManualCapPline;
    MBOOL   m_bStereoManualPline;
    MINT32  m_i4StereoDenoiserto[2];
    MUINT32 m_u4StereoDenoiseMode;
    MINT32  m_i4AEUnderExpDeltaBVIdx;
    MUINT32 m_u4EISExpRatio;
    // AeMgr HBIN size
    MUINT32 m_u4Hbinwidth;
    MUINT32 m_u4Hbinheight;
    // AAO buf info
    MVOID *m_pAAOStatBuf;
    CCU_AAO_PROC_INFO_T m_strAAOProcInfo;
    // VHDR EM ratio
    MUINT32 m_u4EMVHDRratio;
    MUINT32 m_u4CurVHDRratio;
    MBOOL   m_bVHDRChangeFlag;
    // Slow Motion Buffer mode
    MBOOL   m_bEnSWBuffMode;
    MINT32  m_i4SMSubSamples;
    MUINT32 m_u4HFSMSkipFrame;
    // Test
    MUINT32 m_u4HDRcountforrevert;
    // AE HDR mode
    MUINT32 m_u4AEHDRMode;
	MUINT32 m_u4MVHDRRatio_x100;
	MVOID *m_pMVHDR3ExpoStatBuf;
    // CCU control
    MBOOL  m_bCCUAEFlag;
    NS3Av3::Hal3AFlowCtrl* m_pCCUresultCB;
    MBOOL m_bIsCCUStart;
    MBOOL m_bCCUIsSensorSupported;
    //Gyro
    MINT32  m_i4AcceInfo[3];
    MINT32  m_i4GyroInfo[3];
    MINT32  m_i4LightInfo;

    //
    MBOOL m_bAFTouchROISet;
    MINT32 m_i4ZoomRatio;
    MUINT32 m_u4CWVY;
    MBOOL m_bAETrueLaunch;
    MBOOL m_bPlineMappingChange;
    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;
    MBOOL m_bGryoVd;
    MBOOL m_bAcceVd;
    MBOOL m_bLightVd;
    IdxMgr* m_pIdxMgr;
    ISP_NVRAM_REGISTER_STRUCT* ISP_NVRAM_Ptr;
    //CCU manual
    MUINT32 m_u4CCUManualShutter;
    MUINT32 m_u4CCUManualAfeGain;
    MUINT32 m_u4CCUManualIspGain;
    MBOOL m_u4CCUManualEnable;
    MBOOL m_bIsCCUAEInit;
    MBOOL m_bIsFrameRateLocked;
    // Debounce counter
    MUINT32 m_u4ISOIdx1StableCount;
    MUINT32 m_u4ISOIdx2StableCount;
    MUINT32 m_u4Idx1StableThres;
    MUINT32 m_u4Idx2StableThres;
    MUINT32 m_u4ISOIdx1Value;
    MUINT32 m_u4ISOIdx2Value;
    MUINT32 m_u4ISO1IdxStatus;
    MUINT32 m_u4ISO2IdxStatus;
    MBOOL m_bIsCPUNeedDebugInfo;
    //MBOOL mbIsPlineChange;
    MUINT32 m_u4CCUManShutter;
    MUINT32 m_u4CCUManISO;
    MBOOL m_bUpdateAELockIdx;
    MBOOL m_bAlgoResultUpdate;

    /* new AE algo output & setting */
    AE_INFO_T        m_rAEInfoPreview;    // preview AE info
    AE_INFO_T        m_rAEInfoCapture;    // capture AE info
    strAERealSetting m_rAEAlgoOutput;     // AE algo preview output setting, for backup setting
    strAERealSetting m_rAESettingPreview; // AE preview setting
    strAERealSetting m_rAESettingCapture; // AE capture setting
    strAERealSetting m_rAESettingAE2AF;   // AE AE2AF setting
    strAERealSetting m_rAESettingBackup;  // flash backup setting

    /* ADB cmd */
    MBOOL m_bAdbAELock;
    MBOOL m_bAdbAEPreviewUpdate;
    strAERealSetting m_rADBAESetting;     // AE ADB setting
    MUINT32 m_u4IndexTestCnt;
    MUINT32 m_u4AdbISOspeed;

    E_AE_ALGO_TYPE_T    m_eAlgoType;
    E_AE_SETTING_TYPE_T m_eSettingType;
    CCU_AE_STAT m_rCusAEStat;
    AE_CUST_PARAM_T m_rAECustPARAM;

    /* for normal AE setting */
    MUINT32 m_u4AESettingEXP[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AESettingAFE[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AESettingISP[MAX_AE_SETTING_NUM];
    MUINT32 m_u4ExposureMode; // 0: exposure time, 1: exposure line
    /* for VHDR AE setting */
    MUINT32 m_u4AEVHDRSettingEXP_LE[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AEVHDRSettingEXP_ME[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AEVHDRSettingEXP_SE[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AEVHDRSettingAFE_LE[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AEVHDRSettingAFE_ME[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AEVHDRSettingAFE_SE[MAX_AE_SETTING_NUM];
    MUINT32 m_u4AEVHDRSettingISP   [MAX_AE_SETTING_NUM];

    MUINT32 m_u4UpdateFrameRate_x10;
    MUINT32 m_u4StableCnt;
    MUINT32 m_u4AEMgrStateCPU; // updated by doXXAE
    MUINT32 m_u4AEMgrStateCCU; // updated by preset
    MBOOL m_bForceResetCCUStable; // to nofify CCU: force reset AE stable as false (to overwrite AE result-stable_flag of next frame)

    /* Update AE info to framework */
    AEResultConfig_T m_sAEResultConfig;
    AE_PERFRAME_INFO_T m_AEPerFrameInfo;

    /* dynamic high framerate */
    MUINT32 m_u4DynamicHFPS;

    MUINT32 m_u4BinSumRatio; // sensor binning sum ratio
    MBOOL m_bSkipCalAE;
    MBOOL m_bFDSkipCalAE;
    MBOOL m_bShutterISOPriorityMode;
    MBOOL m_bManualByShutterISOPriorityMode;
    CCU_AE_PRIORITY_INFO m_AePriorityParam;
    MBOOL m_bSuperNightShotMode;
    CCU_AE_CUST_Super_Night_Param_T m_AeCustParam;
    MBOOL m_bInitAE;

	ISP_AUTO_LTM_TUNING_T m_LTMParam;
	MINT32 m_i4ISP6LTMidx;
    MBOOL m_bMainFlashON;
};

};  //  namespace NS3Av3

extern MUINT32 Log2x1000[LumLog2x1000_TABLE_SIZE]; // temp put in here, need to move to AE calculation library in P80

#endif // _AE_MGR_H_

