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
#ifndef _MTK_EIS_PLUS_H
#define _MTK_EIS_PLUS_H

#include "MTKEisPlusType.h"
#include "MTKEisPlusErrCode.h"
//#include "EisPlusCore.h"

//#define EIS_PLUS_DEBUG

#define EIS_WIN_NUM 32

typedef enum
{
    EIS_PLUS_STATE_STANDBY,
    EIS_PLUS_STATE_INIT,
    EIS_PLUS_STATE_PROC,
    EIS_PLUS_STATE_FINISH,
    EIS_PLUS_STATE_IDLE
}EIS_PLUS_STATE_ENUM;

typedef enum
{
    EIS_PLUS_PROCESS_OK,
    EIS_PLUS_PROCESS_ERROR_STATE,
    EIS_PLUS_PROCESS_WRONG_MEMORY
} EIS_PLUS_ERROR_ENUM;

typedef enum
{
    EIS_PLUS_FEATURE_BEGIN = 0,
    EIS_PLUS_FEATURE_SET_PROC_INFO,
    EIS_PLUS_FEATURE_GET_PROC_INFO,
    EIS_PLUS_FEATURE_SET_WORK_BUF_INFO,
    EIS_PLUS_FEATURE_GET_RESULT_INFO,
    EIS_PLUS_FEATURE_SET_DEBUG_INFO,
    EIS_PLUS_FEATURE_GET_EIS_STATE,
    EIS_PLUS_FEATURE_SAVE_LOG,
    EIS_PLUS_FEATURE_MAX
} EIS_PLUS_FEATURE_ENUM;

typedef enum
{
    EIS_PLUS_WARP_PRECISION_5_BITS = 0,
    EIS_PLUS_WARP_PRECISION_4_BITS = 1
} EIS_PLUS_WARP_PRECISION_ENUM;

typedef struct
{
    void* extMemStartAddr; //working buffer start address
    MUINT32 extMemSize;
} EIS_PLUS_SET_WORKING_BUFFER_STRUCT, *P_EIS_PLUS_SET_WORKING_BUFFER_STRUCT;

typedef struct
{
    MINT32 warping_mode; //0: 6 coefficient , 1: 4 coefficient, 2: 6/4 adaptive (default), 3: 2 coefficient
    MINT32 effort; //0~2, 0: high, 1:middle, 2:low
    MINT32 search_range_x;//32~64, default:64
    MINT32 search_range_y;//32~64, default:64
    MINT32 crop_ratio; //10~40, default:20
    MINT32 gyro_still_time_th; // 1~20  ,default :6
    MINT32 gyro_max_time_th; // 8~52 ,default : 14
    MINT32 gyro_similar_th; // 0~100 ,default : 77
    MFLOAT stabilization_strength;  //0.5~0.95, default:0.9
}EIS_PLUS_TUNING_PARA_STRUCT, *P_EIS_PLUS_TUNING_PARA_STRUCT;

typedef struct
{
    MBOOL en_dejello;
    MFLOAT stabilization_strength;
    MINT32 stabilization_level;
    MFLOAT gyro_still_mv_th;
    MFLOAT gyro_still_mv_diff_th;
}EIS25_TUNING_PARA_STRUCT, *P_EIS25_TUNING_PARA_STRUCT;

typedef struct
{
    MFLOAT stabilization_strength;
    MINT32 stabilization_level;
    MFLOAT gyro_still_mv_th;
    MFLOAT gyro_still_mv_diff_th;
}EIS30_TUNING_PARA_STRUCT, *P_EIS30_TUNING_PARA_STRUCT;

typedef struct
{
	MINT32 ISO;
	MFLOAT exposure_time;
	MFLOAT gain;
}EIS_AE_DATA_STRUCT;

typedef struct
{
	MINT32 flag;
	MINT32 LDC_grid_num_x;
	MINT32 LDC_grid_num_y;
	MFLOAT LDC_grid;
}EIS_LDC_DATA;

typedef struct
{
    EIS_PLUS_TUNING_PARA_STRUCT eis_plus_tuning_data;
    EIS25_TUNING_PARA_STRUCT eis25_tuning_data;
    EIS30_TUNING_PARA_STRUCT eis30_tuning_data;
	EIS_LDC_DATA eis_LDC_data;

    MINT32* wide_angle_lens; //reserved for wide angle lens matrix
    MINT32 MultiScale_width[3];
    MINT32 MultiScale_height[3];
    MINT32 MultiScale_blocksize[3];
    MUINT8 EIS_mode;
    MBOOL  GyroValid;
    MBOOL  Gvalid;
    MBOOL en_gyro_fusion;
    MBOOL debug;
    EIS_PLUS_WARP_PRECISION_ENUM warpPrecision;
    MINT32 MVWidth;
    MINT32 MVHeight;
    MBOOL EIS_standard_en; //previous frame only
    MBOOL EIS_advance_en; //previous and future frame
    MBOOL FSC_en;
    MINT32 FSCSlices;
    MINT32 MVWidth1;
    MINT32 MVHeight1;
    MINT32 MVWidth2;
    MINT32 MVHeight2;
    MINT32 MVWidth3;
    MINT32 MVHeight3;
	MFLOAT FSC_margin;
} EIS_PLUS_SET_ENV_INFO_STRUCT, *P_EIS_PLUS_SET_ENV_INFO_STRUCT;


