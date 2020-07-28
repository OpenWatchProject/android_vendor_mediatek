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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _DBG_ISP_PARAM_H_
#define _DBG_ISP_PARAM_H_
/*******************************************************************************
*
*******************************************************************************/
#include "../../ver1/dbg_id_param.h"

namespace NSIspExifDebug
{

enum { IspDebugTagVersion = 7};

enum IspDebugTagID
{
    IspTagVersion,
    UniqueKey,
    //  RAWIspCamInfo
    IspProfile_P1,
    IspProfile_P2,
    SensorMode,
    SceneIdx,
    ISOValue,
    ISOIdx,
    SwnrEncEnableIsoThreshold,
    ShadingIdx,
    ZoomRatio_x100,
    LightValue_x10,
    // Effect
    EffectMode,
    // UserSelectLevel
    EdgeIdx,
    HueIdx,
    SatIdx,
    BrightIdx,
    ContrastIdx,
    //  Index
    IDX_SL2F,
    IDX_DBS,
    IDX_OBC,
    IDX_BPC,
    IDX_NR1,
    IDX_PDC,
    IDX_RMM,
    IDX_SL2G,
    IDX_DBS2,
    IDX_OBC2,
    IDX_BPC2,
    IDX_NR12,
    IDX_PDC2,
    IDX_RMM2,
    IDX_RNR,
    IDX_SL2,
    IDX_UDM,
    //IDX_CCM_CFC,
    IDX_GGM,
    IDX_ANR,
    IDX_ANR2,
    IDX_CCR,
    //IDX_BOK,
    IDX_PCA,
    IDX_HFG,
    IDX_EE,
    //IDX_NR3D,
    IDX_MFB,
    IDX_MIX3,
    // PCA slider
    PCA_SLIDER,
    // CCM Weight
    SMOOTH_CCM,
    CCM_Weight_Strobe,
    CCM_Weight_A,
    CCM_Weight_TL84,
    CCM_Weight_CWF,
    CCM_Weight_D65,
    CCM_Weight_RSV1,
    CCM_Weight_RSV2,
    CCM_Weight_RSV3,
    // ISP interpolation
    UPPER_ISO_IDX,
    LOWER_ISO_IDX,
    DBS_UPPER_IDX,
    DBS_LOWER_IDX,
    OBC_UPPER_IDX,
    OBC_LOWER_IDX,
    BPC_UPPER_IDX,
    BPC_LOWER_IDX,
    NR1_UPPER_IDX,
    NR1_LOWER_IDX,
    PDC_UPPER_IDX,
    PDC_LOWER_IDX,
    RMM_UPPER_IDX,
    RMM_LOWER_IDX,
    DBS2_UPPER_IDX,
    DBS2_LOWER_IDX,
    OBC2_UPPER_IDX,
    OBC2_LOWER_IDX,
    BPC2_UPPER_IDX,
    BPC2_LOWER_IDX,
    NR12_UPPER_IDX,
    NR12_LOWER_IDX,
    PDC2_UPPER_IDX,
    PDC2_LOWER_IDX,
    RMM2_UPPER_IDX,
    RMM2_LOWER_IDX,
    RNR_UPPER_IDX,
    RNR_LOWER_IDX,
    UDM_UPPER_IDX,
    UDM_LOWER_IDX,
    ANR_UPPER_IDX,
    ANR_LOWER_IDX,
    ANR2_UPPER_IDX,
    ANR2_LOWER_IDX,
    CCR_UPPER_IDX,
    CCR_LOWER_IDX,
    HFG_UPPER_IDX,
    HFG_LOWER_IDX,
    EE_UPPER_IDX,
    EE_LOWER_IDX,
    //NR3D_UPPER_IDX,
    //NR3D_LOWER_IDX,
    //
    // AEInfo Exposure
    AE_INFO_EXP_TIME,
    AE_INFO_AFE_GAIN,
    AE_INFO_ISP_GAIN,
    AE_INFO_REAL_ISO,
    //
    // ISP enable (TOP)
    CAM_CTL_EN,
    CAM_CTL_DMA_EN,
    DIP_X_CTL_RGB_EN,
    DIP_X_CTL_YUV_EN,
    DIP_X_CTL_YUV2_EN,
    //
    // SL2F
    CAM_SL2F_CEN,
    CAM_SL2F_RR_CON0,
    CAM_SL2F_RR_CON1,
    CAM_SL2F_GAIN,
    CAM_SL2F_RZ,
    CAM_SL2F_XOFF,
    CAM_SL2F_YOFF,
    CAM_SL2F_SLP_CON0,
    CAM_SL2F_SLP_CON1,
    CAM_SL2F_SIZE,
    //
    //  DBS
    CAM_DBS_SIGMA,
    CAM_DBS_BSTBL_0,
    CAM_DBS_BSTBL_1,
    CAM_DBS_BSTBL_2,
    CAM_DBS_BSTBL_3,
    CAM_DBS_CTL,
    CAM_DBS_SIGMA_2,
    CAM_DBS_YGN,
    CAM_DBS_SL_Y12,
    CAM_DBS_SL_Y34,
    CAM_DBS_SL_G12,
    CAM_DBS_SL_G34,
    //
    //  OBC
    CAM_OBC_OFFST0,
    CAM_OBC_OFFST1,
    CAM_OBC_OFFST2,
    CAM_OBC_OFFST3,
    CAM_OBC_GAIN0,
    CAM_OBC_GAIN1,
    CAM_OBC_GAIN2,
    CAM_OBC_GAIN3,
    //
    //  BPC
    CAM_BNR_BPC_CON,
    CAM_BNR_BPC_TH1,
    CAM_BNR_BPC_TH2,
    CAM_BNR_BPC_TH3,
    CAM_BNR_BPC_TH4,
    CAM_BNR_BPC_DTC,
    CAM_BNR_BPC_COR,
    CAM_BNR_BPC_TBLI1,
    CAM_BNR_BPC_TBLI2,
    CAM_BNR_BPC_TH1_C,
    CAM_BNR_BPC_TH2_C,
    CAM_BNR_BPC_TH3_C,
    //
    //  NR1
    CAM_BNR_NR1_CON,
    CAM_BNR_NR1_CT_CON,
    //
    //  PDC
    CAM_BNR_PDC_CON,
    CAM_BNR_PDC_GAIN_L0,
    CAM_BNR_PDC_GAIN_L1,
    CAM_BNR_PDC_GAIN_L2,
    CAM_BNR_PDC_GAIN_L3,
    CAM_BNR_PDC_GAIN_L4,
    CAM_BNR_PDC_GAIN_R0,
    CAM_BNR_PDC_GAIN_R1,
    CAM_BNR_PDC_GAIN_R2,
    CAM_BNR_PDC_GAIN_R3,
    CAM_BNR_PDC_GAIN_R4,
    CAM_BNR_PDC_TH_GB,
    CAM_BNR_PDC_TH_IA,
    CAM_BNR_PDC_TH_HD,
    CAM_BNR_PDC_SL,
    CAM_BNR_PDC_POS,
    //
    //RMM
    CAM_RMM_OSC,
    CAM_RMM_MC,
    CAM_RMM_REVG_1,
    CAM_RMM_REVG_2,
    CAM_RMM_LEOS,
    CAM_RMM_MC2,
    CAM_RMM_DIFF_LB,
    CAM_RMM_MA,
    CAM_RMM_TUNE,
    CAM_RMM_IDX,
    //
    //LSC
    CAM_LSC_CTL1,
    CAM_LSC_CTL2,
    CAM_LSC_CTL3,
    CAM_LSC_LBLOCK,
    CAM_LSC_RATIO_0,
    CAM_LSC_RATIO_1,
    //
    //RMG
    CAM_RMG_HDR_CFG,
    CAM_RMG_HDR_GAIN,
    CAM_RMG_HDR_CFG2,
    //
    //  RPG
    CAM_RPG_SATU_1,
    CAM_RPG_SATU_2,
    CAM_RPG_GAIN_1,
    CAM_RPG_GAIN_2,
    CAM_RPG_OFST_1,
    CAM_RPG_OFST_2,
    //
    //  LCS
    CAM_LCS25_FLR,
    CAM_LCS25_SATU_1,
    CAM_LCS25_SATU_2,
    CAM_LCS25_GAIN_1,
    CAM_LCS25_GAIN_2,
    CAM_LCS25_OFST_1,
    CAM_LCS25_OFST_2,
    CAM_LCS25_G2G_CNV_1,
    CAM_LCS25_G2G_CNV_2,
    CAM_LCS25_G2G_CNV_3,
    CAM_LCS25_G2G_CNV_4,
    CAM_LCS25_G2G_CNV_5,
    //
    // SL2G
    DIP_X_SL2G_CEN,
    DIP_X_SL2G_RR_CON0,
    DIP_X_SL2G_RR_CON1,
    DIP_X_SL2G_GAIN,
    DIP_X_SL2G_RZ,
    DIP_X_SL2G_XOFF,
    DIP_X_SL2G_YOFF,
    DIP_X_SL2G_SLP_CON0,
    DIP_X_SL2G_SLP_CON1,
    DIP_X_SL2G_SIZE,
    //
    //  DBS2
    DIP_X_DBS2_SIGMA,
    DIP_X_DBS2_BSTBL_0,
    DIP_X_DBS2_BSTBL_1,
    DIP_X_DBS2_BSTBL_2,
    DIP_X_DBS2_BSTBL_3,
    DIP_X_DBS2_CTL,
    DIP_X_DBS2_SIGMA_2,
    DIP_X_DBS2_YGN,
    DIP_X_DBS2_SL_Y12,
    DIP_X_DBS2_SL_Y34,
    DIP_X_DBS2_SL_G12,
    DIP_X_DBS2_SL_G34,
    //
    //  OBC2
    DIP_X_OBC2_OFFST0,
    DIP_X_OBC2_OFFST1,
    DIP_X_OBC2_OFFST2,
    DIP_X_OBC2_OFFST3,
    DIP_X_OBC2_GAIN0,
    DIP_X_OBC2_GAIN1,
    DIP_X_OBC2_GAIN2,
    DIP_X_OBC2_GAIN3,
    //
    //  BPC2
    DIP_X_BNR2_BPC_CON,
    DIP_X_BNR2_BPC_TH1,
    DIP_X_BNR2_BPC_TH2,
    DIP_X_BNR2_BPC_TH3,
    DIP_X_BNR2_BPC_TH4,
    DIP_X_BNR2_BPC_DTC,
    DIP_X_BNR2_BPC_COR,
    DIP_X_BNR2_BPC_TBLI1,
    DIP_X_BNR2_BPC_TBLI2,
    DIP_X_BNR2_BPC_TH1_C,
    DIP_X_BNR2_BPC_TH2_C,
    DIP_X_BNR2_BPC_TH3_C,
    //
    //  NR12
    DIP_X_BNR2_NR1_CON,
    DIP_X_BNR2_NR1_CT_CON,
    //
    //  PDC2
    DIP_X_BNR2_PDC_CON,
    DIP_X_BNR2_PDC_GAIN_L0,
    DIP_X_BNR2_PDC_GAIN_L1,
    DIP_X_BNR2_PDC_GAIN_L2,
    DIP_X_BNR2_PDC_GAIN_L3,
    DIP_X_BNR2_PDC_GAIN_L4,
    DIP_X_BNR2_PDC_GAIN_R0,
    DIP_X_BNR2_PDC_GAIN_R1,
    DIP_X_BNR2_PDC_GAIN_R2,
    DIP_X_BNR2_PDC_GAIN_R3,
    DIP_X_BNR2_PDC_GAIN_R4,
    DIP_X_BNR2_PDC_TH_GB,
    DIP_X_BNR2_PDC_TH_IA,
    DIP_X_BNR2_PDC_TH_HD,
    DIP_X_BNR2_PDC_SL,
    DIP_X_BNR2_PDC_POS,
    //
    //   RMM2
    DIP_X_RMM2_OSC,
    DIP_X_RMM2_MC,
    DIP_X_RMM2_REVG_1,
    DIP_X_RMM2_REVG_2,
    DIP_X_RMM2_LEOS,
    DIP_X_RMM2_MC2,
    DIP_X_RMM2_DIFF_LB,
    DIP_X_RMM2_MA,
    DIP_X_RMM2_TUNE,
    DIP_X_RMM2_IDX,
    //
    //RMG2
    DIP_X_RMG2_HDR_CFG,
    DIP_X_RMG2_HDR_GAIN,
    DIP_X_RMG2_HDR_CFG2,
    //
    //LSC2
    DIP_X_LSC2_CTL1,
    DIP_X_LSC2_CTL2,
    DIP_X_LSC2_CTL3,
    DIP_X_LSC2_LBLOCK,
    DIP_X_LSC2_RATIO_0,
    DIP_X_LSC2_RATIO_1,
    //
    //  PGN
    DIP_X_PGN_SATU_1,
    DIP_X_PGN_SATU_2,
    DIP_X_PGN_GAIN_1,
    DIP_X_PGN_GAIN_2,
    DIP_X_PGN_OFST_1,
    DIP_X_PGN_OFST_2,
    //
    // SL2
    DIP_X_SL2_CEN,
    DIP_X_SL2_RR_CON0,
    DIP_X_SL2_RR_CON1,
    DIP_X_SL2_GAIN,
    DIP_X_SL2_RZ,
    DIP_X_SL2_XOFF,
    DIP_X_SL2_YOFF,
    DIP_X_SL2_SLP_CON0,
    DIP_X_SL2_SLP_CON1,
    DIP_X_SL2_SIZE,
    // SL2B
    DIP_X_SL2B_CEN,
    DIP_X_SL2B_RR_CON0,
    DIP_X_SL2B_RR_CON1,
    DIP_X_SL2B_GAIN,
    DIP_X_SL2B_RZ,
    DIP_X_SL2B_XOFF,
    DIP_X_SL2B_YOFF,
    DIP_X_SL2B_SLP_CON0,
    DIP_X_SL2B_SLP_CON1,
    DIP_X_SL2B_SIZE,
    // SL2C
    DIP_X_SL2C_CEN,
    DIP_X_SL2C_RR_CON0,
    DIP_X_SL2C_RR_CON1,
    DIP_X_SL2C_GAIN,
    DIP_X_SL2C_RZ,
    DIP_X_SL2C_XOFF,
    DIP_X_SL2C_YOFF,
    DIP_X_SL2C_SLP_CON0,
    DIP_X_SL2C_SLP_CON1,
    DIP_X_SL2C_SIZE,
    // SL2D
    DIP_X_SL2D_CEN,
    DIP_X_SL2D_RR_CON0,
    DIP_X_SL2D_RR_CON1,
    DIP_X_SL2D_GAIN,
    DIP_X_SL2D_RZ,
    DIP_X_SL2D_XOFF,
    DIP_X_SL2D_YOFF,
    DIP_X_SL2D_SLP_CON0,
    DIP_X_SL2D_SLP_CON1,
    DIP_X_SL2D_SIZE,
    // SL2H
    DIP_X_SL2H_CEN,
    DIP_X_SL2H_RR_CON0,
    DIP_X_SL2H_RR_CON1,
    DIP_X_SL2H_GAIN,
    DIP_X_SL2H_RZ,
    DIP_X_SL2H_XOFF,
    DIP_X_SL2H_YOFF,
    DIP_X_SL2H_SLP_CON0,
    DIP_X_SL2H_SLP_CON1,
    DIP_X_SL2H_SIZE,
    //
    // SL2I
    DIP_X_SL2I_CEN,
    DIP_X_SL2I_RR_CON0,
    DIP_X_SL2I_RR_CON1,
    DIP_X_SL2I_GAIN,
    DIP_X_SL2I_RZ,
    DIP_X_SL2I_XOFF,
    DIP_X_SL2I_YOFF,
    DIP_X_SL2I_SLP_CON0,
    DIP_X_SL2I_SLP_CON1,
    DIP_X_SL2I_SIZE,
    //
    //  RNR
    DIP_X_RNR_CON1,
    DIP_X_RNR_VC0,
    DIP_X_RNR_VC1,
    DIP_X_RNR_GO_B,
    DIP_X_RNR_GO_G,
    DIP_X_RNR_GO_R,
    DIP_X_RNR_GAIN_ISO,
    DIP_X_RNR_RNG,
    DIP_X_RNR_CON2,
    DIP_X_RNR_CON3,
    DIP_X_RNR_SL,
    DIP_X_RNR_SSL_STH,
    //
    //  UDM
    DIP_X_UDM_INTP_CRS,
    DIP_X_UDM_INTP_NAT,
    DIP_X_UDM_INTP_AUG,
    DIP_X_UDM_LUMA_LUT1,
    DIP_X_UDM_LUMA_LUT2,
    DIP_X_UDM_SL_CTL,
    DIP_X_UDM_HFTD_CTL,
    DIP_X_UDM_NR_STR,
    DIP_X_UDM_NR_ACT,
    DIP_X_UDM_HF_STR,
    DIP_X_UDM_HF_ACT1,
    DIP_X_UDM_HF_ACT2,
    DIP_X_UDM_CLIP,
    DIP_X_UDM_DSB,
    DIP_X_UDM_LR_RAT,
    //
    //  G2G (CCM)
    DIP_X_G2G_CNV_1,
    DIP_X_G2G_CNV_2,
    DIP_X_G2G_CNV_3,
    DIP_X_G2G_CNV_4,
    DIP_X_G2G_CNV_5,
    DIP_X_G2G_CNV_6,
    //
    //LCE
    DIP_X_LCE25_CON,
    DIP_X_LCE25_ZR,
    DIP_X_LCE25_SLM_SIZE,
    //DIP_X_LCE25_OFST,
    //DIP_X_LCE25_BIAS,
    //DIP_X_LCE25_IMAGE_SIZE,
    DIP_X_LCE25_BIL_TH0,
    DIP_X_LCE25_BIL_TH1,
    DIP_X_LCE25_TM_PARA0,
    DIP_X_LCE25_TM_PARA1,
    DIP_X_LCE25_TM_PARA2,
    DIP_X_LCE25_TM_PARA3,
    DIP_X_LCE25_TM_PARA4,
    DIP_X_LCE25_TM_PARA5,
    DIP_X_LCE25_TM_PARA6,
    //
    //  G2G  CFC
    //DIP_X_G2G_CFC,
    //
    //  G2C
    DIP_X_G2C_CONV_0A,
    DIP_X_G2C_CONV_0B,
    DIP_X_G2C_CONV_1A,
    DIP_X_G2C_CONV_1B,
    DIP_X_G2C_CONV_2A,
    DIP_X_G2C_CONV_2B,
    //
    //  ANR
    DIP_X_ANR_CON1,
    DIP_X_ANR_CON2,
    DIP_X_ANR_YAD1,
    DIP_X_ANR_YAD2,
    DIP_X_ANR_Y4LUT1,
    DIP_X_ANR_Y4LUT2,
    DIP_X_ANR_Y4LUT3,
    DIP_X_ANR_C4LUT1,
    DIP_X_ANR_C4LUT2,
    DIP_X_ANR_C4LUT3,
    DIP_X_ANR_A4LUT2,
    DIP_X_ANR_A4LUT3,
    DIP_X_ANR_L4LUT1,
    DIP_X_ANR_L4LUT2,
    DIP_X_ANR_L4LUT3,
    DIP_X_ANR_PTY,
    DIP_X_ANR_CAD,
    DIP_X_ANR_PTC,
    DIP_X_ANR_LCE,
    DIP_X_ANR_MED1,
    DIP_X_ANR_MED2,
    DIP_X_ANR_MED3,
    DIP_X_ANR_MED4,
    DIP_X_ANR_MED5,
    DIP_X_ANR_HP1,
    DIP_X_ANR_HP2,
    DIP_X_ANR_HP3,
    DIP_X_ANR_ACT1,
    DIP_X_ANR_ACT2,
    DIP_X_ANR_ACT3,
    DIP_X_ANR_ACT4,
    DIP_X_ANR_ACTYHL,
    DIP_X_ANR_ACTYHH,
    DIP_X_ANR_ACTYL,
    DIP_X_ANR_ACTYHL2,
    DIP_X_ANR_ACTYHH2,
    DIP_X_ANR_ACTYL2,
    DIP_X_ANR_ACTC,
    DIP_X_ANR_YLAD,
    DIP_X_ANR_YLAD2,
    DIP_X_ANR_YLAD3,
    DIP_X_ANR_PTYL,
    DIP_X_ANR_LCOEF,
    DIP_X_ANR_YDIR,
    //
    //ANR2
    DIP_X_ANR2_CON1,
    DIP_X_ANR2_CON2,
    DIP_X_ANR2_YAD1,
    DIP_X_ANR2_Y4LUT1,
    DIP_X_ANR2_Y4LUT2,
    DIP_X_ANR2_Y4LUT3,
    DIP_X_ANR2_L4LUT1,
    DIP_X_ANR2_L4LUT2,
    DIP_X_ANR2_L4LUT3,
    DIP_X_ANR2_CAD,
    DIP_X_ANR2_PTC,
    DIP_X_ANR2_LCE,
    DIP_X_ANR2_MED1,
    DIP_X_ANR2_MED2,
    DIP_X_ANR2_MED3,
    DIP_X_ANR2_MED4,
    DIP_X_ANR2_MED5,
    DIP_X_ANR2_ACTY,
    DIP_X_ANR2_ACTC,
    //
    //  CCR
    DIP_X_CCR_CON,
    DIP_X_CCR_YLUT,
    DIP_X_CCR_UVLUT,
    DIP_X_CCR_YLUT2,
    DIP_X_CCR_SAT_CTRL,
    DIP_X_CCR_UVLUT_SP,
    DIP_X_CCR_HUE1,
    DIP_X_CCR_HUE2,
    DIP_X_CCR_HUE3,
    DIP_X_CCR_L4LUT1,
    DIP_X_CCR_L4LUT2,
    DIP_X_CCR_L4LUT3,
    //
    //  BOK
    //DIP_X_BOK_CON,
    //DIP_X_BOK_TUN,
    //DIP_X_BOK_OFF,
    //DIP_X_BOK_RSV1,
    //
    //  PCA
    DIP_X_PCA_CON1,
    DIP_X_PCA_CON2,
    DIP_X_PCA_CON7,
    //
    //HFG
    DIP_X_HFG_CON_0,
    DIP_X_HFG_LUMA_0,
    DIP_X_HFG_LUMA_1,
    DIP_X_HFG_LUMA_2,
    DIP_X_HFG_LCE_0,
    DIP_X_HFG_LCE_1,
    DIP_X_HFG_LCE_2,
    DIP_X_HFG_RAN_0,
    DIP_X_HFG_RAN_1,
    DIP_X_HFG_RAN_3,
    //
    //  EE
    DIP_X_SEEE_CLIP_CTRL_1,
    DIP_X_SEEE_CLIP_CTRL_2,
    DIP_X_SEEE_CLIP_CTRL_3,
    DIP_X_SEEE_BLND_CTRL_1,
    DIP_X_SEEE_BLND_CTRL_2,
    DIP_X_SEEE_GN_CTRL,
    DIP_X_SEEE_LUMA_CTRL_1,
    DIP_X_SEEE_LUMA_CTRL_2,
    DIP_X_SEEE_LUMA_CTRL_3,
    DIP_X_SEEE_LUMA_CTRL_4,
    DIP_X_SEEE_SLNK_CTRL_1,
    DIP_X_SEEE_SLNK_CTRL_2,
    DIP_X_SEEE_GLUT_CTRL_1,
    DIP_X_SEEE_GLUT_CTRL_2,
    DIP_X_SEEE_GLUT_CTRL_3,
    DIP_X_SEEE_GLUT_CTRL_4,
    DIP_X_SEEE_GLUT_CTRL_5,
    DIP_X_SEEE_GLUT_CTRL_6,
    //
    // SE
    DIP_X_SEEE_OUT_EDGE_CTRL,
    DIP_X_SEEE_SE_Y_CTRL,
    DIP_X_SEEE_SE_EDGE_CTRL_1,
    DIP_X_SEEE_SE_EDGE_CTRL_2,
    DIP_X_SEEE_SE_EDGE_CTRL_3,
    DIP_X_SEEE_SE_SPECL_CTRL,
    DIP_X_SEEE_SE_CORE_CTRL_1,
    DIP_X_SEEE_SE_CORE_CTRL_2,
    //
    // NR3D
/*
    DIP_X_NR3D_AY_CON1,
    DIP_X_NR3D_AY_CON2,
    DIP_X_NR3D_AY_CON3,
    DIP_X_NR3D_AY_CON4,
    DIP_X_NR3D_AY_CON5,
    DIP_X_NR3D_AC_CON1,
    DIP_X_NR3D_AC_CON2,
    DIP_X_NR3D_AC_CON3,
    DIP_X_NR3D_AC_CON4,
    DIP_X_NR3D_AC_CON5,
    DIP_X_NR3D_B1Y_CON1,
    DIP_X_NR3D_B1Y_CON2,
    DIP_X_NR3D_B1Y_CON3,
    DIP_X_NR3D_B1C_CON1,
    DIP_X_NR3D_B1C_CON2,
    DIP_X_NR3D_B1C_CON3,
    DIP_X_NR3D_LSCG1_XCON,
    DIP_X_NR3D_LSCG1_YCON,
    DIP_X_NR3D_LSCG1_CON,
    DIP_X_NR3D_LSCG1_SCON,
    DIP_X_NR3D_NLY_CON1,
    DIP_X_NR3D_NLY_CON2,
    DIP_X_NR3D_MLY_CON,
    DIP_X_NR3D_NLC_CON1,
    DIP_X_NR3D_NLC_CON2,
    DIP_X_NR3D_MLC_CON,
    DIP_X_NR3D_SLY_CON,
    DIP_X_NR3D_SLC_CON,
    DIP_X_NR3D_DIFG_CON,
    DIP_X_NR3D_YCJC_CON1,
    DIP_X_NR3D_YCJC_CON2,
*/
    //
    // MFB
    DIP_X_MFB_CON,
    DIP_X_MFB_LL_CON1,
    DIP_X_MFB_LL_CON2,
    DIP_X_MFB_LL_CON4,
    //
    // MIXER3
    DIP_X_MIX3_CTRL_0,
    DIP_X_MIX3_CTRL_1,
    //
    // adaptive Gamma
    DIP_X_GMA_GMAMode,
    DIP_X_GMA_GMACamMode,
    DIP_X_GMA_GmaProfile,
    DIP_X_GMA_ChipVersion,
    DIP_X_GMA_MainVersion,
    DIP_X_GMA_SubVersion,
    DIP_X_GMA_SystemVersion,
    DIP_X_GMA_EVRatio,
    DIP_X_GMA_LowContrastThr,
    DIP_X_GMA_LowContrastRatio,
    DIP_X_GMA_LowContrastSeg,
    DIP_X_GMA_Contrast,
    DIP_X_GMA_Contrast_L,
    DIP_X_GMA_Contrast_H,
    DIP_X_GMA_HdrContrastWeight,
    DIP_X_GMA_EVContrastY,
    DIP_X_GMA_ContrastY_L,
    DIP_X_GMA_ContrastY_H,
    DIP_X_GMA_NightContrastWeight,
    DIP_X_GMA_LV,
    DIP_X_GMA_LV_L,
    DIP_X_GMA_LV_H,
    DIP_X_GMA_HdrLVWeight,
    DIP_X_GMA_NightLVWeight,
    DIP_X_GMA_SmoothEnable,
    DIP_X_GMA_SmoothSpeed,
    DIP_X_GMA_SmoothWaitAE,
    DIP_X_GMA_GMACurveEnable,
    DIP_X_GMA_CenterPt,
    DIP_X_GMA_LowCurve,
    DIP_X_GMA_SlopeL,
    DIP_X_GMA_FlareEnable,
    DIP_X_GMA_FlareOffset,
    //
    //dynamic LCE
    DIP_X_LCE_LceProfile,
    DIP_X_LCE_LCECamMode,
    DIP_X_LCE_AutoHDREnable,
    DIP_X_LCE_ChipVersion,
    DIP_X_LCE_MainVersion,
    DIP_X_LCE_SubVersion,
    DIP_X_LCE_SystemVersion,
    DIP_X_LCE_LV,
    DIP_X_LCE_ContrastY10,
    DIP_X_LCE_EVRatio,
    DIP_X_LCE_EVContrastY10,
    DIP_X_LCE_SegDiv,
    DIP_X_LCE_ContrastIdx_L,
    DIP_X_LCE_ContrastIdx_H,
    DIP_X_LCE_LVIdx_L,
    DIP_X_LCE_LVIdx_H,
    DIP_X_LCE_DetailRatio1,
    DIP_X_LCE_DetailRatio50,
    DIP_X_LCE_DetailRatio500,
    DIP_X_LCE_DetailRatio950,
    DIP_X_LCE_DetailRatio999,
    DIP_X_LCE_IntpDiffRangex128,
    DIP_X_LCE_DiffRangeIdx_L,
    DIP_X_LCE_DiffRangeIdx_H,
    DIP_X_LCE_P1,
    DIP_X_LCE_P50,
    DIP_X_LCE_P500,
    DIP_X_LCE_P950,
    DIP_X_LCE_P999,
    DIP_X_LCE_O1,
    DIP_X_LCE_O50,
    DIP_X_LCE_O500,
    DIP_X_LCE_O950,
    DIP_X_LCE_O999,
    DIP_X_LCE_CenMaxSlope,
    DIP_X_LCE_CenMinSlope,
    DIP_X_LCE_SmoothEnable,
    DIP_X_LCE_SmoothSpeed,
    DIP_X_LCE_SmoothWaitAE,
    DIP_X_LCE_FlareEnable,
    DIP_X_LCE_FlareOffset,

