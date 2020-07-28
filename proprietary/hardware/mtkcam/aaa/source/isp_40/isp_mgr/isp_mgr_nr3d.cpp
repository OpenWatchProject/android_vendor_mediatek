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
#define LOG_TAG "isp_mgr_nr3d"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_NR3D_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

#define CLIP_NR3D(a,b,c)   ( (a<b)?  b : ((a>c)? c: a) )
#define ADD_AND_WRITE_R2CF_CNT(NR3D_R2C_VAL, NR3D_R2CF_CNT)                                                                     \
    do {                                                                                                                        \
        MUINT32 R2C_VAL = reinterpret_cast<ISP_DIP_X_MDP_TNR_R2C_1_T*>(REG_INFO_VALUE_PTR(DIP_X_MDP_TNR_R2C_1))->NR3D_R2C_VAL;  \
        if (m_##NR3D_R2CF_CNT >= R2C_VAL)                                                                                       \
        {                                                                                                                       \
            m_##NR3D_R2CF_CNT = 0;                                                                                              \
        }                                                                                                                       \
        else                                                                                                                    \
        {                                                                                                                       \
            m_##NR3D_R2CF_CNT++;                                                                                                \
        }                                                                                                                       \
        reinterpret_cast<ISP_DIP_X_MDP_TNR_R2C_3_T*>(REG_INFO_VALUE_PTR(DIP_X_MDP_TNR_R2C_3))->NR3D_R2CF_CNT = m_##NR3D_R2CF_CNT;\
    } while (0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_NR3D_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
put(ISP_NVRAM_NR3D_T const& rParam)
{
    PUT_REG_INFO(DIP_X_NR3D_ON_CON,  on_con);
    //PUT_REG_INFO(DIP_X_NR3D_ON_OFF,  on_off);
    //PUT_REG_INFO(DIP_X_NR3D_ON_SIZ,  on_siz);
    PUT_REG_INFO(DIP_X_MDP_TNR_TNR_ENABLE,  tnr_tnr_enable);
    PUT_REG_INFO(DIP_X_MDP_TNR_FLT_CONFIG,  tnr_flt_config);
    PUT_REG_INFO(DIP_X_MDP_TNR_FB_INFO1,  tnr_fb_info1);
    PUT_REG_INFO(DIP_X_MDP_TNR_THR_1,  tnr_thr_1);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_1,  tnr_curve_1);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_2,  tnr_curve_2);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_3,  tnr_curve_3);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_4,  tnr_curve_4);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_5,  tnr_curve_5);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_6,  tnr_curve_6);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_7,  tnr_curve_7);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_8,  tnr_curve_8);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_9,  tnr_curve_9);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_10,  tnr_curve_10);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_11,  tnr_curve_11);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_12,  tnr_curve_12);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_13,  tnr_curve_13);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_14,  tnr_curve_14);
    PUT_REG_INFO(DIP_X_MDP_TNR_CURVE_15,  tnr_curve_15);
    PUT_REG_INFO(DIP_X_MDP_TNR_R2C_1,     tnr_r2c_1);
    PUT_REG_INFO(DIP_X_MDP_TNR_R2C_2,     tnr_r2c_2);
//    PUT_REG_INFO(DIP_X_MDP_TNR_R2C_3,     tnr_r2c_3);

    //PUT_REG_INFO(DIP_X_MDP_TNR_DBG_6,  tnr_dbg_6);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DBG_15,  tnr_dbg_15);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DBG_16,  tnr_dbg_16);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DEMO_1,  tnr_demo_1);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DEMO_2,  tnr_demo_2);
    //PUT_REG_INFO(DIP_X_MDP_TNR_ATPG,  tnr_atpg);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DMY_0,  tnr_dmy_0);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DMY_1,  tnr_dmy_1);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DMY_2,  tnr_dmy_2);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DBG_1,  tnr_dbg_1);
    //PUT_REG_INFO(DIP_X_MDP_TNR_DBG_2,  tnr_dbg_2);
    return  (*this);
}

