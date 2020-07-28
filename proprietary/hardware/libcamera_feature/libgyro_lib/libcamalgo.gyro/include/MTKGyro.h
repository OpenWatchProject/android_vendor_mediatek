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
#ifndef _MTK_GYRO_H
#define _MTK_GYRO_H

#include "MTKGyroType.h"
#include "MTKGyroErrCode.h"

#define MAX_FRAME_GYRO (100)
#define N_PARAM (6)

typedef enum
{
    GYRO_STATE_STANDBY,
    GYRO_STATE_INIT,
    GYRO_STATE_PROC,
    GYRO_STATE_FINISH,
    GYRO_STATE_IDLE
}GYRO_STATE_ENUM;

typedef enum
{
    GYRO_FEATURE_BEGIN = 0,

    GYRO_FEATURE_GET_PROC_INFO,
    GYRO_FEATURE_SET_WORK_BUF_INFO,
    GYRO_FEATURE_SET_PROC_INFO,
    GYRO_FEATURE_GET_CAL_RESULT_INFO,
    GYRO_FEATURE_GET_MV_RESULT_INFO,
    GYRO_FEATURE_SET_PTHREAD_ATTR,
    GYRO_FEATURE_SET_CAL_NONACTIVE,
    GYRO_FEATURE_SAVE_EM_INFO,
    GYRO_FEATURE_SAVE_LOG,

    GYRO_FEATURE_MAX

}    GYRO_FEATURE_ENUM;

typedef enum
{
    GYRO_PROC_MODE_CAL = 0,
    GYRO_PROC_MODE_MV,
    GYRO_PROC_MODE_UNKNOWN
} GYRO_PROC_MODE_ENUM;


typedef struct
{
    MINT32 PIXEL_MODE;
    MINT32 EIS_OP_H_step;//EIS_OP_HORI
    MINT32 EIS_OP_V_step;//EIS_OP_VERT
    MINT32 EIS_Left_offset;//EIS_RP_HOFST
    MINT32 EIS_Top_offset;//EIS_RP_VOFST
    MINT32 EIS_H_win_num;//EIS_NUM_HWIN
    MINT32 EIS_V_win_num;//EIS_NUM_VWIN
    MINT32 EIS_H_win_size;//EIS_WIN_HSIZE
    MINT32 EIS_V_win_size; //EIS_WIN_VSIZE
    MINT32 EIS_RP_num;//EIS_DC_DL
    MINT32 EIS_RP_H_num;//EIS_NUM_HRP
    MINT32 EIS_RP_V_num;//EIS_NUM_VRP
    MINT32 GYRO_sample_rate;
} GYRO_EIS_INFO_STRUCT, *P_GYRO_EIS_INFO_STRUCT;

typedef struct
{
    float x;
    float y;
} COORDINATE, *P_COORDINATE;

typedef struct
{
    GYRO_PROC_MODE_ENUM    ProcMode;
    GYRO_EIS_INFO_STRUCT    GyroCalInfo; // for calibration only
    double    *param; //cal: N_PARAM_CANDI * N_PARAM, mv:N_PARAM
    MUINT64    sleep_t;
    MINT32  param_Width;      //nvram size
    MINT32  param_Height;     //nvram size
    MINT32  param_crop_Y;     //nvram size
    MINT32  sensor_Width;     //sensor
    MINT32  sensor_Height;    //sensor
    MINT32  rrz_crop_Width;   //sensor crop
    MINT32  rrz_crop_Height;  //sensor crop
    MINT32  rrz_crop_X;
    MINT32  rrz_crop_Y;
    MINT32  rrz_scale_Width;  //RRZ output
    MINT32  rrz_scale_Height; //RRZ output
    MINT32  crz_crop_Width;   //CRZ output
    MINT32  crz_crop_Height;  //CRZ output
    MINT32  crz_crop_X;
    MINT32  crz_crop_Y;
    MINT32  fov_align_Width; //fov aligned output width
    MINT32  fov_align_Height; //fov aligned output height
    MFLOAT  block_size;
    MINT32  MVWidth;          //Horizontal MV number
    MINT32  MVHeight;         //Vertical MV number
    MBOOL   isBinning;
    MBOOL   debug;
    MBOOL   EMmode;
    MUINT8  eis_mode;
    MINT32  FSCSlices;
    MBOOL   FSC_en;
} GYRO_INIT_INFO_STRUCT, *P_GYRO_INIT_INFO_STRUCT;

typedef struct
{
    MBOOL   cam_idx; //0:wide, 1:tele
    MINT32  sensor_Width;
    MINT32  sensor_Height;
    double* param;
    MBOOL   vHDR_idx; //0:off, 1:on
    MUINT32 frame_LE;
    MUINT32 frame_SE;
    MINT32  rrz_crop_Width;   //sensor crop
    MINT32  rrz_crop_Height;  //sensor crop
    MINT32  rrz_crop_X;
    MINT32  rrz_crop_Y;
    MINT32  rrz_scale_Width;  //RRZ output
    MINT32  rrz_scale_Height; //RRZ output
    MINT32  crz_crop_Width;   //CRZ output
    MINT32  crz_crop_Height;  //CRZ output
    MINT32  crz_crop_X;
    MINT32  crz_crop_Y;
    MINT32  fov_align_Width; //fov aligned output width
    MINT32  fov_align_Height; //fov aligned output height
    MINT32  block_size;
    MINT32  MVWidth;
    MINT32  MVHeight;
    COORDINATE warp_grid[4]; //warp's gird (4 points: x0,y0 x1,y1)
                             //            (          x2,y2 x3,y3)
    MINT32    gyro_num;
    MUINT32   frame_AE;
    MUINT64   frame_t;
    MUINT64   *gyro_t_frame;   //MAX_FRAME_GYRO
    double    *gyro_xyz_frame; //MAX_FRAME_GYRO * 3
    MBOOL     val_LMV; // for calibration only
    MINT32    *EIS_LMV;// for calibration only, MAX_EIS_RP_NUM * 4
    MINT32     EIS_RP_num;
    MINT32    *FSCScalingFactor;
}GYRO_SET_PROC_INFO_STRUCT, *P_GYRO_SET_PROC_INFO_STRUCT;


typedef struct
{
    MUINT32 ext_mem_size; //working buffer size
}GYRO_GET_PROC_INFO_STRUCT, *P_GYRO_GET_PROC_INFO_STRUCT;

typedef struct
{
    void* extMemStartAddr; //working buffer start address
    MUINT32 extMemSize;
} GYRO_SET_WORKING_BUFFER_STRUCT, *P_GYRO_SET_WORKING_BUFFER_STRUCT;

typedef struct
{
    double paramFinal[N_PARAM];
    MUINT32 dataValid;
}GYRO_CAL_RESULT_INFO_STRUCT, *P_GYRO_CAL_RESULT_INFO_STRUCT;

typedef struct
{
    MUINT8 *mv;   //MAX_MV_NUM * 4 (TBD:double buffer?)
    MUINT8 *valid_gyro_num; //MAX_MV_V_NUM ((TBD:double buffer?)
}GYRO_MV_RESULT_INFO_STRUCT, *P_GYRO_MV_RESULT_INFO_STRUCT;


class MTKGyro
{
public:
    static MTKGyro* createInstance();
    virtual void   destroyInstance(MTKGyro* obj) = 0;

    virtual ~MTKGyro(){};
    // Process Control
    virtual MRESULT GyroInit(void* InitInData);
    virtual MRESULT GyroMain();    // START
    virtual MRESULT GyroReset();   //Reset

    // Feature Control
    virtual MRESULT GyroFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
};


#endif