    //  Common
    COMM_00,
    COMM_01,
    COMM_02,
    COMM_03,
    COMM_04,
    COMM_05,
    COMM_06,
    COMM_07,
    COMM_08,
    COMM_09,
    COMM_10,
    COMM_11,
    COMM_12,
    COMM_13,
    COMM_14,
    COMM_15,
    COMM_16,
    COMM_17,
    COMM_18,
    COMM_19,
    COMM_20,
    COMM_21,
    COMM_22,
    COMM_23,
    COMM_24,
    COMM_25,
    COMM_26,
    COMM_27,
    COMM_28,
    COMM_29,
    COMM_30,
    COMM_31,
    COMM_32,
    COMM_33,
    COMM_34,
    COMM_35,
    COMM_36,
    COMM_37,
    COMM_38,
    COMM_39,
    COMM_40,
    COMM_41,
    COMM_42,
    COMM_43,
    COMM_44,
    COMM_45,
    COMM_46,
    COMM_47,
    COMM_48,
    COMM_49,
    COMM_50,
    COMM_51,
    COMM_52,
    COMM_53,
    COMM_54,
    COMM_55,
    COMM_56,
    COMM_57,
    COMM_58,
    COMM_59,
    COMM_60,
    COMM_61,
    COMM_62,
    COMM_63,
    //
};

enum
{
    // CAM_CTL_EN_P1
    CAM_CTL_EN_P1_Begin     =   CAM_CTL_EN,
    // CAM_CTL_EN_P2
    CAM_CTL_EN_P2_Begin     =   DIP_X_CTL_RGB_EN,
    //  SL2F
    CAM_SL2F_Begin          =   CAM_SL2F_CEN,
    //  DBS
    CAM_DBS_Begin           =   CAM_DBS_SIGMA,
    //  OBC
    CAM_OBC_Begin           =   CAM_OBC_OFFST0,
    //  BPC
    CAM_BPC_Begin           =   CAM_BNR_BPC_CON,
    //  NR1
    CAM_NR1_Begin           =   CAM_BNR_NR1_CON,
    //  PDC
    CAM_PDC_Begin           =   CAM_BNR_PDC_CON,
    //  RMM
    CAM_RMM_Begin           =   CAM_RMM_OSC,
    //  RMG
    CAM_RMG_Begin           =   CAM_RMG_HDR_CFG,
    //  LCS
    CAM_LCS_Begin           =   CAM_LCS25_FLR,
    //  LSC
    CAM_LSC_Begin           =   CAM_LSC_CTL1,
    //  RPG
    CAM_RPG_Begin           =   CAM_RPG_SATU_1,
    //  SL2G
    DIP_X_SL2G_Begin        =   DIP_X_SL2G_CEN,
    //  DBS
    DIP_X_DBS2_Begin        =   DIP_X_DBS2_SIGMA,
    //  OBC
    DIP_X_OBC2_Begin        =   DIP_X_OBC2_OFFST0,
    //  BPC
    DIP_X_BPC2_Begin        =   DIP_X_BNR2_BPC_CON,
    //  NR1
    DIP_X_NR12_Begin        =   DIP_X_BNR2_NR1_CON,
    //  PDC
    DIP_X_PDC2_Begin        =   DIP_X_BNR2_PDC_CON,
    //  RMM
    DIP_X_RMM2_Begin        =   DIP_X_RMM2_OSC,
    //  RMG2
    DIP_X_RMG2_Begin        =   DIP_X_RMG2_HDR_CFG,
    //  LSC
    DIP_X_LSC2_Begin        =   DIP_X_LSC2_CTL1,
    //  PGN
    DIP_X_PGN_Begin         =   DIP_X_PGN_SATU_1,
    //  SL2
    DIP_X_SL2_Begin         =   DIP_X_SL2_CEN,
    DIP_X_SL2B_Begin        =   DIP_X_SL2B_CEN,
    DIP_X_SL2C_Begin        =   DIP_X_SL2C_CEN,
    DIP_X_SL2D_Begin        =   DIP_X_SL2D_CEN,
    DIP_X_SL2H_Begin        =   DIP_X_SL2H_CEN,
    DIP_X_SL2I_Begin        =   DIP_X_SL2I_CEN,
    //  RNR
    DIP_X_RNR_Begin         =   DIP_X_RNR_CON1,
    //  UDM
    DIP_X_UDM_Begin         =   DIP_X_UDM_INTP_CRS,
    //  G2G
    DIP_X_G2G_Begin         =   DIP_X_G2G_CNV_1,
    //  G2C_CFC
    //DIP_X_G2G_CFC_Begin     =   DIP_X_G2G_CFC,
    DIP_X_LCE_Begin         =   DIP_X_LCE25_CON,
    //  G2C
    DIP_X_G2C_Begin         =   DIP_X_G2C_CONV_0A,
    //  ANR
    DIP_X_ANR_Begin         =   DIP_X_ANR_CON1,
    //  ANR2
    DIP_X_ANR2_Begin        =   DIP_X_ANR2_CON1,
    //  CCR
    DIP_X_CCR_Begin         =   DIP_X_CCR_CON,
    //  BOK
    //DIP_X_BOK_Begin         =   DIP_X_BOK_CON,
    //  PCA
    DIP_X_PCA_Begin         =   DIP_X_PCA_CON1,
    //  HFG
    DIP_X_HFG_Begin         =   DIP_X_HFG_CON_0,
    //  EE
    DIP_X_EE_Begin          =   DIP_X_SEEE_CLIP_CTRL_1,
    //  SE
    DIP_X_SE_Begin          =   DIP_X_SEEE_OUT_EDGE_CTRL,
    //  NR3D
    //DIP_X_NR3D_Begin        =   DIP_X_NR3D_AY_CON1,
    //  MFB
    DIP_X_MFB_Begin         =   DIP_X_MFB_CON,
    //  MIXER3
    DIP_X_MIXER3_Begin      =   DIP_X_MIX3_CTRL_0,
    //adaptive Gamma
    DIP_X_GMA_Begin         =   DIP_X_GMA_GMAMode,
    //dynamic LCE
    DIP_X_LCE_ENV_Begin     =   DIP_X_LCE_LceProfile,

