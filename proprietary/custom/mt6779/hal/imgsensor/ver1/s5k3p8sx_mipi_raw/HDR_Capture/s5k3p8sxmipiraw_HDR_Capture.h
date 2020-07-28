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
#ifndef _HDR_CAPTURE_H
#define _HDR_CAPTURE_H

#include "camera_custom_nvram.h"

extern "C"
{
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0200;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0201;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0202;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0203;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0204;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0205;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0206;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0207;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0208;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0209;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0210;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0211;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0212;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0213;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0214;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0215;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0216;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0217;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0218;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0219;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0360;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0361;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0362;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0363;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0364;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0365;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0366;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0367;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0368;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0369;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0370;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0371;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0372;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0373;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0374;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0375;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0376;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0377;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0378;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0379;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0360;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0361;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0362;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0363;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0364;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0365;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0366;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0367;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0368;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0369;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0370;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0371;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0372;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0373;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0374;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0375;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0376;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0377;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0378;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0379;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0280;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0281;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0282;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0283;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0284;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0285;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0286;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0287;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0288;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0289;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0290;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0291;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0292;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0293;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0294;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0295;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0296;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0297;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0298;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0299;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0280;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0281;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0282;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0283;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0284;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0285;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0286;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0287;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0288;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0289;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0290;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0291;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0292;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0293;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0294;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0295;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0296;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0297;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0298;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0299;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0120;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0121;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0122;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0123;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0124;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0125;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0126;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0127;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0128;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0129;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0130;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0131;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0132;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0133;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0134;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0135;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0136;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0137;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0138;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0139;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0120;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0121;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0122;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0123;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0124;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0125;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0126;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0127;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0128;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0129;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0130;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0131;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0132;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0133;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0134;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0135;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0136;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0137;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0138;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0139;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0024;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0025;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0026;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0027;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0028;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0029;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0240;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0241;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0242;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0243;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0244;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0245;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0246;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0247;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0248;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0249;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0250;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0251;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0252;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0253;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0254;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0255;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0256;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0257;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0258;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0259;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0260;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0261;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0262;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0263;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0264;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0265;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0266;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0267;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0268;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0269;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0270;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0271;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0272;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0273;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0274;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0275;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0276;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0277;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0278;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0279;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0280;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0281;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0282;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0283;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0284;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0285;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0286;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0287;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0288;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0289;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0290;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0291;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0292;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0293;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0294;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0295;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0296;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0297;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0298;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0299;
    extern const ISP_NVRAM_COLOR_PARAM_T s5k3p8sxmipiraw_COLOR_PARAM_0004;
    extern const ISP_NVRAM_LCE_TUNING_PARAM_T s5k3p8sxmipiraw_LCE_0002;
    extern const ISP_NVRAM_GMA_STRUCT_T s5k3p8sxmipiraw_GMA_0002;
    extern const ISP_NVRAM_DCE_TUNING_PARAM_T s5k3p8sxmipiraw_DCE_0002;
    extern const FEATURE_NVRAM_CA_LTM_T s5k3p8sxmipiraw_CA_LTM_0005;

}
#endif