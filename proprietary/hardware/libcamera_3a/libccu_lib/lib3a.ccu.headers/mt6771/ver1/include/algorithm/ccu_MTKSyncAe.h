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
#ifndef _CCU_MTK_SYNCAE_H
#define _CCU_MTK_SYNCAE_H

#include "ccu_ext_interface/ccu_MTKSync3AType.h"
//#include <camera_custom_nvram.h>
//#include <camera_custom_AEPlinetable.h>
#include "algorithm/ccu_AEPlinetable.h"
#include "algorithm/ccu_awb_param.h"
#include "algorithm/ccu_ae_param.h"
 #include "algorithm/ccu_ae_nvram.h"
#include "ccu_n3d_sync2a_tuning_param.h"
//#include "MTKSyncAeErrCode.h"
//#include "camera_custom_AEPlinetable.h"
//#include "Ae_param.h"

//#define SYNC_AE_DEBUG

typedef enum
{
    CCU_SYNC_AE_SCENARIO_PIP,
    CCU_SYNC_AE_SCENARIO_N3D
} CCU_SYNC_AE_SCENARIO_ENUM;

typedef enum
{
    CCU_SYNC_AE_FOLLOW_MAIN = 0,
    CCU_SYNC_AE_FOLLOW_LDR
} CCU_SYNC_AE_POLICY;

typedef enum
{
    CCU_SYNC_AE_PLINES_CONST_W = 0,
    CCU_SYNC_AE_EXPTIME_CONST_W,
    CCU_SYNC_AE_GAIN_CONST_W,
    CCU_SYNC_AE_EXPTIME_GAIN_INCONST_W,
    CCU_SYNC_AE_MAX_DYN_RANGE_INCONST_W
} CCU_SYNC_AE_MODE_SELECTION;

typedef enum
{
    CCU_SYNC_AE_STATE_STANDBY,
    CCU_SYNC_AE_STATE_INIT,
    CCU_SYNC_AE_STATE_PROC,
    CCU_SYNC_AE_STATE_READY,
    CCU_SYNC_AE_STATE_IDLE,
    CCU_SYNC_AE_STATE_MAX
} CCU_SYNC_AE_STATE_ENUM;

typedef enum
{
    CCU_SYNC_AE_FEATURE_SET_PROC_INFO,
    CCU_SYNC_AE_FEATURE_GET_DEBUG_INFO,
    CCU_SYNC_AE_FEATURE_GET_MAPPING_INDEX
} CCU_SYNC_AE_FEATURE_CONTROL_ENUM;

typedef enum
{
    CCU_EXPO_TIME = 0,
    CCU_EXPO_LINE
} CCU_EXPO_TYPE;

typedef enum
{
    CCU_SYNC_AE_CAM_SCENARIO_PREVIEW,
    CCU_SYNC_AE_CAM_SCENARIO_CAPTURE
} CCU_SYNC_AE_CAM_SCENARIO_ENUM;




typedef struct
{
    MUINT32 Main_LU_X;
    MUINT32 Main_LU_Y;

    MUINT32 Main_RU_X;
    MUINT32 Main_RU_Y;

    MUINT32 Main_LD_X;
    MUINT32 Main_LD_Y;

    MUINT32 Main_RD_X;
    MUINT32 Main_RD_Y;

    MUINT32 Main2_LU_X;
    MUINT32 Main2_LU_Y;

    MUINT32 Main2_RU_X;
    MUINT32 Main2_RU_Y;

    MUINT32 Main2_LD_X;
    MUINT32 Main2_LD_Y;

    MUINT32 Main2_RD_X;
    MUINT32 Main2_RD_Y;


} CCU_strImageCornerMappingTable_syncAe;


typedef struct
{
    MINT32 R;
    MINT32 G;
    MINT32 B;
}CCU_AWB_Calibration_Data;

typedef struct
{
    CCU_AWB_Calibration_Data Golden;
    CCU_AWB_Calibration_Data Unit;
} CCU_SYNCAE_EEPROM_DATA_STRUCT;

/**
 * AAO footprint
 * aao buffer pointer and its width
 */
