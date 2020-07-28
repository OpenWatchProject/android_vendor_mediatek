/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#include "camera_custom_nvram.h"
#include "camera_custom_isp_nvram.h"

#include "inc.h"

#define CNR_CCR_BASE                                                 CNR_CCR_ISP_Other_BASE
#define CNR_ABF_BASE                                                 CNR_ABF_ISP_Other_BASE
#define SWNR_THRES_BASE                                              SWNR_THRES_ISP_Other_BASE
#define SWNR_BASE                                                    SWNR_ISP_Other_BASE
#define LPCNR_BASE                                                   LPCNR_ISP_Other_BASE
#define FUS_BASE                                                     FUS_ISP_Other_BASE
#define ZFUS_BASE                                                    ZFUS_ISP_Other_BASE

const ISP_NVRAM_CNR_CCR_T CNR_CCR_BASE[8] = {
    // IDX_0
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_1
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_2
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_3
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_4
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_5
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_6
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
    // IDX_7
    {
        .con       ={.bits ={.CNR_CCR_EN=1, .CNR_CCR_SL2_LINK=1, .CNR_CCR_SL2_MODE=0, .rsv_4=0, .CNR_CCR_OR_MODE=0, .rsv_6=0, .CNR_CCR_UV_GAIN_MODE=1, .rsv_9=0, .CNR_CCR_UV_GAIN2=0, .rsv_23=0, .CNR_CCR_Y_CPX3=255}},
        .ylut      ={.bits ={.CNR_CCR_Y_CPX1=32, .CNR_CCR_Y_CPX2=96, .CNR_CCR_Y_SP1=24, .rsv_23=0, .CNR_CCR_Y_CPY1=16, .rsv_31=0}},
        .uvlut     ={.bits ={.CNR_CCR_UV_X1=5, .CNR_CCR_UV_X2=37, .CNR_CCR_UV_X3=255, .CNR_CCR_UV_GAIN1=64, .rsv_31=0}},
        .ylut2     ={.bits ={.CNR_CCR_Y_SP0=16, .rsv_7=0, .CNR_CCR_Y_SP2=0, .rsv_15=0, .CNR_CCR_Y_CPY0=0, .rsv_23=0, .CNR_CCR_Y_CPY2=64, .rsv_31=0}},
        .sat_ctrl  ={.bits ={.CNR_CCR_MODE=1, .rsv_1=0, .CNR_CCR_CEN_U=0, .rsv_10=0, .CNR_CCR_CEN_V=0, .rsv_18=0}},
        .uvlut_sp  ={.bits ={.CNR_CCR_UV_GAIN_SP1=64, .rsv_10=0, .CNR_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
        .hue1      ={.bits ={.CNR_CCR_HUE_X1=0, .rsv_9=0, .CNR_CCR_HUE_X2=16, .rsv_25=0}},
        .hue2      ={.bits ={.CNR_CCR_HUE_X3=270, .rsv_9=0, .CNR_CCR_HUE_X4=286, .rsv_25=0}},
        .hue3      ={.bits ={.CNR_CCR_HUE_SP1=128, .CNR_CCR_HUE_SP2=127, .CNR_CCR_HUE_GAIN1=64, .rsv_23=0, .CNR_CCR_HUE_GAIN2=0, .rsv_31=0}},
        .l4lut1    ={.bits ={.CNR_CCR_SL2_X1=64, .CNR_CCR_SL2_X2=128, .CNR_CCR_SL2_X3=192, .rsv_24=0}},
        .l4lut2    ={.bits ={.CNR_CCR_SL2_GAIN0=8, .rsv_6=0, .CNR_CCR_SL2_GAIN1=10, .rsv_14=0, .CNR_CCR_SL2_GAIN2=12, .rsv_22=0, .CNR_CCR_SL2_GAIN3=16, .rsv_30=0}},
        .l4lut3    ={.bits ={.CNR_CCR_SL2_SP0=1, .rsv_6=0, .CNR_CCR_SL2_SP1=1, .rsv_14=0, .CNR_CCR_SL2_SP2=2, .rsv_22=0, .CNR_CCR_SL2_SP3=2, .rsv_30=0}},
    },
};
const ISP_NVRAM_CNR_ABF_T CNR_ABF_BASE[8] = {
    // IDX_0
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_1
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_2
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_3
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_4
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_5
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_6
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
    // IDX_7
    {
        .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
        .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
        .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
        .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
        .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
        .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
        .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
        .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
        .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
        .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
    },
};
const NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT SWNR_THRES_BASE[1] = {
    // IDX_0
    {
        1600, 60000, 2
    },
};
const NVRAM_CAMERA_FEATURE_SWNR_STRUCT SWNR_BASE[8] = {
    // IDX_0
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_1
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_2
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_3
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_4
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_5
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_6
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
    // IDX_7
    {
        .NR={.NR_K=300, .NR_S=30, .NR_SD=1, .NR_BLD_W=32, .NR_BLD_TH=5, .NR_SMTH=0, .NR_NTRL_TH_1_Y=255, .NR_NTRL_TH_2_Y=255, .NR_NTRL_TH_1_UV=255, .NR_NTRL_TH_2_UV=255},
        .HFG={.HFG_ENABLE=0, .HFG_GSD=1, .HFG_SD0=55552, .HFG_SD1=56905, .HFG_SD2=46189, .HFG_TX_S=128, .HFG_LCE_LINK_EN=0, .HFG_LUMA_CPX1=64, .HFG_LUMA_CPX2=128, .HFG_LUMA_CPX3=192, .HFG_LUMA_CPY0=32, .HFG_LUMA_CPY1=28, .HFG_LUMA_CPY2=24, .HFG_LUMA_CPY3=20, .HFG_LUMA_SP0=-4, .HFG_LUMA_SP1=-4, .HFG_LUMA_SP2=-4, .HFG_LUMA_SP3=-4},
        .CCR={.CCR_ENABLE=0, .CCR_CEN_U=0, .CCR_CEN_V=0, .CCR_Y_CPX1=32, .CCR_Y_CPX2=96, .CCR_Y_CPY1=16, .CCR_Y_SP1=24, .CCR_UV_X1=5, .CCR_UV_X2=37, .CCR_UV_X3=255, .CCR_UV_GAIN1=64, .CCR_UV_GAIN2=0, .CCR_UV_GAIN_SP1=64, .CCR_UV_GAIN_SP2=0, .CCR_Y_CPX3=224, .CCR_Y_CPY0=0, .CCR_Y_CPY2=64, .CCR_Y_SP0=16, .CCR_Y_SP2=25, .CCR_UV_GAIN_MODE=0, .CCR_MODE=0, .CCR_OR_MODE=0, .CCR_HUE_X1=0, .CCR_HUE_X2=16, .CCR_HUE_X3=270, .CCR_HUE_X4=286, .CCR_HUE_SP1=-128, .CCR_HUE_SP2=127, .CCR_HUE_GAIN1=64, .CCR_HUE_GAIN2=0},
    },
};
const ISP_NVRAM_LPCNR_T LPCNR_BASE[8] = {
    // IDX_0
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_1
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_2
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_3
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_4
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_5
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_6
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
    // IDX_7
    {
        .nr_w1     ={.bits ={.LPCNR_NR_W00=253, .LPCNR_NR_W01=178, .LPCNR_NR_W02=125, .LPCNR_NR_W03=88}},
        .nr_w2     ={.bits ={.LPCNR_NR_W04=61, .LPCNR_NR_W05=43, .LPCNR_NR_W06=31, .LPCNR_NR_W07=22}},
        .nr_w3     ={.bits ={.LPCNR_NR_W08=15, .LPCNR_NR_W09=11, .LPCNR_NR_W10=7, .LPCNR_NR_W11=5}},
        .nr_w4     ={.bits ={.LPCNR_NR_W12=4, .LPCNR_NR_W13=3, .LPCNR_NR_W14=2, .LPCNR_NR_W15=1}},
        .nr_w5     ={.bits ={.LPCNR_NR_W16=1, .LPCNR_NR_W17=1, .LPCNR_NR_W18=0, .LPCNR_NR_W19=0}},
        .nr_w6     ={.bits ={.LPCNR_NR_W20=0, .LPCNR_NR_W21=0, .LPCNR_NR_W22=0, .LPCNR_NR_W23=0}},
        .nr_w7     ={.bits ={.LPCNR_NR_W24=0, .LPCNR_NR_W25=0, .LPCNR_NR_W26=0, .LPCNR_NR_W27=0}},
        .nr_w8     ={.bits ={.LPCNR_NR_W28=0, .LPCNR_NR_W29=0, .LPCNR_NR_W30=0, .LPCNR_NR_W31=0}},
        .nr_w9     ={.bits ={.LPCNR_NR_W32=0, .LPCNR_NR_W33=0, .LPCNR_NR_W34=0, .LPCNR_NR_W35=0}},
        .nr_w10    ={.bits ={.LPCNR_NR_W36=0, .LPCNR_NR_W37=0, .LPCNR_NR_W38=0, .LPCNR_NR_W39=0}},
        .nr_w11    ={.bits ={.LPCNR_NR_W40=0, .LPCNR_NR_W41=0, .LPCNR_NR_W42=0, .LPCNR_NR_W43=0}},
        .nr_w12    ={.bits ={.LPCNR_NR_W44=0, .LPCNR_NR_W45=0, .LPCNR_NR_W46=0, .LPCNR_NR_W47=0}},
        .nr_w13    ={.bits ={.LPCNR_NR_W48=0, .LPCNR_NR_W49=0, .LPCNR_NR_W50=0, .LPCNR_NR_W51=0}},
        .nr_w14    ={.bits ={.LPCNR_NR_W52=0, .LPCNR_NR_W53=0, .rsv_16=0}},
        .sm        ={.bits ={.LPCNR_NR_SD=55552, .LPCNR_SM_TH=16, .rsv_21=0}},
        .bld       ={.bits ={.LPCNR_BLD_TH=5, .LPCNR_BLD_SP=6, .rsv_13=0, .LPCNR_BLD_W0=0, .rsv_23=0, .LPCNR_BLD_W1=32, .rsv_31=0}},
        .ntrl_y    ={.bits ={.LPCNR_NTRL_Y_TH0=2, .LPCNR_NTRL_Y_TH1=251, .LPCNR_NTRL_Y_SP0=4, .LPCNR_NTRL_Y_SP1=4, .rsv_24=0}},
        .ntrl_c    ={.bits ={.LPCNR_NTRL_C_TH=4, .LPCNR_NTRL_C_SP=4, .rsv_12=0}},
    },
};
const ISP_NVRAM_FUS_T FUS_BASE[8] = {
    // IDX_0
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_1
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_2
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_3
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_4
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_5
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_6
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
    // IDX_7
    {
        .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
        .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
        .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
    },
};
const ISP_NVRAM_ZFUS_T ZFUS_BASE[8] = {
    // IDX_0
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_1
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_2
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_3
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_4
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_5
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_6
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
    // IDX_7
    {
        .ma        ={.bits ={.ZFUS_MASE_BLDWD=8, .ZFUS_OSBLD_WD=4, .ZFUS_SEYOS_LEFAC=8, .rsv_13=0, .ZFUS_SENOS_LEFAC=8, .ZFUS_DIFF_LB=0}},
        .mc        ={.bits ={.ZFUS_MO_EN=1, .rsv_1=0, .ZFUS_MOBLD_FT=4, .rsv_7=0, .ZFUS_MOTH_RATIO=2, .rsv_13=0, .ZFUS_MOSE_TH=512, .ZFUS_MOSE_BLDWD=1}},
        .osc       ={.bits ={.ZFUS_OSC_TH=4080, .ZFUS_SEDIR_SL=4, .rsv_15=0, .ZFUS_SEBLD_WD=1, .ZFUS_LEBLD_WD=1, .ZFUS_LE_INV_CTL=0, .ZFUS_EDGE=15}},
        .tune      ={.bits ={.ZFUS_PS_BLUR=1, .ZFUS_OSC_REPEN=1, .ZFUS_SOFT_TH_EN=0, .ZFUS_LE_LOWPA_EN=0, .ZFUS_SE_LOWPA_EN=0, .ZFUS_PSSEC_ONLY=0, .ZFUS_OSCLE_ONLY=0, .ZFUS_MOLE_DIREN=1, .ZFUS_MOSE_DIREN=1, .ZFUS_MO_2DBLD_EN=1, .rsv_10=0, .ZFUS_PS_TH=1024, .rsv_24=0}},
    },
};
