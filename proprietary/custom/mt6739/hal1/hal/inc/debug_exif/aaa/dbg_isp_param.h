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

enum { IspDebugTagVersion = 8};

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
    IDX_PDC,
    IDX_NSL2A,
    IDX_CFA,
    IDX_ANR,
    IDX_CCR,
    IDX_PCA,
    IDX_EE,
    //
    ANR_TBL_CT_IDX,
    ANR_TBL_ISO_IDX,
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
    DBS_UPPER_ISO,
    DBS_LOWER_ISO,
    DBS_UPPER_IDX,
    DBS_LOWER_IDX,
    OBC_UPPER_ISO,
    OBC_LOWER_ISO,
    OBC_UPPER_IDX,
    OBC_LOWER_IDX,
    BPC_UPPER_ISO,
    BPC_LOWER_ISO,
    BPC_UPPER_IDX,
    BPC_LOWER_IDX,
    NR1_UPPER_ISO,
    NR1_LOWER_ISO,
    NR1_UPPER_IDX,
    NR1_LOWER_IDX,
    PDC_UPPER_ISO,
    PDC_LOWER_ISO,
    PDC_UPPER_IDX,
    PDC_LOWER_IDX,
    CFA_UPPER_ISO,
    CFA_LOWER_ISO,
    CFA_UPPER_IDX,
    CFA_LOWER_IDX,
    ANR_UPPER_ISO,
    ANR_LOWER_ISO,
    ANR_UPPER_IDX,
    ANR_LOWER_IDX,
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
    //  NR1
    CAM_NR1_CON,
    CAM_NR1_CT_CON,
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
    //  LSC
    CAM_LSC_CTL1,
    CAM_LSC_CTL2,
    CAM_LSC_CTL3,
    CAM_LSC_LBLOCK,
    CAM_LSC_RATIO,
    CAM_LSC_GAIN_TH,
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
    //  SL2
    CAM_SL2_CEN,
    CAM_SL2_MAX0_RR,
    CAM_SL2_MAX1_RR,
    CAM_SL2_MAX2_RR,
    CAM_SL2_HRZ,
    CAM_SL2_XOFF,
    CAM_SL2_YOFF,
    //
    //  CFA
    CAM_DM_O_BYP,
    CAM_DM_O_ED_FLAT,
    CAM_DM_O_ED_NYQ,
    CAM_DM_O_ED_STEP,
    CAM_DM_O_RGB_HF,
    CAM_DM_O_DOT,
    CAM_DM_O_F1_ACT,
    CAM_DM_O_F2_ACT,
    CAM_DM_O_F3_ACT,
    CAM_DM_O_F4_ACT,
    CAM_DM_O_F1_L,
    CAM_DM_O_F2_L,
    CAM_DM_O_F3_L,
    CAM_DM_O_F4_L,
    CAM_DM_O_HF_RB,
    CAM_DM_O_HF_GAIN,
    CAM_DM_O_HF_COMP,
    CAM_DM_O_HF_CORIN_TH,
    CAM_DM_O_ACT_LUT,
    //CAM_DM_O_SPARE,
    CAM_DM_O_BB,
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
    //NSL2A
    CAM_NSL2A_CEN,
    CAM_NSL2A_RR_CON0,
    CAM_NSL2A_RR_CON1,
    CAM_NSL2A_GAIN,
    CAM_NSL2A_RZ,
    CAM_NSL2A_XOFF,
    CAM_NSL2A_YOFF,
    CAM_NSL2A_SLP_CON0,
    CAM_NSL2A_SLP_CON1,
    CAM_NSL2A_SIZE,
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
    CAM_ANR_ACTYH,
    CAM_ANR_ACTC,
    CAM_ANR_ACTYL,
    CAM_ANR_YLAD,
    CAM_ANR_PTYL,
    CAM_ANR_LCOEF,
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
    //  NR1
    CAM_NR1_Begin           =   CAM_NR1_CON,
    //  PDC
    CAM_PDC_Begin           =   CAM_BNR_PDC_CON,
    //  LSC
    CAM_LSC_Begin           =   CAM_LSC_CTL1,
    //  RPG
    CAM_RPG_Begin           =   CAM_RPG_SATU_1,
    //  PGN
    CAM_PGN_Begin           =   CAM_PGN_SATU_1,
    //  SL2
    CAM_SL2_Begin           =   CAM_SL2_CEN,
    //  CFA
    CAM_CFA_Begin           =   CAM_DM_O_BYP,
    //  G2G
    CAM_G2G_Begin           =   CAM_G2G_CNV_1,
    //  G2C
    CAM_G2C_Begin           =   CAM_G2C_CONV_0A,
    //  NSL2
    CAM_NSL2A_Begin         =   CAM_NSL2A_CEN,
    //  ANR
    CAM_ANR_Begin           =   CAM_ANR_CON1,
    //  CCR
    CAM_CCR_Begin           =   CAM_CCR_CON,
    //  PCA
    CAM_PCA_Begin           =   CAM_PCA_CON1,
    //  EE
    CAM_EE_Begin            =   CAM_SEEE_SRK_CTRL,
    //  SE
    CAM_SE_Begin            =   CAM_SEEE_OUT_EDGE_CTRL,

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
    u4PCATableInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspGGMTableInfo)
};

};  //  namespace NSIspExifDebug
#endif // _DBG_ISP_PARAM_H_

