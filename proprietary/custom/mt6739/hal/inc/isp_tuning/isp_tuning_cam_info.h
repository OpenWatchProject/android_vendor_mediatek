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
#include <ae_param.h>
#include <af_param.h>
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


//  ISO index.
typedef enum EIndex_ISO
{
    eIDX_ISO_100 = 0,
    eIDX_ISO_200,
    eIDX_ISO_400,
    eIDX_ISO_800,
    eIDX_ISO_1200,
    eIDX_ISO_1600,
    eIDX_ISO_2000,
    eIDX_ISO_2400,
    eIDX_ISO_2800,
    eIDX_ISO_3200,
    eIDX_ISO_4000,
    eIDX_ISO_4800,
    eIDX_ISO_5600,
    eIDX_ISO_6400,
    eIDX_ISO_7200,
    eIDX_ISO_8000,
    eIDX_ISO_8800,
    eIDX_ISO_9600,
    eIDX_ISO_11200,
    eIDX_ISO_12800,
    eNUM_OF_ISO_IDX
} EIndex_ISO_T;

typedef enum EIndex_ZOOM
{
    eIDX_ZOOM_0 =0,
    eIDX_ZOOM_1,
    eIDX_ZOOM_2,
    eNUM_OF_ZOOM_IDX
} EIndex_ZOOM_T;

// CCM index
typedef enum EIndex_CCM
{
    eIDX_CCM_D65  = 0,
    eIDX_CCM_TL84,
    eIDX_CCM_CWF,
    eIDX_CCM_A,
    eIDX_CCM_NUM
} EIndex_CCM_T;

// PCA LUT index
typedef enum EIndex_PCA_LUT
{
    eIDX_PCA_LOW  = 0,
    eIDX_PCA_MIDDLE,
    eIDX_PCA_HIGH,
    eIDX_PCA_LOW_2,
    eIDX_PCA_MIDDLE_2,
    eIDX_PCA_HIGH_2
} EIndex_PCA_LUT_T;

// COLOR LUT CT index
typedef enum EIndex_PCA_LUT_CT
{
    eIDX_PCA_CT_00  = 0,
    eIDX_PCA_CT_01,
    eIDX_PCA_CT_02,
    eIDX_PCA_CT_03,
    eIDX_PCA_CT_04

} EIndex_PCA_LUT_CT_T;


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
    MUINT32 u4DbsUpperISO;
    MUINT32 u4DbsLowerISO;
    MUINT16 u2DbsUpperIdx;
    MUINT16 u2DbsLowerIdx;
    MUINT32 u4ObcUpperISO;
    MUINT32 u4ObcLowerISO;
    MUINT16 u2ObcUpperIdx;
    MUINT16 u2ObcLowerIdx;
    MUINT32 u4BpcUpperISO;
    MUINT32 u4BpcLowerISO;
    MUINT16 u2BpcUpperIdx;
    MUINT16 u2BpcLowerIdx;
    MUINT32 u4Nr1UpperISO;
    MUINT32 u4Nr1LowerISO;
    MUINT16 u2Nr1UpperIdx;
    MUINT16 u2Nr1LowerIdx;
    MUINT32 u4PdcUpperISO;
    MUINT32 u4PdcLowerISO;
    MUINT16 u2PdcUpperIdx;
    MUINT16 u2PdcLowerIdx;
    MUINT32 u4CfaUpperISO;
    MUINT32 u4CfaLowerISO;
    MUINT16 u2CfaUpperIdx;
    MUINT16 u2CfaLowerIdx;
    MUINT32 u4AnrUpperISO;
    MUINT32 u4AnrLowerISO;
    MUINT16 u2AnrUpperIdx;
    MUINT16 u2AnrLowerIdx;
    MUINT32 u4EeUpperISO;
    MUINT32 u4EeLowerISO;
    MUINT16 u2EeUpperIdx;
    MUINT16 u2EeLowerIdx;
} ISP_INT_INFO_T;

typedef struct {
    //pass algo
    MUINT16 sCcm_tbl_P1_Idx;
    MUINT16 sCcm_tbl_P2_Idx;
} ISP_DYNAMIC_INFO_T;

// Crop Resize info
typedef struct {
    MINT32 i4FullW;
    MINT32 i4FullH;
    MBOOL  fgFBinOnOff;
    MINT32 i4RRZinW;
    MINT32 i4RRZinH;
    MINT32 i4OfstX;
    MINT32 i4OfstY;
    MINT32 i4Width;
    MINT32 i4Height;
    MINT32 i4RzWidth;
    MINT32 i4RzHeight;
    MBOOL  fgOnOff;
} CROP_RZ_INFO_T;

typedef struct {
    MBOOL   bEnable;
    MUINT32 OfstX;
    MUINT32 OfstY;
    MFLOAT  RtoW;
    MFLOAT  RtoH;
} ISP_RRZ_INFO_T;

//  ANR_TBL CT index
typedef enum EIndex_ANR_TBL_CT
{
    eIDX_ANR_TBL_CCT_0   = 0,
    eIDX_ANR_TBL_CCT_1
} EIndex_ANR_TBL_CT_T;

