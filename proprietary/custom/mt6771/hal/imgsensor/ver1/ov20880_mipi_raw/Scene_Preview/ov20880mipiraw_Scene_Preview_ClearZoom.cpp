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
#include "ov20880mipiraw_Scene_Preview.h"

const ISP_NVRAM_CLEARZOOM_T ov20880mipiraw_ClearZoom_0001 = {
    .ZOOM = {3, 100, 200, 400, -1, -1},
    .ISO = {10, 100, 200, 400, 800, 1600, 2400, 3200, 4800, 6400, 9600},
    .CLZ_TUNING={
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        },
        {
            .RszEntrySWReg={
                .ultraResEnable = 0,
                .IBSE_clip_ratio = 0,
                .IBSE_clip_thr = 0,
                .IBSE_gaincontrol_gain = 0,
                .tapAdaptSlope = 0,
                .dynIBSE_gain = 0,   
                },
            .RszEntryHWReg={
                .IBSE_gaincontrol_coring_value = 0,
                .IBSE_gaincontrol_coring_thr = 0,
                .IBSE_gaincontrol_coring_zero = 0,
                .IBSE_gaincontrol_softlimit_ratio = 0,
                .IBSE_gaincontrol_bound = 0,
                .IBSE_gaincontrol_limit = 0,
                .IBSE_gaincontrol_softcoring_gain = 0,
                },
            .iDSHWRegEntry={
                .tdshp_en = 0,
                .tdshp_gain_mid = 0,
                .tdshp_gain_high = 0,
                .tdshp_softcoring_gain = 0,
    
                .tdshp_coring_thr = 0,
                .tdshp_coring_zero = 0,
                .tdshp_gain = 0,
                .tdshp_limit_ratio = 0,
                .tdshp_limit = 0,
                .tdshp_bound = 0,
                .tdshp_coring_value = 0,
    
                .tdshp_clip_en = 0,
                .tdshp_clip_ratio = 0,
                .tdshp_clip_thr = 0,
    
                .pbc1_gain = 0,
                .pbc1_lpf_gain = 0,
    
                .pbc2_gain = 0,
                .pbc2_lpf_gain = 0,
    
                .pbc3_gain = 0,
                .pbc3_lpf_gain = 0,
    
                .edf_flat_gain = 0,
                .edf_detail_gain = 0,
                .edf_edge_gain = 0,
                },
        }
    } 
    };
