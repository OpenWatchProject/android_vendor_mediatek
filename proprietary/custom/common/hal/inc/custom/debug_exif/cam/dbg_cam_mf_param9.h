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
#ifndef _MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM9_H_
#define _MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM9_H_
#pragma once

#include "../dbg_exif_def.h"

//
// Debug Exif Version 9 - MT6771
//
namespace dbg_cam_mf_param_9 {

 // MF debug info
enum { MF_DEBUG_TAG_VERSION = 9 };
enum { MF_DEBUG_TAG_SUBVERSION = 5 };
#define MF_DEBUG_TAG_VERSION_DP ((MF_DEBUG_TAG_SUBVERSION << 16) | MF_DEBUG_TAG_VERSION)

//MF Parameter Structure
typedef enum
{
    // ------------------------------------------------------------------------
    // MFNR SW related info
    // {{{
    //BEGIN_OF_EXIF_TAG
    MF_TAG_VERSION = 0                                                        , // MF tag version. Bits[0:15]: major ver, Bits[16:31]: sub version
    MF_TAG_MFB_MODE                                                           , // 0: disable, 1: MFLL 2: AIS
    MF_TAG_CAPTURE_M                                                          , // max capture frame number
    MF_TAG_BLENDED_N                                                          , // max blended frame number
    MF_TAG_MAX_FRAME_NUMBER                                                   , // capture number
    MF_TAG_PROCESSING_NUMBER                                                  , // blended number
    MF_TAG_EXPOSURE                                                           ,
    MF_TAG_ISO                                                                ,
    MF_TAG_MFNR_ISO_TH                                                        , // threshold to trigger MFNR
    MF_TAG_RAW_WIDTH                                                          ,
    MF_TAG_RAW_HEIGHT                                                         ,
    MF_TAG_BLD_YUV_WIDTH                                                      , // blend yuv image size
    MF_TAG_BLD_YUV_HEIGHT                                                     ,
    MF_TAG_FD_MODE                                                            , // bit0: Is re-capture, bit1: Is same FD for each frames

    // BSS
    MF_TAG_BSS_ENABLE                                                         , // Indicates that firmware enable or disable BSS
    MF_TAG_BSS_ON                                                             , // Indicates that BSS is enabled or not, since BSS v1.2
    MF_TAG_BSS_VER                                                            , // Since BSS v1.2
    MF_TAG_BSS_FRAME_NUM                                                      , // Indicates BSS aglorithm version info, since BSS v1.2
    MF_TAG_BSS_ROI_WIDTH                                                      ,
    MF_TAG_BSS_ROI_HEIGHT                                                     ,
    MF_TAG_BSS_ROI_X0                                                         ,
    MF_TAG_BSS_ROI_Y0                                                         ,
    MF_TAG_BSS_SCALE_FACTOR                                                   ,
    MF_TAG_BSS_CLIP_TH0                                                       ,
    MF_TAG_BSS_CLIP_TH1                                                       ,
    MF_TAG_BSS_CLIP_TH2                                                       ,
    MF_TAG_BSS_CLIP_TH3                                                       ,
    MF_TAG_BSS_ZERO                                                           ,
    MF_TAG_BSS_BEST_IDX                                                       , // Selected frame
    MF_TAG_BSS_ORDER_IDX                                                      , // BSS order for top 8 frames (MSB->LSC), e,g,: (idx 0, Bit[28:31]), (idx 1, Bit[24:27], (idx 2, Bit[23:20]) ... etc
    MF_TAG_BSS_ADF_TH                                                         ,
    MF_TAG_BSS_SDF_TH                                                         ,
    MF_TAG_BSS_GAIN_TH0                                                       ,
    MF_TAG_BSS_GAIN_TH1                                                       ,
    MF_TAG_BSS_MIN_ISP_GAIN                                                   ,
    MF_TAG_BSS_LCSO_SIZE                                                      ,

    // since BSS v1.2
    MF_TAG_BSS_YPF_EN                                                         ,
    MF_TAG_BSS_YPF_FAC                                                        ,
    MF_TAG_BSS_YPF_ADJTH                                                      ,
    MF_TAG_BSS_YPF_DFMED0                                                     ,
    MF_TAG_BSS_YPF_DFMED1                                                     ,
    MF_TAG_BSS_YPF_TH0                                                        ,
    MF_TAG_BSS_YPF_TH1                                                        ,
    MF_TAG_BSS_YPF_TH2                                                        ,
    MF_TAG_BSS_YPF_TH3                                                        ,
    MF_TAG_BSS_YPF_TH4                                                        ,
    MF_TAG_BSS_YPF_TH5                                                        ,
    MF_TAG_BSS_YPF_TH6                                                        ,
    MF_TAG_BSS_YPF_TH7                                                        ,
    MF_TAG_BSS_FD_EN                                                          , // Face info available or not
    MF_TAG_BSS_FD_FAC                                                         ,
    MF_TAG_BSS_FD_FNUM                                                        ,
    MF_TAG_BSS_EYE_EN                                                         , // Eyes info available or not
    MF_TAG_BSS_EYE_CFTH                                                       ,
    MF_TAG_BSS_EYE_RATIO0                                                     ,
    MF_TAG_BSS_EYE_RATIO1                                                     ,
    MF_TAG_BSS_EYE_FAC                                                        ,
    MF_TAG_BSS_AEVC_EN                                                        , // AE Compensation enable or not

    // BSS v1.2 final score, each score is 64 bits
    MF_TAG_BSS_FINAL_SCORE_00_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_01_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_02_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_03_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_04_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_05_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_06_MSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_07_MSB                                             ,
    //
    MF_TAG_BSS_FINAL_SCORE_00_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_01_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_02_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_03_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_04_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_05_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_06_LSB                                             ,
    MF_TAG_BSS_FINAL_SCORE_07_LSB                                             ,

    // BSS v1.2 sharp score, each score is 64 bits
    MF_TAG_BSS_SHARP_SCORE_00_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_01_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_02_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_03_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_04_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_05_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_06_MSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_07_MSB                                             ,
    //
    MF_TAG_BSS_SHARP_SCORE_00_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_01_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_02_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_03_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_04_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_05_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_06_LSB                                             ,
    MF_TAG_BSS_SHARP_SCORE_07_LSB                                             ,

    // BSS Reorder Mapping
    MF_TAG_BSS_REORDER_MAPPING_00                                             , // Frame Mapping: OriginOrder = 
    MF_TAG_BSS_REORDER_MAPPING_01                                             , //                (FrameNumber % 10000) * 100
    MF_TAG_BSS_REORDER_MAPPING_02                                             , //                + NewOrder
    MF_TAG_BSS_REORDER_MAPPING_03                                             , // ex: FrameNumber 10189
    MF_TAG_BSS_REORDER_MAPPING_04                                             , //     OriginOrder 2
    MF_TAG_BSS_REORDER_MAPPING_05                                             , //     NewOrder    1
    MF_TAG_BSS_REORDER_MAPPING_06                                             , //     MF_TAG_BSS_REORDER_MAPPING_02 = 18901
    MF_TAG_BSS_REORDER_MAPPING_07                                             ,

    // GMV
    MF_TAG_GMV_00                                                             , // Global motion vector frame 0 ~ 11
    MF_TAG_GMV_01                                                             ,
    MF_TAG_GMV_02                                                             ,
    MF_TAG_GMV_03                                                             ,
    MF_TAG_GMV_04                                                             ,
    MF_TAG_GMV_05                                                             ,
    MF_TAG_GMV_06                                                             ,
    MF_TAG_GMV_07                                                             ,
    MF_TAG_GMV_08                                                             ,
    MF_TAG_GMV_09                                                             ,
    MF_TAG_GMV_10                                                             ,
    MF_TAG_GMV_11                                                             ,

    // FD
    MF_TAG_FD_RECT0_X0_00                                                     , // Face info frame idx 0,
    MF_TAG_FD_RECT0_Y0_00                                                     ,
    MF_TAG_FD_RECT0_X1_00                                                     ,
    MF_TAG_FD_RECT0_Y1_00                                                     ,
    MF_TAG_FD_RECT0_X0_01                                                     , // Face info frame idx 1,
    MF_TAG_FD_RECT0_Y0_01                                                     ,
    MF_TAG_FD_RECT0_X1_01                                                     ,
    MF_TAG_FD_RECT0_Y1_01                                                     ,
    MF_TAG_FD_RECT0_X0_02                                                     , // Face info frame idx 2,
    MF_TAG_FD_RECT0_Y0_02                                                     ,
    MF_TAG_FD_RECT0_X1_02                                                     ,
    MF_TAG_FD_RECT0_Y1_02                                                     ,
    MF_TAG_FD_RECT0_X0_03                                                     , // Face info frame idx 3,
    MF_TAG_FD_RECT0_Y0_03                                                     ,
    MF_TAG_FD_RECT0_X1_03                                                     ,
    MF_TAG_FD_RECT0_Y1_03                                                     ,
    MF_TAG_FD_RECT0_X0_04                                                     , // Face info frame idx 4,
    MF_TAG_FD_RECT0_Y0_04                                                     ,
    MF_TAG_FD_RECT0_X1_04                                                     ,
    MF_TAG_FD_RECT0_Y1_04                                                     ,
    MF_TAG_FD_RECT0_X0_05                                                     , // Face info frame idx 5,
    MF_TAG_FD_RECT0_Y0_05                                                     ,
    MF_TAG_FD_RECT0_X1_05                                                     ,
    MF_TAG_FD_RECT0_Y1_05                                                     ,
    MF_TAG_FD_RECT0_X0_06                                                     , // Face info frame idx 6,
    MF_TAG_FD_RECT0_Y0_06                                                     ,
    MF_TAG_FD_RECT0_X1_06                                                     ,
    MF_TAG_FD_RECT0_Y1_06                                                     ,
    MF_TAG_FD_RECT0_X0_07                                                     , // Face info frame idx 7,
    MF_TAG_FD_RECT0_Y0_07                                                     ,
    MF_TAG_FD_RECT0_X1_07                                                     ,
    MF_TAG_FD_RECT0_Y1_07                                                     ,

    // MEMC
    MF_TAG_MEMC_VERSION                                                       ,
    MF_TAG_MEMC_WIDTH                                                         ,
    MF_TAG_MEMC_HEIGHT                                                        ,
    MF_TAG_MEMC_INPUT_FORMAT                                                  ,
    MF_TAG_MEMC_OUTPUT_FORMAT                                                 ,
    MF_TAG_MEMC_SKIP                                                          ,
    MF_TAG_MEMC_MCFULLSIZE                                                    , // 1: full size MC, 0: quarter size MC
    MF_TAG_MEMC_BAD_MB_COUNT0                                                 ,
    MF_TAG_MEMC_BAD_MB_COUNT1                                                 ,
    MF_TAG_MEMC_BAD_MB_COUNT2                                                 ,
    MF_TAG_MEMC_BAD_MB_TH                                                     ,
    MF_TAG_MEMC_BAD_MV_RANGE                                                  ,
    MF_TAG_MEMC_NOISE_LVL                                                     ,
    MF_TAG_MEMC_MPME                                                          ,
    MF_TAG_MEMC_CONFMAP                                                       ,
    // }}}

    // ------------------------------------------------------------------------
    // Stage SF
    // {{{
    // ANR
    MF_TAG_SINGLE_DIP_X_NBC_ANR_CON1                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_CON2                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_YAD1                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_YAD2                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT1              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT2              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT3              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_C4LUT1              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_C4LUT2              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_C4LUT3              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_A4LUT2              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_A4LUT3              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_L4LUT1              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_L4LUT2              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_L4LUT3              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY0V               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_CAD                 ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY1V               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_SL2                 ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY2V               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY3V               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY0H               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY1H               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY2H               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_T4LUT1              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_T4LUT2              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_T4LUT3              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_ACT1                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTY3H               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTCV                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_ACT4                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_PTCH                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_YLVL0               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_YLVL1               ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_COR              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_ACT0             ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_ACT1             ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_ACTC                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_YLAD                ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_ACT2             ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_ACT3             ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_LUMA0            ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_HF_LUMA1            ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_GAIN1           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_GAIN2           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTP1           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTP2           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTO1           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTO2           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTS1           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTS2           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTS3           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_LTM_LUTS4           ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT4              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT5              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT6              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_Y4LUT7              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_A4LUT1              ,
    MF_TAG_SINGLE_DIP_X_NBC_ANR_RSV1                ,

    // ANR2
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_CON1               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_CON2               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_YAD1               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_Y4LUT1             ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_Y4LUT2             ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_Y4LUT3             ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_L4LUT1             ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_L4LUT2             ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_L4LUT3             ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_CAD                ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_PTC                ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_SL2                ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_MED1               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_MED2               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_MED3               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_MED4               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_MED5               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_ACTC               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_RSV1               ,
    MF_TAG_SINGLE_DIP_X_NBC2_ANR2_RSV2               ,
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
    MF_TAG_BEFORE_DIP_X_UDM_P1_ACT              ,
    MF_TAG_BEFORE_DIP_X_UDM_LR_RAT              ,
    MF_TAG_BEFORE_DIP_X_UDM_HFTD_CTL2           ,
    MF_TAG_BEFORE_DIP_X_UDM_EST_CTL             ,
    MF_TAG_BEFORE_DIP_X_UDM_SPARE_2             ,
    MF_TAG_BEFORE_DIP_X_UDM_SPARE_3             ,
    MF_TAG_BEFORE_DIP_X_UDM_INT_CTL             ,
    MF_TAG_BEFORE_DIP_X_UDM_EE                  ,

    // ANR
    MF_TAG_BEFORE_DIP_X_NBC_ANR_CON1                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_CON2                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_YAD1                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_YAD2                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT1              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT2              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT3              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_C4LUT1              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_C4LUT2              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_C4LUT3              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_A4LUT2              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_A4LUT3              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_L4LUT1              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_L4LUT2              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_L4LUT3              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY0V               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_CAD                 ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY1V               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_SL2                 ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY2V               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY3V               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY0H               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY1H               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY2H               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_T4LUT1              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_T4LUT2              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_T4LUT3              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_ACT1                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTY3H               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTCV                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_ACT4                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_PTCH                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_YLVL0               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_YLVL1               ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_COR              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_ACT0             ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_ACT1             ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_ACTC                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_YLAD                ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_ACT2             ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_ACT3             ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_LUMA0            ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_HF_LUMA1            ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_GAIN1           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_GAIN2           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTP1           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTP2           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTO1           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTO2           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTS1           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTS2           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTS3           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_LTM_LUTS4           ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT4              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT5              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT6              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_Y4LUT7              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_A4LUT1              ,
    MF_TAG_BEFORE_DIP_X_NBC_ANR_RSV1                ,

    // ANR2
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_CON1               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_CON2               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_YAD1               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_Y4LUT1             ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_Y4LUT2             ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_Y4LUT3             ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_L4LUT1             ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_L4LUT2             ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_L4LUT3             ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_CAD                ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_PTC                ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_SL2                ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_MED1               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_MED2               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_MED3               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_MED4               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_MED5               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_ACTC               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_RSV1               ,
    MF_TAG_BEFORE_DIP_X_NBC2_ANR2_RSV2               ,

    // SEEE
    MF_TAG_BEFORE_DIP_X_SEEE_CTRL               ,
    MF_TAG_BEFORE_DIP_X_SEEE_TOP_CTRL           ,
    MF_TAG_BEFORE_DIP_X_SEEE_BLND_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_BLND_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_CORE_CTRL          ,
    MF_TAG_BEFORE_DIP_X_SEEE_GN_CTRL_1          ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_LUMA_SLNK_CTRL     ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_4        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_5        ,
    MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_6        ,
    MF_TAG_BEFORE_DIP_X_SEEE_ARTIFACT_CTRL      ,
    MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL          ,
    MF_TAG_BEFORE_DIP_X_SEEE_GN_CTRL_2          ,
    MF_TAG_BEFORE_DIP_X_SEEE_ST_CTRL_1          ,
    MF_TAG_BEFORE_DIP_X_SEEE_ST_CTRL_2          ,
    MF_TAG_BEFORE_DIP_X_SEEE_CT_CTRL            ,
    MF_TAG_BEFORE_DIP_X_SEEE_CBOOST_CTRL_1      ,
    MF_TAG_BEFORE_DIP_X_SEEE_CBOOST_CTRL_2      ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC1_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC1_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC1_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC2_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC2_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC2_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC3_CTRL_1        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC3_CTRL_2        ,
    MF_TAG_BEFORE_DIP_X_SEEE_PBC3_CTRL_3        ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_Y_SPECL_CTRL    ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_1     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_2     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_CORE_CTRL_1     ,
    MF_TAG_BEFORE_DIP_X_SEEE_SE_CORE_CTRL_2     ,

    // Stage BFBLD }}}

    // ------------------------------------------------------------------------
    // Stage AFBLD
    // {{{
    // ANR
    MF_TAG_AFTER_DIP_X_NBC_ANR_CON1                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_CON2                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_YAD1                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_YAD2                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT1               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT2               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT3               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_C4LUT1               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_C4LUT2               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_C4LUT3               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_A4LUT2               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_A4LUT3               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_L4LUT1               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_L4LUT2               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_L4LUT3               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY0V                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_CAD                  ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY1V                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_SL2                  ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY2V                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY3V                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY0H                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY1H                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY2H                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_T4LUT1               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_T4LUT2               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_T4LUT3               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_ACT1                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTY3H                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTCV                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_ACT4                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_PTCH                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_YLVL0                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_YLVL1                ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_COR               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_ACT0              ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_ACT1              ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_ACTC                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_YLAD                 ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_ACT2              ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_ACT3              ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_LUMA0             ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_HF_LUMA1             ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_GAIN1            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_GAIN2            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTP1            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTP2            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTO1            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTO2            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTS1            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTS2            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTS3            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_LTM_LUTS4            ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT4               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT5               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT6               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_Y4LUT7               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_A4LUT1               ,
    MF_TAG_AFTER_DIP_X_NBC_ANR_RSV1                 ,

    // ANR2
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_CON1                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_CON2                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_YAD1                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_Y4LUT1              ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_Y4LUT2              ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_Y4LUT3              ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_L4LUT1              ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_L4LUT2              ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_L4LUT3              ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_CAD                 ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_PTC                 ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_SL2                 ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_MED1                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_MED2                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_MED3                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_MED4                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_MED5                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_ACTC                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_RSV1                ,
    MF_TAG_AFTER_DIP_X_NBC2_ANR2_RSV2                ,

    // SEEE
    MF_TAG_AFTER_DIP_X_SEEE_CTRL                ,
    MF_TAG_AFTER_DIP_X_SEEE_TOP_CTRL            ,
    MF_TAG_AFTER_DIP_X_SEEE_BLND_CTRL_1         ,
    MF_TAG_AFTER_DIP_X_SEEE_BLND_CTRL_2         ,
    MF_TAG_AFTER_DIP_X_SEEE_CORE_CTRL           ,
    MF_TAG_AFTER_DIP_X_SEEE_GN_CTRL_1           ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_1         ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_2         ,
    MF_TAG_AFTER_DIP_X_SEEE_LUMA_SLNK_CTRL      ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_1         ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_2         ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_3         ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_4         ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_5         ,
    MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_6         ,
    MF_TAG_AFTER_DIP_X_SEEE_ARTIFACT_CTRL       ,
    MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL           ,
    MF_TAG_AFTER_DIP_X_SEEE_GN_CTRL_2           ,
    MF_TAG_AFTER_DIP_X_SEEE_ST_CTRL_1           ,
    MF_TAG_AFTER_DIP_X_SEEE_ST_CTRL_2           ,
    MF_TAG_AFTER_DIP_X_SEEE_CT_CTRL             ,
    MF_TAG_AFTER_DIP_X_SEEE_CBOOST_CTRL_1       ,
    MF_TAG_AFTER_DIP_X_SEEE_CBOOST_CTRL_2       ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC1_CTRL_1         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC1_CTRL_2         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC1_CTRL_3         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC2_CTRL_1         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC2_CTRL_2         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC2_CTRL_3         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC3_CTRL_1         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC3_CTRL_2         ,
    MF_TAG_AFTER_DIP_X_SEEE_PBC3_CTRL_3         ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_Y_SPECL_CTRL     ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_1      ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_2      ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_CORE_CTRL_1      ,
    MF_TAG_AFTER_DIP_X_SEEE_SE_CORE_CTRL_2      ,

    // MIX3
    MF_TAG_AFTER_DIP_X_MIX3_CTRL_0              ,
    MF_TAG_AFTER_DIP_X_MIX3_CTRL_1              ,
    MF_TAG_AFTER_DIP_X_MIX3_SPARE               ,

    // HFG
    MF_TAG_AFTER_DIP_X_HFG_CON_0                ,
    MF_TAG_AFTER_DIP_X_HFG_LUMA_0               ,
    MF_TAG_AFTER_DIP_X_HFG_LUMA_1               ,
    MF_TAG_AFTER_DIP_X_HFG_LUMA_2               ,
    MF_TAG_AFTER_DIP_X_HFG_LCE_0                ,
    MF_TAG_AFTER_DIP_X_HFG_LCE_1                ,
    MF_TAG_AFTER_DIP_X_HFG_LCE_2                ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_0                ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_1                ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_2                ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_3                ,
    MF_TAG_AFTER_DIP_X_HFG_RAN_4                ,
    MF_TAG_AFTER_DIP_X_HFG_CROP_X               ,
    MF_TAG_AFTER_DIP_X_HFG_CROP_Y               ,

    // NDG
    MF_TAG_AFTER_DIP_X_NDG_RAN_0              ,
    MF_TAG_AFTER_DIP_X_NDG_RAN_1              ,
    MF_TAG_AFTER_DIP_X_NDG_RAN_2              ,
    MF_TAG_AFTER_DIP_X_NDG_RAN_3              ,
    MF_TAG_AFTER_DIP_X_NDG_CROP_X             ,
    MF_TAG_AFTER_DIP_X_NDG_CROP_Y             ,

    // NDG2
    MF_TAG_AFTER_DIP_X_NDG2_RAN_0              ,
    MF_TAG_AFTER_DIP_X_NDG2_RAN_1              ,
    MF_TAG_AFTER_DIP_X_NDG2_RAN_2              ,
    MF_TAG_AFTER_DIP_X_NDG2_RAN_3              ,
    MF_TAG_AFTER_DIP_X_NDG2_CROP_X             ,
    MF_TAG_AFTER_DIP_X_NDG2_CROP_Y             ,

    // ABF
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CON1           ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CON2           ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_RCON           ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_YLUT           ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CXLUT          ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CYLUT          ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_YSP            ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CXSP           ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CYSP           ,
    MF_TAG_AFTER_DIP_X_NBC2_ABF_CLP            ,

    // CCR
    MF_TAG_AFTER_DIP_X_NBC2_CCR_CON            ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_YLUT           ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_UVLUT          ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_YLUT2          ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_SAT_CTRL       ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_UVLUT_SP       ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_HUE1           ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_HUE2           ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_HUE3           ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_L4LUT1         ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_L4LUT2         ,
    MF_TAG_AFTER_DIP_X_NBC2_CCR_L4LUT3         ,
    // Stage AFBLD }}}

    // ------------------------------------------------------------------------
    // Stage MFB
    // {{{
    MF_TAG_BLEND_MFB_CON                  ,
    MF_TAG_BLEND_MFB_LL_CON1              ,
    MF_TAG_BLEND_MFB_LL_CON2              ,
    MF_TAG_BLEND_MFB_LL_CON3              ,
    MF_TAG_BLEND_MFB_LL_CON4              ,
    MF_TAG_BLEND_MFB_EDGE                 ,
    MF_TAG_BLEND_MFB_LL_CON5              ,
    MF_TAG_BLEND_MFB_LL_CON6              ,
    MF_TAG_BLEND_MFB_LL_CON7              ,
    MF_TAG_BLEND_MFB_LL_CON8              ,
    MF_TAG_BLEND_MFB_LL_CON9              ,
    MF_TAG_BLEND_MFB_LL_CON10             ,
    MF_TAG_BLEND_MFB_MBD_CON0             ,
    MF_TAG_BLEND_MFB_MBD_CON1             ,
    MF_TAG_BLEND_MFB_MBD_CON2             ,
    MF_TAG_BLEND_MFB_MBD_CON3             ,
    MF_TAG_BLEND_MFB_MBD_CON4             ,
    MF_TAG_BLEND_MFB_MBD_CON5             ,
    MF_TAG_BLEND_MFB_MBD_CON6             ,
    MF_TAG_BLEND_MFB_MBD_CON7             ,
    MF_TAG_BLEND_MFB_MBD_CON8             ,
    MF_TAG_BLEND_MFB_MBD_CON9             ,
    MF_TAG_BLEND_MFB_MBD_CON10            ,
    // }}}

    // ------------------------------------------------------------------------
    // Extension: HDR related tags
    // {{{
    MF_TAG_IMAGE_HDR,
    // }}}

    // indicates to size
    MF_DEBUG_TAG_SIZE,
    //END_OF_EXIF_TAG
} DEBUG_MF_TAG_T;

typedef struct DEBUG_MF_INFO_S
{
    debug_exif_field Tag[MF_DEBUG_TAG_SIZE];
} DEBUG_MF_INFO_T;


};  //namespace
#endif//_MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM9_H_