    //  Common
    COMM_Begin              =   COMM_00,
    //
    //
    TagID_Total_Num         =   COMM_63 + 1
};

struct IspDebugTag
{
    MUINT32     u4ID;
    MUINT32     u4Val;
};

typedef struct IspExifDebugInfo
{
    struct  Header
    {
        MUINT32     u4KeyID;
        MUINT32     u4ModuleCount;
        MUINT32     u4DebugInfoOffset;
        MUINT32     u4GGMTableInfoOffset;
        MUINT32     u4PCATableInfoOffset;
        MUINT32     u4ANRTableInfoOffset;
        //MUINT32     u4GmaInfoOffset;
    }   hdr;

    struct IspDebugInfo
    {
        IspDebugTag     tags[TagID_Total_Num];
    } debugInfo;

    struct IspGGMTableInfo
    {
        MUINT32         u4TableSize;
        MUINT32         GGM[288];
    } GGMTableInfo;

    struct IspPCATableInfo
    {
        MUINT32         u4TableSize;
        MUINT32         PCA[180];
    } PCATableInfo;

    struct IspANRTableInfo
    {
        MUINT32         u4TableSize;
        MUINT32         ANR[256];
    } ANRTableInfo;

    struct IspGmaInfo
    {
        MINT32 i4GMAMode;
        MINT32 i4GMACamMode;
        MINT32 i4GmaProfile;
        MINT32 i4ChipVersion;
        MINT32 i4MainVersion;
        MINT32 i4SubVersion;
        MINT32 i4SystemVersion;
        MINT32 i4EVRatio;
        MINT32 i4LowContrastThr;
        MINT32 i4LowContrastRatio;
        MINT32 i4LowContrastSeg;
        MINT32 i4Contrast;
        MINT32 i4Contrast_L;
        MINT32 i4Contrast_H;
        MINT32 i4HdrContrastWeight;
        MINT32 i4EVContrastY;
        MINT32 i4ContrastY_L;
        MINT32 i4ContrastY_H;
        MINT32 i4NightContrastWeight;
        MINT32 i4LV;
        MINT32 i4LV_L;
        MINT32 i4LV_H;
        MINT32 i4HdrLVWeight;
        MINT32 i4NightLVWeight;
        MINT32 i4SmoothEnable;
        MINT32 i4SmoothSpeed;
        MINT32 i4SmoothWaitAE;
        MINT32 i4GMACurveEnable;
        MINT32 i4CenterPt;
        MINT32 i4LowCurve;
        MINT32 i4SlopeL;
        MINT32 i4FlareEnable;
        MINT32 i4FlareOffset;
    };/* GmaInfo;*/

    struct IspLceInfo
    {
        MINT32 i4LceProfile;
        MINT32 i4LCECamMode;
        MINT32 i4AutoHDREnable;
        MINT32 i4ChipVersion;
        MINT32 i4MainVersion;
        MINT32 i4SubVersion;
        MINT32 i4SystemVersion;
        MINT32 i4LV;
        MINT32 i4ContrastY10;
        MINT32 i4EVRatio;
        MINT32 i4EVContrastY10;
        MINT32 i4SegDiv;
        MINT32 i4ContrastIdx_L;
        MINT32 i4ContrastIdx_H;
        MINT32 i4LVIdx_L;
        MINT32 i4LVIdx_H;
        MINT32 i4DetailRatio1;
        MINT32 i4DetailRatio50;
        MINT32 i4DetailRatio500;
        MINT32 i4DetailRatio950;
        MINT32 i4DetailRatio999;
        MINT32 i4IntpDiffRange128;
        MINT32 i4DiffRangeIdx_L;
        MINT32 i4DiffRangeIdx_H;

        MINT32 i4P1;
        MINT32 i4P50;
        MINT32 i4P500;
        MINT32 i4P950;
        MINT32 i4P999;
        MINT32 i4O1;
        MINT32 i4O50;
        MINT32 i4O500;
        MINT32 i4O950;
        MINT32 i4O999;
        MINT32 i4CenMaxSlope;
        MINT32 i4CenMinSlope;

        MINT32 i4SmoothEnable;
        MINT32 i4SmoothSpeed;
        MINT32 i4SmoothWaitAE;
        MINT32 i4FlareEnable;
        MINT32 i4FlareOffset;

    };

} IspExifDebugInfo_T;


};  //  namespace NSIspExifDebug
/*******************************************************************************
*
*******************************************************************************/
namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
template <MUINT32 total_module, MUINT32 tag_module>
struct ModuleNum
{
/*
    |   8  |       8      |   8  |     8      |
    | 0x00 | total_module | 0x00 | tag_module |
*/
    enum
    {
        val = ((total_module & 0xFF) << 16) | ((tag_module & 0xFF))
    };
};