typedef struct
{
    MVOID *pAEStatisticBuf;
    MUINT32 buf_line_size;
} CCU_SYNC_AE_AAO_BUFFER_INTO_STRUCT;

 typedef enum
{
    CCU_BAYER_SENSOR,
    CCU_MONO_SENSOR,

} CCU_SYNCAE_CAMERA_TYPE_ENUM;


typedef struct
{
    CCU_strAEOutput   Input; //assign in syncAEMain@Sync3A.cpp
    MUINT32       u4SyncGain;     // AE compensation gain //assign in syncAEMain@Sync3A.cpp
    CCU_strAEOutput   PrevInput;  // maybe not used
    unsigned int  u4ExposureMode; // specify the unit of the TV (line or ms) // 0: exposure time, 1: exposure line //assign in syncAEMain@Sync3A.cpp
    CCU_strEvPline *pCurrentTable;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MINT32        i4MaxBV;
    MINT32        i4MinBV;
    CCU_SYNCAE_CAMERA_TYPE_ENUM syncAECamType;
	CCU_strEvPline *pAEFinerEVPline;      // the Finer table used to generate Input
	MUINT32 m_u4FinerIndexMax;
    MUINT32 m_u4FinerIndexMin;
    MINT32        i4FinerMaxBV;
    MINT32        i4FinerMinBV;
    CCU_AWB_GAIN_T    wb_gain; //assign in syncAEMain@Sync3A.cpp but is it related to syncawb?
    MUINT32       u4FinverEVidx;
	MUINT32 i4Offset1000;
    CCU_SYNC_AE_AAO_BUFFER_INTO_STRUCT aao_buffer_info; //aao buffer including the buffer pointer and its length //assign in syncAEMain@Sync3A.cpp
    CCU_AE_NVRAM_T*   pAeNvram; //assign in syncAEMain@Sync3A.cpp
} CCU_SYNC_AE_INPUT_INFO_STRUCT;

typedef struct
{
    MUINT32 width;
    MUINT32 height;
    MUINT32 offset_x;
    MUINT32 offset_y;
    MUINT32 size_x;
    MUINT32 size_y;
    MUINT32 num_x;
    MUINT32 num_y;
} CCU_SYNC_AE_AAO_CONFIG_STRUCT;

typedef struct
{
    MINT32  M11;
    MINT32  M12;
    MINT32  M13;
    MINT32  M21;
    MINT32  M22;
    MINT32  M23;
    MINT32  M31;
    MINT32  M32;
    MINT32  M33;
} CCU_SYNC_AE_LOCATION_MAPPING_MATRIX_STRUCT;

typedef struct
{
    CCU_SYNC_AE_AAO_CONFIG_STRUCT AAOWinConfig_main;
    CCU_SYNC_AE_AAO_CONFIG_STRUCT AAOWinConfig_main2;
    CCU_SYNC_AE_LOCATION_MAPPING_MATRIX_STRUCT  Mapping_Matrix;
} CCU_SYNC_AE_LOCATION_TRANS_STRUCT;

