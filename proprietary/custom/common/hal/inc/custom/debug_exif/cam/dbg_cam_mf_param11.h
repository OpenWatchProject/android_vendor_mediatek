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
#ifndef _MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM11_H_
#define _MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM11_H_
#pragma once

#include "../dbg_exif_def.h"

//
// Debug Exif Version 11 - MT6779
//
namespace dbg_cam_mf_param_11 {

 // MF debug info
enum { MF_DEBUG_TAG_VERSION = 11 };
enum { MF_DEBUG_TAG_SUBVERSION = 2 };
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
    // new ME parameters
    MF_TAG_MEMC_BAD_MV_RATE_TH                                                ,
    MF_TAG_CONF_NOISE_LV                                                      ,
    MF_TAG_DSDN_DIVIDEND                                                      ,
    MF_TAG_DSDN_DIVISOR                                                       ,
    MF_TAG_MEMC_DSUS_MODE                                                     ,
    MF_TAG_AEVC_LCSO_EN                                                       ,
    MF_TAG_POST_RAFINE_EN                                                     ,
    MF_TAG_POST_ME_REFINE_EN                                                  ,
    MF_TAG_POST_ME_REFINE_MV_RATIO                                            ,
    MF_TAG_POST_ME_REFINE_FACE_RATIO_THL                                      ,
    MF_TAG_POST_ME_REFINE_FACE_RATIO_THH                                      ,
    MF_TAG_POST_ME_REFINE_FULL_RATIO_THL                                      ,
    MF_TAG_POST_ME_REFINE_FULL_RATIO_THH                                      ,
    MF_TAG_POST_ME_REFINE_EDGE_CCL_TH0                                        ,
    MF_TAG_POST_ME_REFINE_EDGE_CCL_TH1                                        ,
    MF_TAG_POST_ME_REFINE_EDGE_CCLNUM_TH                                      ,
    MF_TAG_POST_ME_REFINE_EDGE_FDAREATHL                                      ,
    MF_TAG_POST_ME_REFINE_EDGE_FDAREATHH                                      ,
    MF_TAG_LCL_DECONF_EN                                                      ,
    MF_TAG_LCL_DECONF_NOISE_LV                                                ,
    MF_TAG_LCL_DECONF_BG_BSS_RATIO                                            ,
    MF_TAG_LCL_DECONF_FD_BSS_RATIO                                            ,
    MF_TAG_LCL_DECONF_DLTVER_EN                                               ,
    MF_TAG_EXT_SETTING                                                        ,
    MF_TAG_BLINK_EYE_EN                                                       ,
    MF_TAG_BLINK_EYE_THL                                                      ,
    MF_TAG_BLINK_EYE_THH                                                      ,
    MF_TAG_BLINK_EYE_QSTEP                                                    ,
    MF_TAG_POST_REFINE_INT                                                    ,
    MF_TAG_ME_LARGE_MV_THD                                                    ,
    MF_TAG_ME_LARGE_MV_SAD_THD                                                ,
    MF_TAG_ME_LARGE_MV_RATIO                                                  ,
    MF_TAG_ME_LARGE_MV_TXTR_EN                                                ,
    MF_TAG_ME_LARGE_MV_TXTR_WEI                                               ,
    MF_TAG_ME_LARGE_MV_TXTR_THD                                               ,
    // ------------------------------------------------------------------------
    // new BSS parameters
    MF_TAG_BSS_FACECVTH                                                       ,
    MF_TAG_BSS_GRADTHL                                                        ,
    MF_TAG_BSS_GRADTHH                                                        ,
    MF_TAG_BSS_FACEAREATHL0                                                   ,
    MF_TAG_BSS_FACEAREATHL1                                                   ,
    MF_TAG_BSS_FACEAREATHH0                                                   ,
    MF_TAG_BSS_FACEAREATHH1                                                   ,
    MF_TAG_BSS_APLDELTATH0                                                    ,
    MF_TAG_BSS_APLDELTATH1                                                    ,
    MF_TAG_BSS_APLDELTATH2                                                    ,
    MF_TAG_BSS_APLDELTATH3                                                    ,
    MF_TAG_BSS_APLDELTATH4                                                    ,
    MF_TAG_BSS_APLDELTATH5                                                    ,
    MF_TAG_BSS_APLDELTATH6                                                    ,
    MF_TAG_BSS_APLDELTATH7                                                    ,
    MF_TAG_BSS_APLDELTATH8                                                    ,
    MF_TAG_BSS_APLDELTATH9                                                    ,
    MF_TAG_BSS_APLDELTATH10                                                   ,
    MF_TAG_BSS_APLDELTATH11                                                   ,
    MF_TAG_BSS_APLDELTATH12                                                   ,
    MF_TAG_BSS_APLDELTATH13                                                   ,
    MF_TAG_BSS_APLDELTATH14                                                   ,
    MF_TAG_BSS_APLDELTATH15                                                   ,
    MF_TAG_BSS_APLDELTATH16                                                   ,
    MF_TAG_BSS_APLDELTATH17                                                   ,
    MF_TAG_BSS_APLDELTATH18                                                   ,
    MF_TAG_BSS_APLDELTATH19                                                   ,
    MF_TAG_BSS_APLDELTATH20                                                   ,
    MF_TAG_BSS_APLDELTATH21                                                   ,
    MF_TAG_BSS_APLDELTATH22                                                   ,
    MF_TAG_BSS_APLDELTATH23                                                   ,
    MF_TAG_BSS_APLDELTATH24                                                   ,
    MF_TAG_BSS_APLDELTATH25                                                   ,
    MF_TAG_BSS_APLDELTATH26                                                   ,
    MF_TAG_BSS_APLDELTATH27                                                   ,
    MF_TAG_BSS_APLDELTATH28                                                   ,
    MF_TAG_BSS_APLDELTATH29                                                   ,
    MF_TAG_BSS_APLDELTATH30                                                   ,
    MF_TAG_BSS_APLDELTATH31                                                   ,
    MF_TAG_BSS_APLDELTATH32                                                   ,
    MF_TAG_BSS_GRADRATIOTH0                                                   ,
    MF_TAG_BSS_GRADRATIOTH1                                                   ,
    MF_TAG_BSS_GRADRATIOTH2                                                   ,
    MF_TAG_BSS_GRADRATIOTH3                                                   ,
    MF_TAG_BSS_GRADRATIOTH4                                                   ,
    MF_TAG_BSS_GRADRATIOTH5                                                   ,
    MF_TAG_BSS_GRADRATIOTH6                                                   ,
    MF_TAG_BSS_GRADRATIOTH7                                                   ,
    MF_TAG_BSS_EYEDISTTHL                                                     ,
    MF_TAG_BSS_EYEDISTTHH                                                     ,
    MF_TAG_BSS_EYEMINWEIGHT                                                   ,
    MF_TAG_BSS_EXT_SETTING                                                    ,
    // ------------------------------------------------------------------------
    MF_TAG_DBG_ME_FRMLEVEL0                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 1st ME
    MF_TAG_DBG_ME_FRMLEVEL1                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 2nd ME
    MF_TAG_DBG_ME_FRMLEVEL2                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 3rd ME
    MF_TAG_DBG_ME_FRMLEVEL3                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 4th ME
    MF_TAG_DBG_ME_FRMLEVEL4                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 5th ME
    MF_TAG_DBG_ME_FRMLEVEL5                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 6th ME
    MF_TAG_DBG_ME_FRMLEVEL6                                                   , //MFBLL_PROC1_OUT::u4FrmLevel of 7th ME
    // ------------------------------------------------------------------------
    // Extension: HDR related tags
    // {{{
    MF_TAG_IMAGE_HDR,
    // }}}
    MF_TAG_AINR_EN,

    // indicates to size
    MF_DEBUG_TAG_SIZE,
    //END_OF_EXIF_TAG
} DEBUG_MF_TAG_T;

typedef struct DEBUG_MF_INFO_S
{
    debug_exif_field Tag[MF_DEBUG_TAG_SIZE];
} DEBUG_MF_INFO_T;


};  //namespace
#endif//_MTK_CUSTOM_DEBUG_EXIF_CAM_DBG_CAM_MF_PARAM11_H_