template <MUINT32 module_id, MUINT32 tag_id, MUINT32 line_keep = 0>
struct ModuleTag
{
/*
    |     8     |      1    |   7  |    16    |
    | module_id | line_keep | 0x00 |  tag_id  |
*/
    enum
    {
        val = ((module_id & 0xFF) << 24)
            | ((line_keep & 0x01) << 23)
            | ((tag_id  & 0xFFFF) << 0)
    };
};


inline MUINT32 getModuleTag(MUINT32 module_id, MUINT32 tag_id, MUINT32 line_keep = 0)
{
/*
    |     8     |      1    |   7  |    16    |
    | module_id | line_keep | 0x00 |  tag_id  |
*/
    return  ((module_id & 0xFF) << 24)
          | ((line_keep & 0x01) << 23)
          | ((tag_id  & 0xFFFF) << 0)
            ;
}


enum { EModuleID_IspDebug = 0x0004 };
template <MUINT32 tag_id, MUINT32 line_keep = 0>
struct IspTag
{
    enum { val = ModuleTag<EModuleID_IspDebug, tag_id, line_keep>::val };
};


inline MUINT32 getIspTag(MUINT32 tag_id, MUINT32 line_keep = 0)
{
    return  getModuleTag(EModuleID_IspDebug, tag_id, line_keep);
}


//  Default of IspExifDebugInfo::Header
static NSIspExifDebug::IspExifDebugInfo::Header const g_rIspExifDebugInfoHdr =
{
    u4KeyID:            ISP_DEBUG_KEYID,
    u4ModuleCount:      ModuleNum<4, 1>::val,
    u4DebugInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header),
    u4GGMTableInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo),
    u4PCATableInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspGGMTableInfo),
    u4ANRTableInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspGGMTableInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspPCATableInfo)
};

};  //  namespace NSIspExifDebug
#endif // _DBG_ISP_PARAM_H_