// MBOOL
// Sync3AWrapper::
// syncAeMain@Sync3a.cpp
 // rAeSyncInput.eSyncAEMode = SYNC_AE_EXPTIME_CONST_W;
 // rAeSyncInput.SyncCamScenario = SYNC_AE_CAM_SCENARIO_PREVIEW;
 //     // All we need is CCU_strAEOutput@ccu_ae_param.h
 //    // bAEStable     -> bAEStable@ccu_ae_param.h             
 //    // EvSetting     -> EvSetting@ccu_ae_param.h            
 //    // Bv            -> Bv@ccu_ae_param.h            
 //    // u4AECondition -> u4AECondition@ccu_ae_param.h            
 //    // i4DeltaBV     -> i4DeltaBV@ccu_ae_param.h            
 //    // u4ISO         -> u4ISO@ccu_ae_param.h            
 //    // u2FrameRate   -> u2FrameRate@ccu_ae_param.h            
 //    // i2FlareOffset -> i2FlareOffset@ccu_ae_param.h            
 //    // i2FlareGain -> i2FlareGain@ccu_ae_param.h            
 //    // i2FaceDiffIndex -> i2FaceDiffIndex@ccu_ae_param.h                            
 //    // Except
 //    // rAeSyncInput.main_ch.u4SyncGain = 1024
 //    // rAeSyncInput.main_ch.u4ExposureMode ----> need to check
 //    // rAeSyncInput.main_ch.wb_gain ----> need to check
 //    // rAeSyncInput.main_ch.aao_buffer_info.buf_line_size = 780
 //    // rAeSyncInput.main_ch.aao_buffer_info.pAEStatisticBuf 780*90 aao buffer
 //    rAeSyncInput.main_ch.Input.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4Master);
 //    rAeSyncInput.main_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main.u4Eposuretime;
 //    rAeSyncInput.main_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main.u4AfeGain;
 //    rAeSyncInput.main_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main.u4IspGain;
 //    rAeSyncInput.main_ch.Input.EvSetting.uIris = 0;  // Iris fix
 //    rAeSyncInput.main_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
 //    rAeSyncInput.main_ch.Input.EvSetting.uFlag = 0; // No hypersis
 //    rAeSyncInput.main_ch.Input.Bv = IAeMgr::getInstance().getBVvalue(m_i4Master);
 //    rAeSyncInput.main_ch.Input.u4AECondition = u4AEConditionMain;
 //    rAeSyncInput.main_ch.Input.i4DeltaBV = IAeMgr::getInstance().getDeltaBV(m_i4Master);
 //    rAeSyncInput.main_ch.Input.u4ISO = _a_rAEOutput_main.u4RealISO;
 //    rAeSyncInput.main_ch.Input.u2FrameRate = _a_rAEOutput_main.u2FrameRate;
 //    rAeSyncInput.main_ch.Input.i2FlareOffset = _a_rAEOutput_main.i2FlareOffset;
 //    rAeSyncInput.main_ch.Input.i2FlareGain = _a_rAEOutput_main.i2FlareGain;
 //    rAeSyncInput.main_ch.Input.i2FaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4Master);
 //    rAeSyncInput.main_ch.Input.i4AEidxCurrent = rLCEInfoMain.i4AEidxCur; 
 //    rAeSyncInput.main_ch.Input.i4AEidxNext = rLCEInfoMain.i4AEidxNext; 
 //    rAeSyncInput.main_ch.Input.u4CWValue = _a_rAEOutput_main.u4CWValue;
 //    rAeSyncInput.main_ch.u4SyncGain = m_rAeSyncOutput.main_ch.u4SyncGain; //init
 //    rAeSyncInput.main_ch.u4ExposureMode = _a_rAEOutput_main.u4ExposureMode;   // exposure time // ask yuming
 //    rAeSyncInput.main_ch.pAETable = &strPreviewAEPlineTableMain;
 //    rAeSyncInput.main_ch.pAeNvram = &pMasterNvram->rAENVRAM[0];
 //    rAeSyncInput.main_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pMasterSttBuf->getPart(AAOSepBuf); // reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4Master));
 //    rAeSyncInput.main_ch.aao_buffer_info.buf_line_size = IAeMgr::getInstance().getAAOLineByteSize(m_i4Master);
 //    rAeSyncInput.main_ch.wb_gain = rAwbMaster.rAWBSyncInput_N3D.rAlgGain; //Ask jh

 //    rAeSyncInput.sub_ch.Input.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4Slave);
 //    rAeSyncInput.sub_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main2.u4Eposuretime;
 //    rAeSyncInput.sub_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main2.u4AfeGain;
 //    rAeSyncInput.sub_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main2.u4IspGain;
 //    rAeSyncInput.sub_ch.Input.EvSetting.uIris = 0;  // Iris fix
 //    rAeSyncInput.sub_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
 //    rAeSyncInput.sub_ch.Input.EvSetting.uFlag = 0; // No hypersis
 //    rAeSyncInput.sub_ch.Input.Bv = IAeMgr::getInstance().getBVvalue(m_i4Slave);
 //    rAeSyncInput.sub_ch.Input.u4AECondition = u4AEConditionMain2;
 //    rAeSyncInput.sub_ch.Input.i4DeltaBV = IAeMgr::getInstance().getDeltaBV(m_i4Slave);
 //    rAeSyncInput.sub_ch.Input.u4ISO = _a_rAEOutput_main.u4RealISO;
 //    rAeSyncInput.sub_ch.Input.u2FrameRate = _a_rAEOutput_main2.u2FrameRate;
 //    rAeSyncInput.sub_ch.Input.i2FlareOffset = _a_rAEOutput_main2.i2FlareOffset;
 //    rAeSyncInput.sub_ch.Input.i2FlareGain = _a_rAEOutput_main2.i2FlareGain;
 //    rAeSyncInput.sub_ch.Input.i2FaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4Slave);
 //    rAeSyncInput.sub_ch.Input.i4AEidxCurrent = rLCEInfoMain2.i4AEidxCur;
 //    rAeSyncInput.sub_ch.Input.i4AEidxNext = rLCEInfoMain2.i4AEidxNext;
 //    rAeSyncInput.sub_ch.Input.u4CWValue = _a_rAEOutput_main2.u4CWValue;
 //    rAeSyncInput.sub_ch.u4SyncGain = m_rAeSyncOutput.sub_ch.u4SyncGain;
 //    rAeSyncInput.sub_ch.u4ExposureMode = _a_rAEOutput_main2.u4ExposureMode;   // exposure time
 //    rAeSyncInput.sub_ch.pAETable = &strPreviewAEPlineTableMain2;
 //    rAeSyncInput.sub_ch.pAeNvram = &pSlaveNvram->rAENVRAM[0];
 //    rAeSyncInput.sub_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pSlaveSttBuf->getPart(AAOSepBuf); //reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4Slave));
 //    rAeSyncInput.sub_ch.aao_buffer_info.buf_line_size = IAeMgr::getInstance().getAAOLineByteSize(m_i4Slave);
 //    rAeSyncInput.sub_ch.wb_gain = rAwbSlave.rAWBSyncInput_N3D.rAlgGain;

