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

#define P1_BUFFER_SIZE 1000
#define P2_BUFFER_SIZE 5000
#define MFB_BUFFER_SIZE 60
enum { IspDebugTagVersion = ((2 <<16) | 3)};
#define ISP_DEBUG_TAG_VERSION_DP (IspDebugTagVersion)

enum IspDebugTagID
{
    //BEGIN_OF_EXIF_TAG
    IspTagVersion,
    UniqueKey,
    //  RAWIspCamInfo
    IspProfile_P1,
    isHDR_P1,
    SensorMode_P1,
    SceneIdx_P1,
    ISOIdx_P1,
    UPPER_ISO_IDX_P1,
    LOWER_ISO_IDX_P1,
    UPPER_ZOOM_IDX_P1,
    LOWER_ZOOM_IDX_P1,
    CTIdx_P1,
    UPPER_CT_IDX_P1,
    LOWER_CT_IDX_P1,
    ZoomRatio_x100_P1,
    ShadingIdx_P1,
    Raw_Bit_P1,
    Custom_Interp_P1,
    IspProfile_P2,
    isHDR_P2,
    SensorMode_P2,
    SceneIdx_P2,
    ISOIdx_P2,
    UPPER_ISO_IDX_P2,
    LOWER_ISO_IDX_P2,
    UPPER_ZOOM_IDX_P2,
    LOWER_ZOOM_IDX_P2,
    CTIdx_P2,
    UPPER_CT_IDX_P2,
    LOWER_CT_IDX_P2,
    ZoomRatio_x100_P2,
    ShadingIdx_P2,
    Raw_Bit_P2,
    Custom_Interp_P2,
    //
    // RAWIspCamInfo (only P2)
    // Effect
    EffectMode,
    // UserSelectLevel
    EdgeIdx,
    HueIdx,
    SatIdx,
    BrightIdx,
    ContrastIdx,
    //
    HighQCap,
    SensorDev,
    RawFmt,
    Dbg_Interp,
    //TBD
    CCUEnable,

    //Paramctrl member
    SwnrEncEnableIsoThreshold,

