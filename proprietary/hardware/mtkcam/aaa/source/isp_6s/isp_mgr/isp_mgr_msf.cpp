/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_mgr_msf"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MSF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_MSF_T&
ISP_MGR_MSF_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(MSF);
}

template <>
ISP_MGR_MSF_T&
ISP_MGR_MSF_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_MSF_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
	PUT_REG_INFO_MULTI(SubModuleIndex, TOP_CTRL,							  top_ctrl						);
	PUT_REG_INFO_MULTI(SubModuleIndex, MULTISCALE_ROLLING,					  multiscale_rolling			);
	PUT_REG_INFO_MULTI(SubModuleIndex, TNR_STR_Y,							  tnr_str_y 					);
	PUT_REG_INFO_MULTI(SubModuleIndex, TNR_STR_UV,							  tnr_str_uv					);
	PUT_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_DIFFERENCE,				  frequency_difference			);
	PUT_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_NES,						  frequency_nes 				);
	PUT_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_BANDPASS_FILTER0,			  frequency_bandpass_filter0	);
	PUT_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_BANDPASS_FILTER1,			  frequency_bandpass_filter1	);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_DIFFERENCE0,				  gradient_difference0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_DIFFERENCE1,				  gradient_difference1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_NES,						  gradient_nes					);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_THD0,					  gradient_smooth_thd0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_THD1,					  gradient_smooth_thd1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_WGT0,					  gradient_smooth_wgt0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_SLP0,					  gradient_smooth_slp0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_DIFFERENCE0,					  mean_difference0				);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_DIFFERENCE1,					  mean_difference1				);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_DIFFERENCE2,					  mean_difference2				);
	PUT_REG_INFO_MULTI(SubModuleIndex, MEAN_NES,							  mean_nes						);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_THD0,				  mean_smooth_thd0				);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_THD1,				  mean_smooth_thd1				);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_WGT0,				  mean_smooth_wgt0				);
	PUT_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_SLP0,				  mean_smooth_slp0				);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESIDUE_DIFFERENCE0, 				  residue_difference0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESIDUE_DIFFERENCE1, 				  residue_difference1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_THD0,					  resi_y_smooth_thd0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_THD1,					  resi_y_smooth_thd1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_WGT0,					  resi_y_smooth_wgt0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_SLP0,					  resi_y_smooth_slp0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_THD0, 				  resi_uv_smooth_thd0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_THD1, 				  resi_uv_smooth_thd1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_WGT0, 				  resi_uv_smooth_wgt0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_SLP0, 				  resi_uv_smooth_slp0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT0,		  global_frequency_analysis0	);
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT1,		  global_frequency_analysis1	);
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT2,		  global_frequency_analysis2	);
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT3,		  global_frequency_analysis3	);
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS0,			  global_frequency_coefficient0 );
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS1,			  global_frequency_coefficient1 );
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS2,			  global_frequency_coefficient2 );
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS3,			  global_frequency_coefficient3 );
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT4,		  global_frequency_coefficient4 );
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT5,		  global_frequency_coefficient5 );
	PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_STATISTICS0,		  global_frequency_statistics0	);
	PUT_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE0,					  edge_confidence0				);
	PUT_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE1,					  edge_confidence1				);
	PUT_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE2,					  edge_confidence2				);
	PUT_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE3,					  edge_confidence3				);
	PUT_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE4,					  edge_confidence4				);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL0,					  blending_control0 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL1,					  blending_control1 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL2,					  blending_control2 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL3,					  blending_control3 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL4,					  blending_control4 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL5,					  blending_control5 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL6,					  blending_control6 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL7,					  blending_control7 			);
	PUT_REG_INFO_MULTI(SubModuleIndex, CONFIDENCE_ACCUMULATION, 			  confidence_accumulation		);
	PUT_REG_INFO_MULTI(SubModuleIndex, AGGRESSIVE_BLENDING0,				  aggressive_blending0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, AGGRESSIVE_BLENDING1,				  aggressive_blending1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL0,					  luminance_control0			);
	PUT_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL1,					  luminance_control1			);
	PUT_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL2,					  luminance_control2			);
	PUT_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL3,					  luminance_control3			);
	PUT_REG_INFO_MULTI(SubModuleIndex, SHADING_LINK0,						  shading_link0 				);
	PUT_REG_INFO_MULTI(SubModuleIndex, SHADING_LINK1,						  shading_link1 				);
	PUT_REG_INFO_MULTI(SubModuleIndex, NOISE_LEVEL_STATISTICS0, 			  noise_level_statistics0		);
	PUT_REG_INFO_MULTI(SubModuleIndex, NOISE_LEVEL_STATISTICS1, 			  noise_level_statistics1		);
	PUT_REG_INFO_MULTI(SubModuleIndex, BACKGROUND_PROTECTION0,				  background_protection0		);
	PUT_REG_INFO_MULTI(SubModuleIndex, BACKGROUND_PROTECTION1,				  background_protection1		);
	PUT_REG_INFO_MULTI(SubModuleIndex, VALID_BIT_ADJUSTMENT,				  valid_bit_adjustment			);
	PUT_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING0,			  non_local_means_filtering0	);
	PUT_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING1,			  non_local_means_filtering1	);
	PUT_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING2,			  non_local_means_filtering2	);
	PUT_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING3,			  non_local_means_filtering3	);
	PUT_REG_INFO_MULTI(SubModuleIndex, HIGH_FREQUENCY_ENHANCEMENT0, 		  high_frequency_enhancement0	);
	PUT_REG_INFO_MULTI(SubModuleIndex, HIGH_FREQUENCY_ENHANCEMENT1, 		  high_frequency_enhancement1	);
	PUT_REG_INFO_MULTI(SubModuleIndex, HIGH_FREQUENCY_ENHANCEMENT2, 		  high_frequency_enhancement2	);
	PUT_REG_INFO_MULTI(SubModuleIndex, HFE_NES0,							  hfe_nes0						);
	PUT_REG_INFO_MULTI(SubModuleIndex, HFE_NES1,							  hfe_nes1						);
	PUT_REG_INFO_MULTI(SubModuleIndex, TEMPORAL,							  temporal						);
	PUT_REG_INFO_MULTI(SubModuleIndex, SPATIAL, 							  spatial						);
	PUT_REG_INFO_MULTI(SubModuleIndex, HDR_FUSION_0,						  hdr_fusion_0					);
	PUT_REG_INFO_MULTI(SubModuleIndex, HDR_FUSION_1,						  hdr_fusion_1					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_FILTER0, 						  syn_filter0					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_FILTER1, 						  syn_filter1					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_FILTER2, 						  syn_filter2					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_DENOISE_AND_DIFF,				  syn_denoise_and_diff			);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_CONF0,							  syn_conf0 					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_CONF1,							  syn_conf1 					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_PRIOR0,							  syn_prior0					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_PRIOR1,							  syn_prior1					);
	PUT_REG_INFO_MULTI(SubModuleIndex, SYN_GENERAL, 						  syn_general					);

    return (*this);
}

