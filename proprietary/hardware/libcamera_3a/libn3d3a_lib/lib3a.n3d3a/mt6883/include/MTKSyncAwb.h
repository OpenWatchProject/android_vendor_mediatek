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
#ifndef _MTK_SYNCAWB_H
#define _MTK_SYNCAWB_H

#include "MTKSync3AType.h"
//#include <camera_custom_3a_nvram.h>
#include <camera_custom_awb_nvram.h>
#include <camera_custom_awb_sync_nvram.h>
//#include <camera_custom_AEPlinetable.h>
#include "MTKSyncAwbErrCode.h"
//#include <awb_param.h>
#include "awb_param_if.h"
//#define SYNC_AWB_DEBUG

#define SYNC_AWB_GAIN_UNIT (512)

typedef enum
{
    SYNC_AWB_MAIN_CH = 0,
    SYNC_AWB_SUB_CH  = 1,
    SYNC_AWB_CH_NUM  = 2
} AWB_SYNC_CHANNEL_ENUM;


/*
typedef struct
{
    MINT32 i4R; // R gain
    MINT32 i4G; // G gain
    MINT32 i4B; // B gain
}AWB_GAIN_STRUCT;
*/

typedef enum
{
    SYNC_AWB_SCENARIO_PIP,
    SYNC_AWB_SCENARIO_N3D
}SYNC_AWB_SCENARIO_ENUM;


typedef enum
{
    SYNC_AWB_STATE_STANDBY,
    SYNC_AWB_STATE_INIT,
    SYNC_AWB_STATE_PROC,
    SYNC_AWB_STATE_READY,
    SYNC_AWB_STATE_IDLE,
    SYNC_AWB_STATE_MAX
}SYNC_AWB_STATE_ENUM;


typedef enum
{
    SYNC_AWB_FEATURE_GET_DEBUG_INFO,
    SYNC_AWB_FEATURE_GET_MAPPING_GAIN,
    SYNC_AWB_FEATURE_GET_STATE
}SYNC_AWB_FEATURE_CONTROL_ENUM;



// === Init struct ===//
typedef enum
{
    eHorizon_Light = 0,
    eA_Light,
    eTL84_Light,
    eDNP_Light,
    eD65_Light,
    eExtd_Light,
    eEND_Light
}eLIGHT_SOURCE_NUM;


typedef enum
{
    eYRef_Low = 0,
    eYRef_Mid,
    eYRef_High,
    eYRef_Num
}eLIGHT_Y_Ref_NUM;


// Light source definition
typedef enum
{
    N3D_AWB_LIGHT_STROBE = 0,                  // Strobe
    N3D_AWB_LIGHT_TUNGSTEN,                    // Tungsten
    N3D_AWB_LIGHT_WARM_FLUORESCENT,            // Warm fluorescent
    N3D_AWB_LIGHT_FLUORESCENT,                 // Fluorescent (TL84)
    N3D_AWB_LIGHT_CWF,                         // CWF
    N3D_AWB_LIGHT_DAYLIGHT,                    // Daylight
    N3D_AWB_LIGHT_SHADE,                       // Shade
    N3D_AWB_LIGHT_DAYLIGHT_FLUORESCENT,        // Daylight fluorescent
    N3D_AWB_LIGHT_NUM,                         // Light source number
    N3D_AWB_LIGHT_NONE = N3D_AWB_LIGHT_NUM,        // None: not neutral block
    //AWB_LIGHT_DONT_CARE = 0xFF             // Don't care: don't care the light source of block
} N3D_AWB_LIGHT_T;

// Light source probability
typedef struct
{
    MINT32 i4P0[N3D_AWB_LIGHT_NUM]; // Probability 0
    MINT32 i4P1[N3D_AWB_LIGHT_NUM]; // Probability 1
    MINT32 i4P2[N3D_AWB_LIGHT_NUM]; // Probability 2
    MINT32 i4P[N3D_AWB_LIGHT_NUM];  // Probability
} N3D_AWB_LIGHT_PROBABILITY_T;

typedef struct
{
    AWB_GAIN_T NormalWB_Gain[eEND_Light];
}NORMAL_GAIN_INIT_INPUT_PARAM;