template <>
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
put(ISP_NVRAM_COLOR_T const& rParam)
{
    PUT_REG_INFO(DIP_X_CAM_COLOR_CFG_MAIN ,color_cfg_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_PXL_CNT_MAIN ,color_pxl_cnt_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_LINE_CNT_MAIN ,color_line_cnt_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_WIN_X_MAIN ,color_win_x_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_WIN_Y_MAIN ,color_win_y_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_TIMING_DETECTION_0 ,color_timing_detection_0);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_TIMING_DETECTION_1 ,color_timing_detection_1);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_DBG_CFG_MAIN ,color_dbg_cfg_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_C_BOOST_MAIN ,color_c_boost_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_C_BOOST_MAIN_2 ,color_c_boost_main_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LUMA_ADJ ,color_luma_adj);
    PUT_REG_INFO(DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1 ,color_g_pic_adj_main_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2 ,color_g_pic_adj_main_2);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_POS_MAIN ,color_pos_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_INK_DATA_MAIN ,color_ink_data_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_INK_DATA_MAIN_CR ,color_ink_data_main_cr);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN ,color_cap_in_data_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR ,color_cap_in_data_main_cr);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN ,color_cap_out_data_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR ,color_cap_out_data_main_cr);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN ,color_y_slope_1_0_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN ,color_y_slope_3_2_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN ,color_y_slope_5_4_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN ,color_y_slope_7_6_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN ,color_y_slope_9_8_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN ,color_y_slope_11_10_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN ,color_y_slope_13_12_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN ,color_y_slope_15_14_main);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LOCAL_HUE_CD_0 ,color_local_hue_cd_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LOCAL_HUE_CD_1 ,color_local_hue_cd_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LOCAL_HUE_CD_2 ,color_local_hue_cd_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LOCAL_HUE_CD_3 ,color_local_hue_cd_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LOCAL_HUE_CD_4 ,color_local_hue_cd_4);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_TWO_D_WINDOW_1 ,color_two_d_window_1);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_TWO_D_W1_RESULT ,color_two_d_w1_result);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN ,color_sat_hist_x_cfg_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN ,color_sat_hist_y_cfg_main);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_BWS_2 ,color_bws_2);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CRC_0 ,color_crc_0);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CRC_1 ,color_crc_1);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CRC_2 ,color_crc_2);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CRC_3 ,color_crc_3);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CRC_4 ,color_crc_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0 ,color_partial_sat_gain1_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1 ,color_partial_sat_gain1_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2 ,color_partial_sat_gain1_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3 ,color_partial_sat_gain1_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4 ,color_partial_sat_gain1_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0 ,color_partial_sat_gain2_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1 ,color_partial_sat_gain2_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2 ,color_partial_sat_gain2_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3 ,color_partial_sat_gain2_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4 ,color_partial_sat_gain2_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0 ,color_partial_sat_gain3_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1 ,color_partial_sat_gain3_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2 ,color_partial_sat_gain3_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3 ,color_partial_sat_gain3_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4 ,color_partial_sat_gain3_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0 ,color_partial_sat_point1_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1 ,color_partial_sat_point1_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2 ,color_partial_sat_point1_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3 ,color_partial_sat_point1_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4 ,color_partial_sat_point1_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0 ,color_partial_sat_point2_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1 ,color_partial_sat_point2_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2 ,color_partial_sat_point2_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3 ,color_partial_sat_point2_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4 ,color_partial_sat_point2_4);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_START ,color_start);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_INTEN ,color_inten);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_INTSTA ,color_intsta);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_OUT_SEL ,color_out_sel);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_FRAME_DONE_DEL ,color_frame_done_del);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CRC ,color_crc);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_SW_SCRATCH ,color_sw_scratch);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CK_ON ,color_ck_on);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH ,color_internal_ip_width);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT ,color_internal_ip_height);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CM1_EN ,color_cm1_en);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_CM2_EN ,color_cm2_en);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_SHADOW_CTRL ,color_shadow_ctrl);
    //PUT_REG_INFO(DIP_X_CAM_COLOR_R0_CRC ,color_r0_crc);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0 ,color_s_gain_by_y0_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1 ,color_s_gain_by_y0_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2 ,color_s_gain_by_y0_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3 ,color_s_gain_by_y0_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4 ,color_s_gain_by_y0_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0 ,color_s_gain_by_y64_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1 ,color_s_gain_by_y64_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2 ,color_s_gain_by_y64_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3 ,color_s_gain_by_y64_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4 ,color_s_gain_by_y64_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0 ,color_s_gain_by_y128_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1 ,color_s_gain_by_y128_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2 ,color_s_gain_by_y128_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3 ,color_s_gain_by_y128_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4 ,color_s_gain_by_y128_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0 ,color_s_gain_by_y192_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1 ,color_s_gain_by_y192_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2 ,color_s_gain_by_y192_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3 ,color_s_gain_by_y192_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4 ,color_s_gain_by_y192_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0 ,color_s_gain_by_y256_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1 ,color_s_gain_by_y256_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2 ,color_s_gain_by_y256_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3 ,color_s_gain_by_y256_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4 ,color_s_gain_by_y256_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LSP_1 ,color_lsp_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_LSP_2 ,color_lsp_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_CONTROL,  color_cm_control);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_HUE_0,  color_cm_w1_hue_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_HUE_1,  color_cm_w1_hue_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_HUE_2,  color_cm_w1_hue_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_HUE_3,  color_cm_w1_hue_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_HUE_4,  color_cm_w1_hue_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_LUMA_0,  color_cm_w1_luma_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_LUMA_1,  color_cm_w1_luma_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_LUMA_2,  color_cm_w1_luma_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_LUMA_3,  color_cm_w1_luma_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_LUMA_4,  color_cm_w1_luma_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_SAT_0,  color_cm_w1_sat_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_SAT_1,  color_cm_w1_sat_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_SAT_2,  color_cm_w1_sat_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_SAT_3,  color_cm_w1_sat_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W1_SAT_4,  color_cm_w1_sat_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_HUE_0,  color_cm_w2_hue_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_HUE_1,  color_cm_w2_hue_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_HUE_2,  color_cm_w2_hue_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_HUE_3,  color_cm_w2_hue_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_HUE_4,  color_cm_w2_hue_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_LUMA_0,  color_cm_w2_luma_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_LUMA_1,  color_cm_w2_luma_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_LUMA_2,  color_cm_w2_luma_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_LUMA_3,  color_cm_w2_luma_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_LUMA_4,  color_cm_w2_luma_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_SAT_0,  color_cm_w2_sat_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_SAT_1,  color_cm_w2_sat_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_SAT_2,  color_cm_w2_sat_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_SAT_3,  color_cm_w2_sat_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W2_SAT_4,  color_cm_w2_sat_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_HUE_0,  color_cm_w3_hue_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_HUE_1,  color_cm_w3_hue_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_HUE_2,  color_cm_w3_hue_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_HUE_3,  color_cm_w3_hue_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_HUE_4,  color_cm_w3_hue_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_LUMA_0,  color_cm_w3_luma_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_LUMA_1,  color_cm_w3_luma_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_LUMA_2,  color_cm_w3_luma_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_LUMA_3,  color_cm_w3_luma_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_LUMA_4,  color_cm_w3_luma_4);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_SAT_0,  color_cm_w3_sat_0);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_SAT_1,  color_cm_w3_sat_1);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_SAT_2,  color_cm_w3_sat_2);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_SAT_3,  color_cm_w3_sat_3);
    PUT_REG_INFO(DIP_X_CAM_COLOR_CM_W3_SAT_4,  color_cm_w3_sat_4);
    return  (*this);
}


template <>
MBOOL
ISP_MGR_NR3D_T::
get(ISP_NVRAM_NR3D_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_CAM_TNR_ENG_CON,  tnr_eng_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_ON_CON,  on_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_ON_OFF,  on_off);
        GET_REG_INFO_BUF(DIP_X_NR3D_ON_SIZ,  on_siz);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_TNR_ENABLE,  tnr_tnr_enable);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_FLT_CONFIG,  tnr_flt_config);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_FB_INFO1,  tnr_fb_info1);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_THR_1,  tnr_thr_1);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_1,  tnr_curve_1);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_2,  tnr_curve_2);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_3,  tnr_curve_3);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_4,  tnr_curve_4);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_5,  tnr_curve_5);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_6,  tnr_curve_6);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_7,  tnr_curve_7);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_8,  tnr_curve_8);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_9,  tnr_curve_9);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_10,  tnr_curve_10);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_11,  tnr_curve_11);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_12,  tnr_curve_12);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_13,  tnr_curve_13);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_14,  tnr_curve_14);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_CURVE_15,  tnr_curve_15);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_6,  tnr_dbg_6);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_15,  tnr_dbg_15);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_16,  tnr_dbg_16);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DEMO_1,  tnr_demo_1);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DEMO_2,  tnr_demo_2);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_ATPG,  tnr_atpg);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DMY_0,  tnr_dmy_0);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_17,  tnr_dbg_17);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_18,  tnr_dbg_18);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_1,  tnr_dbg_1);
        GET_REG_INFO_BUF(DIP_X_MDP_TNR_DBG_2,  tnr_dbg_2);
    }

    return MTRUE;
}

