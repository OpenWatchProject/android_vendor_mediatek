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

#include <utils/Mutex.h>
#include <isp_tuning.h>
#include <ae_mgr_if.h>
//#include <ae_algo_if.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_ivhdr.h"
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <ae_algo_if.h>
#include <IThread.h>
#include <IAeTest.h>
#include <AeFlow/IAeFlow.h>
#include <AeSetting/IAeSetting.h>
#include <Hal3AFlowCtrl.h>

//CCU
#define AE_MGR_INCLUDING_CCU
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "iccu_ctrl_ae.h"

// Index manager
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif


#define ISP_GAIN_DELAY_OFFSET 2
#define INIT_WOF_WAIT_CYCLE 8
#define PV_CWVY_FLOOR 30
#define PV_CWVY_CEILING 200
#define INIT_WAIT_FD_SKIP_AE 1

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

using namespace android;
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
public:
    //  Copy constructor is disallowed.
    AeMgr(AeMgr const&);
    //  Copy-assignment operator is disallowed.
    AeMgr& operator=(AeMgr const&);

public:  ////
    AeMgr(MINT32 const i4SensorIndex);
    ~AeMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AeMgr& getInstance(MINT32 const i4SensorDev);
    MRESULT cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam);
    MRESULT Start();
    MRESULT uninit();
    MRESULT Stop();
    void setAeMeterAreaEn(int en);
    void setExp(int exp);
    void setAfe(int afe);
    void setIsp(int isp);
    void setPfPara(int exp, int afe, int isp);
    void setRestore(int frm);
    MRESULT setAEMeteringArea(CameraMeteringArea_T const *sNewAEMeteringArea);
    MRESULT setAEEVCompIndex(MINT32 i4NewEVIndex, MFLOAT fStep, MBOOL is_preset = MFALSE);
    MRESULT setAEMeteringMode(MUINT32 u4NewAEMeteringMode);
    MINT32 getAEMeterMode() const;
    MRESULT setAEISOSpeed(MUINT32 i4NewAEISOSpeed);
    MINT32 getAEISOSpeedMode() const;
    MRESULT setAEMinMaxFrameRate(MINT32 i4NewAEMinFps, MINT32 i4NewAEMaxFps);
    MRESULT setAEFlickerMode(MUINT32 u4NewAEFLKMode);
    MRESULT setAEAutoFlickerMode(MUINT32 u4NewAEAutoFLKMode);
    MRESULT setFlickerFrameRateActive(MBOOL a_bFlickerFPSAvtive);
    MRESULT setAESMBuffermode(MBOOL bSMBuffmode, MINT32 i4subframeCnt);
    MRESULT enableAEManualPline(EAEManualPline_T eAEManualPline, MBOOL bEnable);
    MRESULT enableAEManualCapPline(EAEManualPline_T eAEManualCapPline, MBOOL bEnable);
    MRESULT setAECamMode(MUINT32 u4NewAECamMode);
    MRESULT setAEShotMode(MUINT32 u4NewAEShotMode);
    MRESULT setCamScenarioMode(MUINT32 u4NewCamScenarioMode, MBOOL bInit = MFALSE);
    MRESULT setNVRAMIndex(MUINT32 a_eNVRAMIndex, MBOOL bInit = MFALSE);
    MRESULT setAAOMode(MUINT32 u4NewAEAAOMode);
    MRESULT setAEHDRMode(MUINT32 u4NewAEHDRMode);
    MINT32 getAEHDROnOff();
    MRESULT setAELimiterMode(MBOOL bAELimter);
    MRESULT setSceneMode(MUINT32 u4NewScene);
    MINT32 getAEScene() const;
    MRESULT setAEMode(MUINT32 u4NewAEmode);
    MINT32 getAEMode() const;
    MINT32 getAEState() const;
    MBOOL setSensorMode(MINT32 i4NewSensorMode, MUINT32 width, MUINT32 height);
    MBOOL updateAEScenarioMode(EIspProfile_T eIspProfile);
    MINT32 getSensorMode() const;
    MRESULT setAPAELock(MBOOL bAPAELock);
    MRESULT setAFAELock(MBOOL bAFAELock);
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT enableAE();
    MRESULT disableAE();
    MRESULT setAAOProcInfo(MVOID *pPSOStatBuf, AAO_PROC_INFO_T const *sNewAAOProcInfo);
    MRESULT setMVHDR3ExpoProcInfo(MVOID *pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100);
    MRESULT doCapFlare(MVOID *pAEStatBuf, MBOOL bIsStrobe);
    MRESULT doAFAEAAO(MVOID *pAEStatBuf, MUINT32 u4AAOUpdate);
    MRESULT doAFAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule,MBOOL bIsFlashCapture);
    MRESULT doPreCapAE(MINT64 i8TimeStamp, MBOOL bIsStrobeFired, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule);
    MRESULT doCapAE();
    MRESULT doBackAEInfo(MBOOL bTorchMode=MFALSE);
    MRESULT doRestoreAEInfo(MBOOL bRestorePrvOnly,MBOOL bTorchMode=MFALSE);
    MRESULT doPvAE(MINT64 i8TimeStamp, MVOID *pAEStatBuf, MINT32 i4ActiveAEItem, MUINT32 u4AAOUpdate, MBOOL bAAASchedule,MUINT8 u1AeMode);
    MRESULT doPvAE_TG_INT(MVOID *pAEStatBuf);
    MRESULT getDebugInfo(AE_DEBUG_INFO_T &rAEDebugInfo, AE_PLINE_DEBUG_INFO_T &rAEPlineDebugInfo);
    MINT32 getLVvalue(MBOOL isStrobeOn);
    MINT32 getAOECompLVvalue(MBOOL isStrobeOn);
    MINT32 getBVvalue();
    MINT32 getDeltaBV();
    MINT32 getCaptureLVvalue();
    MUINT32 getAEMaxMeterAreaNum();
    MINT32 getEVCompensateIndex();
    MRESULT getCurrentPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable, strAFPlineInfo &a_StrobeAEPlineInfo);
    MRESULT getCurrentPlineTableF(strFinerEvPline &a_PrvAEPlineTable, strFinerEvPline &a_CapAEPlineTable);
    MRESULT getAECapPlineTable(MINT32 *i4CapIndex, strAETable &a_AEPlineTable);
    MRESULT getSensorDeviceInfo(AE_DEVICES_INFO_T &a_rDeviceInfo);
    MBOOL IsDoAEInPreAF();
    MBOOL IsAEStable();
    MBOOL IsAELock();
    MBOOL IsAPAELock();
    MBOOL IsStrobeBVTrigger();
    MRESULT getPreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MBOOL bReCalcul = MFALSE);
    MRESULT IsGetManualExpState();
    MRESULT updateSensorListenerParams(MINT32 *i4SensorInfo);
    MRESULT updatePreviewParams(AE_MODE_CFG_T &a_rPreviewInfo, MINT32 i4AEidxNext, MINT32 i4AEidxNextF);
    MRESULT updateAEUnderExpdeltaBVIdx(MINT32 i4AEdeltaBV);
    MRESULT updateISPNvramOBCTable(MVOID* pISPNvramOBCTable, MINT32 i4Tableidx);
    MRESULT updateISPNvramOBCIndex(MVOID* rMappingInfo,MINT32 i4Tableidx);
    MRESULT getExposureInfo(ExpSettingParam_T &strHDRInputSetting);
    MRESULT getCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo);
    MRESULT updateCaptureParams(AE_MODE_CFG_T &a_rCaptureInfo, MBOOL bRemosaicEn = MFALSE);
    MRESULT getAEMeteringYvalue(AEMeterArea_T rWinSize, MUINT8 *iYvalue);
    MRESULT getAEMeteringBlockAreaValue(AEMeterArea_T rWinSize, MUINT8 *uYvalue, MUINT16 *u2YCnt);
    MRESULT getRTParams(FrameOutputParam_T &a_strFrameInfo);
    MRESULT setFDenable(MBOOL bFDenable);
    MRESULT getAEFDMeteringAreaInfo(android::Vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight);
    MRESULT setFDInfo(MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight);
    MRESULT setOTInfo(MVOID* a_sOT);
    MRESULT setStrobeMode(MBOOL bIsStrobeOn);
    MRESULT setAERotateDegree(MINT32 i4RotateDegree);
    MBOOL getAECondition(MUINT32 i4AECondition);
    MRESULT getLCEPlineInfo(LCEInfo_T &a_rLCEInfo);
    MINT16 getAEFaceDiffIndex();
    MRESULT updateSensorDelayInfo(MINT32* i4SutterDelay, MINT32* i4SensorGainDelay, MINT32* i4IspGainDelay);
    MRESULT getBrightnessValue(MBOOL * bFrameUpdate, MINT32* i4Yvalue);
    MRESULT UpdateSensorISPParams(AE_STATE_T eNewAEState);
    MBOOL getAENvramData(AE_NVRAM_T &rAENVRAM);
    MRESULT getNvramData(MINT32 i4SensorDev, MBOOL isForce = MFALSE);
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
    MRESULT UpdateSensorParams(AE_SENSOR_PARAM_T strSensorParams);
    MRESULT bBlackLevelLock(MBOOL bLockBlackLevel);
    MRESULT getSensorParams(AE_SENSOR_PARAM_T &a_rSensorInfo);
    MRESULT getSensorParamsCCU(AE_SENSOR_PARAM_T &a_rSensorInfo);
    MRESULT getAEConfigParams(MBOOL &bHDRen, MBOOL &bOverCnten, MBOOL &bTSFen);
    MINT64 getSensorRollingShutter() const;
    MRESULT setAEState2Converge();
    MINT32 get3ACaptureDelayFrame();
    MRESULT IsMultiCapture(MBOOL bMultiCap);
    MRESULT IsAEContinueShot(MBOOL bCShot);
    MRESULT enableEISRecording(MBOOL bEISRecording);
    MRESULT enableAISManualPline(MUINT32 u4AISPline);
    MRESULT enableMFHRManualPline(MBOOL bMFHRPline);
    MRESULT enableBMDNManualPline(MBOOL bBMDNPline);
    MRESULT updateStereoDenoiseRatio(MINT32* i4StereoDenoiserto);
    MRESULT enableStereoDenoiseRatio(MUINT32 u4enableStereoDenoise);
    MRESULT enableFlareInManualControl(MBOOL bIsFlareInManual);
    MRESULT enableAEStereoManualPline(MBOOL bstereomode);
    MRESULT setzCHDRShot(MBOOL bHDRShot);
    MRESULT enableHDRShot(MBOOL bHDRShot);
    MRESULT updateCaptureShutterValue();
    MRESULT prepareCapParam();
    MRESULT setSensorDirectly(CaptureParam_T &a_rCaptureInfo, MBOOL bDirectlySetting);
    MBOOL isLVChangeTooMuch();
    MRESULT switchCapureDiffEVState(MINT8 iDiffEV, strAEOutput &aeoutput);
    MBOOL SaveAEMgrInfo(const char * fname);
    MBOOL setTGInfo(MINT32 const i4TGInfo);
    MINT32 getAAOLineByteSize() const;
    MRESULT getAAOSize(MUINT32 &u4BlockNumW, MUINT32 &u4BlockNumH);
    MRESULT getAEdelayInfo(MINT32* i4ExpDelay, MINT32* i4AEGainDelay, MINT32* i4IspGainDelay);
    MRESULT getTgIntAEInfo(MBOOL &bTgIntAEEn, MFLOAT &fTgIntAERatio);
    MRESULT getPerframeAEFlag(MBOOL &bPerframeAE);
    MRESULT getAFAEOutput(strAEOutput &a_rAEOutput);
    MINT32 getCaptureMaxFPS();
    // EM feature APIs.
    MINT32 getNVRAMParam(MVOID *a_pAENVRAM, MUINT32 *a_pOutLen);
    MRESULT updateAEBV(MVOID *pAEStatBuf);
    MRESULT getAEInitExpoSetting(AEInitExpoSetting_T &a_rAEInitExpoSetting);
    MRESULT getAE2AFInfo(AEMeterArea_T rAeWinSize, AE2AFInfo_T &rAEInfo);
    MRESULT reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg);
    MRESULT switchExpSettingByShutterISOpriority(AE_EXP_SETTING_T &strExpInput, AE_EXP_SETTING_T &strExpOutput);
    MRESULT getISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status);
    MRESULT resetGetISOThresStatus(MINT32 *a_ISOIdx1Status, MINT32 *a_ISOIdx2Status);
    MRESULT resetMvhdrRatio(MBOOL bReset);
    MRESULT prepareCapParams(MBOOL bRemosaicEn=MFALSE,MBOOL bIsEVChange=MFALSE);
    MRESULT UpdateCustParams(AE_Cust_Param_T *a_rCustomParam);
    MRESULT EnableSuperNightShot(MINT32 i4SuperNightShot);

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
    MINT32 CCTOPAEGetIspOB(MUINT32 *a_pIspOB, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspOB(MUINT32 a_IspOB);
    MINT32 CCTOPAEGetIspRAWGain(MUINT32 *a_pIspRawGain, MUINT32 *a_pOutLen);
    MINT32 CCTOPAESetIspRAWGain(MUINT32 a_IspRAWGain);
    MINT32 CCTOPAESetSensorExpTime(MUINT32 a_ExpTime);
    MINT32 CCTOPAESetSensorExpLine(MUINT32 a_ExpLine) const;
    MINT32 CCTOPAESetSensorGain(MUINT32 a_SensorGain) const;
    MINT32 CCTOPAESetCaptureMode(MUINT32 a_CaptureMode);
    MINT32 CCTOSetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOGetCaptureParams(MVOID *a_pAEExpParam);
    MINT32 CCTOPAEGetFlareOffset(MUINT32 a_FlareThres, MUINT32 *a_pAEFlareOffset, MUINT32 *a_pOutLen);
    MINT32 CCTOPSetAETargetValue(MUINT32 u4AETargetValue);
    MINT32 CCTOPAEApplyPlineNVRAM(MVOID *a_pAEPlineNVRAM);
    MINT32 CCTOPAEGetPlineNVRAM(MVOID *a_pAEPlineNVRAM,MUINT32 *a_pOutLen);
    MINT32 CCTOPAESavePlineNVRAM();
    MBOOL getCCUresultCBActive(MVOID* pfCB);
    MRESULT setDigZoomRatio(MINT32 i4ZoomRatio);
    MRESULT setFrameRateLock(MBOOL enable);
    MRESULT setCCUOnOff(MBOOL enable);
    MRESULT PresetControlCCU(MBOOL SkipCCUAE);
    MRESULT CCUManualControl(MUINT32 u4ExpTime,MUINT32 u4AfeGain,MUINT32 u4IspGain,MBOOL EnableManual);
    MBOOL IsNeedPresetControlCCU();
    MRESULT IsCCUAEInit(MBOOL bInit);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MRESULT getSensorResolution();

    MRESULT AEInit(Param_T &rParam);
    MRESULT copyAEInfo2mgr(AE_MODE_CFG_T *sAEOutputInfo, strAEOutput *sAEInfo,MBOOL binit=MFALSE);
    MRESULT modifyAEInfoby3DNR(MUINT32& u4Exp, MUINT32& u4Afe, MUINT32& u4Isp);
    MRESULT PreviewAEInit(MINT32 m_i4SensorIdx, Param_T &rParam);
    MRESULT updatePreviewParamsByHFSM(AE_MODE_CFG_T *sAEInfo);
    MRESULT updateEMISOCaptureParams(AE_MODE_CFG_T &inputparam);
    MRESULT ModifyCaptureParamsBySensorMode(MINT32 i4newSensorMode, MINT32 i4oldSensorMode);
    MRESULT setAELock();
    MRESULT ClrCurrentInfo_AELock();
    MRESULT GetCurrentInfo_AELock();
    MRESULT UpdateFlare2ISP();
    MRESULT updateAEInfo2ISP(AE_STATE_T eNewAEState, MUINT32 u4ISPGain);
    MRESULT updateAEScenarioPline(MINT32 i4SensorMode);
    // AeMgr p-line
    MRESULT getAEPLineMappingID(LIB3A_AE_SCENE_T  a_eAESceneID, MINT32 i4SensorMode, eAETableID *pAEPrePLineID, eAETableID *pAECapPLineID);
    MRESULT searchAETable(AE_PLINETABLE_T *a_aepara ,eAETableID id,strAETable** a_ppPreAETable);
    MRESULT searchPreviewIndexLimit();
    MRESULT setAEMinMaxFrameRatePlineIdx(MINT32 a_eAEMinFrameRate, MINT32 a_eAEMaxFrameRate);
    MRESULT setAEMaxFrameRateGainIndexRange();
    MRESULT getSenstivityDeltaIndex(MUINT32 u4NextSenstivity);
    MRESULT setAETable(eAETableID a_AEPreTableID, eAETableID a_AECapTableID);
    MRESULT setAEScene(LIB3A_AE_SCENE_T  a_eAEScene);
    MRESULT getPlineTable(strAETable &a_PrvAEPlineTable, strAETable &a_CapAEPlineTable, strAETable &a_StrobeAEPlineTable);
    MRESULT getOnePlineTable(eAETableID eTableID, strAETable &a_AEPlineTable);
    MRESULT getAETableMaxBV(const AE_PLINETABLE_T *a_aepara ,eAETableID id,MINT32 *i4MaxBV);
    MRESULT setIsoSpeed(MUINT32  a_u4ISO);
    MRESULT setAEFlickerModePlineIdx(LIB3A_AE_FLICKER_MODE_T a_eAEFlickerMode);
    MRESULT setAEFlickerAutoModePlineIdx(LIB3A_AE_FLICKER_AUTO_MODE_T a_eAEFlickerAutoMode);
    MRESULT switchSensorModeMaxBVSensitivityDiff(MINT32 i4newSensorMode, MINT32 i4oldSensorMode, MINT32 &i4SenstivityDeltaIdx ,MINT32 &i4BVDeltaIdx );
    MRESULT setManualAETable(EAEManualPline_T a_eAEManualPline);
    MRESULT updateAEidxtoExpsetting();
    MRESULT UpdateGyroInfo();
    MUINT32 getsensorDevID(ESensorDev_T eSensorDev);

    //----ae_mgr_flow.cpp----//
    // CCU refactoring
    MRESULT prepareCCUStartParam(AE_FLOW_INPUT_INFO_T* pinput);
    MRESULT prepareCCUPvParam(AE_FLOW_CONTROL_INFO_T* pinput);
    MRESULT copyCCUAEInfo2mgr(AE_CORE_MAIN_OUT* poutput);
    MRESULT setAEInfo2CCU(MUINT32 exp, MUINT32 afe, MUINT32 isp, MUINT32 iso, MUINT32 effective_idx, MUINT32 idx, MUINT32 delata_idx);
    MRESULT ShouldCCUStart();
    // CPU refactoring
    MRESULT prepareCPUStartParam(AE_FLOW_CPUSTART_INFO_T* pinput);
    MRESULT prepareSchedulerParam(AE_FLOW_SCHEDULE_INFO_T* pinput, E_AE_FLOW_CPU_T etype, MBOOL bAAASchedule, MINT32 i4ActiveAEItem);
    MRESULT prepareCalculateParam(AE_FLOW_CALCULATE_INFO_T* pinput, E_AE_FLOW_CPU_T etype, MVOID* pStatistic, MINT64 i8timestamp);
    MRESULT copyScheduleInfo2mgr(AE_FLOW_SCHEDULE_OUTPUT_T* poutput, MBOOL& bCalc, MBOOL& bApply);
    //----ae_mgr_setting.cpp----//
    // CPU refactoring
    MRESULT prepareSetStartParam(AE_SETTING_INPUT_INFO_T* pinput);
    MRESULT prepareSetControlParam(AE_SETTING_CONTROL_INFO_T* pinput);
    MUINT32 getISPRegNormalPipe(MUINT32 RegAddr);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    NSCcuIf::ICcuCtrlAe* m_pICcuAe;
    NS3A::IAeAlgo* m_pIAeAlgo;
    IAeTest* m_pIAeTest;
    IAeFlow* m_pIAeFlowCPU;
    IAeFlow* m_pIAeFlowCCU;
    IAeSetting* m_pIAeSettingCPU;
    IAeSetting* m_pIAeSettingCCU;
    AE_CCT_CFG_T m_AeMgrCCTConfig;
    EZOOM_WINDOW_T m_eZoomWinInfo;
    AEMeteringArea_T m_eAEMeterArea;
    AEMeteringArea_T m_eAEFDArea;
    ESensorDev_T m_eSensorDev;
    ESensorTG_T m_eSensorTG;
    ESensorMode_T m_eSensorMode;
    EIspProfile_T m_eISPProfile;
    CAM_SCENARIO_T m_eCamScenarioMode;
    MUINT32 m_u4AENVRAMIdx;
    MINT32 m_i4SensorIdx;
    MINT32 m_BVvalue;
    MINT32 m_i4DeltaBV;
    MINT32 m_AOECompBVvalue;
    MINT32 m_BVvalueWOStrobe;
    MINT32 m_i4EVvalue;
    MINT32 m_i4WaitVDNum;
    MINT32 m_i4RotateDegree;
    MINT32 m_i4TimeOutCnt;
    MINT32 m_i4ShutterDelayFrames;
    MINT32 m_i4SensorGainDelayFrames;
    MINT32 m_i4SensorGainDelayFramesWOShutter;
    MINT32 m_i4IspGainDelayFrames;
    MINT32 m_i4SensorCaptureDelayFrame;
    MINT32 m_i4TotalCaptureDelayFrame;
    MINT32 m_i4AEidxCurrent;  // current AE idx
    MINT32 m_i4AEidxNext;   // next AE idx
    MINT32 m_i4AEidxNextF;   // next AE Finner idx
    MINT16 m_i2AEFaceDiffIndex;
    MUINT32 m_u4PreExposureTime;
    MUINT32 m_u4PreSensorGain;
    MUINT32 m_u4PreIspGain;
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
    MBOOL m_bEnableAE;
    MBOOL m_bVideoDynamic;
    MBOOL m_bRealISOSpeed;
    MBOOL m_bAElimitor;
    MBOOL m_bAEStable;
    MBOOL m_bAEReadyCapture;
    MBOOL m_bLockExposureSetting;
    MBOOL m_bStrobeOn;
    MBOOL m_bAEMgrDebugEnable;
    MBOOL m_bRestoreAE;
    MBOOL m_bAECaptureUpdate;
    MBOOL m_bOtherIPRestoreAE;
    LIB3A_AE_SCENE_T m_eAEScene;     // change AE Pline
    LIB3A_AE_MODE_T m_eAEMode;
    mtk_camera_metadata_enum_android_control_ae_state_t m_ePreAEState;
    mtk_camera_metadata_enum_android_control_ae_state_t m_eAEState;
    MFLOAT  m_fEVCompStep;
    MINT32  m_i4EVIndex; // for usage of "set"
    MINT32  m_i4EVIndexPreset;  // for usage of "preset"
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
    LIB3A_AE_EVCOMP_T m_eAEEVcomp;  // for usage of "set"
    LIB3A_AE_EVCOMP_T m_eAEEVcompPreset;  // for usafe of "preset"
    AE_MODE_CFG_T mPreviewMode;
    AE_MODE_CFG_T mPreviewModeBackup;
    AE_MODE_CFG_T mPreviewModeBackupTorch;
    AE_MODE_CFG_T mCaptureMode;
    Hal3A_HDROutputParam_T m_strHDROutputInfo;
    AE_STATE_T m_AEState;
    MBOOL m_bIsAutoFlare;
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
    MUINT32 m_u4StableCnt;
    MUINT32 m_u4HwMagicNum;
    MUINT32 m_u4veLTCRate;
    MUINT32 m_u4veLimitGain;
    IVHDRExpSettingOutputParam_T m_strIVHDROutputSetting;
    IVHDRExpSettingOutputParam_T m_strIVHDRCaptureSetting;

    int m_isAeMeterAreaEn;
    AE_PLINETABLE_T* m_pAEPlineTable;
    AE_PLINEMAPPINGTABLE_T *m_pAEMapPlineTable;
    NVRAM_CAMERA_3A_STRUCT* m_p3ANVRAM;
    ISP_NVRAM_ISO_INTERVAL_T m_rISPNvramISOInterval;
    CCU_ISP_NVRAM_OBC_T m_rISPNvramOBC_Table[4];
    ISP_NVRAM_ISO_INTERVAL_T m_rISP5NvramISOInterval;
    CCU_ISP_NVRAM_OBC_T m_rISP5NvramOBC_Table[20];
    eAETargetMODE m_eAETargetMode;
    eAETargetMODE m_eAETargetMode2; // customize for 4cell mvhdr preview <-> normal preview
    CameraMeteringArea_T m_backupMeterArea;
    AE_INITIAL_INPUT_T m_rAEInitInput;
    AE_OUTPUT_T m_rAEOutput;
    strAEOutput m_rAFAEOutput;
    AE_STAT_PARAM_T m_rAEStatCfg;
    SENSOR_RES_INFO_T m_rSensorResolution[2]; // [0]: for TG1 (main/sub), [1]: for TG2(main_2)
    AE_PLINE_LIMITATION_T m_rAEPLineLimitation;
    MUINT32 m_u4PrevExposureTime;
    MUINT32 m_u4PrevSensorGain;
    MUINT32 m_u4PrevIspGain;
    LIB3A_AECAM_MODE_T m_ePrevAECamMode;
    MUINT32 m_3ALogEnable;
    MINT32 m_i4DgbLogE;
    MBOOL m_bAEOneShotControl;
    MBOOL bUpdateSensorAWBGain;
    SENSOR_VC_INFO_T m_rSensorVCInfo;
    MBOOL bApplyAEPlineTableByCCT;
    MUINT32 m_u4UpdateShutterValue;
    MUINT32 m_u4UpdateGainValue;
    MUINT32 m_u4UpdateGainValue2;
    MUINT32 m_u4UpdateGainValue3;
    MFLOAT m_u4UpdateFrameRate_x10;
    // for Camera 3
    MBOOL m_bTouchAEAreaChage;
    MBOOL m_bInitWaitFaceSkipAE;
    MUINT8 m_u4InitWaitFaceCount;
    MUINT8 m_u4InitWaitCount;
    MBOOL m_bMultiCap;
    MBOOL m_bCShot;
    AE_SENSOR_QUEUE_CTRL_T m_SensorQueueCtrl;
    MBOOL m_bLockBlackLevel;
    AE_SENSOR_PARAM_T m_rSensorCurrentInfo;
    AE_SENSOR_PARAM_T m_rSensorCurrentInfoCCU;
    AE_SENSOR_PARAM_T m_rSensorCurrentInfo_aelock;
    AE_SENSOR_PARAM_T m_rSensorCurrentInfoCCU_aelock;
    MUINT32 m_u4WOFDCnt;
    MBOOL m_bFDenable;
    MBOOL m_bFaceAEAreaChage;
    MUINT32 m_u4StableYValue;
    char* mEngFileName;
    MBOOL m_bzcHDRshot;
    MBOOL m_bHDRshot;
    MBOOL m_bResetZHDRMode;
    MUINT32 m_u4VsyncCnt;
    MUINT32 m_rSensorMaxFrmRate[NSIspTuning::ESensorMode_NUM];
    mutable android::Mutex m_Lock;
    MUINT32 m_u4LEHDRshutter;
    MUINT32 m_u4SEHDRshutter;
    MUINT32 m_u4MEHDRshutter;
    MBOOL m_bIsFlareInManual;
    //Perframe update AE
    MUINT8 m_uEvQueueSize;
    MUINT32 m_u4PreviousSensorgain;
    MUINT32 m_u4PreviousISPgain;
    MBOOL m_bPerframeAEFlag;
    // TG interrupt
    MBOOL    m_bTgIntAEEn;
    MINT32   m_i4AETgValidBlockWidth;  // AE TG valid block width
    MINT32   m_i4AETgValidBlockHeight; // AE TG valid block height
    // AeMgr p-line
    strAETable *m_pPreviewTableCurrent;
    strAETable *m_pPreviewTableNew;
    strAETable *m_pCaptureTable;
    strAETable *m_pStrobeTable;
    // finer EV p-line
    strFinerEvPline m_PreviewTableF;      // current finer EV table for preview
    strFinerEvPline m_CaptureTableF;      // current finer EV table for capture
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;  //boundary of AE table
    MUINT32 m_u4Index;       // current AE index
    // finer EV index
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexF;
    MUINT32 m_u4Prvflare;
    MUINT32 m_u4FinerEVIdxBase;
    // current AE index
    MUINT32 m_u4LCEGainStartIdx;
    MUINT32 m_u4LCEGainEndIdx;
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
    // AeMgr AAO Separation buffer
    MUINT32 m_u4BlockNumX;
    MUINT32 m_u4BlockNumY;
    MBOOL m_bEnableHDRYConfig;
    MBOOL m_bEnableOverCntConfig;
    MBOOL m_bEnableTSFConfig;
    MBOOL m_bDisablePixelHistConfig;
    MUINT32 m_u4AEOverExpCntThs;
    MUINT32 m_u4AEOverExpCntShift;
    MUINT32 m_u4AEYCoefR;
    MUINT32 m_u4AEYCoefG;
    MUINT32 m_u4AEYCoefB;
    // PSO info
    MVOID *m_pPSOStatBuf;
    AAO_PROC_INFO_T m_strAAOProcInfo;
    // VHDR EM ratio
    MUINT32 m_u4VHDRratio;
    MBOOL   m_bVHDRChangeFlag;
    // Slow Motion Buffer mode
    MBOOL   m_bEnSWBuffMode;
    MINT32  m_i4SMSubSamples;
    MUINT32 m_u4HFSMSkipFrame;
    // Test
    MUINT32 m_u4HDRcountforrevert;
    // AAO Mode
    MUINT32 m_i4AAOmode;
    // AE HDR mode
    MUINT32 m_u4AEHDRMode;
    // CCU control
    MBOOL  m_bCCUAEFlag;
    MINT32 m_i4OBCTableidx;
    MINT32 m_i4ISP5OBCITableidx;
    NS3Av3::Hal3AFlowCtrl* m_pCCUresultCB;
    //Gyro
    MINT32  m_i4AcceInfo[3];
    MINT32  m_i4GyroInfo[3];
    //
    MBOOL m_bAFTouchROISet;
    MINT32 m_i4ZoomRatio;
    MUINT32 m_u4CCUResultCwvy;
    MUINT32 m_u4CCUResultCwvyStable;
    MBOOL m_bAETrueLaunch;
    MBOOL m_bPlineMappingChange;
    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;
    MBOOL m_bGryoVd;
    MBOOL m_bAcceVd;
    #if MTK_CAM_NEW_NVRAM_SUPPORT
    IdxMgr* m_pIdxMgr;
    ISP_NVRAM_REGISTER_STRUCT* ISP_NVRAM_Ptr;
    #endif
    //CCU manual
    MUINT32 m_u4CCUManualShutter;
    MUINT32 m_u4CCUManualAfeGain;
    MUINT32 m_u4CCUManualIspGain;
    MBOOL m_u4CCUManualEnable;
    MBOOL m_bIsCCUAEInit;
    MBOOL m_bIsFrameRateLocked;
    //
    MUINT32 m_u4CCUEndBankIdx;
    // Debounce counter
    MUINT32 m_u4MaxIdxStableCount;
    MUINT32 m_u4ISOIdx1StableCount;
    MUINT32 m_u4ISOIdx2StableCount;
    MUINT32 m_u4Idx1StableThres;
    MUINT32 m_u4Idx2StableThres;
    MUINT32 m_u4ISOIdx1Value;
    MUINT32 m_u4ISOIdx2Value;
    MUINT32 m_u4ISO1IdxStatus;
    MUINT32 m_u4ISO2IdxStatus;
    MBOOL m_bIsMaxIndexStable;
    MBOOL m_bIsCPUNeedDebugInfo;
    MUINT32 m_u4FDProbForCCU;
    MINT32 m_u4FaceFailCntForCCU;
    MUINT32 m_u4FaceFoundCntForCCU;
    MBOOL mbIsPlineChange;
    MBOOL mbIsIndexChange;
    MUINT32 m_u4CCUManShutter;
    MUINT32 m_u4CCUManISO;
    MBOOL m_bUpdateAELockIdx;
    MBOOL m_bMVHDR3FlashUpdate; // customize for 4-cell mvhdr flashlight
    MBOOL m_bResetMvhdrRatio;
    MBOOL m_bForceUpdateAEInfoByCCU;
    MBOOL m_bForceSkipCCU;
    MBOOL m_bIsLongExpControlbyCCU;
    MBOOL m_bForceResetCCUStable;
    MINT32 m_i4CurrSensorId;
    MINT32 m_i4SuperNightShotMode;
    AE_Cust_Param_T m_AeCustParam;
    MUINT8 m_uManualModeIndex;
    MBOOL  m_bEnableCustMode;
    MBOOL NeedCPUUpdateSensor;
    AE_INFO_T rAEInfo2ISPBack;
    MBOOL m_bNeedApplyBackupAEInfo;
    AE_DEBUG_INFO_T rAEDebugInfoBack;
    MBOOL mIsBackupTorch;
};

};  //  namespace NS3Av3
#endif // _AE_MGR_H_