typedef struct
{
    AWB_GAIN_T Golden_Gain;
    AWB_GAIN_T Unit_Gain;
}SYNC_GAIN_INIT_INPUT_PARAM;

/*
typedef enum
{
    //SYNC_AWB_CCT_TH_METHOD = 0,
    //SYNC_AWB_GAIN_INTERPOLATION_METHOD,
    SYNC_AWB_ADV_PP_METHOD,
    SYNC_AWB_BLENDING_METHOD,
    SYNC_AWB_WHITPOINT_MAP_METHOD,
    SYNC_AWB_FREE_RUN
}eSYNC_AWB_METHOD;
*/

typedef struct
{
    MBOOL PP_method_consider_ccm;
    MUINT32 PP_method_Y_threshold_low;
    MUINT32 PP_method_Y_threshold_high;
    MUINT32 PP_method_valid_block_num_ratio; // %precentage

    MUINT32 PP_method_valid_childblock_percent_th;
    MUINT32 PP_method_diff_percent_th;
	MUINT32 PP_method_enhance_change;

}SYNC_AWB_PP_TUNING_STRUCT;


typedef struct
{
    MUINT32 BlendingTh[4];

}SYNC_AWB_BLEND_TUNING_STRUCT;

typedef struct
{
    MUINT32 WP_GR_ratio_th_main[2];  //[0] low bound, [1] high bound
    MUINT32 WP_GB_ratio_th_main[2];

    MUINT32 WP_GR_ratio_th_sub[2];
    MUINT32 WP_GB_ratio_th_sub[2];

    MUINT8  WP_area_hit_ratio;
    MUINT8  WP_PP_gain_ratio_tbl[2];

    MUINT32 WP_Y_th[2];
}SYNC_AWB_WHITEPOINTS_MAPPING_TUNING_STRUCT;


typedef struct
{
    MBOOL bEnSyncWBSmooth;
    MUINT32 u4SyncAWBSmoothParam[3];

}SYNC_AWB_SMOOTH_TUNING_STRUCT;

typedef struct
{
    MINT32 PP_method_Tungsten_prefer[3];
	MINT32 PP_method_WarmFL_prefer[3];
	MINT32 PP_method_FL_prefer[3];
	MINT32 PP_method_CWF_prefer[3];
	MINT32 PP_method_Day_prefer[3];
	MINT32 PP_method_Shade_prefer[3];
	MINT32 PP_method_DayFL_prefer[3];
    
}SYNC_AWB_PREFER_TUNING_STRUCT;

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
    
}SYNC_AWB_PREFER_LVBASE_STRUCT;

typedef struct
{
    MINT32 i4LUT[AWB_LV_INDEX_NUM];
} SYNC_AWB_LV_LUT_T;


typedef struct
{
    MUINT32 Geometric_Smooth_Radius;
    MUINT32 Temporary_Smooth_Speed;
    MUINT32 Smooth_Convergence_th;
}STANDBY_SYNC_AWB_SMOOTH_TUNING_STRUCT;

typedef struct
{
    MUINT32 standbysyncAWBMethod;  //Choose use which kind of sync method
    STANDBY_SYNC_AWB_SMOOTH_TUNING_STRUCT StandbysyncAWB_tuning_param;

}STANDBY_SYNC_AWB_TUNNING_PARAM_STRUCT;

typedef struct
{
    MINT32 i4X;
    MINT32 i4Y;
} SYNC_AWB_XY_COORDINATE_STRUCT;

typedef struct
{
    MINT32 i4Lut[AWB_SYNC_Y_OFFSET_INDEX_NUM]; // Look-up table
} SYNC_AWB_Y_OFFSET_LUT_T;


typedef struct
{
    SYNC_AWB_XY_COORDINATE_STRUCT rRotXY[1];
    SYNC_AWB_XY_COORDINATE_STRUCT rY_Href[3];
    SYNC_AWB_XY_COORDINATE_STRUCT rY_Lref[3];    
    SYNC_AWB_XY_COORDINATE_STRUCT rCCTRefXY_offset[6];
    SYNC_AWB_Y_OFFSET_LUT_T  rY_offset[eEND_Light];

}SYNC_AWB_CCT_MAPPING_TUNNING_PARAM_STRUCT;


