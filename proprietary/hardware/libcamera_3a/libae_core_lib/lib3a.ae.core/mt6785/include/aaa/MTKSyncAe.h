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
#ifndef _MTK_SYNCAE_H
#define _MTK_SYNCAE_H

#include "camera_custom_ae_nvram.h"
#include "MTKSyncAeErrCode.h"

/*****************************************************************************
custom\mt6771\hal\inc\n3d_sync2a_tuning_param.h - Start
******************************************************************************/
#define MAX_MAPPING_DELTABV_ISPRATIO 30
/*****************************************************************************
custom\mt6771\hal\inc\n3d_sync2a_tuning_param.h - End
******************************************************************************/


typedef enum
{
    SYNC_AE_SCENARIO_PIP,
    SYNC_AE_SCENARIO_N3D
} SYNC_AE_SCENARIO_ENUM;

typedef enum
{
    SYNC_AE_FOLLOW_MAIN = 0,
    SYNC_AE_FOLLOW_LDR
} SYNC_AE_POLICY;

typedef enum
{
    SYNC_AE_PLINES_CONST_W = 0,
    SYNC_AE_EXPTIME_CONST_W,
    SYNC_AE_GAIN_CONST_W,
    SYNC_AE_EXPTIME_GAIN_INCONST_W,
    SYNC_AE_MAX_DYN_RANGE_INCONST_W
} SYNC_AE_MODE_SELECTION;

typedef enum
{
    SYNC_AE_STATE_STANDBY,
    SYNC_AE_STATE_INIT,
    SYNC_AE_STATE_PROC,
    SYNC_AE_STATE_READY,
    SYNC_AE_STATE_IDLE,
    SYNC_AE_STATE_MAX
} SYNC_AE_STATE_ENUM;

typedef enum
{
    SYNC_AE_FEATURE_SET_PROC_INFO,
    SYNC_AE_FEATURE_GET_DEBUG_INFO,
    SYNC_AE_FEATURE_GET_MAPPING_INDEX
} SYNC_AE_FEATURE_CONTROL_ENUM;

typedef enum
{
    EXPO_TIME = 0,
    EXPO_LINE
} EXPO_TYPE;

typedef enum
{
    SYNC_AE_CAM_SCENARIO_PREVIEW,
    SYNC_AE_CAM_SCENARIO_CAPTURE
} SYNC_AE_CAM_SCENARIO_ENUM;



// 2A sync
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


} strImageCornerMappingTable_syncAe;

/*****************************************************************************
custom\mt6771\hal\inc\Camera_custom_awb_nvram.h - Start
******************************************************************************/

// AWB gain
typedef struct
{
    INT32 i4R; // R gain
    INT32 i4G; // G gain
   INT32 i4B; // B gain
} AESYNC_AWB_GAIN_T;

/*****************************************************************************
custom\mt6771\hal\inc\Camera_custom_awb_nvram.h - End
******************************************************************************/

typedef struct
{
    MINT32 R;
    MINT32 G;
    MINT32 B;
}AWB_Calibration_Data;

typedef struct
{
    AWB_Calibration_Data Golden;
    AWB_Calibration_Data Unit;
} SYNCAE_EEPROM_DATA_STRUCT;


typedef struct
{
    MVOID *pAEStatisticBuf;
    MUINT32 buf_line_size;
} SYNC_AE_AAO_BUFFER_INTO_STRUCT;


typedef struct
{
    strAEOutput   Input;
    MUINT32       u4SyncGain;     // AE compensation gain
    strAEOutput   PrevInput;
    unsigned int  u4ExposureMode; // specify the unit of the TV (line or ms)
    strAETable    *pAETable;      // the table used to generate Input
    strFinerEvPline *pAEFinerEVPline;      // the Finer table used to generate Input

    AESYNC_AWB_GAIN_T    wb_gain;
    MUINT32       u4FinverEVidx;
    SYNC_AE_AAO_BUFFER_INTO_STRUCT aao_buffer_info;
    AE_NVRAM_T*   pAeNvram;
} SYNC_AE_INPUT_INFO_STRUCT;

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
} SYNC_AE_AAO_CONFIG_STRUCT;

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
} SYNC_AE_LOCATION_MAPPING_MATRIX_STRUCT;

