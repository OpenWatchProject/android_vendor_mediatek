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
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0120;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0121;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0122;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0123;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0124;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0125;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0126;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0127;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0128;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0129;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0130;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0131;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0132;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0133;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0134;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0135;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0136;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0137;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0138;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0139;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0200;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0201;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0202;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0203;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0204;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0205;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0206;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0207;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0208;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0209;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0210;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0211;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0212;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0213;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0214;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0215;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0216;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0217;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0218;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0219;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0200;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0201;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0202;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0203;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0204;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0205;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0206;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0207;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0208;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0209;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0210;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0211;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0212;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0213;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0214;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0215;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0216;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0217;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0218;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0219;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0160;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0161;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0162;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0163;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0164;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0165;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0166;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0167;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0168;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0169;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0170;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0171;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0172;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0173;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0174;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0175;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0176;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0177;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0178;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0179;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0160;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0161;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0162;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0163;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0164;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0165;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0166;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0167;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0168;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0169;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0170;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0171;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0172;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0173;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0174;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0175;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0176;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0177;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0178;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0179;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0080;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0081;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0082;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0083;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0084;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0085;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0086;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0087;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0088;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0089;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0090;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0091;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0092;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0093;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0094;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0095;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0096;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0097;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0098;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0099;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0080;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0081;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0082;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0083;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0084;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0085;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0086;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0087;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0088;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0089;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0090;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0091;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0092;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0093;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0094;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0095;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0096;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0097;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0098;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0099;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0012;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0013;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0014;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0015;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0016;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8spmipiraw_CCM_0017;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0120;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0121;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0122;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0123;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0124;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0125;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0126;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0127;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0128;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0129;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0130;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0131;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0132;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0133;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0134;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0135;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0136;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0137;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0138;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0139;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0140;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0141;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0142;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0143;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0144;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0145;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0146;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0147;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0148;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0149;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0150;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0151;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0152;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0153;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0154;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0155;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0156;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0157;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0158;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0159;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0160;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0161;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0162;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0163;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0164;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0165;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0166;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0167;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0168;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0169;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0170;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0171;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0172;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0173;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0174;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0175;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0176;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0177;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0178;
    extern const ISP_NVRAM_SCOLOR_PARAM_T s5k3p8spmipiraw_COLOR_0179;
    extern const ISP_NVRAM_COLOR_PARAM_T s5k3p8spmipiraw_COLOR_PARAM_0002;
    extern const FEATURE_NVRAM_CA_LTM_T s5k3p8spmipiraw_CA_LTM_0001;

}
#endif