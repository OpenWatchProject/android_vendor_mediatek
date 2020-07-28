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
#define LOG_TAG "isp_mgr_color"

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
// COLOR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_COLOR_T&
ISP_MGR_COLOR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(COLOR);
}

template <>
ISP_MGR_COLOR_T&
ISP_MGR_COLOR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_COLOR_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CFG_MAIN,        cfg_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_C_BOOST_MAIN,    c_boost_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_C_BOOST_MAIN_2,  c_boost_main_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LUMA_ADJ,        luma_adj);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_G_PIC_ADJ_MAIN_1, g_pic_adj_main_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_G_PIC_ADJ_MAIN_2, g_pic_adj_main_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_1_0_MAIN, y_slope_1_0_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_3_2_MAIN, y_slope_3_2_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_5_4_MAIN, y_slope_5_4_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_7_6_MAIN, y_slope_7_6_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_9_8_MAIN, y_slope_9_8_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_11_10_MAIN, y_slope_11_10_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_13_12_MAIN, y_slope_13_12_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_15_14_MAIN, y_slope_15_14_main);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_0,  local_hue_cd_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_1,  local_hue_cd_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_2,  local_hue_cd_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_3,  local_hue_cd_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_4,  local_hue_cd_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_0, partial_sat_gain1_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_1, partial_sat_gain1_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_2, partial_sat_gain1_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_3, partial_sat_gain1_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_4, partial_sat_gain1_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_0, partial_sat_gain2_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_1, partial_sat_gain2_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_2, partial_sat_gain2_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_3, partial_sat_gain2_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_4, partial_sat_gain2_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_0, partial_sat_gain3_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_1, partial_sat_gain3_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_2, partial_sat_gain3_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_3, partial_sat_gain3_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_4, partial_sat_gain3_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_0, partial_sat_point1_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_1, partial_sat_point1_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_2, partial_sat_point1_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_3, partial_sat_point1_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_4, partial_sat_point1_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_0, partial_sat_point2_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_1, partial_sat_point2_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_2, partial_sat_point2_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_3, partial_sat_point2_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_4, partial_sat_point2_4);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_START,           start);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_0,  s_gain_by_y0_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_1,  s_gain_by_y0_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_2,  s_gain_by_y0_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_3,  s_gain_by_y0_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_4,  s_gain_by_y0_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_0, s_gain_by_y64_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_1, s_gain_by_y64_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_2, s_gain_by_y64_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_3, s_gain_by_y64_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_4, s_gain_by_y64_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_0, s_gain_by_y128_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_1, s_gain_by_y128_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_2, s_gain_by_y128_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_3, s_gain_by_y128_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_4, s_gain_by_y128_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_0, s_gain_by_y192_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_1, s_gain_by_y192_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_2, s_gain_by_y192_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_3, s_gain_by_y192_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_4, s_gain_by_y192_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_0, s_gain_by_y256_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_1, s_gain_by_y256_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_2, s_gain_by_y256_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_3, s_gain_by_y256_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_4, s_gain_by_y256_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LSP_1,           lsp_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LSP_2,           lsp_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_CONTROL,      cm_control);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_0,     cm_w1_hue_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_1,     cm_w1_hue_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_2,     cm_w1_hue_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_3,     cm_w1_hue_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_4,     cm_w1_hue_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_0,    cm_w1_luma_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_1,    cm_w1_luma_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_2,    cm_w1_luma_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_3,    cm_w1_luma_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_4,    cm_w1_luma_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_0,     cm_w1_sat_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_1,     cm_w1_sat_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_2,     cm_w1_sat_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_3,     cm_w1_sat_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_4,     cm_w1_sat_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_0,     cm_w2_hue_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_1,     cm_w2_hue_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_2,     cm_w2_hue_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_3,     cm_w2_hue_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_4,     cm_w2_hue_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_0,    cm_w2_luma_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_1,    cm_w2_luma_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_2,    cm_w2_luma_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_3,    cm_w2_luma_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_4,    cm_w2_luma_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_0,     cm_w2_sat_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_1,     cm_w2_sat_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_2,     cm_w2_sat_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_3,     cm_w2_sat_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_4,     cm_w2_sat_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_0,     cm_w3_hue_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_1,     cm_w3_hue_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_2,     cm_w3_hue_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_3,     cm_w3_hue_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_4,     cm_w3_hue_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_0,    cm_w3_luma_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_1,    cm_w3_luma_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_2,    cm_w3_luma_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_3,    cm_w3_luma_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_4,    cm_w3_luma_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_0,     cm_w3_sat_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_1,     cm_w3_sat_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_2,     cm_w3_sat_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_3,     cm_w3_sat_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_4,     cm_w3_sat_4);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_TILE,            tile);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CROP,            crop);
    return (*this);
}