    //AE Info
    AE_INFO_EXP_TIME_P1,
    AE_INFO_AFE_GAIN_P1,
    AE_INFO_ISP_GAIN_P1,
    AE_INFO_REAL_ISO_P1,
    AE_INFO_LV_x10_P1,
    OBC_AE_GAIN,
    RPG_FLARE_GAIN,
    RPG_FLARE_OFST,
    AE_INFO_EXP_TIME_P2,
    AE_INFO_AFE_GAIN_P2,
    AE_INFO_ISP_GAIN_P2,
    AE_INFO_REAL_ISO_P2,
    AE_INFO_LV_x10_P2,
    OBC2_AE_GAIN,
    PGN_FLARE_GAIN,
    PGN_FLARE_OFST,
    //
    //Flash Info
    FLASH_INFO_Enable_P1,
    FLASH_INFO_Enable_P2,
    //
    //AWB_INFO
    AWB_INFO_CURRENT_GAIN_R_P1,
    AWB_INFO_CURRENT_GAIN_G_P1,
    AWB_INFO_CURRENT_GAIN_B_P1,
    AWB_INFO_CT_P1,
    AWB_INFO_CURRENT_GAIN_R_P2,
    AWB_INFO_CURRENT_GAIN_G_P2,
    AWB_INFO_CURRENT_GAIN_B_P2,
    AWB_INFO_CT_P2,
    //  P1 Index
    IDX_SL2F,
    IDX_DBS,
    IDX_DBS_UPPER,
    IDX_DBS_LOWER,
    IDX_OBC,
    IDX_OBC_UPPER,
    IDX_OBC_LOWER,
    IDX_BPC,
    IDX_BPC_UPPER,
    IDX_BPC_LOWER,
    IDX_NR1,
    IDX_NR1_UPPER,
    IDX_NR1_LOWER,
    IDX_PDC,
    IDX_PDC_UPPER,
    IDX_PDC_LOWER,
    //IDX_RMM,
    //IDX_RMM_UPPER,
    //IDX_RMM_LOWER,
    //RMM_SW_IDX,
    // P2 Index
    IDX_SL2G,
    IDX_DBS2,
    IDX_DBS2_UPPER,
    IDX_DBS2_LOWER,
    IDX_OBC2,
    IDX_OBC2_UPPER,
    IDX_OBC2_LOWER,
    IDX_BPC2,
    IDX_BPC2_UPPER,
    IDX_BPC2_LOWER,
    IDX_NR12,
    IDX_NR12_UPPER,
    IDX_NR12_LOWER,
    IDX_PDC2,
    IDX_PDC2_UPPER,
    IDX_PDC2_LOWER,
    //IDX_RMM2,
    //IDX_RMM2_UPPER,
    //IDX_RMM2_LOWER,
    RMM2_SW_IDX,
    //
    IDX_RNR,
    IDX_RNR_UPPER,
    IDX_RNR_LOWER,
    IDX_SL2,
    IDX_UDM,
    IDX_UDM_UPPER_ISO_UPPER_ZOOM,
    IDX_UDM_LOWER_ISO_UPPER_ZOOM,
    IDX_UDM_UPPER_ISO_LOWER_ZOOM,
    IDX_UDM_LOWER_ISO_LOWER_ZOOM,
    //
    IDX_ANR,
   IDX_ANR_UPPER_ISO_UPPER_ZOOM,
   IDX_ANR_LOWER_ISO_UPPER_ZOOM,
   IDX_ANR_UPPER_ISO_LOWER_ZOOM,
   IDX_ANR_LOWER_ISO_LOWER_ZOOM,
    IDX_ANR2,
   IDX_ANR2_UPPER_ISO_UPPER_ZOOM,
   IDX_ANR2_LOWER_ISO_UPPER_ZOOM,
   IDX_ANR2_UPPER_ISO_LOWER_ZOOM,
   IDX_ANR2_LOWER_ISO_LOWER_ZOOM,
    IDX_CCR,
    IDX_CCR_UPPER,
    IDX_CCR_LOWER,
    //IDX_BOK,
    IDX_HFG,
    IDX_HFG_UPPER_ISO_UPPER_ZOOM,
    IDX_HFG_LOWER_ISO_UPPER_ZOOM,
    IDX_HFG_UPPER_ISO_LOWER_ZOOM,
    IDX_HFG_LOWER_ISO_LOWER_ZOOM,
    IDX_EE,
    IDX_EE_UPPER_ISO_UPPER_ZOOM,
    IDX_EE_LOWER_ISO_UPPER_ZOOM,
    IDX_EE_UPPER_ISO_LOWER_ZOOM,
    IDX_EE_LOWER_ISO_LOWER_ZOOM,
    IDX_NR3D,
   IDX_NR3D_UPPER_ISO_UPPER_ZOOM,
   IDX_NR3D_LOWER_ISO_UPPER_ZOOM,
   IDX_NR3D_UPPER_ISO_LOWER_ZOOM,
   IDX_NR3D_LOWER_ISO_LOWER_ZOOM,
    IDX_MFB,
    IDX_MIX3,
    //
    // CCM Weight
    CCM_Method,
    DYNAMIC_CCM_IDX_P1,
    DYNAMIC_CCM_IDX_P2,
    SMOOTH_CCM_Ver,
    SMOOTH_CCM_PQType,
    SMOOTH_CCM_LV_NUM,
    SMOOTH_CCM_FLASH_NUM,
    SMOOTH_CCM_CT_NUM,
    //
    COLOR_Method,
    SMOOTH_PCA_Ver,
    SMOOTH_COLOR_Ver,
    //
    LCS_IN_Width,
    LCS_IN_Height,
    LCS_OUT_Width,
    LCS_OUT_Height,
    LCS_CROP_X,
    LCS_CROP_Y,
    LCS_LRZR_1,
    LCS_LRZR_2,
    FrontBin_ByLCS,
    QBin_Rto_ByLCS,
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
    DIP_X_LCE_LCE_FD_Enable,
    DIP_X_LCE_FaceNum,
    DIP_X_LCE_LCSXLow,
    DIP_X_LCE_LCSXHi,
    DIP_X_LCE_LCSYLow,
    DIP_X_LCE_LCSYHi,
    DIP_X_LCE_FaceLoBound,
    DIP_X_LCE_FaceHiBound,
    DIP_X_LCE_ctrlPoint_f0,
    DIP_X_LCE_ctrlPoint_f1,
    DIP_X_LCE_FDY,
    DIP_X_LCE_MeterFDTarget,
    DIP_X_LCE_FDProb,
    DIP_X_LCE_AEGain,
    DIP_X_LCE_KeepBrightEnable,
    DIP_X_LCE_currBrightSlope,
    DIP_X_LCE_Ori_P1,
    DIP_X_LCE_Ori_P50,
    DIP_X_LCE_Ori_P500,
    DIP_X_LCE_Ori_P950,
    DIP_X_LCE_Ori_P999,
    DIP_X_LCE_Ori_O1,
    DIP_X_LCE_Ori_O50,
    DIP_X_LCE_Ori_O500,
    DIP_X_LCE_Ori_O950,
    DIP_X_LCE_Ori_O999,
    DIP_X_LCE_Face_P1,
    DIP_X_LCE_Face_P50,
    DIP_X_LCE_Face_P500,
    DIP_X_LCE_Face_P950,
    DIP_X_LCE_Face_P999,
    DIP_X_LCE_Face_O1,
    DIP_X_LCE_Face_O50,
    DIP_X_LCE_Face_O500,
    DIP_X_LCE_Face_O950,
    DIP_X_LCE_Face_O999,
    DIP_X_LCE_SmoothEnable,
    DIP_X_LCE_SmoothSpeed,
    DIP_X_LCE_SmoothWaitAE,
    DIP_X_LCE_FlareEnable,
    DIP_X_LCE_FlareOffset,