typedef struct
{
    SYNC_AE_AAO_CONFIG_STRUCT AAOWinConfig_main;
    SYNC_AE_AAO_CONFIG_STRUCT AAOWinConfig_main2;
    SYNC_AE_LOCATION_MAPPING_MATRIX_STRUCT  Mapping_Matrix;
} SYNC_AE_LOCATION_TRANS_STRUCT;


typedef struct
{
    SYNC_AE_MODE_SELECTION eSyncAEMode; // 2A sync
    SYNC_AE_LOCATION_TRANS_STRUCT syncTransInfo;  // 2A sync
    SYNC_AE_CAM_SCENARIO_ENUM SyncCamScenario;
    SYNC_AE_INPUT_INFO_STRUCT main_ch;
    SYNC_AE_INPUT_INFO_STRUCT sub_ch;
} SYNC_AE_INPUT_STRUCT;

typedef struct
{
    strAEOutput   Output;         // the result of Input calculation
    MUINT32       u4SyncGain;     // AE compensation gain
} SYNC_AE_OUTPUT_INFO_STRUCT;

typedef struct
{
    SYNC_AE_OUTPUT_INFO_STRUCT main_ch;
    SYNC_AE_OUTPUT_INFO_STRUCT sub_ch;
} SYNC_AE_OUTPUT_STRUCT;

typedef struct
{
    bool enFinerSyncOffset;
    bool enPPGainComp;  // 2A sync
    MUINT32 u4FixSyncGain;
    MUINT32 u4RegressionType;//0:cwv 1:avg
    SYNC_AE_POLICY syncPolicy;
    MUINT32 pDeltaBVtoRatioArray[SYNC_AE_DUAL_CAM_DENOISE_MAX][MAX_MAPPING_DELTABV_ISPRATIO];
} SYNC_AE_TUNNING_PARAM_STRUCT;



typedef enum
{
    BAYER_SENSOR,
    MONO_SENSOR,

} SYNCAE_CAMERA_TYPE_ENUM;

typedef struct
{
    AE_NVRAM_T* pAeNvram;
    MINT32 BVOffset[2];
    MINT32 RGB2YCoef[3];
    SYNCAE_EEPROM_DATA_STRUCT syncAECalData;
    SYNCAE_CAMERA_TYPE_ENUM syncAECamType;
} SYNC_AE_INIT_INPUT_PARAM;

typedef struct
{
    SYNC_AE_SCENARIO_ENUM SyncScenario;
    MINT32 SyncNum;
    SYNC_AE_INIT_INPUT_PARAM main_param;
    SYNC_AE_INIT_INPUT_PARAM sub_param;
    SYNC_AE_TUNNING_PARAM_STRUCT SyncAeTuningParam;
} SYNC_AE_INIT_STRUCT;



typedef struct
{
    MINT32 MasterCamAeIdx;
    MINT32 MasterCamAeIdxF;
    MINT32 MasterFinerEVIdxBase;
    MINT32 MasterCamBVOffset;
    strAETable *pMasterAETable;
    MINT32 SlaveFinerEVIdxBase;
    MINT32 SlaveCamBVOffset;
    strAETable *pSlaveAETable;
}SYNC_AE_DIRECT_INDEX_MAPPING_STRUCT;


typedef struct
{
    MUINT32 u4SlaveAEindex;
    MUINT32 u4SlaveAEindexF;
    strEvSetting EVSetting;
}SYNC_AE_DIRECT_MAP_RESULT_STRUCT;




class MTKSyncAe
{
public:
    static MTKSyncAe* createInstance();
    virtual void destroyInstance() = 0;

    virtual ~MTKSyncAe() {};

    virtual MRESULT SyncAeInit(void* InitData);
    virtual MRESULT SyncAeMain(SYNC_AE_INPUT_STRUCT *pAeSyncInput, SYNC_AE_OUTPUT_STRUCT *pAeSyncOutput);

    virtual MRESULT SyncAeFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};


#endif