template <>
MBOOL
ISP_MGR_NR3D_T::
get(ISP_NVRAM_COLOR_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CFG_MAIN ,color_cfg_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PXL_CNT_MAIN,color_pxl_cnt_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LINE_CNT_MAIN,color_line_cnt_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_WIN_X_MAIN,color_win_x_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_WIN_Y_MAIN,color_win_y_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_TIMING_DETECTION_0,color_timing_detection_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_TIMING_DETECTION_1,color_timing_detection_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_DBG_CFG_MAIN,color_dbg_cfg_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_C_BOOST_MAIN,color_c_boost_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_C_BOOST_MAIN_2,color_c_boost_main_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LUMA_ADJ,color_luma_adj);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_1,color_g_pic_adj_main_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_G_PIC_ADJ_MAIN_2,color_g_pic_adj_main_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_POS_MAIN,color_pos_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_INK_DATA_MAIN,color_ink_data_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_INK_DATA_MAIN_CR,color_ink_data_main_cr);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN,color_cap_in_data_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CAP_IN_DATA_MAIN_CR,color_cap_in_data_main_cr);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN,color_cap_out_data_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CAP_OUT_DATA_MAIN_CR,color_cap_out_data_main_cr);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_1_0_MAIN,color_y_slope_1_0_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_3_2_MAIN,color_y_slope_3_2_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_5_4_MAIN,color_y_slope_5_4_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_7_6_MAIN,color_y_slope_7_6_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_9_8_MAIN,color_y_slope_9_8_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_11_10_MAIN,color_y_slope_11_10_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_13_12_MAIN,color_y_slope_13_12_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_Y_SLOPE_15_14_MAIN,color_y_slope_15_14_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LOCAL_HUE_CD_0,color_local_hue_cd_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LOCAL_HUE_CD_1,color_local_hue_cd_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LOCAL_HUE_CD_2,color_local_hue_cd_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LOCAL_HUE_CD_3,color_local_hue_cd_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LOCAL_HUE_CD_4,color_local_hue_cd_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_TWO_D_WINDOW_1,color_two_d_window_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_TWO_D_W1_RESULT,color_two_d_w1_result);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_SAT_HIST_X_CFG_MAIN,color_sat_hist_x_cfg_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_SAT_HIST_Y_CFG_MAIN,color_sat_hist_y_cfg_main);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_BWS_2,color_bws_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CRC_0,color_crc_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CRC_1,color_crc_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CRC_2,color_crc_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CRC_3,color_crc_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CRC_4,color_crc_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_0,color_partial_sat_gain1_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_1,color_partial_sat_gain1_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_2,color_partial_sat_gain1_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_3,color_partial_sat_gain1_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN1_4,color_partial_sat_gain1_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_0,color_partial_sat_gain2_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_1,color_partial_sat_gain2_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_2,color_partial_sat_gain2_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_3,color_partial_sat_gain2_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN2_4,color_partial_sat_gain2_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_0,color_partial_sat_gain3_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_1,color_partial_sat_gain3_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_2,color_partial_sat_gain3_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_3,color_partial_sat_gain3_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_GAIN3_4,color_partial_sat_gain3_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_0,color_partial_sat_point1_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_1,color_partial_sat_point1_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_2,color_partial_sat_point1_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_3,color_partial_sat_point1_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT1_4,color_partial_sat_point1_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_0,color_partial_sat_point2_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_1,color_partial_sat_point2_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_2,color_partial_sat_point2_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_3,color_partial_sat_point2_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_PARTIAL_SAT_POINT2_4,color_partial_sat_point2_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_START,color_start);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_INTEN,color_inten);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_INTSTA,color_intsta);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_OUT_SEL,color_out_sel);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_FRAME_DONE_DEL,color_frame_done_del);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CRC,color_crc);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_SW_SCRATCH,color_sw_scratch);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CK_ON,color_ck_on);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_INTERNAL_IP_WIDTH,color_internal_ip_width);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_INTERNAL_IP_HEIGHT,color_internal_ip_height);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM1_EN,color_cm1_en);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM2_EN,color_cm2_en);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_SHADOW_CTRL,color_shadow_ctrl);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_R0_CRC,color_r0_crc);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_0,color_s_gain_by_y0_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_1,color_s_gain_by_y0_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_2,color_s_gain_by_y0_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_3,color_s_gain_by_y0_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y0_4,color_s_gain_by_y0_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_0,color_s_gain_by_y64_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_1,color_s_gain_by_y64_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_2,color_s_gain_by_y64_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_3,color_s_gain_by_y64_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y64_4,color_s_gain_by_y64_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_0,color_s_gain_by_y128_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_1,color_s_gain_by_y128_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_2,color_s_gain_by_y128_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_3,color_s_gain_by_y128_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y128_4,color_s_gain_by_y128_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_0,color_s_gain_by_y192_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_1,color_s_gain_by_y192_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_2,color_s_gain_by_y192_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_3,color_s_gain_by_y192_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y192_4,color_s_gain_by_y192_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_0,color_s_gain_by_y256_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_1,color_s_gain_by_y256_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_2,color_s_gain_by_y256_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_3,color_s_gain_by_y256_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_S_GAIN_BY_Y256_4,color_s_gain_by_y256_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LSP_1,color_lsp_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_LSP_2,color_lsp_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_CONTROL,  color_cm_control);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_HUE_0,  color_cm_w1_hue_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_HUE_1,  color_cm_w1_hue_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_HUE_2,  color_cm_w1_hue_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_HUE_3,  color_cm_w1_hue_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_HUE_4,  color_cm_w1_hue_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_LUMA_0,  color_cm_w1_luma_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_LUMA_1,  color_cm_w1_luma_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_LUMA_2,  color_cm_w1_luma_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_LUMA_3,  color_cm_w1_luma_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_LUMA_4,  color_cm_w1_luma_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_SAT_0,  color_cm_w1_sat_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_SAT_1,  color_cm_w1_sat_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_SAT_2,  color_cm_w1_sat_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_SAT_3,  color_cm_w1_sat_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W1_SAT_4,  color_cm_w1_sat_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_HUE_0,  color_cm_w2_hue_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_HUE_1,  color_cm_w2_hue_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_HUE_2,  color_cm_w2_hue_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_HUE_3,  color_cm_w2_hue_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_HUE_4,  color_cm_w2_hue_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_LUMA_0,  color_cm_w2_luma_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_LUMA_1,  color_cm_w2_luma_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_LUMA_2,  color_cm_w2_luma_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_LUMA_3,  color_cm_w2_luma_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_LUMA_4,  color_cm_w2_luma_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_SAT_0,  color_cm_w2_sat_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_SAT_1,  color_cm_w2_sat_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_SAT_2,  color_cm_w2_sat_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_SAT_3,  color_cm_w2_sat_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W2_SAT_4,  color_cm_w2_sat_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_HUE_0,  color_cm_w3_hue_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_HUE_1,  color_cm_w3_hue_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_HUE_2,  color_cm_w3_hue_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_HUE_3,  color_cm_w3_hue_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_HUE_4,  color_cm_w3_hue_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_LUMA_0,  color_cm_w3_luma_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_LUMA_1,  color_cm_w3_luma_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_LUMA_2,  color_cm_w3_luma_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_LUMA_3,  color_cm_w3_luma_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_LUMA_4,  color_cm_w3_luma_4);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_SAT_0,  color_cm_w3_sat_0);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_SAT_1,  color_cm_w3_sat_1);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_SAT_2,  color_cm_w3_sat_2);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_SAT_3,  color_cm_w3_sat_3);
        GET_REG_INFO_BUF(DIP_X_CAM_COLOR_CM_W3_SAT_4,  color_cm_w3_sat_4);

    }
    return MTRUE;
}


MBOOL
ISP_MGR_NR3D_T::
apply(EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
    MBOOL bNR3D_EN = ( isNr3dEnable() | isColorEnable());//= isEnable();

    reinterpret_cast<ISP_DIP_X_CAM_TNR_ENG_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_CAM_TNR_ENG_CON))->NR3D_CAM_TNR_EN = isNr3dEnable();
    reinterpret_cast<ISP_DIP_X_CAM_COLOR_START_T*>(REG_INFO_VALUE_PTR(DIP_X_CAM_COLOR_START))->COLOR_DISP_COLOR_START = isColorEnable();

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, NR3D_EN, bNR3D_EN);

    //R2C calculation
    ADD_AND_WRITE_R2CF_CNT(NR3D_R2C_VAL1, NR3D_R2CF_CNT1);
    ADD_AND_WRITE_R2CF_CNT(NR3D_R2C_VAL2, NR3D_R2CF_CNT2);
    ADD_AND_WRITE_R2CF_CNT(NR3D_R2C_VAL3, NR3D_R2CF_CNT3);
    ADD_AND_WRITE_R2CF_CNT(NR3D_R2C_VAL4, NR3D_R2CF_CNT4);

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("NR3D");

    return  MTRUE;
}

