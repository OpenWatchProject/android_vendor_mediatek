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
#include <isp_tuning.h>
#include <awb_param.h>
#include "aaa/ae_param.h"
//#include <af_param.h>
#include <flash_param.h>
#include <camera_custom_nvram.h>
#include <vector>
#include <string>
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
    ISP_INT_ISO_T sNr1;
    ISP_INT_ISO_T sPdc;
    ISP_INT_ISO_T sRmm;
    ISP_INT_ISO_T sDbs2;
    ISP_INT_ISO_T sAdbs2;
    ISP_INT_ISO_T sObc2;
    ISP_INT_ISO_T sBpc2;
    ISP_INT_ISO_T sNr12;
    ISP_INT_ISO_T sPdc2;
    ISP_INT_ISO_T sRmm2;
    ISP_INT_ISO_T sRnr;
    ISP_INT_ISO_T sUdm;
    ISP_INT_ISO_T sAnr;
    ISP_INT_ISO_T sAnr2;
    ISP_INT_ISO_T sAbf;
    ISP_INT_ISO_T sCcr;
    ISP_INT_ISO_T sHfg;
    ISP_INT_ISO_T sEe;
    ISP_INT_ISO_T sNr3d;
    ISP_INT_ISO_T sMix3;
    ISP_INT_ISO_T sMfb;
    ISP_INT_ISO_T sSwAnrLtm;
    ISP_INT_LV_CT_T sCCM;
    ISP_INT_LV_CT_T sCCM2;
    ISP_INT_LV_CT_T sColor;
    ISP_INT_LV_CT_T sFD_ANR;
    ISP_INT_LV_CT_T NBC_TBL;
    MUINT16 u2LCE;
    MUINT16 u2GGM;
    MUINT16 u2SL2F;
} ISP_INT_INFO_T;

typedef struct {
    //pass algo
    MUINT16 sCcm_tbl_P1_Idx;
    MUINT16 sCcm_tbl_P2_Idx;
} ISP_DYNAMIC_INFO_T;

typedef struct {
    MUINT16 sCCM;
    MUINT16 sPCA;
    MUINT16 sCOLOR;
} ISP_SWALGO_VER_T;
// SL2 info
typedef struct {
    MINT32 i4CenterX;
    MINT32 i4CenterY;
    MINT32 i4R0;
    MINT32 i4R1;
    MINT32 i4R2;
} SL2_INFO_T;
// Crop Resize info
typedef struct {
    MINT32 i4TGoutW; //i4FullW;
    MINT32 i4TGoutH; //i4FullH;
    MBOOL  fgFBinOnOff;
    MINT32 i4RRZinW;
    MINT32 i4RRZinH;
    MBOOL  fgRRZOnOff; //fgOnOff;
    MINT32 i4RRZofstX; //i4OfstX;
    MINT32 i4RRZofstY; //i4OfstY;
    MINT32 i4RRZcropW; //i4Width;
    MINT32 i4RRZcropH; //i4Height;
    MINT32 i4RRZoutW; //i4RzWidth;
    MINT32 i4RRZoutH; //i4RzHeight;
} CROP_RZ_INFO_T;

typedef struct {
    ISP_NVRAM_LCS_T lcs;
    MBOOL  fgOnOff;
    MUINT32 i4FrmId;
} ISP_LCS_IN_INFO_T;

typedef struct{
    MUINT32 u4InWidth;
    MUINT32 u4InHeight;
    MUINT32 u4OutWidth;
    MUINT32 u4OutHeight;
    MUINT32 u4CropX;
    MUINT32 u4CropY;
    MUINT32 u4Lrzr1;
    MUINT32 u4Lrzr2;
    MUINT32 i4FrmId;
    MBOOL bFrontBin;
    MUINT32 u4Qbin_Rto;
}ISP_LCS_OUT_INFO_T;

typedef struct{
    ISP_LCS_IN_INFO_T rInSetting;
    ISP_LCS_OUT_INFO_T rOutSetting;
}ISP_LCS_INFO_T;
//CCT info (pass1 to pass2)
typedef union {
    struct {
    FIELD fgSL2F    : 1;
    FIELD fgDBS     : 1;
    FIELD fgOBC     : 1;
    FIELD fgBPC     : 1;
    FIELD fgNR1     : 1;
    FIELD fgPDC     : 1;
    FIELD fgRMM     : 1;
    FIELD rsv       : 25;
    } Bits;
    MUINT32 u4Val;
} ISP_CCT_EN_INFO_T;

typedef struct{
    //MBOOL fgBIN;
    MUINT32 u4RAW_Width;
    MUINT32 u4RAW_Height;
} ISP_RAW_SIZE_T;

