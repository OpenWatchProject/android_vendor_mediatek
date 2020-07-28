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
#ifndef _FACE_CAPTURE_H
#define _FACE_CAPTURE_H

#include "camera_custom_nvram.h"

extern "C"
{
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0040;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0041;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0042;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0043;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0044;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0045;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0046;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0047;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0048;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0049;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0050;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0051;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0052;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0053;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0054;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0055;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0056;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0057;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0058;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0059;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0080;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0081;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0082;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0083;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0084;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0085;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0086;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0087;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0088;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0089;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0090;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0091;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0092;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0093;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0094;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0095;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0096;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0097;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0098;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0099;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0080;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0081;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0082;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0083;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0084;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0085;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0086;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0087;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0088;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0089;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0090;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0091;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0092;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0093;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0094;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0095;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0096;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0097;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0098;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0099;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0060;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0061;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0062;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0063;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0064;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0065;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0066;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0067;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0068;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0069;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0070;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0071;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0072;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0073;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0074;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0075;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0076;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0077;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0078;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0079;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0060;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0061;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0062;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0063;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0064;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0065;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0066;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0067;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0068;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0069;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0070;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0071;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0072;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0073;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0074;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0075;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0076;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0077;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0078;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0079;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0020;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0021;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0022;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0023;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0024;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0025;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0026;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0027;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0028;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0029;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0030;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0031;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0032;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0033;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0034;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0035;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0036;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0037;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0038;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0039;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0020;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0021;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0022;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0023;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0024;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0025;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0026;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0027;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0028;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0029;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0030;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0031;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0032;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0033;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0034;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0035;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0036;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0037;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0038;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0039;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0060;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0061;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0062;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0063;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0064;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0065;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0066;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0067;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0068;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0069;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0070;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0071;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0072;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0073;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0074;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0075;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0076;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0077;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0078;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0079;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0100;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0101;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0102;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0103;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0104;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0105;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0106;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0107;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0108;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0109;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0110;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0111;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0112;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0113;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0114;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0115;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0116;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0117;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0118;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0119;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0120;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0121;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0122;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0123;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0124;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0125;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0126;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0127;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0128;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0129;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0130;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0131;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0132;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0133;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0134;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0135;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0136;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0137;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0138;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0139;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0140;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0141;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0142;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0143;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0144;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0145;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0146;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0147;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0148;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0149;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0150;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0151;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0152;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0153;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0154;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0155;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0156;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0157;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0158;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0159;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0100;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0101;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0102;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0103;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0104;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0105;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0106;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0107;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0108;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0109;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0110;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0111;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0112;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0113;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0114;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0115;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0116;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0117;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0118;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0119;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0120;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0121;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0122;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0123;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0124;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0125;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0126;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0127;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0128;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0129;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0130;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0131;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0132;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0133;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0134;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0135;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0136;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0137;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0138;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0139;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0140;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0141;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0142;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0143;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0144;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0145;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0146;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0147;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0148;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0149;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0150;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0151;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0152;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0153;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0154;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0155;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0156;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0157;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0158;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0159;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0080;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0081;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0082;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0083;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0084;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0085;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0086;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0087;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0088;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0089;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0090;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0091;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0092;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0093;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0094;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0095;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0096;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0097;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0098;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0099;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0100;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0101;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0102;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0103;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0104;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0105;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0106;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0107;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0108;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0109;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0110;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0111;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0112;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0113;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0114;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0115;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0116;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0117;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0118;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0119;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0080;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0081;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0082;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0083;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0084;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0085;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0086;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0087;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0088;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0089;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0090;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0091;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0092;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0093;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0094;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0095;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0096;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0097;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0098;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0099;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0100;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0101;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0102;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0103;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0104;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0105;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0106;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0107;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0108;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0109;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0110;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0111;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0112;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0113;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0114;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0115;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0116;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0117;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0118;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0119;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0020;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0021;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0022;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0023;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0024;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0025;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0026;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0027;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0028;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0029;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0030;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0031;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0032;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0033;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0034;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0035;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0036;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0037;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0038;
    extern const ISP_NVRAM_MFB_T a5142mipiraw_MFB_0039;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0020;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0021;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0022;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0023;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0024;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0025;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0026;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0027;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0028;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0029;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0030;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0031;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0032;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0033;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0034;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0035;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0036;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0037;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0038;
    extern const ISP_NVRAM_MIXER3_T a5142mipiraw_MIXER3_0039;
    extern const NVRAM_CAMERA_FEATURE_MFLL_STRUCT a5142mipiraw_MFNR_0001;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0040;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0041;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0042;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0043;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0044;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0045;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0046;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0047;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0048;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0049;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0050;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0051;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0052;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0053;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0054;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0055;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0056;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0057;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0058;
    extern const NVRAM_CAMERA_FEATURE_SWNR_STRUCT a5142mipiraw_SWNR_0059;
    extern const NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT a5142mipiraw_SWNR_THRES_0002;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT a5142mipiraw_CCM_0006;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT a5142mipiraw_CCM_0007;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT a5142mipiraw_CCM_0008;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT a5142mipiraw_CCM_0009;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT a5142mipiraw_CCM_0010;
    extern const ISP_NVRAM_MULTI_CCM_STRUCT a5142mipiraw_CCM_0011;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0060;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0061;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0062;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0063;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0064;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0065;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0066;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0067;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0068;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0069;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0070;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0071;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0072;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0073;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0074;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0075;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0076;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0077;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0078;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0079;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0080;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0081;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0082;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0083;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0084;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0085;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0086;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0087;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0088;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0089;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0090;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0091;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0092;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0093;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0094;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0095;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0096;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0097;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0098;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0099;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0100;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0101;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0102;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0103;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0104;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0105;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0106;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0107;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0108;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0109;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0110;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0111;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0112;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0113;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0114;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0115;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0116;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0117;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0118;
    extern const ISP_NVRAM_SCOLOR_PARAM_T a5142mipiraw_COLOR_0119;
    extern const ISP_NVRAM_COLOR_PARAM_T a5142mipiraw_COLOR_PARAM_0001;

}
#endif