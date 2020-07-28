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
#ifndef _ISP_TUNING_CAM_INFO_H_
#define _ISP_TUNING_CAM_INFO_H_
#include <aaa_log.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam3/feature/3dnr/3dnr_isp_defs.h>
#include <isp_tuning.h>
#include <camera_custom_nvram.h>
#include <camera_custom_isp_nvram.h>
#include <vector>
#include <string>
#include <tuning/ae_flow_custom.h>
#include <tuning/awb_flow_custom.h>
#include "mtkcam/def/UITypes.h"

namespace NSIspTuning
{
/*******************************************************************************
*
*******************************************************************************/
//  Scene index
typedef mtk_camera_metadata_enum_android_control_scene_mode_t EIndex_Scene_T;
enum { eNUM_OF_SCENE_IDX = MTK_CONTROL_SCENE_MODE_NUM };
//  Color Effect Index
typedef mtk_camera_metadata_enum_android_control_effect_mode_t EIndex_Effect_T;
//  ISP End-User-Define Tuning Index:
//  Edge, Hue, Saturation, Brightness, Contrast
typedef mtk_camera_metadata_enum_android_control_isp_edge_t        EIndex_Isp_Edge_T;
typedef mtk_camera_metadata_enum_android_control_isp_hue_t         EIndex_Isp_Hue_T;
typedef mtk_camera_metadata_enum_android_control_isp_saturation_t  EIndex_Isp_Saturation_T;
typedef mtk_camera_metadata_enum_android_control_isp_brightness_t  EIndex_Isp_Brightness_T;
typedef mtk_camera_metadata_enum_android_control_isp_contrast_t    EIndex_Isp_Contrast_T;
typedef struct IspUsrSelectLevel
{
    EIndex_Isp_Edge_T           eIdx_Edge;
    EIndex_Isp_Hue_T            eIdx_Hue;
    EIndex_Isp_Saturation_T     eIdx_Sat;
    EIndex_Isp_Brightness_T     eIdx_Bright;
    EIndex_Isp_Contrast_T       eIdx_Contrast;
/*
    IspUsrSelectLevel()
        : eIdx_Edge     (MTK_CONTROL_ISP_EDGE_MIDDLE)
        , eIdx_Hue      (MTK_CONTROL_ISP_HUE_MIDDLE)
        , eIdx_Sat      (MTK_CONTROL_ISP_SATURATION_MIDDLE)
        , eIdx_Bright   (MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE)
        , eIdx_Contrast (MTK_CONTROL_ISP_CONTRAST_MIDDLE)
    {}
*/
} IspUsrSelectLevel_T;
/*
typedef enum EIndex_INT_SCENARIO
{
    eIDX_UpperIso_UpperZoom  = 0,
    eIDX_LowerIso_UpperZoom,
    eIDX_UpperIso_LowerZoom,
    eIDX_LowerIso_LowerZoom,
    eIDX_LowerLv_LowerCt,
    eIDX_LowerLv_UpperCt,
    eIDX_UpperLv_LowerCt,
    eIDX_UpperLv_UpperCt,
    eIDX_LowerCT,
    eIDX_UpperCT,
    eIDX_LowerLV,
    eIDX_UpperLV,
    eIDX_LowerIso,
    eIDX_UpperIso,
    eIDX_ISP_Scenario_Num
} EIndex_INT_SCENARIO_T;
*/
typedef enum EIndex_CCM
{
    eIDX_CCM_D65  = 0,
    eIDX_CCM_TL84,
    eIDX_CCM_CWF,
    eIDX_CCM_A,
    eIDX_CCM_NUM
} EIndex_CCM_T;
// COLOR LUT CT index
typedef enum EIndex_PCA_LUT_CT
{
    eIDX_PCA_CT_00  = 0,
    eIDX_PCA_CT_01,
    eIDX_PCA_CT_02,
    eIDX_PCA_CT_03,
    eIDX_PCA_CT_04
} EIndex_PCA_LUT_CT_T;
// COLOR LUT CT index
typedef enum EIndex_PDC_TBL
{
    eIDX_PDE  = 0,
    eIDX_PDC_FULL,
    eIDX_PDC_BIN
} EIndex_PDC_TBL_T;
//  Correlated color temperature index for shading.
typedef enum EIndex_Shading_CCT
{
    eIDX_Shading_CCT_BEGIN  = 0,
    eIDX_Shading_CCT_ALight   = eIDX_Shading_CCT_BEGIN,
    eIDX_Shading_CCT_CWF,
    eIDX_Shading_CCT_D65,
    eIDX_Shading_CCT_RSVD
} EIndex_Shading_CCT_T;
// ISP interpolation info
typedef struct {
    MUINT16 u2UpperIso_UpperZoom;
    MUINT16 u2LowerIso_UpperZoom;
    MUINT16 u2UpperIso_LowerZoom;
    MUINT16 u2LowerIso_LowerZoom;
}ISP_INT_ISO_T;

typedef struct {
    MUINT16 u2LowerLv_UpperCt;
    MUINT16 u2LowerLv_LowerCt;
    MUINT16 u2UpperLv_UpperCt;
    MUINT16 u2UpperLv_LowerCt;
    MUINT16 u2ParamIdx;
}ISP_INT_LV_CT_T;

typedef struct {
    ISP_INT_ISO_T sDbs;
    ISP_INT_ISO_T sAdbs;
    ISP_INT_ISO_T sObc;
    ISP_INT_ISO_T sBpc;
    ISP_INT_ISO_T sCt1;
    ISP_INT_ISO_T sPdc;
    ISP_INT_ISO_T sDbs2;
    ISP_INT_ISO_T sAdbs2;
    ISP_INT_ISO_T sObc2;
    ISP_INT_ISO_T sBpc2;
    ISP_INT_ISO_T sCt2;
    ISP_INT_ISO_T sPdc2;
    ISP_INT_ISO_T sRmm2;
    ISP_INT_ISO_T sRnr;
    ISP_INT_ISO_T sDm;
    ISP_INT_ISO_T sYnr;
    ISP_INT_ISO_T sAbf;
    ISP_INT_ISO_T sCcr;
    ISP_INT_ISO_T sEe;
    ISP_INT_ISO_T sNr3d;
    ISP_INT_ISO_T sMix3;
    ISP_INT_ISO_T sMfb;
    ISP_INT_ISO_T sSwYnrLtm;
    ISP_INT_LV_CT_T sCCM;
    ISP_INT_LV_CT_T sCCM2;
    ISP_INT_LV_CT_T sColor;
    ISP_INT_ISO_T sFD_YNR;
    ISP_INT_ISO_T YNR_TBL;
    MUINT16 u2LCE;
    MUINT16 u2GGM;
    MUINT16 u2SL2F;
} ISP_INT_INFO_T;

typedef struct {
    MUINT16 sCCM;
    MUINT16 sPCA;
    MUINT16 sCOLOR;
} ISP_SWALGO_VER_T;
// SL2 info
typedef struct {
    MBOOL  bLSC_R1_En;
    MINT32 i4CenterX;
    MINT32 i4CenterY;
    MINT32 i4R0;
    MINT32 i4R1;
    MINT32 i4R2;
    MINT32 i4Gain0;
    MINT32 i4Gain1;
    MINT32 i4Gain2;
    MINT32 i4Gain3;
    MINT32 i4Gain4;
    MINT32 i4SetZero;
} SL2_INFO_T;
// Crop Resize info
typedef struct {
    NSCam::MSize sTGout;
    MBOOL  fgFBinOnOff;
    NSCam::MSize sRRZin;
    MBOOL  fgRRZOnOff;
    NSCam::MRect rRRZcrop;
    NSCam::MSize sRRZout;
} CROP_RZ_INFO_T;

typedef struct{
    MUINT32 u4InWidth;
    MUINT32 u4InHeight;
    MUINT32 u4OutWidth;
    MUINT32 u4OutHeight;
    MUINT32 u4LrzrX;
    MUINT32 u4LrzrY;
    MUINT32 i4FrmId;
    MBOOL   bFrontBin;
    MUINT32 u4Qbin_Rto;
    MBOOL   bLCESEnable;
}ISP_LCS_OUT_INFO_T;

typedef enum CCU_Valid
{
    CCU_UNVALID = 0,
    CCU_VALID_PRESENT,
    CCU_VALID_LAST
}CCU_Valid_T;

typedef struct{
    CCU_Valid_T LTM_Valid;
    MUINT32   CLIP_TH;
    ISP_NVRAM_LTM_CURVE_T  LTM_Curve;
    ISP_NVRAM_LTM_T P1_LTM_Reg[2];
    ISP_NVRAM_LTMS_T P1_LTMS_Reg;
    MUINT32 LTMSO[2400];  // rLTMInfo.ltmso_size / 4, form CCU define
    MBOOL LTM_EN;
} ISP_CCU_LTM_RESULT_T;

typedef struct{
    CCU_Valid_T HLR_Valid;
    ISP_NVRAM_HLR_T HLR_Data;
    MBOOL HLR_EN;
} ISP_CCU_HLR_RESULT_T;

typedef struct{
    ISP_CCU_LTM_RESULT_T LTM;
    ISP_CCU_HLR_RESULT_T HLR;
} ISP_CCU_RESULT_T;

struct CAMERA_TUNING_FD_INFO_T{
    MINT32 YUVsts[15][5];          // face statistic data, only first five face valid
    MINT8  fld_GenderLabel[15];
    MINT32 fld_GenderInfo[15];
    MINT32 fld_rip[15];
    MINT32 fld_rop[15];
    MINT32 rect[15][4];
	MINT32 GenderNum;
	MINT32 LandmarkNum;
	MINT32 Face_Leye[15][4];
	MINT32 Face_Reye[15][4];
	MINT32 Landmark_CV[15];
    MINT32 FaceNum;
    ISP_NVRAM_GGM_T FaceGGM;
    MINT32 FaceGGM_Idx;
    float FDRatio;

