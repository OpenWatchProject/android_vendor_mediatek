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
#ifndef _STEREO_SYNC2A_TUNING_H_
#define _STEREO_SYNC2A_TUNING_H_

#define MAX_MAPPING_DELTABV_ISPRATIO 30
typedef enum
{
    SYNC_DUAL_CAM_DENOISE_BMDN = 0,
    SYNC_DUAL_CAM_DENOISE_MFNR,
    SYNC_DUAL_CAM_DENOISE_MAX
} SYNC_DUAL_CAM_DENOISE_ENUM;
typedef struct
{
	MBOOL isFinerSyncOffset; // False: 10base, TRUE: 1000base
    MINT32 EVOffset_main[2]; //EVOffset main
    MINT32 EVOffset_main2[2]; //EVOffset main2
    MUINT32 RGB2YCoef_main[3]; //RGB2YCoef main
    MUINT32 RGB2YCoef_main2[3]; //RGB2YCoef main2
    MUINT32 FixSyncGain;
    MUINT32 u4RegressionType;//0:cwv 1:avg	
    MBOOL isDoGainRegression;
    MUINT16 SyncWhichEye;
    MUINT32 pDeltaBVtoRatioArray[SYNC_DUAL_CAM_DENOISE_MAX][MAX_MAPPING_DELTABV_ISPRATIO];
}strSyncAEInitInfo;



typedef struct
{
    MBOOL PP_method_consider_ccm;
    MUINT32 PP_method_Y_threshold_low;
    MUINT32 PP_method_Y_threshold_high;
    MUINT32 PP_method_valid_block_num_ratio; // %precentage

    MUINT32 PP_method_valid_childblock_percent_th;
    MUINT32 PP_method_diff_percent_th;
	  MUINT32 PP_method_enhance_change;

}SYNC_AWB_ADV_PP_METHOD_TUNING_STRUCT;


typedef struct
{
    MUINT32 BlendingTh[4];

}SYNC_AWB_BLEND_METHOD_TUNING_STRUCT;

typedef struct
{
    MUINT32 WP_Gr_ratio_th_main[2];  //[0] low bound, [1] high bound
    MUINT32 WP_Gb_ratio_th_main[2];

    MUINT32 WP_Gr_ratio_th_sub[2];
    MUINT32 WP_Gb_ratio_th_sub[2];

    MUINT8  WP_area_hit_ratio;
    MUINT8  WP_PP_gain_ratio_tbl[2];

    MUINT32 WP_Y_th[2];

}SYNC_AWB_WP_METHOD_TUNING_STRUCT;


typedef struct
{
    MBOOL bEnSyncWBSmooth;
    MUINT32 u4SyncAWBSmoothParam[3];
    
}SYNC_AWBGAIN_SMOOTH_TUNING_STRUCT;

typedef struct
{
    MINT32 PP_method_Tungsten_prefer[3];
	MINT32 PP_method_WarmFL_prefer[3];
	MINT32 PP_method_FL_prefer[3];
	MINT32 PP_method_CWF_prefer[3];
	MINT32 PP_method_Day_prefer[3];
	MINT32 PP_method_Shade_prefer[3];
	MINT32 PP_method_DayFL_prefer[3];
    
}SYNC_AWBGAIN_PREFER_TUNING_STRUCT;

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
    
}SYNC_AWBGAIN_PREFER_LVBASE_STRUCT;

typedef struct
{
    MUINT32 syncAWBMethod;  //Choose use which kind of sync method

    SYNC_AWB_ADV_PP_METHOD_TUNING_STRUCT syncAWB_pp_method_tuning_param;
    SYNC_AWB_BLEND_METHOD_TUNING_STRUCT syncAWB_blend_method_tuning_param;
    SYNC_AWB_WP_METHOD_TUNING_STRUCT syncAWB_wp_method_tuning_param;

    MBOOL isFixMapLocation;
    MUINT32 u4SyncAWB_fov_ratio;

    SYNC_AWBGAIN_SMOOTH_TUNING_STRUCT syncAWB_smooth_param;
	SYNC_AWBGAIN_PREFER_TUNING_STRUCT syncAWB_prefer_param;
	SYNC_AWBGAIN_PREFER_LVBASE_STRUCT syncAWB_prefer_lvbase;

}strSyncAWBInitInfo;



const strSyncAEInitInfo* getSyncAEInitInfo();
const strSyncAWBInitInfo* getSyncAWBInitInfo();

#endif //  _STEREO_SYNC2A_TUNING_H_

