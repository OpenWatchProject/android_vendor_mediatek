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
#ifndef _MTK_STEREO_KERNEL_H_
#define _MTK_STEREO_KERNEL_H_

//#define __DEPTH_AF__

#include "MTKStereoKernelScenario.h"
#include "MTKUtilType.h"
#include "MTKStereoKernelErrCode.h"
//#include "dbg_cam_param.h"      // For DEBUG_CAM_MF_MID. It also includes "dbg_cam_mf_param.h (for DEBUG_MF_INFO_T)".

#define MTK_STEREO_KERNEL_NVRAM_LENGTH      5800
#define MAX_FRAME_NUM                       5
#define CAPTURE_MAX_FRAME_NUM               1

typedef enum
{
    STEREO_KERNEL_FEATURE_BEGIN = 0,
    STEREO_KERNEL_FEATURE_GET_RESULT,
    STEREO_KERNEL_FEATURE_GET_STATUS,
    STEREO_KERNEL_FEATURE_SAVE_LOG,

    STEREO_KERNEL_FEATURE_SET_PROC_INFO,

    STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO,
    STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO,

    STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING,

    STEREO_KERNEL_FEATURE_GET_WIN_REMAP_INFO,

    STEREO_KERNEL_FEATURE_MAX,

    STEREO_KERNEL_FEATURE_LOAD_NVRAM,

    STEREO_KERNEL_FEATURE_SAVE_NVRAM,
}
STEREO_KERNEL_FEATURE_ENUM;

/////////////////////////////
//    For Tuning
/////////////////////////////
typedef struct
{
    MUINT8  alg_color        ;
    MUINT8  cc_thr           ;
    MINT8   cc_protect_gap   ;

    MUINT8  learn_tolerance  ;

    MFLOAT  search_range_xL  ;
    MFLOAT  search_range_xR  ;
    MFLOAT  search_range_yT  ;
    MFLOAT  search_range_yD  ;
    MUINT16 fefm_in_width    ;
    MUINT16 fefm_in_height   ;

    MUINT16 rgba_in_width    ;
    MUINT16 rgba_in_height   ;
}
STEREO_KERNEL_TUNING_PARA_STRUCT, *P_STEREO_KERNEL_TUNING_PARA_STRUCT ;

/////////////////////////////
typedef struct
{
    MUINT16 pixel_array_width ;
    MUINT16 pixel_array_height ;
    MUINT16 sensor_offset_x0 ;
    MUINT16 sensor_offset_y0 ;
    MUINT16 sensor_size_w0 ;
    MUINT16 sensor_size_h0 ;
    MUINT16 sensor_scale_w ;
    MUINT16 sensor_scale_h ;
    MUINT16 sensor_offset_x1 ;
    MUINT16 sensor_offset_y1 ;
    //MUINT16 sensor_size_w1 ;
    //MUINT16 sensor_size_h1 ;
    MUINT16 tg_offset_x ;
    MUINT16 tg_offset_y ;
    //MUINT16 tg_size_w ;
    //MUINT16 tg_size_h ;
    MUINT16 rrz_offset_x ;
    MUINT16 rrz_offset_y ;
    MUINT16 rrz_usage_width ;
    MUINT16 rrz_usage_height ;
    MUINT16 rrz_out_width ;
    MUINT16 rrz_out_height ;
}
STEREO_KERNEL_COORD_REMAP_INFO_STRUCT ;

typedef struct
{
    STEREO_KERNEL_SCENARIO_ENUM scenario ;

    // MAIN CAMERA IMAGE CAPTURE RESOLUTION
    MUINT16 main_source_image_width  ; // for IMAGE REFOCUS
    MUINT16 main_source_image_height ;
    MUINT16 main_output_image_width  ;
    MUINT16 main_output_image_height ;

    // ALGORITHM INPUT and SbS OUTPUT
    MUINT16 algo_source_image_width  ;
    MUINT16 algo_source_image_height ;
    MUINT16 algo_output_image_width  ; // for IMAGE REFOCUS/N3D
    MUINT16 algo_output_image_height ;

    // HWFE INPUT - the actual size for HWFE (after SRZ)
    MUINT16 fefm_image_width    ;
    MUINT16 fefm_image_height   ;
    MUINT32 fefm_image_stride   ;
    MUINT32 fefm_image_format   ;   // 0: YV12, 1: RGBA

    // COLOR CORRECTION INPUT
    MUINT16 rgba_image_width    ;
    MUINT16 rgba_image_height   ;
    MUINT16 rgba_image_stride   ;
    MUINT32 rgba_image_depth    ;

    MUINT8  hwfe_block_size     ;

    // Learning
    MFLOAT  hori_fov_main       ;   // Horizontal FOV camera L
    MFLOAT  hori_fov_minor      ;   // Horizontal FOV camera R
    MFLOAT  stereo_baseline     ;
    MUINT8  sensor_config       ;   // L-R, 0: main_minor, 1: minor_main // warp_index = !sensor_config
    MUINT32 support_diff_fov_fm ;
    MUINT32    module_orientation  ;   // 0: horizontal, 1: vertical

    MUINT16 dac_convert_info[9] ;   // 3 set, dac z disparity
    MUINT16 af_dac_start        ;   // DAC Start Current

    // WARPING / CROPING
    MUINT8 enable_cc            ;
    MUINT8 enable_gpu           ;
    MUINT32 enable_verify       ;

    // Learning Coordinates RE-MAPPING
    STEREO_KERNEL_COORD_REMAP_INFO_STRUCT remap_main ;
    STEREO_KERNEL_COORD_REMAP_INFO_STRUCT remap_minor ;

    // OUTPUT after Initialization
    MUINT32 working_buffer_size ;
    STEREO_KERNEL_TUNING_PARA_STRUCT *ptuning_para;
}
STEREO_KERNEL_SET_ENV_INFO_STRUCT ;