typedef struct
{
    MUINT16* FE_X;
    MUINT16* FE_Y;
    MUINT16* FE_RES;
    MUINT16* FE_DES;
    MUINT16* FE_X_SUB;
    MUINT16* FE_Y_SUB;
    MUINT8* FE_VALID;
}EIS_PLUS_FE_INFO_STRUCT, *P_EIS_PLUS_FE_INFO_STRUCT;

typedef struct
{
    MUINT16 *fe_cur_info[3];
    MUINT16 *fe_pre_info[3];
}EIS25_FE_INFO_STRUCT, *P_EIS25_FE_INFO_STRUCT;


typedef struct
{
    MINT32 eis_gmv_conf[2];
    MFLOAT eis_gmv[2];
}EIS_PLUS_EIS_INFO_STRUCT, *P_EIS_PLUS_EIS_INFO_STRUCT;

typedef struct
{
    MFLOAT GyroInfo[3];
    MFLOAT AcceInfo[3];
    MUINT8 *gyro_in_mv; //add for GIS
    MUINT8 *valid_gyro_num;// valid gyro number per line
    MUINT8 *fbuf_in_rsc_mv;
    MUINT8 *fbuf_in_rsc_var;
}EIS_PLUS_SENSOR_INFO_STRUCT, *P_EIS_PLUS_SENSOR_INFO_STRUCT;

typedef struct
{
  MUINT16 *fm_fw_idx[3];
  MUINT16 *fm_bw_idx[3];
  MINT32 fm_pre_res[3];
  MINT32 fm_cur_res[3];
}EIS25_FM_INFO_STRUCT, *P_EIS25_FM_INFO_STRUCT;

typedef struct
{
    MFLOAT x;
    MFLOAT y;
} EIS_PLUS_COORDINATE, *P_EIS_PLUS_COORDINATE;

typedef struct
{
    EIS_PLUS_FE_INFO_STRUCT fe_info;
    EIS_PLUS_EIS_INFO_STRUCT eis_info;
    EIS_PLUS_SENSOR_INFO_STRUCT sensor_info;
    EIS25_FE_INFO_STRUCT fe_result; // 20151221 New
    EIS25_FM_INFO_STRUCT fm_result;
	EIS_AE_DATA_STRUCT eis_AE_data;
    MFLOAT block_size;
    MINT32 gyro_block_size;
    MINT32 RSSoWidth;
    MINT32 RSSoHeight;
    MINT32 MVWidth;
    MINT32 MVHeight;
    MINT32 imgiWidth;
    MINT32 imgiHeight;
    MINT32 CRZoWidth;
    MINT32 CRZoHeight;
    MINT32 FSCProcWidth;
    MINT32 FSCProcHeight;
    MINT32 FovAlignWidth; //fov aligned output width
    MINT32 FovAlignHeight;//fov aligned output height
    EIS_PLUS_COORDINATE WarpGrid[4]; //warp's gird for fov aligned area (4 points: x0,y0 x1,y1)
                                     //                                 (          x2,y2 x3,y3)
    MINT32 *FSCScalingFactor;
    MINT32 SRZoWidth;
    MINT32 SRZoHeight;
    MINT32 oWidth;
    MINT32 oHeight;
    MINT32 TargetWidth;
    MINT32 TargetHeight;
    MINT32 cropX;
    MINT32 cropY;
    MUINT64 frame_t;
    MBOOL  mv_idx; //0:wide, 1:tele
    MINT32 RSCLevel;
    //skew
    MFLOAT Trs;
    MINT32 process_mode; // 20151221 New
    MINT32 process_idx; // 20151221 New
    MINT32 ShutterTime;
	MINT32 frame_rate;
}EIS_PLUS_SET_PROC_INFO_STRUCT, *P_EIS_PLUS_SET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32 ext_mem_size; //working buffer size
    MUINT32                 Grid_W;                  // Grid_W = 2;
    MUINT32                 Grid_H;                  // Grid_H = 2;
}EIS_PLUS_GET_PROC_INFO_STRUCT, *P_EIS_PLUS_GET_PROC_INFO_STRUCT;


typedef struct
{
    MINT32*                  GridX;                // Grid X[Grid_W*Grid_H]
    MINT32*                  GridY;                // Grid Y[Grid_W*Grid_H]
    MINT32*                  GridX_standard;                // Grid X[Grid_W*Grid_H]
    MINT32*                  GridY_standard;                // Grid Y[Grid_W*Grid_H]
    MUINT32                  ClipX;    // image offset X
    MUINT32                  ClipY;    // image offset Y
}EIS_PLUS_RESULT_INFO_STRUCT, *P_EIS_PLUS_RESULT_INFO_STRUCT;

class MTKEisPlus
{
public:
    static MTKEisPlus* createInstance();
    virtual void   destroyInstance(MTKEisPlus* obj) = 0;

    virtual ~MTKEisPlus(){};
    // Process Control
    virtual MRESULT EisPlusInit(void* InitInData);
    virtual MRESULT EisPlusMain(EIS_PLUS_RESULT_INFO_STRUCT *EisPlusResult);    // START
    virtual MRESULT EisPlusReset();   //Reset

    // Feature Control
    virtual MRESULT EisPlusFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};


#endif
