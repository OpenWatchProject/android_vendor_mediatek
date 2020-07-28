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
#include "algorithm/ccu_AEPlinetable.h"
#include "algorithm/ccu_ae_param.h"
#include "algorithm/ccu_ae_nvram.h"
#include "algorithm/ccu_ae_sync_nvram.h"
#include "ccu_n3d_sync2a_tuning_param.h"

#define CCU_SYNC_AE_SUPPORT_SENSORS 4

// remove it
typedef enum
{
    CCU_SYNC_AE_FOLLOW_MAIN = 0,
    CCU_SYNC_AE_FOLLOW_LDR
} CCU_SYNC_AE_POLICY;

// Sync type
typedef enum
{
    CCU_SYNC_AE_FULLY_SYNC = 0,
    CCU_SYNC_AE_HALF_SYNC,
} CCU_SYNC_AE_SYNC_TYPE_ENUM;

// align mode
typedef enum
{
    CCU_SYNC_AE_NON_SYNC = 0,
    CCU_SYNC_AE_NON_SYNC_LIMIT,
    CCU_SYNC_AE_PLINES_ALIGN,
    CCU_SYNC_AE_PLINES_ALIGN_SHUTTER,
    CCU_SYNC_AE_PLINES_ALIGN_GAIN,
    CCU_SYNC_AE_ALIGN_SHUTTER,
    CCU_SYNC_AE_ALIGN_GAIN,
} CCU_SYNC_AE_ALIGN_MODE_ENUM;


typedef enum
{
    CCU_SYNC_AE_FEATURE_SET_PROC_INFO,
    CCU_SYNC_AE_FEATURE_GET_DEBUG_INFO,
    CCU_SYNC_AE_FEATURE_GET_MAPPING_INDEX
} CCU_SYNC_AE_FEATURE_CONTROL_ENUM;


// camera type
typedef enum
{
    CCU_BAYER_SENSOR,
    CCU_MONO_SENSOR,
} CCU_SYNCAE_CAMERA_TYPE_ENUM;


// Range for uint32 type
typedef struct
{
    MUINT32 u4Min;
    MUINT32 u4Max;
}CCU_SYNCAE_U4RANGE_T;


// Range for int32 type
typedef struct
{
    MINT32 i4Min;
    MINT32 i4Max;
}CCU_SYNCAE_I4RANGE_T;


// Ae sync input data for camera X
typedef struct
{
    CCU_SYNCAE_CAMERA_TYPE_ENUM eCamType;
    CCU_LIB3A_AE_FLICKER_MODE_T eAeFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAeAutoFlickerMode;

    MINT32 i4SensorMode;
    MUINT32 u4BaseIdxF;
    MUINT32 u4BaseISPGain;
    MUINT32 u4AvgY;

    MUINT32 u4ExpUnit;
    MUINT32 u4ShutterBase;

    CCU_SYNCAE_U4RANGE_T rAeIdxFRange;
    CCU_SYNCAE_U4RANGE_T rExpRange;
    CCU_SYNCAE_U4RANGE_T rAfeGainRange;
    CCU_SYNCAE_U4RANGE_T rIspGainRange;
    CCU_SYNCAE_I4RANGE_T rBvFRange;

    CCU_strAEOutput rInput;
    CCU_strAEOutput rPrevInput;

    CCU_strFinerEvPline *prPlineTableF;
    CCU_AE_NVRAM_T *prAeNvram;
    CCU_strAEPLineGainList *prSensorGainTable;
    CCU_AESYNC_NVRAM_T *prAeSyncNvram;
} CCU_SYNC_AE_CAM_INPUT_T;


// Ae sync input data
typedef struct
{
    MUINT16 u2SyncNum;
    MUINT16 u2MasterId;
    CCU_SYNC_AE_SYNC_TYPE_ENUM eSyncType;
    CCU_SYNC_AE_CAM_INPUT_T rCamInput[CCU_SYNC_AE_SUPPORT_SENSORS];
} CCU_SYNC_AE_INPUT_T;


// Ae sync output data for camera X
typedef struct
{
    MUINT32 u4OffsetBase;
    MUINT32 u4RegGain;
    MUINT32 u4ApplyGain;
    MINT32 i4EvDiff;
    CCU_strAEOutput   Output;
} CCU_SYNC_AE_CAM_OUTPUT_T;


// Ae sync output data
typedef struct
{
    CCU_SYNC_AE_CAM_OUTPUT_T rCamOutput[CCU_SYNC_AE_SUPPORT_SENSORS];
    CCU_SYNC2SINGLE_REPORT strSync2Single;
} CCU_SYNC_AE_OUTPUT_T;


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
    CCU_SYNCAE_CAMERA_TYPE_ENUM syncAECamType;
} CCU_SYNC_AE_INIT_INPUT_PARAM;


typedef struct
{
    MINT32 SyncNum;
    CCU_SYNC_AE_INIT_INPUT_PARAM main_param;
    CCU_SYNC_AE_INIT_INPUT_PARAM sub_param;
    CCU_SYNC_AE_TUNNING_PARAM_STRUCT SyncAeTuningParam;
} CCU_SYNC_AE_INIT_STRUCT;


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
    MINT32 MasterID;
    MINT32 isMasterStable;
}CCU_SYNC_AE_DIRECT_INDEX_MAPPING_STRUCT;


typedef struct
{
    MUINT32 u4SlaveAEindex;
    MUINT32 u4SlaveAEindexF;
    CCU_strEvSetting EVSetting;
    CCU_SYNC2SINGLE_REPORT strSync2Single;
}CCU_SYNC_AE_DIRECT_MAP_RESULT_STRUCT;

#endif
