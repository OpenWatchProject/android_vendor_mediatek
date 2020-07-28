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
#ifndef _VIDEO_1080_H
#define _VIDEO_1080_H

#include "camera_custom_nvram.h"

extern "C"
{
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0040;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0041;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0042;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0043;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0044;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0045;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0046;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0047;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0048;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0049;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0050;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0051;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0052;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0053;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0054;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0055;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0056;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0057;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0058;
    extern const ISP_NVRAM_DBS_T a5142mipiraw_DBS_0059;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0040;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0041;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0042;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0043;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0044;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0045;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0046;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0047;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0048;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0049;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0050;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0051;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0052;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0053;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0054;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0055;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0056;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0057;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0058;
    extern const ISP_NVRAM_ADBS_T a5142mipiraw_ADBS_0059;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0040;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0041;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0042;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0043;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0044;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0045;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0046;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0047;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0048;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0049;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0050;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0051;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0052;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0053;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0054;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0055;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0056;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0057;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0058;
    extern const ISP_NVRAM_OBC_T a5142mipiraw_OBC_0059;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0040;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0041;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0042;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0043;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0044;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0045;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0046;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0047;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0048;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0049;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0050;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0051;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0052;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0053;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0054;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0055;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0056;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0057;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0058;
    extern const ISP_NVRAM_BNR_BPC_T a5142mipiraw_BNR_BPC_0059;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0040;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0041;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0042;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0043;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0044;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0045;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0046;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0047;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0048;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0049;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0050;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0051;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0052;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0053;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0054;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0055;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0056;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0057;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0058;
    extern const ISP_NVRAM_BNR_NR1_T a5142mipiraw_BNR_NR1_0059;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0040;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0041;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0042;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0043;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0044;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0045;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0046;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0047;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0048;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0049;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0050;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0051;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0052;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0053;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0054;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0055;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0056;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0057;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0058;
    extern const ISP_NVRAM_BNR_PDC_T a5142mipiraw_BNR_PDC_0059;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0040;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0041;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0042;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0043;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0044;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0045;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0046;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0047;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0048;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0049;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0050;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0051;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0052;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0053;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0054;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0055;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0056;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0057;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0058;
    extern const ISP_NVRAM_RMM_T a5142mipiraw_RMM_0059;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0040;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0041;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0042;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0043;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0044;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0045;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0046;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0047;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0048;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0049;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0050;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0051;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0052;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0053;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0054;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0055;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0056;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0057;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0058;
    extern const ISP_NVRAM_RNR_T a5142mipiraw_RNR_0059;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0100;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0101;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0102;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0103;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0104;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0105;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0106;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0107;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0108;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0109;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0110;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0111;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0112;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0113;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0114;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0115;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0116;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0117;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0118;
    extern const ISP_NVRAM_UDM_T a5142mipiraw_UDM_0119;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0180;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0181;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0182;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0183;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0184;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0185;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0186;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0187;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0188;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0189;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0190;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0191;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0192;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0193;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0194;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0195;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0196;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0197;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0198;
    extern const ISP_NVRAM_ANR_T a5142mipiraw_NBC_ANR_0199;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0180;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0181;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0182;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0183;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0184;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0185;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0186;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0187;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0188;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0189;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0190;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0191;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0192;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0193;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0194;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0195;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0196;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0197;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0198;
    extern const ISP_NVRAM_ANR2_T a5142mipiraw_NBC2_ANR2_0199;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0140;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0141;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0142;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0143;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0144;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0145;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0146;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0147;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0148;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0149;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0150;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0151;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0152;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0153;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0154;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0155;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0156;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0157;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0158;
    extern const ISP_NVRAM_HFG_T a5142mipiraw_HFG_0159;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0140;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0141;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0142;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0143;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0144;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0145;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0146;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0147;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0148;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0149;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0150;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0151;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0152;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0153;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0154;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0155;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0156;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0157;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0158;
    extern const ISP_NVRAM_EE_T a5142mipiraw_EE_0159;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0060;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0061;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0062;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0063;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0064;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0065;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0066;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0067;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0068;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0069;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0070;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0071;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0072;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0073;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0074;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0075;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0076;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0077;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0078;
    extern const ISP_NVRAM_NR3D_T a5142mipiraw_NR3D_0079;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0060;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0061;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0062;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0063;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0064;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0065;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0066;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0067;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0068;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0069;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0070;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0071;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0072;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0073;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0074;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0075;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0076;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0077;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0078;
    extern const ISP_NBC_LTM_SW_PARAM_T a5142mipiraw_NBC_LCE_LINK_0079;
    extern const FEATURE_NVRAM_CA_LTM_T a5142mipiraw_CA_LTM_0002;

}
#endif