typedef struct
{
    CCU_SYNC_AE_MODE_SELECTION eSyncAEMode; //assign in syncAEMain@Sync3A.cpp
    CCU_SYNC_AE_LOCATION_TRANS_STRUCT syncTransInfo;  //need to check
    CCU_SYNC_AE_CAM_SCENARIO_ENUM SyncCamScenario; //assign in syncAEMain@Sync3A.cpp
    CCU_SYNC_AE_INPUT_INFO_STRUCT main_ch; 
    CCU_SYNC_AE_INPUT_INFO_STRUCT main2_ch;
} CCU_SYNC_AE_INPUT_STRUCT;

typedef struct
{
    CCU_strAEOutput   Output;         // the result of Input calculation
    MUINT32       u4SyncGain;     // AE compensation gain
} CCU_SYNC_AE_OUTPUT_INFO_STRUCT;

typedef struct
{
    CCU_SYNC_AE_OUTPUT_INFO_STRUCT main_ch;
    CCU_SYNC_AE_OUTPUT_INFO_STRUCT main2_ch;
} CCU_SYNC_AE_OUTPUT_STRUCT;

typedef struct
{
	bool enFinerSyncOffset; // False: 10base, TRUE: 1000base
    bool enPPGainComp;  
    MUINT32 u4FixSyncGain;
    CCU_SYNC_AE_POLICY syncPolicy;   
    MUINT32 pDeltaBVtoRatioArray[CCU_SYNC_AE_DUAL_CAM_DENOISE_MAX][CCU_MAX_MAPPING_DELTABV_ISPRATIO];    
} CCU_SYNC_AE_TUNNING_PARAM_STRUCT;





