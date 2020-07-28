/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include "imx386mipiraw_Scene_Preview.h"

const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0020 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0021 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0022 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0023 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0024 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0025 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0026 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0027 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0028 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0029 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0030 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0031 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0032 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0033 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0034 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0035 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0036 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0037 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0038 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_DBS_T imx386mipiraw_DBS_0039 = {
    .sigma   ={.bits={.DBS_OFST=5, .rsv_12=0, .DBS_SL=21, .rsv_24=0}},
    .bstbl_0 ={.bits={.DBS_BIAS_Y0=4, .DBS_BIAS_Y1=2, .DBS_BIAS_Y2=1, .DBS_BIAS_Y3=0}},
    .bstbl_1 ={.bits={.DBS_BIAS_Y4=0, .DBS_BIAS_Y5=0, .DBS_BIAS_Y6=0, .DBS_BIAS_Y7=0}},
    .bstbl_2 ={.bits={.DBS_BIAS_Y8=0, .DBS_BIAS_Y9=0, .DBS_BIAS_Y10=0, .DBS_BIAS_Y11=0}},
    .bstbl_3 ={.bits={.DBS_BIAS_Y12=0, .DBS_BIAS_Y13=0, .DBS_BIAS_Y14=0, .rsv_24=0}},
    .ctl     ={.bits={.DBS_HDR_GNP=0, .rsv_3=0, .DBS_SL_EN=0, .rsv_5=0, .DBS_LE_INV_CTL=0, .DBS_EDGE=15, .DBS_HDR_GAIN=64, .DBS_HDR_GAIN2=8}},
    .sigma_2 ={.bits={.DBS_MUL_B=96, .DBS_MUL_GB=96, .DBS_MUL_GR=96, .DBS_MUL_R=96}},
    .ygn     ={.bits={.DBS_YGN_B=32, .rsv_6=0, .DBS_YGN_GB=32, .rsv_14=0, .DBS_YGN_GR=32, .rsv_22=0, .DBS_YGN_R=32, .rsv_30=0}},
    .sl_y12  ={.bits={.DBS_SL_Y1=0, .rsv_10=0, .DBS_SL_Y2=0, .rsv_26=0}},
    .sl_y34  ={.bits={.DBS_SL_Y3=0, .rsv_10=0, .DBS_SL_Y4=0, .rsv_26=0}},
    .sl_g12  ={.bits={.DBS_SL_G1=0, .rsv_10=0, .DBS_SL_G2=0, .rsv_26=0}},
    .sl_g34  ={.bits={.DBS_SL_G3=0, .rsv_10=0, .DBS_SL_G4=0, .rsv_26=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0020 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0021 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0022 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0023 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0024 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0025 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0026 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0027 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0028 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0029 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0030 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0031 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0032 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0033 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0034 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0035 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0036 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0037 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0038 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_ADBS_T imx386mipiraw_ADBS_0039 = {
    .gray_bld_0={.bits={.ADBS_LUMA_MODE=0, .rsv_1=0, .ADBS_BLD_MXRT=0, .rsv_24=0}},
    .gray_bld_1={.bits={.ADBS_BLD_LOW=269, .rsv_12=0, .ADBS_BLD_SLP=341, .rsv_28=0}},
    .bias_lut_r0={.bits={.ADBS_BIAS_R0=0, .ADBS_BIAS_R1=0, .ADBS_BIAS_R2=0, .ADBS_BIAS_R3=0}},
    .bias_lut_r1={.bits={.ADBS_BIAS_R4=0, .ADBS_BIAS_R5=0, .ADBS_BIAS_R6=0, .ADBS_BIAS_R7=0}},
    .bias_lut_r2={.bits={.ADBS_BIAS_R8=0, .ADBS_BIAS_R9=0, .ADBS_BIAS_R10=0, .ADBS_BIAS_R11=0}},
    .bias_lut_r3={.bits={.ADBS_BIAS_R12=0, .ADBS_BIAS_R13=0, .ADBS_BIAS_R14=0, .rsv_24=0}},
    .bias_lut_g0={.bits={.ADBS_BIAS_G0=0, .ADBS_BIAS_G1=0, .ADBS_BIAS_G2=0, .ADBS_BIAS_G3=0}},
    .bias_lut_g1={.bits={.ADBS_BIAS_G4=0, .ADBS_BIAS_G5=0, .ADBS_BIAS_G6=0, .ADBS_BIAS_G7=0}},
    .bias_lut_g2={.bits={.ADBS_BIAS_G8=0, .ADBS_BIAS_G9=0, .ADBS_BIAS_G10=0, .ADBS_BIAS_G11=0}},
    .bias_lut_g3={.bits={.ADBS_BIAS_G12=0, .ADBS_BIAS_G13=0, .ADBS_BIAS_G14=0, .rsv_24=0}},
    .bias_lut_b0={.bits={.ADBS_BIAS_B0=0, .ADBS_BIAS_B1=0, .ADBS_BIAS_B2=0, .ADBS_BIAS_B3=0}},
    .bias_lut_b1={.bits={.ADBS_BIAS_B4=0, .ADBS_BIAS_B5=0, .ADBS_BIAS_B6=0, .ADBS_BIAS_B7=0}},
    .bias_lut_b2={.bits={.ADBS_BIAS_B8=0, .ADBS_BIAS_B9=0, .ADBS_BIAS_B10=0, .ADBS_BIAS_B11=0}},
    .bias_lut_b3={.bits={.ADBS_BIAS_B12=0, .ADBS_BIAS_B13=0, .ADBS_BIAS_B14=0, .rsv_24=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0020 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0021 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0022 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0023 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0024 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0025 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0026 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0027 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0028 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0029 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0030 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0031 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0032 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0033 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0034 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0035 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0036 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0037 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0038 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T imx386mipiraw_OBC_0039 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0020 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0021 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0022 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0023 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0024 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0025 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0026 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0027 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0028 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0029 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0030 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0031 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0032 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0033 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0034 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0035 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0036 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0037 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0038 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_BPC_T imx386mipiraw_BNR_BPC_0039 = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0020 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0021 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0022 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0023 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0024 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0025 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0026 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0027 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0028 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0029 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0030 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0031 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0032 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0033 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0034 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0035 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0036 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0037 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0038 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T imx386mipiraw_BNR_NR1_0039 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0020 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0021 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0022 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0023 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0024 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0025 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0026 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0027 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0028 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0029 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0030 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0031 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0032 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0033 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0034 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0035 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0036 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0037 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0038 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_BNR_PDC_T imx386mipiraw_BNR_PDC_0039 = {
    .con     ={.bits={.PDC_EN=0, .rsv_1=0, .PDC_CT=1, .rsv_5=0, .PDC_MODE=1, .rsv_10=0, .PDC_OUT=1, .rsv_17=0}},
    .gain_l0 ={.bits={.PDC_GCF_L00=256, .rsv_12=0, .PDC_GCF_L10=0, .rsv_28=0}},
    .gain_l1 ={.bits={.PDC_GCF_L01=0, .rsv_12=0, .PDC_GCF_L20=0, .rsv_28=0}},
    .gain_l2 ={.bits={.PDC_GCF_L11=0, .rsv_12=0, .PDC_GCF_L02=0, .rsv_28=0}},
    .gain_l3 ={.bits={.PDC_GCF_L30=0, .rsv_12=0, .PDC_GCF_L21=0, .rsv_28=0}},
    .gain_l4 ={.bits={.PDC_GCF_L12=0, .rsv_12=0, .PDC_GCF_L03=0, .rsv_28=0}},
    .gain_r0 ={.bits={.PDC_GCF_R00=256, .rsv_12=0, .PDC_GCF_R10=0, .rsv_28=0}},
    .gain_r1 ={.bits={.PDC_GCF_R01=0, .rsv_12=0, .PDC_GCF_R20=0, .rsv_28=0}},
    .gain_r2 ={.bits={.PDC_GCF_R11=0, .rsv_12=0, .PDC_GCF_R02=0, .rsv_28=0}},
    .gain_r3 ={.bits={.PDC_GCF_R30=0, .rsv_12=0, .PDC_GCF_R21=0, .rsv_28=0}},
    .gain_r4 ={.bits={.PDC_GCF_R12=0, .rsv_12=0, .PDC_GCF_R03=0, .rsv_28=0}},
    .th_gb   ={.bits={.PDC_GTH=200, .rsv_12=0, .PDC_BTH=40, .rsv_28=0}},
    .th_ia   ={.bits={.PDC_ITH=0, .rsv_12=0, .PDC_ATH=300, .rsv_28=0}},
    .th_hd   ={.bits={.PDC_NTH=80, .rsv_12=0, .PDC_DTH=512, .rsv_28=0}},
    .sl     ={.bits={.PDC_GSL=8, .PDC_BSL=6, .PDC_ISL=7, .PDC_ASL=7, .PDC_GCF_NORM=13, .rsv_20=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0020 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0021 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0022 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0023 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0024 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0025 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0026 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0027 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0028 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0029 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0030 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0031 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0032 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0033 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0034 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0035 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0036 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0037 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0038 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T imx386mipiraw_RMM_0039 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0020 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0021 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0022 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0023 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0024 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0025 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0026 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0027 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0028 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0029 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0030 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0031 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0032 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0033 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0034 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0035 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0036 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0037 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0038 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T imx386mipiraw_RNR_0039 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
