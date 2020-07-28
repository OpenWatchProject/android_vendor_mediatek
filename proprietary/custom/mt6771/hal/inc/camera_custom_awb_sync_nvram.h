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
#ifndef _CAMERA_CUSTOM_AWB_SYNC_NVRAM_H_
#define _CAMERA_CUSTOM_AWB_SYNC_NVRAM_H_

#define AWBSYNC_REVERSE_ARRAY_MAX 30

#define CCT_SYNC_LIGHT_SOURCE_NUM 6

#define AWB_SYNC_Y_OFFSET_INDEX_NUM 11

typedef enum
{
    SYNC_AWB_ADV_PP_METHOD,
    SYNC_AWB_BLENDING_METHOD,
    SYNC_AWB_WHITPOINT_MAP_METHOD,
    SYNC_AWB_CCT_METHOD,
    SYNC_AWB_FREE_RUN
 
}eSYNC_AWB_METHOD;

typedef enum
{
    STANDBY_SYNC_AWB_Geometric_Smooth,
    STANDBY_SYNC_AWB_Tempoary_Smooth,
    STANDBY_SYNC_AWB_FREE_RUN
}eSTANDBY_SYNC_AWB_METHOD;

typedef struct
{
    MBOOL PP_method_consider_ccm;
    MUINT32 PP_method_Y_threshold_low;
    MUINT32 PP_method_Y_threshold_high;
    MUINT32 PP_method_valid_block_num_ratio; // %precentage

    MUINT32 PP_method_valid_childblock_percent_th;
    MUINT32 PP_method_diff_percent_th;
    MUINT32 PP_method_enhance_change;

}AWB_SYNC_ADV_PP_METHOD_TUNING_STRUCT;


typedef struct
{
    MUINT32 BlendingTh[4];

}AWB_SYNC_BLEND_METHOD_TUNING_STRUCT;

typedef struct
{
    MUINT32 WP_GR_ratio_th_main[2];  //[0] low bound, [1] high bound
    MUINT32 WP_GB_ratio_th_main[2];

    MUINT32 WP_GR_ratio_th_sub[2];
    MUINT32 WP_GB_ratio_th_sub[2];

    MUINT8  WP_area_hit_ratio;
    MUINT8  WP_PP_gain_ratio_tbl[2];

    MUINT32 WP_Y_th[2];

}AWB_SYNC_WP_METHOD_TUNING_STRUCT;


typedef struct
{
    MBOOL bEnSyncWBSmooth;
    MUINT32 u4SyncAWBSmoothParam[3];

}AWB_SYNC_GAIN_SMOOTH_TUNING_STRUCT;

typedef struct
{
    MINT32 PP_method_Tungsten_prefer[3];
    MINT32 PP_method_WarmFL_prefer[3];
    MINT32 PP_method_FL_prefer[3];
    MINT32 PP_method_CWF_prefer[3];
    MINT32 PP_method_Day_prefer[3];
    MINT32 PP_method_Shade_prefer[3];
    MINT32 PP_method_DayFL_prefer[3];

}AWB_SYNC_GAIN_PREFER_TUNING_STRUCT;

typedef struct
{
    MINT32 PP_method_prefer_LV_TH[2];
    MINT32 PP_method_Tungsten_prefer_LV_R[2];
    MINT32 PP_method_Tungsten_prefer_LV_G[2];
    MINT32 PP_method_Tungsten_prefer_LV_B[2];
    MINT32 PP_method_WarmFL_prefer_LV_R[2];
    MINT32 PP_method_WarmFL_prefer_LV_G[2];
    MINT32 PP_method_WarmFL_prefer_LV_B[2];
    MINT32 PP_method_FL_prefer_LV_R[2];
    MINT32 PP_method_FL_prefer_LV_G[2];
    MINT32 PP_method_FL_prefer_LV_B[2];
    MINT32 PP_method_CWF_prefer_LV_R[2];
    MINT32 PP_method_CWF_prefer_LV_G[2];
    MINT32 PP_method_CWF_prefer_LV_B[2];
    MINT32 PP_method_Day_prefer_LV_R[2];
    MINT32 PP_method_Day_prefer_LV_G[2];
    MINT32 PP_method_Day_prefer_LV_B[2];
    MINT32 PP_method_Shade_prefer_LV_R[2];
    MINT32 PP_method_Shade_prefer_LV_G[2];
    MINT32 PP_method_Shade_prefer_LV_B[2];
    MINT32 PP_method_DayFL_prefer_LV_R[2];
    MINT32 PP_method_DayFL_prefer_LV_G[2];
    MINT32 PP_method_DayFL_prefer_LV_B[2];

}AWB_SYNC_GAIN_PREFER_LVBASE_STRUCT;

typedef struct
{
    MINT32 i4LUT[AWB_LV_INDEX_NUM];
} AWB_SYNC_LV_LUT_T;

typedef struct
{
    MUINT32 Geometric_Smooth_Radius;
    MUINT32 Temporary_Smooth_Speed;
    MUINT32 Smooth_Convergence_th;
}AWB_STANDBY_SYNC_SMOOTH_TUNING_STRUCT;

typedef struct
{
    MUINT32 standbysyncAWBMethod;  //Choose use which kind of sync method
    AWB_STANDBY_SYNC_SMOOTH_TUNING_STRUCT StandbysyncAWB_tuning_param;

}AWB_STANDBY_SYNC_TUNNING_PARAM_STRUCT;

typedef struct
{
    MINT32 i4X;
    MINT32 i4Y;
} AWB_XY_COORDINATE_STRUCT;

typedef struct
{
    MINT32 i4Lut[AWB_SYNC_Y_OFFSET_INDEX_NUM]; // Look-up table
} AWB_SYNC_Y_OFFSET_LUT_T;

typedef struct
{
    AWB_XY_COORDINATE_STRUCT rRotXY[1];
    AWB_XY_COORDINATE_STRUCT rY_Href[3];
    AWB_XY_COORDINATE_STRUCT rY_Lref[3];
    AWB_XY_COORDINATE_STRUCT rCCTRefXY_offset[6];
    AWB_SYNC_Y_OFFSET_LUT_T  rY_offset[CCT_SYNC_LIGHT_SOURCE_NUM];

}AWB_CCT_MAPPING_SYNC_TUNNING_PARAM_STRUCT;


typedef struct
{
    MUINT32 syncAWBMethod;  //Choose use which kind of sync method

    AWB_SYNC_ADV_PP_METHOD_TUNING_STRUCT syncAWB_pp_method_tuning_param;
    AWB_SYNC_BLEND_METHOD_TUNING_STRUCT syncAWB_blend_method_tuning_param;
    AWB_SYNC_WP_METHOD_TUNING_STRUCT syncAWB_wp_method_tuning_param;

    MBOOL isFixMapLocation;
    MUINT32 u4SyncAWB_fov_ratio;

    AWB_SYNC_GAIN_SMOOTH_TUNING_STRUCT syncAWB_smooth_param;
    AWB_SYNC_GAIN_PREFER_TUNING_STRUCT syncAWB_prefer_param;
    AWB_SYNC_GAIN_PREFER_LVBASE_STRUCT syncAWB_prefer_lvbase;

    AWB_SYNC_LV_LUT_T BlendRatio;

    AWB_STANDBY_SYNC_TUNNING_PARAM_STRUCT standbySyncAwbCtrlParam;

    AWB_CCT_MAPPING_SYNC_TUNNING_PARAM_STRUCT CCTMappingSyncParam;

    MINT32 pReserved[AWBSYNC_REVERSE_ARRAY_MAX];

}AWBSYNC_NVRAM_T;

#endif