typedef struct
{
    MUINT32 SyncMode;  //Choose use which kind of sync method

    SYNC_AWB_PP_TUNING_STRUCT syncAWB_pp_method_tuning_param;
    SYNC_AWB_BLEND_TUNING_STRUCT syncAWB_blend_method_tuning_param;
    SYNC_AWB_WHITEPOINTS_MAPPING_TUNING_STRUCT syncAWB_wp_method_tuning_param;

    MBOOL isFixMapLocation;
    MUINT32 u4SyncAWB_fov_ratio;

    SYNC_AWB_SMOOTH_TUNING_STRUCT syncAWB_smooth_param;
    SYNC_AWB_PREFER_TUNING_STRUCT syncAWB_prefer_param;
    SYNC_AWB_PREFER_LVBASE_STRUCT syncAWB_prefer_lvbase;

    SYNC_AWB_LV_LUT_T BlendRatio;    

    STANDBY_SYNC_AWB_TUNNING_PARAM_STRUCT standbySyncAwbCtrlParam;

    SYNC_AWB_CCT_MAPPING_TUNNING_PARAM_STRUCT CCTMappingSyncParam;

}SYNC_AWB_TUNNING_PARAM_STRUCT;


typedef struct
{
    SYNC_AWB_SCENARIO_ENUM SyncScenario;
    AWB_NVRAM_T main_param;
    AWB_NVRAM_T sub_param;
    AWBSYNC_NVRAM_T main_sync_param;
    AWBSYNC_NVRAM_T sub_sync_param;
    SYNC_AWB_TUNNING_PARAM_STRUCT SyncAwbTuningParam;
}SYNC_AWB_INIT_INFO_STRUCT;


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
} strImageCornerMappingTable_syncAWB;

typedef struct
{
    MVOID *pAWBStatBuf;
    MUINT32 stat_line_size;
} SYNC_AWB_AAO_BUFFER_INTO_STRUCT;



typedef struct
{
    float CCmatrix[9];
}CC_MATRIX_STRUCT;


//Input and Output info
typedef struct
{
/* W+T modified
    AWB_GAIN_T alg_gain;
    AWB_GAIN_T curr_gain;
    AWB_GAIN_T target_gain; //target_gain = alg_gain * output_cal_gain
    MINT32 m_i4CCT;
    MINT32 m_i4LightMode;
    MINT32  m_i4SceneLV;
    N3D_AWB_LIGHT_PROBABILITY_T rLightProb;
    AWB_GAIN_T rAwbGainNoPref; //AWB gain without preference
    AWB_PARENT_BLK_STAT_T rAwbParentStatBlk;
    MINT32 ParentBlkNumX;
    MINT32 ParentBlkNumY;
    strImageCornerMappingTable_syncAWB *pImageMapping;  
    SYNC_AWB_AAO_BUFFER_INTO_STRUCT aao_buf_info;
*/

    AWB_OUTPUT_N3D_T awb_output_struct; //defined in awb param 
    MINT32 focus_distance;   //focus distance information 
    SYNC_AWB_AAO_BUFFER_INTO_STRUCT aao_buf_info;
    CC_MATRIX_STRUCT ccm;

}SYNC_AWB_INPUT_PROP_STRUCT;

typedef struct
{
    AWB_GAIN_T rAwbGain;
    MINT32 i4CCT;
}SYNC_AWB_OUTPUT;

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
} SYNC_AWB_AAO_CONFIG_STRUCT;

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
} SYNC_AWB_LOCATION_MAPPING_MATRIX_STRUCT;

typedef struct
{
    SYNC_AWB_AAO_CONFIG_STRUCT AAOWinConfig_main;
    SYNC_AWB_AAO_CONFIG_STRUCT AAOWinConfig_main2;
    SYNC_AWB_LOCATION_MAPPING_MATRIX_STRUCT  Mapping_Matrix;
    MBOOL isFixMapLocation;
    MUINT32 u4MainCamFovRatio;
    MUINT32 u4SubCamFovRatio;    
} SYNC_AWB_LOCATION_TRANS_STRUCT;