typedef union _CAM_IDX_QRY_COMB_
{
    struct {
        EIspProfile_T eIspProfile;
        ESensorMode_T eSensorMode;
        EFrontBin_T eFrontBin;
        EP2Size_T eP2size;
        EFlash_T eFlash;
        EApp_T eApp;
        EFaceDetection_T eFaceDetection;
        ELensID_T eLensID;
        EDriverIC_T eDriverIC;
        ECustom_T eCustom;
        EZoom_T eZoom_Idx;
        ELV_T eLV_Idx;
        ECT_T eCT_Idx;
        EISO_T eISO_Idx;
    };
    MUINT32 query[EDim_NUM];
    _CAM_IDX_QRY_COMB_() { memset(query, 0, sizeof(query));}
} CAM_IDX_QRY_COMB;
/*******************************************************************************
*
*******************************************************************************/
struct IspCamInfo
{
public:
    MUINT32             u4Id;        // id for info
    //EIspProfile_T       eIspProfile; // ISP profile.
    //ESensorMode_T       eSensorMode; // sensor mode
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
    //EIndex_ISO_T         eIdx_ISO;         // ISO index
    EISO_T         eIdx_ISO_U;         // Upper ISO index
    EISO_T         eIdx_ISO_L;         // Lower ISO index
    //EIndex_PCA_LUT_CT_T  eIdx_CT;
    ECT_T  eIdx_CT_U;
    ECT_T  eIdx_CT_L;
    ELV_T                eIdx_LV_U;
    ELV_T                eIdx_LV_L;
    EIndex_Shading_CCT_T eIdx_Shading_CCT; // Index for Shading.
    //EIndex_ZOOM_T        eIdx_Zoom;            // ISO index
    EZoom_T        eIdx_Zoom_U;         // Upper ISO index
    EZoom_T        eIdx_Zoom_L;         // Lower ISO index
    AWB_INFO_T           rAWBInfo;         // AWB info for ISP tuning
    AE_INFO_T            rAEInfo;          // AE info for ISP tuning
    MBOOL                rManualAE;
//    AF_INFO_T            rAFInfo;          // AF info for ISP tuning
    FLASH_INFO_T         rFlashInfo;       // Flash info for ISP tuning
    MINT32               i4ZoomRatio_x100; // Zoom ratio x 100
    MINT32               i4PrevZoomRatio_x100;//previous
    CROP_RZ_INFO_T       rCropRzInfo;      // crop and resize info
    MBOOL                fgRPGEnable;      // RPG enable
    MBOOL                fgBlackMute;      // force black mute
    MINT32               i4RawType;        // "IMGO" raw type
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;
    ISP_NVRAM_CCM_T rMtkCCM;
    ISP_NVRAM_RPG_T rRPG;
    ISP_NVRAM_OBC_T rOBC1;
//    ISP_CCM_T rColorCorrectionTransform;
    mtk_camera_metadata_enum_android_edge_mode_t eEdgeMode;
    mtk_camera_metadata_enum_android_noise_reduction_mode_t eNRMode;
    mtk_camera_metadata_enum_android_tonemap_mode_t eToneMapMode;
    MBOOL                fgRWBSensor; //only for Hal1
    ISP_CCT_EN_INFO_T    fgCCTInfo;
    SL2_INFO_T           rSl2Info;          // SL2 info in TG coordinate
    ISP_LCS_INFO_T       rLCS_Info;
    MBOOL                fgNeedKeepP1;
    ISP_RAW_SIZE_T       RawSize;
    MBOOL                fgFDEnable;
    float                FDRatio;
    MBOOL                bBypassLCE;
    MBOOL                bBypassNR;
    MINT32               i4P2InImgFmt;
    MUINT8               u1P2TuningUpdate;
    ISP_NR3D_META_INFO_T  NR3D_Data;
    MBOOL                fgHDR;
    MBOOL                fgDbsEn;
    mtk_camera_metadata_enum_android_control_mode_t  eControlMode;
public:
    RAWIspCamInfo()
        : IspCamInfo()
        //, eIdx_ISO(eIDX_ISO_0)
        , eIdx_ISO_U(EISO_IDX_00)
        , eIdx_ISO_L(EISO_IDX_00)
        //, eIdx_CT(eIDX_PCA_CT_00)
        , eIdx_CT_U(ECT_IDX_00)
        , eIdx_CT_L(ECT_IDX_00)
        , eIdx_LV_U(ELV_IDX_00)
        , eIdx_LV_L(ELV_IDX_00)
        , eIdx_Shading_CCT(eIDX_Shading_CCT_CWF)
        //, eIdx_Zoom(eIDX_ZOOM_OFF)
        , eIdx_Zoom_U(EZoom_IDX_00)
        , eIdx_Zoom_L(EZoom_IDX_00)
        , rAWBInfo()
        , rAEInfo()
        , rManualAE(MFALSE)
//        , rAFInfo()
        , rFlashInfo()
        , i4ZoomRatio_x100(100)
        , i4PrevZoomRatio_x100(100)
        , fgRPGEnable(MFALSE)
        , fgBlackMute(MFALSE)
        , i4RawType(NSIspTuning::ERawType_Proc)
        , eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
        , rMtkCCM()
        , rRPG()
        , rOBC1()
//        , rColorCorrectionTransform()
        , eEdgeMode(MTK_EDGE_MODE_FAST)
        , eNRMode(MTK_NOISE_REDUCTION_MODE_FAST)
        , eToneMapMode(MTK_TONEMAP_MODE_FAST)
        , fgRWBSensor(MFALSE)
        , fgCCTInfo()
        , rSl2Info()
        , fgNeedKeepP1(MFALSE)
        , RawSize()
        , fgFDEnable(MFALSE)
        , FDRatio(0)
        , bBypassLCE(MFALSE)
        , bBypassNR(MFALSE)
        , i4P2InImgFmt(0)
        , u1P2TuningUpdate(0)
        , fgHDR(MFALSE)
        , fgDbsEn(MFALSE)
        , eControlMode(MTK_CONTROL_MODE_OFF)
    {
        memset(&rLCS_Info, 0, sizeof(ISP_LCS_INFO_T));
        memset(&NR3D_Data, 0, sizeof(ISP_NR3D_META_INFO_T));
        memset(&rCropRzInfo, 0, sizeof(CROP_RZ_INFO_T));
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
    AWB_INFO_T              rAWBInfo; \
    AE_INFO_T               rAEInfo; \
    FLASH_INFO_T            rFlashInfo; \
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

