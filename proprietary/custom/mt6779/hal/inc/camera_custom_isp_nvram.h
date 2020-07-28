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
#ifndef _CAMERA_CUSTOM_NVRAM_ISP_H_
#define _CAMERA_CUSTOM_NVRAM_ISP_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "ispif.h"
#include "CFG_Camera_File_Max_Size.h"
#include "camera_custom_3a_nvram.h"
#include "camera_custom_ae_nvram.h"
#include "camera_custom_awb_nvram.h"
#include "camera_custom_af_nvram.h"
#include "camera_custom_flash_nvram.h"
#include "camera_custom_tsf_tbl.h"
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "camera_custom_tuning_size.h"

using namespace NSIspTuning;

/*******************************************************************************
* ISP NVRAM parameter
********************************************************************************/
#define NVRAM_SUPPORT_SENSOR            (10) //10 sensor mode

#define NVRAM_GGM_TBL_NUM               (5)
#define NVRAM_IHDR_GGM_TBL_NUM          (16)
#define NVRAM_ISP_REGS_ISO_GROUP_NUM    (10)
#define NVRAM_ISP_REGS_ISO_SUPPORT_NUM  (20)
#define NVRAM_ISP_REGS_ZOOM_SUPPORT_NUM  (4)
#define NVRAM_ISP_REGS_LV_SUPPORT_NUM  (6)
#define NVRAM_ISP_REGS_CT_SUPPORT_NUM  (10)

#if 0
#define NVRAM_SL2F_TBL_NUM              (NVRAM_SUPPORT_SENSOR)
#define NVRAM_ADBS_TBL_NUM               (1000)
#define NVRAM_DBS_TBL_NUM               (1000)
#define NVRAM_OBC_TBL_NUM               (200)
#define NVRAM_BPC_TBL_NUM               (1000)
#define NVRAM_NR1_TBL_NUM               (1000)
#define NVRAM_PDC_TBL_NUM               (900)
#define NVRAM_RMM_TBL_NUM               (200)
#define NVRAM_HLR_TBL_NUM               (940)
#define NVRAM_RNR_TBL_NUM               (1000)
#define NVRAM_SL2_TBL_NUM               (44)
#define NVRAM_UDM_TBL_NUM               (1300)
#define NVRAM_ANR_TBL_NUM               (1680)
#define NVRAM_ANR2_TBL_NUM              (1680)
#define NVRAM_CCR_TBL_NUM               (540)
//#define NVRAM_BOK_TBL_NUM               (0)
#define NVRAM_HFG_TBL_NUM               (1460)
#define NVRAM_EE_TBL_NUM                (1460)
#define NVRAM_NR3D_TBL_NUM              (880)
#define NVRAM_MFB_TBL_NUM               (200)
#define NVRAM_MIXER3_TBL_NUM            (180)
//#define NVRAM_CCM_CFC_TBL_NUM           (345)

#define NVRAM_ABF_TBL_NUM               (540)
#endif


// camera common parameters and sensor parameters
typedef struct
{
    UINT32 CommReg[64];
} ISP_NVRAM_COMMON_STRUCT, *PISP_NVRAM_COMMON_STRUCT;