    FACE_NUM,
    FACE_1_1,
    FACE_1_2,
    FACE_1_3,
    FACE_1_4,
    FACE_1_5,

    FACE_2_1,
    FACE_2_2,
    FACE_2_3,
    FACE_2_4,
    FACE_2_5,

    FACE_3_1,
    FACE_3_2,
    FACE_3_3,
    FACE_3_4,
    FACE_3_5,

    FACE_4_1,
    FACE_4_2,
    FACE_4_3,
    FACE_4_4,
    FACE_4_5,

    FACE_5_1,
    FACE_5_2,
    FACE_5_3,
    FACE_5_4,
    FACE_5_5,


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
    // END_OF_EXIF_TAG
    //
};

enum
{    //adaptive Gamma
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
        MUINT32     u4P1RegInfoOffset;
        MUINT32     u4P2RegInfoOffset;
        MUINT32     u4MFBRegInfoOffset;
        //MUINT32     u4GmaInfoOffset;
    }   hdr;

    struct IspDebugInfo
    {
        IspDebugTag     tags[TagID_Total_Num];
    } debugInfo;

    struct P1RegInfo
    {
        MUINT32         u4TableSize;
        MUINT32         u4HwVersion;
        MUINT32         regData[P1_BUFFER_SIZE];
    } P1RegInfo;

    struct P2RegInfo
    {
        MUINT32         u4TableSize;
        MUINT32         u4HwVersion;
        MUINT32         regDataP2[P2_BUFFER_SIZE];
    } P2RegInfo;

    struct MFBRegInfo
    {
        MUINT32         u4TableSize;
        MUINT32         u4HwVersion;
        MUINT32         regDataMFB[MFB_BUFFER_SIZE];
    } MFBRegInfo;

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

        MINT32 i4LCE_FD_Enable;
        MINT32 i4FaceNum;
        MINT32 i4LCSXLow;
        MINT32 i4LCSXHi;
        MINT32 i4LCSYLow;
        MINT32 i4LCSYHi;
        MINT32 i4FaceLoBound;
        MINT32 i4FaceHiBound;
        MINT32 i4ctrlPoint_f0;
        MINT32 i4ctrlPoint_f1;
        MINT32 i4FDY;
        MINT32 i4MeterFDTarget;
        MINT32 i4FDProb;
        MINT32 i4AEGain;
        MINT32 i4KeepBrightEnable;
        MINT32 i4currBrightSlope;
        MINT32 i4OriP1;
        MINT32 i4OriP50;
        MINT32 i4OriP500;
        MINT32 i4OriP950;
        MINT32 i4OriP999;
        MINT32 i4OriO1;
        MINT32 i4OriO50;
        MINT32 i4OriO500;
        MINT32 i4OriO950;
        MINT32 i4OriO999;
        MINT32 i4FaceP1;
        MINT32 i4FaceP50;
        MINT32 i4FaceP500;
        MINT32 i4FaceP950;
        MINT32 i4FaceP999;
        MINT32 i4FaceO1;
        MINT32 i4FaceO50;
        MINT32 i4FaceO500;
        MINT32 i4FaceO950;
        MINT32 i4FaceO999;

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
    .u4KeyID=            ISP_DEBUG_KEYID,
    .u4ModuleCount=      ModuleNum<3, 1>::val,
    .u4DebugInfoOffset=  sizeof(NSIspExifDebug::IspExifDebugInfo::Header),
    .u4P1RegInfoOffset=  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo),
    .u4P2RegInfoOffset=  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::P1RegInfo),
    .u4MFBRegInfoOffset=  sizeof(NSIspExifDebug::IspExifDebugInfo::Header) + sizeof(NSIspExifDebug::IspExifDebugInfo::IspDebugInfo) + sizeof(NSIspExifDebug::IspExifDebugInfo::P1RegInfo)
                          + sizeof(NSIspExifDebug::IspExifDebugInfo::P2RegInfo)
};

};  //  namespace NSIspExifDebug
#endif // _DBG_ISP_PARAM_H_