template <>
ISP_MGR_COLOR_T&
ISP_MGR_COLOR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_COLOR_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CFG_MAIN,        cfg_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_C_BOOST_MAIN,    c_boost_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_C_BOOST_MAIN_2,  c_boost_main_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LUMA_ADJ,        luma_adj);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_G_PIC_ADJ_MAIN_1, g_pic_adj_main_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_G_PIC_ADJ_MAIN_2, g_pic_adj_main_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_1_0_MAIN, y_slope_1_0_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_3_2_MAIN, y_slope_3_2_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_5_4_MAIN, y_slope_5_4_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_7_6_MAIN, y_slope_7_6_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_9_8_MAIN, y_slope_9_8_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_11_10_MAIN, y_slope_11_10_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_13_12_MAIN, y_slope_13_12_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_Y_SLOPE_15_14_MAIN, y_slope_15_14_main);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_0,  local_hue_cd_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_1,  local_hue_cd_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_2,  local_hue_cd_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_3,  local_hue_cd_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LOCAL_HUE_CD_4,  local_hue_cd_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_0, partial_sat_gain1_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_1, partial_sat_gain1_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_2, partial_sat_gain1_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_3, partial_sat_gain1_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN1_4, partial_sat_gain1_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_0, partial_sat_gain2_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_1, partial_sat_gain2_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_2, partial_sat_gain2_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_3, partial_sat_gain2_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN2_4, partial_sat_gain2_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_0, partial_sat_gain3_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_1, partial_sat_gain3_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_2, partial_sat_gain3_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_3, partial_sat_gain3_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_GAIN3_4, partial_sat_gain3_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_0, partial_sat_point1_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_1, partial_sat_point1_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_2, partial_sat_point1_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_3, partial_sat_point1_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT1_4, partial_sat_point1_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_0, partial_sat_point2_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_1, partial_sat_point2_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_2, partial_sat_point2_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_3, partial_sat_point2_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_PARTIAL_SAT_POINT2_4, partial_sat_point2_4);
    //GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_START,           start);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_0,  s_gain_by_y0_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_1,  s_gain_by_y0_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_2,  s_gain_by_y0_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_3,  s_gain_by_y0_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y0_4,  s_gain_by_y0_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_0, s_gain_by_y64_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_1, s_gain_by_y64_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_2, s_gain_by_y64_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_3, s_gain_by_y64_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y64_4, s_gain_by_y64_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_0, s_gain_by_y128_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_1, s_gain_by_y128_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_2, s_gain_by_y128_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_3, s_gain_by_y128_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y128_4, s_gain_by_y128_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_0, s_gain_by_y192_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_1, s_gain_by_y192_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_2, s_gain_by_y192_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_3, s_gain_by_y192_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y192_4, s_gain_by_y192_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_0, s_gain_by_y256_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_1, s_gain_by_y256_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_2, s_gain_by_y256_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_3, s_gain_by_y256_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_S_GAIN_BY_Y256_4, s_gain_by_y256_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LSP_1,           lsp_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_LSP_2,           lsp_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_CONTROL,      cm_control);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_0,     cm_w1_hue_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_1,     cm_w1_hue_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_2,     cm_w1_hue_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_3,     cm_w1_hue_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_HUE_4,     cm_w1_hue_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_0,    cm_w1_luma_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_1,    cm_w1_luma_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_2,    cm_w1_luma_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_3,    cm_w1_luma_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_LUMA_4,    cm_w1_luma_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_0,     cm_w1_sat_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_1,     cm_w1_sat_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_2,     cm_w1_sat_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_3,     cm_w1_sat_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W1_SAT_4,     cm_w1_sat_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_0,     cm_w2_hue_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_1,     cm_w2_hue_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_2,     cm_w2_hue_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_3,     cm_w2_hue_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_HUE_4,     cm_w2_hue_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_0,    cm_w2_luma_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_1,    cm_w2_luma_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_2,    cm_w2_luma_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_3,    cm_w2_luma_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_LUMA_4,    cm_w2_luma_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_0,     cm_w2_sat_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_1,     cm_w2_sat_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_2,     cm_w2_sat_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_3,     cm_w2_sat_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W2_SAT_4,     cm_w2_sat_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_0,     cm_w3_hue_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_1,     cm_w3_hue_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_2,     cm_w3_hue_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_3,     cm_w3_hue_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_HUE_4,     cm_w3_hue_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_0,    cm_w3_luma_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_1,    cm_w3_luma_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_2,    cm_w3_luma_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_3,    cm_w3_luma_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_LUMA_4,    cm_w3_luma_4);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_0,     cm_w3_sat_0);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_1,     cm_w3_sat_1);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_2,     cm_w3_sat_2);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_3,     cm_w3_sat_3);
    GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CM_W3_SAT_4,     cm_w3_sat_4);
    //GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_TILE,            tile);
    //GET_REG_INFO_MULTI(SubModuleIndex, DIP_COLOR_CROP,            crop);
    return (*this);
}

MBOOL
ISP_MGR_COLOR_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case ECOLOR_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_COLOR_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }
    if(bEnable){
        reinterpret_cast<COLOR_REG_D1A_COLOR_DIP_COLOR_CFG_MAIN*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DIP_COLOR_CFG_MAIN))->Bits.COLOR_c_pp_cm_dbg_sel = 0;

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("COLOR", SubModuleIndex);
    }

    return MTRUE;
}


};
