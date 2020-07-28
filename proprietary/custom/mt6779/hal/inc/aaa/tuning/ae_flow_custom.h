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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#ifndef _AE_FLOW_CUSTOM_H_
#define _AE_FLOW_CUSTOM_H_

#include "camera_custom_types.h"

#define AE_Y_HISTOGRAM_BIN (256)

#define AE_GAIN_BASE_ISP 4096  // temp define
#define AE_GAIN_BASE_AFE 1024  // temp define

typedef enum
{
    AE_MGR_STATE_INIT = 0,
    AE_MGR_STATE_DOPVAE,
    AE_MGR_STATE_DOAFAE,
    AE_MGR_STATE_DOPRECAPAE,
    AE_MGR_STATE_DOCAPAE,
    AE_MGR_STATE_DOPOSTCAPAE,
    AE_MGR_STATE_DOBACKUPAE,
    AE_MGR_STATE_DORESTOREAE,
    AE_MGR_STATE_MAXIMUM
} AE_MGR_STATE_ENUM;

typedef struct AEMeterAreaInfo {
    MINT32 i4Left;
    MINT32 i4Top;
    MINT32 i4Right;
    MINT32 i4Bottom;
    MINT32 i4Weight;
    MINT32 i4Id;
    MINT32 i4Type; // 0:GFD, 1:LFD, 2:OT
    MINT32 i4Motion[2];
    MINT32 i4Landmark[3][4]; // index 0: left eye, index 1: right eye, index 2:mouth
    MINT32 i4LandmarkCV;
    MINT32 i4ROP;
    MINT32 i4LandMarkRip;
    MINT32 i4LandMarkRop;
} AEMeterAreaInfo_T;

enum EAECtrl_T
{
    EAECtrl_Begin = 0,
    // AE get
    EAECtrl_GetCapDelayFrame = 0x0001,
    EAECtrl_GetAAOLineByteSize,
    EAECtrl_GetCapPlineTable,
    EAECtrl_GetSensorDelayInfo,
    EAECtrl_GetFDMeteringAreaInfo,
    EAECtrl_GetAEInitExpSetting,
    EAECtrl_GetISOSpeedMode,
    EAECtrl_GetAEMaxMeterAreaNum,
    EAECtrl_GetAEMeterMode,
    EAECtrl_GetCapParams,
    EAECtrl_GetCCUResultCBActive,
    EAECtrl_GetCurrentPlineTable,
    EAECtrl_GetCurrentPlineTableF,
    EAECtrl_GetDebugInfo,
    EAECtrl_GetEVCompensateIndex,
    EAECtrl_GetExposureInfo,
    EAECtrl_GetNVRAParam,
    EAECtrl_GetSensorDeviceInfo,
    EAECtrl_GetSensorRollingShutter,
    EAECtrl_GetIsAELockSupported,
    EAECtrl_GetCapDiffEVState,
    EAECtrl_GetExpSettingByShutterISOPriority,
    EAECtrl_GetNeedPresetControlCCU,
    EAECtrl_GetAELock,

    // AE set
    EAECtrl_SetContinueShot = 0x0100,
    EAECtrl_SetPlineTableLimitation,
    EAECtrl_SetPresetControlCCU,
    EAECtrl_SetAAOMode,
    EAECtrl_SetAAOProcInfo,
    EAECtrl_SetAutoFlickerMode,
    EAECtrl_SetLimiterMode,
    EAECtrl_SetSMBuffermode,
    EAECtrl_SetState2Converge,
    EAECtrl_SetTargetMode,
    EAECtrl_SetCamScenarioMode,
    EAECtrl_SetEMVHDRratio,
    EAECtrl_SetFDInfo,
    EAECtrl_SetMVHDR3ExpoProcInfo,
    EAECtrl_SetNVRAMIndex,
    EAECtrl_SetOTInfo,
    EAECtrl_SetSensorMode,
    EAECtrl_SetStrobeMode,
    EAECtrl_SetzCHDRShot,
    EAECtrl_SetUpdateAEBV,
    EAECtrl_SetUnderExpdeltaBVIdx,
    EAECtrl_SetCaptureParams,
    EAECtrl_SetISPNvramLTMIndex,
    EAECtrl_SetISPNvramOBCTable,
    EAECtrl_SetPreviewParams,
    EAECtrl_SetSensorListenerParams,
    EAECtrl_SetSensorParams,
    EAECtrl_SetStereoDenoiseRatio,
    EAECtrl_EnableAEOneShotControl,
    EAECtrl_EnableAEStereoManualPline,
    EAECtrl_EnableAISManualPline,
    EAECtrl_EnableBMDNManualPline,
    EAECtrl_EnableEISRecording,
    EAECtrl_EnableFlareInManualControl,
    EAECtrl_EnableHDRShot,
    EAECtrl_EnableMFHRManualPline,
    EAECtrl_EnableAE,
    EAECtrl_DisableAE,
    EAECtrl_SetCCUManualControl,
    EAECtrl_SetDoBackAEInfo,
    EAECtrl_SetDoRestoreAEInfo,
    EAECtrl_SetAFAELock,
    EAECtrl_SetFDenable,
    EAECtrl_SetRestore,
    EAECtrl_SetSensorbyI2C,
    EAECtrl_SetSensorbyI2CBufferMode,
    EAECtrl_SetCCUOnOff,
    EAECtrl_SetExposureSetting,
    EAECtrl_SetAEMgrPreState,
    EAECtrl_SetAEMode,
    EAECtrl_SetDynamicHFPS,
    EAECtrl_NotifyMainFlashON,
    EAECtrl_SetSkipCalAE,
    EAECtrl_SetFDSkipCalAE,
    EAECtrl_EnableShutterISOPriority,
    EAECtrl_EnableSuperNightShot,
    EAECtrl_SetReconfig,
    EAECtrl_Num
};