typedef enum EIndex_ANR_TBL_ISO
{
    eIDX_ANR_TBL_ISO_0   = 0,
    eIDX_ANR_TBL_ISO_1,
    eIDX_ANR_TBL_ISO_2,
    eIDX_ANR_TBL_ISO_3,
    eIDX_ANR_TBL_ISO_4,
    eIDX_ANR_TBL_ISO_5
} EIndex_ANR_TBL_ISO_T;


// COLOR LUT CT index
typedef enum EIndex_PDC_TBL
{
    eIDX_PDC_FULL  = 0,
    eIDX_PDC_RSV,

} EIndex_PDC_TBL_T;

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
} ISP_LCS_OUT_INFO_T;

typedef struct{
    MBOOL fgBIN;
    MUINT32 u4BIN_Width;
    MUINT32 u4BIN_Height;
} ISP_BIN_INFO_T;

#define Comm_EFlash_No 0
#define Comm_EFlash_MainFlash 1
#define Comm_EFlash_PreFlash 2
#define Comm_EFlash_Torch 3

typedef enum
{
    EFlash_No = Comm_EFlash_No,
    EFlash_MainFlash = Comm_EFlash_MainFlash,
    EFlash_PreFlash = Comm_EFlash_PreFlash,
    EFlash_Torch = Comm_EFlash_Torch,
    EFlash_NUM,
} EFlash_T;

typedef struct
{
    MINT32 flashMode; //LIB3A_FLASH_MODE_AUTO, LIB3A_FLASH_MODE_ON, LIB3A_FLASH_MODE_OFF,
    EFlash_T  isFlash; //0: no flash, 1: image with flash

} FLASH_INFO_T;

/*******************************************************************************
*
*******************************************************************************/
struct IspCamInfo
{
public:
    MUINT32             u4Id;        // id for info
    EIspProfile_T       eIspProfile; // ISP profile.
    ESensorMode_T       eSensorMode; // sensor mode
    EIndex_Scene_T      eIdx_Scene;  // scene mode.
    EIndex_Effect_T     eIdx_Effect; // effect mode
    IspUsrSelectLevel_T rIspUsrSelectLevel; // image property

public:
    IspCamInfo()
    : u4Id(0)
    , eIspProfile(EIspProfile_Preview)
    , eSensorMode(ESensorMode_Preview)
    , eIdx_Scene(MTK_CONTROL_SCENE_MODE_DISABLED)
    , eIdx_Effect(MTK_CONTROL_EFFECT_MODE_OFF)
    {
        rIspUsrSelectLevel.eIdx_Edge = MTK_CONTROL_ISP_EDGE_MIDDLE;
        rIspUsrSelectLevel.eIdx_Hue = MTK_CONTROL_ISP_HUE_MIDDLE;
        rIspUsrSelectLevel.eIdx_Sat = MTK_CONTROL_ISP_SATURATION_MIDDLE;
        rIspUsrSelectLevel.eIdx_Bright = MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE;
        rIspUsrSelectLevel.eIdx_Contrast = MTK_CONTROL_ISP_CONTRAST_MIDDLE;
    }

public:
    void dump() const
    {
        MY_LOG("[IspCamInfo][dump](eIspProfile, eSensorMode, eIdx_Scene, eIdx_Effect)=(%d, %d, %d, %d)", eIspProfile, eSensorMode, eIdx_Scene, eIdx_Effect);
    }
};