static MVOID demoNR3D(MBOOL bEnableSL2E, MRect &fullImg, MRect &onRegion)
{
    CAM_LOGD("demo NR3D_EN(1), SL2E_EN(%d), x,y,w,h = (%d,%d,%d,%d), full x,y,w,h = (%d,%d,%d,%d)",
    bEnableSL2E, onRegion.p.x, onRegion.p.y, onRegion.s.w, onRegion.s.h,
    fullImg.p.x, fullImg.p.y, fullImg.s.w, fullImg.s.h);

    if (onRegion.p.x == 0)
    {
        onRegion.s.w =
            (onRegion.s.w >= fullImg.s.w/2)
            ? fullImg.s.w/2
            : onRegion.s.w;
        onRegion.s.w &= ~1;
    }
    else
    {
        MINT32 tmpVal = fullImg.s.w/2 - onRegion.p.x;
        if (tmpVal >= 0)
        {
            onRegion.s.w = tmpVal;
            onRegion.s.w &= ~1;
        }
        else
        {
            onRegion.s.w = 0;
        }
    }
}

/* strong setting */
/*
adb shell setprop debug.camera.3dnr.level 1
adb shell setprop debug.nr3d.bm.enable 1
adb shell setprop debug.nr3d.bm.q_nl 32
adb shell setprop debug.nr3d.bm.q_sp 0
*/
/* weak setting */
/*
adb shell setprop debug.camera.3dnr.level 1
adb shell setprop debug.nr3d.bm.enable 1
adb shell setprop debug.nr3d.bm.q_nl 12
adb shell setprop debug.nr3d.bm.q_sp 12
*/
static MVOID benchmarkNR3DRegValue(MVOID *pReg)
{
    if (pReg == NULL)
    {
        return;
    }
    dip_x_reg_t *pIspPhyReg = (dip_x_reg_t*) pReg;
    static MINT32 bmCount = 0;
    bmCount++;
    // NR3D_FLT_STR_MAX
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_FLT_STR_MAX = ::property_get_int32("vendor.debug.nr3d.bm.flt_str_max", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_FLT_STR_MAX);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_FLT_STR_MAX =%d", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_FLT_STR_MAX);
    }
    // NR3D_BDI_THR
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_BDI_THR = ::property_get_int32("vendor.debug.nr3d.bm.bdi_thr", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_BDI_THR);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_BDN_THR =%d", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_BDI_THR);
    }
    // NR3D_MV_PEN_W
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_W = ::property_get_int32("vendor.debug.nr3d.bm.mv_pen_w", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_W);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_MV_PEN_W =%d", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_W);
    }
    // NR3D_MV_PEN_THR
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_THR = ::property_get_int32("vendor.debug.nr3d.bm.mv_pen_thr", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_THR);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_MV_PEN_THR =%d", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_THR);
    }
    // NR3D_Q_NL
    pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_NL= ::property_get_int32("vendor.debug.nr3d.bm.q_nl",pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_NL);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_NL =%d", pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_NL);
    }
    // NR3D_Q_SP
    pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_SP= ::property_get_int32("vendor.debug.nr3d.bm.q_sp",pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_SP);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_SP =%d", pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_SP);
    }
    // NR3D_BLEND_RATIO_BLKY
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_BLKY= ::property_get_int32("vendor.debug.nr3d.bm.br_blky", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_BLKY);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_BLEND_RATIO_BLKY =%d", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_BLKY);
    }
    // NR3D_BLEND_RATIO_DE
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_DE= ::property_get_int32("vendor.debug.nr3d.bm.br_de",pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_DE);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_BLEND_RATIO_DE =%d", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_DE);
    }
    // NR3D_BLEND_RATIO_TXTR
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_TXTR= ::property_get_int32("vendor.debug.nr3d.bm.br_txtr",pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_TXTR );
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_BLEND_RATIO_TXTR =%d", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_TXTR);
    }
    // NR3D_BLEND_RATIO_MV
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_MV= ::property_get_int32("vendor.debug.nr3d.bm.br_mv",pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_MV );
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_BLEND_RATIO_MV =%d", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_MV);
    }

    char tmpStr[PROPERTY_VALUE_MAX];
    MINT32 ADB_NR3D_CUR_SEL_VAL=0;
    ADB_NR3D_CUR_SEL_VAL= ::property_get_int32("vendor.debug.nr3d.bm.curve_sel_val", ADB_NR3D_CUR_SEL_VAL);
    MINT32 CMD_NR3D_CURSEL_OFS = 1000;
    MINT32 CMD_NR3D_CURSEL = (ADB_NR3D_CUR_SEL_VAL/CMD_NR3D_CURSEL_OFS);//Type
    MINT32 CMD_NR3D_CTRPNT_OFS = 100;
    MINT32 CMD_NR3D_CTRPNT = (ADB_NR3D_CUR_SEL_VAL - (CMD_NR3D_CURSEL * CMD_NR3D_CURSEL_OFS)) / CMD_NR3D_CTRPNT_OFS;//Point
    MINT32 CMD_NR3D_CTRPNT_VAL = ADB_NR3D_CUR_SEL_VAL - CMD_NR3D_CURSEL * CMD_NR3D_CURSEL_OFS - CMD_NR3D_CTRPNT * CMD_NR3D_CTRPNT_OFS;
    if (CMD_NR3D_CURSEL==1)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==2)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_BLKC_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==3)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==4)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DE1_BASE_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==5)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==6)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==7)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_WVAR_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_WVAR_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_11.Bits.NR3D_Q_WVAR_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_11.Bits.NR3D_Q_WVAR_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_11.Bits.NR3D_Q_WVAR_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_11.Bits.NR3D_Q_WVAR_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_11.Bits.NR3D_Q_WVAR_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_12.Bits.NR3D_Q_WVAR_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==8)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_12.Bits.NR3D_Q_WSM_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_12.Bits.NR3D_Q_WSM_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_12.Bits.NR3D_Q_WSM_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_12.Bits.NR3D_Q_WSM_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_13.Bits.NR3D_Q_WSM_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_13.Bits.NR3D_Q_WSM_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_13.Bits.NR3D_Q_WSM_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_13.Bits.NR3D_Q_WSM_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
    }
    else if (CMD_NR3D_CURSEL==9)
    {
        if (CMD_NR3D_CTRPNT==0)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y0 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==1)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y1 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==2)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y2 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==3)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y3 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==4)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y4 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==5)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y5 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==6)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y6 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==7)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y7 = CMD_NR3D_CTRPNT_VAL;
        }
        else if (CMD_NR3D_CTRPNT==8)
        {
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y8 = CMD_NR3D_CTRPNT_VAL;
        }
    }


	//start of WHITNEY E2 ONLY (registers not exist in E1)
    pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2CENC = ::property_get_int32("vendor.debug.nr3d.bm.r2cenc", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2CENC);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_R2CENC =%d", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2CENC);
    }

	MINT32 ADB_NR3D_R2C_VAL14=
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL1+
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL2*100+
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL3*10000+
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL4*1000000;


	ADB_NR3D_R2C_VAL14= ::property_get_int32("vendor.debug.nr3d.bm.r2cval14", ADB_NR3D_R2C_VAL14);//44332211

	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL1=ADB_NR3D_R2C_VAL14%100;
	ADB_NR3D_R2C_VAL14=ADB_NR3D_R2C_VAL14/100;
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL2=ADB_NR3D_R2C_VAL14%100;
	ADB_NR3D_R2C_VAL14=ADB_NR3D_R2C_VAL14/100;
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL3=ADB_NR3D_R2C_VAL14%100;
	ADB_NR3D_R2C_VAL14=ADB_NR3D_R2C_VAL14/100;
	pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL4=ADB_NR3D_R2C_VAL14%100;

	//end of WHITNEY E2 ONLY
	//************************************
    MINT32 ADB_NR3D_QBLKY_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y3*1000000;
	//33221100
	//Y3Y2Y1Y0

	MINT32 ADB_NR3D_QBLKY_47=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y4+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y7*1000000;

    MINT32 ADB_NR3D_QBLKC_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y3*1000000;

	MINT32 ADB_NR3D_QBLKC_47=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y4+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_BLKC_Y7*1000000;

    MINT32 ADB_NR3D_DTXRLV_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y3*1000000;

	MINT32 ADB_NR3D_DTXRLV_47=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y4+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y7*1000000;

	MINT32 ADB_NR3D_D1B_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DE1_BASE_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y3*1000000;

	MINT32 ADB_NR3D_D1B_47=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y4+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y7*1000000;

    MINT32 ADB_NR3D_D2TXRB_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y3*1000000;

	MINT32 ADB_NR3D_D2TXRB_47=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y4+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y7*1000000;

    MINT32 ADB_NR3D_MV_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y3*1000000;

	MINT32 ADB_NR3D_MV_47=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y4 +
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y7*1000000;

    MINT32 ADB_NR3D_SDL_03=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y0+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y1*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y2*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y3*1000000;

	MINT32 ADB_NR3D_SDL_48=
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y4+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y5*100+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y6*10000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y7*1000000+
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y8*100000000;

    ADB_NR3D_QBLKY_03= ::property_get_int32("vendor.debug.nr3d.bm.blky03", ADB_NR3D_QBLKY_03);
	ADB_NR3D_QBLKY_47= ::property_get_int32("vendor.debug.nr3d.bm.blky47", ADB_NR3D_QBLKY_47);
    ADB_NR3D_QBLKC_03= ::property_get_int32("vendor.debug.nr3d.bm.blkc03", ADB_NR3D_QBLKC_03);
	ADB_NR3D_QBLKC_47= ::property_get_int32("vendor.debug.nr3d.bm.blkc47", ADB_NR3D_QBLKC_47);
    ADB_NR3D_DTXRLV_03= ::property_get_int32("vendor.debug.nr3d.bm.dtxrlv03", ADB_NR3D_DTXRLV_03);
	ADB_NR3D_DTXRLV_47= ::property_get_int32("vendor.debug.nr3d.bm.dtxrlv47", ADB_NR3D_DTXRLV_47);
    ADB_NR3D_D1B_03= ::property_get_int32("vendor.debug.nr3d.bm.d1b03", ADB_NR3D_D1B_03);
	ADB_NR3D_D1B_47= ::property_get_int32("vendor.debug.nr3d.bm.d1b47", ADB_NR3D_D1B_47);
    ADB_NR3D_D2TXRB_03= ::property_get_int32("vendor.debug.nr3d.bm.d2txrb03", ADB_NR3D_D2TXRB_03);
	ADB_NR3D_D2TXRB_47= ::property_get_int32("vendor.debug.nr3d.bm.d2txrb47", ADB_NR3D_D2TXRB_47);
    ADB_NR3D_MV_03= ::property_get_int32("vendor.debug.nr3d.bm.mv03", ADB_NR3D_MV_03);
	ADB_NR3D_MV_47= ::property_get_int32("vendor.debug.nr3d.bm.mv47", ADB_NR3D_MV_47);
    ADB_NR3D_SDL_03= ::property_get_int32("vendor.debug.nr3d.bm.sdl03", ADB_NR3D_SDL_03);
	ADB_NR3D_SDL_48= ::property_get_int32("vendor.debug.nr3d.bm.sdl47", ADB_NR3D_SDL_48);


