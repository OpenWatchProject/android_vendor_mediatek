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
#ifndef _SCENE_PREVIEW_H
#define _SCENE_PREVIEW_H

#include "camera_custom_nvram.h"

extern "C"
{
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0020;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0021;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0022;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0023;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0024;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0025;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0026;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0027;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0028;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0029;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0030;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0031;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0032;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0033;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0034;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0035;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0036;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0037;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0038;
    extern const ISP_NVRAM_DBS_T s5k3p8spmipiraw_DBS_0039;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0020;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0021;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0022;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0023;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0024;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0025;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0026;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0027;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0028;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0029;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0030;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0031;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0032;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0033;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0034;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0035;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0036;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0037;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0038;
    extern const ISP_NVRAM_ADBS_T s5k3p8spmipiraw_ADBS_0039;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0020;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0021;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0022;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0023;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0024;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0025;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0026;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0027;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0028;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0029;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0030;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0031;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0032;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0033;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0034;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0035;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0036;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0037;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0038;
    extern const ISP_NVRAM_OBC_T s5k3p8spmipiraw_OBC_0039;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0020;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0021;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0022;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0023;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0024;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0025;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0026;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0027;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0028;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0029;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0030;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0031;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0032;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0033;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0034;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0035;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0036;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0037;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0038;
    extern const ISP_NVRAM_BNR_BPC_T s5k3p8spmipiraw_BNR_BPC_0039;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0020;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0021;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0022;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0023;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0024;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0025;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0026;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0027;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0028;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0029;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0030;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0031;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0032;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0033;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0034;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0035;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0036;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0037;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0038;
    extern const ISP_NVRAM_BNR_NR1_T s5k3p8spmipiraw_BNR_NR1_0039;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0020;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0021;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0022;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0023;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0024;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0025;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0026;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0027;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0028;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0029;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0030;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0031;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0032;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0033;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0034;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0035;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0036;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0037;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0038;
    extern const ISP_NVRAM_BNR_PDC_T s5k3p8spmipiraw_BNR_PDC_0039;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0020;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0021;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0022;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0023;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0024;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0025;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0026;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0027;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0028;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0029;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0030;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0031;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0032;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0033;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0034;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0035;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0036;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0037;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0038;
    extern const ISP_NVRAM_RMM_T s5k3p8spmipiraw_RMM_0039;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0020;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0021;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0022;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0023;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0024;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0025;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0026;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0027;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0028;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0029;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0030;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0031;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0032;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0033;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0034;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0035;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0036;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0037;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0038;
    extern const ISP_NVRAM_RNR_T s5k3p8spmipiraw_RNR_0039;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0080;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0081;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0082;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0083;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0084;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0085;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0086;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0087;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0088;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0089;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0090;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0091;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0092;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0093;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0094;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0095;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0096;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0097;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0098;
    extern const ISP_NVRAM_UDM_T s5k3p8spmipiraw_UDM_0099;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0160;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0161;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0162;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0163;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0164;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0165;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0166;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0167;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0168;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0169;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0170;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0171;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0172;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0173;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0174;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0175;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0176;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0177;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0178;
    extern const ISP_NVRAM_ANR_T s5k3p8spmipiraw_NBC_ANR_0179;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0160;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0161;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0162;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0163;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0164;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0165;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0166;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0167;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0168;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0169;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0170;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0171;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0172;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0173;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0174;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0175;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0176;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0177;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0178;
    extern const ISP_NVRAM_ANR2_T s5k3p8spmipiraw_NBC2_ANR2_0179;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0120;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0121;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0122;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0123;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0124;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0125;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0126;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0127;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0128;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0129;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0130;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0131;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0132;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0133;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0134;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0135;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0136;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0137;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0138;
    extern const ISP_NVRAM_HFG_T s5k3p8spmipiraw_HFG_0139;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0120;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0121;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0122;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0123;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0124;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0125;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0126;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0127;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0128;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0129;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0130;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0131;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0132;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0133;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0134;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0135;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0136;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0137;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0138;
    extern const ISP_NVRAM_EE_T s5k3p8spmipiraw_EE_0139;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0040;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0041;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0042;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0043;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0044;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0045;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0046;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0047;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0048;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0049;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0050;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0051;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0052;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0053;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0054;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0055;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0056;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0057;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0058;
    extern const ISP_NVRAM_NR3D_T s5k3p8spmipiraw_NR3D_0059;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0040;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0041;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0042;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0043;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0044;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0045;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0046;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0047;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0048;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0049;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0050;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0051;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0052;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0053;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0054;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0055;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0056;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0057;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0058;
    extern const ISP_NBC_LTM_SW_PARAM_T s5k3p8spmipiraw_NBC_LCE_LINK_0059;
    extern const ISP_NVRAM_LCE_TUNING_PARAM_T s5k3p8spmipiraw_LCE_0001;
    extern const ISP_NVRAM_GMA_STRUCT_T s5k3p8spmipiraw_GMA_0001;
    extern const ISP_NVRAM_DCE_TUNING_PARAM_T s5k3p8spmipiraw_DCE_0001;
    extern const FEATURE_NVRAM_CA_LTM_T s5k3p8spmipiraw_CA_LTM_0001;
    extern const ISP_NVRAM_CLEARZOOM_T s5k3p8spmipiraw_ClearZoom_0001;

}
#endif