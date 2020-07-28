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
#include "imx258mipiraw_Reserved.h"

const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0000 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0001 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0002 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0003 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0004 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0005 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0006 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0007 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0008 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2F_0009 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0000 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0001 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0002 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0003 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0004 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0005 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0006 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0007 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0008 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_SL2_T imx258mipiraw_SL2_0009 = {
    .cen     ={.bits={.SL2_CENTR_X=0, .rsv_14=0, .SL2_CENTR_Y=0, .rsv_30=0}},
    .rr_con0 ={.bits={.SL2_R_0=0, .rsv_14=0, .SL2_R_1=0, .rsv_30=0}},
    .rr_con1 ={.bits={.SL2_R_2=0, .rsv_14=0, .SL2_GAIN_0=0, .SL2_GAIN_1=64}},
    .gain    ={.bits={.SL2_GAIN_2=128, .SL2_GAIN_3=192, .SL2_GAIN_4=255, .SL2_SET_ZERO=0, .rsv_25=0}},
};
const ISP_NVRAM_PCA_STRUCT_T imx258mipiraw_PCA_0000 = {
    .Config=
    {
        .set={//00
            0x00000000, 0x00000000, 0x00000000
        }
    },

    .PCA_LUT=
    {
        .lut={
            //y_gain sat_gain hue_shift reserved
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //000
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //010
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //020
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //030
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //040
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //050
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //060
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //070
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //080
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //090
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //100
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //110
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //120
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //130
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //140
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //150
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //160
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //170
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}}, //
            {.bits= {.PCA_LUMA_GAIN = 0, .PCA_SAT_GAIN = 0, .PCA_HUE_SHIFT = 0, .rsv_24 = 0}} //
        },
    },
};