/*******************************************************************************
*
*******************************************************************************/
struct RAWIspCamInfo : public IspCamInfo
{
public:
    MUINT32              u4ISOValue;       // ISO value
    EIndex_ISO_T         eIdx_ISO;         // ISO index
    EIndex_ISO_T         eIdx_ISO_U;         // Upper ISO index
    EIndex_ISO_T         eIdx_ISO_L;         // Lower ISO index
    EIndex_PCA_LUT_CT_T  eIdx_CT;
    EIndex_PCA_LUT_CT_T  eIdx_CT_U;
    EIndex_PCA_LUT_CT_T  eIdx_CT_L;
    EIndex_PCA_LUT_T     eIdx_PCA_LUT;     // Index for PCA
    EIndex_CCM_T         eIdx_CCM;         // Index for CCM.
    EIndex_Shading_CCT_T eIdx_Shading_CCT; // Index for Shading.
    EIndex_ZOOM_T        eIdx_Zoom;
    EIndex_ANR_TBL_CT_T  eIdx_ANR_TBL_CT;  // CT Index for ANR_TBL.
    EIndex_ANR_TBL_ISO_T eIdx_ANR_TBL_ISO; // ISO Index for ANR_TBL.
    AWB_INFO_T           rAWBInfo;         // AWB info for ISP tuning
    AE_INFO_T            rAEInfo;          // AE info for ISP tuning
    AF_INFO_T            rAFInfo;          // AF info for ISP tuning
    FLASH_INFO_T         rFlashInfo;       // Flash info for ISP tuning
    MINT32               i4ZoomRatio_x100; // Zoom ratio x 100
    ISP_INT_INFO_T       rIspIntInfo;      // ISP interpolation info
    ISP_RRZ_INFO_T       rRrzInfo;
    MINT32               i4LightValue_x10; // LV x 10
    CROP_RZ_INFO_T       rCropRzInfo;      // crop and resize info
    MBOOL                fgRPGEnable;      // RPG enable
    MINT32               i4RawType;        // "IMGO" raw type
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;
    ISP_NVRAM_CCM_T rMtkCCM;
    ISP_NVRAM_GGM_T rMtkGGM;
    ISP_NVRAM_OBC_T rOBC1;
    //ISP_CCM_T rColorCorrectionTransform;
    mtk_camera_metadata_enum_android_edge_mode_t eEdgeMode;
    mtk_camera_metadata_enum_android_noise_reduction_mode_t eNRMode;
    mtk_camera_metadata_enum_android_tonemap_mode_t eToneMapMode;
    ISP_BIN_INFO_T       BinInfo;
    MBOOL                fgFDEnable;
    float                FDRatio;
    MBOOL                bBypassLCE;
    MBOOL                bBypassNR;
    MINT32               i4P2InImgFmt;
    MUINT8               u1P2TuningUpdate;
    MBOOL                fgHDR;
    mtk_camera_metadata_enum_android_control_mode_t  eControlMode;
public:
    RAWIspCamInfo()
        : IspCamInfo()
        , u4ISOValue(0)
        , eIdx_ISO(eIDX_ISO_100)
        , eIdx_ISO_U(eIDX_ISO_100)
        , eIdx_ISO_L(eIDX_ISO_100)
        , eIdx_CT(eIDX_PCA_CT_00)
        , eIdx_CT_U(eIDX_PCA_CT_00)
        , eIdx_CT_L(eIDX_PCA_CT_00)
        , eIdx_Zoom(eIDX_ZOOM_0)
        , eIdx_PCA_LUT(eIDX_PCA_LOW)
        , eIdx_CCM(eIDX_CCM_D65)
        , eIdx_Shading_CCT(eIDX_Shading_CCT_CWF)
        , eIdx_ANR_TBL_CT(eIDX_ANR_TBL_CCT_0)
        , eIdx_ANR_TBL_ISO(eIDX_ANR_TBL_ISO_0)
        , rAWBInfo()
        , rAEInfo()
        , rAFInfo()
        , rFlashInfo()
        , i4ZoomRatio_x100(100)
        , rIspIntInfo()
        , rRrzInfo()
        , i4LightValue_x10(0)
        , fgRPGEnable(MFALSE)
        , eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
        , rMtkCCM()
        , rMtkGGM()
        , rOBC1()
        //, rColorCorrectionTransform()
        , eEdgeMode(MTK_EDGE_MODE_FAST)
        , eNRMode(MTK_NOISE_REDUCTION_MODE_FAST)
        , eToneMapMode(MTK_TONEMAP_MODE_FAST)
        , fgFDEnable(MFALSE)
        , BinInfo()
        , FDRatio(0)
        , bBypassLCE(MFALSE)
        , bBypassNR(MFALSE)
        , fgHDR(MFALSE)
        , i4P2InImgFmt(0)
        , u1P2TuningUpdate(0)
        , i4RawType(NSIspTuning::ERawType_Proc)
        , eControlMode(MTK_CONTROL_MODE_OFF)
    {
         memset(&rCropRzInfo, 0, sizeof(CROP_RZ_INFO_T));
    }

public:
    void dump() const
    {
        IspCamInfo::dump();
        MY_LOG(
            "[RAWIspCamInfo][dump]"
            "(eIdx_ISO, u4ISOValue, i4ZoomRatio_x100, i4LightValue_x10)"
            "=(%d, %d, %d, %d)"
            , eIdx_ISO, u4ISOValue, i4ZoomRatio_x100, i4LightValue_x10
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
    MUINT32                 u4ISOValue;\
    EIndex_ISO_T            eIdx_ISO;   \
    EIndex_PCA_LUT_T        eIdx_PCA_LUT;\
    EIndex_CCM_T            eIdx_CCM;    \
    EIndex_Shading_CCT_T    eIdx_Shading_CCT;\
    EIndex_ANR_TBL_CT_T     eIdx_ANR_TBL_CT;\
    EIndex_ANR_TBL_ISO_T    eIdx_ANR_TBL_ISO;\
    AWB_INFO_T              rAWBInfo; \
    AE_INFO_T               rAEInfo; \
    AF_INFO_T               rAFInfo; \
    FLASH_INFO_T            rFlashInfo; \
    ISP_INT_INFO_T          rIspIntInfo; \
    ISP_RRZ_INFO_T          rRrzInfo;\
    MINT32                  i4ZoomRatio_x100;\
    CROP_RZ_INFO_T          rCropRzInfo;\
    MINT32                  i4LightValue_x10;\
    MBOOL                   fgRPGEnable;\
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;\
    ISP_NVRAM_CCM_T rMtkCCM;\
    ISP_NVRAM_GGM_T rMtkGGM;\
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
    std::string strFileNamePrefix;
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


/*******************************************************************************
*
*******************************************************************************/
};  //  NSIspTuning
#endif //  _ISP_TUNING_CAM_INFO_H_