template <>
ISP_MGR_MSF_T&
ISP_MGR_MSF_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_MSF_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
	GET_REG_INFO_MULTI(SubModuleIndex, TOP_CTRL,							  top_ctrl						);
	GET_REG_INFO_MULTI(SubModuleIndex, MULTISCALE_ROLLING,					  multiscale_rolling			);
	GET_REG_INFO_MULTI(SubModuleIndex, TNR_STR_Y,							  tnr_str_y 					);
	GET_REG_INFO_MULTI(SubModuleIndex, TNR_STR_UV,							  tnr_str_uv					);
	GET_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_DIFFERENCE,				  frequency_difference			);
	GET_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_NES,						  frequency_nes 				);
	GET_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_BANDPASS_FILTER0,			  frequency_bandpass_filter0	);
	GET_REG_INFO_MULTI(SubModuleIndex, FREQUENCY_BANDPASS_FILTER1,			  frequency_bandpass_filter1	);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_DIFFERENCE0,				  gradient_difference0			);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_DIFFERENCE1,				  gradient_difference1			);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_NES,						  gradient_nes					);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_THD0,					  gradient_smooth_thd0			);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_THD1,					  gradient_smooth_thd1			);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_WGT0,					  gradient_smooth_wgt0			);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_SMOOTH_SLP0,					  gradient_smooth_slp0			);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_DIFFERENCE0,					  mean_difference0				);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_DIFFERENCE1,					  mean_difference1				);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_DIFFERENCE2,					  mean_difference2				);
	GET_REG_INFO_MULTI(SubModuleIndex, MEAN_NES,							  mean_nes						);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_THD0,				  mean_smooth_thd0				);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_THD1,				  mean_smooth_thd1				);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_WGT0,				  mean_smooth_wgt0				);
	GET_REG_INFO_MULTI(SubModuleIndex, GRADIENT_SMOOTH_SLP0,				  mean_smooth_slp0				);
	GET_REG_INFO_MULTI(SubModuleIndex, RESIDUE_DIFFERENCE0, 				  residue_difference0			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESIDUE_DIFFERENCE1, 				  residue_difference1			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_THD0,					  resi_y_smooth_thd0			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_THD1,					  resi_y_smooth_thd1			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_WGT0,					  resi_y_smooth_wgt0			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_Y_SMOOTH_SLP0,					  resi_y_smooth_slp0			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_THD0, 				  resi_uv_smooth_thd0			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_THD1, 				  resi_uv_smooth_thd1			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_WGT0, 				  resi_uv_smooth_wgt0			);
	GET_REG_INFO_MULTI(SubModuleIndex, RESI_UV_SMOOTH_SLP0, 				  resi_uv_smooth_slp0			);
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT0,		  global_frequency_analysis0	);
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT1,		  global_frequency_analysis1	);
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT2,		  global_frequency_analysis2	);
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT3,		  global_frequency_analysis3	);
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS0,			  global_frequency_coefficient0 );
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS1,			  global_frequency_coefficient1 );
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS2,			  global_frequency_coefficient2 );
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_ANALYSIS3,			  global_frequency_coefficient3 );
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT4,		  global_frequency_coefficient4 );
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_COEFFICIENT5,		  global_frequency_coefficient5 );
	GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL_FREQUENCY_STATISTICS0,		  global_frequency_statistics0	);
	GET_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE0,					  edge_confidence0				);
	GET_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE1,					  edge_confidence1				);
	GET_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE2,					  edge_confidence2				);
	GET_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE3,					  edge_confidence3				);
	GET_REG_INFO_MULTI(SubModuleIndex, EDGE_CONFIDENCE4,					  edge_confidence4				);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL0,					  blending_control0 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL1,					  blending_control1 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL2,					  blending_control2 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL3,					  blending_control3 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL4,					  blending_control4 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL5,					  blending_control5 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL6,					  blending_control6 			);
	GET_REG_INFO_MULTI(SubModuleIndex, BLENDING_CONTROL7,					  blending_control7 			);
	GET_REG_INFO_MULTI(SubModuleIndex, CONFIDENCE_ACCUMULATION, 			  confidence_accumulation		);
	GET_REG_INFO_MULTI(SubModuleIndex, AGGRESSIVE_BLENDING0,				  aggressive_blending0			);
	GET_REG_INFO_MULTI(SubModuleIndex, AGGRESSIVE_BLENDING1,				  aggressive_blending1			);
	GET_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL0,					  luminance_control0			);
	GET_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL1,					  luminance_control1			);
	GET_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL2,					  luminance_control2			);
	GET_REG_INFO_MULTI(SubModuleIndex, LUMINANCE_CONTROL3,					  luminance_control3			);
	GET_REG_INFO_MULTI(SubModuleIndex, SHADING_LINK0,						  shading_link0 				);
	GET_REG_INFO_MULTI(SubModuleIndex, SHADING_LINK1,						  shading_link1 				);
	GET_REG_INFO_MULTI(SubModuleIndex, NOISE_LEVEL_STATISTICS0, 			  noise_level_statistics0		);
	GET_REG_INFO_MULTI(SubModuleIndex, NOISE_LEVEL_STATISTICS1, 			  noise_level_statistics1		);
	GET_REG_INFO_MULTI(SubModuleIndex, BACKGROUND_PROTECTION0,				  background_protection0		);
	GET_REG_INFO_MULTI(SubModuleIndex, BACKGROUND_PROTECTION1,				  background_protection1		);
	GET_REG_INFO_MULTI(SubModuleIndex, VALID_BIT_ADJUSTMENT,				  valid_bit_adjustment			);
	GET_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING0,			  non_local_means_filtering0	);
	GET_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING1,			  non_local_means_filtering1	);
	GET_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING2,			  non_local_means_filtering2	);
	GET_REG_INFO_MULTI(SubModuleIndex, NON_LOCAL_MEANS_FILTERING3,			  non_local_means_filtering3	);
	GET_REG_INFO_MULTI(SubModuleIndex, HIGH_FREQUENCY_ENHANCEMENT0, 		  high_frequency_enhancement0	);
	GET_REG_INFO_MULTI(SubModuleIndex, HIGH_FREQUENCY_ENHANCEMENT1, 		  high_frequency_enhancement1	);
	GET_REG_INFO_MULTI(SubModuleIndex, HIGH_FREQUENCY_ENHANCEMENT2, 		  high_frequency_enhancement2	);
	GET_REG_INFO_MULTI(SubModuleIndex, HFE_NES0,							  hfe_nes0						);
	GET_REG_INFO_MULTI(SubModuleIndex, HFE_NES1,							  hfe_nes1						);
	GET_REG_INFO_MULTI(SubModuleIndex, TEMPORAL,							  temporal						);
	GET_REG_INFO_MULTI(SubModuleIndex, SPATIAL, 							  spatial						);
	GET_REG_INFO_MULTI(SubModuleIndex, HDR_FUSION_0,						  hdr_fusion_0					);
	GET_REG_INFO_MULTI(SubModuleIndex, HDR_FUSION_1,						  hdr_fusion_1					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_FILTER0, 						  syn_filter0					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_FILTER1, 						  syn_filter1					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_FILTER2, 						  syn_filter2					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_DENOISE_AND_DIFF,				  syn_denoise_and_diff			);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_CONF0,							  syn_conf0 					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_CONF1,							  syn_conf1 					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_PRIOR0,							  syn_prior0					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_PRIOR1,							  syn_prior1					);
	GET_REG_INFO_MULTI(SubModuleIndex, SYN_GENERAL, 						  syn_general					);

    return (*this);
}

MBOOL
ISP_MGR_MSF_T::
apply_MSF(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, mfb_reg_t* pReg)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("MSF", SubModuleIndex);
    return MTRUE;
}


};
