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
#ifndef _FLASH_CAPTURE_H
#define _FLASH_CAPTURE_H

#include "camera_custom_nvram.h"

extern "C"
{
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0120;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0121;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0122;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0123;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0124;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0125;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0126;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0127;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0128;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0129;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0130;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0131;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0132;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0133;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0134;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0135;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0136;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0137;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0138;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0139;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0200;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0201;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0202;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0203;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0204;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0205;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0206;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0207;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0208;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0209;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0210;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0211;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0212;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0213;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0214;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0215;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0216;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0217;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0218;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0219;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0200;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0201;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0202;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0203;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0204;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0205;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0206;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0207;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0208;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0209;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0210;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0211;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0212;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0213;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0214;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0215;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0216;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0217;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0218;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0219;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0160;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0161;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0162;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0163;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0164;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0165;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0166;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0167;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0168;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0169;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0170;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0171;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0172;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0173;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0174;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0175;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0176;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0177;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0178;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0179;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0160;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0161;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0162;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0163;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0164;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0165;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0166;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0167;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0168;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0169;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0170;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0171;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0172;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0173;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0174;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0175;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0176;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0177;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0178;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0179;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0080;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0081;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0082;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0083;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0084;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0085;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0086;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0087;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0088;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0089;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0090;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0091;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0092;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0093;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0094;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0095;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0096;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0097;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0098;
    extern const ISP_NVRAM_NR3D_T s5k3p8sxmipiraw_NR3D_0099;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0080;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0081;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0082;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0083;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0084;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0085;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0086;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0087;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0088;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0089;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0090;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0091;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0092;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0093;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0094;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0095;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0096;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0097;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0098;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8sxmipiraw_NBC_LCE_LINK_0099;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0140;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0141;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0142;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0143;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0144;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0145;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0146;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0147;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0148;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0149;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0150;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0151;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0152;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0153;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0154;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0155;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0156;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0157;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0158;
    extern const ISP_NVRAM_UDM_T s5k3p8sxmipiraw_UDM_0159;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0220;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0221;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0222;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0223;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0224;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0225;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0226;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0227;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0228;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0229;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0230;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0231;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0232;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0233;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0234;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0235;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0236;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0237;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0238;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0239;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0240;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0241;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0242;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0243;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0244;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0245;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0246;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0247;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0248;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0249;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0250;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0251;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0252;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0253;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0254;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0255;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0256;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0257;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0258;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0259;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0260;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0261;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0262;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0263;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0264;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0265;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0266;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0267;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0268;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0269;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0270;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0271;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0272;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0273;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0274;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0275;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0276;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0277;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0278;
    extern const ISP_NVRAM_ANR_T s5k3p8sxmipiraw_NBC_ANR_0279;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0220;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0221;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0222;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0223;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0224;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0225;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0226;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0227;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0228;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0229;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0230;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0231;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0232;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0233;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0234;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0235;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0236;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0237;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0238;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0239;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0240;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0241;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0242;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0243;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0244;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0245;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0246;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0247;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0248;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0249;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0250;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0251;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0252;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0253;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0254;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0255;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0256;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0257;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0258;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0259;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0260;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0261;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0262;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0263;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0264;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0265;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0266;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0267;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0268;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0269;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0270;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0271;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0272;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0273;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0274;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0275;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0276;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0277;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0278;
    extern const ISP_NVRAM_ANR2_T s5k3p8sxmipiraw_NBC2_ANR2_0279;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0180;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0181;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0182;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0183;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0184;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0185;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0186;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0187;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0188;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0189;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0190;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0191;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0192;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0193;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0194;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0195;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0196;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0197;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0198;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0199;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0200;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0201;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0202;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0203;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0204;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0205;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0206;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0207;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0208;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0209;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0210;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0211;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0212;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0213;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0214;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0215;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0216;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0217;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0218;
    extern const ISP_NVRAM_HFG_T s5k3p8sxmipiraw_HFG_0219;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0180;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0181;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0182;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0183;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0184;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0185;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0186;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0187;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0188;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0189;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0190;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0191;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0192;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0193;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0194;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0195;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0196;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0197;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0198;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0199;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0200;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0201;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0202;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0203;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0204;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0205;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0206;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0207;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0208;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0209;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0210;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0211;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0212;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0213;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0214;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0215;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0216;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0217;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0218;
    extern const ISP_NVRAM_EE_T s5k3p8sxmipiraw_EE_0219;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0040;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0041;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0042;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0043;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0044;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0045;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0046;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0047;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0048;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0049;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0050;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0051;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0052;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0053;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0054;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0055;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0056;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0057;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0058;
    extern const ISP_NVRAM_MFB_T s5k3p8sxmipiraw_MFB_0059;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0040;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0041;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0042;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0043;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0044;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0045;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0046;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0047;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0048;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0049;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0050;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0051;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0052;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0053;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0054;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0055;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0056;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0057;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0058;
    extern const ISP_NVRAM_MIXER3_T s5k3p8sxmipiraw_MIXER3_0059;
    extern const NVRAM_CAMERA_FEATURE_MFLL_STRUCT s5k3p8sxmipiraw_MFNR_0002;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0060;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0061;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0062;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0063;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0064;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0065;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0066;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0067;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0068;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0069;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0070;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0071;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0072;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0073;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0074;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0075;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0076;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0077;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0078;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT s5k3p8sxmipiraw_SWNR_0079;
    extern const NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT s5k3p8sxmipiraw_SWNR_THRES_0003;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0012;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0013;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0014;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0015;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0016;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0017;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0120;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0121;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0122;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0123;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0124;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0125;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0126;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0127;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0128;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0129;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0130;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0131;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0132;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0133;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0134;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0135;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0136;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0137;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0138;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0139;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0140;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0141;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0142;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0143;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0144;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0145;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0146;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0147;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0148;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0149;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0150;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0151;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0152;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0153;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0154;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0155;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0156;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0157;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0158;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0159;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0160;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0161;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0162;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0163;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0164;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0165;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0166;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0167;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0168;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0169;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0170;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0171;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0172;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0173;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0174;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0175;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0176;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0177;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0178;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8sxmipiraw_COLOR_0179;
    extern const ISP_NVRAM_COLOR_PARAM_T s5k3p8sxmipiraw_COLOR_PARAM_0002;
    extern const FEATURE_NVRAM_CA_LTM_T s5k3p8sxmipiraw_CA_LTM_0003;

}
#endif