//1.BLKY
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y0=ADB_NR3D_QBLKY_03%100;
	ADB_NR3D_QBLKY_03=ADB_NR3D_QBLKY_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y1=ADB_NR3D_QBLKY_03%100;
	ADB_NR3D_QBLKY_03=ADB_NR3D_QBLKY_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y2=ADB_NR3D_QBLKY_03%100;
	ADB_NR3D_QBLKY_03=ADB_NR3D_QBLKY_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y3=ADB_NR3D_QBLKY_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y4=ADB_NR3D_QBLKY_47%100;
	ADB_NR3D_QBLKY_47=ADB_NR3D_QBLKY_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y5=ADB_NR3D_QBLKY_47%100;
	ADB_NR3D_QBLKY_47=ADB_NR3D_QBLKY_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y6=ADB_NR3D_QBLKY_47%100;
	ADB_NR3D_QBLKY_47=ADB_NR3D_QBLKY_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y7=ADB_NR3D_QBLKY_47%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_BLKY_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y0,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y1,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y2,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y3,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y4,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y5,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y6,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y7
            );
    }

//2.BLKC
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y0=ADB_NR3D_QBLKC_03%100;
	ADB_NR3D_QBLKC_03=ADB_NR3D_QBLKC_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y1=ADB_NR3D_QBLKC_03%100;
	ADB_NR3D_QBLKC_03=ADB_NR3D_QBLKC_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y2=ADB_NR3D_QBLKC_03%100;
	ADB_NR3D_QBLKC_03=ADB_NR3D_QBLKC_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y3=ADB_NR3D_QBLKC_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y4=ADB_NR3D_QBLKC_47%100;
	ADB_NR3D_QBLKC_47=ADB_NR3D_QBLKC_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y5=ADB_NR3D_QBLKC_47%100;
	ADB_NR3D_QBLKC_47=ADB_NR3D_QBLKC_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y6=ADB_NR3D_QBLKC_47%100;
	ADB_NR3D_QBLKC_47=ADB_NR3D_QBLKC_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_BLKC_Y7=ADB_NR3D_QBLKC_47%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_BLKC_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y0,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y1,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y2,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y3,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y4,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y5,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y6,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_BLKC_Y7
            );
    }

//3.DETXRLV
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y0=ADB_NR3D_DTXRLV_03%100;
	ADB_NR3D_DTXRLV_03=ADB_NR3D_DTXRLV_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y1=ADB_NR3D_DTXRLV_03%100;
	ADB_NR3D_DTXRLV_03=ADB_NR3D_DTXRLV_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y2=ADB_NR3D_DTXRLV_03%100;
	ADB_NR3D_DTXRLV_03=ADB_NR3D_DTXRLV_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y3=ADB_NR3D_DTXRLV_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y4=ADB_NR3D_DTXRLV_47%100;
	ADB_NR3D_DTXRLV_47=ADB_NR3D_DTXRLV_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y5=ADB_NR3D_DTXRLV_47%100;
	ADB_NR3D_DTXRLV_47=ADB_NR3D_DTXRLV_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y6=ADB_NR3D_DTXRLV_47%100;
	ADB_NR3D_DTXRLV_47=ADB_NR3D_DTXRLV_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y7=ADB_NR3D_DTXRLV_47%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_DETXTR_LVL_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y0,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y1,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y2,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y3,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y4,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y5,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y6,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y7
            );
    }

//4.DE1BASE
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DE1_BASE_Y0=ADB_NR3D_D1B_03%100;
	ADB_NR3D_D1B_03=ADB_NR3D_D1B_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y1=ADB_NR3D_D1B_03%100;
	ADB_NR3D_D1B_03=ADB_NR3D_D1B_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y2=ADB_NR3D_D1B_03%100;
	ADB_NR3D_D1B_03=ADB_NR3D_D1B_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y3=ADB_NR3D_D1B_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y4=ADB_NR3D_D1B_47%100;
	ADB_NR3D_D1B_47=ADB_NR3D_D1B_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y5=ADB_NR3D_D1B_47%100;
	ADB_NR3D_D1B_47=ADB_NR3D_D1B_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y6=ADB_NR3D_D1B_47%100;
	ADB_NR3D_D1B_47=ADB_NR3D_D1B_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y7=ADB_NR3D_D1B_47%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_DE1_BASE_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DE1_BASE_Y0,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y1,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y2,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y3,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y4,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y5,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y6,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y7
            );
    }