typedef struct ISP_NVRAM_ISO_INTERVAL_STRUCT
{
    MUINT16 u2Length;
    MUINT32 IDX_Partition[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
} ISP_NVRAM_ISO_INTERVAL_T, *PISP_NVRAM_ISO_INTERVAL_T;

typedef struct ISP_NVRAM_ZOOM_INTERVAL_STRUCT
{
    MUINT16 u2Length;
    INT16 IDX_Partition[NVRAM_ISP_REGS_ZOOM_SUPPORT_NUM];
} ISP_NVRAM_ZOOM_INTERVAL_T, *PISP_NVRAM_ZOOM_INTERVAL_T;

typedef struct ISP_NVRAM_LV_INTERVAL_STRUCT
{
    MUINT16 u2Length;
    INT16 IDX_Partition[NVRAM_ISP_REGS_LV_SUPPORT_NUM];
} ISP_NVRAM_LV_INTERVAL_T, *PISP_NVRAM_LV_INTERVAL_T;

typedef struct ISP_NVRAM_CT_INTERVAL_STRUCT
{
    MUINT16 u2Length;
    INT16 IDX_Partition[NVRAM_ISP_REGS_CT_SUPPORT_NUM];
} ISP_NVRAM_CT_INTERVAL_T, *PISP_NVRAM_CT_INTERVAL_T;

// for old flow can build pass
typedef struct
{
    MINT32 x;
} ISP_NVRAM_NBC_LCE_LINK_T, *PISP_NVRAM_NBC_LCE_LINK_T;


//

typedef struct{
    MUINT32 FW_YNR_TC_LINK_STR;
    MUINT32 FW_YNR_TC_LINK_STR_LUMA;
    MUINT32 FW_YNR_CE_LINK_STR;
    MUINT32 FW_YNR_GAIN_CLIP_HI;
    MUINT32 FW_YNR_GAIN_CLIP_LO;
    MUINT32 FW_YNR_GAMMA_SMOOTH;
    MUINT32 FW_YNR_P1_GAIN;
    MUINT32 FW_YNR_P50_GAIN;
    MUINT32 FW_YNR_P250_GAIN;
    MUINT32 FW_YNR_P500_GAIN;
    MUINT32 FW_YNR_P750_GAIN;
    MUINT32 FW_YNR_P950_GAIN;
} ISP_NVRAM_YNR_LCE_LINK_LINK1_T, *PISP_NVRAM_YNR_LCE_LINK1_T;

typedef struct{
    MUINT32 FW_YNR_DCE_LINK_STR;
    MUINT32 FW_YNR_DCE_GAIN_CLIP_HI;
    MUINT32 FW_YNR_DCE_GAIN_CLIP_LO;
    MUINT32 FW_YNR_LINK_BASE_GAIN;
    MUINT32 rsv1;
    MUINT32 rsv2;
} ISP_NVRAM_YNR_LCE_LINK_LINK2_T, *PISP_NVRAM_YNR_LCE_LINK2_T;

typedef struct{
    ISP_NVRAM_YNR_LCE_LINK_LINK1_T LINK1;
    ISP_NVRAM_YNR_LCE_LINK_LINK2_T LINK2;
} ISP_NVRAM_YNR_LCE_LINK_T, *PISP_NVRAM_YNR_LCE_LINK_T;

/////

typedef struct{
    MUINT32 YNR_FD_TBL_EN;
    MUINT32 YNR_FD_TBL_M_GAIN;
    MUINT32 YNR_FD_TBL_F_GAIN;
    MUINT32 YNR_FD_TBL_Y_GAIN;
    MUINT32 YNR_FD_TBL_U_GAIN;
    MUINT32 YNR_FD_TBL_V_GAIN;
    MUINT32 YNR_FD_TBL_Y_Range;
    MUINT32 YNR_FD_TBL_U_Range;
    MUINT32 YNR_FD_TBL_V_Range;
} ISP_NVRAM_YNR_FD_TBL_T, *PISP_NVRAM_YNR_FD_TBL_T;

typedef struct{
    MUINT32 YNR_FD_MAP_EN;
    MUINT32 YNR_FD_MAP_MAX_GAIN;
    MUINT32 YNR_FD_MAP_M_GAIN;
    MUINT32 YNR_FD_MAP_F_GAIN;
    MUINT32 YNR_FD_MAP_W_R;
    MUINT32 YNR_FD_MAP_H_R;
    MUINT32 YNR_FD_MAP_DOWN;
    MUINT32 YNR_FD_MAP_Y_RANGE;
    MUINT32 YNR_FD_MAP_U_RANGE;
    MUINT32 YNR_FD_MAP_V_RANGE;
} ISP_NVRAM_YNR_FD_MAP1_T, *PISP_NVRAM_YNR_FD_MAP1_T;

typedef struct{
    MUINT32 rsv1;
    MUINT32 rsv2;
    MUINT32 rsv3;
    MUINT32 rsv4;
    MUINT32 rsv5;
    MUINT32 rsv6;
    MUINT32 rsv7;
    MUINT32 rsv8;
    MUINT32 rsv9;
    MUINT32 rsv10;
    MUINT32 rsv11;
    MUINT32 rsv12;
    MUINT32 rsv13;
    MUINT32 rsv14;
    MUINT32 rsv15;
    MUINT32 rsv16;
    MUINT32 rsv17;
    MUINT32 rsv18;
    MUINT32 rsv19;
    MUINT32 rsv20;
    MUINT32 rsv21;
    MUINT32 rsv22;
    MUINT32 rsv23;
    MUINT32 rsv24;
    MUINT32 rsv25;
    MUINT32 rsv26;
} ISP_NVRAM_YNR_FD_MAP2_T, *PISP_NVRAM_YNR_FD_MAP2_T;

typedef struct{
    ISP_NVRAM_YNR_FD_TBL_T TBL;
    ISP_NVRAM_YNR_FD_MAP1_T MAP1;
    ISP_NVRAM_YNR_FD_MAP2_T MAP2;
} ISP_NVRAM_YNR_FD_T, *PISP_NVRAM_YNR_FD_T;

typedef struct
{
    ISP_NVRAM_ISO_INTERVAL_T    Iso_Env[NVRAM_ISP_REGS_ISO_GROUP_NUM];
    ISP_NVRAM_ZOOM_INTERVAL_T   Zoom_Env;
    ISP_NVRAM_LV_INTERVAL_T     Lv_Env;
    ISP_NVRAM_CT_INTERVAL_T     Ct_Env;
    ISP_NVRAM_SLK_T             SLK[NVRAM_SLK_TBL_NUM];
    ISP_NVRAM_DM_T              DM[NVRAM_DM_TBL_NUM];
    ISP_NVRAM_YNR_T             YNR[NVRAM_YNR_TBL_NUM];
    ISP_NVRAM_YNR_LCE_LINK_T    YNR_LCE_LINK[NVRAM_YNR_LCE_LINK_TBL_NUM];
    ISP_NVRAM_YNR_FD_T          YNR_FD[NVRAM_YNR_FD_TBL_NUM];
    ISP_NVRAM_CNR_CNR_T         CNR_CNR[NVRAM_CNR_CNR_TBL_NUM];
    ISP_NVRAM_EE_T              EE[NVRAM_EE_TBL_NUM];
    ISP_NVRAM_NR3D_T            NR3D[NVRAM_NR3D_TBL_NUM];
    ISP_NVRAM_OBC_T             OBC[NVRAM_OBC_TBL_NUM];
    ISP_NVRAM_BPC_BPC_T         BPC_BPC[NVRAM_BPC_BPC_TBL_NUM];
    ISP_NVRAM_BPC_CT_T          BPC_CT[NVRAM_BPC_CT_TBL_NUM];
    ISP_NVRAM_BPC_PDC_T         BPC_PDC[NVRAM_BPC_PDC_TBL_NUM];
    ISP_NVRAM_LDNR_T            LDNR[NVRAM_LDNR_TBL_NUM];
    ISP_NVRAM_CNR_CCR_T         CNR_CCR[NVRAM_CNR_CCR_TBL_NUM];
    ISP_NVRAM_CNR_ABF_T         CNR_ABF[NVRAM_CNR_ABF_TBL_NUM];
    ISP_NVRAM_LPCNR_T           LPCNR[NVRAM_LPCNR_TBL_NUM];
    ISP_NVRAM_FUS_T             FUS[NVRAM_FUS_TBL_NUM];
    ISP_NVRAM_ZFUS_T            ZFUS[NVRAM_ZFUS_TBL_NUM];
    ISP_NVRAM_MFB_T             MFB[NVRAM_MFB_TBL_NUM];
    ISP_NVRAM_MIX_T             MIX[NVRAM_MIX_TBL_NUM];
    ISP_NVRAM_YNR_LUT_T         YNR_TBL[NVRAM_YNR_TBL_TBL_NUM];
    ISP_NVRAM_YNRS_T            YNRS[NVRAM_YNRS_TBL_NUM];
} ISP_NVRAM_REGISTER_STRUCT, *PISP_NVRAM_REGISTER_STRUCT;


/*
*   LTM NVRAM Param
*/

enum ltm_mode_enum {
    LTM_MODE_LINEAR = 0,
    LTM_MODE_GLOBAL,
    LTM_MODE_LOCAL
};

struct ltm_mid_perc_lut {
    MUINT32 size;
    MUINT32 x[10];
};

struct ltm_local_global_blending {
    MUINT32 x[2];
    MUINT32 ratio[2];
    MUINT32 tp_ratio[14];
};

struct ltm_smooth_speed_type {
    MUINT32 ae_stable_speed;
    MUINT32 same_direction_speed;
    MUINT32 reverse_direction_speed;
    MUINT32 other_speed;
};

typedef struct {
    enum ltm_mode_enum local_tone_mode;

    // Global Tone Metering
    MUINT32 clipping_thd_perc;
    MUINT32 high_bound_perc;
    struct ltm_mid_perc_lut mid_perc_lut;

    // fix Global Tone
    MUINT32 fix_clipping_thd;
    MUINT32 fix_turning_point;
    MUINT32 fix_gtc_blending_ratio;

    // Local Tone Metering
    MUINT32 strength_lct[14];
    MUINT32 strength_ltp_mean_idx[2];
    MUINT32 strength_ltp[2][14];

    // Face Tone Metering
    bool ltm_face_protection_enable;
    bool multi_face_protection_enable;
    MUINT32 non_robust_face_turning_point_reduction_ratio;
    MUINT32 face_ratio_low_bound;
    MUINT32 face_ratio_high_bound;
	MUINT32 face_protection_strength[14];

    // Temporal Smooth
    struct ltm_smooth_speed_type smooth_speed;
    struct ltm_smooth_speed_type face_smooth_speed;
    MUINT32 delta_cwr_threshold;
    MUINT32 max_index_count_threshold;

    // spatial smooth
    bool spatial_smooth_enable;
	MUINT32 spatial_smooth_strength_table_x[2];
	MUINT32 spatial_smooth_strength_table_y[2][14];
	MUINT32 spatial_smooth_diff_table_x[4];
	MUINT32 spatial_smooth_var_table_x[4];
	MUINT32 spatial_smooth_var_table_y[4];


    // Face spatial smooth
    bool ltm_face_spatial_smooth_enable;
    MUINT32 ltm_b2f_turning_point_diff_spatial_smooth_weighting_table[4][14];
    MUINT32 ltm_b2f_distance_reduction_ratio[2];
	MUINT32 ltm_f2b_turning_point_diff_spatial_smooth_weighting_table[4][14];
	MUINT32 ltm_f2b_distance_reduction_ratio[2];

    // Face temporal smooth
    bool ltm_face_state_link_enable;
    // Local & Global Tone Blending
    struct ltm_local_global_blending local_global_blending;
    // LTM param
    MUINT32 ltm_param_reserve[6][14];
}ISP_AUTO_LTM_TUNING_T;


typedef struct {
    ISP_AUTO_LTM_TUNING_T  rAutoLTMParam;
}ISP_NVRAM_LTM_TUNING_PARAM_T;


enum ISP_HLR_Mode_enum {
    HLR_MODE_OFF = 0,
    HLR_MODE_ON
};
struct ISP_HLR_Default {
	MUINT32 hlr_sat_0;
	MUINT32 hlr_sat_1;
	MUINT32 hlr_sat_2;
	MUINT32 hlr_sat_3;
	MUINT32 hlr_sat_4;
	MUINT32 hlr_sat_5;
	MUINT32 hlr_sat_6;
	MUINT32 hlr_sat_7;
	MUINT32 hlr_node_0;
	MUINT32 hlr_node_1;
	MUINT32 hlr_node_2;
	MUINT32 hlr_node_3;
	MUINT32 hlr_node_4;
	MUINT32 hlr_node_5;
	MUINT32 hlr_node_6;
	MUINT32 hlr_node_7;
	MUINT32 hlr_bld_high_swo;
	MUINT32 hlr_bld_low_swo;
	MUINT32 hlr_prt_high_swo;
	MUINT32 hlr_prt_low_swo;
	MUINT32 hlr_prt_en_swo;
	MUINT32 hlr_slp_0;
	MUINT32 hlr_slp_1;
	MUINT32 hlr_slp_2;
	MUINT32 hlr_slp_3;
	MUINT32 hlr_slp_4;
	MUINT32 hlr_slp_5;
	MUINT32 hlr_max_rat;
	MUINT32 hlr_blue_prt_str;
	MUINT32 hlr_red_prt_str;
	MUINT32 hlr_slp_6;
	MUINT32 hlr_bld_fg;
	MUINT32 hlr_bld_high;
	MUINT32 hlr_bld_slp;
	MUINT32 hlr_prt_th;
	MUINT32 hlr_prt_slp;
	MUINT32 hlr_prt_en;
	MUINT32 hlr_clip_val;
	MUINT32 hlr_off_clip_val;
	MUINT32 hlr_efct_on;
	MUINT32 hlr_tile_edge;
	MUINT32 hlr_tdr_wd;
	MUINT32 hlr_tdr_ht;
	MUINT32 hlr_r1_en;
	MUINT32 hlr_d1_en;
	MUINT32 hlr_r1_lkmsb;
	MUINT32 hlr_d1_lkmsb;
	MUINT32 hlr_r1_lkmode;
	MUINT32 hlr_d1_lkmode;
	MUINT32 hlr_resrv_a;
};

typedef struct {
    MUINT32 lv[4];
    MUINT32 cct[3];
	enum ISP_HLR_Mode_enum hlr_mode;
	struct ISP_HLR_Default hlr_default;
	MUINT32 ISP_HLR_LUT[3][4][3];
	MUINT32 ISP_HLR_Reserv_LUT[4][3];
    MUINT32 ISP_Reserv0;
    MUINT32 ISP_Reserv1;
    MUINT32 ISP_Reserv2;
    MUINT32 ISP_Reserv3;
    MUINT32 ISP_Reserv4;
    MUINT32 ISP_Reserv5;
    MUINT32 ISP_Reserv6;
    MUINT32 ISP_Reserv7;
} ISP_AUTO_HLR_TUNING_T;

typedef struct {
    ISP_AUTO_HLR_TUNING_T  rAutoHLRParam;
}ISP_NVRAM_HLR_TUNING_PARAM_T;

/*
*   GMA NVRAM Param
*/

//#define ISP_NVRAM_GMA_NUM                               (12)     // Normal/HDRon/AutoHDR preview/capture/video + 3 reserved
#define ISP_NVRAM_GMA_CONTRAST_WEIGHTING_TBL_NUM        (11)
#define ISP_NVRAM_GMA_LV_WEIGHTING_TBL_NUM              (20)

typedef enum {
    eISP_FIXED_GMA_MODE = 0,
    eISP_DYNAMIC_GMA_MODE
} eISP_GMA_MODE_T;


typedef struct {
    MINT32 i4HdrContrastWtTbl[ISP_NVRAM_GMA_CONTRAST_WEIGHTING_TBL_NUM];
    MINT32 i4HdrLVWtTbl[ISP_NVRAM_GMA_LV_WEIGHTING_TBL_NUM];
    MINT32 i4NightContrastWtTbl[ISP_NVRAM_GMA_CONTRAST_WEIGHTING_TBL_NUM];
    MINT32 i4NightLVWtTbl[ISP_NVRAM_GMA_LV_WEIGHTING_TBL_NUM];
} ISP_NVRAM_GMA_TUNING_LUT_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4CenterPt;
    MINT32 i4LowPercent;
    MINT32 i4LowCurve100;
    MINT32 i4HighCurve100;
    MINT32 i4HighPercent;
    MINT32 i4SlopeH100;
    MINT32 i4SlopeL100;
} ISP_NVRAM_GMA_TUNING_CURVE_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4WaitAEStable;
    MINT32 i4Speed;     // 0 ~ 10
} ISP_NVRAM_GMA_TUNING_SMOOTH_T;