// Update information for ISP used
struct AE_ISP_INFO_T {
    MBOOL   bAEStable;
    MBOOL   bGammaEnable;
    MBOOL   bAETouchEnable;
    MBOOL   bFaceAELCELinkEnable;
    MBOOL   bIsPlineMaxIndex;
    MBOOL   bDisableDGN;	
    MBOOL   bEnableFaceAE;
    MBOOL   bOTFaceTimeOutLockAE;
	MUINT32 u4RequestNum;
    MUINT32 u4MagicNumber;	
    MUINT32 u4AETarget;
    MUINT32 u4AECurrentTarget;
    MUINT64 u8P1Exposuretime_ns;   //!<: Exposure time in ns, 1000 mean 1 us
    MUINT32 u4P1SensorGain;        //!<: sensor gain,   1x = 1024
    MUINT32 u4P1DGNGain;           //!<: digital gain,  1x = 4096
    MUINT32 u4P1RealISOValue;
    MUINT32 u4P1Gain_x100;         // 1x = 100 (Sensor Gain * ISP Gain)
    MUINT64 u8P2Exposuretime_ns;   //!<: Exposure time in ns, 1000 mean 1 us
    MUINT32 u4P2SensorGain;        //!<: sensor gain,   1x = 1024
    MUINT32 u4P2DGNGain;           //!<: digital gain,  1x = 4096
    MUINT32 u4P2RealISOValue;
    MUINT32 u4P2Gain_x100;         // 1x = 100 (Sensor Gain * ISP Gain)	
    MUINT32 u4AlgoExposuretime_us; //!<: Exposure time in us
    MUINT32 u4AlgoSensorGain;      //!<: sensor gain,   1x = 1024
    MUINT32 u4AlgoDGNGain;         //!<: digital gain,  1x = 4096
    MUINT32 u4AlgoRealISOValue;    
    MINT32  i4LightValue_x10;
    MUINT32 u4AECondition;
    MINT16  i2FlareOffset;       // to be removed
    MINT16  i2FlareGain;         // to be removed
    MINT32  i4GammaIdx;          // next gamma idx
    MINT32  i4LESE_Ratio;        // LE/SE ratio
    MUINT32 u4MaxISO;
    MUINT32 u4AEStableCnt;
    MUINT32 u4Histogrm[AE_Y_HISTOGRAM_BIN];
    MUINT32 u4EVRatio;
    MINT32  i4deltaIndex;
    MINT32  u4AEFinerEVIdxBase;
    MUINT32 u4FaceAEStable;
    MINT32  i4Crnt_FDY;
    MUINT32 u4MeterFDTarget;
    MUINT32 u4FaceFinalTarget;
    MUINT32 u4FDProb;
    MUINT32 u4FaceNum;
    MINT32  i4FDY_ArrayOri[MAX_AE_METER_AREAS];
    MINT32  i4FDY_Array[MAX_AE_METER_AREAS];	
    AEMeterAreaInfo LandMarkFDArea;
    AEMeterAreaInfo LandMarkFDSmoothArea;
    AEMeterAreaInfo FDArea[MAX_AE_METER_AREAS];
    // for face LCE
    
    MUINT32 u4MaxGain;
    MINT32  i4FaceBrightToneProtectValue_12bit[MAX_AE_METER_AREAS];
    MUINT8	uFaceState;
    MUINT32 u4FaceRobustCnt;
    MUINT32 u4FaceRobustTrustCnt;
    MUINT32 u4FD_Lock_MaxCnt;
    MUINT32 u4FDDropTempSmoothCnt;
    MUINT32 u4OTFaceTrustCnt;
    MUINT32 u4MeterFDLinkTarget;
    //mark for P80
    //HDR_TONE_INFO_T rHdrToneInfo;
    //strEvHdrSetting    rHdrAEInfo;
    MINT32  i4RealLightValue_x10;   //real LV
    MINT32  u4EISExpRatio;
	MINT32  i4AEComp;
};

// Update other per-frame infomation
struct AE_UPDATE_INFO_T {
    MBOOL   bEnableRAFastConverge;
    MBOOL   TgCtrlRight;
    MBOOL   bAELock;
    MBOOL   bAELimiter;
    MBOOL   bAEScenarioChange;
    MBOOL   bAPAELock;
    MBOOL   bLVChangeTooMuch;
    MBOOL   bIsNeedUpdateCPUstatus;
    MBOOL   bStrobeBVTrigger;
    MUINT32 pu4AEBlock[AE_BLOCK_NO][AE_BLOCK_NO];
    MUINT16 u4CWValue;
    MUINT16 u4AvgY;
    MINT32  i4AEidxCur;
    MUINT32 u4AEidxCurrentF;
    MINT32  i4AEidxNext;
    MINT32  i4AEidxNextF;
    MINT32  i4DeltaBV;
    MUINT32 u4AECondition;
    MUINT16 u2FrameRate_x10;
    MINT16  i2AEFaceDiffIndex;
    MINT32  i4HdrOnOff;
    MINT32  i4AEMode;
    MINT32  i4AEState;
    MINT32  i4BVvalue_x10;
    MINT32  i4AOECompLVvalue;
    MUINT32 u4ExposureMode;  // 0: exposure time, 1: exposure line
    MUINT64 u8FrameDuration_ns;
	MUINT32 u4LuxValue_x10000;
};

struct AE_PERFRAME_INFO_T {
    AE_ISP_INFO_T rAEISPInfo;
    AE_UPDATE_INFO_T rAEUpdateInfo;
};

#endif

