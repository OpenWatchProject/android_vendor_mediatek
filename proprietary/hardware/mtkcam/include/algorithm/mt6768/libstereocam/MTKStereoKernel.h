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

#include "MTKStereoCommonDef.h"
//#include "dbg_cam_param.h"      // For DEBUG_CAM_MF_MID. It also includes "dbg_cam_mf_param.h (for DEBUG_MF_INFO_T)".

#define MTK_STEREO_KERNEL_NVRAM_LENGTH      8400
#define MAX_FRAME_NUM                       5
#define CAPTURE_MAX_FRAME_NUM               1
#define MAX_GEO_LEVEL                       3

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
    STEREO_KERNEL_FEATURE_LOAD_LENS_INFO,

    STEREO_KERNEL_FEATURE_DEBUG,

}
STEREO_KERNEL_FEATURE_ENUM;

/////////////////////////////
//    For Tuning
/////////////////////////////
typedef struct
{
    MUINT32 p_tune  ;
    MUINT32 s_tune  ;
}
STEREO_KERNEL_TUNING_PARA_STRUCT, *P_STEREO_KERNEL_TUNING_PARA_STRUCT ;

/////////////////////////////
typedef struct
{
    MUINT32 pixel_array_width ;
    MUINT32 pixel_array_height ;
    MUINT32 sensor_offset_x0 ;
    MUINT32 sensor_offset_y0 ;
    MUINT32 sensor_size_w0 ;
    MUINT32 sensor_size_h0 ;
    MUINT32 sensor_scale_w ;
    MUINT32 sensor_scale_h ;
    MUINT32 sensor_offset_x1 ;
    MUINT32 sensor_offset_y1 ;
    //MUINT32 sensor_size_w1 ;
    //MUINT32 sensor_size_h1 ;
    MUINT32 tg_offset_x ;
    MUINT32 tg_offset_y ;
    //MUINT32 tg_size_w ;
    //MUINT32 tg_size_h ;
    MUINT32 rrz_offset_x ;
    MUINT32 rrz_offset_y ;
    MUINT32 rrz_usage_width ;
    MUINT32 rrz_usage_height ;
    MUINT32 rrz_out_width ;
    MUINT32 rrz_out_height ;
}
STEREO_KERNEL_FLOW_INFO_STRUCT ;

typedef struct
{
    STEREO_KERNEL_IMG_INFO_STRUCT img_main ;
    STEREO_KERNEL_IMG_INFO_STRUCT img_auxi ;
    MUINT32 size ;
}
STEREO_KERNEL_GEO_INFO_STRUCT ;

typedef struct
{
    STEREO_KERNEL_SCENARIO_ENUM scenario ;

    MUINT32 img_src_wd ;
    MUINT32 img_src_ht ;
    MUINT32 img_out_wd ;
    MUINT32 img_out_ht ;
    MUINT32 img_inp_wd ;
    MUINT32 img_inp_ht ;

    MUINT32 cap_src_wd ;
    MUINT32 cap_src_ht ;
    MUINT32 cap_out_wd ;
    MUINT32 cap_out_ht ;
    MUINT32 cap_inp_wd ;
    MUINT32 cap_inp_ht ;

    MUINT32 prv_src_wd ;
    MUINT32 prv_src_ht ;
    MUINT32 prv_inp_wd ;
    MUINT32 prv_inp_ht ;

    MUINT32 img_cmp_wd ;
    MUINT32 img_cmp_ht ;

    MUINT32 geo_info ;
    STEREO_KERNEL_GEO_INFO_STRUCT geo_img[MAX_GEO_LEVEL] ;
    STEREO_KERNEL_IMG_INFO_STRUCT pho_img ;

    MFLOAT  hfov_main  ;
    MFLOAT  hfov_auxi  ;
    MFLOAT  baseline   ;

    MUINT32 system_cfg ;

    MUINT32 dac_start  ;
    MUINT32 dac_info[9];

    STEREO_KERNEL_FLOW_INFO_STRUCT flow_main ;
    STEREO_KERNEL_FLOW_INFO_STRUCT flow_auxi ;

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
    MUINT8* addr_ms ;
    MUINT8* addr_md ;
    MUINT8* addr_mm ;
    MUINT8* addr_ml ;

    MUINT8* addr_as ;
    MUINT8* addr_ad ;
    MUINT8* addr_am ;

    MUINT8* addr_mp ;   
    MUINT8* addr_ap ;

    MUINT8* addr_mg[MAX_GEO_LEVEL] ;
    MUINT8* addr_ag[MAX_GEO_LEVEL] ;

    STEREO_KERNEL_GRAPHIC_BUFFER_STRUCT src_gb;
    STEREO_KERNEL_GRAPHIC_BUFFER_STRUCT dst_gb;

    MUINT16 *addr_me[MAX_GEO_LEVEL] ;
    MUINT16 *addr_ae[MAX_GEO_LEVEL] ;
    MUINT16 *addr_rl[MAX_GEO_LEVEL] ;
    MUINT16 *addr_lr[MAX_GEO_LEVEL] ;

    MUINT16 *addr_bs ;
    MUINT16 *addr_bd ;
    MUINT16 *addr_bm ;

    MUINT32 dac_i ;
    MUINT32 dac_v ;
    MFLOAT  dac_c ;
    MUINT32 dac_w[4] ;
    MINT32  eis[4] ;

    void* eglDisplay;   // maybe useless
    void* InputGB;
    void* OutputGB;
}
STEREO_KERNEL_SET_PROC_INFO_STRUCT, *P_STEREO_KERNEL_SET_PROC_INFO_STRUCT;

typedef struct
{
    MFLOAT  out_d[4] ;
    MUINT32 out_n[3] ;
    MFLOAT* out_p[3] ;
    MINT32  out_v[14] ;

#ifdef PC_SIM
    void *out_debug ;
#endif
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
    virtual MRESULT StereoKernelInit(void* InitInData);
    virtual MRESULT StereoKernelMain();
    virtual MRESULT StereoKernelReset();

    virtual MRESULT StereoKernelFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:

};


#endif