//5.D2TXRB
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y0=ADB_NR3D_D2TXRB_03%100;
	ADB_NR3D_D2TXRB_03=ADB_NR3D_D2TXRB_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y1=ADB_NR3D_D2TXRB_03%100;
	ADB_NR3D_D2TXRB_03=ADB_NR3D_D2TXRB_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y2=ADB_NR3D_D2TXRB_03%100;
	ADB_NR3D_D2TXRB_03=ADB_NR3D_D2TXRB_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y3=ADB_NR3D_D2TXRB_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y4=ADB_NR3D_D2TXRB_47%100;
	ADB_NR3D_D2TXRB_47=ADB_NR3D_D2TXRB_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y5=ADB_NR3D_D2TXRB_47%100;
	ADB_NR3D_D2TXRB_47=ADB_NR3D_D2TXRB_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y6=ADB_NR3D_D2TXRB_47%100;
	ADB_NR3D_D2TXRB_47=ADB_NR3D_D2TXRB_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y7=ADB_NR3D_D2TXRB_47%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_DE2TXTR_BASE_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y0,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y1,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y2,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y3,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y4,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y5,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y6,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y7
            );
    }

//6.MV
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y0=ADB_NR3D_MV_03%100;
	ADB_NR3D_MV_03=ADB_NR3D_MV_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y1=ADB_NR3D_MV_03%100;
	ADB_NR3D_MV_03=ADB_NR3D_MV_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y2=ADB_NR3D_MV_03%100;
	ADB_NR3D_MV_03=ADB_NR3D_MV_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y3=ADB_NR3D_MV_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y4=ADB_NR3D_MV_47%100;
	ADB_NR3D_MV_47=ADB_NR3D_MV_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y5=ADB_NR3D_MV_47%100;
	ADB_NR3D_MV_47=ADB_NR3D_MV_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y6=ADB_NR3D_MV_47%100;
	ADB_NR3D_MV_47=ADB_NR3D_MV_47/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y7=ADB_NR3D_MV_47%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_MV_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y0,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y1,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y2,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y3,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y4,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y5,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y6,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y7
            );
    }