typedef struct {
    MINT32 i4Enable;
} ISP_NVRAM_GMA_TUNING_FLARE_T;


typedef struct {
    eISP_GMA_MODE_T i4GMAMode;  // 0: Fixed Gamma  1: Dynamic Gamma 2: Adaptive Gamma
    MINT32 i4LowContrastThr;
    MINT32 i4LowContrastRatio;
    MINT32 i4LowContrastSeg;
    ISP_NVRAM_GMA_TUNING_LUT_T rGMALUTs;
    ISP_NVRAM_GMA_TUNING_SMOOTH_T rGMASmooth;
    ISP_NVRAM_GMA_TUNING_CURVE_T rGMACurve;
    ISP_NVRAM_GMA_TUNING_FLARE_T rGMAFlare;
} ISP_NVRAM_GMA_TUNING_PARAM_T;

/*
*   LCE NVRAM Param
*/
//#define ISP_NVRAM_LCE_NUM                               (12)     // Normal/HDRon/AutoHDR x Preview/Capture/Video + 3 reversed
#define ISP_NVRAM_LCE_LV_TBL_NUM                        (19)
#define ISP_NVRAM_LCE_CONTRAST_TBL_NUM                  (11)
#define ISP_NVRAM_LCE_DETAIL_TBL_NUM                    (11)
#define ISP_NVRAM_LCE_DIFF_RANGE_BIN_NUM                (12)