    CAMERA_TUNING_FD_INFO_T()
        : FaceNum(0),
        GenderNum(0),
        LandmarkNum(0),
        FaceGGM_Idx(0),
        FDRatio(0.0f)
    {
        memset(&FaceGGM, 0, sizeof(ISP_NVRAM_GGM_T));
    }
};

typedef union _CAM_IDX_QRY_COMB_
{
    struct {
        EIspProfile_T eIspProfile;
        ESensorMode_T eSensorMode;
        EFrontBin_T eFrontBin;
        ESize_T eSize;
        EFlash_T eFlash;
        EApp_T eApp;
        EFaceDetection_T eFaceDetection;
        EDriverIC_T eDriverIC;
        ECustom_00_T eCustom_00;
        ECustom_00_T eCustom_01;
        EZoom_T eZoom_Idx;
        ELV_T eLV_Idx;
        ECT_T eCT_Idx;
        EISO_T eISO_Idx[NVRAM_ISP_REGS_ISO_GROUP_NUM];
    };
    MUINT32 query[EDim_NUM+NVRAM_ISP_REGS_ISO_GROUP_NUM-1];
    _CAM_IDX_QRY_COMB_() { memset(query, 0, sizeof(query));}
} CAM_IDX_QRY_COMB;

typedef struct _CAM_IDX_QRY_COMB_WITH_ISO_
{
    CAM_IDX_QRY_COMB mapping_info;
    MINT32 i4ISO;
    _CAM_IDX_QRY_COMB_WITH_ISO_()
        : i4ISO(0){}
} CAM_IDX_QRY_COMB_WITH_ISO;

/*******************************************************************************
*
*******************************************************************************/
struct IspCamInfo
{
public:
    MUINT32             u4Id;        // id for info
    EIndex_Scene_T      eIdx_Scene;  // scene mode.
    EIndex_Effect_T     eIdx_Effect; // effect mode
    IspUsrSelectLevel_T rIspUsrSelectLevel; // image property
    CAM_IDX_QRY_COMB    rMapping_Info;
public:
    IspCamInfo()
    : u4Id(0)
    //, eIspProfile(EIspProfile_Preview)
    //, eSensorMode(ESensorMode_Preview)
    , eIdx_Scene(MTK_CONTROL_SCENE_MODE_DISABLED)
    , eIdx_Effect(MTK_CONTROL_EFFECT_MODE_OFF)
    {
        rIspUsrSelectLevel.eIdx_Edge = MTK_CONTROL_ISP_EDGE_MIDDLE;
        rIspUsrSelectLevel.eIdx_Hue = MTK_CONTROL_ISP_HUE_MIDDLE;
        rIspUsrSelectLevel.eIdx_Sat = MTK_CONTROL_ISP_SATURATION_MIDDLE;
        rIspUsrSelectLevel.eIdx_Bright = MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE;
        rIspUsrSelectLevel.eIdx_Contrast = MTK_CONTROL_ISP_CONTRAST_MIDDLE;
        memset(&rMapping_Info, 0, sizeof(CAM_IDX_QRY_COMB));
    }
public:
    void dump() const
    {
        MY_LOG("[IspCamInfo][dump](eIspProfile, eSensorMode, eIdx_Scene, eIdx_Effect)=(%d, %d, %d, %d)", rMapping_Info.eIspProfile, rMapping_Info.eSensorMode, eIdx_Scene, eIdx_Effect);
    }
};
/*******************************************************************************
*
*******************************************************************************/
struct RAWIspCamInfo : public IspCamInfo
{
public:
    EISO_T               eIdx_ISO_U[NVRAM_ISP_REGS_ISO_GROUP_NUM];   ;         // Upper ISO index
    EISO_T               eIdx_ISO_L[NVRAM_ISP_REGS_ISO_GROUP_NUM];   ;         // Lower ISO index
    ECT_T                eIdx_CT_U;
    ECT_T                eIdx_CT_L;
    ELV_T                eIdx_LV_U;
    ELV_T                eIdx_LV_L;
    EIndex_Shading_CCT_T eIdx_Shading_CCT; // Index for Shading.
    AWB_ISP_INFO_T       rAWBInfo;         // AWB info for ISP tuning
    AE_ISP_INFO_T        rAEInfo;          // AE info for ISP tuning
    MINT32               i4ZoomRatio_x100; // Zoom ratio x 100
    CROP_RZ_INFO_T       rCropRzInfo;      // crop and resize info
    MBOOL                fgRPGEnable;      // RPG enable
    ERawType_T           i4RawType;        // "IMGO" raw type
    ISP_NVRAM_CCM_T rMtkCCM;
    MINT32 rOBC_OFST[4];
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;
    mtk_camera_metadata_enum_android_edge_mode_t eEdgeMode;
    mtk_camera_metadata_enum_android_noise_reduction_mode_t eNRMode;
    mtk_camera_metadata_enum_android_tonemap_mode_t eToneMapMode;
    SL2_INFO_T           rSl2Info;          // SL2 info in TG coordinate
    ISP_LCS_OUT_INFO_T   rLCS_Info;
    MBOOL                fgNeedKeepP1;
    MBOOL                fgFDEnable;
    MBOOL                bBypassLCE;
    MBOOL                bBypassNR;
    MBOOL                bEnableNDD;
    EP2IN_FMT_T          i4P2InImgFmt;
    EP2UPDATE_MODE       u1P2TuningUpdate;
    NSCam::NR3D::NR3DIspParam NR3D_Data;
    MBOOL                fgHDR;
    mtk_camera_metadata_enum_android_control_mode_t  eControlMode;
    CAMERA_TUNING_FD_INFO_T rFdInfo;
    ISP_CCU_RESULT_T     rCCU_Result;
    NSCam::MSize targetSize;

public:
    RAWIspCamInfo()
        : IspCamInfo()
        , eIdx_CT_U(ECT_IDX_00)
        , eIdx_CT_L(ECT_IDX_00)
        , eIdx_LV_U(ELV_IDX_00)
        , eIdx_LV_L(ELV_IDX_00)
        , eIdx_Shading_CCT(eIDX_Shading_CCT_CWF)
        , rAWBInfo()
        , rAEInfo()
        , i4ZoomRatio_x100(100)
        , fgRPGEnable(MFALSE)
        , rMtkCCM()
        , eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
        , eEdgeMode(MTK_EDGE_MODE_FAST)
        , eNRMode(MTK_NOISE_REDUCTION_MODE_FAST)
        , eToneMapMode(MTK_TONEMAP_MODE_FAST)
        , rSl2Info()
        , fgNeedKeepP1(MFALSE)
        , fgFDEnable(MFALSE)
        , bBypassLCE(MFALSE)
        , bBypassNR(MFALSE)
        , bEnableNDD(MFALSE)
        , i4P2InImgFmt(ERaw2Yuv)
        , u1P2TuningUpdate(ENormalUpdate)
        , fgHDR(MFALSE)
        , i4RawType(NSIspTuning::ERawType_Pure)
        , eControlMode(MTK_CONTROL_MODE_OFF)
        , targetSize(0,0)
    {
        memset(&rLCS_Info, 0, sizeof(ISP_LCS_OUT_INFO_T));
        memset(&NR3D_Data, 0, sizeof(NSCam::NR3D::NR3DIspParam));
        memset(&rCropRzInfo, 0, sizeof(CROP_RZ_INFO_T));
        memset(&eIdx_ISO_U, 0, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
        memset(&eIdx_ISO_L, 0, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
        memset(&(rOBC_OFST[0]), 0, sizeof(MINT32)* 4);
        memset(&rFdInfo, 0, sizeof(CAMERA_TUNING_FD_INFO_T));
        memset(&rCCU_Result, 0, sizeof(ISP_CCU_RESULT_T));
    }
public:
    void dump() const
    {
        IspCamInfo::dump();
        MY_LOG(
            "[RAWIspCamInfo][dump]"
            "(eIdx_ISO, i4ZoomRatio_x100)"
            //"=(%d, %d)"
            //, eIdx_ISO, i4ZoomRatio_x100
        );
    }
};
#define _RAWIspCamInfo \
struct {\
    MUINT32                 u4Id;\
    EIspProfile_T           eIspProfile;\
    ESensorMode_T           eSensorMode;\
    EIndex_Scene_T          eIdx_Scene;\
    EIndex_Effect_T         eIdx_Effect;\
    IspUsrSelectLevel_T     rIspUsrSelectLevel;\
    EISO_T                  eIdx_ISO;   \
    EIndex_Shading_CCT_T    eIdx_Shading_CCT;\
    AWB_ISP_INFO_T              rAWBInfo; \
    AE_ISP_INFO_T               rAEInfo; \
    MINT32                  i4ZoomRatio_x100;\
    CROP_RZ_INFO_T          rCropRzInfo;\
    MBOOL                   fgRPGEnable;\
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;\
    ISP_NVRAM_CCM_T rMtkCCM;\
    mtk_camera_metadata_enum_android_edge_mode_t eEdgeMode;\
    mtk_camera_metadata_enum_android_noise_reduction_mode_t eNRMode;\
    mtk_camera_metadata_enum_android_tonemap_mode_t eToneMapMode;\
}
struct __RAWIspCamInfo
{
    _RAWIspCamInfo;
};
typedef union
{
    _RAWIspCamInfo;
    MUINT8 data[sizeof(struct __RAWIspCamInfo)];
} RAWIspCamInfo_U;
typedef struct
{
    NSIspTuning::RAWIspCamInfo rCamInfo;
    CROP_RZ_INFO_T rIspP2CropInfo;
    std::vector<MUINT8> rLscData;
    std::vector<MUINT8> rTsfData;
    MINT32 i4UniqueKey;
    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT hint;
    int isCapture;
} ISP_INFO_T;
/*******************************************************************************
*
*******************************************************************************/
struct YUVIspCamInfo : public IspCamInfo
{
public:
    YUVIspCamInfo()
        : IspCamInfo()
    {}
};

// Copy from MTKLCE.h
typedef struct{
    MBOOL bIsValid;   // is data valid flag

    //LCE Result
    MINT32 LCE_TC_P[8];
    MINT32 LCE_TC_O[8];

    MINT32 LCE_MAX_SLOPE_N;
    MINT32 LCE_MIN_SLOPE_N;

}DUAL_LCE_SYNC_INFO_T;

// Copy from MTKGMA.h
typedef struct{
    MBOOL bIsValid;   // is data valid flag

    //GGMD1 Result
    MBOOL bIsFaceGMA;
    MINT32 i4EncGMA[193];

}DUAL_GGM_SYNC_INFO_T;


typedef struct{
    DUAL_LCE_SYNC_INFO_T sLceSyncInfo;
    DUAL_GGM_SYNC_INFO_T sGmaSyncInfo;
}DUAL_ISP_SYNC_INFO_T;


/*******************************************************************************
*
*******************************************************************************/
};  //  NSIspTuning
#endif //  _ISP_TUNING_CAM_INFO_H_