//7.SDL
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y0=ADB_NR3D_SDL_03%100;
	ADB_NR3D_SDL_03=ADB_NR3D_SDL_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y1=ADB_NR3D_SDL_03%100;
	ADB_NR3D_SDL_03=ADB_NR3D_SDL_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y2=ADB_NR3D_SDL_03%100;
	ADB_NR3D_SDL_03=ADB_NR3D_SDL_03/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y3=ADB_NR3D_SDL_03%100;

	pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y4=ADB_NR3D_SDL_48%100;
	ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y5=ADB_NR3D_SDL_48%100;
	ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y6=ADB_NR3D_SDL_48%100;
	ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y7=ADB_NR3D_SDL_48%100;
	ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
	pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y8=ADB_NR3D_SDL_48%100;

    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_Q_SDL_Y0: %d, Y1: %d, Y2: %d, Y3: %d, Y4: %d, Y5: %d, Y6: %d, Y7: %d",
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y0,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y1,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y2,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y3,

            pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y4,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y5,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y6,
            pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y7
            );
    }


    // NR3D_SL2_OFF
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF = ::property_get_int32("vendor.debug.nr3d.bm.sl2_off", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_SL2_OFF =%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF);
    }

    // NR3D_INK_EN
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_EN = ::property_get_int32("vendor.debug.nr3d.bm.ink_en", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_EN);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_INK_EN =%d", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_EN);
    }
    // NR3D_INK_LEVEL_DISP
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_LEVEL_DISP = ::property_get_int32("vendor.debug.nr3d.bm.ink_level_disp", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_LEVEL_DISP);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_INK_LEVEL_DISP =%d", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_LEVEL_DISP);
    }
    // NR3D_INK_SEL
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_SEL = ::property_get_int32("vendor.debug.nr3d.bm.ink_sel", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_SEL);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_INK_SEL =%d", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_SEL);
    }

    //////////////start of 20170316

    pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_RESET = ::property_get_int32("vendor.debug.nr3d.cam_tnr_reset", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_RESET);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_CAM_TNR_RESET =%d", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_RESET);
    }

    pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN = ::property_get_int32("vendor.debug.nr3d.cam_tnr_en", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_CAM_TNR_EN =%d", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN);
    }

    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF = ::property_get_int32("vendor.debug.nr3d.sl2_off", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_SL2_OFF =%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF );
    }

    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_ON_EN = ::property_get_int32("vendor.debug.nr3d.on_en", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_ON_EN);
    pIspPhyReg->DIP_X_MDP_TNR_TNR_ENABLE.Bits.NR3D_TNR_Y_EN = ::property_get_int32("vendor.debug.nr3d.tnr_y_en", pIspPhyReg->DIP_X_MDP_TNR_TNR_ENABLE.Bits.NR3D_TNR_Y_EN);
    pIspPhyReg->DIP_X_MDP_TNR_TNR_ENABLE.Bits.NR3D_TNR_C_EN = ::property_get_int32("vendor.debug.nr3d.tnr_c_en", pIspPhyReg->DIP_X_MDP_TNR_TNR_ENABLE.Bits.NR3D_TNR_C_EN);
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_BLKY = ::property_get_int32("vendor.debug.nr3d.blend_ratio_blky", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_BLKY);
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_DE = ::property_get_int32("vendor.debug.nr3d.blend_ratio_de", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_DE);
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_TXTR = ::property_get_int32("vendor.debug.nr3d.blend_ratio_txtr", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_TXTR);
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_MV = ::property_get_int32("vendor.debug.nr3d.blend_ratio_mv", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_BLEND_RATIO_MV);
    pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_FLT_STR_MAX = ::property_get_int32("vendor.debug.nr3d.flt_str_max", pIspPhyReg->DIP_X_MDP_TNR_FLT_CONFIG.Bits.NR3D_FLT_STR_MAX);
    pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_NL = ::property_get_int32("vendor.debug.nr3d.q_nl", pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_NL);
    pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_SP = ::property_get_int32("vendor.debug.nr3d.q_sp", pIspPhyReg->DIP_X_MDP_TNR_FB_INFO1.Bits.NR3D_Q_SP);
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_BDI_THR = ::property_get_int32("vendor.debug.nr3d.bdi_thr", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_BDI_THR);
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_W = ::property_get_int32("vendor.debug.nr3d.mv_pen_w", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_W);
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_THR = ::property_get_int32("vendor.debug.nr3d.mv_pen_thr", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_MV_PEN_THR);
    pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_SMALL_SAD_THR = ::property_get_int32("vendor.debug.nr3d.small_sad_thr", pIspPhyReg->DIP_X_MDP_TNR_THR_1.Bits.NR3D_SMALL_SAD_THR);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y0 = ::property_get_int32("vendor.debug.nr3d.q_blky_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y1 = ::property_get_int32("vendor.debug.nr3d.q_blky_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y2 = ::property_get_int32("vendor.debug.nr3d.q_blky_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y3 = ::property_get_int32("vendor.debug.nr3d.q_blky_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y4 = ::property_get_int32("vendor.debug.nr3d.q_blky_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_1.Bits.NR3D_Q_BLKY_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y5 = ::property_get_int32("vendor.debug.nr3d.q_blky_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y6 = ::property_get_int32("vendor.debug.nr3d.q_blky_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y7 = ::property_get_int32("vendor.debug.nr3d.q_blky_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKY_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y0 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y1 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_2.Bits.NR3D_Q_BLKC_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y2 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y3 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y4 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y5 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y6 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_3.Bits.NR3D_Q_BLKC_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_BLKC_Y7 = ::property_get_int32("vendor.debug.nr3d.q_blkc_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_BLKC_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y0 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y1 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y2 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y3 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_4.Bits.NR3D_Q_DETXTR_LVL_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y4 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y5 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y6 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y7 = ::property_get_int32("vendor.debug.nr3d.q_detxtr_lvl_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DETXTR_LVL_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DE1_BASE_Y0 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_5.Bits.NR3D_Q_DE1_BASE_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y1 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y2 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y3 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y4 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y5 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_6.Bits.NR3D_Q_DE1_BASE_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y6 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y7 = ::property_get_int32("vendor.debug.nr3d.q_de1_base_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE1_BASE_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y0 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y1 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y2 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_7.Bits.NR3D_Q_DE2TXTR_BASE_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y3 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y4 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y5 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y6 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y7 = ::property_get_int32("vendor.debug.nr3d.q_de2txtr_base_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_8.Bits.NR3D_Q_DE2TXTR_BASE_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y0 = ::property_get_int32("vendor.debug.nr3d.q_mv_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y1 = ::property_get_int32("vendor.debug.nr3d.q_mv_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y2 = ::property_get_int32("vendor.debug.nr3d.q_mv_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y3 = ::property_get_int32("vendor.debug.nr3d.q_mv_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y4 = ::property_get_int32("vendor.debug.nr3d.q_mv_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_9.Bits.NR3D_Q_MV_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y5 = ::property_get_int32("vendor.debug.nr3d.q_mv_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y6 = ::property_get_int32("vendor.debug.nr3d.q_mv_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y7 = ::property_get_int32("vendor.debug.nr3d.q_mv_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_10.Bits.NR3D_Q_MV_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y0 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y0", pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y0);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y1 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y1", pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y1);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y2 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y2", pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y2);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y3 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y3", pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y3);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y4 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y4", pIspPhyReg->DIP_X_MDP_TNR_CURVE_14.Bits.NR3D_Q_SDL_Y4);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y5 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y5", pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y5);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y6 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y6", pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y6);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y7 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y7", pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y7);
    pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y8 = ::property_get_int32("vendor.debug.nr3d.q_sdl_y8", pIspPhyReg->DIP_X_MDP_TNR_CURVE_15.Bits.NR3D_Q_SDL_Y8);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL1 = ::property_get_int32("vendor.debug.nr3d.r2c_val1", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL1);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL2 = ::property_get_int32("vendor.debug.nr3d.r2c_val2", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL2);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL3 = ::property_get_int32("vendor.debug.nr3d.r2c_val3", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL3);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL4 = ::property_get_int32("vendor.debug.nr3d.r2c_val4", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2C_VAL4);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2CENC = ::property_get_int32("vendor.debug.nr3d.r2cenc", pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2CENC);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR1 = ::property_get_int32("vendor.debug.nr3d.r2c_txtr_thr1", pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR1);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR2 = ::property_get_int32("vendor.debug.nr3d.r2c_txtr_thr2", pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR2);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR3 = ::property_get_int32("vendor.debug.nr3d.r2c_txtr_thr3", pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR3);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR4 = ::property_get_int32("vendor.debug.nr3d.r2c_txtr_thr4", pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THR4);
    pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THROFF = ::property_get_int32("vendor.debug.nr3d.r2c_txtr_throff", pIspPhyReg->DIP_X_MDP_TNR_R2C_2.Bits.NR3D_R2C_TXTR_THROFF);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_FORCE_FLT_STR = ::property_get_int32("vendor.debug.nr3d.force_flt_str", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_FORCE_FLT_STR);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_EN = ::property_get_int32("vendor.debug.nr3d.ink_en", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_EN);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_INK_EN =%d", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_EN );
    }

    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_Y_EN = ::property_get_int32("vendor.debug.nr3d.ink_y_en", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_Y_EN);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_SEL = ::property_get_int32("vendor.debug.nr3d.ink_sel", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_SEL);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_LEVEL_DISP = ::property_get_int32("vendor.debug.nr3d.ink_level_disp", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_INK_LEVEL_DISP);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_FORCE_EN = ::property_get_int32("vendor.debug.nr3d.force_en", pIspPhyReg->DIP_X_MDP_TNR_DBG_6.Bits.NR3D_FORCE_EN);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_EN = ::property_get_int32("vendor.debug.nr3d.osd_en", pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_EN);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_SEL = ::property_get_int32("vendor.debug.nr3d.osd_sel", pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_SEL);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_TARGH = ::property_get_int32("vendor.debug.nr3d.osd_targh", pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_TARGH);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_TARGV = ::property_get_int32("vendor.debug.nr3d.osd_targv", pIspPhyReg->DIP_X_MDP_TNR_DBG_15.Bits.NR3D_OSD_TARGV);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_DISPH = ::property_get_int32("vendor.debug.nr3d.osd_disph", pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_DISPH);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_DISPV = ::property_get_int32("vendor.debug.nr3d.osd_dispv", pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_DISPV);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_DISP_SCALE = ::property_get_int32("vendor.debug.nr3d.osd_disp_scale", pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_DISP_SCALE);
    pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_Y_EN = ::property_get_int32("vendor.debug.nr3d.osd_y_en", pIspPhyReg->DIP_X_MDP_TNR_DBG_16.Bits.NR3D_OSD_Y_EN);
    pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_EN = ::property_get_int32("vendor.debug.nr3d.demo_en", pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_EN);
    pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_SEL = ::property_get_int32("vendor.debug.nr3d.demo_sel", pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_SEL);
    pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_TOP = ::property_get_int32("vendor.debug.nr3d.demo_top", pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_TOP);
    pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_BOT = ::property_get_int32("vendor.debug.nr3d.demo_bot", pIspPhyReg->DIP_X_MDP_TNR_DEMO_1.Bits.NR3D_DEMO_BOT);
    pIspPhyReg->DIP_X_MDP_TNR_DEMO_2.Bits.NR3D_DEMO_LEFT = ::property_get_int32("vendor.debug.nr3d.demo_left", pIspPhyReg->DIP_X_MDP_TNR_DEMO_2.Bits.NR3D_DEMO_LEFT);
    pIspPhyReg->DIP_X_MDP_TNR_DEMO_2.Bits.NR3D_DEMO_RIGHT = ::property_get_int32("vendor.debug.nr3d.demo_right", pIspPhyReg->DIP_X_MDP_TNR_DEMO_2.Bits.NR3D_DEMO_RIGHT);

    //////////////end of 20170316

    //////////////start of 20170623
    // NR3D_UV_Signed
    pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_UV_SIGNED = ::property_get_int32("vendor.debug.nr3d.bm.uv_signed", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_UV_SIGNED);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_CAM_TNR_UV_SIGNED =%d", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_UV_SIGNED);
    }
    //NR3D_CAM_TNR_C42_FILT_DIS
    pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_C42_FILT_DIS = ::property_get_int32("vendor.debug.nr3d.bm.c42_filt_dis", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_C42_FILT_DIS);
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_CAM_TNR_C42_FILT_DIS =%d", pIspPhyReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_C42_FILT_DIS);
    }
    //////////////end of 20170623
}

static MVOID fixTuningValue(dip_x_reg_t *pReg)
{
    if (pReg == NULL)
    {
        return;
    }

    dip_x_reg_t *pIspPhyReg = pReg;

    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_TNR_ENABLE, (0x1 << 31) | (0x1 << 30));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_FLT_CONFIG, (0x8 << 27) | (0x8 << 22) |
        (0x8 << 17) | (0x8 << 12) | (0x1C << 6));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_FB_INFO1, (0x4 << 26) | (0x14 << 20));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_THR_1, (0x4 << 26) | (0x8 << 22) | (0x20 << 16) | (0xA << 10));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_1, (0x1C << 26) | (0x1E << 20) |
        (0x20 << 14) | (0x1A << 8) | (0x10 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_2, (0x4 << 26) | (0x0 << 20) |
        (0x0 << 14) | (0x18 << 8) | (0x1A << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_3, (0x8 << 26) | (0x2 << 20) |
        (0x0 << 14) | (0x0 << 8) | (0x0 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_4, (0x0 << 26) | (0x0 << 20) |
        (0x8 << 14) | (0x10 << 8) | (0x15 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_5, (0x1B << 26) | (0x20 << 20) |
        (0x20 << 14) | (0x20 << 8) | (0x20 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_6, (0x20 << 26) | (0x1B << 20) |
        (0x15 << 14) | (0xC << 8) | (0x0 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_7, (0x0 << 26) | (0x0 << 20) |
        (0x0 << 14) | (0x5 << 8) | (0x16 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_8, (0x20 << 26) | (0x20 << 20) |
        (0x20 << 14) | (0x20 << 8) | (0x20 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_9, (0x0 << 26) | (0x3 << 20) |
        (0x7 << 14) | (0xE << 8) | (0x14 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_10, (0x1A << 26) | (0x1E << 20) |
        (0x20 << 14) | (0x10 << 8) | (0x10 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_11, (0x6 << 26) | (0x3 << 20) |
        (0x2 << 14) | (0x1 << 8) | (0x1 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_12, (0x1 << 26) | (0x0 << 20) |
        (0x1 << 14) | (0x3 << 8) | (0x9 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_13, (0xE << 26) | (0x10 << 20) |
        (0x10 << 14) | (0x10 << 8));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_14, (0x3F << 26) | (0x38 << 20) |
        (0x30 << 14) | (0x28 << 8) | (0x20 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_15, (0x18 << 26) | (0x10 << 20) |
        (0x8 << 14) | (0x0 << 8));
}

MBOOL
ISP_MGR_NR3D_T::
post_apply(MBOOL bEnable, void* pReg)
{
    dip_x_reg_t *pIspPhyReg = (dip_x_reg_t*)pReg;

    MBOOL       bEnableSL2E = MTRUE; // enable sl2e
    MBOOL       bEnableDemo = MFALSE; // enable demo mode
    MBOOL       bEnableBM = MFALSE; // enable benchmark mode
    MBOOL       bfixTuning = MFALSE;
    MBOOL       bForceDisableR2C = MFALSE;

    if (::property_get_int32("vendor.debug.camera.3dnr.level", 0))
    {
        bEnableSL2E = ::property_get_int32("vendor.debug.3dnr.sl2e.enable", 1); // sl2e: default on
        bEnableDemo = ::property_get_int32("vendor.debug.3dnr.demo.enable", 0);
        bEnableBM = ::property_get_int32("vendor.debug.nr3d.bm.enable", 0);
        bfixTuning = ::property_get_int32("vendor.debug.3dnr.fix.tuning", 0);
        bForceDisableR2C = ::property_get_int32("vendor.debug.3dnr.disable.r2c", 0);

        CAM_LOGD_IF(1, "bEnableSL2E(%d), bEnableDemo(%d), bEnableBM(%d)",
            bEnableSL2E, bEnableDemo, bEnableBM);
    }

    if (pIspPhyReg == NULL)
    {
        CAM_LOGD("post_apply pReg NULL");
        return  MTRUE;
    }

    if (bEnable)
    {
        // turn on top register
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, NR3D_EN, bEnable);

        // turn on NR3D by NR3D_CAM_TNR_EN
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CAM_TNR_ENG_CON, NR3D_CAM_TNR_EN, bEnable);
        // nr3d default
        ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_CON, 0x00100F00);
        ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_OFF, 0x00000000);
        ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_SIZ, 0x00000000);

        //-------------------using default tuning value----------------------------------------
        if (bfixTuning)
        {
            fixTuningValue(pIspPhyReg);
        }

        //------------------------------------------------------------
        MRect onRegion = m_onRegion;

        if (bEnableDemo)
        {
            demoNR3D(bEnableSL2E, m_fullImg, onRegion);
        }

        // nr3d_on
        // OFST is relative position to IMGI full image
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_OFF, NR3D_ON_OFST_X, onRegion.p.x + m_fullImg.p.x);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_OFF, NR3D_ON_OFST_Y, onRegion.p.y + m_fullImg.p.y);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_SIZ, NR3D_ON_WD, onRegion.s.w);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_SIZ, NR3D_ON_HT, onRegion.s.h);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_ON_EN, bEnable);

        // nr3d_vipi
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_VIPI_OFFSET, NR3D_VIPI_OFFSET, m_vipiOffst);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_VIPI_SIZE, NR3D_VIPI_WIDTH, m_vipiReadSize.w);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_VIPI_SIZE, NR3D_VIPI_HEIGHT, m_vipiReadSize.h);

        MBOOL sl2e_enable = bEnableSL2E;

        if (bEnableSL2E)
        {
            if ((pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP == 0 || pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP == 0))
            {
                CAM_LOGD("force disable sl2e! DIP_X_SL2E_RZ = 0");
                sl2e_enable = MFALSE;
            }
        }

        // sl2e
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, SL2E_EN, sl2e_enable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_SL2_OFF, !sl2e_enable);

        // debug R2C
        if (bForceDisableR2C)
        {
            ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_MDP_TNR_R2C_1, NR3D_R2CENC, 0);
        }

        CAM_LOGD("NR3D_EN(%d), SL2E_EN(%d), x,y,w,h(%d,%d,%d,%d), full x,y,w,h(%d,%d,%d,%d),"
            "vipi offst(%d) w,h(%d,%d), R2C en(%d) cnt(%d,%d,%d,%d)",
            bEnable, sl2e_enable, onRegion.p.x, onRegion.p.y, onRegion.s.w, onRegion.s.h,
            m_fullImg.p.x, m_fullImg.p.y, m_fullImg.s.w, m_fullImg.s.h,
            m_vipiOffst, m_vipiReadSize.w, m_vipiReadSize.h, pIspPhyReg->DIP_X_MDP_TNR_R2C_1.Bits.NR3D_R2CENC,
            pIspPhyReg->DIP_X_MDP_TNR_R2C_3.Bits.NR3D_R2CF_CNT1, pIspPhyReg->DIP_X_MDP_TNR_R2C_3.Bits.NR3D_R2CF_CNT2,
            pIspPhyReg->DIP_X_MDP_TNR_R2C_3.Bits.NR3D_R2CF_CNT3, pIspPhyReg->DIP_X_MDP_TNR_R2C_3.Bits.NR3D_R2CF_CNT4);

        if (bEnableBM)
        {
            benchmarkNR3DRegValue((VOID*)pIspPhyReg);
        }

    }
    else
    {
        CAM_LOGD("turn off NR3D_EN, NR3D_ON_EN, SL2E_EN, NR3D_SL2_OFF");
        // don't turn off NR3D_EN, isColorEnable may need it
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, NR3D_EN, bEnable);

        // turn off NR3D by NR3D_CAM_TNR_EN
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CAM_TNR_ENG_CON, NR3D_CAM_TNR_EN, bEnable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_ON_EN, bEnable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, SL2E_EN, bEnable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_SL2_OFF, !bEnable);
    }

    return  MTRUE;

}

}
