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
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0180;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0181;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0182;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0183;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0184;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0185;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0186;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0187;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0188;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0189;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0190;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0191;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0192;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0193;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0194;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0195;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0196;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0197;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0198;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0199;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0300;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0301;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0302;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0303;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0304;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0305;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0306;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0307;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0308;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0309;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0310;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0311;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0312;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0313;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0314;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0315;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0316;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0317;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0318;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0319;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0300;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0301;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0302;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0303;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0304;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0305;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0306;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0307;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0308;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0309;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0310;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0311;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0312;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0313;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0314;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0315;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0316;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0317;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0318;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0319;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0240;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0241;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0242;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0243;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0244;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0245;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0246;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0247;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0248;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0249;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0250;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0251;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0252;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0253;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0254;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0255;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0256;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0257;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0258;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0259;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0240;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0241;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0242;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0243;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0244;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0245;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0246;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0247;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0248;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0249;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0250;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0251;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0252;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0253;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0254;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0255;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0256;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0257;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0258;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0259;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0120;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0121;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0122;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0123;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0124;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0125;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0126;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0127;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0128;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0129;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0130;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0131;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0132;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0133;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0134;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0135;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0136;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0137;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0138;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0139;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0120;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0121;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0122;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0123;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0124;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0125;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0126;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0127;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0128;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0129;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0130;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0131;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0132;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0133;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0134;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0135;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0136;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0137;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0138;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0139;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0024;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0025;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0026;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0027;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0028;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0029;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0240;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0241;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0242;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0243;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0244;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0245;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0246;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0247;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0248;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0249;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0250;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0251;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0252;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0253;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0254;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0255;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0256;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0257;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0258;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0259;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0260;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0261;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0262;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0263;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0264;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0265;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0266;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0267;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0268;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0269;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0270;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0271;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0272;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0273;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0274;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0275;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0276;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0277;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0278;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0279;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0280;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0281;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0282;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0283;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0284;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0285;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0286;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0287;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0288;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0289;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0290;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0291;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0292;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0293;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0294;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0295;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0296;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0297;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0298;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0299;
    extern const ISP_NVRAM_COLOR_PARAM_T s5k3p8spmipiraw_COLOR_PARAM_0004;
    extern const ISP_NVRAM_LCE_TUNING_PARAM_T s5k3p8spmipiraw_LCE_0002;
    extern const ISP_NVRAM_GMA_STRUCT_T s5k3p8spmipiraw_GMA_0002;
    extern const ISP_NVRAM_DCE_TUNING_PARAM_T s5k3p8spmipiraw_DCE_0002;
    extern const FEATURE_NVRAM_CA_LTM_T s5k3p8spmipiraw_CA_LTM_0002;

}
#endif