typedef struct
{
/* W+T modified
    SYNC_AWB_LOCATION_TRANS_STRUCT strLocationTransInfo ;
    SYNC_AWB_INPUT_PROP_STRUCT main_ch;
    SYNC_AWB_INPUT_PROP_STRUCT sub_ch;
*/

    MUINT32 feature_type;  //B+M / VSDOF / W+T
    MINT32 display_cam;  // which cam provide display source 
    MINT32 zoom_ratio;	 //zoom in ratio, 32 base (32 = 1x)
    MBOOL EnStandbyMode;
    MBOOL isSwitch;
    SYNC_AWB_LOCATION_TRANS_STRUCT strLocationTransInfo;
    SYNC_AWB_INPUT_PROP_STRUCT main_ch;
    SYNC_AWB_INPUT_PROP_STRUCT sub_ch;

}SYNC_AWB_INPUT_INFO_STRUCT;

typedef struct
{
    SYNC_AWB_OUTPUT main_ch;
    SYNC_AWB_OUTPUT sub_ch;
}SYNC_AWB_OUTPUT_INFO_STRUCT;

/*
//Process info
typedef struct
{
    MINT32 m_i4AWBState;
    MBOOL m_bIsStrobeFired;
}SYNC_AWB_PROC_INFO_STRUCT, *P_SYNC_AWB_PROC_INFO_STRUCT;
*/
/*
typedef struct
{
    MUINT16 ucR; // R average of specified AWB window
    MUINT16 ucG; // G average of specified AWB window
    MUINT16 ucB; // B average of specified AWB window
    MUINT16 ucEL; // error number + light source info
} N3D_SYNCAWB_WINDOW_T;
*/
typedef struct
{
    MUINT32 u4R; // R average of specified AWB window
    MUINT32 u4G; // G average of specified AWB window
    MUINT32 u4B; // B average of specified AWB window

    struct
    {
        MUINT32 uCntR : 9;
        MUINT32 uCntG : 9;
        MUINT32 uCntB : 9;
        MUINT32 rsv: 1;
        MUINT32 uLight : 4;
    };

} SYNC_AWB_STAT_T;



// Child block statistics
#define N3D_AWB_CHILD_WINDOW_NUM_X (120)
#define N3D_AWB_CHILD_WINDOW_NUM_Y (90) 
#define N3D_AWB_CHILD_WINDOW_TOTAL_NUM (N3D_AWB_CHILD_WINDOW_NUM_X * N3D_AWB_CHILD_WINDOW_NUM_Y)

typedef struct
{
    MINT32 i4SumR[N3D_AWB_CHILD_WINDOW_NUM_Y][N3D_AWB_CHILD_WINDOW_NUM_X]; 
    MINT32 i4SumG[N3D_AWB_CHILD_WINDOW_NUM_Y][N3D_AWB_CHILD_WINDOW_NUM_X]; 
    MINT32 i4SumB[N3D_AWB_CHILD_WINDOW_NUM_Y][N3D_AWB_CHILD_WINDOW_NUM_X]; 
    MINT32 i4Light[N3D_AWB_CHILD_WINDOW_NUM_Y][N3D_AWB_CHILD_WINDOW_NUM_X]; 
} N3D_SYNCAWB_CHILD_BLK_STAT_T;


typedef struct
{
    MINT32 i4Display_cam;
    MUINT32 feature_type;  //B+M / VSDOF / W+T
    MBOOL EnStandbyMode;
    AWB_CALIBRATION_DATA_T MasterCalibrationData;
    AWB_CALIBRATION_DATA_T SlaveCalibrationData;
    AWB_OUTPUT_N3D_T rAWBSyncData;  // N3D AWB info

}SYNC_AWB_DIRECT_GAIN_MAPPING_STRUCT;


class MTKSyncAwb
{
public:
    static MTKSyncAwb* createInstance();
    virtual void destroyInstance();

    virtual ~MTKSyncAwb(){};

    virtual MRESULT SyncAwbInit(void* InitData);
    virtual MRESULT SyncAwbMain(SYNC_AWB_INPUT_INFO_STRUCT *pAwbSyncInput, SYNC_AWB_OUTPUT_INFO_STRUCT *pAwbSyncOutput);

    virtual MRESULT SyncAwbFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};



#define DUAL_CAM_FEATURE_VSDOF       0x0001
#define DUAL_CAM_FEATURE_WIDE_TELE   0x0002
#define DUAL_CAM_FEATURE_BAYER_MONO  0x0004

#define ZOOM_RATIO_UNIT  (32)

#endif