#if 0
typedef struct {
    MINT32 i4vTbl1[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl2[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl3[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4vTbl4[ISP_NVRAM_LCE_CONTRAST_TBL_NUM][ISP_NVRAM_LCE_LV_TBL_NUM];
    MINT32 i4DetailWtTbl[ISP_NVRAM_LCE_DETAIL_TBL_NUM];
    MINT32 i4CenSlopeEnhance[ISP_NVRAM_LCE_DIFF_RANGE_BIN_NUM];
} ISP_NVRAM_LCE_TUNING_LUT_T;

typedef struct {
    MINT32 i4Enable;
    MINT32 i4WaitAEStable;
    MINT32 i4Speed;     // 0 ~ 10
} ISP_NVRAM_LCE_TUNING_SMOOTH_T;

typedef struct {
    MINT32 i4Enable;
} ISP_NVRAM_LCE_TUNING_FLARE_T;

typedef struct {
    MINT32 i4HlrRatio;
    MINT32 i4HlrKeepRatio;
    MINT32 i4HlrSMin;
    MINT32 i4HlrScarRatio;
    MINT32 i4HlrFixEn;
} ISP_NVRAM_LCE_HLR_TUNING_T;


typedef struct {
    MINT32 i4HistCCLB;
    MINT32 i4HistCCUB;

    MINT32 i4LCESeg;
    MINT32 i4LCEContrastRatio;
    MINT32 i4LCEHistYThr;
    MINT32 i4SlopeLow;
    MINT32 i4SlopeHigh;
    MINT32 i4SlopeTop;
    MINT32 i4DetailRangeRatio;
    MINT32 i4CenSlopeMin;
    MINT32 i4vBlacKeep;
    MINT32 i4vFixedPt1;
    MINT32 i4vFixedPt2;
    MINT32 i4vFixedPt3;
    MINT32 i4vFixedPt4;
    MINT32 i4TCPLB;
    MINT32 i4TCPUB;

    MINT32 i4Reserved0;
    MINT32 i4Reserved1;
    MINT32 i4Reserved2;
    MINT32 i4Reserved3;
    MINT32 i4Reserved4;

    ISP_NVRAM_LCE_TUNING_LUT_T rLCELUTs;
    ISP_NVRAM_LCE_TUNING_SMOOTH_T rLCESmooth;
    ISP_NVRAM_LCE_TUNING_FLARE_T rLCEFlare;
    ISP_NVRAM_LCE_HLR_TUNING_T rLCEHlrTbl;
}ISP_AUTO_LCE_TUNING_T;
#endif

#if 1

#define LCE_BASIC_TBL_NUM                               (4)
#define LCE_LV_TBL_NUM                                  (19)
#define LCE_CONTRAST_TBL_NUM                            (11)
#define LCE_DARK_STD_TBL_NUM                            (19)
#define LCE_BRIGHT_STD_TBL_NUM                          (5)
#define LCE_DIFF_RANGE_BIN_NUM                          (12)

typedef enum {
    MTKLCE_FACE_PROTECT_KEEP_LIGHT_MODE,
    MTKLCE_FACE_PROTECT_KEEP_DARK_MODE,
    MTKLCE_FACE_PROTECT_MODE_OFF
} eISP_LCE_FACE_PROTECT_MODE_T;

typedef struct {
    MINT32 i4BaseStrLV[LCE_BASIC_TBL_NUM];
    MINT32 i4DarkStrRatio[LCE_BASIC_TBL_NUM];
    MINT32 i4BrightStrRatio[LCE_BASIC_TBL_NUM];
    MINT32 i4LVTarget[LCE_LV_TBL_NUM];
    MINT32 i4MaxLCEGain[LCE_LV_TBL_NUM];
    MINT32 i4MaxFinalTarget[LCE_LV_TBL_NUM];
}  ISP_LCE_TUNING_BASIC_T;

typedef struct {
    MINT32 i4DarkSTDTbl[LCE_CONTRAST_TBL_NUM][LCE_DARK_STD_TBL_NUM];
    MINT32 i4BrightSTDTbl[LCE_CONTRAST_TBL_NUM][LCE_BRIGHT_STD_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];
} ISP_LCE_TUNING_LUT_T;


typedef struct {
    MINT32 u4FlatAreaPercen;
    MINT32 u4FlatRatioTH;
    MBOOL bDarkSkyProtectEnable;
    MINT32 i4DarkFlatPercen;
    MINT32 i4DarkFlatY_L;
    MINT32 i4DarkFlatY_H;
    MINT32 i4FlatLV_L;
    MINT32 i4FlatLV_H;

    MINT32 i4RsvPara0;
    MINT32 i4RsvPara1;
    MINT32 i4RsvPara2;
    MINT32 i4RsvPara3;
    MINT32 i4RsvPara4;
    MINT32 i4RsvPara5;
    MINT32 i4RsvPara6;
    MINT32 i4RsvPara7;
    MINT32 i4RsvPara8;
    MINT32 i4RsvPara9;

    MINT32 i4LVBriRatio[LCE_LV_TBL_NUM];
    MINT32 i4LVBriLimit[LCE_LV_TBL_NUM];
    MINT32 i4FlatLumaLoBound[LCE_LV_TBL_NUM];
    MINT32 i4FlatLumaHiBound[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl1[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl2[LCE_LV_TBL_NUM];
    MINT32 i4LCEParaTbl3[LCE_LV_TBL_NUM];

}  ISP_LCE_TUNING_PARA_T;

typedef struct {
    MBOOL  bFLceEnable;
    MINT32 i4LoBoundRatio;
    MINT32 i4HiBoundRatio;
    MINT32 i4BrightTH;
    MINT32 i4DarkFaceLimit;
    MINT32 i4FrontLightTH_L;
    MINT32 i4FrontLightTH_H;
    MINT32 i4DarkCurveBlendRatio;
    MBOOL bMultiSlopeEnable;
    MINT32 i4SlopeHigh0;
    MINT32 i4SlopeHigh1;
    MINT32 i4SlopeHigh2;
    MINT32 i4SlopeHigh3;
    MINT32 i4SlopeHigh4;
    MINT32 i4SlopeLow;
    MINT32 i4DarkFaceRatio;
    MINT32 i4FDWidthCropRat;
    MINT32 i4FDHeightCropRat;
    MINT32 i4RsvFace0;
    MINT32 i4RsvFace1;
    MINT32 i4RsvFace2;
    MINT32 i4RsvFace3;
    MINT32 i4RsvFace4;
    MINT32 i4RsvFace5;
    MINT32 i4RsvFace6;
    MINT32 i4RsvFace7;
    MINT32 i4RsvFace8;
    MINT32 i4RsvFace9;
    MINT32 i4LCEFaceTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCEFaceTbl1[LCE_LV_TBL_NUM];
    MINT32 i4CenSlopeEnhance[LCE_DIFF_RANGE_BIN_NUM];
    eISP_LCE_FACE_PROTECT_MODE_T eFaceLCEProtectMode;
    MBOOL bFaceLCESmoothLinkEnable;
    MUINT32 u4RobustFaceSlowSpeedCnt;
    MINT32 i4RobustFaceSmoothSpeed;
    MINT32 i4RobustFaceSmoothSlowSpeed;
    MINT32 bNonRobustFaceLockLCE;
    MUINT32 u4FaceLCEStableMaxCnt;
    MINT32 i4FaceLCEInStableThd;
    MINT32 i4FaceLCEOutStableThd;
}  ISP_LCE_TUNING_FACE_T;

typedef struct {
    MBOOL bSmoothEnable;
    MINT32 i4LCEPosSpeed;
    MINT32 i4LCENegSpeed;
    MINT32 i4LCECrosSpeed;
    MINT32 i4LCESpeed4AEStable;     // 0 ~ 10
    MINT32 i4RsvSmooth0;
    MINT32 i4RsvSmooth1;
    MINT32 i4RsvSmooth2;
    MINT32 i4RsvSmooth3;
} ISP_LCE_TUNING_SMOOTH_T;

typedef struct {
    MBOOL bLCEcurrGainEnable;
    MINT32 i4LCECurrGainRatio;
    MINT32 i4RsvCustom0;
    MINT32 i4RsvCustom1;
    MINT32 i4RsvCustom2;
    MINT32 i4RsvCustom3;
    MINT32 i4RsvCustom4;
    MINT32 i4RsvCustom5;
    MINT32 i4RsvCustom6;
    MINT32 i4RsvCustom7;
    MINT32 i4RsvCustom8;
    MINT32 i4RsvCustom9;
    MINT32 i4LCECustomTbl0[LCE_LV_TBL_NUM];
    MINT32 i4LCECustomTbl1[LCE_LV_TBL_NUM];
    MINT32 i4LCECustomTbl2[LCE_LV_TBL_NUM];
    MINT32 i4LCECustomTbl3[LCE_LV_TBL_NUM];
} ISP_LCE_TUNING_CUSTOM_T;

typedef struct {
    MINT32 i4HistCCLB;
    MINT32 i4HistCCUB;
    MINT32 i4GCEratio;
    MINT32 i4TCPLB;
    MINT32 i4TCPUB;
    MINT32 i4Version;
    MINT32 i4DetailRangeRatio;
    MINT32 i4CenSlopeMin;

    MINT32 i4LowYRatio;
    MINT32 i4HighYRatio;
    MBOOL bDRCompensateEnable;
    MINT32 i4CompHighYRatio;
    MINT32 i4DFRatio_max;
    MINT32 i4DFRatio_min;
    MINT32 i4FlatRatioBound;

    MBOOL bMultiSlopeEnable;
    MINT32 i4SlopPoint0;
    MINT32 i4SlopPoint1;
    MINT32 i4SlopPoint2;
    MINT32 i4SlopPoint3;
    MINT32 i4SlopPoint4;
    MINT32 i4SlopeLow;

    MINT32 i4SlopeHigh0;
    MINT32 i4SlopeHigh1;
    MINT32 i4SlopeHigh2;
    MINT32 i4SlopeHigh3;
    MINT32 i4SlopeHigh4;

    MINT32 i4RsvStats0;
    MINT32 i4RsvStats1;
    MINT32 i4RsvStats2;
    MINT32 i4RsvStats3;
    MINT32 i4RsvStats4;
}  ISP_LCE_STATS_T;

typedef struct {
    ISP_LCE_TUNING_BASIC_T rLCEBasic;
    ISP_LCE_TUNING_LUT_T rLCELUTs;
    ISP_LCE_TUNING_PARA_T rLCEPara;
    ISP_LCE_TUNING_FACE_T rFaceLCE;
    ISP_LCE_TUNING_SMOOTH_T rLCESmooth;
    ISP_LCE_TUNING_CUSTOM_T rLCECustom;
    ISP_LCE_STATS_T rLCEStats;
} ISP_AUTO_LCE_TUNING_T;


typedef struct {
    //MINT32 i4AutoHDREnable;
    ISP_AUTO_LCE_TUNING_T rAutoLCEParam;
} ISP_NVRAM_LCE_TUNING_PARAM_T;

#define DCE_LV_TBL_NUM                                  (19)
#define DCE_CONTRAST_TBL_NUM                            (11)
#define GGM_TBL_NUM_ALGO (4)
struct ISP_DCE_TUNING_LUT
{
    int32_t dce_tbl1[DCE_CONTRAST_TBL_NUM][DCE_LV_TBL_NUM];
    int32_t dce_tbl2[DCE_CONTRAST_TBL_NUM][DCE_LV_TBL_NUM];
    int32_t dce_tbl3[DCE_CONTRAST_TBL_NUM][DCE_LV_TBL_NUM];
};

struct ISP_DCE_STATS
{
    int32_t dce_stats0;
    int32_t dce_stats1;
    int32_t dce_stats2;
    int32_t dce_stats3;
    int32_t dce_stats4;
};


struct ISP_DCE_RESERVE_PARAM
{
    uint32_t dce_rsv0;
    uint32_t dce_rsv1;
    uint32_t dce_rsv2;
    uint32_t dce_rsv3;
    uint32_t dce_rsv4;
    uint32_t dce_rsv5;
    uint32_t dce_rsv6;
    uint32_t dce_rsv7;
    uint32_t dce_rsv8;
    uint32_t dce_rsv9;
    uint32_t dce_rsv_tbl[2][DCE_LV_TBL_NUM];
};

typedef struct {
    uint32_t smooth_speed;
    uint32_t flat_ratio_high;
    uint32_t flat_ratio_low;
    bool flt_prot;
    uint32_t max_slope;
    struct ISP_DCE_STATS dce_stats_p;
    struct ISP_DCE_TUNING_LUT dce_tuning_param_p;
    struct ISP_DCE_RESERVE_PARAM dce_reserve_param_p;
} ISP_AUTO_DCE_TUNING_T;

typedef struct {
    ISP_AUTO_DCE_TUNING_T rDceParam;
} ISP_NVRAM_DCE_TUNING_PARAM_T;

typedef struct{
    ISP_NVRAM_GGM_T                 GGM_Reg[GGM_TBL_NUM_ALGO];
    ISP_NVRAM_GMA_TUNING_PARAM_T    rGmaParam;
} ISP_NVRAM_GMA_STRUCT_T, *PISP_NVRAM_GMA_STRUCT_T;

typedef struct{
    ISP_NVRAM_GMA_STRUCT_T          GMA[ISP_NVRAM_GMA_TBL_NUM];
    ISP_NVRAM_LCE_TUNING_PARAM_T    LCE[ISP_NVRAM_LCE_TBL_NUM];
    ISP_NVRAM_DCE_TUNING_PARAM_T    DCE[ISP_NVRAM_DCE_TBL_NUM];
    ISP_NVRAM_LTM_TUNING_PARAM_T    LTM[ISP_NVRAM_LTM_TBL_NUM];
	ISP_NVRAM_HLR_TUNING_PARAM_T    HLR[ISP_NVRAM_HLR_TBL_NUM];
}ISP_NVRAM_TONEMAP_PARAM_T;

#else
// for old flow can build pass
typedef struct{
    ISP_NVRAM_GGM_T                 GGM_Reg;
    ISP_NVRAM_GMA_TUNING_PARAM_T    rGmaParam;
} ISP_NVRAM_GMA_STRUCT_T, *PISP_NVRAM_GMA_STRUCT_T;

typedef struct {
    MINT32 x;
} ISP_NVRAM_DCE_TUNING_PARAM_T;
//
typedef struct {
    MINT32 i4AutoHDREnable;
    ISP_AUTO_LCE_TUNING_T rAutoLCEParam[ISP_NVRAM_LCE_NUM];
} ISP_NVRAM_LCE_TUNING_PARAM_T;

typedef struct{
    ISP_NVRAM_GGM_T                 GGM[NVRAM_GGM_TBL_NUM];
    ISP_NVRAM_GGM_T                 IHDR_GGM[NVRAM_IHDR_GGM_TBL_NUM];
    ISP_NVRAM_GMA_TUNING_PARAM_T    rGmaParam[ISP_NVRAM_GMA_NUM];
    ISP_NVRAM_LCE_TUNING_PARAM_T    rLceParam;
}ISP_NVRAM_TONEMAP_PARAM_T;

#endif

/*
*   CCM NVRAM Param
*/
#define ISP_NVRAM_CCM_CT_NUM                            (8)
#define ISP_NVRAM_CCM_LV_NUM                            (6)
#define ISP_NVRAM_CCM_FLASH_NUM                         (2)
#define ISP_NVRAM_DYNAMIC_CCM_NUM                       (4)

typedef struct
{
    MINT32 i4R_AVG;
    MINT32 i4R_STD;
    MINT32 i4B_AVG;
    MINT32 i4B_STD;
    MINT32 i4R_MAX; // default = (1<<(M+N)) - 1; M = 3, N = 9
    MINT32 i4R_MIN; // default = 1<<N; M = 3, N = 9
    MINT32 i4G_MAX; // default = (1<<(M+N)) - 1; M = 3, N = 9
    MINT32 i4G_MIN; // default = 1<<N; M = 3, N = 9
    MINT32 i4B_MAX; // default = (1<<(M+N)) - 1; M = 3, N = 9
    MINT32 i4B_MIN; // default = 1<<N; M = 3, N = 9
    MINT32 i4P00[9];
    MINT32 i4P10[9];
    MINT32 i4P01[9];
    MINT32 i4P20[9];
    MINT32 i4P11[9];
    MINT32 i4P02[9];
} ISP_NVRAM_CCM_POLY22_STRUCT;


#define ISP_NVRAM_COLOR_SCENE_NUM                         (4)
#define ISP_NVRAM_COLOR_CT_NUM                            (5)

typedef struct{
    MUINT32 u4Ratio_Max; //(0~64, 1x=16)
    MUINT32 u4Ratio_TH; //(0~64, 1x=16)
    MUINT32 u4Ratio_Min; //(0~64, 1x=16)
    MUINT32 u4LV_THA; //(0~255)
    MUINT32 u4LV_THB; //(0~255)
    MUINT32 u4LV_THC; //(0~255)
    MUINT32 u4LV_THD; //(0~255)
    MUINT32 u4DC_Offset; //(0~255, 0x~2x)
    MUINT32 u4Lum_Speed; // 0~32
    MUINT32 u4Hue_Speed; // 0~32
    MUINT32 u4Sat_Speed; // 0~32
    MUINT32 u4LSP_LV_TH; //(0~255)
    MUINT32 u4Outdoor_EN; // 1: on, 0: off
    MUINT32 u4Outdoor_THA; //(0~255)
    MUINT32 u4Outdoor_THB; //(0~255)
    MUINT32 u4Outdoor_Speed; // 0~32
    MUINT32 u4Outdoor_Ratio; // default is 16
} ISP_NVRAM_PCA_PARAM_T;


typedef struct{
    MUINT32 u4FD_CM_Enable;
    MUINT32 u4Priority;
    MUINT32 u4Range_W1_Y;
    MUINT32 u4Range_W1_S;
    MUINT32 u4Range_W1_H;
    MUINT32 u4Offset_W1_Y;
    MUINT32 u4Offset_W1_S;
    MUINT32 u4Offset_W1_H;
    MUINT32 u4Range_W2_Y;
    MUINT32 u4Range_W2_S;
    MUINT32 u4Range_W2_H;
    MUINT32 u4Offset_W2_Y;
    MUINT32 u4Offset_W2_S;
    MUINT32 u4Offset_W2_H;
    MUINT32 u4Range_W3_Y;
    MUINT32 u4Range_W3_S;
    MUINT32 u4Range_W3_H;
    MUINT32 u4Offset_W3_Y;
    MUINT32 u4Offset_W3_S;
    MUINT32 u4Offset_W3_H;
    MUINT32 u4Backup_0;
    MUINT32 u4Backup_1;
    MUINT32 u4Backup_2;
    MUINT32 u4Backup_3;
    MUINT32 u4Backup_4;
    MUINT32 u4Backup_5;
    MUINT32 u4Backup_6;
    MUINT32 u4Backup_7;
    MUINT32 u4Backup_8;
    MUINT32 u4Backup_9;
    MUINT32 u4Backup_10;
    MUINT32 u4Backup_11;
    MUINT32 u4Backup_12;
    MUINT32 u4Backup_13;
    MUINT32 u4Backup_14;
    MUINT32 u4Backup_15;
    MUINT32 u4Backup_16;
    MUINT32 u4Backup_17;
    MUINT32 u4Backup_18;
    MUINT32 u4Backup_19;
    MUINT32 u4Backup_20;
    MUINT32 u4Backup_21;
    MUINT32 u4Backup_22;
    MUINT32 u4Backup_23;
} ISP_NVRAM_FD_COLOR_PARAM_T;

typedef struct{
    MUINT32 u4FD_ANR_LUT_Enable;
    MUINT32 u4Range_W1_Y;
    MUINT32 u4Range_W1_U;
    MUINT32 u4Range_W1_V;
    MUINT32 u4W1_enable;
    MUINT32 u4Gain_W1_Y;
    MUINT32 u4Gain_W1_U;
    MUINT32 u4Gain_W1_V;
    MUINT32 u4Range_W2_Y;
    MUINT32 u4Range_W2_U;
    MUINT32 u4Range_W2_V;
    MUINT32 u4W2_enable;
    MUINT32 u4Gain_W2_Y;
    MUINT32 u4Gain_W2_U;
    MUINT32 u4Gain_W2_V;
    MUINT32 u4Range_W3_Y;
    MUINT32 u4Range_W3_U;
    MUINT32 u4Range_W3_V;
    MUINT32 u4W3_enable;
    MUINT32 u4Gain_W3_Y;
    MUINT32 u4Gain_W3_U;
    MUINT32 u4Gain_W3_V;
    MUINT32 u4Range_W4_Y;
    MUINT32 u4Range_W4_U;
    MUINT32 u4Range_W4_V;
    MUINT32 u4W4_enable;
    MUINT32 u4Gain_W4_Y;
    MUINT32 u4Gain_W4_U;
    MUINT32 u4Gain_W4_V;
    MUINT32 u4Range_W5_Y;
    MUINT32 u4Range_W5_U;
    MUINT32 u4Range_W5_V;
    MUINT32 u4W5_enable;
    MUINT32 u4Gain_W5_Y;
    MUINT32 u4Gain_W5_U;
    MUINT32 u4Gain_W5_V;
    MUINT32 u4Backup_0;
    MUINT32 u4Backup_1;
    MUINT32 u4Backup_2;
    MUINT32 u4Backup_3;
    MUINT32 u4Backup_4;
    MUINT32 u4Backup_5;
    MUINT32 u4Backup_6;
    MUINT32 u4Backup_7;
    MUINT32 u4Backup_8;
    MUINT32 u4Backup_9;
    MUINT32 u4Backup_10;
    MUINT32 u4Backup_11;
    MUINT32 u4Backup_12;
    MUINT32 u4Backup_13;
    MUINT32 u4Backup_14;
    MUINT32 u4Backup_15;
    MUINT32 u4Backup_16;
    MUINT32 u4Backup_17;
    MUINT32 u4Backup_18;
    MUINT32 u4Backup_19;
    MUINT32 u4Backup_20;
    MUINT32 u4Backup_21;
    MUINT32 u4Backup_22;
    MUINT32 u4Backup_23;
} ISP_NVRAM_FD_ANR_PARAM_T;

typedef struct
{
    MUINT32                       COLOR_Method;
    INT16                         COLOR_CT_Env[ISP_NVRAM_COLOR_CT_NUM];
    ISP_NVRAM_PCA_PARAM_T         rPcaParam[ISP_NVRAM_COLOR_SCENE_NUM];
    ISP_NVRAM_COLOR_T             COLOR[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM];
} ISP_NVRAM_COLOR_STRUCT, *PISP_NVRAM_COLOR_STRUCT;

#define ISP_NVRAM_ANR_TBL_CT_NUM        (4)
#define ISP_NVRAM_ANR_TBL_LV_NUM        (8)


typedef struct
{
    struct COLOR_T
    {
        MUINT32                   COLOR_Method;
    } COLOR;
    struct CCM_T
    {
        ISP_NVRAM_CCM_T           dynamic_CCM[ISP_NVRAM_DYNAMIC_CCM_NUM];
        AWB_GAIN_T                dynamic_CCM_AWBGain[ISP_NVRAM_DYNAMIC_CCM_NUM];
    } CCM;

} ISP_NVRAM_COLOR_COMM_T, *PISP_NVRAM_COLOR_COMM_T;

typedef struct
{
    MUINT32                       CCM_CT_valid_NUM;
    MUINT32                       CCM_Coef[3];
    ISP_NVRAM_CCM_T               CCM_Reg[10];
    AWB_GAIN_T                    AWBGain[10];
} ISP_NVRAM_MULTI_CCM_STRUCT, *PISP_NVRAM_MULTI_CCM_STRUCT;

typedef struct{
    MUINT32 u4DC_Offset; //(0~255, 0x~2x)
    MUINT32 u4Lum_Speed; // 0~32
    MUINT32 u4Hue_Speed; // 0~32
    MUINT32 u4Sat_Speed; // 0~32
    MUINT32 u4LSP_LV_TH; //(0~255)
    MUINT32 u4Outdoor_EN; // 1: on, 0: off
    MUINT32 u4Outdoor_Speed; // 0~32
    MUINT32 u4Outdoor_Ratio; // default is 16
} ISP_NVRAM_COLOR_PARAM_T;

typedef struct{
    ISP_NVRAM_FD_COLOR_PARAM_T    FD_COLOR;
    ISP_NVRAM_COLOR_T             COLOR_TBL;
} ISP_NVRAM_SCOLOR_PARAM_T;

typedef struct
{
    ISP_NVRAM_COLOR_COMM_T        COMM;
    ISP_NVRAM_MULTI_CCM_STRUCT    CCM[NVRAM_CCM_TBL_NUM];
    ISP_NVRAM_COLOR_PARAM_T       COLOR_PARAM[NVRAM_COLOR_PARAM_TBL_NUM];
    ISP_NVRAM_SCOLOR_PARAM_T      COLOR[NVRAM_COLOR_TBL_NUM];
} ISP_NVRAM_COLOR_TABLE_STRUCT, *PISP_NVRAM_COLOR_TABLE_STRUCT;

typedef struct
{
    // y = ax + b
    double a;
    double b;
} CoefLinear_T;

typedef struct
{
    CoefLinear_T S;
    CoefLinear_T O;
} DngNoiseProile_T;

typedef struct
{
    MINT32 i4RefereceIlluminant1;
    MINT32 i4RefereceIlluminant2;
    DngNoiseProile_T rNoiseProfile[4];
} ISP_NVRAM_DNG_METADATA_T, *PISP_NVRAM_DNG_METADATA_T;

typedef union
{
    struct  {
        MUINT32                         Version;
        MUINT32                         SensorId;    // ID of sensor module
        ISP_NVRAM_COMMON_STRUCT         ISPComm;
        ISP_NVRAM_REGISTER_STRUCT       ISPRegs;
        ISP_NVRAM_COLOR_TABLE_STRUCT    ISPColorTbl;
        ISP_NVRAM_TONEMAP_PARAM_T       ISPToneMap;
        ISP_NVRAM_DNG_METADATA_T        DngMetadata;
    };
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_ISP_FILE_SIZE];
} NVRAM_CAMERA_ISP_PARAM_STRUCT, *PNVRAM_CAMERA_ISP_PARAM_STRUCT;
static_assert( sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT) <= MAXIMUM_NVRAM_CAMERA_ISP_FILE_SIZE,"nvram size error");

class IspNvramRegMgr
{
public:
    IspNvramRegMgr(ISP_NVRAM_REGISTER_STRUCT*const pIspNvramRegs)
        : m_rRegs(*pIspNvramRegs)
    {}
    virtual ~IspNvramRegMgr() {}

public:
    enum EIndexNum
    {
    	/*
        NUM_SL2F        =   NVRAM_SL2F_TBL_NUM,
        NUM_SL2J        =   NVRAM_SL2_TBL_NUM,
        NUM_DBS         =   NVRAM_DBS_TBL_NUM,
        NUM_ADBS        =   NVRAM_ADBS_TBL_NUM,
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_BNR_BPC     =   NVRAM_BPC_TBL_NUM,
        NUM_BNR_NR1     =   NVRAM_NR1_TBL_NUM,
        NUM_BNR_PDC     =   NVRAM_PDC_TBL_NUM,
        NUM_RMM         =   NVRAM_RMM_TBL_NUM,
        NUM_RNR         =   NVRAM_RNR_TBL_NUM,
        NUM_SL2         =   NVRAM_SL2_TBL_NUM,
        NUM_UDM         =   NVRAM_UDM_TBL_NUM,
        NUM_CCM         =   ISP_NVRAM_CCM_CT_NUM,
        NUM_GGM         =   NVRAM_GGM_TBL_NUM,
        NUM_IHDR_GGM    =   NVRAM_IHDR_GGM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_ANR2        =   NVRAM_ANR2_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
//        NUM_BOK         =   NVRAM_BOK_TBL_NUM,
        NUM_HFG         =   NVRAM_HFG_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM,
        NUM_NR3D        =   NVRAM_NR3D_TBL_NUM,
        NUM_MFB         =   NVRAM_MFB_TBL_NUM,
        NUM_MIXER3      =   NVRAM_MIXER3_TBL_NUM,
        NUM_HLR         =   NVRAM_HLR_TBL_NUM,
        NUM_COLOR       =   (ISP_NVRAM_COLOR_SCENE_NUM * ISP_NVRAM_COLOR_CT_NUM),
        NUM_ABF         =   NVRAM_ABF_TBL_NUM,
        */
    };

private:    ////    Data Members.
    ISP_NVRAM_REGISTER_STRUCT&      m_rRegs;
};

typedef struct
{
//    PNVRAM_CAMERA_DEFECT_STRUCT     pCameraDefect;
    PNVRAM_CAMERA_SHADING_STRUCT    pCameraShading;
    PNVRAM_CAMERA_ISP_PARAM_STRUCT  pCameraPara;
    AWB_GAIN_T                      rCalGain;
} GET_SENSOR_CALIBRATION_DATA_STRUCT, *PGET_SENSOR_CALIBRATION_DATA_STRUCT;

#endif

