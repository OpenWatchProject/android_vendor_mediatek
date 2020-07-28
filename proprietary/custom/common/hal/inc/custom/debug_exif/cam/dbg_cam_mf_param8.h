/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM8_H_
#define _MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM8_H_
#pragma once

#include "../dbg_exif_def.h"

//
// Debug Exif Version 8 - MT6758
//
namespace dbg_cam_mf_param_8 {

 // MF debug info
enum { MF_DEBUG_TAG_VERSION = 8 };
enum { MF_DEBUG_TAG_SUBVERSION = 0 };

//MF Parameter Structure
typedef enum
{
    // ------------------------------------------------------------------------
    // MFNR SW related info
    // {{{
    MF_TAG_VERSION = 0,             //sw info. n, for 3.n
    MF_TAG_MFB_MODE,                // 0: disable, 1: MFLL, 2: AIS
    MF_TAG_MAX_FRAME_NUMBER,        // capture number
    MF_TAG_PROCESSING_NUMBER,       // blended number
    MF_TAG_EXPOSURE,
    MF_TAG_ISO,
    MF_TAG_MFNR_ISO_TH,             // threshold to trigger MFNR
    MF_TAG_ISO_HI_TH,               // non-use now
    MF_TAG_ISO_LOW_TH,              // non-use now
    MF_TAG_RAW_WIDTH,
    MF_TAG_RAW_HEIGHT,
    MF_TAG_BSS_ENABLE,
    MF_TAG_BSS_ROI_WIDTH,
    MF_TAG_BSS_ROI_HEIGHT,
    MF_TAG_BSS_ROI_X0,
    MF_TAG_BSS_ROI_Y0,
    MF_TAG_BSS_SCALE_FACTOR,
    MF_TAG_BSS_CLIP_TH0,
    MF_TAG_BSS_CLIP_TH1,
    MF_TAG_BSS_ZERO,
    MF_TAG_BSS_BEST_IDX,            // selected frame
    MF_TAG_BSS_ADF_TH,
    MF_TAG_BSS_SDF_TH,
    MF_TAG_GMV_00,                  // Global motion vector frame 0 ~ 11
    MF_TAG_GMV_01,
    MF_TAG_GMV_02,
    MF_TAG_GMV_03,
    MF_TAG_GMV_04,
    MF_TAG_GMV_05,
    MF_TAG_GMV_06,
    MF_TAG_GMV_07,
    MF_TAG_GMV_08,
    MF_TAG_GMV_09,
    MF_TAG_GMV_10,
    MF_TAG_GMV_11,
    MF_TAG_GYRO_X0_00,              // Gyro info frame idx 0, each frame has 3 gyro info
    MF_TAG_GYRO_Y0_00,
    MF_TAG_GYRO_Z0_00,
    MF_TAG_GYRO_X1_00,
    MF_TAG_GYRO_Y1_00,
    MF_TAG_GYRO_Z1_00,
    MF_TAG_GYRO_X2_00,
    MF_TAG_GYRO_Y2_00,
    MF_TAG_GYRO_Z2_00,
    MF_TAG_GYRO_X0_01,              // Gyro info frame idx 1
    MF_TAG_GYRO_Y0_01,
    MF_TAG_GYRO_Z0_01,
    MF_TAG_GYRO_X1_01,
    MF_TAG_GYRO_Y1_01,
    MF_TAG_GYRO_Z1_01,
    MF_TAG_GYRO_X2_01,
    MF_TAG_GYRO_Y2_01,
    MF_TAG_GYRO_Z2_01,
    MF_TAG_GYRO_X0_02,              // Gyro info frame idx 2
    MF_TAG_GYRO_Y0_02,
    MF_TAG_GYRO_Z0_02,
    MF_TAG_GYRO_X1_02,
    MF_TAG_GYRO_Y1_02,
    MF_TAG_GYRO_Z1_02,
    MF_TAG_GYRO_X2_02,
    MF_TAG_GYRO_Y2_02,
    MF_TAG_GYRO_Z2_02,
    MF_TAG_GYRO_X0_03,              // Gyro info frame idx 3
    MF_TAG_GYRO_Y0_03,
    MF_TAG_GYRO_Z0_03,
    MF_TAG_GYRO_X1_03,
    MF_TAG_GYRO_Y1_03,
    MF_TAG_GYRO_Z1_03,
    MF_TAG_GYRO_X2_03,
    MF_TAG_GYRO_Y2_03,
    MF_TAG_GYRO_Z2_03,
    MF_TAG_GYRO_X0_04,              // Gyro info frame idx 4
    MF_TAG_GYRO_Y0_04,
    MF_TAG_GYRO_Z0_04,
    MF_TAG_GYRO_X1_04,
    MF_TAG_GYRO_Y1_04,
    MF_TAG_GYRO_Z1_04,
    MF_TAG_GYRO_X2_04,
    MF_TAG_GYRO_Y2_04,
    MF_TAG_GYRO_Z2_04,
    MF_TAG_GYRO_X0_05,              // Gyro info frame idx 5
    MF_TAG_GYRO_Y0_05,
    MF_TAG_GYRO_Z0_05,
    MF_TAG_GYRO_X1_05,
    MF_TAG_GYRO_Y1_05,
    MF_TAG_GYRO_Z1_05,
    MF_TAG_GYRO_X2_05,
    MF_TAG_GYRO_Y2_05,
    MF_TAG_GYRO_Z2_05,
    MF_TAG_MEMC_WIDTH,
    MF_TAG_MEMC_HEIGHT,
    MF_TAG_MEMC_INPUT_FORMAT,
    MF_TAG_MEMC_OUTPUT_FORMAT,
    MF_TAG_MEMC_SKIP,
    MF_TAG_MEMC_MCFULLSIZE,         // 1: full size MC, 0: quarter size MC
    MF_TAG_MEMC_BAD_MB_COUNT0,
    MF_TAG_MEMC_BAD_MB_COUNT1,
    MF_TAG_MEMC_BAD_MB_COUNT2,
    MF_TAG_MEMC_BAD_MB_TH,
    MF_TAG_MEMC_BAD_MV_RANGE,
    MF_TAG_MEMC_NOISE_LVL,
    MF_TAG_MEMC_MPME,
    MF_TAG_MEMC_CONFMAP,
    MF_TAG_AIS_ISO_LEVEL1,
    MF_TAG_AIS_ISO_LEVEL2,
    MF_TAG_AIS_ISO_LEVEL3,
    MF_TAG_AIS_ISO_LEVEL4,
    MF_TAG_AIS_ISO_LEVEL5,
    MF_TAG_AIS_ISO_LEVEL6,
    MF_TAG_AIS_ISO_LEVEL7,
    MF_TAG_AIS_FRAME_NUM1,
    MF_TAG_AIS_FRAME_NUM2,
    MF_TAG_AIS_FRAME_NUM3,
    MF_TAG_AIS_FRAME_NUM4,
    MF_TAG_AIS_FRAME_NUM5,
    MF_TAG_AIS_FRAME_NUM6,
    MF_TAG_AIS_FRAME_NUM7,
    MF_TAG_AIS_FRAME_NUM8,
    // }}}

    // ------------------------------------------------------------------------
    // Stage SF
    // {{{
    // ANR
    MF_TAG_SINGLE_DIP_X_ANR_CON1                ,
    MF_TAG_SINGLE_DIP_X_ANR_CON2                ,
    MF_TAG_SINGLE_DIP_X_ANR_YAD1                ,
    MF_TAG_SINGLE_DIP_X_ANR_YAD2                ,
    MF_TAG_SINGLE_DIP_X_ANR_Y4LUT1              ,
    MF_TAG_SINGLE_DIP_X_ANR_Y4LUT2              ,
    MF_TAG_SINGLE_DIP_X_ANR_Y4LUT3              ,
    MF_TAG_SINGLE_DIP_X_ANR_C4LUT1              ,
    MF_TAG_SINGLE_DIP_X_ANR_C4LUT2              ,
    MF_TAG_SINGLE_DIP_X_ANR_C4LUT3              ,
    MF_TAG_SINGLE_DIP_X_ANR_A4LUT2              ,
    MF_TAG_SINGLE_DIP_X_ANR_A4LUT3              ,
    MF_TAG_SINGLE_DIP_X_ANR_L4LUT1              ,
    MF_TAG_SINGLE_DIP_X_ANR_L4LUT2              ,
    MF_TAG_SINGLE_DIP_X_ANR_L4LUT3              ,
    MF_TAG_SINGLE_DIP_X_ANR_PTY                 ,
    MF_TAG_SINGLE_DIP_X_ANR_CAD                 ,
    MF_TAG_SINGLE_DIP_X_ANR_PTC                 ,
    MF_TAG_SINGLE_DIP_X_ANR_LCE                 ,
    MF_TAG_SINGLE_DIP_X_ANR_T4LUT1              ,
    MF_TAG_SINGLE_DIP_X_ANR_T4LUT2              ,
    MF_TAG_SINGLE_DIP_X_ANR_T4LUT3              ,
    MF_TAG_SINGLE_DIP_X_ANR_ACT1                ,
    MF_TAG_SINGLE_DIP_X_ANR_ACT2                ,
    MF_TAG_SINGLE_DIP_X_ANR_ACT4                ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTYHL              ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTYHH              ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTYL               ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTYHL2             ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTYHH2             ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTYL2              ,
    MF_TAG_SINGLE_DIP_X_ANR_ACTC                ,
    MF_TAG_SINGLE_DIP_X_ANR_YLAD                ,
    MF_TAG_SINGLE_DIP_X_ANR_YLAD2               ,
    MF_TAG_SINGLE_DIP_X_ANR_YLAD3               ,
    MF_TAG_SINGLE_DIP_X_ANR_PTYL                ,
    MF_TAG_SINGLE_DIP_X_ANR_LCOEF               ,
    MF_TAG_SINGLE_DIP_X_ANR_YDIR                ,
    MF_TAG_SINGLE_DIP_X_ANR_RSV1                ,


    // ANR2
    MF_TAG_SINGLE_DIP_X_ANR2_CON1               ,
    MF_TAG_SINGLE_DIP_X_ANR2_CON2               ,
    MF_TAG_SINGLE_DIP_X_ANR2_YAD1               ,
    MF_TAG_SINGLE_DIP_X_ANR2_Y4LUT1             ,
    MF_TAG_SINGLE_DIP_X_ANR2_Y4LUT2             ,
    MF_TAG_SINGLE_DIP_X_ANR2_Y4LUT3             ,
    MF_TAG_SINGLE_DIP_X_ANR2_L4LUT1             ,
    MF_TAG_SINGLE_DIP_X_ANR2_L4LUT2             ,
    MF_TAG_SINGLE_DIP_X_ANR2_L4LUT3             ,
    MF_TAG_SINGLE_DIP_X_ANR2_CAD                ,
    MF_TAG_SINGLE_DIP_X_ANR2_PTC                ,
    MF_TAG_SINGLE_DIP_X_ANR2_LCE                ,
    MF_TAG_SINGLE_DIP_X_ANR2_MED1               ,
    MF_TAG_SINGLE_DIP_X_ANR2_MED2               ,
    MF_TAG_SINGLE_DIP_X_ANR2_MED3               ,
    MF_TAG_SINGLE_DIP_X_ANR2_MED4               ,
    MF_TAG_SINGLE_DIP_X_ANR2_MED5               ,
    MF_TAG_SINGLE_DIP_X_ANR2_ACTC               ,
    MF_TAG_SINGLE_DIP_X_ANR2_RSV1               ,
    MF_TAG_SINGLE_DIP_X_ANR2_RSV2               ,

    // SEEE
    MF_TAG_SINGLE_DIP_X_SEEE_CTRL               ,
    MF_TAG_SINGLE_DIP_X_SEEE_CLIP_CTRL_1        ,
    MF_TAG_SINGLE_DIP_X_SEEE_CLIP_CTRL_2        ,
    MF_TAG_SINGLE_DIP_X_SEEE_CLIP_CTRL_3        ,
    MF_TAG_SINGLE_DIP_X_SEEE_BLND_CTRL_1        ,
    MF_TAG_SINGLE_DIP_X_SEEE_BLND_CTRL_2        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GN_CTRL            ,
    MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_1        ,
    MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_2        ,
    MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_3        ,
    MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_4        ,
    MF_TAG_SINGLE_DIP_X_SEEE_SLNK_CTRL_1        ,
    MF_TAG_SINGLE_DIP_X_SEEE_SLNK_CTRL_2        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_1        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_2        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_3        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_4        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_5        ,
    MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_6        ,
    MF_TAG_SINGLE_DIP_X_SEEE_OUT_EDGE_CTRL      ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_Y_CTRL          ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_EDGE_CTRL_1     ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_EDGE_CTRL_2     ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_EDGE_CTRL_3     ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_SPECL_CTRL      ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_CORE_CTRL_1     ,
    MF_TAG_SINGLE_DIP_X_SEEE_SE_CORE_CTRL_2     ,

    // HFG
    MF_TAG_SINGLE_DIP_X_HFG_CON_0               ,
    MF_TAG_SINGLE_DIP_X_HFG_LUMA_0              ,
    MF_TAG_SINGLE_DIP_X_HFG_LUMA_1              ,
    MF_TAG_SINGLE_DIP_X_HFG_LUMA_2              ,
    MF_TAG_SINGLE_DIP_X_HFG_LCE_0               ,
    MF_TAG_SINGLE_DIP_X_HFG_LCE_1               ,
    MF_TAG_SINGLE_DIP_X_HFG_LCE_2               ,
    MF_TAG_SINGLE_DIP_X_HFG_RAN_0               ,
    MF_TAG_SINGLE_DIP_X_HFG_RAN_1               ,
    MF_TAG_SINGLE_DIP_X_HFG_RAN_2               ,
    MF_TAG_SINGLE_DIP_X_HFG_RAN_3               ,
    MF_TAG_SINGLE_DIP_X_HFG_RAN_4               ,
    MF_TAG_SINGLE_DIP_X_HFG_CROP_X              ,
    MF_TAG_SINGLE_DIP_X_HFG_CROP_Y              ,


    // Stage SF }}}

    // ------------------------------------------------------------------------
    // Stage BFBLD
    // {{{
    // UDM
    MF_TAG_BEFORE_DIP_X_UDM_INTP_CRS            ,
    MF_TAG_BEFORE_DIP_X_UDM_INTP_NAT            ,
    MF_TAG_BEFORE_DIP_X_UDM_INTP_AUG            ,
    MF_TAG_BEFORE_DIP_X_UDM_LUMA_LUT1           ,
    MF_TAG_BEFORE_DIP_X_UDM_LUMA_LUT2           ,
    MF_TAG_BEFORE_DIP_X_UDM_SL_CTL              ,
    MF_TAG_BEFORE_DIP_X_UDM_HFTD_CTL            ,
    MF_TAG_BEFORE_DIP_X_UDM_NR_STR              ,
    MF_TAG_BEFORE_DIP_X_UDM_NR_ACT              ,
    MF_TAG_BEFORE_DIP_X_UDM_HF_STR              ,
    MF_TAG_BEFORE_DIP_X_UDM_HF_ACT1             ,
    MF_TAG_BEFORE_DIP_X_UDM_HF_ACT2             ,
    MF_TAG_BEFORE_DIP_X_UDM_CLIP                ,
    MF_TAG_BEFORE_DIP_X_UDM_DSB                 ,
    MF_TAG_BEFORE_DIP_X_UDM_TILE_EDGE           ,
    MF_TAG_BEFORE_DIP_X_UDM_DSL                 ,
    MF_TAG_BEFORE_DIP_X_UDM_LR_RAT              ,
    MF_TAG_BEFORE_DIP_X_UDM_SPARE_2             ,
    MF_TAG_BEFORE_DIP_X_UDM_SPARE_3             ,

    // ANR
    MF_TAG_BEFORE_DIP_X_ANR_CON1                ,
    MF_TAG_BEFORE_DIP_X_ANR_CON2                ,
    MF_TAG_BEFORE_DIP_X_ANR_YAD1                ,
    MF_TAG_BEFORE_DIP_X_ANR_YAD2                ,
    MF_TAG_BEFORE_DIP_X_ANR_Y4LUT1              ,
    MF_TAG_BEFORE_DIP_X_ANR_Y4LUT2              ,
    MF_TAG_BEFORE_DIP_X_ANR_Y4LUT3              ,
    MF_TAG_BEFORE_DIP_X_ANR_C4LUT1              ,
    MF_TAG_BEFORE_DIP_X_ANR_C4LUT2              ,
    MF_TAG_BEFORE_DIP_X_ANR_C4LUT3              ,
    MF_TAG_BEFORE_DIP_X_ANR_A4LUT2              ,
    MF_TAG_BEFORE_DIP_X_ANR_A4LUT3              ,
    MF_TAG_BEFORE_DIP_X_ANR_L4LUT1              ,
    MF_TAG_BEFORE_DIP_X_ANR_L4LUT2              ,
    MF_TAG_BEFORE_DIP_X_ANR_L4LUT3              ,
    MF_TAG_BEFORE_DIP_X_ANR_PTY                 ,
    MF_TAG_BEFORE_DIP_X_ANR_CAD                 ,
    MF_TAG_BEFORE_DIP_X_ANR_PTC                 ,
    MF_TAG_BEFORE_DIP_X_ANR_LCE                 ,
    MF_TAG_BEFORE_DIP_X_ANR_T4LUT1              ,
    MF_TAG_BEFORE_DIP_X_ANR_T4LUT2              ,
    MF_TAG_BEFORE_DIP_X_ANR_T4LUT3              ,
    MF_TAG_BEFORE_DIP_X_ANR_ACT1                ,
    MF_TAG_BEFORE_DIP_X_ANR_ACT2                ,
    MF_TAG_BEFORE_DIP_X_ANR_ACT4                ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTYHL              ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTYHH              ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTYL               ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTYHL2             ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTYHH2             ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTYL2              ,
    MF_TAG_BEFORE_DIP_X_ANR_ACTC                ,
    MF_TAG_BEFORE_DIP_X_ANR_YLAD                ,
    MF_TAG_BEFORE_DIP_X_ANR_YLAD2               ,
    MF_TAG_BEFORE_DIP_X_ANR_YLAD3               ,
    MF_TAG_BEFORE_DIP_X_ANR_PTYL                ,
    MF_TAG_BEFORE_DIP_X_ANR_LCOEF               ,
    MF_TAG_BEFORE_DIP_X_ANR_YDIR                ,
    MF_TAG_BEFORE_DIP_X_ANR_RSV1                ,

    // ANR2
    MF_TAG_BEFORE_DIP_X_ANR2_CON1               ,
    MF_TAG_BEFORE_DIP_X_ANR2_CON2               ,
    MF_TAG_BEFORE_DIP_X_ANR2_YAD1               ,
    MF_TAG_BEFORE_DIP_X_ANR2_Y4LUT1             ,
    MF_TAG_BEFORE_DIP_X_ANR2_Y4LUT2             ,
    MF_TAG_BEFORE_DIP_X_ANR2_Y4LUT3             ,
    MF_TAG_BEFORE_DIP_X_ANR2_L4LUT1             ,
    MF_TAG_BEFORE_DIP_X_ANR2_L4LUT2             ,
    MF_TAG_BEFORE_DIP_X_ANR2_L4LUT3             ,
    MF_TAG_BEFORE_DIP_X_ANR2_CAD                ,
    MF_TAG_BEFORE_DIP_X_ANR2_PTC                ,
    MF_TAG_BEFORE_DIP_X_ANR2_LCE                ,
    MF_TAG_BEFORE_DIP_X_ANR2_MED1               ,
    MF_TAG_BEFORE_DIP_X_ANR2_MED2               ,
    MF_TAG_BEFORE_DIP_X_ANR2_MED3               ,
    MF_TAG_BEFORE_DIP_X_ANR2_MED4               ,
    MF_TAG_BEFORE_DIP_X_ANR2_MED5               ,
    MF_TAG_BEFORE_DIP_X_ANR2_ACTC               ,
    MF_TAG_BEFORE_DIP_X_ANR2_RSV1               ,
    MF_TAG_BEFORE_DIP_X_ANR2_RSV2               ,

    // SEEE
    MF_TAG_BEFORE_DIP_X_SEEE_CTRL               ,
    MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_BLND_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_BLND_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GN_CTRL            ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_4        ,
    MF_TAG_BEFORE_DIP_X_SEEE_SLNK_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_SLNK_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_4        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_5        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_6        ,
    MF_TAG_BEFORE_DIP_X_SEEE_OUT_EDGE_CTRL      ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_Y_CTRL          ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_1     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_2     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_3     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_SPECL_CTRL      ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_CORE_CTRL_1     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_CORE_CTRL_2     ,
    // Stage BFBLD }}}

    // ------------------------------------------------------------------------
    // Stage AFBLD
    // {{{
    // ANR
    MF_TAG_AFTER_DIP_X_ANR_CON1                 ,
    MF_TAG_AFTER_DIP_X_ANR_CON2                 ,
    MF_TAG_AFTER_DIP_X_ANR_YAD1                ,
    MF_TAG_AFTER_DIP_X_ANR_YAD2                ,
    MF_TAG_AFTER_DIP_X_ANR_Y4LUT1              ,
    MF_TAG_AFTER_DIP_X_ANR_Y4LUT2              ,
    MF_TAG_AFTER_DIP_X_ANR_Y4LUT3              ,
    MF_TAG_AFTER_DIP_X_ANR_C4LUT1              ,
    MF_TAG_AFTER_DIP_X_ANR_C4LUT2              ,
    MF_TAG_AFTER_DIP_X_ANR_C4LUT3              ,
    MF_TAG_AFTER_DIP_X_ANR_A4LUT2              ,
    MF_TAG_AFTER_DIP_X_ANR_A4LUT3              ,
    MF_TAG_AFTER_DIP_X_ANR_L4LUT1              ,
    MF_TAG_AFTER_DIP_X_ANR_L4LUT2              ,
    MF_TAG_AFTER_DIP_X_ANR_L4LUT3              ,
    MF_TAG_AFTER_DIP_X_ANR_PTY                 ,
    MF_TAG_AFTER_DIP_X_ANR_CAD                 ,
    MF_TAG_AFTER_DIP_X_ANR_PTC                 ,
    MF_TAG_AFTER_DIP_X_ANR_LCE                 ,
    MF_TAG_AFTER_DIP_X_ANR_T4LUT1              ,
    MF_TAG_AFTER_DIP_X_ANR_T4LUT2              ,
    MF_TAG_AFTER_DIP_X_ANR_T4LUT3              ,
    MF_TAG_AFTER_DIP_X_ANR_ACT1                ,
    MF_TAG_AFTER_DIP_X_ANR_ACT2                ,
    MF_TAG_AFTER_DIP_X_ANR_ACT4                ,
    MF_TAG_AFTER_DIP_X_ANR_ACTYHL              ,
    MF_TAG_AFTER_DIP_X_ANR_ACTYHH              ,
    MF_TAG_AFTER_DIP_X_ANR_ACTYL               ,
    MF_TAG_AFTER_DIP_X_ANR_ACTYHL2             ,
    MF_TAG_AFTER_DIP_X_ANR_ACTYHH2             ,
    MF_TAG_AFTER_DIP_X_ANR_ACTYL2              ,
    MF_TAG_AFTER_DIP_X_ANR_ACTC                ,
    MF_TAG_AFTER_DIP_X_ANR_YLAD                ,
    MF_TAG_AFTER_DIP_X_ANR_YLAD2               ,
    MF_TAG_AFTER_DIP_X_ANR_YLAD3               ,
    MF_TAG_AFTER_DIP_X_ANR_PTYL                ,
    MF_TAG_AFTER_DIP_X_ANR_LCOEF               ,
    MF_TAG_AFTER_DIP_X_ANR_YDIR                ,
    MF_TAG_AFTER_DIP_X_ANR_RSV1                ,

    // ANR2
    MF_TAG_AFTER_DIP_X_ANR2_CON1               ,
    MF_TAG_AFTER_DIP_X_ANR2_CON2               ,
    MF_TAG_AFTER_DIP_X_ANR2_YAD1               ,
    MF_TAG_AFTER_DIP_X_ANR2_Y4LUT1             ,
    MF_TAG_AFTER_DIP_X_ANR2_Y4LUT2             ,
    MF_TAG_AFTER_DIP_X_ANR2_Y4LUT3             ,
    MF_TAG_AFTER_DIP_X_ANR2_L4LUT1             ,
    MF_TAG_AFTER_DIP_X_ANR2_L4LUT2             ,
    MF_TAG_AFTER_DIP_X_ANR2_L4LUT3             ,
    MF_TAG_AFTER_DIP_X_ANR2_CAD                ,
    MF_TAG_AFTER_DIP_X_ANR2_PTC                ,
    MF_TAG_AFTER_DIP_X_ANR2_LCE                ,
    MF_TAG_AFTER_DIP_X_ANR2_MED1               ,
    MF_TAG_AFTER_DIP_X_ANR2_MED2               ,
    MF_TAG_AFTER_DIP_X_ANR2_MED3               ,
    MF_TAG_AFTER_DIP_X_ANR2_MED4               ,
    MF_TAG_AFTER_DIP_X_ANR2_MED5               ,
    MF_TAG_AFTER_DIP_X_ANR2_ACTC               ,
    MF_TAG_AFTER_DIP_X_ANR2_RSV1               ,
    MF_TAG_AFTER_DIP_X_ANR2_RSV2               ,

    // SEEE
    MF_TAG_AFTER_DIP_X_SEEE_CTRL               ,
    MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL_1        ,
    MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL_2        ,
    MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL_3        ,
    MF_TAG_AFTER_DIP_X_SEEE_BLND_CTRL_1        ,
    MF_TAG_AFTER_DIP_X_SEEE_BLND_CTRL_2        ,
    MF_TAG_AFTER_DIP_X_SEEE_GN_CTRL            ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_1        ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_2        ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_3        ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_4        ,
    MF_TAG_AFTER_DIP_X_SEEE_SLNK_CTRL_1        ,
    MF_TAG_AFTER_DIP_X_SEEE_SLNK_CTRL_2        ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_1        ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_2        ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_3        ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_4        ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_5        ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_6        ,
    MF_TAG_AFTER_DIP_X_SEEE_OUT_EDGE_CTRL      ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_Y_CTRL          ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_1     ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_2     ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_3     ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_SPECL_CTRL      ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_CORE_CTRL_1     ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_CORE_CTRL_2     ,

    // MIX3
    MF_TAG_AFTER_DIP_X_MIX3_CTRL_0             ,
    MF_TAG_AFTER_DIP_X_MIX3_CTRL_1             ,
    MF_TAG_AFTER_DIP_X_MIX3_SPARE              ,

    // HFG
    MF_TAG_AFTER_DIP_X_HFG_CON_0               ,
    MF_TAG_AFTER_DIP_X_HFG_LUMA_0              ,
    MF_TAG_AFTER_DIP_X_HFG_LUMA_1              ,
    MF_TAG_AFTER_DIP_X_HFG_LUMA_2              ,
    MF_TAG_AFTER_DIP_X_HFG_LCE_0               ,
    MF_TAG_AFTER_DIP_X_HFG_LCE_1               ,
    MF_TAG_AFTER_DIP_X_HFG_LCE_2               ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_0               ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_1               ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_2               ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_3               ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_4               ,
    MF_TAG_AFTER_DIP_X_HFG_CROP_X              ,
    MF_TAG_AFTER_DIP_X_HFG_CROP_Y              ,
    // Stage AFBLD }}}

    // ------------------------------------------------------------------------
    // Stage MFB
    // {{{
    /* blending */
    MF_TAG_BLEND_DIP_X_MFB_CON,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON1,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON2,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON3,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON4,
    MF_TAG_BLEND_DIP_X_MFB_EDGE,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON5,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON6,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON7,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON8,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON9,
    MF_TAG_BLEND_DIP_X_MFB_LL_CON10,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON0,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON1,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON2,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON3,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON4,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON5,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON6,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON7,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON8,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON9,
    MF_TAG_BLEND_DIP_X_MFB_MBD_CON10,
    // }}}

    // ------------------------------------------------------------------------
    // Extension: HDR related tags
    // {{{
    /* image */
    MF_TAG_IMAGE_HDR,
    // }}}

    /* indicates to size */
    MF_DEBUG_TAG_SIZE,
} DEBUG_MF_TAG_T;

typedef struct DEBUG_MF_INFO_S
{
    debug_exif_field Tag[MF_DEBUG_TAG_SIZE];
} DEBUG_MF_INFO_T;


};  //namespace
#endif//_MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM8_H_
