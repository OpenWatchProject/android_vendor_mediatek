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
#include "s5k3p8sxmipiraw_Video_1080.h"

const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0040 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0041 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0042 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0043 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0044 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0045 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0046 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0047 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0048 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0049 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0050 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0051 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0052 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0053 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0054 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0055 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0056 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0057 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0058 = {
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
const ISP_NVRAM_DBS_T s5k3p8sxmipiraw_DBS_0059 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0040 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0041 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0042 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0043 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0044 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0045 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0046 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0047 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0048 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0049 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0050 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0051 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0052 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0053 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0054 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0055 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0056 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0057 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0058 = {
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
const ISP_NVRAM_ADBS_T s5k3p8sxmipiraw_ADBS_0059 = {
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
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0040 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0041 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0042 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0043 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0044 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0045 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0046 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0047 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0048 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0049 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0050 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0051 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0052 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0053 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0054 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0055 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0056 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0057 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0058 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_OBC_T s5k3p8sxmipiraw_OBC_0059 = {
    .offst3    ={.bits ={.OBC_OFST_R=7936, .rsv_13=0}},
    .offst1    ={.bits ={.OBC_OFST_GR=7936, .rsv_13=0}},
    .offst2    ={.bits ={.OBC_OFST_GB=7936, .rsv_13=0}},
    .offst0    ={.bits ={.OBC_OFST_B=7936, .rsv_13=0}},
    .gain3     ={.bits ={.OBC_GAIN_R=546, .rsv_13=0}},
    .gain1     ={.bits ={.OBC_GAIN_GR=546, .rsv_13=0}},
    .gain2     ={.bits ={.OBC_GAIN_GB=546, .rsv_13=0}},
    .gain0     ={.bits ={.OBC_GAIN_B=546, .rsv_13=0}},
};
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0040 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0041 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0042 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0043 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0044 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0045 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0046 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0047 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0048 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0049 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0050 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0051 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0052 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0053 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0054 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0055 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0056 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0057 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0058 = {
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
const ISP_NVRAM_BNR_BPC_T s5k3p8sxmipiraw_BNR_BPC_0059 = {
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
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0040 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0041 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0042 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0043 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0044 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0045 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0046 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0047 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0048 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0049 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0050 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0051 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0052 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0053 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0054 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0055 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0056 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0057 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0058 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_NR1_T s5k3p8sxmipiraw_BNR_NR1_0059 = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0040 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0041 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0042 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0043 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0044 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0045 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0046 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0047 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0048 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0049 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0050 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0051 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0052 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0053 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0054 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0055 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0056 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0057 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0058 = {
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
const ISP_NVRAM_BNR_PDC_T s5k3p8sxmipiraw_BNR_PDC_0059 = {
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
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0040 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0041 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0042 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0043 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0044 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0045 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0046 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0047 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0048 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0049 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0050 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0051 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0052 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0053 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0054 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0055 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0056 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0057 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0058 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RMM_T s5k3p8sxmipiraw_RMM_0059 = {
    .osc       ={.bits ={.RMM_OSC_TH=1004, .RMM_SEDIR_SL=4, .rsv_15=0, .RMM_SEBLD_WD=1, .RMM_LEBLD_WD=1, .RMM_LE_INV_CTL=0, .RMM_EDGE=15}},
    .mc        ={.bits ={.RMM_MO_EDGE=1, .rsv_1=0, .RMM_MO_EN=1, .rsv_3=0, .RMM_MOBLD_FT=4, .rsv_7=0, .RMM_MOTH_RATIO=2, .rsv_13=0, .RMM_HORI_ADDWT=4, .rsv_21=0}},
    .mc2       ={.bits ={.RMM_MOSE_TH=128, .rsv_12=0, .RMM_MOSE_BLDWD=5, .rsv_20=0}},
    .ma        ={.bits ={.RMM_MASE_RATIO=8, .rsv_5=0, .RMM_OSBLD_WD=4, .RMM_MASE_BLDWD=8, .RMM_SENOS_LEFAC=8, .rsv_21=0, .RMM_SEYOS_LEFAC=8, .rsv_29=0}},
    .tune      ={.bits ={.RMM_PSHOR_SEEN=1, .RMM_PS_BLUR=1, .rsv_2=0, .RMM_OSC_REPEN=1, .RMM_SOFT_TH_EN=0, .RMM_LE_LOWPA_EN=0, .RMM_SE_LOWPA_EN=0, .RMM_PSSEC_ONLY=0, .rsv_9=0, .RMM_OSCLE_ONLY=0, .rsv_13=0, .RMM_PS_TH=128, .RMM_MOLE_DIREN=1, .RMM_MOSE_DIREN=1, .RMM_MO_2DBLD_EN=1, .rsv_31=0}},
    .ridx    ={.bits={.RMM_OSC_IDX=255, .RMM_PS_IDX=16, .RMM_MOSE_IDX=12, .rsv_25=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0040 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0041 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0042 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0043 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0044 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0045 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0046 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0047 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0048 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0049 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0050 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0051 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0052 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0053 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0054 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0055 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0056 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0057 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0058 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
const ISP_NVRAM_RNR_T s5k3p8sxmipiraw_RNR_0059 = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};
