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
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <camera_custom_nvram.h>
#include <mtkcam/algorithm/lib3a/dynamic_ccm.h>

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
    eNUM_OF_ISO_IDX
} EIndex_ISO_T;

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
    //flash PCA
    eIDX_PCA_LOW_3,
    eIDX_PCA_MIDDLE_3,
    eIDX_PCA_HIGH_3    
} EIndex_PCA_LUT_T;


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
    MUINT32 u4CfaUpperISO;
    MUINT32 u4CfaLowerISO;
    MUINT16 u2CfaUpperIdx;
    MUINT16 u2CfaLowerIdx;
    MUINT32 u4AnrUpperISO;
    MUINT32 u4AnrLowerISO;
    MUINT16 u2AnrUpperIdx;
    MUINT16 u2AnrLowerIdx;
    MUINT32 u4EEUpperISO;
    MUINT32 u4EELowerISO;
    MUINT16 u2EEUpperIdx;
    MUINT16 u2EELowerIdx;    
} ISP_INT_INFO_T;

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
    , eIspProfile(EIspProfile_NormalPreview)
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
    MUINT32             u4ISOValue;         //  ISO value
    EIndex_ISO_T        eIdx_ISO;           //  ISO index
    EIndex_PCA_LUT_T     eIdx_PCA_LUT;     // Index for PCA
    EIndex_CCM_T         eIdx_CCM;         // Index for CCM.
    EIndex_Shading_CCT_T eIdx_Shading_CCT; // Index for Shading.
    AWB_INFO_T           rAWBInfo;         // AWB info for ISP tuning
    AE_INFO_T            rAEInfo;          // AE info for ISP tuning
    AF_INFO_T            rAFInfo;          // AF info for ISP tuning
    FLASH_INFO_T         rFlashInfo;       // Flash info for ISP tuning
    ISP_INT_INFO_T       rIspIntInfo;      // ISP interpolation info
    MINT32               i4ZoomRatio_x100; // Zoom ratio x 100
    MINT32               i4LightValue_x10; // LV x 10
    MBOOL                fgRPGEnable;      // RPG enable
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;
    ISP_NVRAM_CCM_T rMtkCCM;
    ISP_NVRAM_GGM_T rMtkGGM;
    ISP_CCM_T rColorCorrectionTransform;
    mtk_camera_metadata_enum_android_edge_mode_t eEdgeMode;
    mtk_camera_metadata_enum_android_noise_reduction_mode_t eNRMode;
    mtk_camera_metadata_enum_android_tonemap_mode_t eToneMapMode;
    MUINT8 fgToneMapInvert;

public:
    RAWIspCamInfo()
        : IspCamInfo()
        , u4ISOValue(0)
        , eIdx_ISO(eIDX_ISO_100)
        , eIdx_PCA_LUT(eIDX_PCA_LOW)
        , eIdx_CCM(eIDX_CCM_D65)
        , eIdx_Shading_CCT(eIDX_Shading_CCT_CWF)
        , rAWBInfo()
        , rAEInfo()
        , rAFInfo()
        , rFlashInfo()
        , rIspIntInfo()
        , i4ZoomRatio_x100(0)
        , i4LightValue_x10(0)
        , fgRPGEnable(MFALSE)
        , eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
        , rMtkCCM()
        , rMtkGGM()
        , rColorCorrectionTransform()
        , eEdgeMode(MTK_EDGE_MODE_FAST)
        , eNRMode(MTK_NOISE_REDUCTION_MODE_FAST)
        , eToneMapMode(MTK_TONEMAP_MODE_FAST)
        , fgToneMapInvert(0)
    {}

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
    AWB_INFO_T              rAWBInfo; \
    AE_INFO_T               rAEInfo; \
    AF_INFO_T               rAFInfo; \
    FLASH_INFO_T            rFlashInfo; \
    ISP_INT_INFO_T          rIspIntInfo; \
    MINT32                  i4ZoomRatio_x100;\
    MINT32                  i4LightValue_x10;\
    MBOOL                   fgRPGEnable;\
    mtk_camera_metadata_enum_android_color_correction_mode_t eColorCorrectionMode;\
    ISP_NVRAM_CCM_T rMtkCCM;\
    ISP_NVRAM_GGM_T rMtkGGM;\
    ISP_CCM_T rColorCorrectionTransform;\
    mtk_camera_metadata_enum_android_edge_mode_t eEdgeMode;\
    mtk_camera_metadata_enum_android_noise_reduction_mode_t eNRMode;\
    mtk_camera_metadata_enum_android_tonemap_mode_t eToneMapMode;\
    MUINT8 fgToneMapInvert;\
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