typedef struct
{
    CCU_AE_NVRAM_T* pAeNvram;
    MINT32 BVOffset[2];
    MINT32 RGB2YCoef[3];
    CCU_SYNCAE_EEPROM_DATA_STRUCT syncAECalData;
    CCU_SYNCAE_CAMERA_TYPE_ENUM syncAECamType;
} CCU_SYNC_AE_INIT_INPUT_PARAM;
// MBOOL
// Sync3AWrapper::
// syncAeInit() @ Sync3a.cpp
    // pSyncAEInitInfo = getSyncAEInitInfo(); //n3d_sync2a_tuning_param.cpp
    // rAEInitData.SyncAeTuningParam.syncPolicy = (SYNC_AE_POLICY)pSyncAEInitInfo->SyncWhichEye;
    // rAEInitData.SyncAeTuningParam.enPPGainComp = pSyncAEInitInfo->isDoGainRegression;
    // rAEInitData.SyncAeTuningParam.u4FixSyncGain = pSyncAEInitInfo->FixSyncGain;
    // rAEInitData.SyncScenario = SYNC_AE_SCENARIO_N3D;
    // rAEInitData.SyncNum = 2; //N3D 2 cam
    // rAEInitData.main_param.syncAECalData.Golden = AWBCalData;  // todo
    // rAEInitData.main_param.syncAECalData.Unit = AWBCalData;    // todo
    // rAEInitData.sub_param.syncAECalData.Golden = AWBCalData;   // todo
    // rAEInitData.sub_param.syncAECalData.Unit = AWBCalData;     // todo
    // rAEInitData.main_param.syncAECamType = querySensorType(m_i4Master); // BAYER or MONO
    // rAEInitData.sub_param.syncAECamType = querySensorType(m_i4Slave); // BAYER or MONO

    // ::memcpy(rAEInitData.SyncAeTuningParam.pDeltaBVtoRatioArray, &pSyncAEInitInfo->pDeltaBVtoRatioArray[0][0], sizeof(MUINT32)*2*30);
    // ::memcpy(rAEInitData.main_param.BVOffset, pSyncAEInitInfo->EVOffset_main, sizeof(MUINT32)*2);
    // ::memcpy(rAEInitData.sub_param.BVOffset, pSyncAEInitInfo->EVOffset_main2, sizeof(MUINT32)*2);
    // ::memcpy(rAEInitData.main_param.RGB2YCoef, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32)*3);
    // ::memcpy(rAEInitData.sub_param.RGB2YCoef, pSyncAEInitInfo->RGB2YCoef_main2, sizeof(MUINT32)*3);
typedef struct
{
    CCU_SYNC_AE_SCENARIO_ENUM SyncScenario;
    MINT32 SyncNum;
    CCU_SYNC_AE_INIT_INPUT_PARAM main_param;
    CCU_SYNC_AE_INIT_INPUT_PARAM sub_param;
    CCU_SYNC_AE_TUNNING_PARAM_STRUCT SyncAeTuningParam;
} CCU_SYNC_AE_INIT_STRUCT;
// VOID
// Sync3AWrapper::
// syncAeSetting @ Sync3a.cpp
//     rInput.MasterCamAeIdx = rMasterFrameParams.u4AEIndex;
//     rInput.MasterCamBVOffset = pMaster3aNvram->rAENVRAM[0].rCCTConfig.i4BVOffset;
//     rInput.SlaveCamBVOffset = pSlave3aNvram->rAENVRAM[0].rCCTConfig.i4BVOffset;
//     rInput.pMasterAETable = &strPreviewAEPlineTableMain;
//     rInput.pSlaveAETable = &strPreviewAEPlineTableMain2;

typedef struct
{
    MINT32 MasterCamAeIdx;
	MINT32 MasterCamAeIdxF;
    MINT32 MasterFinerEVIdxBase;
    MINT32 MasterCamBVOffset;
    CCU_strEvPline *pCurrentTableMaster;
    MUINT32 m_u4IndexMaxMaster;
    MUINT32 m_u4IndexMinMaster;
    MINT32        i4MaxBVMaster;
    MINT32        i4MinBVMaster;
	MINT32 SlaveFinerEVIdxBase;
    MINT32 SlaveCamBVOffset;
    CCU_strEvPline *pCurrentTableSlave;
    MUINT32 m_u4IndexMaxSlave;
    MUINT32 m_u4IndexMinSlave;
    MINT32        i4MaxBVSlave;
    MINT32        i4MinBVSlave;
}CCU_SYNC_AE_DIRECT_INDEX_MAPPING_STRUCT;


typedef struct
{
    MUINT32 u4SlaveAEindex;
	MUINT32 u4SlaveAEindexF;
    CCU_strEvSetting EVSetting;
}CCU_SYNC_AE_DIRECT_MAP_RESULT_STRUCT;

#endif
