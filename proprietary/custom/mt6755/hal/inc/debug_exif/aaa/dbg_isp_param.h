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
#include "../dbg_id_param.h"

namespace NSIspExifDebug
{

enum { IspDebugTagVersion = 4};

enum IspDebugTagID
{
    IspTagVersion,
    //  RAWIspCamInfo
    IspProfile,
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
    IDX_DBS,
    IDX_OBC,
    IDX_BPC,
    IDX_NR1,
    IDX_LSC,
    IDX_NSL2,
    IDX_UDM,
    IDX_LCE,
    IDX_GGM,
    IDX_ANR,
    IDX_ANR2,
    IDX_CCR,
    IDX_BOK,
    IDX_PCA,
    IDX_EE,
    IDX_NR3D,
    IDX_MFB,
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
    UDM_UPPER_ISO,
    UDM_LOWER_ISO,
    UDM_UPPER_IDX,
    UDM_LOWER_IDX,
    ANR_UPPER_ISO,
    ANR_LOWER_ISO,
    ANR_UPPER_IDX,
    ANR_LOWER_IDX,    
    ANR2_UPPER_ISO,
    ANR2_LOWER_ISO,
    ANR2_UPPER_IDX,
    ANR2_LOWER_IDX,
    EE_UPPER_ISO,
    EE_LOWER_ISO,
    EE_UPPER_IDX,
    EE_LOWER_IDX,
    //
    // ISP enable (TOP)
    CAM_CTL_EN_P1,
    CAM_CTL_EN_P1_D,
    CAM_CTL_EN_P2,
    //
    //  DBS
    CAM_DBS_SIGMA,
    CAM_DBS_BSTBL_0,
    CAM_DBS_BSTBL_1,
    CAM_DBS_BSTBL_2,
    CAM_DBS_BSTBL_3,
    CAM_DBS_CTL,
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
    CAM_BPC_CON,
    CAM_BPC_TH1,
    CAM_BPC_TH2,
    CAM_BPC_TH3,
    CAM_BPC_TH4,
    CAM_BPC_DTC,
    CAM_BPC_COR,
    CAM_BPC_TBLI1,
    CAM_BPC_TBLI2,
    CAM_BPC_TH1_C,
    CAM_BPC_TH2_C,
    CAM_BPC_TH3_C,
    //
    //  RMM
    CAM_BPC_RMM1,
    CAM_BPC_RMM2,
    CAM_BPC_RMM_REVG_1,
    CAM_BPC_RMM_REVG_2,
    CAM_BPC_RMM_LEOS,
    CAM_BPC_RMM_GCNT,
    //
    //  NR1
    CAM_NR1_CON,
    CAM_NR1_CT_CON,
    //
    //  LSC
    CAM_LSC_CTL1,
    CAM_LSC_CTL2,
    CAM_LSC_CTL3,
    CAM_LSC_LBLOCK,
    CAM_LSC_RATIO,
    CAM_LSC_GAIN_TH,
    //
    // NSL2
    CAM_NSL2_CEN,
	CAM_NSL2_RR_CON0,
	CAM_NSL2_RR_CON1,
	CAM_NSL2_GAIN, 
    //
    //  RPG
    CAM_RPG_SATU_1,
    CAM_RPG_SATU_2,
    CAM_RPG_GAIN_1,
    CAM_RPG_GAIN_2,
    CAM_RPG_OFST_1,
    CAM_RPG_OFST_2,        
    //
    //  PGN
    CAM_PGN_SATU_1,
    CAM_PGN_SATU_2,
    CAM_PGN_GAIN_1,
    CAM_PGN_GAIN_2,
    CAM_PGN_OFST_1,
    CAM_PGN_OFST_2,
    //
    //  UDM
    CAM_UDM_INTP_CRS,    
    CAM_UDM_INTP_NAT,     
    CAM_UDM_INTP_AUG,     
    CAM_UDM_LUMA_LUT1,    
    CAM_UDM_LUMA_LUT2,    
    CAM_UDM_SL_CTL,       
    CAM_UDM_HFTD_CTL,     
    CAM_UDM_NR_STR,       
    CAM_UDM_NR_ACT,       
    CAM_UDM_HF_STR,       
    CAM_UDM_HF_ACT1,      
    CAM_UDM_HF_ACT2,      
    CAM_UDM_CLIP,         
    CAM_UDM_DSB,          
    CAM_UDM_TILE_EDGE,    
    CAM_UDM_DSL,          
    CAM_UDM_SPARE_1,      
    CAM_UDM_SPARE_2,      
    CAM_UDM_SPARE_3,  
    //
    //  G2G (CCM)
    CAM_G2G_CNV_1,
    CAM_G2G_CNV_2,
    CAM_G2G_CNV_3,
    CAM_G2G_CNV_4,
    CAM_G2G_CNV_5,
    CAM_G2G_CNV_6,
    //
    //  G2C
    CAM_G2C_CONV_0A,
    CAM_G2C_CONV_0B,
    CAM_G2C_CONV_1A,
    CAM_G2C_CONV_1B,
    CAM_G2C_CONV_2A,
    CAM_G2C_CONV_2B,
    //
    //  ANR
    CAM_ANR_CON1,
    CAM_ANR_CON2,
    CAM_ANR_YAD1,
    CAM_ANR_YAD2,
    CAM_ANR_Y4LUT1,
    CAM_ANR_Y4LUT2,
    CAM_ANR_Y4LUT3,
    CAM_ANR_C4LUT1,
    CAM_ANR_C4LUT2,
    CAM_ANR_C4LUT3,
    CAM_ANR_A4LUT2,
    CAM_ANR_A4LUT3,
    CAM_ANR_L4LUT1,
    CAM_ANR_L4LUT2,
    CAM_ANR_L4LUT3,
    CAM_ANR_PTY,
    CAM_ANR_CAD,
    CAM_ANR_PTC,
    CAM_ANR_LCE,
    CAM_ANR_MED1,
    CAM_ANR_MED2,
    CAM_ANR_MED3,
    CAM_ANR_MED4,
    CAM_ANR_HP1,
    CAM_ANR_HP2,
    CAM_ANR_HP3,
    CAM_ANR_ACT1,
    CAM_ANR_ACT2,
    CAM_ANR_ACT3,
    CAM_ANR_ACTY,
    CAM_ANR_ACTC,
    //
    //ANR2
    CAM_ANR2_CON1,
    CAM_ANR2_CON2,
    CAM_ANR2_YAD1,
    CAM_ANR2_Y4LUT1,
    CAM_ANR2_Y4LUT2,
    CAM_ANR2_Y4LUT3,
    CAM_ANR2_L4LUT1,
    CAM_ANR2_L4LUT2,
    CAM_ANR2_L4LUT3,
    CAM_ANR2_CAD, 
    CAM_ANR2_PTC, 
    CAM_ANR2_LCE, 
    CAM_ANR2_MED1,
    CAM_ANR2_MED2,
    CAM_ANR2_MED3,
    CAM_ANR2_MED4,
    CAM_ANR2_ACTY,
    CAM_ANR2_ACTC,
    //
    //  CCR
    CAM_CCR_CON,
    CAM_CCR_YLUT,
    CAM_CCR_UVLUT,
    CAM_CCR_YLUT2,
    CAM_CCR_SAT_CTRL,
    CAM_CCR_UVLUT_SP,
    CAM_CCR_HUE1,
    CAM_CCR_HUE2,
    CAM_CCR_HUE3,
    //
    //  BOK
    CAM_BOK_CON,
    CAM_BOK_TUN,
    CAM_BOK_OFF,
    CAM_BOK_RSV1,
    //
    //  PCA
    CAM_PCA_CON1,
    CAM_PCA_CON2,
    //
    //  EE
    CAM_SEEE_SRK_CTRL,
    CAM_SEEE_CLIP_CTRL,
    CAM_SEEE_FLT_CTRL_1,
    CAM_SEEE_FLT_CTRL_2,
    CAM_SEEE_GLUT_CTRL_01,
    CAM_SEEE_GLUT_CTRL_02,
    CAM_SEEE_GLUT_CTRL_03,
    CAM_SEEE_GLUT_CTRL_04,
    CAM_SEEE_GLUT_CTRL_05,
    CAM_SEEE_GLUT_CTRL_06,
    CAM_SEEE_EDTR_CTRL,
    CAM_SEEE_GLUT_CTRL_07,
    CAM_SEEE_GLUT_CTRL_08,
    CAM_SEEE_GLUT_CTRL_09,
    CAM_SEEE_GLUT_CTRL_10,
    CAM_SEEE_GLUT_CTRL_11,
    //
    // SE
    CAM_SEEE_OUT_EDGE_CTRL,
    CAM_SEEE_SE_Y_CTRL,
    CAM_SEEE_SE_EDGE_CTRL_1,
    CAM_SEEE_SE_EDGE_CTRL_2,
    CAM_SEEE_SE_EDGE_CTRL_3,
    CAM_SEEE_SE_SPECL_CTRL,
    CAM_SEEE_SE_CORE_CTRL_1,
    CAM_SEEE_SE_CORE_CTRL_2,
    //
    // NR3D
    CAM_NR3D_BLEND,
    CAM_NR3D_LMT_CPX,
    CAM_NR3D_LMT_Y_CON1,
    CAM_NR3D_LMT_Y_CON2,
    CAM_NR3D_LMT_Y_CON3,
    CAM_NR3D_LMT_U_CON1,
    CAM_NR3D_LMT_U_CON2,
    CAM_NR3D_LMT_U_CON3,
    CAM_NR3D_LMT_V_CON1,
    CAM_NR3D_LMT_V_CON2,
    CAM_NR3D_LMT_V_CON3,
    //
    // MFB
    CAM_MFB_LL_CON2,
    CAM_MFB_LL_CON3,
    CAM_MFB_LL_CON4,
    CAM_MFB_LL_CON5,
    CAM_MFB_LL_CON6,
    //
    // MIXER3
    CAM_MIX3_CTRL_0,
    CAM_MIX3_CTRL_1,
    CAM_MIX3_SPARE,
    //
    // LCE
    CAM_LCE_QUA,
    //
    // adaptive Gamma
    CAM_GMA_GMAMode,
    CAM_GMA_SensorMode,
    CAM_GMA_ChipVersion,
    CAM_GMA_MainVersion,
    CAM_GMA_SubVersion,
    CAM_GMA_SystemVersion,
    CAM_GMA_EVRatio,
    CAM_GMA_LowContrastThr,
    CAM_GMA_LowContrastRatio,
    CAM_GMA_LowContrastSeg,
    CAM_GMA_Contrast,
    CAM_GMA_Contrast_L,
    CAM_GMA_Contrast_H,
    CAM_GMA_HdrContrastWeight,
    CAM_GMA_EVContrastY,
    CAM_GMA_ContrastY_L,
    CAM_GMA_ContrastY_H,
    CAM_GMA_NightContrastWeight,
    CAM_GMA_LV,
    CAM_GMA_LV_L,
    CAM_GMA_LV_H,
    CAM_GMA_HdrLVWeight,
    CAM_GMA_NightLVWeight,
    CAM_GMA_SmoothEnable,
    CAM_GMA_SmoothSpeed,
    CAM_GMA_SmoothWaitAE,
    CAM_GMA_GMACurveEnable,
    CAM_GMA_CenterPt,
    CAM_GMA_LowCurve,
    CAM_GMA_SlopeL,
    CAM_GMA_FlareEnable,
    CAM_GMA_FlareOffset,
    //
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
    CAM_CTL_EN_P1_Begin     =   CAM_CTL_EN_P1,
    // CAM_CTL_EN_P1_D
    CAM_CTL_EN_P1_D_Begin   =   CAM_CTL_EN_P1_D,   
    // CAM_CTL_EN_P2
    CAM_CTL_EN_P2_Begin     =   CAM_CTL_EN_P2,
    //  DBS
    CAM_DBS_Begin           =   CAM_DBS_SIGMA,
    //  OBC
    CAM_OBC_Begin           =   CAM_OBC_OFFST0,
    //  BPC
    CAM_BPC_Begin           =   CAM_BPC_CON,
    // RMM
    CAM_RMM_Begin           =   CAM_BPC_RMM1,
    //  NR1
    CAM_NR1_Begin           =   CAM_NR1_CON,
    //  LSC
    CAM_LSC_Begin           =   CAM_LSC_CTL1,
    //  NSL2
    CAM_NSL2_Begin          =   CAM_NSL2_CEN,    
    //  RPG
    CAM_RPG_Begin           =   CAM_RPG_SATU_1,    
    //  PGN
    CAM_PGN_Begin           =   CAM_PGN_SATU_1,
    //  CFA
    CAM_UDM_Begin           =   CAM_UDM_INTP_CRS,
    //  G2G
    CAM_G2G_Begin           =   CAM_G2G_CNV_1,
    //  G2C
    CAM_G2C_Begin           =   CAM_G2C_CONV_0A,
    //  ANR
    CAM_ANR_Begin           =   CAM_ANR_CON1,
    //  ANR2
    CAM_ANR2_Begin          =   CAM_ANR2_CON1,
    //  CCR
    CAM_CCR_Begin           =   CAM_CCR_CON,
    //  BOK
    CAM_BOK_Begin           =   CAM_BOK_CON,
    //  PCA
    CAM_PCA_Begin           =   CAM_PCA_CON1,
    //  EE
    CAM_EE_Begin            =   CAM_SEEE_SRK_CTRL,
    //  SE
    CAM_SE_Begin            =   CAM_SEEE_OUT_EDGE_CTRL,
    //  NR3D
    CAM_NR3D_Begin          =   CAM_NR3D_BLEND,
    //  MFB
    CAM_MFB_Begin           =   CAM_MFB_LL_CON2,
    //  MIXER3
    CAM_MIXER3_Begin        =   CAM_MIX3_CTRL_0,  
    // LCE
    CAM_LCE_Begin           =   CAM_LCE_QUA,
    //adaptive Gamma
    CAM_GMA_Begin           =   CAM_GMA_GMAMode,
    
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

    struct IspGmaInfo
    {
        MINT32 i4GMAMode;
        MINT32 i4SensorMode;
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
    u4ModuleCount:      ModuleNum<3, 1>::val,
    u4DebugInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header),
    u4GGMTableInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo),
    u4PCATableInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspGGMTableInfo),
    //u4GmaInfoOffset:       sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspGGMTableInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspPCATableInfo)
};

};  //  namespace NSIspExifDebug
#endif // _DBG_ISP_PARAM_H_