typedef struct
{
    MUINT32 ext_mem_size;
    MUINT8* ext_mem_start_addr; //working buffer start address
}
STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT, *P_STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT ;

typedef struct
{
    // IMAGE CAPTURE
    MUINT8* warp_image_addr_src ;               // for Warping (image with large view angle)
    MUINT8* warp_image_addr_dst ;               // for output warped image

    MUINT8* rgba_image_left_addr  ;             // for Photometric Correction
    MUINT8* rgba_image_right_addr ;             // for Photometric Correction

    MUINT8* fefm_image_left_addr ;
    MUINT8* fefm_image_right_addr ;

    STEREO_KERNEL_GRAPHIC_BUFFER_STRUCT src_gb; // soruce graphic buffer
    STEREO_KERNEL_GRAPHIC_BUFFER_STRUCT dst_gb; // output graphic buffer

    // HWFE
    MUINT16 *hwfe_data_left  ;                  // Data array for Hardware Feature Extraction, Left  Image
    MUINT16 *hwfe_data_right ;                  // Data array for Hardware Feature Extraction, Right Image

    // AF INFO.
    MUINT16 af_dac_index ;
    MUINT8  af_valid ;
    MFLOAT  af_confidence ;
    MUINT16 af_win_start_x_remap ;
    MUINT16 af_win_start_y_remap ;
    MUINT16 af_win_end_x_remap ;
    MUINT16 af_win_end_y_remap ;

    void* eglDisplay;
    void* InputGB;
    void* OutputGB;
}
STEREO_KERNEL_SET_PROC_INFO_STRUCT, *P_STEREO_KERNEL_SET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32 win_remap_depth_af_offset_x ;
    MUINT32 win_remap_depth_af_offset_y ;
    MFLOAT  win_remap_depth_af_scale_x ;
    MFLOAT  win_remap_depth_af_scale_y ;

    MUINT32 win_remap_refocus_ic_offset_x ;
    MUINT32 win_remap_refocus_ic_offset_y ;
    MFLOAT  win_remap_refocus_ic_scale_x ;
    MFLOAT  win_remap_refocus_ic_scale_y ;
}
STEREO_KERNEL_GET_WIN_REMAP_INFO_STRUCT, *P_STEREO_KERNEL_GET_WIN_REMAP_INFO_STRUCT ;

typedef struct
{
    // Image Capture Crop
    MUINT32  algo_left_offset_x ;
    MUINT32  algo_left_offset_y ;
    MUINT32  algo_right_offset_x ;
    MUINT32  algo_right_offset_y ;

    // Image Refocus
    MUINT32  main_offset_x ;
    MUINT32  main_offset_y ;

    MUINT32  algo_align_shift_x ;
    MUINT32  algo_align_shift_y ;

    MUINT16 num_hwfe_match ;
    MFLOAT *hwfe_match_data ;
    MUINT16 len_coord_trans ;
    MFLOAT *coord_trans_para ;

    // VERIFICATION ITEMS
    MINT32 verify_geo_quality_level ;   // 0: PASS, 1: WARN, 2: FAIL
    MINT32 verify_pho_quality_level ;   // 0: PASS, 1: WARN, 2: FAIL
    MINT32 verify_geo_statistics[6] ;   // 0: black sides(4bits), 1: # of black px, 2: # of px from black boarder (top) 3: (rgt), 4: (bot), 5:(lft)
    MINT32 verify_pho_statistics[4] ;   // 0: Y diff (0-255), 1: R similarity (%), 2: G similarity (%), 3: B similarity (%)
    MINT32 verify_cha_statistics[2] ;   // 0: isMTKChart (0:PASS,1:FAIL), 2: chartScore (>=0)
}
STEREO_KERNEL_RESULT_STRUCT, *P_STEREO_KERNEL_RESULT_STRUCT ;

/*
    CLASS
*/
class MTKStereoKernel{
public:
    static MTKStereoKernel* createInstance();
    virtual void   destroyInstance() = 0;

    virtual ~MTKStereoKernel(){};
    // Process Control
    virtual MRESULT StereoKernelInit(void* InitInData);
    virtual MRESULT StereoKernelMain(); // START
    virtual MRESULT StereoKernelReset();

    // Feature Control
    virtual MRESULT StereoKernelFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

    // N3D AC Debug Info.
    // For N3D AC algo. to set debug tag.
    /*
    inline void setDebugTag(DEBUG_MF_INFO_T &N3dAcDebugInfo, MINT32 i4ID, MINT32 i4Value)
    {
        N3dAcDebugInfo.Tag[i4ID].u4FieldID = CAMTAG(DEBUG_CAM_MF_MID, i4ID, 0);
        N3dAcDebugInfo.Tag[i4ID].u4FieldValue = i4Value;
    }
    */
    // For N3D HAL to get debug info.
    //virtual MRESULT getDebugInfo(DEBUG_MF_INFO_T &rN3dAcDebugInfo);

private:

};


#endif
